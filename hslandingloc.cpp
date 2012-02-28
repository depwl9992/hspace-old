// -----------------------------------------------------------------------
//!
//! @$Id: hslandingloc.cpp,v 1.4 2006/04/04 12:41:11 mark Exp $
//!
//! @par Copyright:
//! Copyright (c) 2005 by HSpace Development Team -- see hscopyright.h
//!
// -----------------------------------------------------------------------


#include "hslandingloc.h"

//
// CHSLandingLoc stuff
//
CHSLandingLoc::CHSLandingLoc()
{
    m_objnum = -1;
    *strCode = '\0';
    bActive = true;
    m_max_capacity = -1;        // Unlimited
    m_capacity = 0;
    m_ownerObj = NULL;
    bVisible = true;
}

CHSLandingLoc::~CHSLandingLoc()
{
    // Clear attributes and flags
    //hsInterface.UnsetToggle(m_objnum, ROOM_HSPACE_LANDINGLOC);
    //ClearObjectAttrs();
}

HS_BOOL8 CHSLandingLoc::LoadFromObject(HS_DBREF obj)
{
    if (hsInterface.ValidObject(obj) == false)
    {
        return false;
    }

    m_objnum = obj;

    if (hsInterface.AtrGet(obj, "HSDB_CODE"))
    {
        strcpy(strCode, hsInterface.m_buffer);
    }

    if (hsInterface.AtrGet(obj, "HSDB_ACTIVE"))
    {
        bActive = hsInterface.m_buffer[0] == '1' ? true : false;
    }

    if (hsInterface.AtrGet(obj, "HSDB_VISIBLE"))
    {
        bVisible = hsInterface.m_buffer[0] == '1' ? true : false;
    }

    if (hsInterface.AtrGet(obj, "HSDB_MAX_CAPACITY"))
    {
        m_max_capacity = atoi(hsInterface.m_buffer);
    }

    if (hsInterface.AtrGet(obj, "HSDB_CAPACITY"))
    {
        m_capacity = atoi(hsInterface.m_buffer);
    }

    // Set some default info on the room
    hsInterface.SetToggle(obj, ROOM_HSPACE_LANDINGLOC);

    return true;
}

void CHSLandingLoc::ClearObjectAttrs()
{
    hsInterface.AtrAdd(m_objnum, "HSDB_CODE", NULL,
                       hsInterface.GetGodDbref());
    hsInterface.AtrAdd(m_objnum, "HSDB_ACTIVE", NULL,
                       hsInterface.GetGodDbref());
    hsInterface.AtrAdd(m_objnum, "HSDB_VISIBLE", NULL,
                       hsInterface.GetGodDbref());
    hsInterface.AtrAdd(m_objnum, "HSDB_CAPACITY", NULL,
                       hsInterface.GetGodDbref());
    hsInterface.AtrAdd(m_objnum, "HSDB_MAX_CAPACITY", NULL,
                       hsInterface.GetGodDbref());
}

// Returns a character string containing the value of the
// requested attribute.
char *CHSLandingLoc::GetAttributeValue(char *strName)
{
    static char rval[32];

    *rval = '\0';
    if (!strcasecmp(strName, "CODE"))
        strcpy(rval, strCode);
    else if (!strcasecmp(strName, "ACTIVE"))
        sprintf(rval, "%d", bActive ? 1 : 0);
    else if (!strcasecmp(strName, "VISIBLE"))
        sprintf(rval, "%d", bVisible ? 1 : 0);
    else if (!strcasecmp(strName, "CAPACITY"))
        sprintf(rval, "%d", m_capacity);
    else if (!strcasecmp(strName, "MAX CAPACITY"))
        sprintf(rval, "%d", m_max_capacity);
    else
        return NULL;

    return rval;
}

// Set a value for a given attribute on the landing location
HS_BOOL8 CHSLandingLoc::SetAttributeValue(char *strName, char *strValue)
{
    int iVal;

    // Match the name .. set the value
    if (!strcasecmp(strName, "CODE"))
    {
        if (strlen(strValue) > 8)
            return false;
        strcpy(strCode, strValue);
        WriteCodeAttr();
        return true;
    }
    else if (!strcasecmp(strName, "ACTIVE"))
    {
        iVal = atoi(strValue);
        if (iVal == 0)
            bActive = false;
        else
            bActive = true;
        WriteActiveAttr();
        return true;
    }
    else if (!strcasecmp(strName, "CAPACITY"))
    {
        iVal = atoi(strValue);
        m_capacity = iVal;
        WriteCapacityAttr();
        return true;
    }
    else if (!strcasecmp(strName, "MAX CAPACITY"))
    {
        iVal = atoi(strValue);
        m_max_capacity = iVal;
        WriteMaxCapacityAttr();
        return true;
    }
    else if (!strcasecmp(strName, "VISIBLE"))
    {
        iVal = atoi(strValue);
        if (iVal == 0)
            bVisible = false;
        else
            bVisible = true;
        WriteVisibleAttr();
        return true;
    }
    else
        return false;
}

// Indicates whether the supplied code matches the currently
// set code.  If no code is set, this always returns true.
HS_BOOL8 CHSLandingLoc::CodeClearance(char *lpstrCode)
{
    if (!*strCode)
        return true;

    if (!strcasecmp(strCode, lpstrCode))
        return true;

    return false;
}

// Handles a message, which for now is just giving it to the room
void CHSLandingLoc::HandleMessage(const char *lpstrMsg, int type)
{
    hsInterface.NotifyContents(m_objnum, (char *) lpstrMsg);
}

// Indicates whether the landing location can accomodate
// the given object based on size.
HS_BOOL8 CHSLandingLoc::CanAccomodate(CHS3DObject * cObj)
{
    // Unlimited capacity?
    if (m_max_capacity < 0)
        return true;

    // Compare current capacity and object size
    if (m_capacity < (int) cObj->GetSize())
        return false;

    return true;
}

void CHSLandingLoc::DeductCapacity(int change)
{
    // If unlimited, do nothing.
    if (m_max_capacity < 0)
        return;

    m_capacity -= change;
    WriteCapacityAttr();
}

// Sets the active status of the landing location
void CHSLandingLoc::SetActive(HS_BOOL8 bStat)
{
    bActive = bStat;
    WriteActiveAttr();
}

// Sets the CHS3DObject that owns the landing location.
void CHSLandingLoc::SetOwnerObject(CHS3DObject * cObj)
{
    m_ownerObj = cObj;
}

// Returns the CHS3DObject that controls the landing location.
CHS3DObject *CHSLandingLoc::GetOwnerObject()
{
    return m_ownerObj;
}

// Returns true or false to indicate if a code is required.
HS_BOOL8 CHSLandingLoc::CodeRequired()
{
    if (*strCode)
        return true;
    else
        return false;
}

void CHSLandingLoc::WriteCodeAttr()
{
    hsInterface.AtrAdd(m_objnum, "HSDB_CODE", strCode,
                       hsInterface.GetGodDbref());
}

void CHSLandingLoc::WriteActiveAttr()
{
    hsInterface.AtrAdd(m_objnum, "HSDB_ACTIVE",
                       hsInterface.HSPrintf("%s", bActive ? "1" : "0"),
                       hsInterface.GetGodDbref());
}

void CHSLandingLoc::WriteVisibleAttr()
{
    hsInterface.AtrAdd(m_objnum, "HSDB_VISIBLE",
                       hsInterface.HSPrintf("%s", bVisible ? "1" : "0"),
                       hsInterface.GetGodDbref());
}

void CHSLandingLoc::WriteMaxCapacityAttr()
{
    hsInterface.AtrAdd(m_objnum, "HSDB_MAX_CAPACITY",
                       hsInterface.HSPrintf("%d", m_max_capacity),
                       hsInterface.GetGodDbref());
}

void CHSLandingLoc::WriteCapacityAttr()
{
    hsInterface.AtrAdd(m_objnum, "HSDB_CAPACITY",
                       hsInterface.HSPrintf("%d", m_capacity),
                       hsInterface.GetGodDbref());
}
