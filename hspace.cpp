// -----------------------------------------------------------------------
//! $Id: hspace.cpp,v 1.6 2007/08/19 17:41:27 grapenut_org Exp $
// -----------------------------------------------------------------------

#include "pch.h"

// This has to come before the other includes, 
// or Windows really screws up
#ifdef WIN32
#undef OPAQUE                   /* Clashes with flags.h */
#endif

#ifndef WIN32
#include <strings.h>
#include <sys/time.h>
#endif

#include "hspace.h"
#include "hsuniverse.h"
#include "hsuniversedb.h"
#include "hsinterface.h"
#include "hscmds.h"
#include "hsfuncs.h"
#include "hsconf.h"
#include "hsdb.h"
#include "hsutils.h"
#include "hsserver.h"
#include "hspilot.h"

extern FILE *spacelog_fp;

// The one and only, HSpace instance!
CHSpace HSpace;

CHSpace::CHSpace() :
    m_cycling(false),
    m_attrs_need_cleaning(false),
    m_cycle_ms(0)
{
}

// Initializes HSpace.  The reboot variable indicates whether the
// server is in reboot mode, in which the proceses will be swapped
// instead of starting from cold boot.
void CHSpace::InitSpace(int reboot)
{
    hsInitMutex();

    // Copy our logfile to a save copy -- no worries if it fails
    rename("space/space.log", "space/space.log.old");

    // Open our space log.
    fopen_s(&spacelog_fp, "space/space.log", "w");
    if(NULL == spacelog_fp)
    {
        fopen_s(&spacelog_fp, "space.log", "w");

        if(NULL == spacelog_fp)
        {
            spacelog_fp = stderr;
        }
    }

    hsInterface.SetupInterfaceCommands();

    // Load the config file first.
    if(!HSCONF.LoadConfigFile(HSPACE_CONFIG_FILE))
    {
        hs_log("CHSpace::InitSpace() - Configuration file loading failed.");
        return;
    }

    InitPilots();

    if(!dbHSDB.LoadDatabases())
    {
        hs_log("CHSpace::InitSpace() - Database loading failed!");
        return;
    }

    // Auto cycle?
    if(HSCONF.autostart)
    {
        hs_log("Automatic cycling enabled.");
        m_cycling = true;
    }

    if(HSCONF.admin_server)
    {
        CHSServer::GetInstance().StartServer(HSCONF.admin_server_port);
        hs_log(hsInterface.
                HSPrintf("ADMIN SERVER: Started remote admin server on port %d.",
                    HSCONF.admin_server_port));
    }
}

void CHSpace::Shutdown()
{
    if(HSCONF.admin_server)
    {
        CHSServer::GetInstance().ShutdownServer();
    }

    hsDestroyMutex();
}

// Called once per second externally to do all of the stuff that
// HSpace needs to do each cycle.
void CHSpace::DoCycle()
{
#ifndef WIN32
    struct timeval tv;
#endif
    double firstms, secondms;


    if(!m_cycling)
        return;

    hsEnterMutex();

    // Run the server
    if(HSCONF.admin_server)
    {
        CHSServer::GetInstance().DoCycle();
    }

    // Grab the current milliseconds
#ifdef WIN32
    firstms = timeGetTime();
#else
    gettimeofday(&tv, (struct timezone *) NULL);
    firstms = (tv.tv_sec * 1000000) + tv.tv_usec;
#endif

    // If attributes need cleaned up, do that.
    if(m_attrs_need_cleaning)
    {
        dbHSDB.CleanupDBAttrs();
        m_attrs_need_cleaning = false;
    }

    // Do cyclic stuff for space objects.
    THSUniverseIterator tIter;
    HS_BOOL8 bIter;
    for (bIter = CHSUniverseDB::GetInstance().GetFirstUniverse(tIter); bIter;
            bIter = CHSUniverseDB::GetInstance().GetNextUniverse(tIter))
    {
        CHSUniverse *pUniverse = tIter.pValue;

        // Grab all of the objects in the universe, and tell them
        // to cycle.
        THSObjectIterator tIterator;
        HS_BOOL8 bContinue;
        for (bContinue = pUniverse->GetFirstObject(tIterator); bContinue;
                bContinue = pUniverse->GetNextObject(tIterator))
        {
            CHS3DObject *pObject = tIterator.pValue;
            pObject->DoCycle();
        }

    }                           // Cycle stuff done for CHS3DObjects

    // Grab the current milliseconds, and subtract from the first
#ifdef WIN32
    secondms = timeGetTime();
    m_cycle_ms = secondms - firstms;
#else
    gettimeofday(&tv, (struct timezone *) NULL);
    secondms = (tv.tv_sec * 1000000) + tv.tv_usec;
    m_cycle_ms = (secondms - firstms) * .001;
#endif

    hsLeaveMutex();
}

void CHSpace::DumpDatabases()
{
    dbHSDB.DumpDatabases();
    m_attrs_need_cleaning = true;
}

char *CHSpace::DoFunction(char *func, HS_DBREF executor, char **args)
{
    HSFUNC *hFunc;

    hFunc = hsFindFunction(func);

    if(!hFunc)
    {
        return "#-1 HSpace function not found.";
    }
    else
    {
        return hFunc->func(executor, args);
    }
}

void CHSpace::DoCommand(char *cmd, const char *switches, char *arg_left,
        char *arg_right, HS_DBREF player)
{
    HSPACE_COMMAND *hCmd;

    hsEnterMutex();

    if(!_stricmp(cmd, "@space"))
    {
        hCmd = hsFindCommand(switches, hsSpaceCommandArray);
    }
    else if(!_stricmp(cmd, "@nav"))
    {
        hCmd = hsFindCommand(switches, hsNavCommandArray);
    }
    else if(!_stricmp(cmd, "@console"))
    {
        hCmd = hsFindCommand(switches, hsConCommandArray);
    }
    else if(!_stricmp(cmd, "@eng"))
    {
        hCmd = hsFindCommand(switches, hsEngCommandArray);
    }
    else
    {
        hsInterface.Notify(player, "Invalid HSpace command.");
        hsLeaveMutex();
        return;
    }

    if(!hCmd)
    {
        hsInterface.Notify(player,
                "Invalid HSpace switch supplied with command.");
        hsLeaveMutex();
        return;
    }

    // Check the permissions of the command
    if(hCmd->perms & HCP_GOD)
    {
        if(hsInterface.GetGodDbref() != player)
        {
            hsInterface.Notify(player, "HSpace command restricted to God.");
            hsLeaveMutex();
            return;
        }
    }

    if(hCmd->perms & HCP_WIZARD)
    {
        if(!hsInterface.IsWizard(player))
        {
            hsInterface.Notify(player,
                    "HSpace command restricted to wizards.");
            hsLeaveMutex();
            return;
        }
    }

    // Log the command first
    if(HSCONF.log_commands)
    {
        hs_log(hsInterface.HSPrintf("CMD: (#%d) %s/%s %s=%s",
                    player, cmd, switches, arg_left, arg_right));
    }

    // Perms look ok, call the function.
    hCmd->func(player, arg_left, arg_right);

    hsLeaveMutex();
}


// Turns the space cycle on/off
void CHSpace::SetCycle(HS_BOOL8 bStart)
{
    hs_log(hsInterface.HSPrintf("Space cycling %s.", 
                bStart ? "enabled" : "disabled"));
    m_cycling = bStart;
}
