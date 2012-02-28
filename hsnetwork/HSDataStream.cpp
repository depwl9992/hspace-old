#include "pch.h"

#include "HSDataStream.h"

#if !defined(WIN32)

#if !defined(__FreeBSD__)
#include <netinet/in.h>
#else
#include <sys/param.h>
#endif

#include <string.h>
#endif

CHSDataStream::CHSDataStream():
    m_pStream(NULL), 
    m_uiCurrentSize(0), 
    m_uiCurrentStreamPos(0)
{
    if (ntohl(1) == 1)
    {
        m_bBigEndian = true;
    }
    else
    {
        m_bBigEndian = false;
    }
}

bool CHSDataStream::GrowToSize(unsigned int uiSize)
{
    if (m_uiCurrentSize >= uiSize)
    {
        return true;            // We're already at this size.
    }

    // If we don't have a current stream, just allocate to this size.
    if (!m_pStream)
    {
        m_pStream = new char[uiSize];
        m_uiCurrentSize = uiSize;
        return true;
    }

    // We're already allocated, and we'll have to reallocate!  Damn the world!
    char *pcNewBuffer;

    pcNewBuffer = new char[uiSize];
    if (!pcNewBuffer)
    {
        return false;           // Problems with mem alloc
    }

    // Copy our current buffer into the new one.
    memcpy(pcNewBuffer, m_pStream, m_uiCurrentSize);

    // Delete the old buffer.
    delete[]m_pStream;

    // The new buffer is now our stream.
    m_pStream = pcNewBuffer;
    m_uiCurrentSize = uiSize;

    return true;
}

bool CHSDataStream::WriteToStream(void *pData, unsigned int uiDataLen)
{
    // Are we already at the end of the stream?
    if ((m_uiCurrentStreamPos + uiDataLen) >= m_uiCurrentSize)
    {
        if ((m_uiCurrentSize * HSCONST_DEFAULT_GROW_FACTOR) <
            (m_uiCurrentSize + uiDataLen))
        {
            if (!GrowToSize(m_uiCurrentSize + uiDataLen))
            {
                return false;
            }
        }
        else
        {
            if (!GrowToSize
                ((unsigned int) (m_uiCurrentSize *
                                 HSCONST_DEFAULT_GROW_FACTOR)))
            {
                return false;
            }
        }
    }

    // We should have enough space to write the new data.
    memcpy(&m_pStream[m_uiCurrentStreamPos], pData, uiDataLen);
    m_uiCurrentStreamPos += uiDataLen;

    return true;
}

bool CHSDataStream::ReadFromStream(void *pBuffer, unsigned int uiLen)
{
    // Enough data left to read?
    if ((m_uiCurrentSize - m_uiCurrentStreamPos) < uiLen)
    {
        return false;           // Don't have that much data left to read.
    }

    memcpy(pBuffer, &m_pStream[m_uiCurrentStreamPos], uiLen);
    m_uiCurrentStreamPos += uiLen;

    return true;
}

bool CHSDataStream::WriteString(const char *pcString)
{
    unsigned short usLen = strlen(pcString) + 1;

    if (WriteToStream(usLen))
    {
        return WriteToStream((void *) pcString, usLen);
    }

    return false;
}

bool CHSDataStream::ReadString(char *pcBuffer)
{
    unsigned short usLen = 0;

    if (ReadFromStream(usLen))
    {
        return ReadFromStream((void *) pcBuffer, usLen);
    }

    return false;
}

bool CHSDataStream::ReadString(char **ppcBuffer)
{
    unsigned short usLen = 0;

    if (ReadFromStream(usLen))
    {
        *ppcBuffer = new char[usLen];

        return ReadFromStream(*ppcBuffer, usLen);
    }

    return false;
}
