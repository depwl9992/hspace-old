// -----------------------------------------------------------------------
//!
//! @$Id: hshatch.cpp,v 1.12 2006/04/25 16:52:01 mark Exp $
//!
//! @par Copyright (see hscopyright.h)
//! Copyright (c) 2005 by HSpace Development Team 
//!
// -----------------------------------------------------------------------

#include "hscopyright.h"
#include "hshatch.h"


CHSHatch::CHSHatch() :
    mObjectDbref(-1), 
    mTargetDbref(HSNOTHING), 
    mTargetHatchID(0), 
    mClamped(0), 
    mOwnerObject(NULL)
{
}

CHSHatch::~CHSHatch()
{
}

HS_BOOL8 CHSHatch::LoadFromObject(HS_DBREF obj)
{
    if (hsInterface.ValidObject(obj) == false)
    {
        hs_log(
            hsInterface.HSPrintf(
               "ERROR: CHSHatch::LoadFromObject() called on invalid dbref #%d",
               mObjectDbref));
        return false;
    }

    mObjectDbref = obj;

    hsInterface.SetToggle(mObjectDbref, EXIT_HSPACE_HATCH);

    if (hsInterface.AtrGet(mObjectDbref, "HSDB_TARGETOBJ"))
        mTargetDbref = strtodbref(hsInterface.m_buffer);

    if (hsInterface.AtrGet(mObjectDbref, "HSDB_TARGETHATCH"))
        mTargetHatchID = atoi(hsInterface.m_buffer);

    if (hsInterface.AtrGet(mObjectDbref, "HSDB_CLAMPED"))
        mClamped = atoi(hsInterface.m_buffer);

    return true;
}

void CHSHatch::WriteTargetDbrefAttribute()
{
    hsInterface.AtrAdd(mObjectDbref, "HSDB_TARGETOBJ",
                       hsInterface.HSPrintf("#%d", mTargetDbref),
                       hsInterface.GetGodDbref(), AF_MDARK | AF_WIZARD);
}

void CHSHatch::WriteTargetIdAttribute()
{
    hsInterface.AtrAdd(mObjectDbref, "HSDB_TARGETHATCH",
                       hsInterface.HSPrintf("%d", mTargetHatchID),
                       hsInterface.GetGodDbref(), AF_MDARK | AF_WIZARD);
}

void CHSHatch::WriteClampedStatusAttribute()
{
    hsInterface.AtrAdd(mObjectDbref, "HSDB_CLAMPED",
                       hsInterface.HSPrintf("%d", mClamped),
                       hsInterface.GetGodDbref(), AF_MDARK | AF_WIZARD);
}

void CHSHatch::ClearObjectAttrs()
{
    hsInterface.AtrAdd(mObjectDbref, "HSDB_TARGETOBJ",
                       NULL, hsInterface.GetGodDbref());
    hsInterface.AtrAdd(mObjectDbref, "HSDB_TARGETHATCH",
                       NULL, hsInterface.GetGodDbref());
    hsInterface.AtrAdd(mObjectDbref, "HSDB_CLAMPED",
                       NULL, hsInterface.GetGodDbref());
    hsInterface.AtrAdd(mObjectDbref, "HSDB_SHIP",
                       NULL, hsInterface.GetGodDbref());
}

void CHSHatch::InitObjectAttrs()
{
    WriteTargetDbrefAttribute();
    WriteTargetIdAttribute();
    WriteClampedStatusAttribute();
}

// All not used yet.
HS_INT8 *CHSHatch::GetAttributeValue(const HS_INT8 * strName)
{
    static char rval[32];

    *rval = '\0';
    if (!strcasecmp(strName, "TARGETOBJ"))
        sprintf(rval, "#%d", mTargetDbref);
    else if (!strcasecmp(strName, "TARGETHATCH"))
        sprintf(rval, "%d", mTargetHatchID);
    else if (!strcasecmp(strName, "CLAMPED"))
        sprintf(rval, "%d", mClamped);
    else
        return NULL;

    return rval;
}

// Unused currently, might be used in the future.
HS_BOOL8 CHSHatch::SetAttributeValue(const HS_INT8 * strName,
                                     HS_INT8 * strValue)
{

    // Match the name .. set the value
    if (!strcasecmp(strName, "TARGETHATCH"))
    {
        mTargetHatchID = atoi(strValue);
        WriteTargetIdAttribute();
        return true;
    }
    if (!strcasecmp(strName, "CLAMPED"))
    {
        if (atoi(strValue) == 1 || atoi(strValue) == 0)
        {
            mClamped = atoi(strValue);
            WriteClampedStatusAttribute();
            return true;
        }
        else
            return false;
    }
    else
        return false;
}

// Handles a message, which for now is just giving it to the room the
// hatch is situated in.
void CHSHatch::HandleMessage(const char *lpstrMsg, int type)
{
    hsInterface.NotifyContents(hsInterface.GetHome(mObjectDbref),
                               (char *) lpstrMsg);
}

// Sets the CHS3DObject that owns the landing location.
void CHSHatch::SetOwnerObject(CHS3DObject * cObj)
{
    mOwnerObject = cObj;
}

// Remove the owner reference
void CHSHatch::ClearOwnerObject()
{
    mOwnerObject = NULL;
}

// Returns the CHS3DObject that controls the landing location.
CHS3DObject *CHSHatch::GetOwnerObject()
{
    return mOwnerObject;
}


void CHSHatch::TargetObject(HS_DBREF val)
{
    mTargetDbref = val;
    WriteTargetDbrefAttribute();
}

void CHSHatch::TargetHatch(HS_INT32 val)
{
    mTargetHatchID = val;
    WriteTargetIdAttribute();
}

void CHSHatch::Clamped(HS_INT32 val)
{
    mClamped = val;
    WriteClampedStatusAttribute();
}
