// -----------------------------------------------------------------------
// $Id: hscmds.cpp,v 1.39 2007/08/19 17:41:27 grapenut_org Exp $
// -----------------------------------------------------------------------

#ifdef WIN32
#pragma warning(disable:4786)
#endif

#include "hscopyright.h"
#include "hspace.h"
#include "hsversion.h"

#include <stdlib.h>
#include <cstring>
#include <assert.h>
#include <ctype.h>

#include "hsversion.h"
#include "hscmds.h"
#include "hsuniverse.h"
#include "hsuniversedb.h"
#include "hscelestial.h"
#include "hsdb.h"
#include "hsinterface.h"
#include "hsutils.h"
#include "hsconf.h"
#include "hsterritory.h"
#include "hsclass.h"
#include "hsflags.h"
#include "hsai.h"

// To add a new HSpace command, add it's prototype here, then
// add it to the hsSpaceCommandArray below.
HSPACE_COMMAND_PROTO(hscNewWeapon)
HSPACE_COMMAND_PROTO(hscCloneObject)
HSPACE_COMMAND_PROTO(hscCheckSpace)
HSPACE_COMMAND_PROTO(hscStartSpace)
HSPACE_COMMAND_PROTO(hscHaltSpace)
HSPACE_COMMAND_PROTO(hscActivateShip)
HSPACE_COMMAND_PROTO(hscMothballObject)
HSPACE_COMMAND_PROTO(hscNewUniverse)
HSPACE_COMMAND_PROTO(hscDelUniverse)
HSPACE_COMMAND_PROTO(hscSetAttribute)
HSPACE_COMMAND_PROTO(hscAddLandingLoc)
HSPACE_COMMAND_PROTO(hscDelLandingLoc)
HSPACE_COMMAND_PROTO(hscAddSroom)
HSPACE_COMMAND_PROTO(hscDelSroom)
HSPACE_COMMAND_PROTO(hscAddHatch)
HSPACE_COMMAND_PROTO(hscDeleteHatch)
HSPACE_COMMAND_PROTO(hscAddConsole)
HSPACE_COMMAND_PROTO(hscDelConsole)
HSPACE_COMMAND_PROTO(hscDelClass)
HSPACE_COMMAND_PROTO(hscAddObject)
HSPACE_COMMAND_PROTO(hscRepairShip)
HSPACE_COMMAND_PROTO(hscSetSystemAttr)
HSPACE_COMMAND_PROTO(hscAddWeapon)
HSPACE_COMMAND_PROTO(hscSetMissile)
HSPACE_COMMAND_PROTO(hscNewClass)
HSPACE_COMMAND_PROTO(hscDumpClassInfo)
HSPACE_COMMAND_PROTO(hscAddSysClass)
HSPACE_COMMAND_PROTO(hscSysInfoClass)
HSPACE_COMMAND_PROTO(hscSysInfo)
HSPACE_COMMAND_PROTO(hscSetSystemAttrClass)
HSPACE_COMMAND_PROTO(hscSetAttrClass)
HSPACE_COMMAND_PROTO(hscListDatabase)
HSPACE_COMMAND_PROTO(hscDelWeapon)
HSPACE_COMMAND_PROTO(hscAddTerritory)
HSPACE_COMMAND_PROTO(hscDelTerritory)
HSPACE_COMMAND_PROTO(hscSetAttrWeapon)
HSPACE_COMMAND_PROTO(hscAddSys)
HSPACE_COMMAND_PROTO(hscDelSys)
HSPACE_COMMAND_PROTO(hscDelSysClass)
HSPACE_COMMAND_PROTO(hscDumpWeapon)
HSPACE_COMMAND_PROTO(hscAddMessage)
HSPACE_COMMAND_PROTO(hscDelMessage) HSPACE_COMMAND_PROTO(hscOptions);
HSPACE_COMMAND_PROTO(hscConfig);
HSPACE_COMMAND_PROTO(hscSaveDatabase);
HSPACE_COMMAND_PROTO(hscVersion);

// For external calling from C code
#ifndef MUX
    extern "C"
    {
#endif
        HSPACE_COMMAND_PROTO(hscManConsole)     // Not an @space command
        HSPACE_COMMAND_PROTO(hscUnmanConsole)   // Not either.
        HSPACE_COMMAND_PROTO(hscDisembark)      // Nope, not one.
        HSPACE_COMMAND_PROTO(hscBoardShip)      // Nope.
#ifndef MUX
    }
#endif

// The hsCommandArray holds all externally callable @space commands.
    HSPACE_COMMAND hsSpaceCommandArray[] = {
        {"", hscCheckSpace, HCP_ANY},
        {"ACTIVATE", hscActivateShip, HCP_WIZARD},
        {"ADDHATCH", hscAddHatch, HCP_WIZARD},
        {"ADDCONSOLE", hscAddConsole, HCP_WIZARD},
        {"ADDLANDINGLOC", hscAddLandingLoc, HCP_WIZARD},
        {"ADDMESSAGE", hscAddMessage, HCP_WIZARD},
        {"ADDOBJECT", hscAddObject, HCP_WIZARD},
        {"ADDSROOM", hscAddSroom, HCP_WIZARD},
        {"ADDSYS", hscAddSys, HCP_WIZARD},
        {"ADDSYSCLASS", hscAddSysClass, HCP_WIZARD},
        {"ADDTERRITORY", hscAddTerritory, HCP_WIZARD},
        {"ADDWEAPON", hscAddWeapon, HCP_WIZARD},
        {"CLONE", hscCloneObject, HCP_WIZARD},
        {"CONFIG", hscConfig, HCP_WIZARD},
        {"DELCLASS", hscDelClass, HCP_WIZARD},
        {"DELCONSOLE", hscDelConsole, HCP_WIZARD},
        {"DELHATCH", hscDeleteHatch, HCP_WIZARD},
        {"DELLANDINGLOC", hscDelLandingLoc, HCP_WIZARD},
        {"DELMESSAGE", hscDelMessage, HCP_WIZARD},
        {"DELSROOM", hscDelSroom, HCP_WIZARD},
        {"DELTERRITORY", hscDelTerritory, HCP_WIZARD},
        {"DELUNIVERSE", hscDelUniverse, HCP_WIZARD},
        {"DELWEAPON", hscDelWeapon, HCP_WIZARD},
        {"DELSYS", hscDelSys, HCP_WIZARD},
        {"DELSYSCLASS", hscDelSysClass, HCP_WIZARD},
        {"DUMPCLASS", hscDumpClassInfo, HCP_WIZARD},
        {"DUMPWEAPON", hscDumpWeapon, HCP_WIZARD},
        {"HALT", hscHaltSpace, HCP_WIZARD},
        {"LIST", hscListDatabase, HCP_WIZARD},
        {"MOTHBALL", hscMothballObject, HCP_WIZARD},
        {"NEWCLASS", hscNewClass, HCP_WIZARD},
        {"NEWUNIVERSE", hscNewUniverse, HCP_WIZARD},
        {"NEWWEAPON", hscNewWeapon, HCP_WIZARD},
        {"OPTIONS", hscOptions, HCP_WIZARD},
        {"REPAIR", hscRepairShip, HCP_WIZARD},
        {"SAVE", hscSaveDatabase, HCP_WIZARD},
        {"SET", hscSetAttribute, HCP_WIZARD},
        {"SETCLASS", hscSetAttrClass, HCP_WIZARD},
        {"SETMISSILE", hscSetMissile, HCP_WIZARD},
        {"SETWEAPON", hscSetAttrWeapon, HCP_WIZARD},
        {"SYSINFO", hscSysInfo, HCP_WIZARD},
        {"SYSINFOCLASS", hscSysInfoClass, HCP_WIZARD},
        {"SYSSET", hscSetSystemAttr, HCP_WIZARD},
        {"SYSSETCLASS", hscSetSystemAttrClass, HCP_WIZARD},
        {"START", hscStartSpace, HCP_WIZARD},
        {"VERSION", hscVersion, HCP_ANY},
        {NULL, NULL, 0}
    };

// Finds a command with a given switch in the supplied command array.
HSPACE_COMMAND *hsFindCommand(const HS_INT8 * switches, HSPACE_COMMAND * cmdlist)
{
    HSPACE_COMMAND *ptr;
    HS_INT32 len;

    len = switches ? strlen(switches) : 0;
    for (ptr = cmdlist; ptr->key; ptr++)
    {
        // Check to see if the key is empty AND the switch is null
        if (!*ptr->key && (len == 0))
            return ptr;

        // Straight comparison of switch and key
        if (!strncasecmp(switches, ptr->key, len))
            return ptr;
    }

    return NULL;
}

HSPACE_COMMAND_HDR(hscSaveDatabase)
{
    hs_log(hsInterface.HSPrintf("Database dump requested by %s (#%d)",
                                hsInterface.GetName(player), player));
    HSpace.DumpDatabases();
    hsInterface.Notify(player, "HSpace database save completed.");
}

HSPACE_COMMAND_HDR(hscVersion)
{
    hsInterface.Notify(player, HSPACE_VERSION);
}

// Called from the game to give a status of HSpace
HSPACE_COMMAND_HDR(hscCheckSpace)
{
    HS_UINT32 uActiveShips;
    HS_UINT32 uShips;
    HS_UINT32 uCelestials;

    // If not a wizard, then just print out the HSpace version
    // and activity status.
    hsInterface.Notify(player, HSPACE_VERSION);

    if (!HSCONF.m_bConfigLoaded)
    {
        hsInterface.Notify(player,
                "**No HSpace configuration file was loaded on startup!**");
    }

    hsInterface.Notify(player,
            hsInterface.HSPrintf("   The system is currently %s.",
                HSpace.  Active()? "ACTIVE" : "INACTIVE"));

    if (!hsInterface.IsWizard(player))
    {
        return;
    }


    // Cycle time
    hsInterface.Notify(player,
                       hsInterface.HSPrintf("   Time per cycle: %.1fms",
                                            HSpace.CycleTime()));

    // Tally up active ships in universes.
    uActiveShips = 0;
    uShips = 0;
    uCelestials = 0;

    THSUniverseIterator tIter;
    HS_BOOL8 bIter;
    for (bIter = CHSUniverseDB::GetInstance().GetFirstUniverse(tIter); bIter;
         bIter = CHSUniverseDB::GetInstance().GetNextUniverse(tIter))
    {
        CHSUniverse *pUniverse = tIter.pValue;

        uActiveShips += pUniverse->GetNumActiveObjects(HST_SHIP);
        uShips += pUniverse->GetNumObjects(HST_SHIP);
        uCelestials += pUniverse->GetNumObjects(HST_PLANET);
    }

    // DB information
    hsInterface.Notify(player, "HSpace Database Report:");
    hsInterface.Notify(player,
                       hsInterface.HSPrintf("   Weapons    : %4d",
                                            dbHSDB.NumWeapons()));
    hsInterface.Notify(player,
                       hsInterface.HSPrintf("   Classes    : %4d",
                                            CHSClassDB::GetInstance().
                                            GetNumClasses()));
    hsInterface.Notify(player,
                       hsInterface.HSPrintf("   Universes  : %4d",
                                            dbHSDB.NumUniverses()));
    hsInterface.Notify(player,
                       hsInterface.HSPrintf("   Ships      : %4d (%d)",
                                            uShips, uActiveShips));
    hsInterface.Notify(player,
                       hsInterface.HSPrintf("   Celestials : %4d",
                                            uCelestials));
    hsInterface.Notify(player,
                       hsInterface.HSPrintf("   Territories: %4d",
                                            taTerritories.NumTerritories()));

    if (HSCONF.admin_server)
    {
        hsInterface.Notify(player,
                           hsInterface.
                           HSPrintf("   Admin port running on port %d (%s).",
                                    HSCONF.admin_server_port,
                                    HSCONF.
                                    admin_server_threaded ? "threaded" :
                                    "unthreaded"));
    }
    else
    {
        hsInterface.Notify(player, "   Admin port is deactivated.");
    }
}

// List configuration variables
HSPACE_COMMAND_HDR(hscOptions)
{
    hsInterface.Notify(player, "HSpace Options:");
    if (HSCONF.admin_server)
    {
        hsInterface.Notify(player,
                           hsInterface.
                           HSPrintf("   Admin port running on port %d (%s).",
                                    HSCONF.admin_server_port,
                                    HSCONF.
                                    admin_server_threaded ? "threaded" :
                                    "unthreaded"));
    }
    else
    {
        hsInterface.Notify(player, "   Admin port is deactivated.");
    }
    hsInterface.Notify(player,
                       hsInterface.HSPrintf("   Afterworld room is: %d.",
                                            HSCONF.afterworld));
    hsInterface.Notify(player,
                       hsInterface.
                       HSPrintf("   Maximum boarding distance: %d %s.",
                                HSCONF.max_board_dist, HSCONF.unit_name));
    hsInterface.Notify(player,
                       hsInterface.
                       HSPrintf("   Maximum docking distance: %d %s.",
                                HSCONF.max_dock_dist, HSCONF.unit_name));
    hsInterface.Notify(player,
                       hsInterface.
                       HSPrintf("   Maximum dropping distance: %d %s.",
                                HSCONF.max_dock_dist, HSCONF.unit_name));
    hsInterface.Notify(player,
                       hsInterface.HSPrintf("   Detection distance:  %d %s.",
                                            HSCONF.detectdist,
                                            HSCONF.unit_name));
    hsInterface.Notify(player,
                       hsInterface.
                       HSPrintf("   Identification distance: %d %s.",
                                HSCONF.identdist, HSCONF.unit_name));
    hsInterface.Notify(player,
                       hsInterface.HSPrintf("   Maximum sensor range: %d %s.",
                                            HSCONF.max_sensor_range,
                                            HSCONF.unit_name));
    hsInterface.Notify(player,
                       hsInterface.
                       HSPrintf("   Ships detect hypervessels: %s.",
                                HSCONF.sense_hypervessels ? "YES" : "NO"));
    hsInterface.Notify(player,
                       hsInterface.
                       HSPrintf("   Max ship size for docking: %d.",
                                HSCONF.max_dock_size));
    hsInterface.Notify(player,
                       hsInterface.HSPrintf("   Max landing speed: %d.",
                                            HSCONF.max_land_speed));
    hsInterface.Notify(player,
                       hsInterface.
                       HSPrintf("   Puppets can control ships: %s.",
                                HSCONF.forbid_puppets ? "YES" : "NO"));
    hsInterface.Notify(player,
                       hsInterface.HSPrintf("   Comm objects are %s.",
                                            HSCONF.
                                            use_comm_objects ? "Enabled" :
                                            "Disabled"));
    hsInterface.Notify(player,
                       hsInterface.
                       HSPrintf("   Ships landing time %d seconds.",
                                HSCONF.seconds_to_drop));
    hsInterface.Notify(player,
                       hsInterface.
                       HSPrintf("   Damage repair time is %d seconds.",
                                HSCONF.damage_repair_time));
    hsInterface.Notify(player,
                       hsInterface.HSPrintf("   Use of Two Fuels is %s.",
                                            HSCONF.
                                            use_two_fuels ? "Enabled" :
                                            "Disabled"));
    hsInterface.Notify(player,
                       hsInterface.
                       HSPrintf
                       ("   Power units produced per unit of fuel %d.",
                        HSCONF.fuel_ratio));
    hsInterface.Notify(player,
                       hsInterface.HSPrintf("   Afterburn speed ratio: %d.",
                                            HSCONF.afterburn_ratio));
    hsInterface.Notify(player,
                       hsInterface.HSPrintf("   Afterburn fuel ratio: %d.",
                                            HSCONF.afterburn_fuel_ratio));
    hsInterface.Notify(player,
                       hsInterface.HSPrintf("   Jump speed multiplier: %d.",
                                            HSCONF.jump_speed_multiplier));
    hsInterface.Notify(player,
                       hsInterface.HSPrintf("   Warp speed exponent: %f.",
                                            HSCONF.warp_exponent));
    hsInterface.Notify(player,
                       hsInterface.HSPrintf("   Warp speed constant: %f.",
                                            HSCONF.warp_constant));

}

// Start HSpace
HSPACE_COMMAND_HDR(hscStartSpace)
{
    if (HSpace.Active())
        hsInterface.Notify(player, "HSpace: System already active.");
    else
    {
        HSpace.SetCycle(true);
        hsInterface.Notify(player, "HSpace: System started.");
    }
}

// Stop HSpace
HSPACE_COMMAND_HDR(hscHaltSpace)
{
    if (!HSpace.Active())
        hsInterface.Notify(player, "HSpace: System already inactive.");
    else
    {
        HSpace.SetCycle(false);
        hsInterface.Notify(player, "HSpace: System halted.");
    }
}

// Activates a new ship
HSPACE_COMMAND_HDR(hscActivateShip)
{
    CHSShip *newShip;
    HS_INT32 iClass;
    HS_DBREF dbConsole;
    CHSUniverse *pDestUniverse;

    // Find the console
    dbConsole = hsInterface.NoisyMatchThing(player, arg_left);
    if (dbConsole == HSNOTHING)
        return;
    if (hsInterface.GetType(dbConsole) != TYPE_THING)
    {
        hsInterface.Notify(player,
                           hsInterface.
                           HSPrintf("%s(#%d) is not of type THING.",
                                    hsInterface.GetName(dbConsole),
                                    dbConsole));
        return;
    }

    // See if it's already a ship.
    if (dbHSDB.FindShip(dbConsole))
    {
        hsInterface.Notify(player, "That object is already an HSpace ship.");
        return;
    }

    // Check the class
    if (!arg_right || !*arg_right)
        iClass = -1;
    else
        iClass = atoi(arg_right);
    if (!CHSClassDB::GetInstance().IsValidClass(iClass))
    {
        hsInterface.Notify(player, "Invalid ship class specified.");
        return;
    }

    // Allocate the new ship
    newShip = new CHSShip;
    newShip->SetDbref(dbConsole);

    // Set the class info
    if (!newShip->SetClassInfo(iClass))
    {
        hsInterface.Notify(player, "Error setting class info.  Aborting.");
        delete newShip;
        return;
    }

    // Add the ship to the first available universe
    THSUniverseIterator tIter;
    if (!CHSUniverseDB::GetInstance().GetFirstUniverse(tIter))
    {
        hsInterface.Notify(player, "No valid universe for this ship exists.");
        newShip->Release();
        return;
    }

    if (!tIter.pValue->AddObject(newShip))
    {
        hsInterface.Notify(player,
                           "Ship could not be added to default universe.");
        newShip->Release();
        return;
    }
    pDestUniverse = tIter.pValue;

    // Set the universe id for the ship
    newShip->SetUID(pDestUniverse->GetID());
    newShip->SetName(hsInterface.GetName(dbConsole));

    // Set the flag
    hsInterface.SetToggle(dbConsole, THING_HSPACE_OBJECT);

    // Fully repair the ship
    newShip->TotalRepair();
    hsInterface.Notify(player,
                       hsInterface.
                       HSPrintf("%s (#%d) - activated with class %d.",
                                hsInterface.GetName(dbConsole), dbConsole,
                                iClass));
}

// Deactivates any HSpace object.
HSPACE_COMMAND_HDR(hscMothballObject)
{
    CHS3DObject *cObj;
    HS_DBREF dbObj;

    // Find the object
    dbObj = hsInterface.NoisyMatchThing(player, arg_left);
    if (dbObj == HSNOTHING)
        return;
    if (hsInterface.GetType(dbObj) != TYPE_THING)
    {
        hsInterface.Notify(player,
                           hsInterface.
                           HSPrintf("%s(#%d) is not of type THING.",
                                    hsInterface.GetName(dbObj), dbObj));
        return;
    }

    // See if it's a space object.
    if (!(cObj = dbHSDB.FindObject(dbObj)))
    {
        hsInterface.Notify(player,
                           "That is either not a space object, or it has no HSDB_TYPE attribute.");
        return;
    }

    // Find the universe it's in.
    CHSUniverse *uSource;
    uSource = cObj->GetUniverse();

    // Remove it from the universe.
    if (uSource)
        uSource->RemoveObject(cObj);

    // Delete the object. This is the end of it all.
    cObj->Release();

    hsInterface.Notify(player,
                       hsInterface.HSPrintf("HSpace object(#%d) mothballed.",
                                            dbObj));
}

// Allows the creation of a new universe.
HSPACE_COMMAND_HDR(hscNewUniverse)
{
    dbHSDB.CreateNewUniverse(player, arg_left);
}

// Allows the destruction of an existing universe.
HSPACE_COMMAND_HDR(hscDelUniverse)
{
    dbHSDB.DeleteUniverse(player, arg_left);
}

// Allows the addition of a new landing location to a planet or
// ship.
HSPACE_COMMAND_HDR(hscAddLandingLoc)
{
    HS_DBREF dbObject;
    HS_DBREF dbRoom;
    CHS3DObject *cObj;

    dbObject = hsInterface.NoisyMatchThing(player, arg_left);
    if (dbObject == HSNOTHING)
        return;

    dbRoom = hsInterface.NoisyMatchRoom(player, arg_right);
    if (dbRoom == HSNOTHING)
        return;

    if (hsInterface.GetType(dbRoom) != TYPE_ROOM)
    {
        hsInterface.Notify(player,
                           "You may only add rooms as landing locations.");
        return;
    }

    // Find the planet or ship
    cObj = dbHSDB.FindObject(dbObject);
    if (!cObj)
    {
        hsInterface.Notify(player,
                           hsInterface.
                           HSPrintf
                           ("That thing(#%d) is not an HSpace object.",
                            dbObject));
        return;
    }

    if (!cObj->AddLandingLoc(dbRoom))
    {
        hsInterface.Notify(player, "Failed to add landing location.");
    }
    else
    {
        hsInterface.AtrAdd(dbRoom, "HSDB_OBJECT",
                           hsInterface.HSPrintf("#%d", dbObject),
                           hsInterface.GetGodDbref());
        hsInterface.Notify(player,
                           hsInterface.
                           HSPrintf
                           ("Landing location #%d added to object #%d.",
                            dbRoom, dbObject));
    }
}

// Allows an admin to remove a landing location from an object.
HSPACE_COMMAND_HDR(hscDelLandingLoc)
{
    HS_DBREF dbConsole;
    HS_DBREF dbRoom;
    CHS3DObject *cObj;

    dbConsole = hsInterface.NoisyMatchThing(player, arg_left);
    if (dbConsole == HSNOTHING)
        return;

    dbRoom = strtodbref(arg_right);

    // Find the planet or ship
    cObj = dbHSDB.FindObject(dbConsole);
    if (!cObj)
    {
        hsInterface.Notify(player,
                           hsInterface.
                           HSPrintf
                           ("That thing(#%d) is not an HSpace object.",
                            dbConsole));
        return;
    }

    // See if this object has the landing location.
    if (cObj->FindLandingLoc(dbRoom) == NULL)
    {
        hsInterface.Notify(player,
                           "That room is not a landing location on that object.");
    }
    else
    {
        if (!cObj->DelLandingLoc(dbRoom))
        {
            hsInterface.Notify(player,
                               "Failed to remove that landing location from that object.");
        }
        else
        {
            hsInterface.Notify(player,
                               hsInterface.
                               HSPrintf
                               ("Removed landing location #%d from object #%d.",
                                dbRoom, dbConsole));
        }
    }
}

// Allows an attribute to be modified on any type of HSpace object.
HSPACE_COMMAND_HDR(hscSetAttribute)
{
    HS_DBREF dbConsole;
    HS_INT8 left_switch[64];
    HS_INT8 right_switch[64];
    HS_INT8 *ptr, *dptr;

    // We have to pull out the left and right parts of
    // the @space/set <console>/<attrib> format.
    dptr = left_switch;
    for (ptr = arg_left; *ptr && *ptr != '/'; ptr++)
    {
        // Check for buffer overflow
        if ((dptr - left_switch) > 62)
        {
            hsInterface.Notify(player,
                               "Invalid object/attribute pair supplied.");
            return;
        }

        *dptr++ = *ptr;
    }
    *dptr = '\0';

    // Right side.
    if (!*ptr)
    {
        hsInterface.Notify(player, "Invalid object/attribute pair supplied.");
        return;
    }
    ptr++;

    dptr = right_switch;
    while (*ptr)
    {
        *dptr++ = *ptr++;
    }
    *dptr = '\0';

    dbConsole = hsInterface.NoisyMatchThing(player, left_switch);
    if (dbConsole == HSNOTHING)
        return;

    if (!*right_switch)
    {
        hsInterface.Notify(player, "Invalid or no attribute name supplied.");
        return;
    }

    // Ok, find the HSpace object, and set the attribute.

    // Check flags first.
    if (hsInterface.HasFlag(dbConsole, TYPE_ROOM, ROOM_HSPACE_LANDINGLOC))
    {
        CHSLandingLoc *llLoc;

        llLoc = dbHSDB.FindLandingLoc(dbConsole);
        if (!llLoc)
        {
            hsInterface.Notify(player,
                               "That thing appears to be a landing location, but it's not.");
            return;
        }
        if (!llLoc->SetAttributeValue(right_switch, arg_right))
            hsInterface.Notify(player, "Attribute - failed.");
        else if (!*arg_right)
            hsInterface.Notify(player, "Attribute - cleared.");
        else
            hsInterface.Notify(player, "Attribute - set.");
    }
    else if (hsInterface.HasFlag(dbConsole, TYPE_THING, THING_HSPACE_OBJECT))
    {
        CHS3DObject *cObj;

        cObj = dbHSDB.FindObject(dbConsole);
        if (!cObj)
        {
            hsInterface.Notify(player,
                               "That thing is not a true HSpace object.");
            return;
        }
        if (!cObj->SetAttributeValue(right_switch, arg_right))
            hsInterface.Notify(player, "Attribute - failed.");
        else if (!*arg_right)
            hsInterface.Notify(player, "Attribute - cleared.");
        else
            hsInterface.Notify(player, "Attribute - set.");
    }
    else if (hsInterface.HasFlag(dbConsole, TYPE_THING, THING_HSPACE_CONSOLE))
    {
        CHS3DObject *cObj;
        CHSConsole *cConsole;

        cObj = dbHSDB.FindObjectByConsole(dbConsole);
        cConsole = dbHSDB.FindConsole(dbConsole);
        if (!cConsole)
        {
            hsInterface.Notify(player,
                               "That console does not belong to any HSpace object.");
            return;
        }

        if (!cObj)
        {
            hsInterface.Notify(player,
                               "That thing is not a true HSpace object.");
            return;
        }
        // Try to set the attribute on the console first.
        // If that fails, set it on the object.
        if (!cConsole->SetAttributeValue(right_switch, arg_right))
        {
            if (!cObj->SetAttributeValue(right_switch, arg_right))
                hsInterface.Notify(player, "Attribute - failed.");
            else if (!*arg_right)
                hsInterface.Notify(player,
                                   hsInterface.
                                   HSPrintf("%s Attribute - cleared.",
                                            cObj->GetName()));
            else
                hsInterface.Notify(player,
                                   hsInterface.HSPrintf("%s Attribute - set.",
                                                        cObj->GetName()));
        }
        else
            hsInterface.Notify(player, "Console Attribute - set.");
    }
    else if (hsInterface.HasFlag(dbConsole, TYPE_ROOM, ROOM_HSPACE_UNIVERSE))
    {
        // Set a universe attribute
        CHSUniverse *pDestUniverse;
        pDestUniverse = CHSUniverseDB::GetInstance().FindUniverse(dbConsole);

        if (!pDestUniverse)
        {
            hsInterface.Notify(player,
                               "That room has a universe flag, but it's not a valid universe.");
            return;
        }
        if (!pDestUniverse->SetAttributeValue(right_switch, arg_right))
            hsInterface.Notify(player, "Attribute - failed.");
        else
            hsInterface.Notify(player, "Attribute - set.");
    }
    else if (hsInterface.HasFlag(dbConsole, TYPE_THING,
                                 THING_HSPACE_TERRITORY))
    {
        // Find the territory.
        CHSTerritory *territory;

        territory = taTerritories.FindTerritory(dbConsole);
        if (!territory)
        {
            hsInterface.Notify(player,
                               "That object has a territory flag, but it's not a valid territory.");
            return;
        }

        if (!territory->SetAttributeValue(right_switch, arg_right))
            hsInterface.Notify(player, "Attribute - failed.");
        else
            hsInterface.Notify(player, "Attribute - set.");
    }
    else
        hsInterface.Notify(player,
                           "That thing does not appear to be an HSpace object.");
}

// Adds a room to a ship object
HSPACE_COMMAND_HDR(hscAddSroom)
{
    HS_DBREF dbConsole;
    HS_DBREF dbRoom;
    CHSShip *cShip;

    dbConsole = hsInterface.NoisyMatchThing(player, arg_left);
    if (dbConsole == HSNOTHING)
        return;

    dbRoom = hsInterface.NoisyMatchRoom(player, arg_right);
    if (dbRoom == HSNOTHING)
        return;
    if (hsInterface.GetType(dbRoom) != TYPE_ROOM)
    {
        hsInterface.Notify(player,
                           "You may only register rooms with this command.");
        return;
    }

    cShip = dbHSDB.FindShip(dbConsole);
    if (!cShip)
    {
        hsInterface.Notify(player, "That is not a HSpace ship.");
        return;
    }

    if (cShip->AddSroom(dbRoom))
        hsInterface.Notify(player, "Room - added.");
    else
        hsInterface.Notify(player, "Room - failed.");
}

// Deletes an sroom from a ship object
HSPACE_COMMAND_HDR(hscDelSroom)
{
    HS_DBREF dbConsole;
    HS_DBREF dbRoom;
    CHSShip *cShip;

    dbConsole = hsInterface.NoisyMatchThing(player, arg_left);
    if (dbConsole == HSNOTHING)
        return;

    if (!arg_right || !*arg_right)
    {
        hsInterface.Notify(player, "No room number specified.");
        return;
    }
    dbRoom = strtodbref(arg_right);

    cShip = dbHSDB.FindShip(dbConsole);
    if (!cShip)
    {
        hsInterface.Notify(player, "That is not a HSpace ship.");
        return;
    }

    if (cShip->DeleteSroom(dbRoom))
        hsInterface.Notify(player, "Room - deleted.");
    else
        hsInterface.Notify(player, "Room - not found.");
}

// Called to allow a player to man a console.
HSPACE_COMMAND_HDR(hscManConsole)
{
    HS_DBREF dbConsole;

    dbConsole = hsInterface.NoisyMatchThing(player, arg_left);
    if (dbConsole == HSNOTHING)
        return;

    if (!hsInterface.HasFlag(dbConsole, TYPE_THING, THING_HSPACE_CONSOLE))
    {
        hsInterface.Notify(player, "That thing is not an HSpace console.");
        return;
    }

    if (hsInterface.HasFlag(dbConsole, TYPE_THING, THING_HSPACE_C_LOCKED))
    {
        hsInterface.Notify(player, "That console is locked.");
        return;
    }

    // Can puppets man the console?
    if (hsInterface.GetType(player) != TYPE_PLAYER && HSCONF.forbid_puppets)
    {
        hsInterface.Notify(player, "Only players may man this console.");
        return;
    }

    if (hsInterface.GetLock(dbConsole, LOCK_USE) == player)
    {
        hsInterface.Notify(player, "You are already manning that console.");
        return;
    }

    if (hsInterface.ConsoleUser(dbConsole) != dbConsole &&
        hsInterface.AtrGet(hsInterface.ConsoleUser(dbConsole), "MCONSOLE"))
    {
        hsInterface.AtrDel(hsInterface.ConsoleUser(dbConsole), "MCONSOLE",
                           hsInterface.GetGodDbref());
    }

#ifdef ONLINE_REG
#ifndef TM3
    // Unregistered players can't use the system.
    if (hsInterface.HasFlag(player, TYPE_PLAYER, PLAYER_UNREG))
    {
        hsInterface.Notify(player,
                           "Unregistered players are not allowed to use the space system.");
        return;
    }
#endif // TM3
#endif

    // Check if the player isn't already manning a console.
    if (hsInterface.AtrGet(player, "MCONSOLE"))
    {
        HS_DBREF dbOldConsole;

        dbOldConsole = strtodbref(hsInterface.m_buffer);

        // Set the lock, give some messages.
        hsInterface.SetLock(dbOldConsole, dbOldConsole, LOCK_USE);

        // Delete attribute from player.
        if (hsInterface.AtrGet(player, "MCONSOLE"))
        {
            hsInterface.AtrDel(player, "MCONSOLE", hsInterface.GetGodDbref());
        }

        if (false == hsInterface.AtrGet(dbOldConsole, "UNMAN"))
        {
            hsInterface.Notify(player,
                               hsInterface.HSPrintf("You unman the %s.",
                                                    hsInterface.
                                                    GetName(dbOldConsole)));
        }

        if (false == hsInterface.AtrGet(dbOldConsole, "OUNMAN"))
        {
            hsInterface.NotifyExcept(hsInterface.
                                     GetContents(hsInterface.
                                                 GetLocation(dbOldConsole)),
                                     player,
                                     hsInterface.HSPrintf("%s unmans the %s.",
                                                          hsInterface.
                                                          GetName(player),
                                                          hsInterface.
                                                          GetName
                                                          (dbOldConsole)));
        }

        hsInterface.InvokeResponse(player, dbOldConsole, "UNMAN", "OUNMAN",
                                   "AUNMAN",
                                   hsInterface.GetLocation(dbOldConsole));

    }

    // Set the lock.
    hsInterface.SetLock(dbConsole, player, LOCK_USE);

    // Set attribute on player.
    hsInterface.AtrAdd(player, "MCONSOLE",
                       hsInterface.HSPrintf("#%d", dbConsole),
                       hsInterface.GetGodDbref(), AF_MDARK || AF_WIZARD);

    // Give some messages

    if (false == hsInterface.AtrGet(dbConsole, "MAN"))
    {
        hsInterface.Notify(player,
                           hsInterface.HSPrintf("You man the %s.",
                                                hsInterface.
                                                GetName(dbConsole)));
    }

    if (false == hsInterface.AtrGet(dbConsole, "OMAN"))
    {
        hsInterface.NotifyExcept(hsInterface.
                                 GetContents(hsInterface.
                                             GetLocation(dbConsole)), player,
                                 hsInterface.HSPrintf("%s mans the %s.",
                                                      hsInterface.
                                                      GetName(player),
                                                      hsInterface.
                                                      GetName(dbConsole)));
    }

    hsInterface.InvokeResponse(player, dbConsole, "MAN", "OMAN", "AMAN",
                               hsInterface.GetLocation(dbConsole));
}

// Called to allow a player to unman a console.
HSPACE_COMMAND_HDR(hscUnmanConsole)
{
    HS_DBREF dbConsole;
	#ifdef PENNMUSH
    dbConsole = hsInterface.MatchThing(player, arg_left);
	#else
	dbConsole = hsInterface.NoisyMatchThing(player, arg_left);
	#endif
    if (dbConsole == HSNOTHING)
	{
		// This section was modified by Mongo on 12/30/08 to allow players
		// to use the unman command without an argument.
		
		// Are we actually manning a console?
		if(!hsInterface.AtrGet(player, "MCONSOLE"))
		{
			hsInterface.Notify(player, "You are not manning a console.");
			return;
		}
		else
		{
			// Set our target console to the one being manned by the player.
			dbConsole = strtodbref(hsInterface.m_buffer);
		}
		
		// Make sure the player is actually the one using our target console.
		// If not, lets clear their attribute.
		if(hsInterface.GetLock(dbConsole, LOCK_USE) != player)
		{
			hsInterface.Notify(player, "You are no longer manning any consoles.");
			//Remove MCONSOLE attribute from player
			hsInterface.AtrDel(player, "MCONSOLE", hsInterface.GetGodDbref());
			return;
		}
		
	}

    if (!hsInterface.HasFlag(dbConsole, TYPE_THING, THING_HSPACE_CONSOLE))
    {
        hsInterface.Notify(player, "You are not manning that object.");
        return;
    }

    if (hsInterface.GetLock(dbConsole, LOCK_USE) != player)
    {
        hsInterface.Notify(player, "You are not manning that console.");
        return;
    }

    // Set the lock, give some messages.
    hsInterface.SetLock(dbConsole, dbConsole, LOCK_USE);

    // Delete attribute from player.
    if (hsInterface.AtrGet(player, "MCONSOLE"))
        hsInterface.AtrDel(player, "MCONSOLE", hsInterface.GetGodDbref());

    if (false == hsInterface.AtrGet(dbConsole, "UNMAN"))
    {
        hsInterface.Notify(player,
                           hsInterface.HSPrintf("You unman the %s.",
                                                hsInterface.
                                                GetName(dbConsole)));
    }

    if (false == hsInterface.AtrGet(dbConsole, "OUNMAN"))
    {
        hsInterface.NotifyExcept(hsInterface.
                                 GetContents(hsInterface.
                                             GetLocation(dbConsole)), player,
                                 hsInterface.HSPrintf("%s unmans the %s.",
                                                      hsInterface.
                                                      GetName(player),
                                                      hsInterface.
                                                      GetName(dbConsole)));
    }

    hsInterface.InvokeResponse(player, dbConsole, "UNMAN", "OUNMAN", "AUNMAN",
                               hsInterface.GetLocation(dbConsole));
}

// Called to add a given object as a new console for the
// space object.
HSPACE_COMMAND_HDR(hscAddConsole)
{
    HS_DBREF dbShip;
    HS_DBREF dbConsole;
    CHS3DObject *cObj;

    dbShip = hsInterface.NoisyMatchThing(player, arg_left);
    if (dbShip == HSNOTHING)
        return;

    dbConsole = hsInterface.NoisyMatchThing(player, arg_right);
    if (dbConsole == HSNOTHING)
        return;

    cObj = dbHSDB.FindObject(dbShip);
    if (!cObj)
    {
        hsInterface.Notify(player, "That thing is not an HSpace object.");
        return;
    }

    switch (cObj->GetType())
    {
    case HST_SHIP:
        CHSShip * ptr;
        ptr = (CHSShip *) cObj;
        if (!ptr->AddConsole(dbConsole))
            hsInterface.Notify(player, "Console - failed.");
        else
            hsInterface.Notify(player, "Console - added.");
        break;

    default:
        hsInterface.Notify(player,
                           "That HSpace object does not support that operation.");
        return;
    }
}

// Called to delete a given console object from a space object
HSPACE_COMMAND_HDR(hscDelConsole)
{
    HS_DBREF dbObj;
    HS_DBREF dbConsole;
    CHS3DObject *cObj;

    dbObj = hsInterface.NoisyMatchThing(player, arg_left);
    if (dbObj == HSNOTHING)
        return;

    if (!arg_right || !*arg_right)
    {
        hsInterface.Notify(player, "No console number specified.");
        return;
    }
    dbConsole = strtodbref(arg_right);

    cObj = dbHSDB.FindObject(dbObj);
    if (!cObj)
    {
        hsInterface.Notify(player, "That is not a HSpace object.");
        return;
    }

    switch (cObj->GetType())
    {
    case HST_SHIP:
        CHSShip * ptr;
        ptr = (CHSShip *) cObj;
        if (!ptr->RemoveConsole(dbConsole))
            hsInterface.Notify(player, "Console - not found.");
        else
            hsInterface.Notify(player, "Console - deleted.");
        break;

    default:
        hsInterface.Notify(player,
                           "That HSpace object does not support that operation.");
        break;
    }
}

// Called to delete a given class from the game
HSPACE_COMMAND_HDR(hscDelClass)
{
    HS_UINT32 uClass;

    if (!arg_left || !*arg_left)
    {
        hsInterface.Notify(player, "Must specify a class number to delete.");
        return;
    }

    uClass = atoi(arg_left);

    // Check to see if any ships remain with that class.
    // If so, don't allow it to be deleted.
    THSUniverseIterator tIter;
    HS_BOOL8 bIter;
    for (bIter = CHSUniverseDB::GetInstance().GetFirstUniverse(tIter); bIter;
         bIter = CHSUniverseDB::GetInstance().GetNextUniverse(tIter))
    {
        CHSUniverse *pUniverse = tIter.pValue;

        // Search ships in the universe
        THSObjectIterator tIterator;
        HS_BOOL8 bContinue;
        for (bContinue = pUniverse->GetFirstObject(tIterator, HST_SHIP);
             bContinue;
             bContinue = pUniverse->GetNextObject(tIterator, HST_SHIP))
        {
            if (tIterator.pValue)
            {
                CHSShip *pShip = static_cast < CHSShip * >(tIterator.pValue);
                if (pShip->ClassNum() == uClass)
                {
                    hsInterface.Notify(player,
                                       "You may not delete a class still in use.");
                    return;
                }
            }
        }
    }

    // If we're here, then no ships of the class were found.
    // Delete it!
    if (!CHSClassDB::GetInstance().RemoveClass(uClass))
        hsInterface.Notify(player, "Class - failed.");
    else
        hsInterface.Notify(player, "Class - deleted.");
}

// Called to add a new space object to the game.
HSPACE_COMMAND_HDR(hscAddObject)
{
    CHS3DObject *cObj;
    HS_DBREF dbObj;
    HS_INT8 strName[32];

    // Find the game object representing the new celestial
    dbObj = hsInterface.NoisyMatchThing(player, arg_left);
    if (dbObj == HSNOTHING)
        return;
    if (hsInterface.GetType(dbObj) != TYPE_THING)
    {
        hsInterface.Notify(player,
                           hsInterface.
                           HSPrintf("%s(#%d) is not of type THING.",
                                    hsInterface.GetName(dbObj), dbObj));
        return;
    }

    // See if it's already an object.
    if (dbHSDB.FindObject(dbObj))
    {
        hsInterface.Notify(player,
                           "That object is already an HSpace object.");
        return;
    }

    // Check the type to turn it into
    if (!arg_right || !*arg_right)
    {
        hsInterface.Notify(player, "You must specify a type of object.");
        return;
    }

    cObj = NULL;

    // Figure out the type of object, allocate the
    // object, etc.
    switch (atoi(arg_right))
    {
    case HST_SHIP:
        hsInterface.Notify(player,
                           "You must use @space/activate for that type.");
        return;

    case HST_PLANET:
        strcpy(strName, "planet");
        cObj = new CHSPlanet;
        break;

    case HST_NEBULA:
        strcpy(strName, "nebula");
        cObj = new CHSNebula;
        break;

    case HST_ASTEROID:
        strcpy(strName, "asteroid belt");
        cObj = new CHSAsteroid;
        break;

    case HST_BLACKHOLE:
        strcpy(strName, "black hole");
        cObj = new CHSBlackHole;
        break;

    case HST_WORMHOLE:
        strcpy(strName, "wormhole");
        cObj = new CHSWormHole;
        break;

    case HST_NOTYPE:           // Generic object
        cObj = new CHS3DObject;
        strcpy(strName, "space object");
        break;

    default:
        hsInterface.Notify(player, "Invalid type of object specified.");
        return;
    }


    // Add the object to the first available universe
    THSUniverseIterator tIter;
    if (!CHSUniverseDB::GetInstance().GetFirstUniverse(tIter))
    {
        hsInterface.Notify(player,
                           "No valid universe for this object exists.");
        cObj->Release();
        return;
    }
    if (!tIter.pValue->AddObject(cObj))
    {
        hsInterface.Notify(player,
                           "Object could not be added to default universe.");
        cObj->Release();
        return;
    }

    CHSUniverse *pDestUniverse = tIter.pValue;

    // Set the UID of the default universe for this object.
    cObj->SetUID(pDestUniverse->GetID());

    cObj->SetDbref(dbObj);
    cObj->SetName(hsInterface.GetName(dbObj));

    // Set the flag
    hsInterface.SetToggle(dbObj, THING_HSPACE_OBJECT);

    hsInterface.Notify(player,
                       hsInterface.
                       HSPrintf("%s (#%d) - is now a new %s in universe 0.",
                                hsInterface.GetName(dbObj), dbObj, strName));
}

// Called to completely repair a ship.
HSPACE_COMMAND_HDR(hscRepairShip)
{
    HS_DBREF dbConsole;
    CHSShip *cShip;

    dbConsole = hsInterface.NoisyMatchThing(player, arg_left);
    if (dbConsole == HSNOTHING)
        return;

    cShip = dbHSDB.FindShip(dbConsole);
    if (!cShip)
    {
        hsInterface.Notify(player, "That is not a HSpace ship.");
        return;
    }

    // Tell the ship to repair!
    cShip->TotalRepair();
    hsInterface.Notify(player, "Ship - repaired.");
}

// Allows an attribute for a particular engineering
// system to be changed.
HSPACE_COMMAND_HDR(hscSetSystemAttr)
{
    CHS3DObject *cObj;
    HS_INT8 *sptr, *dptr;
    HS_INT8 tbuf[256];
    HS_INT8 sysname[64];
    HS_INT8 attrname[64];
    HS_DBREF dbObj;

    // Parse out the parts of the command.
    // Command format is:
    //
    // @space/sysset obj:system/attr=value
    if (!arg_left || !arg_left)
    {
        hsInterface.Notify(player,
                           "You must specify an object and system name.");
        return;
    }
    // Pull out the object of interest
    dptr = tbuf;
    for (sptr = arg_left; *sptr; sptr++)
    {
        if (*sptr == ':')
        {
            *dptr = '\0';
            break;
        }
        else
            *dptr++ = *sptr;
    }
    *dptr = '\0';
    dbObj = hsInterface.NoisyMatchThing(player, tbuf);
    if (dbObj == HSNOTHING)
        return;

    // Pull out the system name
    dptr = sysname;
    if (!*sptr)
    {
        hsInterface.Notify(player, "Invalid system specified.");
        return;
    }
    sptr++;
    while (*sptr)
    {
        if ((*sptr == '/') || ((dptr - sysname) > 63))  // Don't allow overflow
        {
            break;
        }
        else
            *dptr++ = *sptr++;
    }
    *dptr = '\0';

    // Now pull out the attribute name
    if (!*sptr || (*sptr != '/'))
    {
        hsInterface.Notify(player, "Invalid command format given.");
        return;
    }
    sptr++;
    dptr = attrname;
    while (*sptr)
    {
        if ((dptr - attrname) > 63)
            break;
        else
            *dptr++ = *sptr++;
    }
    *dptr = '\0';

    // If we're setting the attribute based on a console
    // object, try to find the CHS3DObject for that console.
    if (hsInterface.HasFlag(dbObj, TYPE_THING, THING_HSPACE_CONSOLE))
        cObj = dbHSDB.FindObjectByConsole(dbObj);
    else
        cObj = dbHSDB.FindObject(dbObj);
    if (!cObj)
    {
        hsInterface.Notify(player,
                           "That thing does not appear to be an HSpace object.");
        return;
    }

    // Currently only support ships
    if (cObj->GetType() != HST_SHIP)
    {
        hsInterface.Notify(player,
                           "That HSpace object does not currently support that operation.");
        return;
    }

    // Try to set the attribute
    CHSShip *cShip;
    cShip = (CHSShip *) cObj;
    if (!cShip->SetSystemAttribute(sysname, attrname, arg_right))
        hsInterface.Notify(player, "SystemAttribute - failed.");
    else
        hsInterface.Notify(player, "SystemAttribute - set.");
}

// Allows a player to disembark from a ship.
HSPACE_COMMAND_HDR(hscDisembark)
{
    HS_DBREF dbRoom;
    HS_DBREF dbShip;
    CHSShip *cShip;
    HS_INT32 id;

    // Grab the location of the player.
    dbRoom = hsInterface.GetLocation(player);

    // See if the room has a SHIP attribute on it.
    if (!hsInterface.AtrGet(dbRoom, "SHIP"))
    {
        hsInterface.Notify(player, "You cannot disembark from here.");
        return;
    }

    // Convert the HS_DBREF string to a ship HS_DBREF
    dbShip = strtodbref(hsInterface.m_buffer);

    // Find the ship.
    cShip = dbHSDB.FindShip(dbShip);
    if (!cShip)
    {
        hsInterface.Notify(player,
                           "This room is setup as a disembarking location, but its SHIP attribute does not reference an actual ship.");
        return;
    }

    // The player can optionally supply and ID for disembarking
    // through a specific board link.
    if (arg_left && *arg_left)
        id = atoi(arg_left);
    else
        id = HSNOTHING;
    cShip->DisembarkPlayer(player, id);
}

// Called to allow a player board a ship.
HSPACE_COMMAND_HDR(hscBoardShip)
{
    HS_DBREF dbShipObj;
    HS_DBREF dbBay;
    CHSShip *cShip;
    HS_INT8 *ptr;

    dbShipObj = hsInterface.NoisyMatchThing(player, arg_left);
    if (dbShipObj == HSNOTHING)
        return;

    // Check to be sure it's a ship object
    if (!hsInterface.HasFlag(dbShipObj, TYPE_THING, THING_HSPACE_OBJECT))
    {
        hsInterface.Notify(player, "That thing is not a space vessel.");
        return;
    }

    // Find the CHSShip object based on the ship HS_DBREF
    cShip = dbHSDB.FindShip(dbShipObj);
    if (!cShip)
    {
        hsInterface.Notify(player,
                           "That thing seems to be a space vessel, but I couldn't find the ship it belongs to.");
        return;
    }

    // Check to see if the ship object has a boarding loc
    if (!hsInterface.AtrGet(dbShipObj, "BAY"))
    {
        hsInterface.Notify(player,
                           "That thing has no BAY attribute to indicate where to put you.");
        hsInterface.Notify(player,
                           "Set the BAY attribute to the room DBREF used as the boarding location.");
        return;
    }
    dbBay = strtodbref(hsInterface.m_buffer);

    // Check boarding code needs
    ptr = cShip->GetBoardingCode();
    if (ptr)
    {
        if (!arg_right)
        {
            hsInterface.Notify(player,
                               "You must specify a boarding code for this vessel.");
            return;
        }

        if (strcmp(arg_right, ptr))
        {
            hsInterface.Notify(player,
                               "Invalid boarding code -- permission denied.");
            return;
        }
    }

    // At this point all is ok.

    // Tell the player about boarding.
    hsInterface.Notify(player,
                       hsInterface.HSPrintf("You board the %s.",
                                            hsInterface.GetName(dbShipObj)));

    // Tell the players inside that the player is boarding.
    hsInterface.NotifyContents(dbBay,
                               hsInterface.
                               HSPrintf
                               ("%s boards the ship from the outside.",
                                hsInterface.GetName(player)));

    HS_DBREF dbPrevLoc = hsInterface.GetLocation(player);

    // Move the player
    hsInterface.MoveObject(player, dbBay);

    // Trigger the Aboard on the ship
    hsInterface.InvokeResponse(player, dbShipObj, NULL, NULL, "ABOARD",
                               dbPrevLoc);

    // Tell the players in the previous location that
    // the player boarded.
    hsInterface.NotifyContents(dbPrevLoc,
                               hsInterface.HSPrintf("%s boards the %s.",
                                                    hsInterface.
                                                    GetName(player),
                                                    hsInterface.
                                                    GetName(dbShipObj)));

    // Set the player's HSPACE_LOCATION attr.
    hsInterface.AtrAdd(player, "HSPACE_LOCATION",
                       hsInterface.HSPrintf("#%d", dbShipObj),
                       hsInterface.GetGodDbref(), AF_MDARK | AF_WIZARD);
}

// Called to allow an administrator to add a weapon to a console.
HSPACE_COMMAND_HDR(hscAddWeapon)
{
    CHSConsole *cConsole;
    HS_DBREF dbConsole;

    dbConsole = hsInterface.NoisyMatchThing(player, arg_left);
    if (dbConsole == HSNOTHING)
        return;

    // Check to be sure it's a ship object
    if (!hsInterface.HasFlag(dbConsole, TYPE_THING, THING_HSPACE_CONSOLE))
    {
        hsInterface.Notify(player, "That thing is not a HSpace console.");
        return;
    }

    // Find the CHSConsole object based on the console HS_DBREF
    cConsole = dbHSDB.FindConsole(dbConsole);
    if (!cConsole)
    {
        hsInterface.Notify(player,
                           "That thing has a console flag, but it's not a console.");
        return;
    }

    // Proper command usage
    if (!arg_right || !*arg_right)
    {
        hsInterface.Notify(player,
                           "You must specify the ID of a weapon type to add.");
        return;
    }

    cConsole->AddWeapon(player, atoi(arg_right));
}

// Called to allow an administrator to delete a weapon 
// from a console.
HSPACE_COMMAND_HDR(hscDelWeapon)
{
    CHSConsole *cConsole;
    HS_DBREF dbConsole;

    dbConsole = hsInterface.NoisyMatchThing(player, arg_left);
    if (dbConsole == HSNOTHING)
        return;

    // Check to be sure it's a console object
    if (!hsInterface.HasFlag(dbConsole, TYPE_THING, THING_HSPACE_CONSOLE))
    {
        hsInterface.Notify(player, "That thing is not a HSpace console.");
        return;
    }

    // Find the CHSConsole object based on the console HS_DBREF
    cConsole = dbHSDB.FindConsole(dbConsole);
    if (!cConsole)
    {
        hsInterface.Notify(player,
                           "That thing has a console flag, but it's not a console.");
        return;
    }

    // Proper command usage
    if (!arg_right || !*arg_right)
    {
        hsInterface.Notify(player,
                           "You must specify the ID of a weapon type to add.");
        return;
    }

    cConsole->DeleteWeapon(player, atoi(arg_right));
}

// Called to allow an administrator set the
// number of a specific type of munition on a ship.
HSPACE_COMMAND_HDR(hscSetMissile)
{
    CHSShip *cShip;
    HS_DBREF dbConsole;
    HS_INT8 strObj[64];
    HS_INT8 strType[8];
    HS_INT8 *sptr, *dptr;

    // Command usage check.
    if (!arg_left || !*arg_left || !arg_right || !*arg_right)
    {
        hsInterface.Notify(player, "Invalid command usage.");
        return;
    }

    // Pull out the object
    dptr = strObj;
    for (sptr = arg_left; *sptr; sptr++)
    {
        if (*sptr == '/' || ((dptr - strObj) > 63))
            break;
        else
            *dptr++ = *sptr;
    }
    *dptr = '\0';

    // Pull out the weapon id
    if (!*sptr)
    {
        hsInterface.Notify(player, "Must specify a weapon type.");
        return;
    }
    sptr++;
    dptr = strType;
    while (*sptr)
    {
        if ((dptr - strType) > 7)
            break;
        else
            *dptr++ = *sptr++;
    }
    *dptr = '\0';

    dbConsole = hsInterface.NoisyMatchThing(player, strObj);
    if (dbConsole == HSNOTHING)
        return;

    // Is the target object a console?  If so, find
    // the ship through the console.
    if (hsInterface.HasFlag(dbConsole, TYPE_THING, THING_HSPACE_CONSOLE))
    {
        CHS3DObject *cObj;
        cObj = dbHSDB.FindObjectByConsole(dbConsole);
        if (cObj && cObj->GetType() != HST_SHIP)
        {
            hsInterface.Notify(player, "You can only do that for ships.");
            return;
        }
        cShip = (CHSShip *) cObj;
    }
    else
        cShip = dbHSDB.FindShip(dbConsole);

    if (!cShip)
    {
        hsInterface.Notify(player, "That is not an HSpace ship.");
        return;
    }

    cShip->SetNumMissiles(player, atoi(strType), atoi(arg_right));
}

// Adds a new class with the specified name to the game
HSPACE_COMMAND_HDR(hscNewClass)
{
    CHSShipClass *pClass;

    // Must specify a name
    if (!arg_left || !*arg_left)
    {
        hsInterface.Notify(player, "Must specify a name for the class.");
        return;
    }

    // Grab a new class.
    pClass = new CHSShipClass(CHSClassDB::GetInstance().GetNextClassID(true));
    if (!pClass)
    {
        hsInterface.Notify(player,
                           "Failed to allocate a new slot for this class.  \
			Probably too many classes already.");
        return;
    }

    // Set size to default of 1
    pClass->Size(1);

    // Set the name
    pClass->ClassName(arg_left);

    if (CHSClassDB::GetInstance().AddClass(pClass))
    {
        hsInterface.Notify(player,
                           hsInterface.HSPrintf("Ship class \"%s\" - added.",
                                                arg_left));
    }
    else
    {
        hsInterface.Notify(player, "Failed to add new ship class.");
        delete pClass;
    }
}

// Prints out information about a given class
HSPACE_COMMAND_HDR(hscDumpClassInfo)
{
    CHSShipClass *pClass;
    HS_INT32 iClass;

    // Find the class based on the number
    iClass = atoi(arg_left);
    pClass = CHSClassDB::GetInstance().GetClass(iClass);
    if (!pClass)
    {
        hsInterface.Notify(player, "Ship class non-existent.");
        return;
    }

    // Print out the info
    hsInterface.Notify(player,
                       hsInterface.HSPrintf("Ship Class: %d   %s",
                                            iClass,
                                            pClass->ClassName()));
    hsInterface.Notify(player,
                       "------------------------------------------------------------");
    hsInterface.Notify(player,
                       hsInterface.
                       HSPrintf("Ship Size : %-3d         Cargo Size: %d",
                                pClass->Size(), pClass->CargoSize()));
    hsInterface.Notify(player,
                       hsInterface.
                       HSPrintf("Min. Crew : %-3d         Max Hull  : %d",
                                pClass->MinCrew(), pClass->MaxHull()));
    hsInterface.Notify(player,
                       hsInterface.HSPrintf("Can Drop  : %-3s",
                                            pClass->CanDrop()? "YES" : "NO"));

    // print out systems info
    hsInterface.Notify(player, "\n- Engineering Systems -");

    if (pClass->m_pSystems)
    {
        CHSEngSystem *cSys, *cNext;
        cSys = pClass->m_pSystems->GetHead();
        while (cSys)
        {
            cNext = cSys->GetNext();

            // Print out two per line or just one?
            if (cNext)
                hsInterface.Notify(player,
                                   hsInterface.HSPrintf("*%-30s*%s",
                                                        cSys->GetName(),
                                                        cNext->GetName()));
            else
                hsInterface.Notify(player,
                                   hsInterface.HSPrintf("*%s",
                                                        cSys->GetName()));

            if (!cNext)
                break;
            else
                cSys = cNext->GetNext();
        }
    }
}

HSPACE_COMMAND_HDR(hscAddSysClass)
{
    HS_INT32 iClass;
    CHSShipClass *pClass;

    // Find the class
    iClass = atoi(arg_left);
    pClass = CHSClassDB::GetInstance().GetClass(iClass);
    if (!pClass)
    {
        hsInterface.Notify(player, "Ship class non-existent.");
        return;
    }

    // Find the system based on the name
    HSS_TYPE type;
    if (!arg_right || !*arg_right)
    {
        hsInterface.Notify(player, "Must specify a system name to add.");
        return;
    }

    type = hsGetEngSystemType(arg_right);
    if (type == HSS_NOTYPE)
    {
        hsInterface.Notify(player, "Invalid system name specified.");
        return;
    }

    // Try to find the system already on the class
    CHSEngSystem *cSys;

    if (type != HSS_FICTIONAL && pClass->m_pSystems)
    {
        cSys = pClass->m_pSystems->GetSystem(type);
        if (cSys)
        {
            hsInterface.Notify(player,
                               "That system already exists for that class.");
            return;
        }
    }

    // Add the system
    if (!pClass->m_pSystems)
    {
        pClass->m_pSystems = new CHSSystemArray;
    }

    cSys = CHSEngSystem::CreateFromType(type);

    if (!cSys)
    {
        hsInterface.Notify(player,
                           "Failed to add the system to the specified class.");
        return;
    }

    pClass->m_pSystems->AddSystem(cSys);

    hsInterface.Notify(player,
                       hsInterface.HSPrintf("%s system added to class %d.",
                                            cSys->GetName(), iClass));
}

// Prints systems information for a given system on a given class.
HSPACE_COMMAND_HDR(hscSysInfoClass)
{
    HS_INT32 iClass;
    CHSShipClass *pClass;

    iClass = atoi(arg_left);
    if (!arg_right || !*arg_right)
    {
        hsInterface.Notify(player, "Must specify a system name.");
        return;
    }

    // Find the class
    pClass = CHSClassDB::GetInstance().GetClass(iClass);
    if (!pClass)
    {
        hsInterface.Notify(player, "Ship class non-existent.");
        return;
    }

    // Find the system type based on the name
    HSS_TYPE type;
    type = hsGetEngSystemType(arg_right);
    if (type == HSS_NOTYPE)
    {
        hsInterface.Notify(player, "Invalid system name specified.");
        return;
    }

    CHSEngSystem *cSys;
    // Grab the system from the class
    cSys = pClass->m_pSystems ? pClass->m_pSystems->GetSystem(type) : NULL;
    if (!cSys)
    {
        hsInterface.Notify(player,
                           "That system does not exist for that class.");
        return;
    }

    // Get a list of valid attributes on the system, and print out their values.
    CHSAttributeList listAttrs;
    cSys->GetAttributeList(listAttrs);

    hsInterface.Notify(player, "----------------------------------------");
    while (!listAttrs.empty())
    {
        std::string & rstrAttribute = listAttrs.front();

        // Query the system for the attribute's value.
        CHSVariant varValue;
        HS_INT8 cBuffer[64];
        if (cSys->
            GetAttributeValue(rstrAttribute.c_str(), varValue, false, false))
        {
            switch (varValue.GetType())
            {
            case CHSVariant::VT_BOOL:
                sprintf(cBuffer, "%s",
                        varValue.GetBool() == true ? "true" : "false");
                break;

            case CHSVariant::VT_INT16:
            case CHSVariant::VT_INT32:
            case CHSVariant::VT_INT8:
            case CHSVariant::VT_UINT16:
            case CHSVariant::VT_UINT32:
            case CHSVariant::VT_UINT8:
                sprintf(cBuffer, "%d", varValue.GetUInt());
                break;

            case CHSVariant::VT_FLOAT:
                sprintf(cBuffer, "%.2f", varValue.GetFloat());
                break;
            case CHSVariant::VT_DOUBLE:
                sprintf(cBuffer, "%.2f", varValue.GetDouble());
                break;

            case CHSVariant::VT_STRING:
                strcpy(cBuffer, varValue.GetString());
                break;

            default:
                continue;
            }

            hsInterface.Notify(player,
                               hsInterface.HSPrintf("%s=%s",
                                                    rstrAttribute.c_str(),
                                                    cBuffer));
        }
        else
        {
            assert(0);          // Failed to query a known attribute?
        }

        listAttrs.pop_front();
    }
    hsInterface.Notify(player, "----------------------------------------");
}

// Allows an attribute for a particular engineering
// system to be changed on a given ship class.
HSPACE_COMMAND_HDR(hscSetSystemAttrClass)
{
    CHSShipClass *pClass;
    HS_INT8 *sptr, *dptr;
    HS_INT8 tbuf[256];
    HS_INT8 sysname[64];
    HS_INT8 attrname[64];
    HS_INT32 iClass;

    // Parse out the parts of the command.
    // Command format is:
    //
    // @space/syssetclass class#:system/attr=value
    if (!arg_left || !arg_left)
    {
        hsInterface.Notify(player,
                           "You must specify an class number and system name.");
        return;
    }
    // Pull out the class of interest
    dptr = tbuf;
    for (sptr = arg_left; *sptr; sptr++)
    {
        if (*sptr == ':')
        {
            *dptr = '\0';
            break;
        }
        else
            *dptr++ = *sptr;
    }
    *dptr = '\0';
    iClass = atoi(tbuf);
    pClass = CHSClassDB::GetInstance().GetClass(iClass);
    if (!pClass)
    {
        hsInterface.Notify(player, "Ship class non-existent.");
        return;
    }

    // Pull out the system name
    dptr = sysname;
    if (!*sptr)
    {
        hsInterface.Notify(player, "Invalid system specified.");
        return;
    }
    sptr++;
    while (*sptr)
    {
        if ((*sptr == '/') || ((dptr - sysname) > 63))  // Don't allow overflow
        {
            break;
        }
        else
            *dptr++ = *sptr++;
    }
    *dptr = '\0';

    // Now pull out the attribute name
    if (!*sptr || (*sptr != '/'))
    {
        hsInterface.Notify(player, "Invalid command format given.");
        return;
    }
    sptr++;
    dptr = attrname;
    while (*sptr)
    {
        if ((dptr - attrname) > 63)
            break;
        else
            *dptr++ = *sptr++;
    }
    *dptr = '\0';

    // Determine what type of system is being queried.
    HSS_TYPE type;
    CHSEngSystem *cSys;
    type = hsGetEngSystemType(sysname);
    if (type == HSS_NOTYPE)
    {
        cSys =
            pClass->m_pSystems ? pClass->m_pSystems->
            GetSystemByName(sysname) : NULL;
        if (!cSys)
        {
            hsInterface.Notify(player,
                               "That system does not exist on that class.");
            return;
        }
    }
    else
    {
        cSys =
            pClass->m_pSystems ? pClass->m_pSystems->GetSystem(type) : NULL;

        if (!cSys)
        {
            hsInterface.Notify(player,
                               "That system does not exist on that class.");
            return;
        }
    }

    // See if the system supports this attribute.
    if (!cSys->SetAttributeValue(attrname, arg_right))
    {
        hsInterface.Notify(player, "SystemAttribute - failed.");
    }
    else
    {
        hsInterface.Notify(player, "SystemAttribute - set.");
    }
}

// Sets an attribute for a specified class.
HSPACE_COMMAND_HDR(hscSetAttrClass)
{
    HS_INT32 iClass;
    CHSShipClass *pClass;
    HS_INT8 *sptr, *dptr;
    HS_INT8 tbuf[64];

    // Command format is:
    //
    // @space/setclass #/attr=value
    //
    if (!arg_left || !*arg_left)
    {
        hsInterface.Notify(player,
                           "You must specify a class number and attribute name.");
        return;
    }

    dptr = tbuf;
    for (sptr = arg_left; *sptr; sptr++)
    {
        if ((dptr - tbuf) > 63)
            break;

        if (*sptr == '/')
            break;

        *dptr++ = *sptr;
    }
    *dptr = '\0';

    iClass = atoi(tbuf);

    // Find the class
    pClass = CHSClassDB::GetInstance().GetClass(iClass);
    if (!pClass)
    {
        hsInterface.Notify(player, "Ship class non-existent.");
        return;
    }

    // Pull out the attr name
    HS_INT8 attrname[64];
    if (!*sptr)
    {
        hsInterface.Notify(player, "You must specify an attribute name.");
        return;
    }
    sptr++;
    dptr = attrname;
    while (*sptr)
    {
        if ((dptr - attrname) > 63)
            break;

        *dptr++ = *sptr++;
    }
    *dptr = '\0';

    // Match the attribute name
    HS_INT32 len;
    HS_INT32 iVal;
    len = strlen(attrname);

    if (!strncasecmp(attrname, "NAME", len))
    {
        pClass->ClassName(arg_right);
    }
    else if (!strncasecmp(attrname, "SIZE", len))
    {
        iVal = atoi(arg_right);
        if (iVal < 1)
        {
            hsInterface.Notify(player, "Sizes must be greater than 0.");
            return;
        }

        pClass->Size(iVal);
    }
    else if (!strncasecmp(attrname, "MAXHULL", len))
    {
        pClass->MaxHull(atoi(arg_right));
    }
    else if (!strncasecmp(attrname, "CAN DROP", len))
    {
        pClass->CanDrop(atoi(arg_right) == 0 ? false : true);
    }
    else if (!strncasecmp(attrname, "SPACEDOCK", len))
    {
        pClass->SpaceDock(atoi(arg_right) == 0 ? false : true);
    }
    else if (!strncasecmp(attrname, "CARGO", len))
    {
        pClass->CargoSize(atoi(arg_right));
    }
    else if (!strncasecmp(attrname, "MINMANNED", len))
    {
        pClass->MinCrew(atoi(arg_right));
    }
    else
    {
        hsInterface.Notify(player, "Invalid attribute specified.");
        return;
    }

    hsInterface.Notify(player, "Class attribute - set.");
}

// Allows an administrator to list information contained
// within a given database (e.g. weapons).
HSPACE_COMMAND_HDR(hscListDatabase)
{
    // Command usage?
    if (!arg_left || !*arg_left)
    {
        hsInterface.Notify(player,
                           "You must specify a given database to list.");
        return;
    }

    // Match the database name
    HS_INT32 len;
    HS_INT8 *ptr, *dptr;
    HS_INT8 strName[32];
    dptr = strName;
    for (ptr = arg_left; *ptr; ptr++)
    {
        if ((dptr - strName) > 31)
            break;

        if (*ptr == '/')
            break;

        *dptr++ = *ptr;
    }
    *dptr = '\0';


    len = strlen(strName);
    if (!strncasecmp(strName, "objects", len))
    {
        // Command usage for this is:
        //
        // @space/list objects/uid=type
        HS_TYPE type;

        if (!arg_right || !*arg_right)
            type = HST_NOTYPE;
        else
            type = (HS_TYPE) atoi(arg_right);

        // Pull out the universe id
        HS_INT8 *dptr;
        for (dptr = arg_left; *dptr; dptr++)
        {
            if (*dptr == '/')
                break;
        }
        if (*dptr != '/')
        {
            hsInterface.Notify(player,
                               "You must specify a valid universe ID.");
            return;
        }

        dptr++;
        HS_INT32 uid;
        uid = atoi(dptr);

        // If uid is < 0, list all universes
        CHSUniverse *pUniverse = NULL;
        HS_UINT32 uiUniversesLeft = 0;
        THSUniverseIterator tIter;
        if (uid < 0)
        {
            uiUniversesLeft = CHSUniverseDB::GetInstance().GetNumUniverses();

            if (CHSUniverseDB::GetInstance().GetFirstUniverse(tIter))
            {
                pUniverse = tIter.pValue;
            }
        }
        else
        {
            // Only list objects for one universe.
            pUniverse = CHSUniverseDB::GetInstance().FindUniverse(uid);
            uiUniversesLeft = 1;
        }

        if (!pUniverse)
        {
            hsInterface.Notify(player,
                               hsInterface.
                               HSPrintf
                               ("No such universe %d found in the database.",
                                uid));
        }
        else
        {
            while (uiUniversesLeft > 0 && pUniverse)
            {
                // Print the header
                hsInterface.Notify(player,
                                   "[Dbref#] Name                    X        Y        Z        UID   Active");

                THSObjectIterator tIterator;
                HS_BOOL8 bContinue;
                for (bContinue = pUniverse->GetFirstObject(tIterator, type);
                     bContinue;
                     bContinue = pUniverse->GetNextObject(tIterator, type))
                {
                    CHS3DObject *pObject = tIterator.pValue;

                    // Print object info
                    hsInterface.Notify(player,
                                       hsInterface.
                                       HSPrintf
                                       ("[%6d] %-24s%-9.0f%-9.0f%-9.0f%-5d  %s",
                                        pObject->GetDbref(),
                                        pObject->GetName(), pObject->GetX(),
                                        pObject->GetY(), pObject->GetZ(),
                                        pObject->GetUID(),
                                        pObject->IsActive()? "YES" : "NO"));
                }

                // Move on to the next universe.
                uiUniversesLeft--;

                if (uiUniversesLeft > 0)
                {
                    // At this piece of code, we know that the THUniverseIterator was used
                    // and is valid.  So we can use that again.
                    if (CHSUniverseDB::GetInstance().GetNextUniverse(tIter))
                    {
                        pUniverse = tIter.pValue;
                    }
                    else
                    {
                        pUniverse = NULL;
                    }
                }
            }
        }
    }
    else if (!strncasecmp(strName, "weapons", len))
    {
        waWeapons.PrintInfo(player);
    }
    else if (!strncasecmp(strName, "universes", len))
    {
        CHSUniverseDB::GetInstance().PrintInfo(player);
    }
    else if (!strncasecmp(strName, "classes", len))
    {
        CHSClassDB::GetInstance().PrintInfo(player);
    }
    else if (!strncasecmp(strName, "territories", len))
    {
        taTerritories.PrintInfo(player);
    }
    else if (!strncasecmp(strName, "destroyed", len))
    {
        // Print the header
        hsInterface.Notify(player,
                           "[Dbref#] Name                    X        Y        Z        UID");

        THSUniverseIterator tIter;
        HS_BOOL8 bIter;
        for (bIter = CHSUniverseDB::GetInstance().GetFirstUniverse(tIter);
             bIter;
             bIter = CHSUniverseDB::GetInstance().GetNextUniverse(tIter))
        {
            CHSUniverse *pUniverse = tIter.pValue;

            THSObjectIterator tIterator;
            HS_BOOL8 bContinue;
            for (bContinue = pUniverse->GetFirstObject(tIterator, HST_SHIP);
                 bContinue;
                 bContinue = pUniverse->GetNextObject(tIterator, HST_SHIP))
            {
                CHSShip *pShip = static_cast < CHSShip * >(tIterator.pValue);

                if (!pShip->IsDestroyed())
                    continue;

                // Print object info
                hsInterface.Notify(player,
                                   hsInterface.
                                   HSPrintf("[%6d] %-24s%-9.0f%-9.0f%-9.0f%d",
                                            pShip->GetDbref(),
                                            pShip->GetName(), pShip->GetX(),
                                            pShip->GetY(), pShip->GetZ(),
                                            pShip->GetUID()));
            }
        }
    }
    else if (!strncasecmp(strName, "autopilots", len))
    {
        cRoster.DumpRoster(player);
    }
    else
        hsInterface.Notify(player,
                           "Must specify one of: objects, weapons, universes, classes, destroyed, or autopilots.");
}

// Prints systems information for a given system on a given object.
HSPACE_COMMAND_HDR(hscSysInfo)
{
    CHS3DObject *cObj;
    HS_DBREF dbObj;

    // Find the object
    dbObj = hsInterface.NoisyMatchThing(player, arg_left);
    if (dbObj == HSNOTHING)
        return;
    if (hsInterface.GetType(dbObj) != TYPE_THING)
    {
        hsInterface.Notify(player,
                           hsInterface.
                           HSPrintf("%s(#%d) is not of type THING.",
                                    hsInterface.GetName(dbObj), dbObj));
        return;
    }

    // See if it's a console.  If so, find the ship based on the console.
    if (hsInterface.HasFlag(dbObj, TYPE_THING, THING_HSPACE_CONSOLE))
    {
        cObj = dbHSDB.FindObjectByConsole(dbObj);
    }
    else
    {
        cObj = dbHSDB.FindObject(dbObj);
    }

    // See if it's a space object.
    if (!cObj)
    {
        hsInterface.Notify(player,
                           "That is either not a space object, or it has no HSDB_TYPE attribute.");
        return;
    }

    // Find the system type based on the name
    HSS_TYPE type;
    type = hsGetEngSystemType(arg_right);
    if (type == HSS_NOTYPE)
    {
        hsInterface.Notify(player, "Invalid system name specified.");
        return;
    }

    CHSEngSystem *cSys;
    // Grab the system from the object
    cSys = cObj->GetEngSystem(type);
    if (!cSys)
    {
        hsInterface.Notify(player,
                           "That system does not exist for that object.");
        return;
    }

    // Get a list of valid attributes on the system, and print out their values.
    CHSAttributeList listAttrs;
    cSys->GetAttributeList(listAttrs);

    hsInterface.Notify(player, "----------------------------------------");
    while (!listAttrs.empty())
    {
        std::string & rstrAttribute = listAttrs.front();

        // Query the system for the attribute's value.
        CHSVariant varValue;
        HS_INT8 cBuffer[64];
        if (cSys->
            GetAttributeValue(rstrAttribute.c_str(), varValue, false, false))
        {
            switch (varValue.GetType())
            {
            case CHSVariant::VT_BOOL:
                sprintf(cBuffer, "%s",
                        varValue.GetBool() == true ? "true" : "false");
                break;

            case CHSVariant::VT_INT16:
            case CHSVariant::VT_INT32:
            case CHSVariant::VT_INT8:
            case CHSVariant::VT_UINT16:
            case CHSVariant::VT_UINT32:
            case CHSVariant::VT_UINT8:
                sprintf(cBuffer, "%d", varValue.GetUInt());
                break;

            case CHSVariant::VT_FLOAT:
                sprintf(cBuffer, "%.2f", varValue.GetFloat());
                break;
            case CHSVariant::VT_DOUBLE:
                sprintf(cBuffer, "%.2f", varValue.GetDouble());
                break;

            case CHSVariant::VT_STRING:
                strcpy(cBuffer, varValue.GetString());
                break;

            default:
                continue;
            }

            hsInterface.Notify(player,
                               hsInterface.HSPrintf("%s=%s",
                                                    rstrAttribute.c_str(),
                                                    cBuffer));
        }

        listAttrs.pop_front();
    }
    hsInterface.Notify(player, "----------------------------------------");
}

// Deactivates any HSpace object.
HSPACE_COMMAND_HDR(hscCloneObject)
{
    CHS3DObject *cObj;
    HS_DBREF dbObj;

    // Find the object
    dbObj = hsInterface.NoisyMatchThing(player, arg_left);
    if (dbObj == HSNOTHING)
        return;

    // See if it's a space object.
    if (!(cObj = dbHSDB.FindObject(dbObj)))
    {
        hsInterface.Notify(player,
                           "That is either not a space object, or it has no HSDB_TYPE attribute.");
        return;
    }

    // Only clone ships right now.
    if (cObj->GetType() != HST_SHIP)
    {
        hsInterface.Notify(player, "You may only clone ships at this time.");
        return;
    }

    CHSShip *cShip;
    cShip = (CHSShip *) cObj;

    // Clone it!
    HS_DBREF dbShipObj = cShip->Clone();

    if (dbShipObj == HSNOTHING)
        hsInterface.Notify(player, "Clone ship - failed.");
    else
        hsInterface.Notify(player,
                           hsInterface.HSPrintf("Clone ship - cloned #%d",
                                                dbShipObj));
}

HSPACE_COMMAND_HDR(hscAddTerritory)
{
    HS_DBREF dbObj;

    // Find the object
    dbObj = hsInterface.NoisyMatchThing(player, arg_left);
    if (dbObj == HSNOTHING)
        return;

    // Check to see if it's already a territory.
    CHSTerritory *territory;
    territory = taTerritories.FindTerritory(dbObj);
    if (territory)
    {
        hsInterface.Notify(player,
                           "That object is already a space territory.");
        return;
    }

    // Proper command usage.
    if (!arg_right || !*arg_right)
    {
        hsInterface.Notify(player, "Must specify type of territory to add.");
        return;
    }

    if (!isdigit(*arg_right) || strlen(arg_right) > 1)
    {
        hsInterface.Notify(player, "Territory type must be either 0 (radial) \
				or 1 (cubic).");
        return;
    }

    // Grab a new territory
    territory = taTerritories.NewTerritory(dbObj, (TERRTYPE) atoi(arg_right));

    if (!territory)
        hsInterface.Notify(player, "Territory - failed.");
    else
        hsInterface.Notify(player,
                           hsInterface.
                           HSPrintf("%s (#%d) added as a new territory.",
                                    hsInterface.GetName(dbObj), dbObj));
}

HSPACE_COMMAND_HDR(hscDelTerritory)
{
    HS_DBREF dbObj;

    dbObj = strtodbref(arg_left);

    // Check to see if it's a territory.
    CHSTerritory *territory;
    territory = taTerritories.FindTerritory(dbObj);
    if (!territory)
    {
        hsInterface.Notify(player, "No such territory with that HS_DBREF.");
        return;
    }

    if (!taTerritories.RemoveTerritory(dbObj))
        hsInterface.Notify(player, "Delete territory - failed.");
    else
        hsInterface.Notify(player, "Delete territory - deleted.");
}

// Allows an admin to add a new weapon to the weapondb
HSPACE_COMMAND_HDR(hscNewWeapon)
{
    // Command usage.
    if (!arg_left || !*arg_left)
    {
        hsInterface.Notify(player,
                           "Must specify the type of weapon to create.");
        return;
    }

    if (!arg_right || !*arg_right)
    {
        hsInterface.Notify(player,
                           "Must specify the name of the new weapon.");
        return;
    }

    // Try to create a weapon data class of the specified weapon class.
    EHSWeaponClass eWeaponClass = (EHSWeaponClass) atoi(arg_left);

    CHSWeaponData *pWeaponData = CHSWeaponData::CreateFromClass(eWeaponClass);
    if (!pWeaponData)
    {
        hsInterface.Notify(player,
                           "Failed to create new weapon by specified weapon class.");
        return;
    }

    // Set the name of the weapon.
    pWeaponData->Name(arg_right);

    // Add the weapon to the global database of weapon types.
    waWeapons.AddWeapon(pWeaponData);

    hsInterface.Notify(player, "Weapon - created.");
}

// Sets an attribute for a specified weapon.
HSPACE_COMMAND_HDR(hscSetAttrWeapon)
{
    HS_UINT32 uiWeaponTypeID;
    HS_INT8 *ptr;
    HS_INT8 name[64];

    // Command format is:
    //
    // @space/setweapon #/attr=value
    //
    if (!arg_left || !*arg_left)
    {
        hsInterface.Notify(player,
                           "You must specify a weapon number and attribute name.");
        return;
    }

    // Pull out the weapon id and attribute name.
    ptr = strchr(arg_left, '/');
    if (!ptr)
    {
        hsInterface.Notify(player, "Invalid weapon/attribute combination.");
        return;
    }

    *ptr = '\0';
    uiWeaponTypeID = atoi(arg_left);
    ptr++;
    strncpy(name, ptr, 62);
    name[63] = '\0';

    // See if the weapon exists.
    CHSWeaponData *pWeaponData;
    pWeaponData = waWeapons.GetWeapon(uiWeaponTypeID);
    if (!pWeaponData)
    {
        hsInterface.Notify(player, "Invalid weapon ID specified.");
        return;
    }

    // Tell the weapon to set the attribute value.
    if (!pWeaponData->SetAttributeValue(name, arg_right))
        hsInterface.Notify(player, "Attribute - failed.");
    else
        hsInterface.Notify(player, "Attribute - set.");
}

HSPACE_COMMAND_HDR(hscAddSys)
{
    HS_DBREF dbObj;
    CHSShip *pShip;
    CHS3DObject *cObj;

    // Parse out the parts of the command.
    // Command format is:
    //
    // @space/sysset obj:system/attr=value
    if (!arg_left || !arg_right || !*arg_right)
    {
        hsInterface.Notify(player,
                           "You must specify an object and system name.");
        return;
    }
    // Pull out the object of interest

    dbObj = hsInterface.NoisyMatchThing(player, arg_left);
    if (dbObj == HSNOTHING)
        return;

    // Find the system based on the name
    HSS_TYPE type;

    type = hsGetEngSystemType(arg_right);
    if (type == HSS_NOTYPE)
    {
        hsInterface.Notify(player, "Invalid system name specified.");
        return;
    }

    if (hsInterface.HasFlag(dbObj, TYPE_THING, THING_HSPACE_CONSOLE))
        cObj = dbHSDB.FindObjectByConsole(dbObj);
    else
        cObj = dbHSDB.FindObject(dbObj);
    if (!cObj)
    {
        hsInterface.Notify(player,
                           "That thing does not appear to be an HSpace object.");
        return;
    }

    // Currently only support ships
    if (cObj->GetType() != HST_SHIP)
    {
        hsInterface.Notify(player,
                           "That HSpace object does not currently support that operation.");
        return;
    }

    pShip = (CHSShip *) cObj;

    // Try to find the system already on the class
    CHSEngSystem *cSys;

    if (type != HSS_FICTIONAL)
    {
        cSys = pShip->GetSystems().GetSystem(type);
        if (cSys)
        {
            hsInterface.Notify(player,
                               "That system already exists for that ship.");
            return;
        }
    }

    // Add the system
    cSys = CHSEngSystem::CreateFromType(type);
    if (!cSys)
    {
        hsInterface.Notify(player,
                           "Failed to add the system to the specified ship.");
        return;
    }

    pShip->GetSystems().AddSystem(cSys);
    hsInterface.Notify(player,
                       hsInterface.HSPrintf("%s system added to the %s.",
                                            cSys->GetName(),
                                            pShip->GetName()));
    cSys->SetOwner(pShip);
}

HSPACE_COMMAND_HDR(hscDelSys)
{
    HS_DBREF dbObj;
    CHSShip *pShip;
    CHS3DObject *cObj;
    CHSEngSystem *cSys;

    // Parse out the parts of the command.
    // Command format is:
    //
    // @space/sysset obj:system/attr=value
    if (!arg_left || !arg_right || !*arg_right)
    {
        hsInterface.Notify(player,
                           "You must specify an object and system name.");
        return;
    }
    // Pull out the object of interest

    dbObj = hsInterface.NoisyMatchThing(player, arg_left);
    if (dbObj == HSNOTHING)
        return;

    if (hsInterface.HasFlag(dbObj, TYPE_THING, THING_HSPACE_CONSOLE))
        cObj = dbHSDB.FindObjectByConsole(dbObj);
    else
        cObj = dbHSDB.FindObject(dbObj);


    if (!cObj)
    {
        hsInterface.Notify(player,
                           "That thing does not appear to be an HSpace object.");
        return;
    }

    // Currently only support ships
    if (cObj->GetType() != HST_SHIP)
    {
        hsInterface.Notify(player,
                           "That HSpace object does not currently support that operation.");
        return;
    }

    pShip = (CHSShip *) cObj;

    // Find the system based on the name
    HSS_TYPE type;

    type = hsGetEngSystemType(arg_right);
    if (type == HSS_NOTYPE || type == HSS_FICTIONAL)
    {
        cSys = pShip->GetSystems().GetSystemByName(arg_right);
        if (!cSys)
        {
            hsInterface.Notify(player, "Invalid system name specified.");
            return;
        }
        else
        {
            type = HSS_FICTIONAL;
        }
    }

    // See if the system is at the ship.

    if (type != HSS_FICTIONAL)
    {
        cSys = pShip->GetSystems().GetSystem(type);
        if (!cSys)
        {
            hsInterface.Notify(player,
                               "That system does not exists for that ship.");
            return;
        }
    }


    HS_INT8 tmp[64];
    sprintf(tmp, "%s", cSys->GetName());

    // Delete the system
    if (pShip->GetSystems().DelSystem(cSys))
        hsInterface.Notify(player,
                           hsInterface.
                           HSPrintf("%s system deleted from the %s.", tmp,
                                    pShip->GetName()));
    else
        hsInterface.Notify(player, "Failed to delete system.");
}

HSPACE_COMMAND_HDR(hscDelSysClass)
{
    HS_UINT32 iClass;
    CHSShipClass *pClass;

    // Find the class
    iClass = atoi(arg_left);
    pClass = CHSClassDB::GetInstance().GetClass(iClass);
    if (!pClass)
    {
        hsInterface.Notify(player, "Ship class non-existent.");
        return;
    }

    // Find the system based on the name
    if (!arg_right || !*arg_right)
    {
        hsInterface.Notify(player, "Must specify a system name to delete.");
        return;
    }

    // Try to find out if the system on the class.
    CHSEngSystem *cSys;
    // Find the system based on the name
    HSS_TYPE type;

    type = hsGetEngSystemType(arg_right);
    if (type == HSS_NOTYPE || type == HSS_FICTIONAL)
    {
        cSys =
            pClass->m_pSystems ? pClass->m_pSystems->
            GetSystemByName(arg_right) : NULL;
        if (!cSys)
        {
            hsInterface.Notify(player, "Invalid system name specified.");
            return;
        }
        else
        {
            type = HSS_FICTIONAL;
        }
    }

    if (type != HSS_FICTIONAL)
    {
        cSys =
            pClass->m_pSystems ? pClass->m_pSystems->GetSystem(type) : NULL;
        if (!cSys)
        {
            hsInterface.Notify(player,
                               "That system does not exists for that class.");
            return;
        }
    }

    HS_INT8 tmp[64];
    sprintf(tmp, "%s", cSys->GetName());

    if (pClass->m_pSystems && pClass->m_pSystems->DelSystem(cSys))
    {
        // Find all ships of this class, and remove this system.
        HS_BOOL8 bIter;
        THSUniverseIterator tIter;
        for (bIter = CHSUniverseDB::GetInstance().GetFirstUniverse(tIter);
             bIter;
             bIter = CHSUniverseDB::GetInstance().GetNextUniverse(tIter))
        {
            CHSUniverse *pUniverse = tIter.pValue;

            // Search ships in the universe
            THSObjectIterator tIterator;
            HS_BOOL8 bContinue;
            for (bContinue = pUniverse->GetFirstObject(tIterator, HST_SHIP);
                 bContinue;
                 bContinue = pUniverse->GetNextObject(tIterator, HST_SHIP))
            {
                if (tIterator.pValue)
                {
                    CHSShip *pShip =
                        static_cast < CHSShip * >(tIterator.pValue);
                    if (pShip->ClassNum() == iClass)
                    {
                        // Tell this ship to remove the system.
                        CHSEngSystem *pSystem =
                            pShip->GetSystems().GetSystem(type);
                        if (pSystem)
                        {
                            pShip->GetSystems().DelSystem(pSystem);
                        }
                    }
                }
            }
        }

        hsInterface.Notify(player,
                           hsInterface.
                           HSPrintf
                           ("%s system deleted from class %d and all ships of this class.",
                            tmp, iClass));
    }
    else
    {
        hsInterface.Notify(player, "Failed to delete system.");
    }
}


/* Special thanks to Chronus for supplying me with this
	piece of code to use in the new patch, thanks Chronus! */
HSPACE_COMMAND_HDR(hscDumpWeapon)
{
    HS_UINT32 uiWeaponTypeID;
    CHSWeaponData *pWeaponData;

    uiWeaponTypeID = atoi(arg_left);

    pWeaponData = waWeapons.GetWeapon(uiWeaponTypeID);

    if (!pWeaponData)
    {
        hsStdError(player, "No such weapon with that weapon ID.");
        return;
    }

    // Print out the weapon data.
    hsInterface.Notify(player,
                       hsInterface.HSPrintf("Weapon: %d    %s",
                                            uiWeaponTypeID,
                                            pWeaponData->Name()));
    hsInterface.Notify(player,
                       "------------------------------------------------------------");

    CHSAttributeList listAttributes;
    pWeaponData->GetAttributeList(listAttributes);

    while (!listAttributes.empty())
    {
        hsInterface.Notify(player,
                           hsInterface.HSPrintf("%s = %s",
                                                listAttributes.front().
                                                c_str(),
                                                pWeaponData->
                                                GetAttributeValue
                                                (listAttributes.front().
                                                 c_str())));

        listAttributes.pop_front();
    }
}

HSPACE_COMMAND_HDR(hscConfig)
{
    if (HSCONF.InputOption(arg_left, arg_right) == true)
    {
        hsInterface.Notify(player, "Set.");
    }
    else
    {
        hsInterface.Notify(player, "Unknown option.");
    }
}

// Adds a room to a ship object
HSPACE_COMMAND_HDR(hscAddHatch)
{
    HS_DBREF dbConsole;
    HS_DBREF dbExit;
    CHSShip *cShip;

    dbConsole = hsInterface.NoisyMatchThing(player, arg_left);
    if (dbConsole == HSNOTHING)
        return;

    dbExit = hsInterface.NoisyMatchExit(player, arg_right);
    if (dbExit == HSNOTHING)
        return;
    if (hsInterface.GetType(dbExit) != TYPE_EXIT)
    {
        hsInterface.Notify(player,
                           "You may only register exits with this command.");
        return;
    }

    // Is the target object a console?  If so, find
    // the ship through the console.
    if (hsInterface.HasFlag(dbConsole, TYPE_THING, THING_HSPACE_CONSOLE))
    {
        CHS3DObject *cObj;
        cObj = dbHSDB.FindObjectByConsole(dbConsole);
        if (cObj && cObj->GetType() != HST_SHIP)
        {
            hsInterface.Notify(player, "That is not a HSpace ship.");
            return;
        }
        cShip = (CHSShip *) cObj;
    }
    else
    {
        cShip = dbHSDB.FindShip(dbConsole);
    }

    if (!cShip)
    {
        hsInterface.Notify(player, "That is not a HSpace ship.");
        return;
    }

    if (cShip->AddHatch(dbExit))
        hsInterface.Notify(player, "Hatch - added.");
    else
        hsInterface.Notify(player, "Hatch - failed.");
}

HSPACE_COMMAND_HDR(hscDeleteHatch)
{
    HS_DBREF dbConsole;
    HS_DBREF dbExit;
    CHSShip *cShip;

    dbConsole = hsInterface.NoisyMatchThing(player, arg_left);
    if (dbConsole == HSNOTHING)
        return;

    dbExit = hsInterface.NoisyMatchExit(player, arg_right);
    if (dbExit == HSNOTHING)
        return;
    if (hsInterface.GetType(dbExit) != TYPE_EXIT)
    {
        hsInterface.Notify(player,
                           "You may only remove exits with this command.");
        return;
    }

    // Is the target object a console?  If so, find
    // the ship through the console.
    if (hsInterface.HasFlag(dbConsole, TYPE_THING, THING_HSPACE_CONSOLE))
    {
        CHS3DObject *cObj;
        cObj = dbHSDB.FindObjectByConsole(dbConsole);
        if (cObj && cObj->GetType() != HST_SHIP)
        {
            hsInterface.Notify(player, "That is not a HSpace ship.");
            return;
        }
        cShip = (CHSShip *) cObj;
    }
    else
    {
        cShip = dbHSDB.FindShip(dbConsole);
    }

    if (!cShip)
    {
        hsInterface.Notify(player, "That is not a HSpace ship.");
        return;
    }

    if (cShip->DeleteHatch(dbExit))
        hsInterface.Notify(player, "Hatch removed.");
    else
        hsInterface.Notify(player, "Hatch removal failed.");

}

HSPACE_COMMAND_HDR(hscAddMessage)
{
    CHSConsole *cConsole;
    HS_DBREF dbObj;
    HS_BOOL8 Success = false;

    // Find the game object representing the console.
    dbObj = hsInterface.NoisyMatchThing(player, arg_left);
    if (dbObj == HSNOTHING)
        return;

    // See if it's a console.
    cConsole = dbHSDB.FindConsole(dbObj);
    if (!cConsole)
    {
        hsInterface.Notify(player, "That is not a valid console.");
        return;
    }

    // Check the type to turn it into
    if (!arg_right || !*arg_right)
    {
        hsInterface.Notify(player, "You must specify a type of message.");
        return;
    }

    // Figure out the message type.
    switch (atoi(arg_right))
    {
    case MSG_GENERAL:
        hsInterface.Notify(player, "All consoles hear general messages.");
        return;

    case MSG_SENSOR:
        Success = cConsole->AddMessage(MSG_SENSOR);
        break;

    case MSG_ENGINEERING:
        Success = cConsole->AddMessage(MSG_ENGINEERING);
        break;

    case MSG_COMBAT:
        Success = cConsole->AddMessage(MSG_COMBAT);
        break;

    case MSG_COMMUNICATION:
        Success = cConsole->AddMessage(MSG_COMMUNICATION);
        break;

    default:
        hsInterface.Notify(player, "Invalid type of message specified.");
        return;
    }

    if (Success)
    {
        hsInterface.Notify(player,
                           hsInterface.HSPrintf("%s - Message type %d added.",
                                                hsInterface.GetName(dbObj),
                                                atoi(arg_right)));
    }
    else
    {
        hsInterface.Notify(player,
                           hsInterface.
                           HSPrintf("%s - Failed to add message type.",
                                    hsInterface.GetName(dbObj)));
    }
}

HSPACE_COMMAND_HDR(hscDelMessage)
{
    CHSConsole *cConsole;
    HS_DBREF dbObj;
    HS_BOOL8 Success = false;

    // Find the game object representing the console.
    dbObj = hsInterface.NoisyMatchThing(player, arg_left);
    if (dbObj == HSNOTHING)
        return;

    // See if it's a console.
    cConsole = dbHSDB.FindConsole(dbObj);
    if (!cConsole)
    {
        hsInterface.Notify(player, "That is not a valid console.");
        return;
    }

    // Check the type to turn it into
    if (!arg_right || !*arg_right)
    {
        hsInterface.Notify(player, "You must specify a type of message.");
        return;
    }

    // Figure out the message type.
    switch (atoi(arg_right))
    {
    case MSG_GENERAL:
        hsInterface.Notify(player, "All consoles hear general messages.");
        return;

    case MSG_SENSOR:
        Success = cConsole->DelMessage(MSG_SENSOR);
        break;

    case MSG_ENGINEERING:
        Success = cConsole->DelMessage(MSG_ENGINEERING);
        break;

    case MSG_COMBAT:
        Success = cConsole->DelMessage(MSG_COMBAT);
        break;

    case MSG_COMMUNICATION:
        Success = cConsole->DelMessage(MSG_COMMUNICATION);
        break;

    default:
        hsInterface.Notify(player, "Invalid type of message specified.");
        return;
    }

    if (Success)
        hsInterface.Notify(player,
                           hsInterface.
                           HSPrintf("%s - Message type %d deleted.",
                                    hsInterface.GetName(dbObj),
                                    atoi(arg_right)));
    else
        hsInterface.Notify(player,
                           hsInterface.
                           HSPrintf("%s - Failed to delete message type.",
                                    hsInterface.GetName(dbObj)));

}
