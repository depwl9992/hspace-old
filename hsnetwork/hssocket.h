#ifndef __HSSOCKET_INCLUDED__
#define __HSSOCKET_INCLUDED__

#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/types.h>
#include <netinet/in.h>
#endif

// Forward declarations
class CHSSocket;
class CHSPacket;

// consts
const unsigned short HSCONST_MAX_RECEIVE = 4096;
const unsigned short HSCONST_MAX_SEND = 4096;
const unsigned short HSCONST_MAX_PACKETSIZE = 4096;

// Types
typedef void (*RECEIVE_CALLBACK) (const char *pcData, unsigned int uiDataLen,
                                  CHSSocket * pSocket);
typedef void (*ACCEPT_CALLBACK) (CHSSocket * pSocket);
typedef void (*CLOSE_CALLBACK) (CHSSocket * pSocket);

// Wraps the tcp/ip socket and provides some abstracted funtionality.
class CHSSocket
{
  public:

  protected:

    unsigned int m_uiRefCount;

    char *m_pcReceivedBuffer;   // Data not yet processed.
    char *m_pcSendBuffer;

    unsigned int m_uiBytesReceived;
    unsigned int m_uiBytesPending;      // Data not yet received

    int m_iSocket;              // This is the TCP/IP socket.

    unsigned short m_usListenPort;      // Only valid for listening connections.

    bool m_bListenConnection;   // Is, or is not.

    // Callbacks
    RECEIVE_CALLBACK m_pReceiveCallback;
    ACCEPT_CALLBACK m_pAcceptCallback;
    CLOSE_CALLBACK m_pCloseCallback;

    struct sockaddr_in m_sinRemoteAddr;

  public:

        CHSSocket();

    bool Process();             // Returns FALSE if disconnected

    void Disconnect();

    bool Listen(unsigned short usPort);

    bool Connect(const char *pcAddress, unsigned short usPort);

    bool Send(const char *pcData, unsigned int uiDataLen);

    unsigned short GetListenPort() const
    {
        return m_usListenPort;
    }

    int GetSocket() const
    {
        return m_iSocket;
    }

    void SetCallbacks(RECEIVE_CALLBACK pReceive,
                      ACCEPT_CALLBACK pAccept, CLOSE_CALLBACK pClose)
    {
        m_pReceiveCallback = pReceive;
        m_pAcceptCallback = pAccept;
        m_pCloseCallback = pClose;
    }

    struct sockaddr_in &GetRemoteAddr()
    {
        return m_sinRemoteAddr;
    }

    void Release();

    void AddRef()
    {
        m_uiRefCount++;
    }

    unsigned int GetRefCount()
    {
        return m_uiRefCount;
    }

  protected:

    ~CHSSocket();

    bool AcceptConnection();
};


#endif // __HSSOCKET_INCLUDED__
