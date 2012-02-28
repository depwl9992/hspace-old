// -----------------------------------------------------------------------
// $Id: HSVariant.h,v 1.6 2006/04/04 12:39:49 mark Exp $
// -----------------------------------------------------------------------

#if !defined(__HSVARIANT_H__)
#define __HSVARIANT_H__

#include "HSDataStream.h"
#include "hstypes.h"

//! The variant class process multiple data types based on an incoming 
//! data value
class CHSVariant
{
  public:
    //! Type declarations for the type value stored in the variant class
    enum EHSVariantType
    {
        VT_INVALID = 0,
        VT_UINT8,
        VT_INT8,
        VT_UINT16,
        VT_INT16,
        VT_UINT32,
        VT_INT32,
        VT_FLOAT,
        VT_DOUBLE,
        VT_STRING,
        VT_BOOL
    };

        CHSVariant():m_eType(VT_INVALID)
    {
        m_uData.dData = 0;
    }

    CHSVariant(const CHSVariant & rVariant)
    {
        *this = rVariant;
    }

    ~CHSVariant()
    {
        FreePreviousMemory();
    }

    void operator=(const CHSVariant & rtVariant)
    {
        FreePreviousMemory();

        if (rtVariant.m_eType == VT_STRING)
        {
            *this = (const char *) (rtVariant.m_uData.pcData);
        }
        else
        {
            m_eType = rtVariant.m_eType;
            m_uData = rtVariant.m_uData;
        }
    }

    void operator=(const char *pcData)
    {
        if (pcData)
        {
            FreePreviousMemory();

            m_eType = VT_STRING;
            m_uData.pcData = new char[strlen(pcData) + 1];
            memcpy(m_uData.pcData, pcData, strlen(pcData) + 1);
        }
    }

    void operator=(bool bData)
    {
        FreePreviousMemory();

        m_eType = VT_BOOL;
        m_uData.cData = bData;
    }

    void operator=(unsigned char cData)
    {
        FreePreviousMemory();

        m_eType = VT_UINT8;
        m_uData.cData = cData;
    }

    void operator=(char cData)
    {
        FreePreviousMemory();

        m_eType = VT_INT8;
        m_uData.cData = cData;
    }

    void operator=(unsigned short usData)
    {
        FreePreviousMemory();

        m_eType = VT_UINT16;
        m_uData.usData = usData;
    }

    void operator=(short sData)
    {
        FreePreviousMemory();

        m_eType = VT_INT16;
        m_uData.usData = sData;
    }

    void operator=(unsigned int uiData)
    {
        FreePreviousMemory();

        m_eType = VT_UINT32;
        m_uData.uiData = uiData;
    }

    void operator=(int iData)
    {
        FreePreviousMemory();

        m_eType = VT_INT32;
        m_uData.uiData = iData;
    }

    void operator=(float fData)
    {
        FreePreviousMemory();

        m_eType = VT_FLOAT;
        m_uData.fData = fData;
    }

    void operator=(double dData)
    {
        FreePreviousMemory();

        m_eType = VT_DOUBLE;
        m_uData.dData = dData;
    }

    EHSVariantType GetType() const
    {
        return m_eType;
    }

    unsigned char GetUChar() const
    {
        return m_uData.cData;
    }

    bool GetBool() const
    {
        return m_uData.cData == 1 ? true : false;
    }

    const char *GetString() const
    {
        return m_uData.pcData;
    }

    char GetChar() const
    {
        return m_uData.cData;
    }

    unsigned short GetUShort() const
    {
        return m_uData.usData;
    }

    short GetShort() const
    {
        return m_uData.usData;
    }

    unsigned int GetUInt() const
    {
        return m_uData.uiData;
    }

    int GetInt() const
    {
        return m_uData.uiData;
    }

    float GetFloat() const
    {
        return m_uData.fData;
    }

    double GetDouble() const
    {
        return m_uData.dData;
    }

    bool WriteToStream(CHSDataStream & rStream)
    {
        unsigned char cType = (unsigned char) m_eType;
        rStream.WriteToStream(cType);

        switch (m_eType)
        {
        case VT_UINT8:
        case VT_INT8:
        case VT_BOOL:
            rStream.WriteToStream(m_uData.cData);
            break;
        case VT_UINT16:
        case VT_INT16:
            rStream.WriteToStream(m_uData.usData);
            break;
        case VT_UINT32:
        case VT_INT32:
            rStream.WriteToStream(m_uData.uiData);
            break;
        case VT_FLOAT:
            rStream.WriteToStream(m_uData.fData);
            break;
        case VT_DOUBLE:
            rStream.WriteToStream(m_uData.dData);
            break;
        case VT_STRING:
            rStream.WriteString(m_uData.pcData);
            break;
            // warning quashing only
        case VT_INVALID:
            break;
        }

        return true;
    }

    bool ReadFromStream(CHSDataStream & rStream)
    {
        // Read type.
        unsigned char cType;
        rStream.ReadFromStream(cType);

        m_eType = (EHSVariantType) cType;
        switch (m_eType)
        {
        case VT_UINT8:
        case VT_INT8:
        case VT_BOOL:
            rStream.ReadFromStream(m_uData.cData);
            break;
        case VT_UINT16:
        case VT_INT16:
            rStream.ReadFromStream(m_uData.usData);
            break;
        case VT_UINT32:
        case VT_INT32:
            rStream.ReadFromStream(m_uData.uiData);
            break;
        case VT_FLOAT:
            rStream.ReadFromStream(m_uData.fData);
            break;
        case VT_DOUBLE:
            rStream.ReadFromStream(m_uData.dData);
            break;
        case VT_STRING:
            rStream.ReadString(&m_uData.pcData);
            break;
        case VT_INVALID:       // warning quashing
            break;
        }

        return true;
    }

  protected:

    union
    {
        unsigned char cData;
        unsigned short usData;
        unsigned int uiData;
        float fData;
        double dData;
        char *pcData;
    } m_uData;

    EHSVariantType m_eType;


    void FreePreviousMemory()
    {
        if (m_eType == VT_STRING)
        {
            delete[]m_uData.pcData;
            m_uData.pcData = NULL;
        }
    }

};
#endif // __HSVARIANT_H__
