#include <stdio.h>
#include <assert.h>

#ifdef WIN32
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winmm.lib")
#endif

#ifndef WIN32
#include <sys/time.h>
#endif

#include "hsutils.h"
#include "HSPackets.h"
#include "hssocket.h"

#include "HSNetwork.h"

CHSNetwork HSNetwork;           // One instance of this.

void NETRECEIVECALLBACK(const char *pcData, unsigned int uiDataLen,
                        CHSSocket * pSocket)
{
    HSNetwork.OnReceive(pcData, uiDataLen, pSocket);
}

void NETCLOSECALLBACK(CHSSocket * pSocket)
{
    HSNetwork.OnClose(pSocket);
}

void NETACCEPTCALLBACK(CHSSocket * pSocket)
{
    HSNetwork.OnAccept(pSocket);
}

CHSNetwork::~CHSNetwork()
{
    while (!m_quePackets.empty())
    {
        CHSPacket *pPacket = m_quePackets.front();

        delete pPacket;

        m_quePackets.pop_front();
    }

    while (!m_listSockets.empty())
    {
        CHSSocket *pSocket = m_listSockets.front();
        pSocket->Disconnect();
        pSocket->Release();

        m_listSockets.pop_front();
    }
}

void CHSNetwork::OnClose(CHSSocket * pSocket)
{
    CHSPConnectionClosed *pClosed = new CHSPConnectionClosed;

    pClosed->SetPacketAddress((unsigned int) pSocket);

    m_quePackets.push_back(pClosed);

    pSocket->Release();
}

void CHSNetwork::OnAccept(CHSSocket * pSocket)
{
    if (pSocket)
    {
        m_listSockets.push_back(pSocket);
    }
}

void CHSNetwork::OnReceive(const char *pcData,
                           unsigned int uiDataLen, CHSSocket * pSocket)
{
    // The first 16 bits of the data should be the packet type.
    unsigned short usType = 0;

    CHSDataStream stream;

    stream.WriteToStream((void *) pcData, uiDataLen);

    stream.SeekToBegin();

    while (stream.GetStreamRemaining() > sizeof(unsigned short))
    {
        stream.ReadFromStream(usType);

        CHSPacket *pPacket = CreatePacket((EHSPacketType) usType);
        if (!pPacket)
        {
            // Failure to create a packet.  The data left in this stream
            // could contain anything.  We're forced to throw it all out.
            return;
        }

        // We have a packet.  Have the packet read from the stream.
        if (!pPacket->ReadFromStream(stream))
        {
            // Bad read.
            return;
        }
        else
        {
            // Good packet ... onto the queue!
            pPacket->SetPacketAddress((unsigned int) pSocket);

            // Is this a packet we've been waiting for?
            if (m_eWaitingFor != PT_INVALID)
            {
                if (pPacket->GetPacketType() == m_eWaitingFor)
                {
                    // Flip the flag, so the waiting function knows it's arrived.
                    m_eWaitingFor = PT_INVALID;

                    // This packet goes to the front.
                    m_quePackets.push_front(pPacket);
                }
                else
                {
                    m_quePackets.push_back(pPacket);
                }
            }
            else
            {
                m_quePackets.push_back(pPacket);
            }
        }
    }
}

CHSPacket *CHSNetwork::GetPendingPacket()
{
    if (m_quePackets.empty())
    {
        return NULL;
    }

    CHSPacket *pPacket = m_quePackets.front();

    m_quePackets.pop_front();

    return pPacket;
}

bool CHSNetwork::InitNetwork()
{
#ifdef WIN32
    WORD wVersion = MAKEWORD(2, 2);
    WSADATA wsaData;

    WSAStartup(wVersion, &wsaData);
#endif

    return true;
}

void CHSNetwork::Shutdown()
{
    CSTLSocketList::iterator iter;
    for (iter = m_listSockets.begin(); iter != m_listSockets.end(); iter++)
    {
        (*iter)->Release();
    }
    m_listSockets.clear();
    m_bClientMode = m_bServerMode = false;
}

unsigned int
    CHSNetwork::Connect(const char *pcAddress,
                        unsigned short usPort,
                        const char *pcPlayerName, const char *pcPassword)
{
    // The CHSNetwork cannot operate in both client and server mode.
    if (m_bServerMode)
    {
        return 0;
    }

    if (!InitNetwork())
    {
        return false;           // Unknown network problem.
    }

    // Allocate a CHSSocket.
    CHSSocket *pSocket = new CHSSocket;
    pSocket->SetCallbacks(NETRECEIVECALLBACK, NULL, NETCLOSECALLBACK);
    if (pSocket->Connect(pcAddress, usPort))
    {
        m_listSockets.push_back(pSocket);
        m_bClientMode = true;
        m_bInitialized = true;

        // Send a login packet.
        CHSPLogin cmdLogin;

        cmdLogin.m_pcPassword = (char *) pcPassword;
        cmdLogin.m_pcPlayerName = (char *) pcPlayerName;

        cmdLogin.SetPacketAddress((unsigned int) pSocket);

        SendPacket(cmdLogin);

        // Wait for the login response.
        if (WaitForPacket(PT_LOGIN_RESPONSE))
        {
            // The login response came back.  It should be the first on the list.
            CHSPacket *pPacket = GetPendingPacket();

            if (pPacket->GetPacketType() == PT_LOGIN_RESPONSE)
            {
                CHSPLoginResponse *cmdResponse =
                    static_cast < CHSPLoginResponse * >(pPacket);

                if (cmdResponse->m_bLoggedIn)
                {
                    return (unsigned int) pSocket;
                }
            }
        }
    }

    // Everything's in the toilet.
    return 0;
}

void CHSNetwork::CloseConnection(unsigned int uiConnIdentifier)
{
    CHSSocket *pTheSocket = NULL;

    // If we're in client mode, just close the first connection.
    if (m_bClientMode)
    {
        if (!m_listSockets.empty())
        {
            pTheSocket = m_listSockets.front();
            m_listSockets.pop_front();
        }
    }
    else
    {
        // Find the connection to close
        CSTLSocketList::iterator iter;
        for (iter = m_listSockets.begin(); iter != m_listSockets.end();
             iter++)
        {
            CHSSocket *pSocket = *iter;

            if ((unsigned int) pSocket == uiConnIdentifier)
            {
                pTheSocket = pSocket;
                m_listSockets.erase(iter);
                break;
            }
        }
    }

    if (pTheSocket)
    {
        // This is the one.
        pTheSocket->Disconnect();
        pTheSocket->Release();
    }
}

void CHSNetwork::Process()
{
    // Do we have any open sockets?
    if (m_listSockets.empty())
    {
        return;
    }

    CSTLSocketList listDeadSockets;

    CSTLSocketList::iterator iter;
    for (iter = m_listSockets.begin(); iter != m_listSockets.end(); iter++)
    {
        CHSSocket *pSocket = *iter;

        pSocket->AddRef();
        pSocket->Process();
        if (pSocket->GetRefCount() == 1)
        {
            // Add a ref so the next release doesn't kill the socket.
            pSocket->AddRef();
            listDeadSockets.push_back(pSocket);
        }
        pSocket->Release();
    }

    // Clean up any sockets that went away during process.
    while (!listDeadSockets.empty())
    {
        CHSSocket *pSocket = listDeadSockets.front();
        listDeadSockets.pop_front();

        for (iter = m_listSockets.begin(); iter != m_listSockets.end();
             iter++)
        {
            CHSSocket *pCompare = *iter;

            if (pCompare == pSocket)
            {
                m_listSockets.erase(iter);
                break;
            }
        }

        pSocket->Release();
    }
}

bool CHSNetwork::Listen(unsigned short usPort)
{
    if (m_bClientMode)
    {
        // The client cannot also be a server.
        return false;
    }

    if (!InitNetwork())
    {
        return false;           // Unknown network problem.
    }

    // Open a new listening socket.
    CHSSocket *pSocket = new CHSSocket;
    pSocket->SetCallbacks(NETRECEIVECALLBACK, NETACCEPTCALLBACK,
                          NETCLOSECALLBACK);
    if (pSocket->Listen(usPort))
    {
        m_listSockets.push_back(pSocket);
        m_bServerMode = true;
        m_bInitialized = true;
        return true;
    }

    // Listen failure.
    return false;
}

bool CHSNetwork::SendPacket(CHSPacket & rPacket)
{
    CHSSocket *pTheSocket = NULL;

    // If we're in client mode, just use the first socket in the list.
    if (m_bClientMode && !m_listSockets.empty())
    {
        pTheSocket = m_listSockets.front();
    }
    else
    {
        // Find the connection through which this packet should go.
        CSTLSocketList::iterator iter;
        for (iter = m_listSockets.begin(); iter != m_listSockets.end();
             iter++)
        {
            CHSSocket *pSocket = *iter;

            unsigned int uiDestSocket = rPacket.GetPacketAddress();

            if ((unsigned int) pSocket == uiDestSocket)
            {
                // This is the one.
                pTheSocket = pSocket;
                break;
            }
        }
    }

    if (pTheSocket)
    {
        // Encode the packet type and packet itself to a data stream.
        CHSDataStream stream;

        unsigned short usType = rPacket.GetPacketType();
        stream.WriteToStream(usType);
        if (rPacket.WriteToStream(stream))
        {
            if (pTheSocket->
                Send(stream.GetBuffer(), stream.GetCurrentPosition()))
            {
                return true;
            }
            else
            {
                return false;   // Failed to send.
            }
        }
        else
        {
            return false;       // Failed to write stream.
        }
    }

    return false;               // Failed to find the socket.
}

CHSPacket *CHSNetwork::CreatePacket(EHSPacketType eType)
{
    switch (eType)
    {
    case PT_CONNECTION_CLOSED:
        return new CHSPConnectionClosed;

    case PT_LOGIN:
        return new CHSPLogin;

    case PT_LOGIN_RESPONSE:
        return new CHSPLoginResponse;

    case PT_GET_CLASS_DATA:
        return new CHSPGetClassData;

    case PT_GET_CLASS_LIST:
        return new CHSPGetClassList;

    case PT_CLASS_LIST:
        return new CHSPClassList;

    case PT_CLASS_DATA:
        return new CHSPClassData;

    case PT_SET_CLASS_DATA:
        return new CHSPSetClassData;

    case PT_GET_SYSTEM_LIST:
        return new CHSPGetSystemList;

    case PT_SYSTEM_LIST:
        return new CHSPSystemList;

    case PT_CREATE_CLASS:
        return new CHSPCreateClass;

    case PT_CREATE_CLASS_RESPONSE:
        return new CHSPCreateClassResponse;

    case PT_SET_CLASS_DATA_RESPONSE:
        return new CHSPSetClassDataResponse;

    case PT_GET_SYSTEM_DATA:
        return new CHSPGetSystemData;

    case PT_SYSTEM_DATA:
        return new CHSPSystemData;

    case PT_SET_SYSTEM_DATA:
        return new CHSPSetSystemData;

    case PT_SET_SYSTEM_DATA_RESPONSE:
        return new CHSPSetSystemDataResponse;

    case PT_DELETE_CLASS:
        return new CHSPDeleteClass;

    case PT_DELETE_CLASS_RESPONSE:
        return new CHSPDeleteClassResponse;

    case PT_ADD_SYSTEM:
        return new CHSPAddSystem;

    case PT_ADD_SYSTEM_RESPONSE:
        return new CHSPAddSystemResponse;

    case PT_DELETE_SYSTEM:
        return new CHSPDeleteSystem;

    case PT_DELETE_SYSTEM_RESPONSE:
        return new CHSPDeleteSystemResponse;

    case PT_GET_UNIVERSE_LIST:
        return new CHSPGetUniverseList;

    case PT_UNIVERSE_LIST:
        return new CHSPUniverseList;

    case PT_DELETE_UNIVERSE:
        return new CHSPDeleteUniverse;

    case PT_DELETE_UNIVERSE_RESPONSE:
        return new CHSPDeleteUniverseResp;

    case PT_GET_OBJECT_LIST:
        return new CHSPGetObjectList;

    case PT_OBJECT_LIST:
        return new CHSPObjectList;

    case PT_GET_OBJECT_DATA:
        return new CHSPGetObjectData;

    case PT_OBJECT_DATA:
        return new CHSPObjectData;

    case PT_SET_OBJECT_DATA:
        return new CHSPSetObjectData;

    case PT_SET_OBJECT_DATA_RESPONSE:
        return new CHSPSetObjectDataResponse;

    case PT_GET_WEAPON_LIST:
        return new CHSPGetWeaponList;

    case PT_WEAPON_LIST:
        return new CHSPWeaponList;

    case PT_CREATE_WEAPON:
        return new CHSPCreateWeapon;

    case PT_CREATE_WEAPON_RESPONSE:
        return new CHSPCreateWeaponResponse;

    case PT_GET_WEAPON_DATA:
        return new CHSPGetWeaponData;

    case PT_WEAPON_DATA:
        return new CHSPWeaponData;

    case PT_SET_WEAPON_DATA:
        return new CHSPSetWeaponData;

    case PT_SET_WEAPON_DATA_RESPONSE:
        return new CHSPSetWeaponDataResponse;

    case PT_INVALID:
    default:
        {
            assert(0);          // A packet type was requested that is not supported.
            return NULL;
        }
    }
}

bool CHSNetwork::WaitForPacket(EHSPacketType eType, unsigned int uiTimeout)
{
    // Go modal, waiting for the specified packet to arrive.
    m_eWaitingFor = eType;

    // Grab the current milliseconds
    double uiCurTime = 0;

#ifdef WIN32
    uiCurTime = timeGetTime();
#else
    struct timeval tv;
    gettimeofday(&tv, (struct timezone *) NULL);
    uiCurTime = (tv.tv_sec * 1000000) + tv.tv_usec;

    // Unix time is usecs, scale timeout value
    uiTimeout *= 1000;
#endif

    double uiEndTime = uiCurTime + uiTimeout;

    // Loop while we don't timeout.
    while (uiCurTime < uiEndTime)
    {
        Process();

        // If the waiting for variable flips, then the packet arrived.
        if (m_eWaitingFor == PT_INVALID)
        {
            // YAY!
            return true;
        }

        // Update the time
#ifdef WIN32
        uiCurTime = timeGetTime();
#else
        struct timeval tv;
        gettimeofday(&tv, (struct timezone *) NULL);
        uiCurTime = (tv.tv_sec * 1000000) + tv.tv_usec;
#endif
    }

    // The packet failed to arrive in the given time.
    return false;
}
