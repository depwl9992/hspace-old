// -----------------------------------------------------------------------
// $Id: hstachyon.h,v 1.3 2006/04/04 12:41:36 mark Exp $
// -----------------------------------------------------------------------
#ifdef _WIN32
#pragma once
#endif

#if !defined(__HSTACHYON_H__)
#define __HSTACHYON_H__

#include "hseng.h"


//! Tachyon Sensors, Generic system with some enhancements.
class CHSSysTach:public CHSEngSystem
{
  public:
    //! Set system type and initialize
    CHSSysTach(void);

    //! Cleanup allocated memory if necessary
       ~CHSSysTach(void);

    //! Save system information the specified file
    void SaveToFile(FILE * fp);

    //! Attempt to set the specified attribute to pcValue
    HS_BOOL8 SetAttributeValue(const HS_INT8 * pcAttrName,
                               const HS_INT8 * pcValue);

    //! Return a current list of manipulateable values for this system
    void GetAttributeList(CHSAttributeList & rlistAttrs);

    //! Return a current attribute value
    HS_BOOL8 GetAttributeValue(const HS_INT8 * pcAttrName,
                               CHSVariant & rvarReturnVal,
                               HS_BOOL8 bAdjusted,
                               HS_BOOL8 bLocalOnly = false);

    //! Return the current efficiency value either from parent or local
    float GetEfficiency(HS_BOOL8 bAdjusted);

    //! Set the current efficiency value from 0-100
    void SetEfficiency(HS_FLOAT32 fValue)
    {
        if (!m_pfEfficiency)
        {
            m_pfEfficiency = new HS_FLOAT32;
        }

           *m_pfEfficiency = fValue;
    }

    //! Return a pointer to the raw efficiency variable
    HS_FLOAT32 *GetRawEfficiency()
    {
        return m_pfEfficiency;
    }

  protected:

    //! Efficiency of the Tachyon Sensor Array (0 - 100)
    HS_FLOAT32 * m_pfEfficiency;
};

#endif // __HSTACHYON_H__
