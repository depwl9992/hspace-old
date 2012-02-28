#ifdef _WIN32
#pragma once
#pragma warning(disable:4786)
#endif

#if !defined(__HSNETWORK_H__)
#define __HSNETWORK_H__


// Library Includes
#include "HSPacketTypes.h"

#include <queue>
#include <list>

// Local Includes

// Forward Declarations
class CHSPacket;
class CHSSocket;

// Types
typedef std::list < CHSPacket * >CSTLPacketQueue;
typedef std::list < CHSSocket * >CSTLSocketList;

// Constants

// Prototypes

// The Singleton CHSNetwork class.
//
// Communication to and from the server is accomplished through this class.
//
// To use this class, simply do the following:
//
// Send a packet: HSNetwork.SendPacket(<reference to packet>);
//
// Give the network processing time: HSNetwork.Process();
//
// Receive packets from the network: HSNetwork.GetPendingPacket();
class CHSNetwork
{
    // Member Variables
  public:

  protected:


    CSTLSocketList m_listSockets;       // A list of sockets current connected.

    CSTLPacketQueue m_quePackets;       // Packets that were received from the network.

    bool m_bServerMode;         // True if in server mode.
    bool m_bClientMode;         // True if in client mode.
    bool m_bInitialized;

    EHSPacketType m_eWaitingFor;

    // Member Functions
  public:

    CHSNetwork():
        m_bServerMode(false),
        m_bClientMode(false), m_bInitialized(false), m_eWaitingFor(PT_INVALID)
    {
    }

    ~CHSNetwork();

    // Call this function to make a connection to a remote server.
    // You must supply a player name and password, or the server will refuse your login.
    // The return value is a 32-bit identifier for the connection to the server.  You must
    // use this for all correspondence with the server.  Refer to CHSPacket::SetPacketAddress()
    // for more info.
    //
    // If an error occurs, 0 is returned.
    unsigned int Connect(const char *pcAddress,
                         unsigned short usPort,
                         const char *pcPlayerName, const char *pcPassword);

    // Completely shutdown the network, closing all connections.
    void Shutdown();

    // Call this function to open the CHSNetwork in server mode.  You really should never
    // be doing this unless you're the HSpace server itself.
    bool Listen(unsigned short usPort);

    // Call this function, giving it the 32-bit connection identifier to close the
    // connection.  The identifier is the one returned from Connect() or stored in a
    // CHSPacket that was received from the network.
    void CloseConnection(unsigned int uiConnIdentifier);

    // Call this to send a packet to the network.  If you are a client, and not
    // the server, you do not need to specify an address to which to send the packet.
    // It will automatically be sent to the first connection in the network, which is to
    // the server.
    bool SendPacket(CHSPacket & /*rPacket */ );

    // Call this to have the network process its connection(s) and receive network data.
    void Process();

    // Call this to pop one inbound packet from the queu.  When this returns NULL, there
    // are no more packets to handle.  The CHSPacket returned from this function belongs
    // to you, and YOU must delete it when you are done.
    CHSPacket *GetPendingPacket();

    // Do not call this function.  It is called internally during processing.
    void OnReceive(const char *pcData,
                   unsigned int uiDataLen, CHSSocket * pSocket);

    // Do not call this function.  It is called internally during processing.
    void OnClose(CHSSocket * pSocket);

    // Do not call this function.  It is called internally during processing.
    void OnAccept(CHSSocket * pSocket);

    // You can use this to wait for a specific packet.  You will get a true/false return
    // value if the packet arrived or if the packet did not arrive in the specified timeout
    // period.  The timeout is in milliseconds.
    //
    // If the packet you are waiting for arrives, and this function returns true, the packet
    // you want will be the first packet returned from GetPendingPacket().
    bool WaitForPacket(EHSPacketType eType, unsigned int uiTimeout = 5000);

    CHSPacket *CreatePacket(EHSPacketType eType);

    // This is only valid for clients.  Server's have multiple connections.
    bool IsConnected() const
    {
        if (!m_bInitialized)
        {
            return false;
        }

        if  (!m_bClientMode)
        {
            return true;
        }

        // If we have a socket in the list, we're connected.
        return !(m_listSockets.empty());
    }

  protected:

    bool InitNetwork();

};

extern CHSNetwork HSNetwork;

#endif // __HSNETWORK_H__
