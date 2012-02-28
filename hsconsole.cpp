// -----------------------------------------------------------------------
//!
//! @$Id: hsconsole.cpp,v 1.30 2006/06/17 02:36:40 mark Exp $
//!
//! @par Copyright:
//! Copyright (c) 2005 by HSpace Development Team -- see hscopyright.h
//!
// -----------------------------------------------------------------------

#include "pch.h"

#include <stdlib.h>
#include <math.h>
#include <cstring>

#include "hscopyright.h"
#include "hspace.h"
#include "hsobjects.h"
#include "hsship.h"
#include "hsutils.h"
#include "hsinterface.h"
#include "hsansi.h"
#include "hsflags.h"
#include "hsmissile.h"

// Extern trig stuff
extern double d2cos_table[];
extern double d2sin_table[];

//
// CHSConsole Stuff
//
CHSConsole::CHSConsole()
{
    HS_INT32 idx;

    m_weapon_array = NULL;
    for (idx = 0; idx < NUM_MESSAGE_TYPES; idx++)
        m_msgtypes[idx] = HSNOTHING;

    m_xyheading = 0;
    m_zheading = 0;
    m_xyoffset = 0;             // Front of ship by default
    m_zoffset = 0;
    m_arc = 30;                 // Default of 30 degree arc

    m_objnum = HSNOTHING;
    m_ownerObj = NULL;
    m_missile_bay = NULL;
    m_target_lock = NULL;
    m_can_rotate = false;
    m_online = false;
    m_autoload = false;
    m_targetting = HSS_NOTYPE;
}

CHSConsole::~CHSConsole()
{
}

// Indicates if the console is powered up.  If there is
// no computer, the console requires no power.
HS_BOOL8 CHSConsole::IsOnline()
{
    if (NULL == GetComputer())
    {
        return true;
    }

    return m_online;
}

// Gets the computer source for the console. This deprecates
// the below SetComputer function.
CHSSysComputer *CHSConsole::GetComputer(void)
{
    CHSShip *cShip = (CHSShip *) m_ownerObj;
    if (!cShip)
    {
        return NULL;
    }

    return (CHSSysComputer *) cShip->GetEngSystem(HSS_COMPUTER);
}

// Takes a string of space delimited weapon IDs and loads
// them into the weapons array.
void CHSConsole::LoadWeapons(HS_INT8 * strWeapons)
{
    HS_INT8 tbuf[32];
    HS_INT32 idx;
    HS_INT32 iVal;

    // Do we have a weapons array?
    if (NULL == m_weapon_array)
    {
        m_weapon_array = new CHSWeaponArray;
    }

    // Extract the IDs from the list, and load
    // them into the array.
    idx = 0;
    extract(strWeapons, tbuf, 0, 1, ' ');
    while (*tbuf)
    {
        iVal = atoi(tbuf);

        CHSWeaponData *pData = waWeapons.GetWeapon(iVal);
        if (pData)
        {
            CHSWeapon *pWeapon =
                CHSWeapon::CreateWeapon(pData->WeaponClass());
            if (pWeapon)
            {
                pWeapon->SetData(pData);
                m_weapon_array->AddWeapon(pWeapon);
            }
        }

        idx++;
        extract(strWeapons, tbuf, idx, 1, ' ');
    }
}

// Clears the attributes from the object
void CHSConsole::ClearObjectAttrs()
{
    if (m_objnum == HSNOTHING)
        return;

    // Set the attributes on the console object.
    hsInterface.AtrAdd(m_objnum, "HSDB_XYHEADING", NULL,
                       hsInterface.GetGodDbref());
    hsInterface.AtrAdd(m_objnum, "HSDB_ZHEADING", NULL,
                       hsInterface.GetGodDbref());
    hsInterface.AtrAdd(m_objnum, "HSDB_XYOFFSET", NULL,
                       hsInterface.GetGodDbref());
    hsInterface.AtrAdd(m_objnum, "HSDB_ZOFFSET", NULL,
                       hsInterface.GetGodDbref());
    hsInterface.AtrAdd(m_objnum, "HSDB_FIRING_ARC", NULL,
                       hsInterface.GetGodDbref());

    HS_INT32 idx;
    for (idx = 0; idx < NUM_MESSAGE_TYPES; idx++)
    {
        HS_INT8 tbuf[32];

        sprintf_s(tbuf, "HSDB_MSGTYPE_%d", idx);
        hsInterface.AtrAdd(m_objnum, tbuf, NULL, hsInterface.GetGodDbref());
    }

}

// Gets the owner of the console .. the HS_DBREF of the owner
HS_DBREF CHSConsole::GetOwner()
{
    return m_owner;
}

// Sets the owner of the console to a specific object in the game
void CHSConsole::SetOwner(HS_DBREF obj)
{
    m_owner = obj;

    // Set my owner attribute
    if (m_objnum != HSNOTHING)
        hsInterface.AtrAdd(m_objnum, "HSDB_OWNER",
                           hsInterface.HSPrintf("#%d", obj),
                           hsInterface.GetGodDbref());
}

// Loads information from a game object into the 
// console object in memory.
HS_BOOL8 CHSConsole::LoadFromObject(HS_DBREF objnum)
{
    if (hsInterface.ValidObject(objnum) == false)
        return false;

    // Set our object number
    m_objnum = objnum;

    // Load attributes
    if (hsInterface.AtrGet(objnum, "HSDB_XYHEADING"))
        m_xyheading = atoi(hsInterface.m_buffer);
    if (hsInterface.AtrGet(objnum, "HSDB_FIRING_ARC"))
        m_arc = atoi(hsInterface.m_buffer);
    if (hsInterface.AtrGet(objnum, "HSDB_ZHEADING"))
        m_zheading = atoi(hsInterface.m_buffer);
    if (hsInterface.AtrGet(objnum, "HSDB_XYOFFSET"))
        m_xyoffset = atoi(hsInterface.m_buffer);
    if (hsInterface.AtrGet(objnum, "HSDB_ZOFFSET"))
        m_zoffset = atoi(hsInterface.m_buffer);
    if (hsInterface.AtrGet(objnum, "HSDB_CAN_ROTATE"))
        m_can_rotate = atoi(hsInterface.m_buffer) == 0 ? false : true;


    // Any weapons to load?
    if (hsInterface.AtrGet(objnum, "HSDB_WEAPONS"))
    {
        LoadWeapons(hsInterface.m_buffer);
    }

    // Load weapon stats
    HS_INT32 idx;

    for (idx = 0; idx < NUM_MESSAGE_TYPES; idx++)
    {
        HS_INT8 tbuf[32];

        sprintf_s(tbuf, "HSDB_MSGTYPE_%d", idx);

        if (hsInterface.AtrGet(m_objnum, tbuf))
            AddMessage(atoi(hsInterface.m_buffer));
    }

    // Set the console flag
    hsInterface.SetToggle(objnum, THING_HSPACE_CONSOLE);

    // Data is now stateful, do not clear post load
    // ClearObjectAttrs();
    return true;

}

// Call this function to have the console handle a message,
// which is usually given to the user of the console in some
// form.
void CHSConsole::HandleMessage(const HS_INT8 * strMsg, HS_INT32 type)
{

    // Is the console online?
    if (!IsOnline())
        return;

    if (GetMessage(type))
    {
        // Check to see if the console is broadcasting to the room.
        if (hsInterface.AtrGet(m_objnum, "ROOM_BROADCAST"))
        {
            hsInterface.NotifyContents(hsInterface.GetLocation(m_objnum),
                                       strMsg);
        }
        else
        {
            HS_DBREF dbUser;

            dbUser = hsInterface.ConsoleUser(m_objnum);
            if (dbUser == HSNOTHING)
                return;
            hsInterface.Notify(dbUser, strMsg);
        }
    }
}

// Returns the power needs for the console, which includes
// a base amount plus any weapon needs.
HS_UINT32 CHSConsole::GetMaximumPower()
{
    HS_UINT32 uPower;

    uPower = 2;                 // Console requires at least 2 MW of power

    // Figure in weapon needs
    if (m_weapon_array)
    {
        uPower += m_weapon_array->GetTotalPower();
    }

    return uPower;
}

// Can be used to tell the console to adjust its heading, for
// example, when the ship it belongs to turns.
void CHSConsole::AdjustHeading(HS_INT32 iXY, HS_INT32 iZ)
{
    m_xyheading += iXY;
    m_zheading += iZ;

    if (m_xyheading > 359)
        m_xyheading -= 360;
    else if (m_xyheading < 0)
        m_xyheading += 360;

    if (0 != iXY)
    {
        WriteXYHeadingAttr();
    }

    if (0 != iZ)
    {
        WriteZHeadingAttr();
    }
}

// Returns a character string containing the value of
// the requested console attribute.
HS_INT8 *CHSConsole::GetAttributeValue(HS_INT8 * strName)
{
    static HS_INT8 rval[64];
    HS_INT8 tmp[32];
    HS_INT32 idx;

    *rval = '\0';
    if (!_stricmp(strName, "CXYHEADING"))
    {
        sprintf_s(rval, "%d", m_xyheading);
    }
    else if (!_stricmp(strName, "CZHEADING"))
    {
        sprintf_s(rval, "%d", m_zheading);
    }
    else if (!_stricmp(strName, "USER"))
    {
        sprintf_s(rval, "#%d", hsInterface.ConsoleUser(m_objnum));
    }
    else if (!_stricmp(strName, "FIRING ARC"))
    {
        sprintf_s(rval, "%d", m_arc);
    }
    else if (!_stricmp(strName, "WEAPONS"))
    {

        // If we have a weapons array, run through
        // the weapons, pulling out their types.

        if (m_weapon_array)
        {
            CHSWeapon *pWeapon;
            for (pWeapon = m_weapon_array->GetFirstWeapon(); pWeapon;
                 pWeapon = m_weapon_array->GetNextWeapon())
            {
                // Add the class of the weapon to the list
                if (!*rval)
                    sprintf_s(tmp, "%d", pWeapon->GetData()->TypeID());
                else
                    sprintf_s(tmp, " %d", pWeapon->GetData()->TypeID());

                strcat_s(rval, tmp);
            }
        }
    }
    else if (!_stricmp(strName, "MESSAGES"))
    {
        for (idx = 0; idx < NUM_MESSAGE_TYPES; idx++)
        {
            if (m_msgtypes[idx] == HSNOTHING)
                continue;

            // Add the class of the message to the list
            if (!*rval)
                sprintf_s(tmp, "%d", m_msgtypes[idx]);
            else
                sprintf_s(tmp, " %d", m_msgtypes[idx]);

            strcat_s(rval, tmp);
        }
    }
    else if (!_stricmp(strName, "XYOFFSET"))
    {
        sprintf_s(rval, "%d", m_xyoffset);
    }
    else if (!_stricmp(strName, "ZOFFSET"))
    {
        sprintf_s(rval, "%d", m_zoffset);
    }
    else if (!_stricmp(strName, "CAN ROTATE"))
    {
        sprintf_s(rval, "%d", m_can_rotate ? 1 : 0);
    }
    else if (!_stricmp(strName, "LOCK"))
    {
        // Are we locked?
        if (!m_target_lock)
            return "#-1";

        CHSSysSensors *cSensors;

        // Get the sensors from our owner
        cSensors = (CHSSysSensors *) m_ownerObj->GetEngSystem(HSS_SENSORS);
        if (!cSensors)
            return "";

        SENSOR_CONTACT *cContact;
        cContact = cSensors->GetContact(m_target_lock);
        if (!cContact)
            return "";

        sprintf_s(rval, "%d", cContact->m_id);
    }
    else if (!_stricmp(strName, "ISPOWERED"))
    {
        sprintf_s(rval, "%d", IsOnline());
    }
    else
        return NULL;

    return rval;
}

// Attempts to set an attribute with a value for the
// console.
HS_BOOL8 CHSConsole::SetAttributeValue(HS_INT8 * strName, HS_INT8 * strValue)
{
    HS_INT32 iVal;

    // Find the attribute name, set the value.
    if (!_stricmp(strName, "XYHEADING"))
    {
        iVal = atoi(strValue);
        if (iVal < 0 || iVal > 359)
            return false;
        m_xyheading = iVal;
        WriteXYHeadingAttr();
        return true;
    }
    else if (!_stricmp(strName, "ZHEADING"))
    {
        iVal = atoi(strValue);
        if (iVal > 90 || iVal < -90)
            return false;
        m_zheading = iVal;
        WriteZHeadingAttr();
        return true;
    }
    else if (!_stricmp(strName, "XYOFFSET"))
    {
        iVal = atoi(strValue);
        if (iVal < 0 || iVal > 359)
            return false;
        m_xyoffset = iVal;
        WriteOffsetAttr();
        return true;
    }
    else if (!_stricmp(strName, "ZOFFSET"))
    {
        iVal = atoi(strValue);
        if (iVal > 90 || iVal < -90)
            return false;

        m_zoffset = iVal;
        WriteOffsetAttr();
        return true;
    }
    else if (!_stricmp(strName, "CAN ROTATE"))
    {
        m_can_rotate = atoi(strValue) == 0 ? false : true;
        WriteCanRotateAttr();
        return true;
    }
    else if (!_stricmp(strName, "FIRING ARC"))
    {
        iVal = atoi(strValue);
        if (iVal < 0 || iVal > 180)
            return false;
        m_arc = iVal;
        WriteFiringArcAttr();
        return true;
    }
    return false;
}

// Sets the CHS3DObject pointer for the object that this
// console belongs to.
void CHSConsole::SetOwnerObj(CHS3DObject * cObj)
{
    m_ownerObj = cObj;
}

// Tries to lock the console onto a target ID.  This is only
// possible if the console belongs to a certain object type
// which has sensors.
void CHSConsole::LockTargetID(HS_DBREF player, HS_INT32 ID)
{
    HS_UINT32 uMaxRange;
    double sX, sY, sZ;          // Our owner's coords.
    double tX, tY, tZ;          // Target's coords
    double dDistance;

    // Do we have an owner object?
    if (!m_ownerObj)
    {
        hsStdError(player,
                   "This console doesn't belong to an HSpace object.");
        return;
    }

    // Does the object type support sensors?
    if (m_ownerObj->GetType() != HST_SHIP)
    {
        hsStdError(player,
                   "Only consoles on space vessels support that command.");
        return;
    }

    // Do we have weapons?
    if (!m_weapon_array)
    {
        hsStdError(player, "This console is not equipped with weaponry.");
        return;
    }

    // Try to find the sensor contact for the vessel.
    CHSShip *cShip;
    SENSOR_CONTACT *cContact;

    cShip = (CHSShip *) m_ownerObj;
    cContact = cShip->GetSensorContact(ID);
    if (!cContact)
    {
        hsStdError(player, "No such contact ID on sensors.");
        return;
    }

    // Grab the CHS3DObject pointer from the sensor contact
    CHS3DObject *cTarget;
    cTarget = cContact->m_obj;

    // Find max weapon range
    // @TODO: This appears to be looking only at the FIRST weapon
    uMaxRange = m_weapon_array->GetMaxRange();

    // Calculate distance to target
    sX = cShip->GetX();
    sY = cShip->GetY();
    sZ = cShip->GetZ();
    tX = cTarget->GetX();
    tY = cTarget->GetY();
    tZ = cTarget->GetZ();
    dDistance = Dist3D(sX, sY, sZ, tX, tY, tZ);

    // Distance greater than max weapon range?
    if (dDistance > uMaxRange)
    {
        hsStdError(player, "Target is out of our max weapons range.");
        return;
    }

    // Already locked onto that target?
    if (m_target_lock)
    {
        if (m_target_lock->GetDbref() == cTarget->GetDbref())
        {
            hsStdError(player,
                       "Weapons are already locked onto that target.");
            return;
        }
    }

    // Lock onto the object
    CHS3DObject *cPrevLock;
    cPrevLock = m_target_lock;
    m_target_lock = cTarget;

    // Tell the old target we've unlocked?
    if (cPrevLock)
        cPrevLock->HandleLock(m_ownerObj, false);

    // Tell the new target we've locked.
    cTarget->HandleLock(m_ownerObj, true);

    hsStdError(player, "Weapons now locked on specified target contact.");
}

void CHSConsole::UnlockTarget(CHS3DObject* cTarget)
{
    // Check for invalid argument
    if(NULL == cTarget)
    {
        return;
    }

    // console is not currently targetting any object
    if(NULL == m_target_lock)
    {
        return;
    }

    if(m_target_lock->GetDbref() != cTarget->GetDbref())
    {
        return;
    }

    
   // Tell the new target we've locked.
   cTarget->HandleLock(m_ownerObj, false);
   m_target_lock = NULL;

   HS_DBREF dbUser = hsInterface.ConsoleUser(m_objnum);
   hsStdError(dbUser, "Target lock has been lost.");

}

// Tries to lock the console onto a target object.  This is only
// possible if the console belongs to a certain object type
// which has sensors.
void CHSConsole::LockTarget(CHS3DObject * cTarget)
{
    CHSShip *cShip;
    HS_UINT32 uMaxRange;
    double sX, sY, sZ;          // Our owner's coords.
    double tX, tY, tZ;          // Target's coords
    double dDistance;

    if (!cTarget)
        return;

    // Do we have an owner object?
    cShip = (CHSShip *) m_ownerObj;
    if (!cShip)
    {
        return;
    }

    // Does the object type support sensors?
    if (m_ownerObj->GetType() != HST_SHIP)
    {
        return;
    }

    // Do we have weapons?
    if (!m_weapon_array)
    {
        return;
    }

    // Find max weapon range
    uMaxRange = m_weapon_array->GetMaxRange();

    // Calculate distance to target
    sX = cShip->GetX();
    sY = cShip->GetY();
    sZ = cShip->GetZ();
    tX = cTarget->GetX();
    tY = cTarget->GetY();
    tZ = cTarget->GetZ();
    dDistance = Dist3D(sX, sY, sZ, tX, tY, tZ);

    // Distance greater than max weapon range?
    if (dDistance > uMaxRange)
    {
        return;
    }

    // Already locked onto that target?
    if (m_target_lock)
    {
        if (m_target_lock->GetDbref() == cTarget->GetDbref())
        {
            return;
        }
    }

    // Lock onto the object
    CHS3DObject *cPrevLock;
    cPrevLock = m_target_lock;
    m_target_lock = cTarget;

    // Tell the old target we've unlocked?
    if (cPrevLock)
        cPrevLock->HandleLock(m_ownerObj, false);

    // Tell the new target we've locked.
    cTarget->HandleLock(m_ownerObj, true);
}

// Returns the weapons array object for the console
CHSWeaponArray *CHSConsole::GetWeaponArray()
{
    return m_weapon_array;
}

// Gives the gunnery readout report for the console.
void CHSConsole::GiveGunneryReport(HS_DBREF player)
{
    CHSWeapon *pWeapon;
    HS_INT8 tbuf[256];

    // Do we have weapons?
    if (!m_weapon_array)
    {
        hsStdError(player, "This console is not equipped with weaponry.");
        return;
    }

    // Print out the header.
    sprintf_s(tbuf,
            "%s%s.-----------------------------------------------------------------------------.%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);
    sprintf_s(tbuf,
            "%s%s|%s Console Weaponry Report            %39s  %s%s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
            m_ownerObj->GetName(), ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);
    sprintf_s(tbuf,
            "%s%s >---------------------------------------------------------------------------<%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    // Print weapon info header
    sprintf_s(tbuf,
            "%s%s| %s[%s%sID%s] Name                         Status         Weapon Attributes          %s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
            ANSI_HILITE, ANSI_GREEN, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    // Run down the weapons in the array, calling on them for
    // information.
    HS_INT8 lpstrName[64];
    HS_UINT32 idx = 0;
    for (pWeapon = m_weapon_array->GetFirstWeapon(); pWeapon;
         pWeapon = m_weapon_array->GetNextWeapon(), idx++)
    {
        // Copy in the weapon name, and truncate it
        strcpy_s(lpstrName, pWeapon->GetName());
        lpstrName[28] = '\0';

        // Print weapon info.  It's up to the weapon to
        // give us most of the info.  The ID of the weapon
        // is array notation + 1.
        sprintf_s(tbuf,
                "%s%s|%s [%2d] %-29s%-15s%-27s%s%s|%s",
                ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
                idx + 1, lpstrName, pWeapon->GetStatus(),
                pWeapon->GetAttrInfo(), ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
        hsInterface.Notify(player, tbuf);
    }

    // Give autoload status
    sprintf_s(tbuf, "%s%s| %75s |%s", ANSI_HILITE, ANSI_BLUE, " ", ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    sprintf_s(tbuf,
            "%s%s| %sAutoloading:%s %-3s    %s%sAutorotate:%s %-3s %40s%s%s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
            m_autoload ? "ON" : "OFF",
            ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
            m_autorotate ? "ON" : "OFF", " ",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    // If there's a missile bay, give info for that
    if (m_missile_bay)
    {
        HS_BOOL8 bHeaderPrinted = false;

        // Print out storage info
        HS_INT32 iMissileType;
        for (iMissileType = m_missile_bay->GetFirstType();
             iMissileType != HSNOTHING;
             iMissileType = m_missile_bay->GetNextType())
        {
            // Has the header been printed?
            if (!bHeaderPrinted)
            {
                bHeaderPrinted = true;
                // Print missile bay header info
                sprintf_s(tbuf,
                        "%s%s >---------------------------------------------------------------------------<%s",
                        ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
                hsInterface.Notify(player, tbuf);
                sprintf_s(tbuf,
                        "%s%s|%s                       %s+%s- Munitions Storage -%s%s+                               %s|%s",
                        ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
                        ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
                        ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
                hsInterface.Notify(player, tbuf);
                sprintf_s(tbuf,
                        "%s%s|%77s|%s",
                        ANSI_HILITE, ANSI_BLUE, " ", ANSI_NORMAL);
                hsInterface.Notify(player, tbuf);
                sprintf_s(tbuf,
                        "%s%s| %s[%s%sID%s] Munitions Type                    Max      Remaining%19s%s|%s",
                        ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
                        ANSI_HILITE, ANSI_GREEN, " ", ANSI_BLUE, ANSI_NORMAL);
                hsInterface.Notify(player, tbuf);

            }

            CHSMissileData *pMissileData =
                static_cast <
                CHSMissileData * >(waWeapons.GetWeapon(iMissileType));

            if (pMissileData)
            {
                // Give the information
                sprintf_s(tbuf,
                        "%s%s|%s [%2d] %-34s%2d          %2d%23s%s%s|%s",
                        ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
                        pMissileData->TypeID(), pMissileData->Name(),
                        m_missile_bay->GetMaxMissiles(pMissileData->TypeID()),
                        m_missile_bay->GetMissilesLeft(pMissileData->
                                                       TypeID()), " ",
                        ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
                hsInterface.Notify(player, tbuf);
            }
        }
    }

    // Finish off the report.
    sprintf_s(tbuf,
            "%s%s`-----------------------------------------------------------------------------'%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);
}

// Allows a player (admin) to delete the specified weapon slot
// from the console.  The gunnery report lists weapons from 1 .. n,
// so we expect the same here.  However, weapon slots are from 
// 0 .. n, so that adjustment is made.
void CHSConsole::DeleteWeapon(HS_DBREF player, HS_INT32 slot)
{
    // Do we have a weapons array?
    if (!m_weapon_array)
    {
        hsInterface.Notify(player,
                           "This console is not equipped with weaponry.");
        return;
    }

    // Decrement slot by 1.
    slot--;
    if (slot < 0)
        slot = 0;

    // Check to see if the weapon exists.
    if (!m_weapon_array->GetWeapon(slot))
    {
        hsInterface.Notify(player,
                           "That weapon slot does not exist on that console.");
        return;
    }

    if (!m_weapon_array->DeleteWeapon(slot))
    {
        hsInterface.Notify(player,
                           "Failed to delete the weapon in that slot.");
        return;
    }

    hsInterface.Notify(player, "Weapon deleted.");
    WriteWeaponAttr();

    // Check to see if there is a weapon in slot 0.  If not,
    // then there are no weapons left, and we can delete the
    // weapons array.
    if (!m_weapon_array->GetWeapon(0))
    {
        delete m_weapon_array;
        m_weapon_array = NULL;
    }
}

// Allows a player (admin) to add a weapon of a certain type
// to the console.  All error checking is performed.
HS_BOOL8 CHSConsole::AddWeapon(HS_DBREF player, HS_INT32 type)
{
    CHSWeaponData *pData = waWeapons.GetWeapon(type);

    // Valid weapon type?
    if (!pData)
    {
        hsInterface.Notify(player, "Invalid weapon type specified.");
        return false;
    }

    // Do we have a weapons array?
    if (!m_weapon_array)
    {
        m_weapon_array = new CHSWeaponArray;

        // Tell the console array of our missile bay
        m_weapon_array->SetMissileBay(m_missile_bay);
    }

    CHSWeapon *pWeapon = CHSWeapon::CreateWeapon(pData->WeaponClass());

    if (!pWeapon)
    {
        hsInterface.Notify(player,
                           "Failed to add specified weapon type to console.");
    }
    else
    {
        pWeapon->SetData(pData);
        hsInterface.Notify(player,
                           hsInterface.
                           HSPrintf("Weapon type %d added to console.",
                                    type));
        m_weapon_array->AddWeapon(pWeapon);
        if (pData->WeaponClass() == WC_MISSILE)
        {
            // This weapon needs a pointer to our missile bay.
            ((CHSMTube *) pWeapon)->SetMissileBay(m_missile_bay);
        }
        WriteWeaponAttr();
    }

    return true;
}

// Tells the console where the missile bay is for its weapons
// to grab missiles from.
void CHSConsole::SetMissileBay(CHSMissileBay * mBay)
{
    m_missile_bay = mBay;

    if (m_weapon_array)
        m_weapon_array->SetMissileBay(mBay);

    WriteWeaponAttr();
}

// Configures a weapon so that it takes on the attributes
// of a specified weapon, usually a missile.
void CHSConsole::ConfigureWeapon(HS_DBREF player,
                                 HS_INT32 weapon, HS_INT32 type)
{
    CHSWeapon *cWeap;
    CHSWeaponData *pData;

    // Weapon and type are specified from 1 .. n, but weapons
    // are stored as 0 .. n-1, so decrement.
    weapon--;

    // Do we have any weapons?
    if (!m_weapon_array)
    {
        hsStdError(player, "This console is not equipped with weaponry.");
        return;
    }

    // Does the weapon exist?
    cWeap = m_weapon_array->GetWeapon(weapon);
    if (!cWeap)
    {
        hsStdError(player, "Invalid weapon ID specified.");
        return;
    }

    // Does the weapon support configuration?
    if (!cWeap->Configurable())
    {
        hsStdError(player, "That weapon type cannot be configured.");
        return;
    }

    // Do we have the type in storage?
    if (!m_missile_bay)
    {
        hsStdError(player, "No munitions storage available.");
        return;
    }

    pData = waWeapons.GetWeapon(type);
    if (!pData)
    {
        hsStdError(player, "Invalid weapon type specified.");
        return;
    }

    // Grab the storage slot.
    if (!m_missile_bay->HasType(type))
    {
        hsStdError(player,
                   hsInterface.
                   HSPrintf("Invalid munitions ID (%d) specified.",
                            type + 1));
        return;
    }

    // Try to configure it.
    if (!cWeap->Configure(pData->TypeID()))
    {
        hsStdError(player,
                   "Failed to configure weapon to specified munitions type.");
        return;
    }

    // All went ok.
    hsStdError(player, "Weapon configured to specified munitions type.");
}

// Allows a player to load a specific weapon.
void CHSConsole::LoadWeapon(HS_DBREF player, HS_INT32 weapon)
{
    CHSWeapon *cWeap;

    // Is our owner still active?
    if (false == m_ownerObj->IsActive())
    {
        if(HST_SHIP == m_ownerObj->GetType())
        {
            if(true == static_cast<CHSShip*>(m_ownerObj)->IsDestroyed())
            {
                hsStdError(player, "This ship has been destroyed!.");
                return;
            }
        }
    }


    // Do we have weapons?
    if (!m_weapon_array)
    {
        hsStdError(player, "This console is not equipped with weaponry.");
        return;
    }

    // Find the weapon.  Decrement weapon variable to go
    // to array notation.
    weapon--;
    cWeap = m_weapon_array->GetWeapon(weapon);
    if (!cWeap)
    {
        hsStdError(player, "Invalid weapon ID specified.");
        return;
    }

    // Is the weapon loadable?
    if (!cWeap->Loadable())
    {
        hsStdError(player, "That weapon cannot be loaded.");
        return;
    }

    // Is it already reloading
    if (cWeap->Reloading())
    {
        hsStdError(player, "That weapon is already currently loading.");
        return;
    }

    // Try to load it.
    if (!cWeap->Reload())
    {
        hsStdError(player, "Unable to reload that weapon at this time.");
        return;
    }

    // Went ok.
    hsStdError(player, "Reloading weapon ...");
}

// Handles cyclic stuff for consoles
void CHSConsole::DoCycle()
{
    HS_DBREF dbUser;
    dbUser = hsInterface.ConsoleUser(m_objnum);

    // Verify that the computer has enough power to
    // power us.
    HS_INT32 need;
    need = GetMaximumPower();

    // Check target lock.
    if (m_target_lock && m_target_lock != NULL)
    {
        HS_BOOL8 bUnlock = false;

        // Target still active?
        if (!m_target_lock->IsActive())
            bUnlock = true;

        // Find max weapon range
        HS_UINT32 uMaxRange;
        if (m_weapon_array)
            uMaxRange = m_weapon_array->GetMaxRange();
        else
            uMaxRange = 0;

        // Calculate distance to target
        double sX, sY, sZ;
        double tX, tY, tZ;
        sX = m_ownerObj->GetX();
        sY = m_ownerObj->GetY();
        sZ = m_ownerObj->GetZ();
        tX = m_target_lock->GetX();
        tY = m_target_lock->GetY();
        tZ = m_target_lock->GetZ();

        double dDistance;
        dDistance = Dist3D(sX, sY, sZ, tX, tY, tZ);

        // Distance greater than max weapon range?
        if (dDistance > uMaxRange)
        {
            bUnlock = true;
        }

        // Is our owner still active?
        if (!m_ownerObj->IsActive())
            bUnlock = true;

        // Unlock weapons?
        if (bUnlock)
        {
            m_target_lock->HandleLock(m_ownerObj, false);
            m_target_lock = NULL;
            if (dbUser != HSNOTHING)
                hsStdError(dbUser, "Target lock no longer capable.");
        }
    }

    if (!IsOnline())
        return;

    // If we have a weapons array, tell it to cycle.
    if (m_weapon_array)
    {
        m_weapon_array->DoCycle();

        // Indicate any weapons that just became ready;
        HS_UINT32 idx = 0;
        HS_INT8 tbuf[64];

        // Find the console's user.
        if (dbUser != HSNOTHING)
        {
            CHSWeapon *pWeapon;
            for (pWeapon = m_weapon_array->GetFirstWeapon();
                 pWeapon; pWeapon = m_weapon_array->GetNextWeapon(), idx++)
            {
                if (pWeapon->GetStatusChange() == STAT_READY)
                {
                    if (pWeapon->Loadable())
                        sprintf_s(tbuf, "%s%s[%s%s%d%s]%s - Weapon loaded.",
                                ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
                                ANSI_HILITE, idx + 1, ANSI_GREEN,
                                ANSI_NORMAL);
                    else
                        sprintf_s(tbuf, "%s%s[%s%s%d%s]%s - Weapon ready.",
                                ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
                                ANSI_HILITE, idx + 1, ANSI_GREEN,
                                ANSI_NORMAL);
                    hsInterface.Notify(dbUser, tbuf);

                    WriteWeaponAttr();
                }
            }
        }
    }
}

// Unlocks the weapons (if locked) from the locked
// target.
void CHSConsole::UnlockWeapons(HS_DBREF player)
{
    // Do we have an owner object?
    if (!m_ownerObj)
    {
        hsStdError(player,
                   "This console doesn't belong to an HSpace object.");
        return;
    }

    // Do we have weapons?
    if (!m_weapon_array)
    {
        hsStdError(player, "This console is not equipped with weaponry.");
        return;
    }

    // Are we currently locked?
    if (!m_target_lock)
    {
        hsStdError(player, "Weapons are not currently locked onto a target.");
        return;
    }

    // Tell the target we've unlocked
    m_target_lock->HandleLock(m_ownerObj, false);

    m_target_lock = NULL;

    // Tell the player
    hsStdError(player, "Weapons now unlocked from previous target.");
}

// Changes the heading of the console (in this case, a turret)
// to a given angle.  Error checking is performed to ensure
// that it doesn't turn into the ship it's on.
void CHSConsole::ChangeHeading(HS_DBREF player, HS_INT32 iXY, HS_INT32 iZ)
{
    HS_INT32 iXYAngle;
    HS_INT32 iZAngle;
    double is, js, ks;          // Ship vector
    double ic, jc, kc;          // Turret vector;

    // Do we have an owner object?
    if (!m_ownerObj)
    {
        hsStdError(player,
                   "This console does not belong to any HSpace object.");
        return;
    }

    // Can the console rotate like a turret?
    if (!m_can_rotate)
    {
        hsStdError(player, "This console does not have rotate capability.");
        return;
    }

    // Do we belong to a ship?
    if (m_ownerObj->GetType() != HST_SHIP)
    {
        hsStdError(player,
                   "This command only applies to consoles located on ships.");
        return;
    }
    CHSShip *cShip;
    cShip = (CHSShip *) m_ownerObj;

    // Do some error checking.
    if (iXY < 0)
        iXY = m_xyheading;
    else if (iXY > 359)
        iXY = 359;

    if (iZ < -90)
        iZ = m_zheading;
    else if (iZ > 90)
        iZ = 90;

    // The following code computes a normal vector for
    // the side of the ship we're located on.  We'll
    // then compute the normal vector for where the
    // console wants to turn to.  If the dot product
    // of the two vectors is negative, the console is
    // trying to turn into the ship.
    iXYAngle = cShip->GetXYHeading() + m_xyoffset;
    iZAngle = cShip->GetZHeading() + m_zoffset;

    // Make sure angles fall within specifications
    if (iXYAngle > 359)
        iXYAngle -= 360;
    else if (iXYAngle < 0)
        iXYAngle += 360;

    if (iZAngle < 0)
        iZAngle += 360;

    // Compute normal vector for the side of the ship
    // we're on.
    is = d2sin_table[iXYAngle] * d2cos_table[iZAngle];
    js = d2cos_table[iXYAngle] * d2cos_table[iZAngle];
    ks = d2sin_table[iZAngle];


    // Find normal vector for where we want to head.
    if (iZ < 0)
        iZ += 360;
    ic = d2sin_table[iXY] * d2cos_table[iZ];
    jc = d2cos_table[iXY] * d2cos_table[iZ];
    kc = d2sin_table[iZ];

    // Compute the dot product between the two vectors
    double dp;
    dp = (is * ic) + (js * jc) + (ks * kc);

    if (dp < 0)
    {
        hsStdError(player,
                   "This console cannot currently rotate to that angle.");
        return;
    }

    // Set the heading
    if (m_xyheading != iXY)
    {
        m_xyheading = iXY;
        WriteXYHeadingAttr();
    }

    // @TODO This looks suspect
    if (iZ > 90)
        iZ -= 360;
    if (m_zheading != iZ)
    {
        m_zheading = iZ;
        WriteZHeadingAttr();
    }

    HS_INT8 tbuf[128];
    sprintf_s(tbuf, "Console heading changed to %d mark %d.", iXY, iZ);
    hsStdError(player, tbuf);
}

// Allows a player to fire a specified weapon number.
void CHSConsole::FireWeapon(HS_DBREF player, HS_INT32 weapon,
                            HS_INT32 hit_value)
{
    // Weapons are specified from 1 .. n, but they're
    // stored in the array as 0 .. n-1, so decrement.
    weapon--;

    // Do we have weapons?
    if (!m_weapon_array)
    {
        hsStdError(player, "This console is not equipped with weaponry.");
        return;
    }

    // Is it a valid weapon?
    CHSWeapon *cWeap;
    cWeap = m_weapon_array->GetWeapon(weapon);
    if (!cWeap)
    {
        hsStdError(player, "Invalid weapon ID specified.");
        return;
    }

    // Does the weapon require a target lock?
    if (cWeap->RequiresLock() && !m_target_lock)
    {
        hsStdError(player,
                   "You must be locked onto a target to fire that weapon.");
        return;
    }

    // Can the weapon attack that type of object?
    if (!cWeap->CanAttackObject(m_target_lock))
    {
        hsStdError(player, "You cannot attack that target with that weapon.");
        return;
    }

    if (!cWeap->CheckTargetSize(m_target_lock))
    {
        hsStdError(player, "This weapon cannot attack a target that small.");
        return;
    }

    // Is the weapon ready?
    if (!cWeap->IsReady())
    {
        hsStdError(player, "That weapon is not ready for firing.");
        return;
    }

    // Do we try to autorotate to the target?
    if (m_target_lock && m_autorotate)
    {
        // Calculate the bearing to the target.
        HS_INT32 iXY, iZ;
        double sX, sY, sZ;      // Our coords
        double tX, tY, tZ;      // Target coords

        sX = m_ownerObj->GetX();
        sY = m_ownerObj->GetY();
        sZ = m_ownerObj->GetZ();

        tX = m_target_lock->GetX();
        tY = m_target_lock->GetY();
        tZ = m_target_lock->GetZ();

        iXY = XYAngle(sX, sY, tX, tY);
        iZ = ZAngle(sX, sY, sZ, tX, tY, tZ);

        ChangeHeading(player, iXY, iZ);
    }

    // Is the target in the cone?
    if (m_target_lock && !IsInCone(m_target_lock))
    {
        hsStdError(player, "Target is not currently within firing cone.");
        return;
    }

    cWeap->AttackObject(m_ownerObj, m_target_lock,
                        this, m_targetting, hit_value);

    // Is the weapon loadable, and should we reload?
    if (m_autoload && cWeap->Loadable())
        cWeap->Reload();
}

// Indicates whether a given CHS3DObject is within the firing
// cone of the console.  To do this, we calculate the vector
// to the target, use the dot product to get the difference
// between that vector and the console heading vector, and
// then convert the dot product to an angle.  We use this
// angle to see if it's larger than the cone.
HS_BOOL8 CHSConsole::IsInCone(CHS3DObject * cTarget)
{
    double ic, jc, kc;          // Console heading vector
    double it, jt, kt;          // Target vector
    HS_INT32 iXY, iZ;
    double tX, tY, tZ;          // Target coords.
    double sX, sY, sZ;          // Source (our) coords

    if (!cTarget)
        return false;

    if (!m_ownerObj)
        return false;

    sX = m_ownerObj->GetX();
    sY = m_ownerObj->GetY();
    sZ = m_ownerObj->GetZ();

    tX = cTarget->GetX();
    tY = cTarget->GetY();
    tZ = cTarget->GetZ();

    // Calculate the vector for where the console is
    // pointing.
    iZ = m_zheading;
    if (iZ < 0)
        iZ += 360;
    ic = d2sin_table[m_xyheading] * d2cos_table[iZ];
    jc = d2cos_table[m_xyheading] * d2cos_table[iZ];
    kc = d2sin_table[iZ];

    // Now calculate the vector to the target.
    iXY = XYAngle(sX, sY, tX, tY);
    if (iXY > 359)
        iXY -= 360;
    else if (iXY < 0)
        iXY += 360;
    iZ = ZAngle(sX, sY, sZ, tX, tY, tZ);
    if (iZ < 0)
        iZ += 360;

    it = d2sin_table[iXY] * d2cos_table[iZ];
    jt = d2cos_table[iXY] * d2cos_table[iZ];
    kt = d2sin_table[iZ];

    // Calculate the dot product of the two vectors.
    double dp;
    dp = (ic * it) + (jc * jt) + (kc * kt);
    if (dp > 1)
        dp = 1;
    else if (dp < -1)
        dp = -1;

    // Determine the angular difference between the vectors
    // based on the dot product.
    double diff;
    diff = acos(dp) * RADTODEG;

    // If the absolute difference between the vectors is
    // greater than 1/2 the firing arc, it's out of the cone.
    if (diff < 0)
        diff *= -1;
    if (diff > (m_arc * .5))
        return false;
    return true;
}

// Returns the current XYHeading of the console
HS_INT32 CHSConsole::GetXYHeading()
{
    return m_xyheading;
}

// Returns the ZHeading of the console
HS_INT32 CHSConsole::GetZHeading()
{
    return m_zheading;
}

// Allows a player to power up the console
void CHSConsole::PowerUp(HS_DBREF player)
{
    HS_INT32 maxpower;

    // Are we already online?
    if (IsOnline())
    {
        hsStdError(player, "This console is already online.");
        return;
    }

    // Does the computer have enough power?
    if (NULL != GetComputer())
    {
        maxpower = GetMaximumPower();
        if (!GetComputer()->DrainPower(maxpower))
        {
            hsStdError(player,
                       "Insufficient computer power to power console.");
            return;
        }
    }

    // Bring it online
    m_online = true;

    hsStdError(player,
               hsInterface.HSPrintf("%s now online.",
                                    hsInterface.GetName(m_objnum)));
}

// Powers down the console, releasing power to the computer
void CHSConsole::PowerDown(HS_DBREF player)
{
    HS_INT32 maxpower;

    if (!IsOnline())
    {
        hsStdError(player, "This console is not currently online.");
        return;
    }

    // Do we have a computer?
    if (NULL != GetComputer())
    {
        maxpower = GetMaximumPower();
        GetComputer()->ReleasePower(maxpower);
        m_online = false;
        hsStdError(player,
                   hsInterface.HSPrintf("%s powered down.",
                                        hsInterface.GetName(m_objnum)));
    }
    else
    {
        hsStdError(player, "That's not necessary with this console.");
    }
}

// Gives a quick, target report for when the weapons are
// locked onto a target.
void CHSConsole::GiveTargetReport(HS_DBREF player)
{
    SENSOR_CONTACT *cContact;

    // Are we locked onto a target?
    if (!m_target_lock)
    {
        hsStdError(player, "Weapons are not currently locked onto a target.");
        return;
    }

    // Find the contact in the owner's sensor array
    CHSShip *cShip;
    cShip = (CHSShip *) m_ownerObj;
    cContact = cShip->GetSensorContact(m_target_lock);

    HS_INT8 tbuf[256];          // For printing info

    double sX, sY, sZ;          // Owner's coords
    double tX, tY, tZ;          // Target's coords

    sX = m_ownerObj->GetX();
    sY = m_ownerObj->GetY();
    sZ = m_ownerObj->GetZ();

    tX = m_target_lock->GetX();
    tY = m_target_lock->GetY();
    tZ = m_target_lock->GetZ();

    // Calculate distance to target
    double dDistance;
    dDistance = Dist3D(sX, sY, sZ, tX, tY, tZ);

    // Print the header info
    sprintf_s(tbuf,
            "%s%s.----------------------------------------------------------.%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    int id = -1;
    if (cContact)
    {
        id = cContact->m_id;
    }

    sprintf_s(tbuf,
            "%s%s|%s Target Info Report                              ID: %-4d %s%s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, id,
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    sprintf_s(tbuf,
            "%s%s >--------------------------------------------------------<%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    // Is the target in the firing cone?
    if (IsInCone(m_target_lock))
    {
        sprintf_s(tbuf,
                "%s%s|%s                       %s%s* IN CONE *                        %s|%s",
                ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
                ANSI_HILITE, ANSI_RED, ANSI_BLUE, ANSI_NORMAL);
    }
    else
        sprintf_s(tbuf,
                "%s%s| %56s |%s", ANSI_HILITE, ANSI_BLUE, " ", ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    sprintf_s(tbuf,
            "%s%s|                  ______________________                  |%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    // Before printing the rest of the display, calculate
    // which vertical line and horizontal slot the indicators
    // are going into.
    double dx, dy, dz;

    // Determine vector from us to target
    dx = (HS_INT32) ((tX - sX) / dDistance);
    dy = (HS_INT32) ((tY - sY) / dDistance);
    dz = (HS_INT32) ((tZ - sZ) / dDistance);
    CHSVector tVec(dx, dy, dz);

    // Determine our heading vector for the console
    HS_INT32 iZAngle = m_zheading;

    //! \todo  This looks suspect too
    if (iZAngle < 0)
        iZAngle += 360;

    dx = d2sin_table[m_xyheading] * d2cos_table[iZAngle];
    dy = d2cos_table[m_xyheading] * d2cos_table[iZAngle];
    dz = d2sin_table[iZAngle];
    CHSVector hVec(dx, dy, dz);

    // Calculate dot product between both vectors
    double dp;
    dp = hVec.DotProduct(tVec);

    // Determine target bearing
    HS_INT32 iXYAngle;
    iZAngle = ZAngle(sX, sY, sZ, tX, tY, tZ);
    iXYAngle = XYAngle(sX, sY, tX, tY);

    // If the dot product is positive, the target is
    // on our visual screen.
    HS_INT32 iVertLine = -1;
    HS_INT32 iHorzSlot = -1;
    if (dp > 0)
    {

        // Determine which vertical line the target falls into
        iVertLine = (HS_INT32) (((iZAngle - m_zheading) - 90) * .05 * -1);

        // Determine which horizontal slot it goes into
        iHorzSlot = (HS_INT32) (((iXYAngle - m_xyheading) + 90) * .166667);
    }

    HS_INT8 tbuf2[32];          // Used for printing out the target

    strcpy_s(tbuf2, "                        ");
    if (iVertLine == 0)
        tbuf2[iHorzSlot - 3] = 'X';
    sprintf_s(tbuf,
            "%s%s|                /%s%s%s\\                |%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_RED, tbuf2, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    strcpy_s(tbuf2, "                            ");
    if (iVertLine == 1)
        tbuf2[iHorzSlot - 1] = 'X';
    sprintf_s(tbuf,
            "%s%s|              /%s%s%s\\              |%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_RED, tbuf2, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    HS_INT8 lbuf[32];           // Used for printing out a line in pieces
    HS_INT8 rbuf[32];

    if (iVertLine == 2)
    {
        strcpy_s(tbuf2, "               -              ");
        if (iHorzSlot > 10 && iHorzSlot < 20 &&
            iVertLine > 1 && iVertLine < 7)
        {
            // Just print everything in red
            tbuf2[iHorzSlot] = 'X';
            sprintf_s(tbuf,
                    "%s%s| %sYou:        %s|%s%s%s| %sTarget:     %s|%s",
                    ANSI_HILITE, ANSI_BLUE, ANSI_GREEN,
                    ANSI_BLUE, ANSI_RED,
                    tbuf2, ANSI_BLUE, ANSI_GREEN, ANSI_BLUE, ANSI_NORMAL);
        }
        else
        {
            // Target red, crosshairs white.  We have
            // to split the line up into three pieces
            strcpy_s(lbuf, "           ");
            strcpy_s(rbuf, "          ");
            if (iHorzSlot < 11)
                lbuf[iHorzSlot] = 'X';
            else
                rbuf[iHorzSlot - 20] = 'X';
            sprintf_s(tbuf,
                    "%s%s| %sYou:        %s|%s%s%s    -    %s%s%s| %sTarget:     %s|%s",
                    ANSI_HILITE, ANSI_BLUE, ANSI_GREEN,
                    ANSI_BLUE, ANSI_RED,
                    lbuf,
                    ANSI_WHITE, ANSI_RED,
                    rbuf, ANSI_BLUE, ANSI_GREEN, ANSI_BLUE, ANSI_NORMAL);
        }
    }
    else
    {
        // Print all in red or all in white
        if (iHorzSlot > 10 && iHorzSlot < 20 &&
            iVertLine > 1 && iVertLine < 7)
            sprintf_s(tbuf,
                    "%s%s| %sYou:        %s|%s               -              %s| %sTarget:     %s|%s",
                    ANSI_HILITE, ANSI_BLUE, ANSI_GREEN,
                    ANSI_BLUE, ANSI_RED,
                    ANSI_BLUE, ANSI_GREEN, ANSI_BLUE, ANSI_NORMAL);
        else
            sprintf_s(tbuf,
                    "%s%s| %sYou:        %s|%s               -              %s| %sTarget:     %s|%s",
                    ANSI_HILITE, ANSI_BLUE, ANSI_GREEN,
                    ANSI_BLUE, ANSI_WHITE,
                    ANSI_BLUE, ANSI_GREEN, ANSI_BLUE, ANSI_NORMAL);
    }
    hsInterface.Notify(player, tbuf);

    // Figure out the heading of the target.  This code
    // is fairly slow, but we have no choice.
    CHSVector thVec = m_target_lock->GetMotionVector();

    HS_INT32 tXYHeading = ((CHSShip*) m_target_lock)->GetXYHeading();
    HS_INT32 tZHeading = ((CHSShip*) m_target_lock)->GetZHeading();

    if (iVertLine == 3)
    {
        strcpy_s(tbuf2, "              ---             ");
        if (iHorzSlot > 10 && iHorzSlot < 20 &&
            iVertLine > 1 && iVertLine < 7)
        {
            // Just print everything in red
            tbuf2[iHorzSlot] = 'X';
            sprintf_s(tbuf,
                    "%s%s| %sH:%s %3dm%-3d  %s%s|%s%s%s| %sH:%s %3dm%-3d  %s%s|%s",
                    ANSI_HILITE, ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL,
                    m_xyheading, m_zheading,
                    ANSI_HILITE, ANSI_BLUE, ANSI_RED,
                    tbuf2,
                    ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL,
                    tXYHeading, tZHeading,
                    ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
        }
        else
        {
            // Target red, crosshairs white.  We have
            // to split the line up into three pieces
            strcpy_s(lbuf, "           ");
            strcpy_s(rbuf, "          ");
            if (iHorzSlot < 11)
                lbuf[iHorzSlot] = 'X';
            else
                rbuf[iHorzSlot - 20] = 'X';
            sprintf_s(tbuf,
                    "%s%s| %sH:%s %3dm%-3d  %s%s|%s%s%s   ---   %s%s%s| %sH:%s %3dm%-3d  %s%s|%s",
                    ANSI_HILITE, ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL,
                    m_xyheading, m_zheading,
                    ANSI_HILITE, ANSI_BLUE, ANSI_RED,
                    lbuf,
                    ANSI_WHITE, ANSI_RED,
                    rbuf, ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL,
                    tXYHeading, tZHeading,
                    ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
        }
    }
    else
    {
        // Print all in red or all in white
        if (iHorzSlot > 10 && iHorzSlot < 20 &&
            iVertLine > 1 && iVertLine < 7)
            sprintf_s(tbuf,
                    "%s%s| %sH:%s %3dm%-3d  %s%s|%s              ---             %s| %sH:%s %3dm%-3d  %s%s|%s",
                    ANSI_HILITE, ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL,
                    m_xyheading, m_zheading,
                    ANSI_HILITE, ANSI_BLUE, ANSI_RED,
                    ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL,
                    tXYHeading, tZHeading,
                    ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
        else
            sprintf_s(tbuf,
                    "%s%s| %sH:%s %3dm%-3d  %s%s|%s              ---             %s| %sH:%s %3dm%-3d  %s%s|%s",
                    ANSI_HILITE, ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL,
                    m_xyheading, m_zheading,
                    ANSI_HILITE, ANSI_BLUE, ANSI_WHITE,
                    ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL,
                    tXYHeading, tZHeading,
                    ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    }
    hsInterface.Notify(player, tbuf);

    if (iVertLine == 4)
    {
        strcpy_s(tbuf2, "           ( ( + ) )          ");
        if (iHorzSlot > 10 && iHorzSlot < 20 &&
            iVertLine > 1 && iVertLine < 7)
        {
            // Just print everything in red
            tbuf2[iHorzSlot] = 'X';
            sprintf_s(tbuf,
                    "%s%s| %sV:%s %-7d  %s%s|%s%s%s| %sV:%s %-7d  %s%s|%s",
                    ANSI_HILITE, ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL,
                    m_ownerObj->GetSpeed(),
                    ANSI_HILITE, ANSI_BLUE, ANSI_RED,
                    tbuf2,
                    ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL,
                    m_target_lock->GetSpeed(),
                    ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
        }
        else
        {
            // Target red, crosshairs white.  We have
            // to split the line up into three pieces
            strcpy_s(lbuf, "           ");
            strcpy_s(rbuf, "          ");
            if (iHorzSlot < 11)
                lbuf[iHorzSlot] = 'X';
            else
                rbuf[iHorzSlot - 20] = 'X';
            sprintf_s(tbuf,
                    "%s%s| %sV:%s %-7d  %s%s|%s%s%s( ( + ) )%s%s%s| %sV:%s %-7d  %s%s|%s",
                    ANSI_HILITE, ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL,
                    m_ownerObj->GetSpeed(),
                    ANSI_HILITE, ANSI_BLUE, ANSI_RED,
                    lbuf,
                    ANSI_WHITE, ANSI_RED,
                    rbuf, ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL,
                    m_target_lock->GetSpeed(),
                    ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
        }
    }
    else
    {
        // Print all in red or all in white
        if (iHorzSlot > 10 && iHorzSlot < 20 &&
            iVertLine > 1 && iVertLine < 7)
            sprintf_s(tbuf,
                    "%s%s| %sV:%s %-7d  %s%s|%s           ( ( + ) )          %s| %sV:%s %-7d  %s%s|%s",
                    ANSI_HILITE, ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL,
                    m_ownerObj->GetSpeed(),
                    ANSI_HILITE, ANSI_BLUE, ANSI_RED,
                    ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL,
                    m_target_lock->GetSpeed(),
                    ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
        else
            sprintf_s(tbuf,
                    "%s%s| %sV:%s %-7d  %s%s|%s           ( ( + ) )          %s| %sV:%s %-7d  %s%s|%s",
                    ANSI_HILITE, ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL,
                    m_ownerObj->GetSpeed(),
                    ANSI_HILITE, ANSI_BLUE, ANSI_WHITE,
                    ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL,
                    m_target_lock->GetSpeed(),
                    ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    }
    hsInterface.Notify(player, tbuf);

    if (iVertLine == 5)
    {
        strcpy_s(tbuf2, "              ___             ");
        if (iHorzSlot > 10 && iHorzSlot < 20 &&
            iVertLine > 1 && iVertLine < 7)
        {
            // Just print everything in red
            tbuf2[iHorzSlot] = 'X';
            sprintf_s(tbuf,
                    "%s%s|             |%s%s%s| %sR:%s %-5.0f    %s%s|%s",
                    ANSI_HILITE, ANSI_BLUE,
                    ANSI_RED,
                    tbuf2,
                    ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL,
                    dDistance, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
        }
        else
        {
            // Target red, crosshairs white.  We have
            // to split the line up into three pieces
            strcpy_s(lbuf, "           ");
            strcpy_s(rbuf, "          ");
            if (iHorzSlot < 11)
                lbuf[iHorzSlot] = 'X';
            else
                rbuf[iHorzSlot - 20] = 'X';
            sprintf_s(tbuf,
                    "%s%s|             |%s%s%s   ___   %s%s%s| %sR:%s %-5.0f    %s%s|%s",
                    ANSI_HILITE, ANSI_BLUE, ANSI_RED,
                    lbuf,
                    ANSI_WHITE, ANSI_RED,
                    rbuf, ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL,
                    dDistance, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
        }
    }
    else
    {
        // Print all in red or all in white
        if (iHorzSlot > 10 && iHorzSlot < 20 &&
            iVertLine > 1 && iVertLine < 7)
            sprintf_s(tbuf,
                    "%s%s|             |%s              ___             %s| %sR:%s %-5.0f    %s%s|%s",
                    ANSI_HILITE, ANSI_BLUE, ANSI_RED,
                    ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL,
                    dDistance, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
        else
            sprintf_s(tbuf,
                    "%s%s|             |%s              ___             %s| %sR:%s %-5.0f    %s%s|%s",
                    ANSI_HILITE, ANSI_BLUE, ANSI_WHITE,
                    ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL,
                    dDistance, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    }
    hsInterface.Notify(player, tbuf);

    if (iVertLine == 6)
    {
        strcpy_s(tbuf2, "               _              ");
        if (iHorzSlot > 10 && iHorzSlot < 20 &&
            iVertLine > 1 && iVertLine < 7)
        {
            // Just print everything in red
            tbuf2[iHorzSlot] = 'X';
            sprintf_s(tbuf,
                    "%s%s|             |%s%s%s| %sB:%s %3dm%-3d  %s%s|%s",
                    ANSI_HILITE, ANSI_BLUE,
                    ANSI_RED,
                    tbuf2,
                    ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL,
                    iXYAngle, iZAngle, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
        }
        else
        {
            // Target red, crosshairs white.  We have
            // to split the line up into three pieces
            strcpy_s(lbuf, "           ");
            strcpy_s(rbuf, "          ");
            if (iHorzSlot < 11)
                lbuf[iHorzSlot] = 'X';
            else
                rbuf[iHorzSlot - 20] = 'X';
            sprintf_s(tbuf,
                    "%s%s|             |%s%s%s    _    %s%s%s| %sB:%s %3dm%-3d  %s%s|%s",
                    ANSI_HILITE, ANSI_BLUE, ANSI_RED,
                    lbuf,
                    ANSI_WHITE, ANSI_RED,
                    rbuf,
                    ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL,
                    iXYAngle, iZAngle, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
        }
    }
    else
    {
        // Print all in red or all in white
        if (iHorzSlot > 10 && iHorzSlot < 20 &&
            iVertLine > 1 && iVertLine < 7)
            sprintf_s(tbuf,
                    "%s%s|             |%s               _              %s| %sB:%s %3dm%-3d  %s%s|%s",
                    ANSI_HILITE, ANSI_BLUE, ANSI_RED,
                    ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL,
                    iXYAngle, iZAngle, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
        else
            sprintf_s(tbuf,
                    "%s%s|             |%s               _              %s| %sB:%s %3dm%-3d  %s%s|%s",
                    ANSI_HILITE, ANSI_BLUE, ANSI_WHITE,
                    ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL,
                    iXYAngle, iZAngle, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    }
    hsInterface.Notify(player, tbuf);

    strcpy_s(tbuf2, "                              ");
    if (iVertLine == 7)
        tbuf2[iHorzSlot] = 'X';
    sprintf_s(tbuf,
            "%s%s|             |%s%s%s|             |%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_RED, tbuf2, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    strcpy_s(tbuf2, "                            ");
    if (iVertLine == 8)
        tbuf2[iHorzSlot - 1] = 'X';
    sprintf_s(tbuf,
            "%s%s|              \\%s%s%s/              |%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_RED, tbuf2, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    sprintf_s(tbuf,
            "%s%s`---------------\\\\.______________________.//---------------'%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);
}

// Indicates whether the console is on a given frequency.
HS_BOOL8 CHSConsole::OnFrq(double frq)
{
    // Check for the COMM_FRQS attr.
    if (!hsInterface.AtrGet(m_objnum, "COMM_FRQS"))
        return false;

    // Separate the string into blocks of spaces
    HS_INT8 *ptr;
    HS_INT8 *bptr;              // Points to the first HS_INT8 in the frq string
    double cfrq;
    ptr = strchr(hsInterface.m_buffer, ' ');
    bptr = hsInterface.m_buffer;
    while (ptr)
    {
        *ptr = '\0';
        cfrq = atof(bptr);

        // Do we have a match, Johnny?
        if (cfrq == frq)
            return true;

        // Find the next frq in the string.
        bptr = ptr + 1;
        ptr = strchr(bptr, ' ');
    }
    // Last frq in the string.
    cfrq = atof(bptr);
    if (cfrq == frq)
        return true;

    // No match!
    return false;
}

// Attempts to unload a given weapon, if that weapon is
// a loadable type of weapon.
void CHSConsole::UnloadWeapon(HS_DBREF player, HS_INT32 weapon)
{
    // Do we have any weapons?
    if (!m_weapon_array)
    {
        hsStdError(player, "This console is not equipped with weaponry");
        return;
    }

    // Find the weapon.
    weapon--;
    CHSWeapon *cWeap;
    cWeap = m_weapon_array->GetWeapon(weapon);
    if (!cWeap)
    {
        hsStdError(player, "Invalid weapon ID specified.");
        return;
    }

    // Is the weapon loadable and, therefore, unloadable?
    if (!cWeap->Loadable())
    {
        hsStdError(player, "That weapon cannot be unloaded.");
        return;
    }

    if (!cWeap->Unload())
        hsStdError(player, "Failed to unload specified weapon.");
    else
        hsStdError(player, "Weapon unloaded.");
}

// Sets the autoloading status for the console.  This is a
// level higher than letting the weapon handle it.  The console
// tries to reload any loadable weapons when they're fired.
void CHSConsole::SetAutoload(HS_DBREF player, HS_BOOL8 bStat)
{
    if (!bStat)
    {
        m_autoload = false;
        hsStdError(player, "Autoloading deactivated.");
    }
    else
    {
        m_autoload = true;
        hsStdError(player, "Autoloading activated.");
    }
}

// Sets the autorotate status for the console.  If activated,
// the turret automatically tries to rotate to the target
// when the fire command is issued.
void CHSConsole::SetAutoRotate(HS_DBREF player, HS_BOOL8 bStat)
{
    if (!bStat)
    {
        m_autorotate = false;
        hsStdError(player, "Autorotate deactivated.");
    }
    else
    {
        m_autorotate = true;
        hsStdError(player, "Autorotate activated.");
    }
}

// Sets the system that the player wishes to target when
// firing a targetable weapon.
void CHSConsole::SetSystemTarget(HS_DBREF player, HS_INT32 type)
{
    const HS_INT8 *ptr;

    // Do we have weapons?
    if (!m_weapon_array)
    {
        hsStdError(player, "This console is not equipped with weaponry.");
        return;
    }

    // Are weapons locked?
    if (!m_target_lock)
    {
        hsStdError(player, "Weapons are not currently locked on a target.");
        return;
    }

    if (type == HSS_NOTYPE)
    {
        m_targetting = HSS_NOTYPE;
        hsStdError(player, "System targetting disabled.");
    }
    else
    {
        // Find the system name.
        ptr = hsGetEngSystemName(type);

        m_targetting = type;

        if (!ptr)
            hsStdError(player, "Targetting unknown system on enemy target.");
        else
            hsStdError(player,
                       hsInterface.
                       HSPrintf
                       ("Weapons now targeting %s system on enemy target.",
                        ptr));
    }
}

// Returns a CHSVector object based on the current heading.
CHSVector & CHSConsole::GetHeadingVector()
{
    double i, j, k;
    HS_INT32 zang;

    zang = m_zheading;
    if (zang < 0)
        zang += 360;

    // Calculate coefficients
    i = d2sin_table[m_xyheading] * d2cos_table[zang];
    j = d2cos_table[m_xyheading] * d2cos_table[zang];
    k = d2sin_table[zang];

    static CHSVector tvec(i, j, k);
    return tvec;
}

// Returns the current target lock.
CHS3DObject *CHSConsole::GetObjectLock()
{
    return m_target_lock;
}

HS_BOOL8 CHSConsole::GetMessage(HS_INT32 msgType)
{
    HS_INT32 idx;
    HS_BOOL8 FoundType = false;

    if (msgType == MSG_GENERAL)
        return true;

    for (idx = 0; idx < NUM_MESSAGE_TYPES; idx++)
    {
        if (m_msgtypes[idx] == HSNOTHING)
            continue;
        else
            FoundType = true;

        if (m_msgtypes[idx] == msgType)
            return true;
    }

    if (FoundType)
        return false;
    else
        return true;
}

HS_BOOL8 CHSConsole::AddMessage(HS_INT32 msgType)
{
    HS_INT32 idx;

    for (idx = 0; idx < NUM_MESSAGE_TYPES; idx++)
    {
        if (m_msgtypes[idx] == HSNOTHING)
            continue;

        if (m_msgtypes[idx] == msgType)
            return false;
    }

    for (idx = 0; idx < NUM_MESSAGE_TYPES; idx++)
    {
        if (m_msgtypes[idx] == HSNOTHING)
            break;
    }

    if (idx == NUM_MESSAGE_TYPES)
        return false;

    m_msgtypes[idx] = msgType;
    WriteMessageTypeAttrs();
    return true;
}

HS_BOOL8 CHSConsole::DelMessage(HS_INT32 msgType)
{
    HS_INT32 idx;

    for (idx = 0; idx < NUM_MESSAGE_TYPES; idx++)
    {
        if (m_msgtypes[idx] == HSNOTHING)
            continue;

        if (m_msgtypes[idx] == msgType)
        {
            m_msgtypes[idx] = HSNOTHING;
            return true;
        }

    }

    return false;
}

void CHSConsole::WriteFiringArcAttr()
{
    hsInterface.AtrAdd(m_objnum, "HSDB_FIRING_ARC",
                       hsInterface.HSPrintf("%d", m_arc),
                       hsInterface.GetGodDbref(), AF_MDARK | AF_WIZARD);
}

void CHSConsole::WriteCanRotateAttr()
{
    hsInterface.AtrAdd(m_objnum, "HSDB_CAN_ROTATE",
                       hsInterface.HSPrintf("%d", m_can_rotate ? 1 : 0),
                       hsInterface.GetGodDbref(), AF_MDARK | AF_WIZARD);
}

void CHSConsole::WriteOffsetAttr()
{
    hsInterface.AtrAdd(m_objnum, "HSDB_XYOFFSET",
                       hsInterface.HSPrintf("%d", m_xyoffset),
                       hsInterface.GetGodDbref(), AF_MDARK | AF_WIZARD);
    hsInterface.AtrAdd(m_objnum, "HSDB_ZOFFSET",
                       hsInterface.HSPrintf("%d", m_zoffset),
                       hsInterface.GetGodDbref(), AF_MDARK | AF_WIZARD);
}

void CHSConsole::WriteMessageTypeAttrs()
{
    char tbuf[80];
    for (int idx = 0; idx < NUM_MESSAGE_TYPES; idx++)
    {
        if (m_msgtypes[idx] == HSNOTHING)
            continue;
        _snprintf_s(tbuf, 79, "HSDB_MSGTYPE_%d", idx);
        hsInterface.AtrAdd(m_objnum, tbuf,
                           hsInterface.HSPrintf("%d", m_msgtypes[idx]),
                           hsInterface.GetGodDbref(), AF_MDARK | AF_WIZARD);

    }
}

void CHSConsole::WriteXYHeadingAttr()
{
    hsInterface.AtrAdd(m_objnum, "HSDB_XYHEADING",
                       hsInterface.HSPrintf("%d", m_xyheading),
                       hsInterface.GetGodDbref(), AF_MDARK | AF_WIZARD);
}

void CHSConsole::WriteZHeadingAttr()
{
    hsInterface.AtrAdd(m_objnum, "HSDB_ZHEADING",
                       hsInterface.HSPrintf("%d", m_zheading),
                       hsInterface.GetGodDbref(), AF_MDARK | AF_WIZARD);
}


void CHSConsole::WriteWeaponAttr()
{
    HS_INT8 tbuf[256];
    HS_INT8 tbuf2[32];

    if (m_objnum == HSNOTHING)
        return;

    // Write weapons info
    if (m_weapon_array)
    {
        CHSWeapon *pWeapon;

        *tbuf = '\0';
        for (pWeapon = m_weapon_array->GetFirstWeapon(); pWeapon;
             pWeapon = m_weapon_array->GetNextWeapon())
        {
            if (!*tbuf)
                sprintf_s(tbuf2, "%d", pWeapon->GetData()->TypeID());
            else
                sprintf_s(tbuf2, " %d", pWeapon->GetData()->TypeID());

            strcat_s(tbuf, tbuf2);

        }
        hsInterface.AtrAdd(m_objnum, "HSDB_WEAPONS",
                           tbuf, hsInterface.GetGodDbref());
    }
    else
    {
        hsInterface.AtrAdd(m_objnum, "HSDB_WEAPONS", NULL,
                           hsInterface.GetGodDbref());
    }
}
