// -----------------------------------------------------------------------
// $Id: hscommunications.h,v 1.3 2006/04/04 12:41:36 mark Exp $
// -----------------------------------------------------------------------
#if !defined(__HSCOMMUNICATIONS_H__)
#define __HSCOMMUNICATIONS_H__

#include "hseng.h"


//! The communications system simply indicates if the
//! parent object can receive and transmit messages
class CHSSysComm:public CHSEngSystem
{
  public:
    CHSSysComm(void);
       ~CHSSysComm(void);

    HS_BOOL8 SetAttributeValue(const HS_INT8 * pcAttrName,
                               const HS_INT8 * pcValue);

    void GetAttributeList(CHSAttributeList & rlistAttrs);

    HS_BOOL8 GetAttributeValue(const HS_INT8 * pcAttrName,
                               CHSVariant & rvarReturnVal,
                               HS_BOOL8 bAdjusted,
                               HS_BOOL8 bLocalOnly = false);

    HS_UINT32 GetMaxRange(HS_BOOL8 bAdjusted = true);
    void SaveToFile(FILE *);

    void SetMaxRange(unsigned int uiValue)
    {
        if (!m_puiCommRange)
        {
            m_puiCommRange = new HS_UINT32;
        }
           *m_puiCommRange = uiValue;
    }

    unsigned int *GetRawCommRange()
    {
        return m_puiCommRange;
    }

  protected:

    HS_UINT32 * m_puiCommRange;
};

#endif // __HSCOMMUNICATIONS_H__
