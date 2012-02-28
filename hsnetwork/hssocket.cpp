
#include "StdAfx.h"

#if !defined(WIN32)
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#else
#include <io.h>
#endif // !WIN32

#include "hstypes.h"
#include "HSDataStream.h"
#include "hsutils.h"
#include "hssocket.h"

CHSSocket::CHSSocket():
m_uiRefCount(1),
m_pcReceivedBuffer(NULL),
m_pcSendBuffer(NULL),
m_uiBytesReceived(0),
m_uiBytesPending(0),
m_iSocket(-1),
m_usListenPort(0),
m_bListenConnection(false),
m_pReceiveCallback(NULL), m_pAcceptCallback(NULL), m_pCloseCallback(NULL)
{
}

CHSSocket::~CHSSocket()
{
    Disconnect();

    if (m_pcReceivedBuffer)
    {
        delete[]m_pcReceivedBuffer;
    }
}

void CHSSocket::Release()
{
    if (m_uiRefCount == 0)
    {
        return;
    }

    m_uiRefCount--;

    if (m_uiRefCount == 0)
    {
        delete this;
    }
}

void CHSSocket::Disconnect()
{
    if (m_iSocket != -1)
    {
        shutdown(m_iSocket, 2);
        close(m_iSocket);
        m_iSocket = -1;
    }
}

bool CHSSocket::Connect(const char *pcAddress, unsigned short usPort)
{
    if (m_iSocket != -1)
    {
        return false;           // Already connected.
    }

    m_iSocket =::socket(AF_INET, SOCK_STREAM, 0);

    if (m_iSocket == -1)
    {
        // Failed to get a socket.
        return false;
    }

    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));

    sin.sin_family = AF_INET;
    sin.sin_port = htons(usPort);

    struct hostent *pHost;

    pHost = gethostbyname(pcAddress);
    if (!pHost)
    {
        return false;           // Failed to resolve address.
    }

    memcpy(&(sin.sin_addr.s_addr), pHost->h_addr, pHost->h_length);

    m_sinRemoteAddr = sin;

    if (connect(m_iSocket, (struct sockaddr *) &sin, sizeof(sin)) != 0)
    {
        return false;
    }

#ifdef WIN32
    int opt = 1;
    if (ioctlsocket(m_iSocket, FIONBIO, (ULONG *) & opt) != 0)
#else
    if (fcntl(m_iSocket, F_SETFL, O_NONBLOCK) != 0)
#endif
    {
        return false;
    }

    return true;
}


bool CHSSocket::Listen(unsigned short usPort)
{
    if (m_iSocket != -1)
    {
        // Already got a socket, so this object is already doing something.
        return false;
    }

    m_iSocket =::socket(AF_INET, SOCK_STREAM, 0);

    if (m_iSocket == -1)
    {
        // Failed to get a socket.
        return false;
    }

    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));

    sin.sin_family = AF_INET;
    sin.sin_port = htons(usPort);

    int opt;
    if (setsockopt
        (m_iSocket, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt)) < 0)
    {
        return false;
    }

    if (::bind(m_iSocket, (struct sockaddr *) &sin, sizeof(sin)) != 0)
    {
        return false;
    }

    if (::listen(m_iSocket, 1) < 0)
    {
        return false;
    }

    // We're bound and good to go.  Query the port we're listening on.
#ifdef WIN32
    int iSize = sizeof(sin);
    if (getsockname(m_iSocket, (struct sockaddr *) &sin, &iSize) < 0)
#else
    socklen_t len = sizeof(sin);
    if (getsockname(m_iSocket, (struct sockaddr *) &sin, &len) < 0)
#endif
    {
        // This is not a critical error.
    }

    m_usListenPort = ntohs(sin.sin_port);

    m_bListenConnection = true;

    return true;
}

bool CHSSocket::Process()
{
    fd_set fdReadSet;
    struct timeval timeout;
    int iSelectVal = 0;

    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    FD_ZERO(&fdReadSet);
    FD_SET(m_iSocket, &fdReadSet);

#ifdef WIN32
    if ((iSelectVal =
         select(1, &fdReadSet, NULL, NULL, (struct timeval *) &timeout)) < 0)
#else
    if ((iSelectVal =
         select(m_iSocket + 1, &fdReadSet, NULL, NULL,
                (struct timeval *) &timeout)) < 0)
#endif
    {
        // An error has occurred.  For now, just return false.
        return false;
    }
#ifdef WIN32
    else if ((iSelectVal == 1) && FD_ISSET(m_iSocket, &fdReadSet))
#else
    else if ((iSelectVal > 0) && FD_ISSET(m_iSocket, &fdReadSet))
#endif
    {
        // Our socket has something to read.
        if (m_bListenConnection)
        {
            // We have a new connection.  Accept it.
            if (!AcceptConnection())
            {
            }
        }
        else
        {
            // We're a normal data connection.

            // Allocate a buffer if needed.
            if (!m_pcReceivedBuffer)
            {
                m_pcReceivedBuffer = new char[HSCONST_MAX_RECEIVE];
                m_uiBytesPending = 0;
                m_uiBytesReceived = 0;
            }

            char *pCurPos = &(m_pcReceivedBuffer[m_uiBytesReceived]);
            unsigned int uiReceived;

            if ((uiReceived =
                 recv(m_iSocket, pCurPos,
                      HSCONST_MAX_RECEIVE - m_uiBytesReceived, 0)) <= 0)
            {
                if (m_pCloseCallback)
                {
                    m_pCloseCallback(this);
                }
                return false;   // Socket is shut down.
            }

            m_uiBytesReceived += uiReceived;

            // If we weren't expecting any data, determine how much we should receive.
            if (m_uiBytesPending == 0)
            {
                // Do we at least have enough data to decode the header size?
                if (uiReceived >= (int) sizeof(unsigned int))
                {
                    // Pull out the packet header.
                    m_uiBytesPending =
                        ntohl(*((unsigned int *) m_pcReceivedBuffer));

                    // Is this bigger than the allowed limit?
                    if (m_uiBytesPending > HSCONST_MAX_PACKETSIZE)
                    {
                        m_uiBytesPending = 0;
                        m_uiBytesReceived = 0;
                    }
                    else
                    {
                        // Double-check that the size specified corresponds to the data received.
                        if (m_uiBytesPending > uiReceived)
                        {
                            m_uiBytesPending = 0;
                            m_uiBytesReceived = 0;
                        }
                    }
                }
                else
                {
                    m_uiBytesReceived = 0;
                }
            }

            // Did we receive the data we were expecting?
            pCurPos = m_pcReceivedBuffer;
            while (m_uiBytesReceived
                   && (m_uiBytesReceived >= m_uiBytesPending))
            {
                // Skip over the size info.
                pCurPos += sizeof(unsigned int);

                // Call the receive callback, excluding the packet size.
                if (m_pReceiveCallback)
                {
                    m_pReceiveCallback(pCurPos,
                                       m_uiBytesPending -
                                       sizeof(unsigned int), this);
                }

                // Advance over this data.
                m_uiBytesReceived -= m_uiBytesPending;
                pCurPos += (m_uiBytesPending - sizeof(unsigned int));

                // No bytes pending that we know of.
                m_uiBytesPending = 0;

                // Is there more?
                if (m_uiBytesReceived > 0)
                {
                    // Do we at least have enough data to decode the header size?
                    if (m_uiBytesReceived >= sizeof(unsigned int))
                    {
                        // Pull out the packet header.
                        m_uiBytesPending = ntohl(*((unsigned int *) pCurPos));

                        // Is this bigger than the allowed limit?
                        if (m_uiBytesPending > HSCONST_MAX_PACKETSIZE)
                        {
                            m_uiBytesPending = 0;
                            m_uiBytesReceived = 0;
                        }
                        else
                        {
                            // Double-check that the size specified corresponds to the data received.
                            if (m_uiBytesPending > uiReceived)
                            {
                                m_uiBytesPending = 0;
                                m_uiBytesReceived = 0;
                            }
                        }
                    }
                    else
                    {
                        m_uiBytesPending = 0;
                        m_uiBytesReceived = 0;
                    }
                }
                else
                {
                    break;
                }
            }

            // If we processed any data, we need to move the remaining data to the beginning of the buffer.
            if ((pCurPos != m_pcReceivedBuffer) && (m_uiBytesPending > 0))
            {
                int iOffset = pCurPos - m_pcReceivedBuffer;
                int iLeftToRead = m_uiBytesReceived;

                while (iLeftToRead > 0)
                {
                    *(pCurPos - iOffset) = (*pCurPos)++;
                    iLeftToRead--;
                }
            }

            // At this point we should either have no data in receive buffer, not
            // enough data to determine the full packet type, or a partial packet
            // waiting for the rest to arrive.
        }
    }
    return true;
}

bool CHSSocket::AcceptConnection()
{
    int iNewSocket;
    struct sockaddr_in sAddr;

#ifdef WIN32
    iSize = sizeof(sAddr);
    iNewSocket =::accept(m_iSocket, (struct sockaddr *) &sAddr, &iSize);
#else
    socklen_t len = sizeof(sAddr);
    iNewSocket = accept(m_iSocket, (struct sockaddr *) &sAddr, &len);
#endif

    if (iNewSocket > 0)
    {
        // Create a new connection for this socket.
        CHSSocket *pSocket = new CHSSocket;

        pSocket->m_iSocket = iNewSocket;

        // Initialize callbacks to our callbacks.
        pSocket->SetCallbacks(m_pReceiveCallback, m_pAcceptCallback,
                              m_pCloseCallback);
        pSocket->m_sinRemoteAddr = *((struct sockaddr_in *) &sAddr);

        if (m_pAcceptCallback)
        {
            m_pAcceptCallback(pSocket);
        }
        else
        {
            delete pSocket;
        }

        return true;
    }

    return false;
}

bool CHSSocket::Send(const char *pcData, unsigned int uiDataLen)
{
    if (m_iSocket == -1)
    {
        return false;           // Not connected.
    }

    if (uiDataLen > HSCONST_MAX_SEND)
    {
        return false;
    }

    if (!m_pcSendBuffer)
    {
        m_pcSendBuffer = new char[HSCONST_MAX_SEND];

        if (!m_pcSendBuffer)
        {
            return false;       // Memory allocation probs
        }
    }

    // Encode the packet size.
    char *pcCurPos = m_pcSendBuffer;
    (*(unsigned int *) pcCurPos) = htonl(uiDataLen + sizeof(unsigned int));

    pcCurPos += sizeof(unsigned int);
    memcpy((void *) pcCurPos, pcData, uiDataLen);

    // Send it to the network.
    unsigned int uiBytesSent;

    uiBytesSent =
        send(m_iSocket, m_pcSendBuffer, uiDataLen + sizeof(unsigned int), 0);

    if (uiBytesSent == (unsigned int) -1)
    {
#ifdef WIN32
        if (WSAGetLastError() != WSAEWOULDBLOCK)
        {
            // The socket is no longer valid.
            if (m_pCloseCallback)
            {
                m_pCloseCallback(this);
            }

            return false;
        }
#else
        if (errno != EWOULDBLOCK)
        {
            if (m_pCloseCallback)
            {
                m_pCloseCallback(this);
            }
        }
#endif
    }

    return true;
}
