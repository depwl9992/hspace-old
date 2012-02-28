// -----------------------------------------------------------------------
//!
//! @$Id: hslandingloc.h,v 1.4 2006/04/04 12:41:36 mark Exp $
//!
//! @par Copyright:
//! Copyright (c) 2005 by HSpace Development Team -- see hscopyright.h
//!
//! @brief CHSLandingLoc declaration
//!
// -----------------------------------------------------------------------

#ifndef __HSLANDINGLOC_INCLUDED__
#define __HSLANDINGLOC_INCLUDED__

#include "hsobjects.h"
#include "hstypes.h"
#include <cstdio>
#include <stdlib.h>

class CHS3DObject;

// This generic landing location class can be used for landing
// pads on planets or as landing bays in a ship.

#define HS_LANDING_CODE_LEN  8
class CHSLandingLoc
{
  public:
    // Methods
    CHSLandingLoc(void);
    ~CHSLandingLoc(void);

    void HandleMessage(const HS_INT8 *, HS_INT32);
    void DeductCapacity(HS_INT32);
    void SetActive(HS_BOOL8);
    void SetOwnerObject(CHS3DObject *);
    HS_BOOL8 HandleKey(HS_INT32, HS_INT8 *, FILE *);
    HS_INT8 *GetAttributeValue(HS_INT8 *);
    HS_BOOL8 SetAttributeValue(HS_INT8 *, HS_INT8 *);
    HS_BOOL8 CodeRequired(void);
    HS_BOOL8 LoadFromObject(HS_DBREF);
    HS_BOOL8 IsActive(void) const
    {
        return bActive;
    }

    HS_BOOL8 IsVisible(void) const
    {
        return bVisible;
    }

    HS_BOOL8 CodeClearance(HS_INT8 *);
    HS_BOOL8 CanAccomodate(CHS3DObject *);

    CHS3DObject *GetOwnerObject(void);

    HS_DBREF Object() const
    {
        return m_objnum;
    }
    void Object(HS_DBREF val)
    {
        m_objnum = val;
    }
  protected:
    void ClearObjectAttrs(void);
    void WriteCodeAttr();
    void WriteActiveAttr();
    void WriteVisibleAttr();
    void WriteMaxCapacityAttr();
    void WriteCapacityAttr();

    // Attributes
    HS_DBREF m_objnum;

    CHS3DObject *m_ownerObj;

    HS_INT32 m_capacity;
    HS_INT32 m_max_capacity;    // -1 for unlimited
    HS_INT8 strCode[9];
    HS_BOOL8 bActive;
    HS_BOOL8 bVisible;
};


#endif // __HSLANDINGLOC_INCLUDED__
