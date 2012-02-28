// -----------------------------------------------------------------------
// $Id: hsserver.cpp,v 1.10 2006/04/04 12:41:11 mark Exp $
// -----------------------------------------------------------------------

#ifdef WIN32
#pragma warning(disable:4786)
#endif

// check for the odd location for select() under cygwin
#if defined(__CYGWIN32__) || defined(__MINGW32__)
#include <sys/select.h>
#endif

#if defined(__FreeBSD__)
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#endif

#ifndef WIN32
#include <time.h>
#include <sys/timeb.h>
#include <pthread.h>
#else
#include <Windows.h>
#endif

#include "hstypes.h"
#include "HSNetwork.h"
#include "HSPacketHandler.h"
#include "HSPackets.h"
#include "hsutils.h"
#include "HSHandlerLogin.h"
#include "HSHandlerClass.h"
#include "HSHandlerMisc.h"
#include "HSHandlerUniverse.h"
#include "HSHandlerObject.h"
#include "HSHandlerWeapon.h"
#include "hsconf.h"

#include "hsserver.h"

HS_BOOL8 g_bHSThreadShutdown = false;

#define CREATE_HANDLER_OBJECT(CLASS_NAME) \
	CLASS_NAME*		p##CLASS_NAME = new CLASS_NAME; \
	if (!p##CLASS_NAME->Initialize()) \
	{\
		delete p##CLASS_NAME; \
		return false;\
	}\
	else \
	{\
		m_listPacketHandlers.push_back(p##CLASS_NAME);\
	}

CHSServer myCHSServer;


#ifdef WIN32
unsigned long __stdcall
#else
void *
#endif
HSServerThread(void *pvParam)
{
#ifndef WIN32
    struct timeval tv;

    tv.tv_sec = 0;
    tv.tv_usec = 20000;
#endif

    // Loop every 20 ms.
    while (1)
    {

        hsEnterMutex();

        if (g_bHSThreadShutdown)
        {
            hsLeaveMutex();
#ifdef WIN32
            ExitThread(0);
#else
            pthread_exit(NULL);
#endif
        }
        else
        {
            CHSServer::GetInstance().DoThreadedCycle();

            hsLeaveMutex();
#ifdef WIN32
            SleepEx(20, true);
#else
            select(0, NULL, NULL, NULL, &tv);
            tv.tv_sec = 0;
            tv.tv_usec = 20000;
#endif
        }
    }
}

HS_BOOL8 CHSServerDef::StartServer(HS_UINT16 usPort)
{
    // Tell the CHSNetwork singleton to listen.
    if (HSNetwork.Listen(usPort) == true)
    {
        // We're listening, so register all packet handlers and initializers.
        CREATE_HANDLER_OBJECT(CHSHandlerLogin)
            CREATE_HANDLER_OBJECT(CHSHandlerClass)
            CREATE_HANDLER_OBJECT(CHSHandlerMisc)
            CREATE_HANDLER_OBJECT(CHSHandlerUniverse)
            CREATE_HANDLER_OBJECT(CHSHandlerObject)
            CREATE_HANDLER_OBJECT(CHSHandlerWeapon)
            // Is the admin server threaded?
            if (HSCONF.admin_server_threaded)
        {
#ifdef WIN32
            DWORD dwThreadID;
            m_tWorkerThread =
                CreateThread(NULL, 0, &HSServerThread, NULL, 0, &dwThreadID);

            if (m_tWorkerThread != NULL)
            {
                hs_log("ADMIN SERVER: Created in a separate thread.");
                return true;
            }
#else
            if (pthread_create(&m_tWorkerThread, NULL, &HSServerThread, NULL)
                == 0)
            {
                hs_log("ADMIN SERVER: Created in a separate thread.");
                return true;
            }
#endif
            return false;
        }
        hs_log("ADMIN SERVER: Created in main process thread.");
        return true;
    }

    return false;
}

void CHSServerDef::ShutdownServer()
{

    if (HSCONF.admin_server_threaded)
    {
        hsEnterMutex();
        g_bHSThreadShutdown = true;
        HSNetwork.Shutdown();
#ifdef WIN32

        // Harsh, but seemingly necessary.
        TerminateThread(m_tWorkerThread, 0);
#endif
        hsLeaveMutex();
    }
}

void CHSServerDef::DoCycle()
{
    // If the admin server is not threaded, run the thread cycle too.
    if (!HSCONF.admin_server_threaded)
    {
        DoThreadedCycle();
    }

    // Process packets that need handled in the main thread.
    while (!m_queThreadUnsafePackets.empty())
    {
        CHSPacket *pPacket = m_queThreadUnsafePackets.front();

        m_queThreadUnsafePackets.pop_front();

        HandlePacket(pPacket);

        delete pPacket;
    }
}

void CHSServerDef::DoThreadedCycle()
{
    // Process the network.
    HSNetwork.Process();

    // See if any inbound packets are waiting to be processed.
    CHSPacket *pPacket = HSNetwork.GetPendingPacket();
    while (pPacket)
    {
        // If this is not a login packet, the source address needs to already
        // be validated.
        if (pPacket->GetPacketType() != PT_LOGIN)
        {
            // Check to see if this source address is validated.
            CSTLValidatedPlayerSet::iterator iter =
                m_setValidatedPlayers.find(pPacket->GetPacketAddress());
            if (iter == m_setValidatedPlayers.end())
            {
                // This is someone trying to send a packet when they are not yet logged in.
                HSNetwork.CloseConnection(pPacket->GetPacketAddress());

                delete pPacket;

                pPacket = HSNetwork.GetPendingPacket();
                continue;
            }
            else
            {
                // If this is a closed connection, remove the player from the validated set.
                if (pPacket->GetPacketType() == PT_CONNECTION_CLOSED)
                {
                    m_setValidatedPlayers.erase(iter);

                    delete pPacket;

                    pPacket = HSNetwork.GetPendingPacket();
                    continue;
                }
            }
        }

        // The packet has passed validation.  

        // Is the handler for this packet ok to run in the worker thread?
        if (!IsHandlerThreadSafe(pPacket->GetPacketType()))
        {
            // Put this packet on the main thread queue.
            m_queThreadUnsafePackets.push_back(pPacket);
        }
        else
        {
            // Find a handler for it.
            HandlePacket(pPacket);
            delete pPacket;
        }

        pPacket = HSNetwork.GetPendingPacket();
    }
}

void CHSServerDef::HandlePacket(CHSPacket * pPacket)
{

    CSTLPacketHandlerMap::iterator iter =
        m_mapPacketHandlers.find(pPacket->GetPacketType());
    if (iter == m_mapPacketHandlers.end())
    {
        hs_log("Failed to find a packet handler for an inbound packet.");
    }
    else
    {
        THSPacketHandlerEntry & rtEntry = iter->second;

        CHSPacketHandler *pHandler = rtEntry.pHandler;

        ((pHandler)->*rtEntry.pFunc) (pPacket);
    }
}

void CHSServerDef::AddValidatedPlayer(unsigned int iContext)
{
    m_setValidatedPlayers.insert(iContext);
}

HS_BOOL8 CHSServerDef::RegisterPacketHandler(EHSPacketType eType,
                                             CHSPacketHandler * pHandler,
                                             PACKET_HANDLER_FUNC pFunc,
                                             HS_BOOL8 bThreadSafe)
{
    if (!pFunc)
    {
        hs_log("NULL handler sent to RegisterPacketHandler().");
        return false;
    }

    CSTLPacketHandlerMap::iterator iter;

    if ((iter = m_mapPacketHandlers.find(eType)) != m_mapPacketHandlers.end())
    {
        // There's already a handler registered for this type.
        hs_log("Handler already registered for specified packet type.");
        return false;
    }

    THSPacketHandlerEntry tEntry;

    tEntry.pFunc = pFunc;
    tEntry.pHandler = pHandler;
    tEntry.bThreadSafe = bThreadSafe;

    m_mapPacketHandlers[eType] = tEntry;

    return true;                // Good to go.
}

HS_BOOL8 CHSServerDef::IsHandlerThreadSafe(EHSPacketType eType)
{
    CSTLPacketHandlerMap::iterator iter;

    if ((iter = m_mapPacketHandlers.find(eType)) != m_mapPacketHandlers.end())
    {
        // Check to see if this handler is thread safe.
        THSPacketHandlerEntry & rtEntry = iter->second;
        return rtEntry.bThreadSafe;
    }
    return true;
}
