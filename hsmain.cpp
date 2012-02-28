// This file will serve as our connection between PennMUSH and
// HSpace.
#include "pch.h"

#include "hsinterface.h"
#include "hspace.h"
#include "hsobjects.h"
#include "hsdb.h"
#include "hsuniverse.h"

#ifndef MUX
extern "C"
{
#endif
    void hsInit(HS_INT32);
    void hsDumpDatabases(void);
    void hsCommand(char *, const char *, char *, char *, HS_INT32);
    void hsCycle(void);
    void hsDestroyObject(HS_DBREF objnum);
    void hsShortCycle(void);
    void hsShutdown(void);
    char *hsFunction(char *, HS_DBREF, char **);
#ifndef MUX
}
#endif

void hsInit(HS_INT32 reboot)
{
    HSpace.InitSpace(reboot);
}

void hsDumpDatabases(void)
{
    HSpace.DumpDatabases();
}

void hsShutdown(void)
{
    HSpace.Shutdown();
}

void
hsCommand(char *strCmd, const char *switches,
          char *arg_left, char *arg_right, HS_INT32 player)
{
    HSpace.DoCommand(strCmd, switches, arg_left, arg_right, player);
}

char *hsFunction(char *strFunc, HS_DBREF executor, char **args)
{
    return HSpace.DoFunction(strFunc, executor, args);
}

void hsCycle(void)
{
    HSpace.DoCycle();
}

// Handle destruction of an object from the game server.
// If it is a space object, properly cleanup to avoid invalid obj references
void hsDestroyObject(HS_DBREF objnum)
{
    // Only THINGs need mothballed
    if(TYPE_THING != hsInterface.GetType(objnum))
    {
        return;
    }

    // Locate the object in the runtime database
    CHS3DObject* cObj= dbHSDB.FindObject(objnum);

    // Not found -- good, just return
    if(NULL == cObj )
    {
        return;
    }

    // Find the object's universe -- if not found, just return
    CHSUniverse* uSource = cObj->GetUniverse();
    if(NULL == uSource)
    {
        return;
    }

    hs_log(hsInterface.HSPrintf("Automatic mothball of %s (#%d).",
                cObj->GetName(), objnum));

    // Remove the object from the appropriate universe
    uSource->RemoveObject(cObj);
    // Release the object completely
    cObj->Release();
}

