// -----------------------------------------------------------------------
//  $Id: hsfictional.h,v 1.4 2006/12/13 15:47:59 mark Exp $
// -----------------------------------------------------------------------




#ifndef __HSFICTIONAL_INCLUDED__
#define __HSFICTIONAL_INCLUDED__

#include "hscopyright.h"
#include "hseng.h"

// Copyright 2006

// Fictional System, Generic System with no hardcoded purpose.
class CHSFictional:public CHSEngSystem
{
  public:
    CHSFictional();
    virtual ~CHSFictional()
    {
    }

    void DoCycle();
    void SaveToFile(FILE * fp);

    HS_BOOL8 SetAttributeValue(const HS_INT8 * pcAttrName,
                               const HS_INT8 * pcValue);

    void GetAttributeList(CHSAttributeList & rlistAttrs);

    HS_BOOL8 GetAttributeValue(const HS_INT8 * pcAttrName,
                               CHSVariant & rvarReturnVal,
                               HS_BOOL8 bAdjusted,
                               HS_BOOL8 bLocalOnly = false);


  protected:
    HS_FLOAT32 m_fChargeLevel;
    HS_FLOAT32 m_fChargeRate;

};


#endif // __HSFICTIONAL_INCLUDED__
