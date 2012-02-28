#include "pch.h"

#include <cstdio>
#include <stdlib.h>

#include "hsobjects.h"
#include "hscmds.h"
#include "hsdb.h"
#include "hseng.h"
#include "hsinterface.h"
#include "hsutils.h"
#include "hsconf.h"
#include "hstractor.h"
#include "hsautopilot.h"


HSPACE_COMMAND_PROTO(hscSetVelocity)
HSPACE_COMMAND_PROTO(hscSetHeading)
HSPACE_COMMAND_PROTO(hscSetRoll)
HSPACE_COMMAND_PROTO(hscSensorReport)
HSPACE_COMMAND_PROTO(hscNavStatus)
HSPACE_COMMAND_PROTO(hscSetMapRange)
HSPACE_COMMAND_PROTO(hscLandVessel)
HSPACE_COMMAND_PROTO(hscUndockVessel)
HSPACE_COMMAND_PROTO(hscDoAfterburn)
HSPACE_COMMAND_PROTO(hscDoJump)
HSPACE_COMMAND_PROTO(hscDoWarp)
HSPACE_COMMAND_PROTO(hscDoBoardLink)
HSPACE_COMMAND_PROTO(hscBreakBoardLink)
HSPACE_COMMAND_PROTO(hscTaxiVessel)
HSPACE_COMMAND_PROTO(hscDoView)
HSPACE_COMMAND_PROTO(hscScan)
HSPACE_COMMAND_PROTO(hscDoCloak)
HSPACE_COMMAND_PROTO(hscDoSView)
HSPACE_COMMAND_PROTO(hscDoGate)
HSPACE_COMMAND_PROTO(hscHatchStat)
HSPACE_COMMAND_PROTO(hscTractorMode)
HSPACE_COMMAND_PROTO(hscTractorLock)
HSPACE_COMMAND_PROTO(hscTractorUnLock)
HSPACE_COMMAND_PROTO(hscTractorDock) 
HSPACE_COMMAND_PROTO(hscAutoPilot)

// The hsNavCommandArray holds all externally callable 
// @nav commands.
HSPACE_COMMAND hsNavCommandArray[] = {
    {"AFTERBURN", hscDoAfterburn, HCP_ANY},
    {"AUTOPILOT", hscAutoPilot, HCP_ANY},
    {"BOARDLINK", hscDoBoardLink, HCP_ANY},
    {"BOARDUNLINK", hscBreakBoardLink, HCP_ANY},
    {"CLOAK", hscDoCloak, HCP_ANY},
    {"GATE", hscDoGate, HCP_ANY},
    {"JUMP", hscDoJump, HCP_ANY},
    {"HSTAT", hscHatchStat, HCP_ANY}, 
    {"LAND", hscLandVessel, HCP_ANY},
    {"MAPRANGE", hscSetMapRange, HCP_ANY},
    {"SCAN", hscScan, HCP_ANY},
    {"SENSORREPORT", hscSensorReport, HCP_ANY},
    {"SETHEADING", hscSetHeading, HCP_ANY},
    {"SETROLL", hscSetRoll, HCP_ANY},
    {"SETVELOCITY", hscSetVelocity, HCP_ANY},
    {"STATUS", hscNavStatus, HCP_ANY},
    {"SVIEW", hscDoSView, HCP_ANY},
    {"TAXI", hscTaxiVessel, HCP_ANY},
    {"TRACTORDOCK", hscTractorDock, HCP_ANY},
    {"TRACTORLOCK", hscTractorLock, HCP_ANY},
    {"TRACTORMODE", hscTractorMode, HCP_ANY},
    {"TRACTORUNLOCK", hscTractorUnLock, HCP_ANY},
    {"UNDOCK", hscUndockVessel, HCP_ANY},
    {"VIEW", hscDoView, HCP_ANY},
    {"WARP", hscDoWarp, HCP_ANY},
    {NULL, NULL, 0}
};

// Sets the velocity of a vessel
HSPACE_COMMAND_HDR(hscSetVelocity)
{
    HS_DBREF dbUser;
    CHSShip *cShip;
    int iVel;

    // Find the ship based on the console
    cShip = (CHSShip *) dbHSDB.FindObjectByConsole(player);
    if (!cShip || (cShip->GetType() != HST_SHIP))
    {
        hsInterface.Notify(player, "Go away!  You are not an HSpace ship!");
        return;
    }

    // Find the user of the console.
    dbUser = hsInterface.ConsoleUser(player);
    if (dbUser == HSNOTHING)
        return;

    // Is the ship destroyed?
    if (cShip->IsDestroyed())
    {
        hsInterface.Notify(dbUser, "This vessel has been destroyed!");
        return;
    }

    // Find the console object
    CHSConsole *cConsole;
    cConsole = cShip->FindConsole(player);
    if (cConsole && !cConsole->IsOnline())
    {
        hsStdError(dbUser,
                   hsInterface.HSPrintf("%s is currently powered down.",
                                        hsInterface.GetName(player)));
        return;
    }

    CHSSysAutoPilot *cAutoPilot =
        (CHSSysAutoPilot *) cShip->GetSystems().GetSystem(HSS_AUTOPILOT);
    if (cAutoPilot)
    {
        if (cAutoPilot->IsEngaged())
            cShip->EngageAutoPilot(dbUser, false);
    }

    iVel = atoi(arg_left);
    cShip->SetVelocity(dbUser, iVel);
}

// Sets the heading of a vessel
HSPACE_COMMAND_HDR(hscSetHeading)
{
    HS_DBREF dbUser;
    CHSShip *cShip;
    int xy, z;

    // Find the ship based on the console
    cShip = (CHSShip *) dbHSDB.FindObjectByConsole(player);
    if (!cShip || (cShip->GetType() != HST_SHIP))
    {
        hsInterface.Notify(player, "Go away!  You are not an HSpace ship!");
        return;
    }

    // Find the user of the console.
    dbUser = hsInterface.ConsoleUser(player);
    if (dbUser == HSNOTHING)
        return;

    // Is the ship destroyed?
    if (cShip->IsDestroyed())
    {
        hsInterface.Notify(dbUser, "This vessel has been destroyed!");
        return;
    }

    // Find the console object
    CHSConsole *cConsole;
    cConsole = cShip->FindConsole(player);
    if (cConsole && !cConsole->IsOnline())
    {
        hsStdError(dbUser,
                   hsInterface.HSPrintf("%s is currently powered down.",
                                        hsInterface.GetName(player)));
        return;
    }

    if (!arg_left || !*arg_left || !arg_right || !*arg_right)
    {
        hsStdError(dbUser, "You must specify a XY and Z heading.");
        return;
    }

    CHSSysAutoPilot *cAutoPilot =
        (CHSSysAutoPilot *) cShip->GetSystems().GetSystem(HSS_AUTOPILOT);
    if (cAutoPilot)
    {
        if (cAutoPilot->IsEngaged())
            cShip->EngageAutoPilot(dbUser, false);
    }

    xy = atoi(arg_left);
    z = atoi(arg_right);

    cShip->SetHeading(dbUser, xy, z);
}

// Sets the roll of a vessel
HSPACE_COMMAND_HDR(hscSetRoll)
{
    HS_DBREF dbUser;
    CHSShip *cShip;
    int roll;

    // Find the ship based on the console
    cShip = (CHSShip *) dbHSDB.FindObjectByConsole(player);
    if (!cShip || (cShip->GetType() != HST_SHIP))
    {
        hsInterface.Notify(player, "Go away!  You are not an HSpace ship!");
        return;
    }

    // Find the user of the console.
    dbUser = hsInterface.ConsoleUser(player);
    if (dbUser == HSNOTHING)
        return;

    // Is the ship destroyed?
    if (cShip->IsDestroyed())
    {
        hsInterface.Notify(dbUser, "This vessel has been destroyed!");
        return;
    }

    // Find the console object
    CHSConsole *cConsole;
    cConsole = cShip->FindConsole(player);
    if (cConsole && !cConsole->IsOnline())
    {
        hsStdError(dbUser,
                   hsInterface.HSPrintf("%s is currently powered down.",
                                        hsInterface.GetName(player)));
        return;
    }

    if (!arg_left)
        roll = HSNOTHING;
    else
        roll = atoi(arg_left);

    cShip->SetHeading(dbUser, roll);
}


// Prints out a sensor report, perhaps for a specific object type.
HSPACE_COMMAND_HDR(hscSensorReport)
{
    HS_DBREF dbUser;
    CHSShip *cShip;

    // Find the ship based on the console
    cShip = (CHSShip *) dbHSDB.FindObjectByConsole(player);
    if (!cShip || (cShip->GetType() != HST_SHIP))
    {
        hsInterface.Notify(player, "Go away!  You are not an HSpace ship!");
        return;
    }

    // Find the user of the console.
    dbUser = hsInterface.ConsoleUser(player);
    if (dbUser == HSNOTHING)
        return;

    // Is the ship destroyed?
    if (cShip->IsDestroyed())
    {
        hsInterface.Notify(dbUser, "This vessel has been destroyed!");
        return;
    }

    // Find the console object
    CHSConsole *cConsole;
    cConsole = cShip->FindConsole(player);
    if (cConsole && !cConsole->IsOnline())
    {
        hsStdError(dbUser,
                   hsInterface.HSPrintf("%s is currently powered down.",
                                        hsInterface.GetName(player)));
        return;
    }

    if (!arg_left || !*arg_left)
        cShip->GiveSensorReport(dbUser);
    else
        cShip->GiveSensorReport(dbUser, (HS_TYPE) atoi(arg_left));
}

// Gives the big navigation status display
HSPACE_COMMAND_HDR(hscNavStatus)
{
    HS_DBREF dbUser;
    CHSShip *cShip;

    // Find the ship based on the console
    cShip = (CHSShip *) dbHSDB.FindObjectByConsole(player);
    if (!cShip || (cShip->GetType() != HST_SHIP))
    {
        hsInterface.Notify(player, "Go away!  You are not an HSpace ship!");
        return;
    }

    // Find the user of the console.
    dbUser = hsInterface.ConsoleUser(player);
    if (dbUser == HSNOTHING)
        return;

    // Is the ship destroyed?
    if (cShip->IsDestroyed())
    {
        hsInterface.Notify(dbUser, "This vessel has been destroyed!");
        return;
    }

    // Find the console object
    CHSConsole *cConsole;
    cConsole = cShip->FindConsole(player);
    if (cConsole && !cConsole->IsOnline())
    {
        hsStdError(dbUser,
                   hsInterface.HSPrintf("%s is currently powered down.",
                                        hsInterface.GetName(player)));
        return;
    }

    cShip->GiveNavigationStatus(dbUser);
}

// Allows the player to set the ship's map range
HSPACE_COMMAND_HDR(hscSetMapRange)
{
    HS_DBREF dbUser;
    CHSShip *cShip;
    char tbuf[64];
    int units;

    // Find the ship based on the console
    cShip = (CHSShip *) dbHSDB.FindObjectByConsole(player);
    if (!cShip || (cShip->GetType() != HST_SHIP))
    {
        hsInterface.Notify(player, "Go away!  You are not an HSpace ship!");
        return;
    }

    // Find the user of the console.
    dbUser = hsInterface.ConsoleUser(player);
    if (dbUser == HSNOTHING)
        return;

    // Is the ship destroyed?
    if (cShip->IsDestroyed())
    {
        hsInterface.Notify(dbUser, "This vessel has been destroyed!");
        return;
    }

    // Find the console object
    CHSConsole *cConsole;
    cConsole = cShip->FindConsole(player);
    if (cConsole && !cConsole->IsOnline())
    {
        hsStdError(dbUser,
                   hsInterface.HSPrintf("%s is currently powered down.",
                                        hsInterface.GetName(player)));
        return;
    }

    units = atoi(arg_left);
    if (!cShip->SetMapRange(dbUser, units))
        hsStdError(dbUser, "Failed to set map range.");
    else
    {
        sprintf_s(tbuf, "Map range now set to %d %s.", units, HSCONF.unit_name);
        hsStdError(dbUser, tbuf);
    }
}

// Allows a player to land the vessel on some object, at some
// location, with a code (if needed)
HSPACE_COMMAND_HDR(hscLandVessel)
{
    HS_DBREF dbUser;
    CHSShip *cShip;
    char *sptr, *dptr;
    int id;
    char tbuf[64];

    // Find the ship based on the console
    cShip = (CHSShip *) dbHSDB.FindObjectByConsole(player);
    if (!cShip || (cShip->GetType() != HST_SHIP))
    {
        hsInterface.Notify(player, "Go away!  You are not an HSpace ship!");
        return;
    }

    // Find the user of the console.
    dbUser = hsInterface.ConsoleUser(player);
    if (dbUser == HSNOTHING)
        return;

    // Is the ship destroyed?
    if (cShip->IsDestroyed())
    {
        hsInterface.Notify(dbUser, "This vessel has been destroyed!");
        return;
    }

    // Find the console object
    CHSConsole *cConsole;
    cConsole = cShip->FindConsole(player);
    if (cConsole && !cConsole->IsOnline())
    {
        hsStdError(dbUser,
                   hsInterface.HSPrintf("%s is currently powered down.",
                                        hsInterface.GetName(player)));
        return;
    }

    if (!arg_left || !*arg_left)
    {
        hsInterface.Notify(dbUser, "You must specify a target ID/location.");
        return;
    }

    // The format of the landing command is:
    //
    // @nav/land ID/loc=code
    //
    // Parse out this info
    dptr = tbuf;
    for (sptr = arg_left; *sptr; sptr++)
    {
        if (*sptr == '/')
        {
            *dptr = '\0';
            id = atoi(tbuf);
            break;
        }
        else
            *dptr++ = *sptr;
    }

    // Was a location specified?
    if (!*sptr)
    {
        hsInterface.Notify(player,
                           "You must specify the number of a location at that target.");
        return;
    }

    // Skip the slash
    sptr++;
    dptr = tbuf;
    while (*sptr)
    {
        *dptr++ = *sptr++;
    }
    *dptr = '\0';

    // Tell the ship to land.
    cShip->LandVessel(dbUser, id, tbuf, arg_right);
}

// Allows the player to undock/lift off the vessel
HSPACE_COMMAND_HDR(hscUndockVessel)
{
    HS_DBREF dbUser;
    CHSShip *cShip;

    // Find the ship based on the console
    cShip = (CHSShip *) dbHSDB.FindObjectByConsole(player);
    if (!cShip || (cShip->GetType() != HST_SHIP))
    {
        hsInterface.Notify(player, "Go away!  You are not an HSpace ship!");
        return;
    }

    // Find the user of the console.
    dbUser = hsInterface.ConsoleUser(player);
    if (dbUser == HSNOTHING)
        return;

    // Is the ship destroyed?
    if (cShip->IsDestroyed())
    {
        hsInterface.Notify(dbUser, "This vessel has been destroyed!");
        return;
    }

    // Find the console object
    CHSConsole *cConsole;
    cConsole = cShip->FindConsole(player);
    if (cConsole && !cConsole->IsOnline())
    {
        hsStdError(dbUser,
                   hsInterface.HSPrintf("%s is currently powered down.",
                                        hsInterface.GetName(player)));
        return;
    }

    if (cShip->GetMannedConsoles() < cShip->GetMinManned())
    {
        hsStdError(dbUser,
                   hsInterface.
                   HSPrintf
                   ("Only %d out of a minimum of %d consoles are manned.",
                    cShip->GetMannedConsoles(), cShip->GetMinManned()));
        return;
    }

    cShip->UndockVessel(dbUser);
}

// Allows a player to engage/disengage the afterburners.
HSPACE_COMMAND_HDR(hscDoAfterburn)
{
    HS_DBREF dbUser;
    CHSShip *cShip;

    // Find the ship based on the console
    cShip = (CHSShip *) dbHSDB.FindObjectByConsole(player);
    if (!cShip || (cShip->GetType() != HST_SHIP))
    {
        hsInterface.Notify(player, "Go away!  You are not an HSpace ship!");
        return;
    }

    // Find the user of the console.
    dbUser = hsInterface.ConsoleUser(player);
    if (dbUser == HSNOTHING)
        return;

    // Is the ship destroyed?
    if (cShip->IsDestroyed())
    {
        hsInterface.Notify(dbUser, "This vessel has been destroyed!");
        return;
    }

    // Find the console object
    CHSConsole *cConsole;
    cConsole = cShip->FindConsole(player);
    if (cConsole && !cConsole->IsOnline())
    {
        hsStdError(dbUser,
                   hsInterface.HSPrintf("%s is currently powered down.",
                                        hsInterface.GetName(player)));
        return;
    }

    if (!arg_left || !*arg_left)
    {
        hsInterface.Notify(dbUser,
                           "You must specify 0 or 1 for afterburn status.");
        return;
    }
    cShip->EngageAfterburn(dbUser, atoi(arg_left) == 0 ? false : true);
}

// Allows a player to engage/disengage the jump drive.
HSPACE_COMMAND_HDR(hscDoJump)
{
    HS_DBREF dbUser;
    CHSShip *cShip;

    // Find the ship based on the console
    cShip = (CHSShip *) dbHSDB.FindObjectByConsole(player);
    if (!cShip || (cShip->GetType() != HST_SHIP))
    {
        hsInterface.Notify(player, "Go away!  You are not an HSpace ship!");
        return;
    }

    // Find the user of the console.
    dbUser = hsInterface.ConsoleUser(player);
    if (dbUser == HSNOTHING)
        return;

    // Is the ship destroyed?
    if (cShip->IsDestroyed())
    {
        hsInterface.Notify(dbUser, "This vessel has been destroyed!");
        return;
    }

    // Find the console object
    CHSConsole *cConsole;
    cConsole = cShip->FindConsole(player);
    if (cConsole && !cConsole->IsOnline())
    {
        hsStdError(dbUser,
                   hsInterface.HSPrintf("%s is currently powered down.",
                                        hsInterface.GetName(player)));
        return;
    }

    if (!arg_left || !*arg_left)
    {
        hsInterface.Notify(dbUser,
                           "You must specify 0 or 1 for jump drive engage status.");
        return;
    }
    cShip->EngageJumpDrive(dbUser, atoi(arg_left) == 0 ? false : true);
}

// Allows a player to engage/disengage the warp drive.
HSPACE_COMMAND_HDR(hscDoWarp)
{
    HS_DBREF dbUser;
    CHSShip *cShip;

    // Find the ship based on the console
    cShip = (CHSShip *) dbHSDB.FindObjectByConsole(player);
    if (!cShip || (cShip->GetType() != HST_SHIP))
    {
        hsInterface.Notify(player, "Go away!  You are not an HSpace ship!");
        return;
    }

    // Find the user of the console.
    dbUser = hsInterface.ConsoleUser(player);
    if (dbUser == HSNOTHING)
        return;

    // Is the ship destroyed?
    if (cShip->IsDestroyed())
    {
        hsInterface.Notify(dbUser, "This vessel has been destroyed!");
        return;
    }

    // Find the console object
    CHSConsole *cConsole;
    cConsole = cShip->FindConsole(player);
    if (cConsole && !cConsole->IsOnline())
    {
        hsStdError(dbUser,
                   hsInterface.HSPrintf("%s is currently powered down.",
                                        hsInterface.GetName(player)));
        return;
    }

    if (!arg_left || !*arg_left || atof(arg_left) < 0)
    {
        hsInterface.Notify(dbUser,
            "You must specify 0 to 9.9999 for warp drive engage status.");
        return;
    }
    
    cShip->EngageWarpDrive(dbUser, atof(arg_left)); 
}

HSPACE_COMMAND_HDR(hscDoBoardLink)
{
    HS_DBREF dbUser;
    CHSShip *cShip;
    char left_switch[64];
    char right_switch[64];
    char *ptr, *dptr;


    // Find the ship based on the console
    cShip = (CHSShip *) dbHSDB.FindObjectByConsole(player);
    if (!cShip || (cShip->GetType() != HST_SHIP))
    {
        hsInterface.Notify(player, "Go away!  You are not an HSpace ship!");
        return;
    }

    // We have to pull out the left and right parts of
    // the @space/set <console>/<attrib> format.
    dptr = left_switch;
    for (ptr = arg_left; *ptr && *ptr != '/'; ptr++)
    {
        // Check for buffer overflow
        if ((dptr - left_switch) > 62)
        {
            hsInterface.Notify(player,
                               "Invalid contact/local hatch pair supplied.");
            return;
        }

        *dptr++ = *ptr;
    }
    *dptr = '\0';

    // Right side.
    if (!*ptr)
    {
        hsInterface.Notify(player,
                           "Invalid contact/local hatch pair supplied.");
        return;
    }
    ptr++;

    dptr = right_switch;
    while (*ptr)
    {
        *dptr++ = *ptr++;
    }
    *dptr = '\0';

    // Find the user of the console.
    dbUser = hsInterface.ConsoleUser(player);
    if (dbUser == HSNOTHING)
        return;

    // Is the ship destroyed?
    if (cShip->IsDestroyed())
    {
        hsInterface.Notify(dbUser, "This vessel has been destroyed!");
        return;
    }

    // Find the console object
    CHSConsole *cConsole;
    cConsole = cShip->FindConsole(player);
    if (cConsole && !cConsole->IsOnline())
    {
        hsStdError(dbUser,
                   hsInterface.HSPrintf("%s is currently powered down.",
                                        hsInterface.GetName(player)));
        return;
    }

    if (!arg_right || !*arg_right)
    {
        hsInterface.Notify(dbUser, "You must specify a target hatch.");
        return;
    }

    if (!left_switch || !*left_switch)
    {
        hsInterface.Notify(dbUser, "You must specify a target contact.");
        return;
    }
    if (!right_switch || !*right_switch)
    {
        hsInterface.Notify(dbUser, "You must specify a local hatch.");
        return;
    }
    // Tell the ship to board link.
    cShip->DoBoardLink(dbUser, atoi(left_switch), atoi(right_switch),
                       atoi(arg_right));
}

HSPACE_COMMAND_HDR(hscBreakBoardLink)
{
    HS_DBREF dbUser;
    CHSShip *cShip;

    // Find the ship based on the console
    cShip = (CHSShip *) dbHSDB.FindObjectByConsole(player);
    if (!cShip || (cShip->GetType() != HST_SHIP))
    {
        hsInterface.Notify(player, "Go away!  You are not an HSpace ship!");
        return;
    }

    // Find the user of the console.
    dbUser = hsInterface.ConsoleUser(player);
    if (dbUser == HSNOTHING)
        return;

    // Is the ship destroyed?
    if (cShip->IsDestroyed())
    {
        hsInterface.Notify(dbUser, "This vessel has been destroyed!");
        return;
    }

    // Find the console object
    CHSConsole *cConsole;
    cConsole = cShip->FindConsole(player);
    if (cConsole && !cConsole->IsOnline())
    {
        hsStdError(dbUser,
                   hsInterface.HSPrintf("%s is currently powered down.",
                                        hsInterface.GetName(player)));
        return;
    }

    if (!arg_left || !*arg_left)
    {
        hsInterface.Notify(dbUser,
                           "You must specify a target contact number.");
        return;
    }

    // Tell the ship to board link.
    cShip->DoBreakBoardLink(dbUser, atoi(arg_left));
}

HSPACE_COMMAND_HDR(hscTaxiVessel)
{
    HS_DBREF dbUser;
    CHSShip *cShip;
    HS_DBREF dbObj;
    HS_DBREF dbExit;

    // Find the ship based on the console
    cShip = (CHSShip *) dbHSDB.FindObjectByConsole(player);
    if (!cShip || (cShip->GetType() != HST_SHIP))
    {
        hsInterface.Notify(player, "Go away!  You are not an HSpace ship!");
        return;
    }

    // Find the user of the console.
    dbUser = hsInterface.ConsoleUser(player);
    if (dbUser == HSNOTHING)
        return;

    // Is the ship destroyed?
    if (cShip->IsDestroyed())
    {
        hsInterface.Notify(dbUser, "This vessel has been destroyed!");
        return;
    }

    if (!arg_left || !*arg_left)
    {
        hsInterface.Notify(dbUser, "You must specify a direction to taxi.");
        return;
    }

    // Find the console object
    CHSConsole *cConsole;
    cConsole = cShip->FindConsole(player);
    if (cConsole && !cConsole->IsOnline())
    {
        hsStdError(dbUser,
                   hsInterface.HSPrintf("%s is currently powered down.",
                                        hsInterface.GetName(player)));
        return;
    }

    // Ship must not be in space
    if (!cShip->Landed())
    {
        hsStdError(dbUser, "Must be landed or docked to taxi.");
        return;
    }

    // Grab the ship's shipobj
    dbObj = cShip->GetDbref();
    if (dbObj == HSNOTHING)
    {
        hsInterface.Notify(dbUser,
                           "Your ship has no ship object to taxi around.");
        return;
    }

    // Find the exit
    dbExit = hsInterface.NoisyMatchExit(dbObj, arg_left);
    if (dbExit == HSNOTHING || hsInterface.GetType(dbExit) != TYPE_EXIT)
    {
        hsInterface.Notify(dbUser, "That is not a valid direction to taxi.");
        return;
    }

    // Try to move the ship object
    if (hsInterface.GetLocation(dbExit) == HSNOTHING)
    {
        hsInterface.Notify(dbUser,
                           "That location leads to nowhere.  You don't want to go there.");
        return;
    }

    // See if the object passes the lock
    if (!hsInterface.PassesLock(dbObj, dbExit, LOCK_NORMAL))
    {
        hsStdError(dbUser, "That location is blocked to our departure.");
        return;
    }
    hsInterface.MoveObject(dbObj, hsInterface.GetLocation(dbExit));

    hsInterface.NotifyContents(hsInterface.GetLocation(dbObj),
                               hsInterface.
                               HSPrintf
                               ("The %s fires is vector rockets and taxis to %s.",
                                cShip->GetName(),
                                hsInterface.GetName(hsInterface.
                                                    GetLocation(dbExit))));
    hsInterface.MoveObject(dbObj, hsInterface.GetLocation(dbExit));
    hsInterface.NotifyContents(hsInterface.GetLocation(dbExit),
                               hsInterface.
                               HSPrintf
                               ("The %s taxis in, firing its vector rockets.",
                                cShip->GetName()));
    hsStdError(dbUser, "You fire the vector rockets and taxi onward.");
    hsInterface.LookInRoom(dbUser, hsInterface.GetLocation(dbExit), 0);
}

HSPACE_COMMAND_HDR(hscDoView)
{
    HS_DBREF dbUser;
    CHSShip *cShip;
    HS_DBREF dbObj;
    HS_DBREF dbLoc;

    // Find the ship based on the console
    cShip = (CHSShip *) dbHSDB.FindObjectByConsole(player);
    if (!cShip || (cShip->GetType() != HST_SHIP))
    {
        hsInterface.Notify(player, "Go away!  You are not an HSpace ship!");
        return;
    }

    // Find the user of the console.
    dbUser = hsInterface.ConsoleUser(player);
    if (dbUser == HSNOTHING)
        return;

    // Is the ship destroyed?
    if (cShip->IsDestroyed())
    {
        hsInterface.Notify(dbUser, "This vessel has been destroyed!");
        return;
    }

    // Ship must not be in space
    if (false == cShip->Landed())
    {
        hsStdError(dbUser, "This vessel is not currently landed.");
        return;
    }

    // Grab the ship's shipobj
    dbObj = cShip->GetDbref();
    if (dbObj == HSNOTHING)
    {
        hsInterface.Notify(dbUser,
                           "Your ship has no ship object.  Don't know where you're landed.");
        return;
    }

    // Find the location of the shipobj
    dbLoc = hsInterface.GetLocation(dbObj);

    hsInterface.Notify(dbUser, "Outside the vessel you see ...");
    hsInterface.LookInRoom(dbUser, dbLoc, 0);
}

// Allows a player to scan an object on sensors
HSPACE_COMMAND_HDR(hscScan)
{
    HS_DBREF dbUser;
    CHSShip *cShip;

    // Find the ship based on the console
    cShip = (CHSShip *) dbHSDB.FindObjectByConsole(player);
    if (!cShip || (cShip->GetType() != HST_SHIP))
    {
        hsInterface.Notify(player, "Go away!  You are not an HSpace ship!");
        return;
    }

    // Find the user of the console.
    dbUser = hsInterface.ConsoleUser(player);
    if (dbUser == HSNOTHING)
        return;

    // Is the ship destroyed?
    if (cShip->IsDestroyed())
    {
        hsInterface.Notify(dbUser, "This vessel has been destroyed!");
        return;
    }

    // Find the console object
    CHSConsole *cConsole;
    cConsole = cShip->FindConsole(player);
    if (cConsole && !cConsole->IsOnline())
    {
        hsStdError(dbUser,
                   hsInterface.HSPrintf("%s is currently powered down.",
                                        hsInterface.GetName(player)));
        return;
    }

    if (!arg_left || !*arg_left)
    {
        hsInterface.Notify(dbUser,
                           "You must specify a target contact number.");
        return;
    }

    // Tell the ship to board link.
    cShip->ScanObjectID(dbUser, atoi(arg_left));
}

HSPACE_COMMAND_HDR(hscAutoPilot)
{
    HS_DBREF dbUser;
    CHSShip *cShip;

    // Find the ship based on the console
    cShip = (CHSShip *) dbHSDB.FindObjectByConsole(player);
    if (!cShip || (cShip->GetType() != HST_SHIP))
    {
        hsInterface.Notify(player, "Go away! You are not an HSpace ship!");
        return;
    }

    // Find the user on the console.
    dbUser = hsInterface.ConsoleUser(player);
    if (dbUser == HSNOTHING)
        return;

    // Is the ship destroyed?
    if (cShip->IsDestroyed())
    {
        hsInterface.Notify(dbUser, "This vessel has been destroyed!");
        return;
    }

    // Find the console object
    CHSConsole *cConsole;
    cConsole = cShip->FindConsole(player);
    if (cConsole && !cConsole->IsOnline())
    {
        hsStdError(dbUser,
                   hsInterface.HSPrintf("%s is currently powered down.",
                                        hsInterface.GetName(player)));
        return;
    }

    if (!arg_left || !*arg_left)
    {
        hsInterface.Notify(dbUser,
                           "You must specify 0 or 1 for autopilot engage status.");
        return;
    }
    cShip->EngageAutoPilot(dbUser, atoi(arg_left) == 0 ? false : true);
}

HSPACE_COMMAND_HDR(hscDoCloak)
{
    HS_DBREF dbUser;
    CHSShip *cShip;

    // Find the ship based on the console
    cShip = (CHSShip *) dbHSDB.FindObjectByConsole(player);
    if (!cShip || (cShip->GetType() != HST_SHIP))
    {
        hsInterface.Notify(player, "Go away!  You are not an HSpace ship!");
        return;
    }

    // Find the user of the console.
    dbUser = hsInterface.ConsoleUser(player);
    if (dbUser == HSNOTHING)
        return;

    // Is the ship destroyed?
    if (cShip->IsDestroyed())
    {
        hsInterface.Notify(dbUser, "This vessel has been destroyed!");
        return;
    }

    // Find the console object
    CHSConsole *cConsole;
    cConsole = cShip->FindConsole(player);
    if (cConsole && !cConsole->IsOnline())
    {
        hsStdError(dbUser,
                   hsInterface.HSPrintf("%s is currently powered down.",
                                        hsInterface.GetName(player)));
        return;
    }

    if (!arg_left || !*arg_left)
    {
        hsInterface.Notify(dbUser,
                           "You must specify 0 or 1 for cloaking device engage status.");
        return;
    }
    cShip->EngageCloak(dbUser, atoi(arg_left) == 0 ? false : true);
}

HSPACE_COMMAND_HDR(hscDoSView)
{
    HS_DBREF dbUser;
    CHSShip *cShip;

    // Find the ship based on the console
    cShip = (CHSShip *) dbHSDB.FindObjectByConsole(player);
    if (!cShip || (cShip->GetType() != HST_SHIP))
    {
        hsInterface.Notify(player, "Go away!  You are not an HSpace ship!");
        return;
    }

    // Find the user of the console.
    dbUser = hsInterface.ConsoleUser(player);
    if (dbUser == HSNOTHING)
        return;

    // Is the ship destroyed?
    if (cShip->IsDestroyed())
    {
        hsInterface.Notify(dbUser, "This vessel has been destroyed!");
        return;
    }

    // Find the console object
    CHSConsole *cConsole;
    cConsole = cShip->FindConsole(player);
    if (cConsole && !cConsole->IsOnline())
    {
        hsStdError(dbUser,
                   hsInterface.HSPrintf("%s is currently powered down.",
                                        hsInterface.GetName(player)));
        return;
    }

    if (!arg_left || !*arg_left)
    {
        hsInterface.Notify(dbUser,
                           "You must specify a target contact number.");
        return;
    }

    // Tell the ship to view the object.
    cShip->ViewObjectID(dbUser, atoi(arg_left));
}

HSPACE_COMMAND_HDR(hscDoGate)
{
    HS_DBREF dbUser;
    CHSShip *cShip;

    // Find the ship based on the console
    cShip = (CHSShip *) dbHSDB.FindObjectByConsole(player);
    if (!cShip || (cShip->GetType() != HST_SHIP))
    {
        hsInterface.Notify(player, "Go away!  You are not an HSpace ship!");
        return;
    }

    // Find the user of the console.
    dbUser = hsInterface.ConsoleUser(player);
    if (dbUser == HSNOTHING)
        return;

    // Is the ship destroyed?
    if (cShip->IsDestroyed())
    {
        hsInterface.Notify(dbUser, "This vessel has been destroyed!");
        return;
    }

    // Find the console object
    CHSConsole *cConsole;
    cConsole = cShip->FindConsole(player);
    if (cConsole && !cConsole->IsOnline())
    {
        hsStdError(dbUser,
                   hsInterface.HSPrintf("%s is currently powered down.",
                                        hsInterface.GetName(player)));
        return;
    }

    if (!arg_left || !*arg_left)
    {
        hsInterface.Notify(dbUser,
                           "You must specify a target contact number.");
        return;
    }

    // Tell the ship to gate the object.
    cShip->GateObjectID(dbUser, atoi(arg_left));
}

HSPACE_COMMAND_HDR(hscHatchStat)
{
    HS_DBREF dbUser;
    CHSShip *cShip;

    // Find the ship based on the console
    cShip = (CHSShip *) dbHSDB.FindObjectByConsole(player);
    if (!cShip || (cShip->GetType() != HST_SHIP))
    {
        hsInterface.Notify(player, "Go away!  You are not an HSpace ship!");
        return;
    }

    // Find the user of the console.
    dbUser = hsInterface.ConsoleUser(player);
    if (dbUser == HSNOTHING)
        return;

    // Is the ship destroyed?
    if (cShip->IsDestroyed())
    {
        hsInterface.Notify(dbUser, "This vessel has been destroyed!");
        return;
    }

    // Find the console object
    CHSConsole *cConsole;
    cConsole = cShip->FindConsole(player);
    if (cConsole && !cConsole->IsOnline())
    {
        hsStdError(dbUser,
                   hsInterface.HSPrintf("%s is currently powered down.",
                                        hsInterface.GetName(player)));
        return;
    }

    cShip->GiveHatchRep(dbUser);
}

HSPACE_COMMAND_HDR(hscTractorMode)
{
    HS_DBREF dbUser;
    CHSShip *cShip;

    // Find the ship based on the console
    cShip = (CHSShip *) dbHSDB.FindObjectByConsole(player);
    if (!cShip || (cShip->GetType() != HST_SHIP))
    {
        hsInterface.Notify(player, "Go away!  You are not an HSpace ship!");
        return;
    }

    // Find the user of the console.
    dbUser = hsInterface.ConsoleUser(player);
    if (dbUser == HSNOTHING)
        return;

    // Is the ship destroyed?
    if (cShip->IsDestroyed())
    {
        hsInterface.Notify(dbUser, "This vessel has been destroyed!");
        return;
    }

    // Find the console object
    CHSConsole *cConsole;
    cConsole = cShip->FindConsole(player);
    if (cConsole && !cConsole->IsOnline())
    {
        hsStdError(dbUser,
                   hsInterface.HSPrintf("%s is currently powered down.",
                                        hsInterface.GetName(player)));
        return;
    }

    CHSSysTractor *cTractor;
    cTractor = (CHSSysTractor *) cShip->GetSystems().GetSystem(HSS_TRACTOR);
    if (!cTractor)
    {
        hsStdError(dbUser, "No tractor beam system on this vessel.");
        return;
    }

    if (atoi(arg_left) < 0 || atoi(arg_left) > 2)
    {
        hsStdError(dbUser, "Invalid tractor beam mode setting.");
        return;
    }

    cTractor->SetMode(atoi(arg_left));
}

HSPACE_COMMAND_HDR(hscTractorUnLock)
{
    HS_DBREF dbUser;
    CHSShip *cShip;

    // Find the ship based on the console
    cShip = (CHSShip *) dbHSDB.FindObjectByConsole(player);
    if (!cShip || (cShip->GetType() != HST_SHIP))
    {
        hsInterface.Notify(player, "Go away!  You are not an HSpace ship!");
        return;
    }

    // Find the user of the console.
    dbUser = hsInterface.ConsoleUser(player);
    if (dbUser == HSNOTHING)
        return;

    // Is the ship destroyed?
    if (cShip->IsDestroyed())
    {
        hsInterface.Notify(dbUser, "This vessel has been destroyed!");
        return;
    }

    // Find the console object
    CHSConsole *cConsole;
    cConsole = cShip->FindConsole(player);
    if (cConsole && !cConsole->IsOnline())
    {
        hsStdError(dbUser,
                   hsInterface.HSPrintf("%s is currently powered down.",
                                        hsInterface.GetName(player)));
        return;
    }

    CHSSysTractor *cTractor;
    cTractor = (CHSSysTractor *) cShip->GetSystems().GetSystem(HSS_TRACTOR);
    if (!cTractor)
    {
        hsStdError(dbUser, "No tractor beam system on this vessel.");
        return;
    }

    // Tell the ship to unlock the tractor beam
    cTractor->ReleaseLock(dbUser);

}


HSPACE_COMMAND_HDR(hscTractorLock)
{
    HS_DBREF dbUser;
    CHSShip *cShip;

    // Find the ship based on the console
    cShip = (CHSShip *) dbHSDB.FindObjectByConsole(player);
    if (!cShip || (cShip->GetType() != HST_SHIP))
    {
        hsInterface.Notify(player, "Go away!  You are not an HSpace ship!");
        return;
    }

    // Find the user of the console.
    dbUser = hsInterface.ConsoleUser(player);
    if (dbUser == HSNOTHING)
        return;

    // Is the ship destroyed?
    if (cShip->IsDestroyed())
    {
        hsInterface.Notify(dbUser, "This vessel has been destroyed!");
        return;
    }

    // Find the console object
    CHSConsole *cConsole;
    cConsole = cShip->FindConsole(player);
    if (cConsole && !cConsole->IsOnline())
    {
        hsStdError(dbUser,
                   hsInterface.HSPrintf("%s is currently powered down.",
                                        hsInterface.GetName(player)));
        return;
    }

    CHSSysTractor *cTractor;
    cTractor = (CHSSysTractor *) cShip->GetSystems().GetSystem(HSS_TRACTOR);
    if (!cTractor)
    {
        hsStdError(dbUser, "No tractor beam system on this vessel.");
        return;
    }

    // Tell the ship to lock the object.
    cTractor->SetLock(dbUser, atoi(arg_left));
}

HSPACE_COMMAND_HDR(hscTractorDock)
{
    HS_DBREF dbUser;
    CHSShip *cShip;

    // Find the ship based on the console
    cShip = (CHSShip *) dbHSDB.FindObjectByConsole(player);
    if (!cShip || (cShip->GetType() != HST_SHIP))
    {
        hsInterface.Notify(player, "Go away!  You are not an HSpace ship!");
        return;
    }

    // Find the user of the console.
    dbUser = hsInterface.ConsoleUser(player);
    if (dbUser == HSNOTHING)
        return;

    // Is the ship destroyed?
    if (cShip->IsDestroyed())
    {
        hsInterface.Notify(dbUser, "This vessel has been destroyed!");
        return;
    }

    // Find the console object
    CHSConsole *cConsole;
    cConsole = cShip->FindConsole(player);
    if (cConsole && !cConsole->IsOnline())
    {
        hsStdError(dbUser,
                   hsInterface.HSPrintf("%s is currently powered down.",
                                        hsInterface.GetName(player)));
        return;
    }

    CHSSysTractor *cTractor;
    cTractor = (CHSSysTractor *) cShip->GetSystems().GetSystem(HSS_TRACTOR);
    if (!cTractor)
    {
        hsStdError(dbUser, "No tractor beam system on this vessel.");
        return;
    }

    if (!arg_left)
    {
        hsStdError(dbUser, "No docking location specified.");
        return;
    }

    if (!arg_right)
    {
        hsStdError(dbUser, "No target contact specified.");
        return;
    }

    // Tell the ship to dock the object.
    cTractor->DockShip(dbUser, atoi(arg_right), atoi(arg_left));
}
