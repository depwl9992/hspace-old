// -----------------------------------------------------------------------
//!
//! @$Id: hshatch.h,v 1.9 2006/04/25 16:52:01 mark Exp $
//!
//! @par Copyright:
//! Copyright (c) 2005 by HSpace Development Team -- see hscopyright.h
//!
//! @brief  Hatch/Airlock class declaration
//!
// -----------------------------------------------------------------------

#ifndef HSHATCH_H__
#define HSHATCH_H__

#include "pch.h"
#include <cstring>
#include <stdlib.h>

#include "hscopyright.h"
#include "hsobjects.h"
#include "hsinterface.h"
#include "hsutils.h"
#include "hspace.h"
#include "hsflags.h"

class CHS3DObject;


//!  Hatch (airlock) class to allow ship-to-ship connections
class CHSHatch
{
  public:
    //! Primary constructor: initializes hatch members to safe values
    CHSHatch();

    //! Clears values/attributes off game object
       ~CHSHatch();

    //! Writes necessary data attributes to the game object for storage
    void InitObjectAttrs();

    //! Remove attributes written to game object used as interface to hatch
    void ClearObjectAttrs();

    //! Passes message to the contents of the room containing the hatch
    void HandleMessage(const HS_INT8 *, HS_INT32);

    //! Sets the CHS3DObject that owns the hatch
    void SetOwnerObject(CHS3DObject *);

    //! Clears the owner object
    void ClearOwnerObject();

    //! Returns value of the specified attribute as set on the hatch
    HS_INT8 *GetAttributeValue(const HS_INT8 *);

    //! Set atrName attribute to atrValue
    HS_BOOL8 SetAttributeValue(const HS_INT8 * atrName, HS_INT8 * atrValue);

    //! Loads hatch values from the in game object specified
    HS_BOOL8 LoadFromObject(HS_DBREF);

    //! Returns a pointer to the CHS3DObject that owns the hatch
    CHS3DObject *GetOwnerObject();

    //! Get the game object dbref that represents the hatch
    HS_DBREF Object() const
    {
        return mObjectDbref;
    }

    //! Set the game object dbref that represents the hatch
    void Object(HS_DBREF val)
    {
        mObjectDbref = val;
    }

    //! Get the dbref off the target object
    HS_DBREF TargetObject() const
    {
        return mTargetDbref;
    }

    //! Set the target object dbref; write data on game object
    void TargetObject(HS_DBREF val);

    //! Get Target Hatch ID
    HS_INT32 TargetHatch() const
    {
        return mTargetHatchID;
    }

    //! Set Target Hatch ID, write data on game object
    void TargetHatch(HS_INT32 val);

    // Is target clamped?
    HS_INT32 Clamped() const
    {
        return mClamped;
    }

    // Set clamped status; write data to game object 
    void Clamped(HS_INT32 val);

  protected:
    //! Write the Target Object DBref storage attribute
    void WriteTargetDbrefAttribute();

    //! Write Target Hatch Identification Attribute
    void WriteTargetIdAttribute();

    //! Write Clamped Status Attribute
    void WriteClampedStatusAttribute();

    //! DBREF of game exit object that represents the hatch
    HS_DBREF mObjectDbref;

    //! Target Object DBREF
    HS_DBREF mTargetDbref;

    //! Target Hatch Identification Number
    HS_INT32 mTargetHatchID;

    //! Clamped flag to indicate if connected (1) or not (0)
    HS_INT32 mClamped;

    //! Space object that owns and therefore contains this hatch
    CHS3DObject *mOwnerObject;
};

#endif
