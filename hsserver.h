// -----------------------------------------------------------------------
// $Id: hsserver.h,v 1.5 2006/04/04 12:41:36 mark Exp $
// -----------------------------------------------------------------------
#ifndef __HSSERVER_INCLUDED__
#define __HSSERVER_INCLUDED__

#if defined(__FreeBSD__)
#include <pthread.h>
#endif

// Library includes
#include <map>
#include <set>
#include <list>

#include "HSSingleton.h"
#include "HSPacketHandler.h"
#include "HSPacketTypes.h"

// Forwards
class CHSPacket;

//! Typedef the packet handler functionality for readability and laziness
typedef void (CHSPacketHandler::*PACKET_HANDLER_FUNC) (CHSPacket *);

//! Standard entry for a packet handler
struct THSPacketHandlerEntry
{
    PACKET_HANDLER_FUNC pFunc;
    CHSPacketHandler *pHandler;
    HS_BOOL8 bThreadSafe;
};

typedef std::map < EHSPacketType,
    THSPacketHandlerEntry > CSTLPacketHandlerMap;
typedef std::set < unsigned int >CSTLValidatedPlayerSet;
typedef std::list < CHSPacketHandler * >CSTLPacketHandlerList;

// Constants
#define REGISTER_PACKET_HANDLER(packet_type, handler_function, thread_safe) \
	CHSServer::GetInstance().RegisterPacketHandler(packet_type, this, static_cast<PACKET_HANDLER_FUNC>(handler_function), thread_safe)

//! AdminPort handler for processing external connections to get and set
//! space data
class CHSServerDef:public CHSSingleton < CHSServerDef >
{
  public:
    typedef std::list < CHSPacket * >CSTLPacketQueue;

    //! The empty default constructor
    CHSServerDef()
    {
    }

    //! @brief Start the admin server on the specified port.  If the
    //! configuration calls for a separate thread, spawn it. Also 
    //! register all current packet handlers
    //! @return true on success, false on failure
    HS_BOOL8 StartServer(HS_UINT16 usPort);

    //! Cleanup and close down the port used for the admin server
    void ShutdownServer();

    //! Run one cycle of the admin server, called from the game thread
    //! so it performs safe operations within the game
    void DoCycle();

    //! This runs in the worker thread.  It is ok to do things
    //! inside of HSpace, but it may not access the main game.
    void DoThreadedCycle();

    //! Register a specific packet handler
    HS_BOOL8 RegisterPacketHandler(EHSPacketType eType,
                                   CHSPacketHandler * pHandler,
                                   PACKET_HANDLER_FUNC pFunc,
                                   HS_BOOL8 bThreadSafe);

    //! Add an authenticated player to the list
    void AddValidatedPlayer(unsigned int iContext);

  protected:

    //! Current map of objects that will handle inbound packets
    CSTLPacketHandlerMap m_mapPacketHandlers;

    //! Set of validated players
    CSTLValidatedPlayerSet m_setValidatedPlayers;

    //! Current list of packet handler objects
    CSTLPacketHandlerList m_listPacketHandlers;

#ifdef WIN32
    HANDLE m_tWorkerThread;
#else
    pthread_t m_tWorkerThread;
#endif

    //! Current queue of packets that are waiting to be handled 
    CSTLPacketQueue m_queThreadUnsafePackets;

    //! Find a handler method for the incoming packet and call the handler func.
    void HandlePacket(CHSPacket * pPacket);

    //! Check to see if the handler method is thread safe
    HS_BOOL8 IsHandlerThreadSafe(EHSPacketType eType);
};

typedef CHSServerDef CHSServer;

#endif // __HSSERVER_INCLUDED__
