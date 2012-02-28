#if !defined(__HSDATASTREAM_H__)
#define __HSDATASTREAM_H__

const unsigned int HSCONST_DEFAULT_STREAM_SIZE = 1024;
const float HSCONST_DEFAULT_GROW_FACTOR = 1.5f; // Grow 150% each realloc

// Used for packing data types
class CHSDataStream
{
  public:

  protected:

    char *m_pStream;
    unsigned int m_uiCurrentSize;
    unsigned int m_uiCurrentStreamPos;

    bool m_bBigEndian;

  public:

        CHSDataStream();

       ~CHSDataStream()
    {
        if (m_pStream)
        {
            delete[]m_pStream;
        }
    }

    unsigned int GetStreamRemaining() const
    {
        return m_uiCurrentStreamPos - m_uiCurrentSize;
    }

    bool WriteToStream(void *pData, unsigned int uiDataLen);

    bool WriteString(const char *pcString);

    bool WriteToStream(unsigned int &rValue)
    {
        return WriteToStream((void *) &rValue, sizeof(unsigned int));
    }
    bool WriteToStream(char &rValue)
    {
        return WriteToStream((void *) &rValue, sizeof(char));
    }
    bool WriteToStream(unsigned char &rValue)
    {
        return WriteToStream((void *) &rValue, sizeof(unsigned char));
    }
    bool WriteToStream(int &rValue)
    {
        return WriteToStream((void *) &rValue, sizeof(int));
    }
    bool WriteToStream(bool & rValue)
    {
        return WriteToStream((void *) &rValue, sizeof(bool));
    }
    bool WriteToStream(unsigned short &rValue)
    {
        return WriteToStream((void *) &rValue, sizeof(unsigned short));
    }
    bool WriteToStream(short &rValue)
    {
        return WriteToStream((void *) &rValue, sizeof(short));
    }
    bool WriteToStream(float &rValue)
    {
        return WriteToStream((void *) &rValue, sizeof(float));
    }
    bool WriteToStream(double &rValue)
    {
        return WriteToStream((void *) &rValue, sizeof(double));
    }

    bool ReadFromStream(void *pBuffer, unsigned int uiLen);

    // Use this to read into an already allocated buffer.
    bool ReadString(char *pcBuffer);

    // use this to read into an unallocated buffer.
    bool ReadString(char **ppcBuffer);

    bool ReadFromStream(char &rValue)
    {
        return ReadFromStream((void *) &rValue, sizeof(char));
    }
    bool ReadFromStream(short &rValue)
    {
        return ReadFromStream((void *) &rValue, sizeof(short));
    }
    bool ReadFromStream(unsigned short &rValue)
    {
        return ReadFromStream((void *) &rValue, sizeof(unsigned short));
    }
    bool ReadFromStream(int &rValue)
    {
        return ReadFromStream((void *) &rValue, sizeof(int));
    }
    bool ReadFromStream(unsigned int &rValue)
    {
        return ReadFromStream((void *) &rValue, sizeof(unsigned int));
    }
    bool ReadFromStream(float &rValue)
    {
        return ReadFromStream((void *) &rValue, sizeof(float));
    }
    bool ReadFromStream(double &rValue)
    {
        return ReadFromStream((void *) &rValue, sizeof(double));
    }
    bool ReadFromStream(unsigned char &rValue)
    {
        return ReadFromStream((void *) &rValue, sizeof(unsigned char));
    }

    bool ReadFromStream(bool & rValue)
    {
        return ReadFromStream((void *) &rValue, sizeof(bool));
    }

    bool IsBigEndianSystem() const
    {
        return m_bBigEndian;
    }

    unsigned int GetCurrentPosition()
    {
        return m_uiCurrentStreamPos;
    }

    char *GetBuffer()
    {
        return m_pStream;
    }

    bool SeekTo(unsigned int uiPos)
    {
        if (!m_pStream)
        {
            if (!GrowToSize(HSCONST_DEFAULT_STREAM_SIZE))
            {
                return false;   // Problem growing. ?
            }
        }

        if ((uiPos > m_uiCurrentSize) || (uiPos < 0))
        {
            return false;
        }

        m_uiCurrentStreamPos = uiPos;
        return true;
    }

    bool SeekToBegin()
    {
        return SeekTo(0);
    }

    bool SeekToEnd()
    {
        return SeekTo(m_uiCurrentSize);
    }

  protected:

    bool GrowToSize(unsigned int uiSize);
};

#endif // __HSDATASTREAM_H__
