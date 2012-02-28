// -----------------------------------------------------------------------
//! $Id: hsship.cpp,v 1.59 2007/01/15 12:21:05 worsel Exp $
// -----------------------------------------------------------------------

#include "pch.h"

#include <cstring>
#include <stdlib.h>

#include "hscopyright.h"
#include "hsobjects.h"
#include "hsinterface.h"
#include "hsuniverse.h"
#include "hsuniversedb.h"
#include "hsutils.h"
#include "hsconf.h"
#include "hspace.h"
#include "hsansi.h"
#include "hsengines.h"
#include "hsjumpdrive.h"
#include "hswarpdrive.h"
#include "hscomm.h"
#include "hsdb.h"
#include "hsclass.h"
#include "hssensors.h"
#include "hsflags.h"
#include "hsmissile.h"
#include "hsshields.h"
#include "hsreactor.h"
#include "hscommunications.h"
#include "hslifesupport.h"
#include "hscloaking.h"
#include "hstachyon.h"
#include "hscelestial.h"

CHSShip::CHSShip()
{
    int idx;

    // Initialize a slew of variables
    m_type = HST_SHIP;
    m_ident = NULL;
    m_target = NULL;
    m_current_xyheading = 0;
    m_desired_xyheading = 0;
    m_current_zheading = 0;
    m_desired_zheading = 0;
    m_current_roll = 0;
    m_desired_roll = 0;
    m_hull_points = 1;
    m_map_range = 1000;
    m_drop_status = 0;
    m_dock_status = 0;
    m_age = 0;
    m_size = 0;

    bReactorOnline = false;
    bGhosted = false;
    m_in_space = true;
    m_docked = false;
    m_destroyed = false;
    m_hyperspace = false;
    m_warpengaged = false;
    m_class = 0;
    m_boarding_code = NULL;
    m_territory = NULL;
    m_objlocation = HSNOTHING;
    m_self_destruct_timer = 0;

    // Overridables
    m_can_drop = NULL;
    m_spacedock = NULL;
    m_cargo_size = NULL;
    m_minmanned = NULL;
    m_maxhull = NULL;

    memset(&m_classinfo, 0, sizeof(m_classinfo));

    SetHeadingVector(0, 0, 0);

    // Initialize board links
    m_hatches.clear();

    // Initialize consoles
    for (idx = 0; idx < MAX_SHIP_CONSOLES; idx++)
        m_console_array[idx] = NULL;
}

CHSShip::~CHSShip()
{
    int idx;

    if (m_ident)
        delete[]m_ident;

    // Delete consoles
    for (idx = 0; idx < MAX_SHIP_CONSOLES; idx++)
    {
        if (m_console_array[idx])
            delete m_console_array[idx];
    }
}

// Determines whether the ship is actively in space or, perhaps,
// docked or dropped.
HS_BOOL8 CHSShip::IsActive()
{
    if (!m_docked && m_in_space && !m_destroyed)
        return true;

    return false;
}

// Sets the heading vector on the ship to new coefficients
void CHSShip::SetHeadingVector(int i, int j, int k)
{
    CHSVector vec(i, j, k);

    m_motion_vector = vec;
}

// Given a class number, this function will setup the
// appropriate pointers and system information so that
// the ship becomes of that class type.  The function
// is not just used for changing a class.  It's also
// used when the ship is loaded from the database and
// needs its class information setup.
HS_BOOL8 CHSShip::SetClassInfo(HS_UINT32 uClass)
{
    CHSShipClass *hsClass;
    char tbuf[64];
    CHSEngSystem *cSys;
    CHSEngSystem *tSys = NULL;

    // Grab the class from the global array
    hsClass = CHSClassDB::GetInstance().GetClass(uClass);
    if (!hsClass)
    {
        sprintf(tbuf, "Class info for class #%d not found.", uClass);
        hs_log(tbuf);
        return false;
    }

    // Set the pointer to the class.
    m_classinfo = hsClass;

    
    if(0 == m_size)
    {
        m_size = hsClass->Size();
    }

    // Set the class id in case it hasn't already
    m_class = uClass;

    // ONLY duplicate system information if this is a fresh ship
    // without systems saved for itsself already!
    if (!m_systems.GetHead() && hsClass->m_pSystems)
    {
        m_systems = *(hsClass->m_pSystems);

        // Run through the systems, setting the owner to us.
        for (cSys = m_systems.GetHead(); cSys; cSys = cSys->GetNext())
        {
            tSys = hsClass->m_pSystems->GetSystem(cSys->GetType());
            if (tSys)
            {
                cSys->SetParentSystem(tSys);
            }
            cSys->SetOwner(this);
        }
    }


    // Do some fuel systems handling
    CHSFuelSystem *cFuel;
    cFuel = (CHSFuelSystem *) m_systems.GetSystem(HSS_FUEL_SYSTEM);
    if (cFuel)
    {
        CHSReactor *cReactor;
        cReactor = (CHSReactor *) m_systems.GetSystem(HSS_REACTOR);

        // Tell the reactor where to get fuel
        if (cReactor)
        {
            cReactor->SetFuelSource(cFuel);
        }

        // Tell the engines where to get fuel
        CHSSysEngines *cEngines;
        cEngines = (CHSSysEngines *) m_systems.GetSystem(HSS_ENGINES);
        if (cEngines)
        {
            cEngines->SetFuelSource(cFuel);
        }

        // Tell jump drives where to get fuel
        CHSJumpDrive *cJumpers;
        cJumpers = (CHSJumpDrive *) m_systems.GetSystem(HSS_JUMP_DRIVE);
        if (cJumpers)
        {
            cJumpers->SetFuelSource(cFuel);
        }

        CHSWarpDrive* cWarp;
        cWarp = (CHSWarpDrive*)  m_systems.GetSystem(HSS_WARP_DRIVE);
        if(cWarp)
        {
            cWarp->SetFuelSource(cFuel);
        }

    }
    return true;
}

HS_BOOL8 CHSShip::AddConsole(HS_DBREF objnum)
{
    int idx;
    int slot;

    // Verify that it's a good object
    if (!hsInterface.ValidObject(objnum))
        return false;

    // Find a slot
    slot = MAX_SHIP_CONSOLES;
    for (idx = 0; idx < MAX_SHIP_CONSOLES; idx++)
    {
        // Empty slot?
        if (!m_console_array[idx] && (slot == MAX_SHIP_CONSOLES))
            slot = idx;

        // Console already in array?
        if (m_console_array[idx] &&
            (m_console_array[idx]->m_objnum == objnum))
            return false;
    }

    if (slot == MAX_SHIP_CONSOLES)
    {
        char tbuf[256];
        sprintf(tbuf, "WARNING: Maximum consoles for ship #%d reached.",
                m_objnum);
        hs_log(tbuf);
        return false;
    }

    // Make a new console .. set the variables
    m_console_array[slot] = new CHSConsole;

    // Tell the console to load attributes from the object.
    // Even if it's a new console and new object, that's ok.
    m_console_array[slot]->LoadFromObject(objnum);

    // Record it's owner for later use in finding the
    // actual console object.
    m_console_array[slot]->SetOwner(m_objnum);

    // Record it's CHS3DObject owner
    m_console_array[slot]->SetOwnerObj(this);

    // Tell the console where our missile bay is
    m_console_array[slot]->SetMissileBay(&m_missile_bay);

    return true;
}

// Call this to have the object clear its attributes AND
// all attrs in the base objects that were saved to objects
// in the game.
void CHSShip::ClearObjectAttrs()
{

    CHS3DObject::ClearObjectAttrs();

    // Clear console attributes
    // Console data is now stateful, leave the data on the object
    //HS_UINT32 idx;
    //for (idx = 0; idx < MAX_SHIP_CONSOLES; idx++)
    //{
    //      if (m_console_array[idx])
    //              m_console_array[idx]->ClearObjectAttrs();
    //}
}

// Called upon to set an attribute value.  If the attribute
// name is not found, it's passed up to the base CHS3DObject 
// for handling.
HS_BOOL8 CHSShip::SetAttributeValue(char *strName, char *strValue)
{
    int iVal;

    if (!strcasecmp(strName, "IDENT"))
    {
        if (m_ident)
            delete[]m_ident;

        if (strValue)
        {
            m_ident = new char[strlen(strValue) + 1];
            strcpy(m_ident, strValue);
        }
        return true;
    }
    else if (!strcasecmp(strName, "ZHEADING"))
    {
        iVal = atoi(strValue);
        if (iVal < -90 || iVal > 90)
            return false;

        m_current_zheading = iVal;

        return true;
    }
    else if (!strcasecmp(strName, "XYHEADING"))
    {
        iVal = atoi(strValue);
        if (iVal < 0 || iVal > 359)
            return false;

        m_current_xyheading = iVal;

        return true;
    }
    else if (!strcasecmp(strName, "DESTROYED"))
    {
        iVal = atoi(strValue);

        if (m_destroyed && !iVal)
            ResurrectMe();

        m_destroyed = iVal == 0 ? false : true;

        return true;
    }
    else if (!strcasecmp(strName, "BOARDING CODE"))
    {
        if (m_boarding_code)
        {
            delete[]m_boarding_code;
            m_boarding_code = NULL;
        }

        if (strValue && (strlen(strValue) > 0))
        {
            m_boarding_code = new char[strlen(strValue) + 1];
            strcpy(m_boarding_code, strValue);
        }
        return true;
    }
    else if (!strcasecmp(strName, "CARGO SIZE"))
    {
        if (!strValue || !*strValue)
        {
            if (m_cargo_size)
            {
                delete m_cargo_size;
                m_cargo_size = NULL;
            }
            return true;
        }


        if (!m_cargo_size)
            m_cargo_size = new int;

        *m_cargo_size = atoi(strValue);
        return true;
    }
    else if (!strcasecmp(strName, "CAN DROP"))
    {
        if (!strValue || !*strValue)
        {
            if (m_can_drop)
            {
                delete m_can_drop;
                m_can_drop = NULL;
            }
            return true;
        }


        if (!m_can_drop)
            m_can_drop = new HS_BOOL8;

        *m_can_drop = atoi(strValue) == 0 ? false : true;
        return true;
    }
    else if (!strcasecmp(strName, "SPACEDOCK"))
    {
        if (!strValue || !*strValue)
        {
            if (m_spacedock)
            {
                delete m_spacedock;
                m_spacedock = NULL;
            }
            return true;
        }


        if (!m_spacedock)
            m_spacedock = new HS_BOOL8;

        *m_spacedock = atoi(strValue) == 0 ? false : true;
        return true;
    }
    else if (!strcasecmp(strName, "HULL"))
    {
        iVal = atoi(strValue);

        m_hull_points = iVal;

        if (m_hull_points <= 0 && !m_destroyed)
        {
            ExplodeMe();
        }
        return true;
    }
    else if (!strcasecmp(strName, "MAXHULL"))
    {
        if (!strValue || !*strValue)
        {
            if (NULL != m_maxhull)
            {
                delete m_maxhull;
                m_maxhull = NULL;
            }
            return true;
        }
        else
        {
            if (NULL == m_maxhull)
            {
                m_maxhull = new HS_UINT32;
            }

            if (NULL != m_maxhull)
            {
                *m_maxhull = atoi(strValue);
                return true;
            }

            return false;
        }
    }
    else if (!strcasecmp(strName, "MINMANNED"))
    {
        if (!strValue || !*strValue)
        {
            if (m_minmanned)
            {
                delete m_minmanned;
                m_minmanned = NULL;
            }
            return true;
        }


        if (!m_minmanned)
            m_minmanned = new int;

        *m_minmanned = atoi(strValue);
        return true;
    }
    else if (!strcasecmp(strName, "AGE"))
    {
        if (!strValue || !*strValue)
        {
            m_age = 0;
            return true;
        }
        m_age = atoi(strValue);
        return true;
    }
    else if (!strcasecmp(strName, "DOCKLOC")
             || !strcasecmp(strName, "DROPLOC"))
    {
        CHSUniverse *uDest;

        iVal = strtodbref(strValue);

        // If iVal < 0, then we set the vessel's
        // status to not-docked.
        if (iVal < 0)
        {
            if (!m_docked)      // Do nothing
                return true;


            uDest = GetUniverse();
            if (!uDest)
                return false;

            uDest->AddActiveObject(this);

            // Put the ship into space.
            MoveShipObject(uDest->GetID());

            m_docked = false;
            m_objlocation = HSNOTHING;
            m_in_space = true;

            return true;
        }
        else
        {
            if (!hsInterface.ValidObject(iVal))
                return false;

            MoveShipObject(iVal);
            m_docked = true;

            uDest = GetUniverse();
            if (uDest)
                uDest->RemoveActiveObject(this);

            m_in_space = false;

            // See if we can find what HSpace object
            // we're on now.
            CHS3DObject *cDest;
            cDest = dbHSDB.FindObjectByRoom(iVal);
            if (!cDest)
            {
                m_objlocation = HSNOTHING;
            }
            else
                m_objlocation = cDest->GetDbref();

            return true;
        }
    }
    else
        return CHS3DObject::SetAttributeValue(strName, strValue);
}

HS_UINT32 CHSShip::CargoSize()
{
    if (NULL != m_cargo_size)
    {
        return *m_cargo_size;
    }

    if (NULL != m_classinfo)
    {
        return m_classinfo->CargoSize();
    }

    return 0;
}

int CHSShip::GetMinManned()
{
    if (m_minmanned)
    {
        return *m_minmanned;
    }

    if (NULL != m_classinfo)
    {
        return m_classinfo->MinCrew();
    }

    return -1;
}


void CHSShip::GetAttributeList(CHSAttributeList & rlistAttributes)
{
    rlistAttributes.push_back("IDENT");
    rlistAttributes.push_back("HATCHES");
    rlistAttributes.push_back("SROOMS");
    rlistAttributes.push_back("CONSOLES");
    rlistAttributes.push_back("DESTROYED");
    rlistAttributes.push_back("BOARDING CODE");
    rlistAttributes.push_back("CAN DROP");
    rlistAttributes.push_back("SPACEDOCK");
    rlistAttributes.push_back("OBJLOC");
    rlistAttributes.push_back("MAXHULL");
    rlistAttributes.push_back("HULL");
    rlistAttributes.push_back("AGE");
    rlistAttributes.push_back("DOCKLOC");
    rlistAttributes.push_back("DXYHEADING");
    rlistAttributes.push_back("DZHEADING");
    rlistAttributes.push_back("DROLL");
    rlistAttributes.push_back("ZHEADING");
    rlistAttributes.push_back("XYHEADING");
    rlistAttributes.push_back("ROLL");
    rlistAttributes.push_back("CARGO SIZE");
    rlistAttributes.push_back("CLASS");
    rlistAttributes.push_back("CLASS NAME");
    rlistAttributes.push_back("MCONSOLES");
    rlistAttributes.push_back("MINMANNED");

    CHS3DObject::GetAttributeList(rlistAttributes);
}

// Returns a character string containing the value of the
// requested attribute.
char *CHSShip::GetAttributeValue(char *strName)
{
    static char rval[2048];
    char tmp[256];

    unsigned int idx;

    *rval = 0;
    if (!strcasecmp(strName, "IDENT"))
    {
        if (m_ident)
            strcpy(rval, m_ident);
    }
    else if (!strcasecmp(strName, "HATCHES"))
    {
        for (idx = 0; idx < m_hatches.size(); idx++)
        {
            sprintf(tmp, "#%d ", m_hatches.at(idx)->Object());
            strcat(rval, tmp);
        }
    }
    else if (!strcasecmp(strName, "SROOMS"))
    {
        CHSShipRoomSet::iterator iter;
        for (iter = m_setRooms.begin(); iter != m_setRooms.end(); iter++)
        {
            if (!*rval)
                sprintf(tmp, "#%d", *iter);
            else
                sprintf(tmp, " #%d", *iter);

            strcat(rval, tmp);
        }
    }
    else if (!strcasecmp(strName, "CONSOLES"))
    {
        for (idx = 0; idx < MAX_SHIP_CONSOLES; idx++)
        {
            if (m_console_array[idx])
            {
                if (!*rval)
                    sprintf(tmp, "#%d", m_console_array[idx]->m_objnum);
                else
                    sprintf(tmp, " #%d", m_console_array[idx]->m_objnum);

                strcat(rval, tmp);
            }
        }
    }
    else if (!strcasecmp(strName, "DESTROYED"))
    {
        sprintf(rval, "%d", m_destroyed ? 1 : 0);
    }
    else if (!strcasecmp(strName, "BOARDING CODE"))
    {
        if (m_boarding_code)
            strcpy(rval, m_boarding_code);
    }
    else if (!strcasecmp(strName, "CAN DROP"))
    {
        sprintf(rval, "%d", CanDrop());
    }
    else if (!strcasecmp(strName, "SPACEDOCK"))
    {
        sprintf(rval, "%d", IsSpacedock());
    }
    else if (!strcasecmp(strName, "OBJLOC"))
    {
        sprintf(rval, "#%d", m_objlocation);
    }
    else if (!strcasecmp(strName, "MAXHULL"))
    {
        sprintf(rval, "%d", GetMaxHullPoints());
    }
    else if (!strcasecmp(strName, "HULL"))
    {
        sprintf(rval, "%d", GetHullPoints());
    }
    else if (!strcasecmp(strName, "AGE"))
    {
        sprintf(rval, "%d", m_age);
    }
    else if (!strcasecmp(strName, "DOCKLOC"))
    {
        sprintf(rval, "#%d", m_docked ?
                hsInterface.GetLocation(m_objnum) : -1);
    }
    else if (!strcasecmp(strName, "DXYHEADING"))
        sprintf(rval, "%d", m_desired_xyheading);
    else if (!strcasecmp(strName, "DZHEADING"))
        sprintf(rval, "%d", m_desired_zheading);
    else if (!strcasecmp(strName, "DROLL"))
        sprintf(rval, "%d", m_desired_roll);
    else if (!strcasecmp(strName, "XYHEADING"))
        sprintf(rval, "%d", m_current_xyheading);
    else if (!strcasecmp(strName, "ZHEADING"))
        sprintf(rval, "%d", m_current_zheading);
    else if (!strcasecmp(strName, "ROLL"))
        sprintf(rval, "%d", m_current_roll);
    else if (!strcasecmp(strName, "CARGO SIZE"))
    {
        sprintf(rval, "%d", CargoSize());
    }
    else if (!strcasecmp(strName, "CLASS"))
        sprintf(rval, "%d", m_class);
    else if (!strcasecmp(strName, "CLASS NAME"))
    {
        strcpy(rval, m_classinfo != NULL ?
               m_classinfo->ClassName() : "Unknown Class");
    }
    else if (!strcasecmp(strName, "CLASSNAME"))
    {
        strcpy(rval, m_classinfo != NULL ?
               m_classinfo->ClassName() : "Unknown Class");
    }
    else if (!strcasecmp(strName, "MCONSOLES"))
        sprintf(rval, "%d", GetMannedConsoles());
    else if (!strcasecmp(strName, "MINMANNED"))
        sprintf(rval, "%d", GetMinManned());
    else
        return CHS3DObject::GetAttributeValue(strName);

    return rval;
}

// Return amount of manned consoles
int CHSShip::GetMannedConsoles()
{
    int rval, idx;

    rval = 0;
    for (idx = 0; idx < MAX_SHIP_CONSOLES; idx++)
    {
        if (m_console_array[idx])
        {
            if (hsInterface.ConsoleUser(m_console_array[idx]->m_objnum) !=
                m_console_array[idx]->m_objnum)
                rval += 1;
        }
    }
    return rval;
}

// Adds a registered room to a ship
HS_BOOL8 CHSShip::AddSroom(HS_DBREF dbRoom)
{
    if (HasSroom(dbRoom))
    {
        return true;
    }

    if (HSCONF.autozone)
    {
        hsInterface.SetLock(m_objnum, hsInterface.GetGodDbref(), LOCK_ZONE);
        hsInterface.SetObjectZone(dbRoom, m_objnum);
    }

    m_setRooms.insert(dbRoom);

    return true;
}

// Adds a registered blink to a ship
HS_BOOL8 CHSShip::AddHatch(HS_DBREF dbExit)
{
    CHSHatch *cHatch = NewHatch();

    if (!cHatch)
    {
        hs_log(hsInterface.
               HSPrintf("WARNING: Maximum hatches reached for ship #%d.",
                        m_objnum));
        return false;
    }

    // Initialize some attrs
    cHatch->Object(dbExit);
    cHatch->SetOwnerObject(this);

    // Set the flag
    hsInterface.SetToggle(dbExit, EXIT_HSPACE_HATCH);

    // Set the attributes on the object
    hsInterface.AtrAdd(dbExit, "HSDB_SHIP",
                       hsInterface.HSPrintf("#%d", m_objnum),
                       hsInterface.GetGodDbref(), AF_MDARK | AF_WIZARD);
    cHatch->InitObjectAttrs();

    return true;
}

// Remove a hatch from the hatches array.
HS_BOOL8 CHSShip::DeleteHatch(HS_DBREF dbExit)
{

    for (unsigned int idx = 0; idx < m_hatches.size(); idx++)
    {
        if (m_hatches.at(idx)->Object() == dbExit)
        {
            // clear the owner reference BEFORE deleting
            m_hatches.at(idx)->ClearOwnerObject();
            // clear space related data attributes
            m_hatches.at(idx)->ClearObjectAttrs();
            hsInterface.UnsetToggle(m_hatches.at(idx)->Object(),
                    EXIT_HSPACE_HATCH);

            delete m_hatches.at(idx);
            m_hatches.erase(m_hatches.begin() + idx);
            return true;
        }
    }

    return false;
}

// Deletes a given console from the registered consoles
HS_BOOL8 CHSShip::RemoveConsole(HS_DBREF dbConsole)
{
    int idx;

    for (idx = 0; idx < MAX_SHIP_CONSOLES; idx++)
    {
        if (m_console_array[idx] &&
            (m_console_array[idx]->m_objnum == dbConsole))
        {
            // Reset the flag
            hsInterface.UnsetToggle(dbConsole, THING_HSPACE_CONSOLE);

            m_console_array[idx]->ClearObjectAttrs();

            // Free the object
            delete m_console_array[idx];
            m_console_array[idx] = NULL;
            return true;
        }
    }
    return false;
}

HS_UINT32 CHSShip::ClassNum()
{
    return m_class;
}

// Deletes a given room from the registered srooms
HS_BOOL8 CHSShip::DeleteSroom(HS_DBREF dbRoom)
{
    CHSShipRoomSet::iterator iter = m_setRooms.find(dbRoom);
    if (iter == m_setRooms.end())
    {
        return false;
    }

    m_setRooms.erase(iter);
    return true;
}

CHSConsole *CHSShip::FindConsole(HS_DBREF dbConsole)
{
    HS_UINT32 idx;

    // Traverse our console array, looking for consoles
    // with matching dbrefs.
    for (idx = 0; idx < MAX_SHIP_CONSOLES; idx++)
    {
        if (m_console_array[idx] &&
            (m_console_array[idx]->m_objnum == dbConsole))
            return m_console_array[idx];
    }

    // Not found
    return NULL;
}

// Passed keys and values by the CHS3DObject LoadFromFile().
// We just have to handle them.
HS_BOOL8 CHSShip::HandleKey(int key, char *strValue, FILE * fp)
{
    HS_DBREF obj;
    HS_DBREF objnum;

    // Determine key and handle it
    switch (key)
    {
    case HSK_MISSILEBAY:       // Load in missile bay info
        m_missile_bay.LoadFromFile(fp);
        return true;

    case HSK_IDENT:
        if (m_ident)
            delete[]m_ident;
        m_ident = new char[strlen(strValue) + 1];
        strcpy(m_ident, strValue);
        return true;

    case HSK_BOARDING_CODE:
        if (m_boarding_code)
        {
            delete[]m_boarding_code;
            m_boarding_code = NULL;
        }

        if (strValue && strlen(strValue) > 0)
        {
            m_boarding_code = new char[strlen(strValue) + 1];
            strcpy(m_boarding_code, strValue);
        }
        return true;

    case HSK_HATCH:
        CHSHatch * cHatch;

        objnum = atoi(strValue);
        if (!hsInterface.ValidObject(objnum))
        {
            hs_log(hsInterface.HSPrintf("Invalid hatch skipped #%d",
                        objnum));
            return true;
        }

        cHatch = NewHatch();
        if (!cHatch)
        {
            hs_log("ERROR: Couldn't add specified hatch.");
        }
        else
        {
            cHatch->LoadFromObject(objnum);
            
            cHatch->SetOwnerObject(this);
            // Set the planet attribute
            hsInterface.AtrAdd(objnum, "HSDB_SHIP",
                               hsInterface.HSPrintf("#%d", m_objnum),
                               hsInterface.GetGodDbref());
        }
        return true;


    case HSK_CARGOSIZE:
        if (!m_cargo_size)
            m_cargo_size = new int;

        *m_cargo_size = atoi(strValue);
        return true;

    case HSK_DROP_CAPABLE:
        if (!m_can_drop)
            m_can_drop = new HS_BOOL8;

        *m_can_drop = atoi(strValue) == 0 ? false : true;
        return true;

    case HSK_HULL_POINTS:
        m_hull_points = atoi(strValue);
        return true;

    case HSK_SPACEDOCK:
        if (!m_spacedock)
            m_spacedock = new HS_BOOL8;

        *m_spacedock = atoi(strValue) == 0 ? false : true;
        return true;

    case HSK_OBJLOCATION:
        m_objlocation = atoi(strValue);
        if (!hsInterface.ValidObject(m_objlocation))
            m_objlocation = HSNOTHING;
        return true;

    case HSK_CLASSID:
        m_class = atoi(strValue);
        SetClassInfo(m_class);  /* We just want to load it from Object DB so we can
                                   can just delete systems manually */
        return true;

    case HSK_DROPPED:          // depricated, but still handled during loading
        {
            if (!m_bLoading)
            {
                return false;
            }

            if (!m_docked)
            {
                m_docked = atoi(strValue) == 0 ? false : true;

                if (m_docked)
                {
                    m_in_space = false;
                }
            }
            return true;
        }
        break;

    case HSK_DOCKED:
        {
            // If we're loading an older db, 
            // we have to watch for clobbering the dock status.
            if (m_bLoading && m_docked)
            {
                // Quit quietly.
                return true;
            }

            m_docked = atoi(strValue) == 0 ? false : true;

            if (m_docked)
            {
                m_in_space = false;
            }
            return true;
        }
        break;

    case HSK_DESTROYED:
        m_destroyed = atoi(strValue) == 0 ? false : true;
        return true;

    case HSK_XYHEADING:
        m_current_xyheading = atoi(strValue);
        m_desired_xyheading = m_current_xyheading;

        // Recompute heading vectors
        RecomputeVectors();
        return true;

    case HSK_ZHEADING:
        m_current_zheading = atoi(strValue);
        m_desired_zheading = m_current_zheading;

        // Recompute heading vectors
        RecomputeVectors();
        return true;

    case HSK_ROLL:
        m_current_roll = atoi(strValue);
        m_desired_roll = m_current_roll;
        return true;

    case HSK_MAXHULL:
        if (NULL == m_maxhull)
        {
            m_maxhull = new HS_UINT32;

            if (NULL == m_maxhull)
            {
                return false;
            }
        }

        *m_maxhull = atoi(strValue);
        return true;
        break;

    case HSK_MINMANNED:
        if (!m_minmanned)
            m_minmanned = new int;

        *m_minmanned = atoi(strValue);
        return true;

    case HSK_ROOM:
        {
            obj = atoi(strValue);

            // Double-check that it's a good object.
            if (!hsInterface.ValidObject(obj))
                return false;

            // Double-check that it's a room.
            if (hsInterface.GetType(obj) != TYPE_ROOM)
            {
                return false;
            }
            AddSroom(obj);
        }
        return true;

    case HSK_CONSOLE:
        {
            obj = atoi(strValue);

            // Double-check that it's a good object.
            if (!hsInterface.ValidObject(obj))
                return false;

            // Double-check that it's an object.
            if (hsInterface.GetType(obj) != TYPE_THING)
            {
                return false;
            }

            AddConsole(obj);
        }
        return true;

    case HSK_SYSTEMDEF:        // Handle a system!
        LoadSystem(fp);
        return true;

    case HSK_AGE:
        m_age = atoi(strValue);
        return true;

    default:                   // Just call the base function
        return (CHS3DObject::HandleKey(key, strValue, fp));
    }
}

// Cleans up any final ship details after it is loaded from
// the database.
void CHSShip::EndLoadObject()
{
}

// Overridden from the base CHS3DObject class, but we'll call up
// to that object as well.
void CHSShip::WriteToFile(FILE * fp)
{
    unsigned int idx;

    // Call the base class first, then output our
    // stuff.  It's best to do it that way, but not
    // really necessary.
    CHS3DObject::WriteToFile(fp);

    // Now our's
    if (m_ident)
        fprintf(fp, "IDENT=%s\n", m_ident);
    if (m_boarding_code)
        fprintf(fp, "BOARDING_CODE=%s\n", m_boarding_code);
    fprintf(fp, "HULL_POINTS=%d\n", m_hull_points);
    fprintf(fp, "DOCKED=%d\n", m_docked);
    fprintf(fp, "DESTROYED=%d\n", m_destroyed);
    fprintf(fp, "XYHEADING=%d\n", m_current_xyheading);
    fprintf(fp, "ZHEADING=%d\n", m_current_zheading);
    fprintf(fp, "ROLL=%d\n", m_current_roll);
    fprintf(fp, "AGE=%d\n", m_age);

    if (m_objlocation != HSNOTHING)
        fprintf(fp, "OBJLOCATION=%d\n", m_objlocation);

    if (m_can_drop)
        fprintf(fp, "DROP CAPABLE=%d\n", *m_can_drop);

    if (m_spacedock)
        fprintf(fp, "SPACEDOCK=%d\n", *m_spacedock);

    if (m_cargo_size)
        fprintf(fp, "CARGOSIZE=%d\n", *m_cargo_size);

    if (m_minmanned)
        fprintf(fp, "MINMANNED=%d\n", *m_minmanned);

    if (NULL != m_maxhull)
    {
        fprintf(fp, "MAXHULL=%d\n", *m_maxhull);
    }

    // Write out missile bay info.  This needs to come
    // before consoles.
    fprintf(fp, "MISSILEBAY\n");
    m_missile_bay.SaveToFile(fp);

    // Write out rooms
    CHSShipRoomSet::iterator iter;
    for (iter = m_setRooms.begin(); iter != m_setRooms.end(); iter++)
    {
        fprintf(fp, "ROOM=%d\n", *iter);
    }

    // Write out consoles
    for (idx = 0; idx < MAX_SHIP_CONSOLES; idx++)
    {
        if (m_console_array[idx])
        {
            fprintf(fp, "CONSOLE=%d\n", m_console_array[idx]->m_objnum);

            // Console data is now stateful, no need to write to 
            // the game object during dumps
            //m_console_array[idx]->WriteObjectAttrs();
        }
    }

    // Hatches
    for (idx = 0; idx < m_hatches.size(); idx++)
    {
        fprintf(fp, "HATCH=%d\n", m_hatches.at(idx)->Object());
    }

    fprintf(fp, "CLASSID=%d\n", m_class);

    // Output system information
    m_systems.SaveToFile(fp);
}

// When the HandleKey() encounters a SYSTEMDEF key, it
// calls this function to load the system information into
// a system that already exists on the ship.
HS_BOOL8 CHSShip::LoadSystem(FILE * fp)
{
    CHSEngSystem *cSys;
    CHSEngSystem *cSysTmp;
    char strKey[64];
    char strValue[64];
    char *ptr;
    char tbuf[256];
    HS_DBKEY key;
    int iCurSlot;
    int iPriority;
    HSS_TYPE type;

    cSys = NULL;

    // We have NO WAY of knowing what kind of information each
    // system needs.  We ONLY know what types of systems exist
    // because this is defined in hseng.h.  Thus, we simply
    // figure out the type of system, and then pass the key/value
    // information to the system as if it were setting attributes.
    while (fgets(tbuf, 255, fp))
    {
        // Truncate newlines
        if ((ptr = strchr(tbuf, '\n')) != NULL)
            *ptr = '\0';
        if ((ptr = strchr(tbuf, '\r')) != NULL)
            *ptr = '\0';

        extract(tbuf, strKey, 0, 1, '=');
        extract(tbuf, strValue, 1, 1, '=');
        key = HSFindKey(strKey);

        // Check for end of database .. would be an error
        if (!strcmp(strKey, "*END*"))
        {
            hs_log("ERROR: Encountered end of objectdb before loading \
					specified system info.");
            break;
        }

        // Check the key, and do something
        switch (key)
        {
        case HSK_SYSTYPE:
            // Find the system on the ship
            type = (HSS_TYPE) atoi(strValue);
            cSys = m_systems.GetSystem(type);
            // Don't exit if system not found, just
            // warning.
            if (!cSys || type == HSS_FICTIONAL)
            {
                cSys = CHSEngSystem::CreateFromType(type);

                if (!cSys)
                {
                    hs_log(hsInterface.
                           HSPrintf("WARNING: Failed to load system \
									type %d on object #%d.", type, m_objnum));
                    return false;
                }
                m_systems.AddSystem(cSys);
            }
            break;

        case HSK_SYSTEMEND:
            {
                cSys->SetOwner(this);
            }
            return true;

        case HSK_PRIORITY:
            // Put this system back into it's proper slot.
            // Initially it is in the priority position specified
            // by how it was loaded in the class object.  That
            // may or may not be the position it was at for the
            // specific ship.  Thus, we have to handle that.
            iPriority = atoi(strValue);

            // If no system, then do nothing
            if (!cSys)
                break;

            // Find the current priority level
            iCurSlot = 0;
            for (cSysTmp = m_systems.GetHead(); cSysTmp;
                 cSysTmp = cSysTmp->GetNext())
            {
                if (!strcmp(cSysTmp->GetName(), cSys->GetName()))
                {
                    // System match.
                    break;
                }
                else
                {
                    iCurSlot++;
                }
            }

            if (iCurSlot > iPriority)
            {
                // Move the system up in priority
                while (iCurSlot > iPriority)
                {
                    if (!m_systems.BumpSystem(cSys, 1))
                        break;

                    iCurSlot--;
                }
            }
            else if (iCurSlot < iPriority)
            {
                // Move the system down
                while (iCurSlot < iPriority)
                {
                    if (!m_systems.BumpSystem(cSys, -1))
                        break;

                    iCurSlot++;
                }
            }
            break;

        default:
            {
                // Simply call upon the object to set it's
                // own attributes.  We don't know what they
                // are.
                if (cSys)
                {
                    if (!cSys->SetAttributeValue(strKey, strValue))
                    {
                        sprintf(tbuf, "WARNING: Failed to set supported \
									attribute \"%s\" on system type %d.", strKey, type);
                        hs_log(tbuf);
                    }
                }
            }
            break;
        }
    }
    return false;
}

// Does various system stuff each cycle.
void CHSShip::HandleSystems()
{
    CHSSysEngines *cEngines;
    CHSReactor *cReactor;
    CHSJumpDrive *cJumpers;
    int iPowerDeficit;
    int iSysPower;
    int iSpeed;
    HS_BOOL8 bStat;
    char tbuf[256];
    CHSEngSystem *cSys;


    // Find the regular engines.
    cEngines = (CHSSysEngines *) m_systems.GetSystem(HSS_ENGINES);
    if (cEngines)
        iSpeed = (int) cEngines->GetCurrentSpeed();

    // Find jump drives to see if we get a disengage status.
    HS_BOOL8 bEngaged = false;
    cJumpers = (CHSJumpDrive *) m_systems.GetSystem(HSS_JUMP_DRIVE);
    if (cJumpers)
    {
        bEngaged = cJumpers->GetEngaged();
        if (bEngaged)
        {
            // Tell the jumpers our current sublight speed.
            cJumpers->SetSublightSpeed(iSpeed);
        }
    }

    // Tell the systems to do their cycle stuff
    m_systems.DoCycle();

    // Check various jump conditions.
    if (bEngaged)
    {
        if (!cJumpers->GetEngaged())
        {
            ExitHyperspace();
        }
        else if (cEngines)
        {
            if (cEngines->GetCurrentSpeed() < cJumpers->GetMinJumpSpeed())
            {
                cJumpers->SetEngaged(false);
                ExitHyperspace();
            }
        }
    }

    if (m_self_destruct_timer > 0)
    {
        m_self_destruct_timer--;

        if (m_self_destruct_timer < 1)
        {
            NotifySrooms(hsInterface.
                         HSPrintf("%s%s*** SELF DESTRUCT IN PROGRESS ***%s",
                                  ANSI_GREEN, ANSI_HILITE, ANSI_NORMAL));

            ExplodeMe();
            hsInterface.InvokeResponse(m_objnum, m_objnum, NULL, NULL,
                                       "ASELFDESTRUCT");

            if (!hsInterface.HasFlag(m_objnum, TYPE_THING, THING_HSPACE_SIM))
                KillShipCrew("SELF DESTRUCTED");
        }
        else
        {
            if (m_self_destruct_timer == 3600)
            {
                NotifySrooms(hsInterface.
                             HSPrintf("%s%s*** SELF DESTRUCT IN 1 HOUR ***%s",
                                      ANSI_RED, ANSI_HILITE, ANSI_NORMAL));
            }
            else if (m_self_destruct_timer == 1200)
            {
                NotifySrooms(hsInterface.
                             HSPrintf
                             ("%s%s*** SELF DESTRUCT IN 20 MINUTES ***%s",
                              ANSI_RED, ANSI_HILITE, ANSI_NORMAL));
            }
            else if (m_self_destruct_timer == 300)
            {
                NotifySrooms(hsInterface.
                             HSPrintf
                             ("%s%s*** SELF DESTRUCT IN 5 MINUTES ***%s",
                              ANSI_RED, ANSI_HILITE, ANSI_NORMAL));
            }
            else if (m_self_destruct_timer == 120)
            {
                NotifySrooms(hsInterface.
                             HSPrintf
                             ("%s%s*** SELF DESTRUCT IN 2 MINUTES ***%s",
                              ANSI_RED, ANSI_HILITE, ANSI_NORMAL));
            }
            else if (m_self_destruct_timer == 60)
            {
                NotifySrooms(hsInterface.
                             HSPrintf
                             ("%s%s*** SELF DESTRUCT IN 1 MINUTE ***%s",
                              ANSI_RED, ANSI_HILITE, ANSI_NORMAL));
            }
            else if (m_self_destruct_timer == 30)
            {
                NotifySrooms(hsInterface.
                             HSPrintf
                             ("%s%s*** SELF DESTRUCT IN 30 SECONDS ***%s",
                              ANSI_RED, ANSI_HILITE, ANSI_NORMAL));
            }
            else if (m_self_destruct_timer == 20)
            {
                NotifySrooms(hsInterface.
                             HSPrintf
                             ("%s%s*** SELF DESTRUCT IN 20 SECONDS ***%s",
                              ANSI_RED, ANSI_HILITE, ANSI_NORMAL));
            }
            else if (m_self_destruct_timer < 11)
            {
                NotifySrooms(hsInterface.
                             HSPrintf
                             ("%s%s*** SELF DESTRUCT IN %d SECONDS ***%s",
                              ANSI_RED, ANSI_HILITE, m_self_destruct_timer,
                              ANSI_NORMAL));
            }
        }
    }

    cReactor = (CHSReactor *) m_systems.GetSystem(HSS_REACTOR);
    if (cReactor)
    {
        bStat = cReactor->IsOnline();
        if (!bReactorOnline && bStat)
        {
            bReactorOnline = true;
            sprintf(tbuf,
                    "%s%s-%s A light flashes on your console, indicating that the main reactor is now online.",
                    ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL);
            NotifyConsoles(tbuf, MSG_ENGINEERING);

            if (hsInterface.AtrGet(m_objnum, "HSMSG_REACTOR_ACTIVATING"))
            {
                char *msg = hsInterface.EvalExpression(hsInterface.m_buffer,
                                                       m_objnum, m_objnum,
                                                       m_objnum);
                NotifySrooms(msg);
            }
            else
            {
                NotifySrooms(HSCONF.reactor_activating);
            }

            // Give an outside effects message
            HS_DBREF loc;

            loc = GetDockedLocation();
            if (loc != HSNOTHING)
            {
                if (hsInterface.AtrGet(m_objnum,
                                       "HSMSG_REACTOR_ACTIVATING_EXTERNAL"))
                {
                    char *msg =
                        hsInterface.EvalExpression(hsInterface.m_buffer,
                                                   m_objnum, m_objnum,
                                                   m_objnum);
                    hsInterface.NotifyContents(loc, msg);
                }
                else
                {
                    char tbuf[128];
                    sprintf(tbuf,
                            "Lights flicker on the %s as its reactor comes online.",
                            GetName());
                    hsInterface.NotifyContents(loc, tbuf);
                }
            }
        }
        else if (bReactorOnline && !bStat)
            bReactorOnline = false;

        // Check power availability.  If the reactor is producing
        // less power than we're using, we have to start drawing power
        // from systems.
        iPowerDeficit = m_systems.GetPowerUse() - cReactor->GetOutput();
        if (iPowerDeficit > 0)
        {
            // Start drawing power from systems
            cSys = m_systems.GetHead();


            while (cSys && (iPowerDeficit > 0))
            {
                // Don't draw power from non-visible systems
                if (!cSys->IsVisible())
                {
                    cSys = cSys->GetNext();
                    continue;
                }

                iSysPower = cSys->GetCurrentPower();

                // Can we get enough power from this system?
                if (iSysPower >= iPowerDeficit)
                {
                    // Draw the needed power from this system.
                    iSysPower -= iPowerDeficit;
                    iPowerDeficit -= cSys->GetCurrentPower();
                    cSys->SetCurrentPower(iSysPower);

                    sprintf(tbuf,
                            "%s%s-%s A warning light flashes, indicating that the %s system has lost power.",
                            ANSI_HILITE, ANSI_YELLOW, ANSI_NORMAL,
                            cSys->GetName());
                    NotifyConsoles(tbuf, MSG_ENGINEERING);
                }
                else if (iSysPower > 0)
                {
                    // Draw ALL power from this system.
                    iPowerDeficit -= iSysPower;
                    cSys->SetCurrentPower(0);

                    sprintf(tbuf,
                            "%s%s-%s A warning light flashes, indicating that the %s system has lost power.",
                            ANSI_HILITE, ANSI_YELLOW, ANSI_NORMAL,
                            cSys->GetName());
                    NotifyConsoles(tbuf, MSG_ENGINEERING);
                }
                cSys = cSys->GetNext();
            }
        }
        if (bReactorOnline)
            m_age++;
    }

    // Now do stuff for each system.

    // Do sensor stuff (this isn't finding contacts.
    // That's done in the DoCycle() of systems.).
    HandleSensors();

    // Life support
    HandleLifeSupport();
}

// This is THE CYCLE for ships.  It handles everything from
// traveling to sensor sweeping.
void CHSShip::DoCycle()
{
    int idx;

    // Don't do anything if we're destroyed
    if (m_destroyed)
        return;


    // Consoles
    for (idx = 0; idx < MAX_SHIP_CONSOLES; idx++)
    {
        if (m_console_array[idx])
        {
            m_console_array[idx]->DoCycle();
        }
    }

    HS_DBREF loc;
    loc = GetDockedLocation();
    if (loc)
    {
        CHSLandingLoc *cLocation;
        cLocation = dbHSDB.FindLandingLoc(loc);
        if (cLocation)
        {
            CHS3DObject *cDocked;

            cDocked = cLocation->GetOwnerObject();

            if (cDocked)
            {
                if (cDocked->GetType() == HST_SHIP)
                {
                    CHSShip *cShip;

                    cShip = (CHSShip *) cLocation->GetOwnerObject();
                    if (cShip)
                    {
                        SetX(cShip->GetX());
                        SetY(cShip->GetY());
                        SetZ(cShip->GetZ());
                    }
                }
            }
        }
    }

    // System stuff first.
    HandleSystems();

    // See if we're landing or taking off
    if (m_drop_status > 0 || m_dock_status > 0)
    {
        HandleLanding();
    }
    else if (m_drop_status < 0 || m_dock_status < 0)
    {
        HandleUndocking();
    }

    // Change the heading of the ship.
    ChangeHeading();

    // Move the ship to new coordinates
    Travel();

    // Check Hatches validity
    ConfirmHatches();

}


// Call this function to completely repair everything on the ship.
void CHSShip::TotalRepair()
{
    CHSEngSystem *cSys;

    // Not destroyed
    m_destroyed = false;

    // Repair all systems
    for (cSys = m_systems.GetHead(); cSys; cSys = cSys->GetNext())
    {
        cSys->ReduceDamage(DMG_NONE);
        cSys->SetAttributeValue("STRESS", "0");
    }

    // Repair hull
    m_hull_points = GetMaxHullPoints();

    // Put the ship into space if it's not docked or dropped
    if (!m_docked)
    {
        ResurrectMe();
    }
}

// Returns the current cloaking efficiency for the ship.
// If the ship cannot cloak or isn't cloaked, this is just 0.
// This value ranges from 0 - 1.00, where 1.00 is 100% visible.
float CHSShip::CloakingEffect()
{
    CHSSysCloak *cCloak;
    float rval;

    // Look for the cloaking device.
    cCloak = (CHSSysCloak *) m_systems.GetSystem(HSS_CLOAK);
    if (!cCloak)
    {
        return 1;
    }

    if (cCloak->GetEngaged())
    {
        rval = cCloak->GetEfficiency(true);
        rval = (100 - rval) / 100;
        return rval;
    }
    else
    {
        return 1;
    }
}

float CHSShip::TachyonEffect()
{
    CHSSysTach *cTach;
    float rval;


    // Look for the Tachyon Sensor Array.
    cTach = (CHSSysTach *) m_systems.GetSystem(HSS_TACHYON);
    if (!cTach)
    {
        return 0;
    }

    rval = cTach->GetEfficiency(true);
    rval = rval / 100;
    return rval;

}

// Used to query sensors, give messages to the ship consoles, etc.
void CHSShip::HandleSensors()
{
    CHSSysSensors *cSensors;
    SENSOR_CONTACT *cContact;
    char tbuf[512];
    char name[64];

    cSensors = (CHSSysSensors *) m_systems.GetSystem(HSS_SENSORS);
    if (!cSensors)
        return;

    // Set the sensors to the first new contact.
    cContact = cSensors->GetFirstNewContact();

    // This is ok to do because the Next and First functions
    // return the same first contact.
    while ((cContact = cSensors->GetNextNewContact()))
    {
        // Special name handling for ships.
        if (cContact->m_obj->GetType() == HST_SHIP)
            sprintf(name, "the %s", cContact->m_obj->GetName());
        else
            strcpy(name, cContact->m_obj->GetName());

        if (cContact->status == DETECTED)
            sprintf(tbuf,
                    "%s[%s%s%d%s%s]%s - Unidentified contact has appeared on sensors.",
                    cContact->m_obj->GetObjectColor(), ANSI_NORMAL,
                    ANSI_HILITE, cContact->m_id, ANSI_NORMAL,
                    cContact->m_obj->GetObjectColor(), ANSI_NORMAL);
        else if (cContact->status == UPDATED)
            sprintf(tbuf,
                    "%s[%s%s%d%s%s]%s - Contact identified as %s.",
                    cContact->m_obj->GetObjectColor(), ANSI_NORMAL,
                    ANSI_HILITE, cContact->m_id, ANSI_NORMAL,
                    cContact->m_obj->GetObjectColor(), ANSI_NORMAL, name);
        else
            sprintf(tbuf,
                    "%s[%s%s%d%s%s]%s - New contact identified as %s.",
                    cContact->m_obj->GetObjectColor(), ANSI_NORMAL,
                    ANSI_HILITE, cContact->m_id, ANSI_NORMAL,
                    cContact->m_obj->GetObjectColor(), ANSI_NORMAL, name);
        NotifyConsoles(tbuf, MSG_SENSOR);
    }

    // Now lost contacts
    // Set the sensors to the first lost contact.
    cContact = cSensors->GetFirstLostContact();

    // This is ok to do because the Next and First functions
    // return the same first contact.
    while ((cContact = cSensors->GetNextLostContact()))
    {
        // Special name handling for ships.
        if (cContact->m_obj->GetType() == HST_SHIP)
            sprintf(name, "The %s", cContact->m_obj->GetName());
        else
        {
            strncpy(name, cContact->m_obj->GetName(), 63);
            name[63] = '\0';
        }
        if (cContact->status == DETECTED)
        {
            sprintf(tbuf,
                    "%s[%s%s%d%s%s]%s - Unidentified contact has been lost from sensors.",
                    cContact->m_obj->GetObjectColor(), ANSI_NORMAL,
                    ANSI_HILITE, cContact->m_id, ANSI_NORMAL,
                    cContact->m_obj->GetObjectColor(), ANSI_NORMAL);
        }
        else
        {
            sprintf(tbuf,
                    "%s[%s%s%d%s%s]%s - %s has been lost from sensors.",
                    cContact->m_obj->GetObjectColor(), ANSI_NORMAL,
                    ANSI_HILITE, cContact->m_id, ANSI_NORMAL,
                    cContact->m_obj->GetObjectColor(), ANSI_NORMAL, name);
        }

        // Remove target locks if the ship is locked on the object that
        // has been lost from sensors
        if(true == IsObjectLocked(cContact->m_obj))
        {
            for (int idx = 0; idx < MAX_SHIP_CONSOLES; idx++)
            {
                if(NULL != m_console_array[idx])
                {
                    m_console_array[idx]->UnlockTarget(cContact->m_obj);
                }
            }

        }

        NotifyConsoles(tbuf, MSG_SENSOR);
    }
}

// Sends a message to all consoles on a ship
void CHSShip::NotifyConsoles(char *strMsg, int msgLevel)
{
    int idx;

    for (idx = 0; idx < MAX_SHIP_CONSOLES; idx++)
    {
        if (m_console_array[idx])
        {
            m_console_array[idx]->HandleMessage(strMsg, msgLevel);
        }
    }
}

// Sends a message to all registered ship rooms
void CHSShip::NotifySrooms(char *strMsg)
{
    CHSShipRoomSet::iterator iter;
    for (iter = m_setRooms.begin(); iter != m_setRooms.end(); iter++)
    {
        hsInterface.NotifyContents(*iter, strMsg);
    }
}

// Returns the color of the ship object
char *CHSShip::GetObjectColor()
{
    static char tbuf[32];

    sprintf(tbuf, "%s%s", ANSI_HILITE, ANSI_RED);
    return tbuf;
}


// Returns the CHSConsole object in a given slot in the
// ship's console array.
CHSConsole *CHSShip::GetConsole(HS_UINT32 slot)
{
    if ((slot < 0) || (slot >= MAX_SHIP_CONSOLES))
        return NULL;

    return m_console_array[slot];
}

// Returns true if any of the ship's console are locked
// on the given target object.
HS_BOOL8 CHSShip::IsObjectLocked(CHS3DObject * cObj)
{
    if (!cObj)
        return false;

    // Run through our consoles, checking their locks.
    int idx;
    CHS3DObject *cTarget;
    for (idx = 0; idx < MAX_SHIP_CONSOLES; idx++)
    {
        if (m_console_array[idx])
        {
            cTarget = m_console_array[idx]->GetObjectLock();
            if (cTarget && (cTarget->GetDbref() == cObj->GetDbref()))
                return true;
        }
    }
    return false;
}

// Gives a report of sensor contacts to a player.  An object 
// type can be specified to only display sensor contacts with
// a certain type of object class.  If this variable is not
// set, all types are displayed.
void CHSShip::GiveSensorReport(HS_DBREF player, HS_TYPE tType)
{
    CHSSysSensors *cSensors;
    SENSOR_CONTACT *cContact;
    int idx;
    HS_BOOL8 bHeaderGiven;
    char tbuf[512];
    double rangelist[64];
    char *bufflist[64];         // Used to sort output
    int uBuffIdx;
    static char strSizes[32] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    float dDistance;
    char dDistanceS[64], dType[64];
    double dX, dY, dZ;
    char dXYHeading[64], dZHeading[64], dSpeed[64];
    int iXYBearing, iZBearing;
    char sizesymbol;
    char lockchar, cloakchar;
    HS_BOOL8 bLinked;
    int numcontacts = 0;

    cSensors = (CHSSysSensors *) m_systems.GetSystem(HSS_SENSORS);
    if (!cSensors)
    {
        hsStdError(player, "This vessel has no sensor array.");
        return;
    }

    if (!cSensors->GetCurrentPower())
    {
        hsStdError(player, "Sensor array is not currently online.");
        return;
    }

    // No contacts on sensors?
    if (!(numcontacts = cSensors->NumContacts()))
    {
        if (tType == HST_NOTYPE)
            hsStdError(player, "No contacts currently on sensors.");
        else
            hsStdError(player,
                       "No contacts of that type currently on sensors.");
        return;
    }
    else if (tType != HST_NOTYPE &&
             !(numcontacts = cSensors->NumContacts(tType)))
    {
        hsStdError(player, "No contacts of that type currently on sensors.");
        return;
    }

    bufflist[0] = NULL;
    uBuffIdx = 0;

    // All looks good.  Give the report.
    bHeaderGiven = false;
    for (idx = 0; idx < HS_MAX_CONTACTS; idx++)
    {
        // Grab the contact
        cContact = cSensors->GetContact(idx);
        if (!cContact)
            continue;

        // Store a pointer to the object for ease of use.
        CHS3DObject *cObj;
        cObj = cContact->m_obj;

        // Be sure it's the proper type.
        if (tType != HST_NOTYPE)
        {
            if (cObj->GetType() != tType)
                continue;
        }

        // The object is of our type, now give the header.
        // We do it here because if tType is not of NOTYPE,
        // we need to query the CHS3DObject to know what color
        // it displays as.
        if (!bHeaderGiven)
        {
            if (tType != HST_NOTYPE)
                sprintf(tbuf, "%sSensor Contacts:%s %d", cObj->GetObjectColor(), ANSI_NORMAL, numcontacts);     //cSensors->NumContacts());
            else
                sprintf(tbuf, "%sContacts:%s %d", ANSI_HILITE, ANSI_NORMAL, numcontacts);       //cSensors->NumContacts());

            hsInterface.Notify(player, tbuf);
            sprintf(tbuf,
                    "%s%s-------------------------------------------------------------------------------%s",
                    ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
            hsInterface.Notify(player, tbuf);
            sprintf(tbuf,
                    "%sC   ####  Name                     Bearing Range      Heading Speed  Type      %s",
                    ANSI_YELLOW, ANSI_NORMAL);
            hsInterface.Notify(player, tbuf);
            sprintf(tbuf,
                    "%s-  ------ ------------------------ --- --- ---------- --- --- ------ ----------%s",
                    ANSI_BLUE, ANSI_NORMAL);
            hsInterface.Notify(player, tbuf);

            bHeaderGiven = true;
        }

        // Calculate a variety of information.
        dX = cObj->GetX();
        dY = cObj->GetY();
        dZ = cObj->GetZ();

        // Distance
        dDistance = Dist3D(m_x, m_y, m_z, dX, dY, dZ);
        if (dDistance < 1000000)
          sprintf(dDistanceS, "%.2f", dDistance);
        else
          if (dDistance > 999999999)
            sprintf(dDistanceS, "#########");
          else
            sprintf(dDistanceS, "%.0f", dDistance);


        // Bearing
        iXYBearing = XYAngle(m_x, m_y, dX, dY);
        iZBearing = ZAngle(m_x, m_y, m_z, dX, dY, dZ);

        // Symbol representing the size
        if (cObj->GetSize() >= 27)
            sizesymbol = '+';
        else
            sizesymbol = strSizes[cObj->GetSize() - 1];

        // Is it a ship, and is it locked on us?
        bLinked = false;
        if (cObj->GetType() == HST_SHIP)
        {
            CHSShip *source;

            source = (CHSShip *) cObj;
            if (source->IsObjectLocked(this))
                lockchar = '*';
            else
                lockchar = ' ';
            if (source->CloakingEffect() < 1)
                cloakchar = '~';
            else
                cloakchar = ' ';

            // Do we have a boarding link with the object?
            bLinked = false;
            for (unsigned int jdx = 0; jdx < m_hatches.size(); jdx++)
            {
                if (m_hatches[jdx] &&
                    (m_hatches[jdx]->TargetObject() == cObj->GetDbref()))
                {
                    bLinked = true;
                    break;
                }
            }
        }
        else
        {
            lockchar = ' ';
            cloakchar = ' ';
        }

        // Based on the type of object, given one of a variety
        // of print outs.  Not all objects may be supported, in
        // which case a generic report is given.
        bufflist[uBuffIdx] = new char[512];
        rangelist[uBuffIdx] = dDistance;

        CHSShip *cShip;
        HS_BOOL8 NoEngines, Jumping;
        Jumping = false;
        NoEngines = false;

        HS_BOOL8 WarpEngaged = false;
        HS_FLOAT32 WarpLevel = 0;

        if (cObj->GetType() == HST_SHIP)
        {
            cShip = (CHSShip *) cObj;
            CHSJumpDrive *cJumpers;
            CHSSysEngines *cEngines;
            CHSWarpDrive* cWarp;

            cEngines = (CHSSysEngines *)
                cShip->m_systems.GetSystem(HSS_ENGINES);
            if (!cEngines)
                NoEngines = true;
            cJumpers = (CHSJumpDrive *)
                cShip->m_systems.GetSystem(HSS_JUMP_DRIVE);
            if (cJumpers)
                if (cJumpers->GetEngaged())
                    Jumping = true;
            cWarp = (CHSWarpDrive *)
                cShip->m_systems.GetSystem(HSS_WARP_DRIVE);
            if(NULL != cWarp)
            {
                WarpEngaged = cWarp->GetEngaged();
                WarpLevel = cWarp->GetCurrentWarp();
            }
            sprintf(dXYHeading, "%d", cShip->GetXYHeading());
            sprintf(dZHeading, "%d", cShip->GetZHeading());
        }
        else if (cObj->GetType() == HST_MISSILE)
        {
            CHSMissile *pMissile = static_cast < CHSMissile * >(cObj);
            sprintf(dXYHeading, "%d", pMissile->GetXYHeading());
            sprintf(dZHeading, "%d", pMissile->GetZHeading());
        }
        else
        {
            cShip = NULL;
            sprintf(dXYHeading, "--");
            sprintf(dZHeading, "--");
        }

        sprintf(dSpeed, "%d", cObj->GetSpeed());

        std::string oName = cObj->GetTypeName();

        if (oName.size() > 0)
        {
            sprintf(dType, "%s", oName.c_str());
        }
        else                    // handle default information
        {

            switch (cObj->GetType())
            {
            case HST_SHIP:
                if (NoEngines)
                    sprintf(dType, "Base");
                else
                    sprintf(dType, "Ship");
                break;

            case HST_PLANET:
                sprintf(dType, "Planet");
                break;

            case HST_BLACKHOLE:
                sprintf(dType, "Black Hole");
                break;

            case HST_WORMHOLE:
                sprintf(dType, "Wormhole");
                break;

            case HST_MISSILE:
                sprintf(dType, "Missile");
                break;

            case HST_NEBULA:
                sprintf(dType, "Nebula");
                break;

            case HST_ASTEROID:
                sprintf(dType, "Asteroids");
                break;

            default:
                sprintf(dType, "Unknown");
                break;
            }
        }


        sprintf(bufflist[uBuffIdx],
                "%c%s%c%s%c%s[%s%d%s]%s%s%c%s%-24s %3d %3d %10s %3s %3s%c%-6s %-10s",
                sizesymbol,
                ANSI_HILITE, lockchar, ANSI_NORMAL,
                bLinked ? 'd' : ' ',
                cObj->GetObjectColor(), ANSI_NORMAL,
                cContact->m_id, cObj->GetObjectColor(),
                ANSI_HILITE, ANSI_BLUE, cloakchar, ANSI_NORMAL,
                cContact->status == DETECTED ? "Unknown" : cObj->GetName(),
                iXYBearing,
                iZBearing,
                dDistanceS,
                dXYHeading, dZHeading, Jumping ? 'J' : WarpEngaged ? 'W' : ' ',
                dSpeed, dType);


        uBuffIdx++;
    }

    // Now sort the contacts according to range.  This is your
    // pretty standard, very inefficient bubble-sort.
    int i, j;
    char *ptr;
    for (i = 0; i < uBuffIdx; i++)
    {
        for (j = i + 1; j < uBuffIdx; j++)
        {
            // Compare element ranges
            if (rangelist[i] > rangelist[j])
            {
                // Swap elements
                dDistance = rangelist[i];
                ptr = bufflist[i];

                rangelist[i] = rangelist[j];
                bufflist[i] = bufflist[j];

                bufflist[j] = ptr;
                rangelist[j] = dDistance;
            }
        }
    }

    // List sorted, now print it out to the player
    for (i = 0; i < uBuffIdx; i++)
    {
        hsInterface.Notify(player, bufflist[i]);

        // Free the buffer
        delete[]bufflist[i];
    }

    // Print the closing line
    sprintf(tbuf,
            "%s%s-------------------------------------------------------------------------------%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);
}

// Handles life support for the vessel, which basically
// just means checking to see if there's any life support
// left .. oh, and killing players.
void CHSShip::HandleLifeSupport()
{
    CHSSysLifeSupport *cLife;
    char tbuf[256];

    // Find the system
    cLife = (CHSSysLifeSupport *) m_systems.GetSystem(HSS_LIFE_SUPPORT);
    if (!cLife || !IsActive())
        return;

    // Is there air left?
    if (cLife->GetAirLeft() <= 0)
    {
        if (bGhosted)
        {
            // Do nothing
            return;
        }

        // Kill people on the ship.
        bGhosted = true;
        sprintf(tbuf,
                "%s%s%s*%s LIFE SUPPORT SYSTEMS FAILURE %s%s%s*%s",
                ANSI_HILITE, ANSI_BLINK, ANSI_RED, ANSI_NORMAL,
                ANSI_HILITE, ANSI_BLINK, ANSI_RED, ANSI_NORMAL);
        NotifySrooms(tbuf);
        if (!hsInterface.HasFlag(m_objnum, TYPE_THING, THING_HSPACE_SIM))
            KillShipCrew("YOU HAVE SUFFOCATED!");
    }
    else if (cLife->GetCurrentPower() == 0)
    {
        double remainder;
        int airleft;

        airleft = (int) (cLife->GetAirLeft() / 10);
        remainder = (cLife->GetAirLeft() * .1) - airleft;

        // Send out a warning each 10 percent drop in life support.
        // Because life support may be decremented each cycle by
        // something that doesn't always add up to 1%, we need to
        // account for some minor error.  For example, decreasing
        // 1% over 3 cycles is .3333 each cycle, for a total of
        // .9999% over 3 cycles.  That gives an error of .0001%,
        // so the remainder will never be exactly 0.
        if (remainder < .001)
        {
            sprintf(tbuf,
                    "%s%s%s*%s %s%sWARNING%s %s%s%s*%s Life support systems are failing.",
                    ANSI_HILITE, ANSI_BLINK, ANSI_YELLOW, ANSI_NORMAL,
                    ANSI_HILITE, ANSI_YELLOW, ANSI_NORMAL,
                    ANSI_HILITE, ANSI_BLINK, ANSI_YELLOW, ANSI_NORMAL);
            NotifySrooms(tbuf);
        }
    }
    else if (bGhosted)
        bGhosted = false;
}

// Returns the maximum hull points for the vessel
//! @todo hull points should be overrideable on the ship
HS_UINT32 CHSShip::GetMaxHullPoints()
{
    if (NULL != m_maxhull)
    {
        return *m_maxhull;
    }

    if (NULL != m_classinfo)
    {
        return m_classinfo->MaxHull();
    }

    return 1;
}

// Returns the current hull points for the vessel
HS_UINT32 CHSShip::GetHullPoints()
{
    return m_hull_points;
}

// Returns the drop capable status of the ship
HS_BOOL8 CHSShip::CanDrop()
{
    if (NULL != m_can_drop)
    {
        return *m_can_drop;
    }

    if (NULL != m_classinfo)
    {
        return m_classinfo->CanDrop();
    }

    return 0;
}

HS_BOOL8 CHSShip::IsSpacedock()
{
    if (NULL != m_spacedock)
    {
        return *m_spacedock;
    }

    if (NULL != m_classinfo)
    {
        return m_classinfo->SpaceDock();
    }

    return 0;
}

// Ships are represented as a plus sign.
char CHSShip::GetObjectCharacter()
{
    return '+';
}


// Retrieves a landing location (a bay) given a slot number (0 - n)
CHSHatch *CHSShip::GetHatch(int which)
{
    if (which < 0 || which >= (int) m_hatches.size())
        return NULL;

    return m_hatches.at(which);
}


// Moves the ship object, which represents the ship, to a location.
void CHSShip::MoveShipObject(HS_DBREF whereto)
{
    if (!hsInterface.ValidObject(whereto))
        return;

    if (m_objnum != HSNOTHING)
        hsInterface.MoveObject(m_objnum, whereto);
}

CHSHatch *CHSShip::NewHatch()
{
    CHSHatch* hatch = new CHSHatch();

    if(NULL != hatch)
    {
        m_hatches.push_back(hatch);
        return hatch;
    }

    
    return NULL;
}


CHSHatch *CHSShip::FindHatch(HS_DBREF objnum)
{
    for (unsigned int idx = 0; idx < m_hatches.size(); idx++)
    {
        if (m_hatches.at(idx)->Object() == objnum)
            return m_hatches.at(idx);
    }
    return NULL;
}

HS_UINT32 CHSShip::GetSize()
{
    return m_size;
}

// Takes a message and propogates it to the consoles.  If the
// cObj variable is not NULL, the ship will attempt to find
// the sensor contact for that object and prefix the message
// with a contact id.  The long pointer points to whatever
// data were passed to the function.
void CHSShip::HandleMessage(const HS_INT8 * lpstrMsg,
                            HS_INT32 msgType, long *data)
{
    SENSOR_CONTACT *cContact;
    CHSSysSensors *cSensors;
    char tbuf[512];

    if (!lpstrMsg)
        return;

    // Determine message type
    switch (msgType)
    {
    case MSG_SENSOR:
    case MSG_GENERAL:
    case MSG_ENGINEERING:
    case MSG_COMBAT_DAMAGE:
    case MSG_COMBAT:
        {
            CHS3DObject *pFromObject = (CHS3DObject *) data;


            // Find our sensors
            cSensors = (CHSSysSensors *) m_systems.GetSystem(HSS_SENSORS);
            if (!cSensors || !pFromObject)
                cContact = NULL;
            else
                cContact = cSensors->GetContact(pFromObject);

            if (cContact)
            {
                sprintf(tbuf, "%s[%s%s%4d%s%s]%s - %s",
                        pFromObject->GetObjectColor(), ANSI_NORMAL,
                        ANSI_HILITE, cContact->m_id, ANSI_NORMAL,
                        pFromObject->GetObjectColor(), ANSI_NORMAL, lpstrMsg);
            }
            else
                strcpy(tbuf, lpstrMsg);

            NotifyConsoles(tbuf, msgType);

            if (MSG_COMBAT_DAMAGE == msgType)
            {
                NotifySrooms(tbuf);
            }
        }
        break;

    case MSG_COMMUNICATION:
        HandleCommMsg(lpstrMsg, data);
        break;
    }
}

// Runs through the boarding links, confirming their
// validity.
void CHSShip::ConfirmHatches()
{
    unsigned int idx;
    HS_BOOL8 bBreak;
    CHSShip *cShip;
    double sX, sY, sZ;          // Our coords
    double tX, tY, tZ;          // Other ship's coords
    double dDist;

    sX = GetX();
    sY = GetY();
    sZ = GetZ();

    // Run through boarding links, checking conditions.
    for (idx = 0; idx < m_hatches.size(); idx++)
    {
        cShip = (CHSShip *) dbHSDB.FindObject(m_hatches[idx]->TargetObject());

        if (!cShip)
            continue;

        bBreak = false;         // Don't break link by default.

        tX = cShip->GetX();
        tY = cShip->GetY();
        tZ = cShip->GetZ();

        // Check to see if the other ship is destroyed
        if (cShip->IsDestroyed())
            bBreak = true;

        // Check to see if we're still in space
        else if (!IsActive())
            bBreak = true;

        // Are we dropping or docking?
        else if (m_drop_status || m_dock_status)
            bBreak = true;

        // Check to see if the other guy is still
        // in space.
        else if (!cShip->IsActive())
            bBreak = true;

        // Check distance
        else
        {
            dDist = Dist3D(sX, sY, sZ, tX, tY, tZ);
            if (dDist > HSCONF.max_board_dist)
                bBreak = true;
        }


        if (bBreak)
        {
            CHSHatch *lHatch;
            lHatch = m_hatches[idx];

            int port;
            port = lHatch->TargetHatch();

            CHSHatch *cHatch;
            cHatch = cShip->GetHatch(port);
            if (cHatch)
            {
                cHatch->TargetObject(HSNOTHING);
                cHatch->TargetHatch(HSNOTHING);
                cHatch->HandleMessage(hsInterface.
                                      HSPrintf("%s is disconnected.",
                                               hsInterface.GetName(cHatch->
                                                                   Object())),
                                      MSG_GENERAL);
                hsInterface.UnlinkExits(cHatch->Object(), lHatch->Object());
            }
            else
            {
                hsInterface.MoveObject(lHatch->Object(), HSNOTHING);
            }

            lHatch->TargetObject(HSNOTHING);
            lHatch->TargetHatch(HSNOTHING);
            lHatch->HandleMessage(hsInterface.HSPrintf("%s is disconnected.",
                                                       hsInterface.
                                                       GetName(lHatch->
                                                               Object())),
                                  MSG_GENERAL);

            char tbuf[256];

            sprintf(tbuf, "Docking couplings on hatch %d disengaged.",
                    port + 1);
            cShip->NotifyConsoles(tbuf, MSG_GENERAL);

            NotifyConsoles(hsInterface.
                           HSPrintf
                           ("Docking couplings on hatch %d disengaged.",
                            idx + 1), MSG_GENERAL);
        }
    }
}

// Indicates if the ship is destroyed or not.
HS_BOOL8 CHSShip::IsDestroyed()
{
    return m_destroyed;
}

// Allows a player to disembark from the ship.  This could
// be if the ship is docked, dropped, or boardlinked.
void CHSShip::DisembarkPlayer(HS_DBREF player, int id)
{
    HS_DBREF dbLoc = HSNOTHING;
    HS_DBREF dbDestObj = HSNOTHING;

    // If the ship is docked or dropped, find the location
    // of the ship object, and put the player there.
    if (m_docked)
    {
        if (m_objnum == HSNOTHING)
        {
            hsInterface.Notify(player,
                               "This ship has no ship object, so I don't know where to put you.");
            return;
        }
        dbLoc = hsInterface.GetLocation(m_objnum);

        dbDestObj = m_objlocation;
    }
    if (dbLoc == HSNOTHING)
    {
        hsInterface.Notify(player,
                           "You cannot disembark from the vessel at this time.");
        return;
    }
    // At this point, dbLoc should be set to a
    // location.
    if (!hsInterface.ValidObject(dbLoc))
    {
        hsInterface.Notify(player,
                           "Erg.  The location of this vessel's ship object is invalid.");
        return;
    }

    // Tell the player of the disembarking
    hsInterface.Notify(player, "You disembark from the vessel.");

    // Tell players in the other location of the disembarking.
    hsInterface.NotifyContents(dbLoc,
                               hsInterface.
                               HSPrintf("%s disembarks from the %s.",
                                        hsInterface.GetName(player),
                                        GetName()));

    HS_DBREF dbPrevLoc = hsInterface.GetLocation(player);

    // Move the player
    hsInterface.MoveObject(player, dbLoc);

    hsInterface.InvokeResponse(player, m_objnum, NULL, NULL, "ADISEMBARK",
                               dbPrevLoc);

    // Tell players in the previous location that the
    // player disembarked.
    hsInterface.NotifyContents(dbPrevLoc,
                               hsInterface.
                               HSPrintf("%s disembarks from the vessel.",
                                        hsInterface.GetName(player)));

    // Set the location attribute on the player.
    hsInterface.AtrAdd(player, "HSPACE_LOCATION",
                       hsInterface.HSPrintf("#%d", dbDestObj),
                       hsInterface.GetGodDbref(), AF_MDARK | AF_WIZARD);
}

// Returns the boarding code for the vessel.
char *CHSShip::GetBoardingCode()
{
    return m_boarding_code;
}

// Returns true or false to indicate whether the ship is
// landed or docked.
HS_BOOL8 CHSShip::Landed()
{
    if (m_docked && !m_in_space)
        return true;

    return false;
}

// When another object locks onto the ship, this function
// can be called to handle that situation.
void CHSShip::HandleLock(CHS3DObject * cLocker, HS_BOOL8 bStat)
{
    SENSOR_CONTACT *cContact;
    CHSSysSensors *cSensors;
    char tbuf[256];

    // bStat could be true for locking, or false
    // for unlocking.

    // Find our sensors
    cSensors = (CHSSysSensors *) m_systems.GetSystem(HSS_SENSORS);
    if (!cSensors)
        return;

    // Find the contact on sensors
    cContact = cSensors->GetContact(cLocker);

    // If not on sensors, give a general message
    if (!cContact)
    {
        if (bStat)
            sprintf(tbuf,
                    "%s%s-%s An unknown contact has locked weapons!",
                    ANSI_HILITE, ANSI_YELLOW, ANSI_NORMAL);
        else
            sprintf(tbuf,
                    "%s%s-%s An unknown contact has released weapons lock.",
                    ANSI_HILITE, ANSI_YELLOW, ANSI_NORMAL);
    }
    else if (cContact->status == DETECTED)
    {
        if (bStat)
            sprintf(tbuf,
                    "%s[%s%s%d%s%s]%s - Contact has locked weapons!",
                    cLocker->GetObjectColor(), ANSI_NORMAL,
                    ANSI_HILITE, cContact->m_id, ANSI_NORMAL,
                    cLocker->GetObjectColor(), ANSI_NORMAL);
        else
            sprintf(tbuf,
                    "%s[%s%s%d%s%s]%s - Contact has released weapons lock.",
                    cLocker->GetObjectColor(), ANSI_NORMAL,
                    ANSI_HILITE, cContact->m_id, ANSI_NORMAL,
                    cLocker->GetObjectColor(), ANSI_NORMAL);
    }
    else
    {
        char name[64];
        if (cLocker->GetType() == HST_SHIP)
            sprintf(name, "The %s", cLocker->GetName());
        else
            strcpy(name, cLocker->GetName());

        if (bStat)
            sprintf(tbuf,
                    "%s[%s%s%d%s%s]%s - %s has locked weapons!",
                    cLocker->GetObjectColor(), ANSI_NORMAL,
                    ANSI_HILITE, cContact->m_id, ANSI_NORMAL,
                    cLocker->GetObjectColor(), ANSI_NORMAL, name);
        else
            sprintf(tbuf,
                    "%s[%s%s%d%s%s]%s - %s has released weapons lock.",
                    cLocker->GetObjectColor(), ANSI_NORMAL,
                    ANSI_HILITE, cContact->m_id, ANSI_NORMAL,
                    cLocker->GetObjectColor(), ANSI_NORMAL, name);
    }

    // Notify the consoles of this message
    NotifyConsoles(tbuf, MSG_COMBAT);
}

// Handles damage from an attacker, with a weapon, and a given
// level of damage.
void CHSShip::HandleDamage(CHS3DObject * cSource,
                           CHSWeaponData * pWeaponData,
                           int strength, CHSConsole * cConsole, int iSysType)
{
    HS_DBREF dbAttacker;
    CHSSysShield *cShield;
    char msgUs[256];            // Message gets sent to our ship
    char msgThem[256];          // Message gets sent back to attacker

    // Do nothing if the ship is already destroyed.
    if (IsDestroyed())
    {
        return;
    }

    // Get the attacking player.
    dbAttacker = hsInterface.ConsoleUser(cConsole->m_objnum);

    // Determine which shield is hit.  To do that, we
    // supply the XY and Z angles from the attacker to
    // us.
    int iXYAngle;
    int iZAngle;

    iXYAngle = XYAngle(cSource->GetX(), cSource->GetY(), GetX(), GetY());

    iZAngle = ZAngle(cSource->GetX(), cSource->GetY(), cSource->GetZ(),
                     GetX(), GetY(), GetZ());

    cShield = DetermineHitShield(iXYAngle, iZAngle);

    // It's possible the ship has no shields
    char strShieldType[32];
    if (cShield)
    {
        // Tell the shield to take damage, and record
        // how much damage was not handled.
        strength = cShield->DoDamage(strength);
        int shieldtype = cShield->GetShieldType();

        if (shieldtype == ST_DEFLECTOR)
            strcpy(strShieldType, "deflected");
        else
            strcpy(strShieldType, "absorbed");
    }
    if (cShield && strength <= 0)
    {
        // Shields absorbed damage

        // Customize message for type of weapon.
        switch (pWeaponData->WeaponClass())
        {
        case WC_LASER:
            sprintf(msgUs,
                    "%s have %s an incoming energy blast.",
                    cShield->GetName(), strShieldType);
            sprintf(msgThem,
                    "Your energy blast has been %s by the enemy's shields.",
                    strShieldType);
            break;

        case WC_MISSILE:
            sprintf(msgUs,
                    "Our shields have %s the impact from an inbound missile.",
                    strShieldType);
            sprintf(msgThem,
                    "The impact of your missile has been %s by the enemy's shields.",
                    strShieldType);
            break;
        }

        // Give messages
        HandleMessage(msgUs, MSG_COMBAT, (long *) cSource);

        if (dbAttacker != HSNOTHING)
            hsStdError(dbAttacker, msgThem);
    }
    else
    {
        // Some damage to the hull occurs.  If it's greater
        // than a critical hit value, then systems take
        // damage.

        HS_BOOL8 HitSys;

        // Deduct from hull
        if (pWeaponData->WeaponClass() == WC_LASER)
        {
            if (!static_cast < CHSLaserData * >(pWeaponData)->NoHull())
            {
                m_hull_points -= strength;
            }
        }
        else
        {
            m_hull_points -= strength;
        }

        double CritPerc;

        if (pWeaponData->WeaponClass() == WC_LASER)
        {
            if (!static_cast < CHSLaserData * >(pWeaponData)->NoHull())
            {
                CritPerc = .2 /
                    static_cast < CHSLaserData * >(pWeaponData)->Accuracy();
                HitSys = hsInterface.GetRandom(2) == 0 ? false : true;
            }
            else
            {
                CritPerc = .1;
                HitSys = 1;
            }
        }
        else
        {
            CritPerc = .1;
            HitSys = 1;
        }


        // Is hull < 0?
        if (m_hull_points <= 0)
        {
            ExplodeMe();
            if (!hsInterface.HasFlag(m_objnum, TYPE_THING, THING_HSPACE_SIM))
                KillShipCrew("YOU HAVE BEEN BLOWN TO TINY BITS!");

            // Trigger akill
            hsInterface.InvokeResponse(dbAttacker, m_objnum, NULL, NULL,
                                       "AKILL");
        }
        else
        {
            // Was the hit strong enough to do critical
            // damage?  Critical damage is defined as
            // 1/10th the hull value.
            double dCritStrength;
            dCritStrength = m_hull_points * CritPerc;

            if (strength >= dCritStrength && HitSys)
            {
                // Damage a system.
                CHSEngSystem *cSys;

                // Try to get the system of choice
                if (iSysType != HSS_NOTYPE)
                {
                    cSys = m_systems.GetSystem((HSS_TYPE) iSysType);

                    // If the system wasn't found, grab a
                    // random one.
                    if (!cSys)
                        cSys = m_systems.GetRandomSystem();
                }
                else
                    cSys = m_systems.GetRandomSystem();
                if (!cSys)
                {
                    // Odd.  No system found?  Just give
                    // a hull-like message.
                    hsInterface.Notify(1, "NO SYSTEM");
                    switch (pWeaponData->WeaponClass())
                    {
                    case WC_LASER:
                        if (static_cast <
                            CHSLaserData * >(pWeaponData)->NoHull())
                        {
                            strcpy(msgUs,
                                   "An incoming energy blast has not damaged any systems.");
                            strcpy(msgThem,
                                   "Your energy blast has not damaged any systems.");
                        }
                        else
                        {
                            strcpy(msgUs,
                                   "An incoming energy blast has damaged the hull.");
                            strcpy(msgThem,
                                   "Your energy blast has landed damage to the enemy's hull.");
                        }
                        break;

                    case WC_MISSILE:
                        strcpy(msgUs,
                               "An inbound missile has landed damage to our hull.");
                        strcpy(msgThem,
                               "Your missile has landed damage to the enemy's hull.");
                        break;
                    }
                }
                else
                {
                    // Give a message
                    switch (pWeaponData->WeaponClass())
                    {
                    case WC_LASER:
                        sprintf(msgUs,
                                "An incoming energy blast has damaged the %s.",
                                cSys->GetName());
                        sprintf(msgThem,
                                "Your energy blast has landed damage to the enemy's %s.",
                                cSys->GetName());
                        break;

                    case WC_MISSILE:
                        sprintf(msgUs,
                                "An inbound missile has landed damage to our %s.",
                                cSys->GetName());
                        sprintf(msgThem,
                                "Your missile has landed damage to the enemy's %s.",
                                cSys->GetName());
                        break;
                    }

                    // Damage the system
                    cSys->DoDamage();
                }

                HandleMessage(msgUs, MSG_COMBAT_DAMAGE, (long *) cSource);

                if (dbAttacker != HSNOTHING)
                    hsStdError(dbAttacker, msgThem);
            }
            else
            {
                // Just hull damage
                switch (pWeaponData->WeaponClass())
                {
                case WC_LASER:
                    if (static_cast < CHSLaserData * >(pWeaponData)->NoHull())
                    {
                        strcpy(msgUs,
                               "An incoming energy blast has not damaged any systems.");
                        strcpy(msgThem,
                               "Your energy blast has not damaged any systems.");
                    }
                    else
                    {
                        strcpy(msgUs,
                               "An incoming energy blast has damaged the hull.");
                        strcpy(msgThem,
                               "Your energy blast has landed damage to the enemy's hull.");
                    }
                    break;

                case WC_MISSILE:
                    strcpy(msgUs,
                           "An inbound missile has landed damage to our hull.");
                    strcpy(msgThem,
                           "Your missile has landed damage to the enemy's hull.");
                    break;
                }

                HandleMessage(msgUs, MSG_COMBAT_DAMAGE, (long *) cSource);

                if (dbAttacker != HSNOTHING)
                    hsStdError(dbAttacker, msgThem);
            }
        }
    }
}

void CHSShip::HandleDamage(HS_FLOAT64 x, HS_FLOAT64 y, HS_FLOAT64 z,
                           HS_INT32 strength)
{
    CHSSysShield *cShield;
    char msgUs[256];            // Message gets sent to our ship

    // Do nothing if the ship is already destroyed.
    if (IsDestroyed())
    {
        return;
    }

    // Determine which shield is hit.  To do that, we
    // supply the XY and Z angles from the attacker to
    // us.
    int iXYAngle;
    int iZAngle;

    iXYAngle = XYAngle(x, y, GetX(), GetY());

    iZAngle = ZAngle(x, y, z, GetX(), GetY(), GetZ());

    cShield = DetermineHitShield(iXYAngle, iZAngle);

    // It's possible the ship has no shields
    char strShieldType[32];
    if (cShield)
    {
        // Tell the shield to take damage, and record
        // how much damage was not handled.
        strength = cShield->DoDamage(strength);
        int shieldtype = cShield->GetShieldType();

        if (shieldtype == ST_DEFLECTOR)
            strcpy(strShieldType, "deflected");
        else
            strcpy(strShieldType, "absorbed");
    }

    // does damage get past shields?
    if (cShield && strength <= 0)
    {
        sprintf(msgUs, "%s have %s an explosive blast.",
                cShield->GetName(), strShieldType);
        HandleMessage(msgUs, MSG_COMBAT, NULL);
    }
    else if (strength > 0)      // some damage penetrated the shields
    {
        m_hull_points -= strength;

        // Check for ship death
        if (m_hull_points <= 0)
        {
            ExplodeMe();
            if (!hsInterface.HasFlag(m_objnum, TYPE_THING, THING_HSPACE_SIM))
                KillShipCrew("YOU HAVE BEEN BLOWN TO TINY BITS!");

            // Trigger akill
            hsInterface.InvokeResponse(m_objnum, m_objnum, NULL, NULL,
                                       "AKILL");
        }
        else
        {
            // Should handle random system damage here....
            if (strength > (m_hull_points * 0.10))
            {
                CHSEngSystem *cSys = m_systems.GetRandomSystem();
                if (cSys)
                {
                    sprintf(msgUs, "A explosive blast damages the %s.",
                            cSys->GetName());
                    cSys->DoDamage();
                }
            }
            else
            {
                // Default message for non-damaged system
                strcpy(msgUs,
                       "An explosive blast has damaged the ship's hull.");
            }
            HandleMessage(msgUs, MSG_COMBAT, NULL);
        }
    }

}

// Can be used to explode the ship, for whatever reason.
void CHSShip::ExplodeMe()
{
    CHSUniverse *uSource;
    char tbuf[128];

    // Find my universe
    uSource = GetUniverse();
    if (uSource)
    {
        // Give out a message
        sprintf(tbuf,
                "You gaze in awe as the %s explodes before your eyes.",
                GetName());

        uSource->SendContactMessage(tbuf, IDENTIFIED, this);

        // Remove me from active space.
        uSource->RemoveActiveObject(this);

        m_in_space = false;
    }

    // I'm destroyed
    m_destroyed = true;

    // Clear any weapons locks consoles may have
    int idx;
    for (idx = 0; idx < MAX_SHIP_CONSOLES; idx++)
    {
        if (m_console_array[idx] && m_console_array[idx]->GetObjectLock())
        {
            m_console_array[idx]->UnlockWeapons(HSNOTHING);
            m_console_array[idx]->PowerDown(HSNOTHING);
        }
    }
}

// Can be used to resurrect the ship, which just means
// it's no longer destroyed and is now in active space
void CHSShip::ResurrectMe()
{
    CHSUniverse *uDest;

    m_destroyed = false;

    // Put us into our universe, or a default one if
    // needed.
    uDest = GetUniverse();
    if (!uDest)
    {
        // Find the first available universe
        THSUniverseIterator tIter;

        if (!CHSUniverseDB::GetInstance().GetFirstUniverse(tIter))
        {
            return;
        }

        // Since we had to find this universe, that also
        // means this ship is not in that universe, so add
        // it.
        uDest->AddObject(this);
    }

    // A universe exists, so add it.
    uDest->AddActiveObject(this);

    m_in_space = true;

    // Be sure to verify the UID
    m_uid = uDest->GetID();
}

// Called by another object to give a scan report.  We
// decide what information to include in the scan.
void CHSShip::GiveScanReport(CHS3DObject * cScanner,
                             HS_DBREF player, HS_BOOL8 id)
{
    char tbuf[256];
    CHSSysEngines *cEngines;

    // Find our engines for speed info
    cEngines = (CHSSysEngines *) m_systems.GetSystem(HSS_ENGINES);

    // Give the header info
    sprintf(tbuf,
            "%s%s.-----------------------------------------------------.%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);
    char tbuf2[256];
    if (id)
        sprintf(tbuf2, "%s(%s)", GetName(), m_ident ? m_ident : "--");
    else
        sprintf(tbuf2, "Unknown");
    sprintf(tbuf,
            "%s%s|%s Vessel Scan Report %31s  %s%s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
            tbuf2, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);
    sprintf(tbuf,
            "%s%s >---------------------------------------------------<%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    // Give the vessel class
    sprintf(tbuf,
            "%s%s|%s %-40s            %s%s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
            id ? m_classinfo->ClassName() : "Vessel Class Unknown",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    // Give coordinate and heading information.
    sprintf(tbuf,
            "%s%s| %sX:%s %9.0f%23s%s%sSize:%s %-3d        %s%s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
            GetX(), " ",
            ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
            GetSize(), ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    sprintf(tbuf,
            "%s%s| %sY:%s %9.0f%23s%s%sHeading:%s %3d/%-3d %s%s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
            GetY(), " ",
            ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
            m_current_xyheading, m_current_zheading,
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    sprintf(tbuf,
            "%s%s| %sZ:%s %9.0f%23s%s%sVelocity:%s %-6.0f %s%s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
            GetZ(), " ",
            ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
            cEngines ? cEngines->GetCurrentSpeed() : 0.0,
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    // If we're identified, give systems information
    if (id)
    {
        CHSEngSystem *cSys;
        char strDamage[64];
        CHSSysShield *cFore;
        CHSSysShield *cAft;
        CHSSysShield *cStar;
        CHSSysShield *cPort;

        sprintf(tbuf,
                "%s%s >---------------------------------------------------<%s",
                ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
        hsInterface.Notify(player, tbuf);

        // Find the shields
        cFore = (CHSSysShield *) m_systems.GetSystem(HSS_FORE_SHIELD);
        cAft = (CHSSysShield *) m_systems.GetSystem(HSS_AFT_SHIELD);
        cStar = (CHSSysShield *) m_systems.GetSystem(HSS_STARBOARD_SHIELD);
        cPort = (CHSSysShield *) m_systems.GetSystem(HSS_PORT_SHIELD);

        // Give hull, shield info
        sprintf(tbuf,
                "%s%s| %sHull Status               Shield Status             %s|%s",
                ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_BLUE, ANSI_NORMAL);
        hsInterface.Notify(player, tbuf);

        char strFore[8];        // Shield status strings
        char strAft[8];
        char strStar[8];
        char strPort[8];

        if (cFore)
        {
            sprintf(strFore, "%.0f%%", cFore->GetShieldPerc());
        }
        else
            strcpy(strFore, "N/A");

        if (cAft)
        {
            sprintf(strAft, "%.0f%%", cAft->GetShieldPerc());
        }
        else
            strcpy(strAft, "N/A");

        if (cPort)
        {
            sprintf(strPort, "%.0f%%", cPort->GetShieldPerc());
        }
        else
            strcpy(strPort, "N/A");

        if (cStar)
        {
            sprintf(strStar, "%.0f%%", cStar->GetShieldPerc());
        }
        else
            strcpy(strStar, "N/A");

        int hullperc;
        hullperc = (int) (100 * GetHullPoints() / (float) GetMaxHullPoints());
        sprintf(tbuf,
                "%s%s|%s    %3d%%       %s%sF:%s %-4s  %s%sA:%s %-4s  %s%sP:%s %-4s  %s%sS:%s %-4s    %s%s|%s",
                ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
                hullperc,
                ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
                strFore,
                ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
                strAft,
                ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, strPort,
                ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
                strStar, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
        hsInterface.Notify(player, tbuf);


        // print systems header
        sprintf(tbuf,
                "%s%s|%53s|%s", ANSI_HILITE, ANSI_BLUE, " ", ANSI_NORMAL);
        hsInterface.Notify(player, tbuf);

        sprintf(tbuf,
                "%s%s| %sSystem Name          Status       Damage    %8s%s|%s",
                ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, " ", ANSI_BLUE,
                ANSI_NORMAL);
        hsInterface.Notify(player, tbuf);

        // Run down the list of systems, giving info for
        // visible ones (or reactor).
        for (cSys = m_systems.GetHead(); cSys; cSys = cSys->GetNext())
        {
            if (cSys->IsVisible() || cSys->GetType() == HSS_REACTOR)
            {
                // Setup damage indicator
                switch (cSys->GetDamageLevel())
                {
                case DMG_LIGHT:
                    sprintf(strDamage,
                            "%s%s  LIGHT   %s",
                            ANSI_HILITE, ANSI_BGREEN, ANSI_NORMAL);
                    break;

                case DMG_MEDIUM:
                    sprintf(strDamage,
                            "%s%s  MEDIUM  %s",
                            ANSI_HILITE, ANSI_BYELLOW, ANSI_NORMAL);
                    break;

                case DMG_HEAVY:
                    sprintf(strDamage,
                            "%s%s  HEAVY   %s",
                            ANSI_HILITE, ANSI_BRED, ANSI_NORMAL);
                    break;

                case DMG_INOPERABLE:
                    sprintf(strDamage,
                            "%s%sINOPERABLE%s",
                            ANSI_HILITE, ANSI_BBLACK, ANSI_NORMAL);
                    break;

                default:
                    strcpy(strDamage, "None   ");
                }
                sprintf(tbuf,
                        "%s%s|%s %-20s %-12s %-10s        %s%s|%s",
                        ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
                        cSys->GetName(),
                        cSys->GetStatus(),
                        strDamage, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
                hsInterface.Notify(player, tbuf);
            }
        }

        // Do we have any landing locs to report?
        if (GetNumVisibleLandingLocs() > 0)
        {
            // Print em baby.
            hsInterface.Notify(player,
                               hsInterface.HSPrintf("%s%s|%53s|%s",
                                                    ANSI_HILITE, ANSI_BLUE,
                                                    " ", ANSI_NORMAL));
            sprintf(tbuf, "%s%s| %sLanding Locations:%s %-2d%31s%s%s|%s",
                    ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
                    m_listLandingLocs.size(), " ", ANSI_HILITE, ANSI_BLUE,
                    ANSI_NORMAL);
            hsInterface.Notify(player, tbuf);

            sprintf(tbuf,
                    "%s%s| %s[%s##%s] Name                          Doors   Code     %s|%s",
                    ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_WHITE,
                    ANSI_GREEN, ANSI_BLUE, ANSI_NORMAL);
            hsInterface.Notify(player, tbuf);

            char strPadName[32];
            CSTLLandingLocList::iterator iterLocs;
            HS_UINT32 idx = 0;
            for (iterLocs = m_listLandingLocs.begin();
                 iterLocs != m_listLandingLocs.end(); iterLocs++)
            {
                CHSLandingLoc *pLoc = *iterLocs;

                // Not visible?  Don't print it.
                if (!pLoc->IsVisible())
                {
                    continue;
                }

                strncpy(strPadName, hsInterface.GetName(pLoc->Object()), 32);
                strPadName[23] = '\0';
                sprintf(tbuf,
                        "%s%s|%s  %2d  %-29s %-6s  %3s      %s%s|%s",
                        ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
                        idx + 1,
                        strPadName,
                        pLoc->IsActive()? "Open" : "Closed",
                        pLoc->CodeRequired()? "Yes" : "No",
                        ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
                hsInterface.Notify(player, tbuf);

                idx++;
            }
        }

    }



    // Finish the report
    sprintf(tbuf,
            "%s%s`-----------------------------------------------------'%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    // Do we hsInterface.Notify consoles that we're being scanned?
    if (HSCONF.notify_shipscan)
    {
        CHSSysSensors *cSensors;

        cSensors = (CHSSysSensors *) m_systems.GetSystem(HSS_SENSORS);
        if (cSensors && cSensors->GetCurrentPower() > 0)
            HandleMessage("We are being scanned by another contact.",
                          MSG_SENSOR, (long *) cScanner);
    }


}

// Sets the number of a specific type of missile
// in the missile storage bay.
HS_BOOL8 CHSShip::SetNumMissiles(HS_DBREF player, int type, int max)
{
    // Valid type of weapon?
    CHSWeaponData *pData;
    if (!(pData = waWeapons.GetWeapon(type)))
    {
        hsInterface.Notify(player, "Invalid weapon type specified.");
        return false;
    }

    if (pData->WeaponClass() != WC_MISSILE)
    {
        hsInterface.Notify(player, "That weapon is not a missile!");
        return false;
    }

    if (max > 0)
    {
        if (!m_missile_bay.SetRemaining(type, max))
        {
            hsInterface.Notify(player,
                               "Failed to set storage value for that weapon type.");
            return false;
        }
        else
            hsInterface.Notify(player, "Missile storage value - set.");
    }
    else
    {
        if (m_missile_bay.RemoveMissileType(type))
        {
            hsInterface.Notify(player, "Missile type removed.");
        }
        else
        {
            hsInterface.Notify(player,
                               "Missile type not found.  Remove failed.");
        }
    }

    return true;
}

// Retrieves the missile bay for the ship, or NULL if none.
CHSMissileBay *CHSShip::GetMissileBay()
{
    return &m_missile_bay;
}

// Runs through the registered rooms on the ship, moving the
// contents to the afterworld.  The ship is dead.  The msg
// is what is shown to the player/object when it is killed.
void CHSShip::KillShipCrew(const char *msg)
{
    CHSShipRoomSet::iterator iter;
    for (iter = m_setRooms.begin(); iter != m_setRooms.end(); iter++)
    {
        CSTLDbrefList listContents;
        HS_DBREF dbRoom = *iter;

        hsInterface.GetContents(dbRoom, listContents, NOTYPE);

        while (!listContents.empty())
        {
            HS_DBREF dbContent = listContents.front();
            listContents.pop_front();

            if (hsInterface.GetType(dbContent) != TYPE_PLAYER)
            {
                continue;
            }

#ifdef PENNMUSH
            hsInterface.BroadcastWithFlags(0, PLAYER_HSPACE_ADMIN,
                                           "HSPACE: %s(#%i) has died.",
                                           hsInterface.GetName(dbContent),
                                           dbContent);

#else
            hsInterface.BroadcastWithFlags(0, WIZARD,
                                           "HSPACE: %s(#%i) has died.",
                                           hsInterface.GetName(dbContent),
                                           dbContent);
#endif
            hsInterface.Notify(dbContent, msg);
            hsInterface.MoveObject(dbContent, HSCONF.afterworld);
        }
    }
}

// Refuels the ship
void CHSShip::Refuel()
{
    CHSFuelSystem *cFuel;

    cFuel = (CHSFuelSystem *) m_systems.GetSystem(HSS_FUEL_SYSTEM);

    if (cFuel)
    {
        cFuel->Refuel();
    }
}

// Clones the ship, including all registered rooms and objects.
// It's handy!
HS_DBREF CHSShip::Clone()
{
    std::map < HS_DBREF, HS_DBREF > mapVisitedRooms;    // A map of already cloned rooms and their counterparts.

    // Clone the ship object.
    HS_DBREF dbShipObj = hsInterface.CloneThing(m_objnum);

    // Allocate the new ship
    CHSShip *newShip;
    newShip = new CHSShip;
    newShip->SetDbref(dbShipObj);

    if (HSCONF.autozone)
        hsInterface.SetLock(dbShipObj, hsInterface.GetGodDbref(), LOCK_ZONE);

    // Set the class info
    if (!newShip->SetClassInfo(m_class))
    {
        delete newShip;
        return HSNOTHING;
    }

    // Add the ship to the first available universe
    CHSUniverse *unDest;
    unDest = NULL;
    unDest = GetUniverse();
    unDest->AddObject(newShip);

    // Set the UID of the ship.
    newShip->SetUID(unDest->GetID());

    // Repair the ship
    newShip->TotalRepair();

    // Refuel the new ship
    newShip->Refuel();

    hsInterface.MoveObject(dbShipObj, hsInterface.GetLocation(m_objnum));

    // Setdock loc
    if (m_docked)
    {
        newShip->m_in_space = false;
        newShip->m_docked = true;
    }

    // Start with the first registered room, and recursively
    // clone through the exits until all rooms are visited.
    if (!m_setRooms.empty())
    {
        CHSShipRoomSet::iterator iter = m_setRooms.begin();
        CloneRoom(*iter, mapVisitedRooms, newShip);
    }


    return dbShipObj;
}

// Recursive function to clone a specific room.  The return
// value is the HS_DBREF of the cloned room.
HS_DBREF CHSShip::CloneRoom(HS_DBREF room,
                            std::map < HS_DBREF, HS_DBREF > &rmapVisitedRooms,
                            CHSShip * newShip)
{
    // Check to see if the room is already visited.  If so, return it's
    // cloned room dbref.
    std::map < HS_DBREF, HS_DBREF >::iterator iter =
        rmapVisitedRooms.find(room);
    if (iter != rmapVisitedRooms.end())
    {
        // This room has already been cloned.  Return the cloned room dbref.
        return iter->second;
    }

    // Room is new, so clone it, find exits, and clone their
    // rooms.
#ifdef PENNMUSH
    HS_DBREF dbNewRoom = hsInterface.CreateNewGameObject();
#else
    HS_DBREF dbNewRoom = hsInterface.CreateNewGameObject(TYPE_ROOM);
#endif

    // Set some info on the new room.
    hsInterface.SetObjectName(dbNewRoom, hsInterface.GetName(room));

#ifdef PENNMUSH
    // These are not necessary for TM3 either
    hsInterface.SetFlag(dbNewRoom, TYPE_ROOM, true);
    hsInterface.SetFlag(dbNewRoom,
                        hsInterface.GetGameOption(HSGO_ROOM_FLAGS));
    hsInterface.SetToggle(dbNewRoom,
                          hsInterface.GetGameOption(HSGO_ROOM_TOGGLES));
    // TM3 doesn't use floating
    hsInterface.SetToggle(dbNewRoom, ROOM_FLOATING);
#endif

#ifdef MUX
    //hsInterface.SetToggle(dbNewRoom, FLOATING);
    s_Flags(dbNewRoom, FLAG_WORD2, Flags2(dbNewRoom) | FLOATING);
#endif

    // set the room owner
    hsInterface.SetObjectOwner(dbNewRoom, hsInterface.GetOwner(room));
    // Set theh room parent
    hsInterface.SetObjectParent(dbNewRoom, hsInterface.GetParent(room));

    // Copy room attributes
    hsInterface.CopyAttributes(room, dbNewRoom);

    // Add the new room to the ship
    newShip->AddSroom(dbNewRoom);

    if (hsInterface.GetZone(room) != m_objnum)
    {
        hsInterface.SetObjectZone(dbNewRoom, hsInterface.GetZone(room));
    }

    // Get the HS_DBREF of the shipobj for the new ship.
    HS_DBREF dbShipObj = newShip->GetDbref();

    // Now see if this room has a SHIP attr on it.  If so,
    // set it to the new ship.
    char tbuf[32];
    if (hsInterface.AtrGet(dbNewRoom, "SHIP"))
    {
        sprintf(tbuf, "#%d", dbShipObj);
        hsInterface.AtrAdd(dbNewRoom, "SHIP", tbuf,
                           hsInterface.GetGodDbref());
    }

    // See if this model room is designated as our bay of
    // our shipobj.  If so, then set the bay of the new
    // shipobj.
    if (hsInterface.AtrGet(m_objnum, "BAY"))
    {
        HS_DBREF dbOurBay = strtodbref(hsInterface.m_buffer);
        if (room == dbOurBay)
        {
            // Set the bay attr on the new ship obj to this
            // new room.
            sprintf(tbuf, "#%d", dbNewRoom);
            hsInterface.AtrAdd(dbShipObj, "BAY", tbuf,
                               hsInterface.GetGodDbref());
        }
    }

    // Room cloned.  Add it to the map of cloned rooms.
    rmapVisitedRooms[room] = dbNewRoom;

    // Clone any consoles that might be in the room.  This
    // is not a very robust clone.  Only the console object
    // gets cloned and added to the ship.  It does not
    // clone the total information (weapons) of the console.
    // That needs to be added still.
    HS_DBREF thing;
    HS_DBREF dbNewConsole;
    for (thing = hsInterface.GetFirstContent(room); thing != HSNOTHING;
         thing = hsInterface.GetNextContent(thing))
    {
        if (hsInterface.HasFlag(thing, TYPE_THING, THING_HSPACE_CONSOLE))
        {
            // Potential console found.  Look for it in our
            // console list.
            for (int idx = 0; idx < MAX_SHIP_CONSOLES; idx++)
            {
                if (m_console_array[idx]
                    && m_console_array[idx]->m_objnum == thing)
                {
                    // Definite console.  Clone it and add
                    // it to the new ship.
                    dbNewConsole = hsInterface.CloneThing(thing);
                    newShip->AddConsole(dbNewConsole);
                    hsInterface.MoveObject(dbNewConsole, dbNewRoom);

                    // Console data is now stateful, do not clear
                    // m_console_array[idx]->ClearObjectAttrs();
                }
            }
        }
    }

    if (hsInterface.HasFlag(room, TYPE_ROOM, ROOM_HSPACE_LANDINGLOC))
    {
        newShip->AddLandingLoc(dbNewRoom);
    }

    // Iterate all exits in the current room, cloning those exits,
    // cloning the rooms they connect to, and then linking the exits
    // to the destination rooms.
    HS_DBREF exit_m;
    HS_DBREF loc;
    HS_DBREF rRoom;
    HS_DBREF dbNewExit;
    for (exit_m = hsInterface.GetFirstExit(room);
         exit_m && exit_m != HSNOTHING;
         exit_m = hsInterface.GetNextExit(exit_m))
    {
        // Grab the location of the exit .. the next room
        // to clone.  Clone that room, and open an exit 
        // to it.
        rRoom = HSNOTHING;

        loc = hsInterface.GetLocation(exit_m);

        // Clone the model exit.
#ifdef PENNMUSH
        dbNewExit = hsInterface.CreateNewGameObject();
#else
        dbNewExit = hsInterface.CreateNewGameObject(TYPE_EXIT);
#endif

        // If this is a hatch, don't clone the destination room, just open an exit.
        if (!hsInterface.HasFlag(exit_m, TYPE_EXIT, EXIT_HSPACE_HATCH))
        {
            if (loc != HSNOTHING)
            {
                // We have a destination room, so clone that room if and
                // only if it is a room of the origin ship.
                // This is, of course, recursive.

                if(false != HasSroom(loc))
                {
                    rRoom = CloneRoom(loc, rmapVisitedRooms, newShip);
                }
            }
        }
        else
        {
            // This exit is a hatch, add a hatch to the new ship
            // Do not copy original hatch attributes since the
            // new ship shouldn't be clamped to the same hatch
            // as the origin ship
            newShip->AddHatch(dbNewExit);
        }

        /*
         * initialize everything
         */
        hsInterface.SetObjectName(dbNewExit, hsInterface.GetName(exit_m));
        hsInterface.SetObjectOwner(dbNewExit, hsInterface.GetOwner(exit_m));
        hsInterface.SetObjectZone(dbNewExit, hsInterface.GetZone(exit_m));
        hsInterface.LinkExitToRoom(dbNewExit, dbNewRoom);
        hsInterface.SetFlag(dbNewExit, TYPE_EXIT, true);
        hsInterface.SetObjectParent(dbNewExit, hsInterface.GetParent(exit_m));
        hsInterface.SetFlag(dbNewExit,
                            hsInterface.GetGameOption(HSGO_EXIT_FLAGS));
        hsInterface.SetToggle(dbNewExit,
                              hsInterface.GetGameOption(HSGO_EXIT_TOGGLES));

        // Copy attributes
        hsInterface.CopyAttributes(exit_m, dbNewExit);

        if (rRoom != HSNOTHING)
        {
            hsInterface.SetObjectLocation(dbNewExit, rRoom);
        }
    }

    return dbNewRoom;
}

// Handles any communications messages that were sent to the
// ship.  The long pointer points to a HSCOMM structure with
// the information needed.
void CHSShip::HandleCommMsg(const char *msg, long *data)
{
    HSCOMM *hsComm;
    char tbuf[1024];
    char strName[64];
    char strFrq[64];
    int idx;
    CHSSysSensors *cSensors;

    hsComm = (HSCOMM *) data;

    // Do we have a comm array, and is it on?
    CHSSysComm *cComm;
    cComm = (CHSSysComm *) m_systems.GetSystem(HSS_COMM);
    if (!cComm || !cComm->GetCurrentPower())
        return;

    // Setup the message of how it will be printed to people
    // who get it.
    hsComm->msg[900] = '\0';    // Just in case it's a long one.

    // Find the source object on sensors, if applicable.
    if (!hsComm->cObj)
        strcpy(strName, "Unknown");
    else
    {
        // Are we the source object?
        if (hsComm->cObj->GetDbref() == m_objnum)
            return;

        // Find our sensors.
        cSensors = (CHSSysSensors *) m_systems.GetSystem(HSS_SENSORS);
        if (!cSensors)
            strcpy(strName, "Unknown");
        else
        {
            SENSOR_CONTACT *cContact;
            cContact = cSensors->GetContact(hsComm->cObj);
            if (!cContact)
                strcpy(strName, "Unknown");
            else
            {
                // Determine if contact is id'd or not.
                if (cContact->status == IDENTIFIED)
                    strcpy(strName, hsComm->cObj->GetName());
                else
                    sprintf(strName, "%d", cContact->m_id);
            }
        }
    }
    sprintf(tbuf, "%s[COMM Frq:%s %.2f  %sSource:%s %s%s]%s\n%s\n%s*EOT*%s",
            ANSI_CYAN, ANSI_NORMAL,
            hsComm->frq,
            ANSI_CYAN, ANSI_NORMAL,
            strName,
            ANSI_CYAN, ANSI_NORMAL, hsComm->msg, ANSI_CYAN, ANSI_NORMAL);

    // Run through our consoles, looking for comm consoles on this
    // frequency.
    for (idx = 0; idx < MAX_SHIP_CONSOLES; idx++)
    {
        if (m_console_array[idx])
        {
            // Is it a COMM console, and is it on this frq?
            if (hsInterface.HasFlag(m_console_array[idx]->m_objnum,
                                    TYPE_THING, THING_HSPACE_COMM) &&
                m_console_array[idx]->OnFrq(hsComm->frq))
            {
                // If the console has a COMM_HANDLER attribute, if 
                // so, execute it rather than the default message
                // handling
                if (hsInterface.AtrGet(m_console_array[idx]->m_objnum,
                                       "COMM_HANDLER"))
                {

                    sprintf(strFrq, "%.2f", hsComm->frq);
                    hsInterface.ClearEnvironmentVariables();
                    hsInterface.SetEnvironmentVariable(hsComm->msg);
                    hsInterface.SetEnvironmentVariable(strFrq);
                    hsInterface.SetEnvironmentVariable(strName);
                    hsInterface.EvalExpression(hsInterface.m_buffer,
                                               m_console_array[idx]->m_objnum,
                                               m_console_array[idx]->m_objnum,
                                               m_console_array[idx]->
                                               m_objnum);
                }
                else
                {
                    // Give the user the message.
                    m_console_array[idx]->HandleMessage(tbuf,
                                                        MSG_COMMUNICATION);
                }
            }
        }
    }
}

void CHSShip::InitSelfDestruct(HS_DBREF player, int Timer, char *BoardCode)
{
    if (GetBoardingCode())
    {
        if (strcmp(BoardCode, m_boarding_code))
        {
            hsInterface.Notify(player,
                               hsInterface.
                               HSPrintf("%s%s-%s Invalid access code.",
                                        ANSI_HILITE, ANSI_GREEN,
                                        ANSI_NORMAL));
            return;
        }
    }

    if (Timer < 1)
    {
        if (m_self_destruct_timer > 0)
        {
            NotifySrooms(hsInterface.HSPrintf("%s%sSelf destruct aborted.%s",
                                              ANSI_HILITE, ANSI_GREEN,
                                              ANSI_NORMAL));
            NotifyConsoles(hsInterface.
                           HSPrintf("%s%s-%s Self destruct aborted.",
                                    ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL),
                           MSG_GENERAL);
            m_self_destruct_timer = 0;
        }
        else
        {
            hsInterface.Notify(player,
                               hsInterface.
                               HSPrintf
                               ("%s%s-%s Self destruct not in progress.",
                                ANSI_GREEN, ANSI_HILITE, ANSI_NORMAL));
        }
    }
    else
    {
        NotifySrooms(hsInterface.
                     HSPrintf("%s%s*** SELF DESTRUCT IN %d SECONDS ***%s",
                              ANSI_HILITE, ANSI_RED, Timer, ANSI_NORMAL));
        NotifyConsoles(hsInterface.
                       HSPrintf("%s%s-%s Self destruct initiated.",
                                ANSI_GREEN, ANSI_HILITE, ANSI_NORMAL),
                       MSG_GENERAL);
        m_self_destruct_timer = Timer;
    }
}
