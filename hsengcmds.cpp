#include "pch.h"

#include <cstdio>
#include <stdlib.h>

#ifndef WIN32
#include <cstring>
#endif

#include "hscopyright.h"
#include "hscmds.h"
#include "hsdb.h"
#include "hseng.h"
#include "hsinterface.h"
#include "hsutils.h"
#include "hsobjects.h"
#include "hsship.h"


HSPACE_COMMAND_PROTO(hscSetSystemPower)
HSPACE_COMMAND_PROTO(hscSystemReport)
HSPACE_COMMAND_PROTO(hscSetSystemPriority)
HSPACE_COMMAND_PROTO(hscShipStats)
HSPACE_COMMAND_PROTO(hscCrewRep)
HSPACE_COMMAND_PROTO(hscAssignCrew) 
HSPACE_COMMAND_PROTO(hscUnassignCrew) 
HSPACE_COMMAND_PROTO(hscSelfDestruct)

    // The hsCommandArray holds all externally callable @eng commands.
HSPACE_COMMAND hsEngCommandArray[] = {
    {"SELFDESTRUCT", hscSelfDestruct, HCP_ANY},
    {"SETSYSPOWER", hscSetSystemPower, HCP_ANY},
    {"SHIPSTATS", hscShipStats, HCP_ANY},
    {"SYSTEMREPORT", hscSystemReport, HCP_ANY},
    {"SYSTEMPRIORITY", hscSetSystemPriority, HCP_ANY},
    {"CREWREP", hscCrewRep, HCP_ANY},
    {"ASSIGNCREW", hscAssignCrew, HCP_ANY},
    {"UNASSIGNCREW", hscUnassignCrew, HCP_ANY},
    {NULL, NULL, 0}
    };

HSPACE_COMMAND_HDR(hscSetSystemPower)
{
    HS_DBREF dbUser;
    CHSShip *cShip;
    int iLvl;

    // arg_left is the system name
    // arg_right is the level to set

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

    if (!arg_left || !*arg_left || !arg_right || !*arg_right)
    {
        hsStdError(dbUser,
                   "You must specify both a system name and power level.");
        return;
    }

    iLvl = atoi(arg_right);
    if (arg_right[strlen(arg_right) - 1] == '%')
        cShip->SetSystemPower(dbUser, arg_left, iLvl, true);
    else
        cShip->SetSystemPower(dbUser, arg_left, iLvl, false);
}

HSPACE_COMMAND_HDR(hscSystemReport)
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

    cShip->GiveEngSysReport(dbUser);
}

HSPACE_COMMAND_HDR(hscSetSystemPriority)
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

    if (!arg_left || !*arg_left || !arg_right || !*arg_right)
    {
        hsStdError(dbUser,
                   "You must specify both a system name and priority change.");
        return;
    }

    cShip->ChangeSystemPriority(dbUser, arg_left, atoi(arg_right));
}

HSPACE_COMMAND_HDR(hscShipStats)
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

    cShip->GiveVesselStats(dbUser);
}

HSPACE_COMMAND_HDR(hscCrewRep)
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

    cShip->GiveCrewRep(dbUser);
}

HSPACE_COMMAND_HDR(hscUnassignCrew)
{
    HS_DBREF dbUser;
    CHSShip *cShip;
    int iCrew;


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
        hsStdError(dbUser,
                   "You must specify a crew number.");
        return;
    }

    iCrew = atoi(arg_left);
    cShip->UnassignCrew(dbUser, iCrew);
}

HSPACE_COMMAND_HDR(hscAssignCrew)
{
    HS_DBREF dbUser;
    CHSShip *cShip;
    int iCrew;

    // arg_left is the system name
    // arg_right is the level to set

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

    if (!arg_left || !*arg_left || !arg_right || !*arg_right)
    {
        hsStdError(dbUser,
                   "You must specify both a crew number and a system.");
        return;
    }

    iCrew = atoi(arg_left);
    cShip->AssignCrew(dbUser, iCrew, arg_right);
}

HSPACE_COMMAND_HDR(hscSelfDestruct)
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

    if (!arg_left || !*arg_left)
    {
        hsStdError(dbUser, "You must specify the timer value.");
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

    cShip->InitSelfDestruct(dbUser, atoi(arg_left), arg_right);
}
