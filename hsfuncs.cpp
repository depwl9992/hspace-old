#include "pch.h"

#include <cstring>
#include <stdlib.h>
#include <ctype.h>

#include "hscopyright.h"
#include "hstypes.h"
#include "hsfuncs.h"
#include "hsinterface.h"
#include "hsobjects.h"
#include "hsmissile.h"
#include "hsdb.h"
#include "hsutils.h"
#include "hsuniverse.h"
#include "hsuniversedb.h"
#include "hsterritory.h"
#include "hspace.h"
#include "hseng.h"
#include "hssensors.h"
#include "hscomm.h"
#include "hsflags.h"
#include "hscmds.h"

HSFUNC hs_func_list[] = 
{
    {"ADDSYS", hsfAddSys},
    {"ADDWEAPON", hsfAddWeapon},
    {"CLONE", hsfClone},
    {"COMMAND", hsfConsoleCommand},
    {"COMMMSG", hsfCommMsg},
    {"CONWEAP", hsfConsoleWeaponAttr},
    {"DECAYMSG", hsfDecayMessage},
    {"DELSYS", hsfDelSys},
    {"DELWEAPON", hsfDelWeapon},
    {"EXPLODE", hsfExplode},
    {"GETATTR", hsfGetAttr}, 
    {"GETENGSYSTEMS", hsfGetEngSystems},
    {"GETMISSILE", hsfGetMissile},
    {"LIST", hsfList},
    {"NEARBY", hsfNearby},
    {"SENSORCONTACTS", hsfGetSensorContacts},
    {"SETATTR", hsfSetAttr},
    {"SETMISSILE", hsfSetMissile},
    {"SPACEMSG", hsfSpaceMsg},
    {"SYSATTR", hsfSysAttr},
    {"SYSSET", hsfSysSet},
    {"WEAPONATTR", hsfWeaponAttr},
    {"XYANG", hsfCalcXYAngle},
    {"ZANG", hsfCalcZAngle},
    {NULL}
};

// Finds a function with a given name.
HSFUNC *hsFindFunction(char *name)
{
    HSFUNC *ptr;
    for (ptr = hs_func_list; ptr->name; ptr++)
    {
        // Straight comparison of names
        if (!strcasecmp(name, ptr->name))
            return ptr;
    }

    return NULL;
}

// Gets or sets an attribute on one of the globally loaded database
// weapons.
HSPACE_FUNC_HDR(hsfWeaponAttr)
{
    int iWclass;
    char arg_left[32];          // left of colon
    char arg_right[32];         // right of colon

    // Executor must be a wizard
    if (!hsInterface.IsWizard(executor))
        return "#-1 Permission denied.";


    // Find a colon (:) in the attrname parameter.  If one
    // exists, this is a set operation as opposed to get.
    *arg_left = *arg_right = '\0';
    char *ptr;
    ptr = strchr(args[1], ':');
    if (ptr)
    {
        // Split args[1] in half at the colon.  Copy the left
        // side into arg_left and the right side into .. arg_right.
        *ptr = '\0';
        strncpy(arg_left, args[1], 31);
        arg_left[31] = '\0';

        // Increment ptr and copy arg_right stuff.
        ptr++;
        strncpy(arg_right, ptr, 31);
        arg_right[31] = '\0';
    }
    else
    {
        // This is a GET operation.  Args[1] is an attrname and
        // no more.
        strncpy(arg_left, args[1], 31);
        arg_left[31] = '\0';
    }

    // Determine weapon class #
    iWclass = atoi(args[0]);

    CHSWeaponData *pData = waWeapons.GetWeapon(iWclass);
    if (!pData)
    {
        return "#-1 Bad weapon identifier.";
    }

    // If this is a GET, then just ask the weapon for the
    // info.
    if (!*arg_right)
    {
        const char *ptr = pData->GetAttributeValue(arg_left);

        if (!ptr)
            return "#-1 Attribute not found.";
        else
            return (char *) ptr;
    }
    else
    {
        if (!pData->SetAttributeValue(arg_left, arg_right))
            return "#-1 Failed to set attribute.";
        else
            return "Weapon attribute - Set.";
    }
}

// Gets or sets an attribute on console weapon.
HSPACE_FUNC_HDR(hsfConsoleWeaponAttr)
{
    HS_DBREF dbObj;
    char *ptr;
    int iWnum;
    CHS3DObject *cObj;
    CHSConsole *cConsole;
    char arg_left[32];          // left of colon
    char arg_right[32];         // right of colon

    // Args[0] should be obj/attr, so drop a null in the 
    // split.
    ptr = strchr(args[0], '/');
    if (!ptr)
        return "#-1 Invalid console/weapon combination.";

    *ptr = '\0';
    // Find the object in question
    dbObj = hsInterface.NoisyMatchThing(executor, args[0]);

    if (dbObj == HSNOTHING)
        return "#-1 Not found.";

    if (!hsInterface.IsNearby(executor, dbObj)
        && !hsInterface.IsWizard(executor))
    {
        hsInterface.Notify(executor, "I don't see that here.");
        return "#-1 Not found.";
    }

    // If it's a console, search for the object based on
    // the console.
    if (hsInterface.HasFlag(dbObj, TYPE_THING, THING_HSPACE_CONSOLE))
        cObj = dbHSDB.FindObjectByConsole(dbObj);
    else
        return "#-1 Invalid HSpace console.";

    if (!cObj)
        return "#-1 Invalid HSpace console.";

    cConsole = cObj->FindConsole(dbObj);
    if (!cConsole)
        return "#-1 Invalid HSpace console.";

    ptr++;
    iWnum = atoi(ptr) - 1;

    // Find a colon (:) in the attrname parameter.  If one
    // exists, this is a set operation as opposed to get.
    *arg_left = *arg_right = '\0';
    ptr = strchr(args[1], ':');
    if (ptr)
    {
        // Split args[1] in half at the colon.  Copy the left
        // side into arg_left and the right side into .. arg_right.
        *ptr = '\0';
        strncpy(arg_left, args[1], 31);
        arg_left[31] = '\0';

        // Increment ptr and copy arg_right stuff.
        ptr++;
        strncpy(arg_right, ptr, 31);
        arg_right[31] = '\0';
    }
    else
    {
        // This is a GET operation.  Args[1] is an attrname and
        // no more.
        strncpy(arg_left, args[1], 31);
        arg_left[31] = '\0';
    }

    CHSWeaponArray *cWArray = cConsole->GetWeaponArray();
    if (!cWArray)
        return "#-1 Console has no weapons.";

    CHSWeapon *cWeap = cWArray->GetWeapon(iWnum);

    if (!cWeap)
        return "#-1 No such weapon.";

    // If this is a GET, then just ask the weapon for the
    // info.
    if (!*arg_right)
    {
        const HS_INT8 *str = cWeap->GetAttributeValue(arg_left);

        if (!str)
            return "#-1 Attribute not found.";
        else
            return (char *) str;
    }
    else
    {
        if (!cWeap->SetAttributeValue(arg_left, arg_right))
            return "#-1 Failed to set attribute.";
        else
            return "Weapon attribute - Set.";
    }
}

HSPACE_FUNC_HDR(hsfAddWeapon)
{
    HS_DBREF dbObj;

    // Find the object in question
    dbObj = hsInterface.NoisyMatchThing(executor, args[0]);

    if (dbObj == HSNOTHING)
        return "#-1";

    // Executor must control the object
    if (!hsInterface.ControlsObject(executor, dbObj))
    {
        return "#-1 Permission denied.";
    }

    if (!hsInterface.IsNearby(executor, dbObj)
        && !hsInterface.CanSeeAll(executor))
    {
        hsInterface.Notify(executor, "I don't see that here.");
        return "#-1";
    }

    // If object is not a console, don't do it.
    if (!hsInterface.HasFlag(dbObj, TYPE_THING, THING_HSPACE_CONSOLE))
    {
        return "#-1 Target object must be a ship console.";
    }

    // Find the console object
    CHSConsole *cConsole;

    cConsole = dbHSDB.FindConsole(dbObj);
    if (!cConsole)
        return "#-1 Invalid HSpace console.";

    // Tell the console to add the weapon
    if (!cConsole->AddWeapon(executor, atoi(args[1])))
        return "#-1 Failed to add weapon.";
    else
        return "Weapon - Added.";
}

HSPACE_FUNC_HDR(hsfDelWeapon)
{
    HS_DBREF dbObj;

    // Find the object in question
    dbObj = hsInterface.NoisyMatchThing(executor, args[0]);

    if (dbObj == HSNOTHING)
        return "#-1";

    // Executor must control the object
    if (!hsInterface.ControlsObject(executor, dbObj))
    {
        return "#-1 Permission denied.";
    }

    if (!hsInterface.IsNearby(executor, dbObj)
        && !hsInterface.CanSeeAll(executor))
    {
        hsInterface.Notify(executor, "I don't see that here.");
        return "#-1 No such object.";
    }

    // If object is not a console, don't do it.
    if (!hsInterface.HasFlag(dbObj, TYPE_THING, THING_HSPACE_CONSOLE))
    {
        return "#-1 Target object must be a ship console.";
    }

    // Find the console object
    CHSConsole *cConsole;

    cConsole = dbHSDB.FindConsole(dbObj);
    if (!cConsole)
        return "#-1 Invalid HSpace console.";

    // Tell the console to delete the weapon.
    // DeleteWeapon doesn't return a value, probably should be a boolean
    cConsole->DeleteWeapon(executor, atoi(args[1]));
    return "";
}


HSPACE_FUNC_HDR(hsfDecayMessage)
{
    double decay = atof(args[1]);
    char *msg = args[0];

    static char newmsg[2048];
    char *ptr, *ptr2;
    char word[2048];
    int i, len;


    ptr2 = newmsg;
    i = 0;
    len = 0;
    for (ptr = msg; (len < 1900) && *ptr; ptr++)
    {
        if (*ptr == ' ')
        {
            word[i++] = ' ';
            word[i] = '\0';
            if (hsInterface.GetRandom(100) < decay)
                strcpy(word, "... ");
            for (i = 0; word[i]; i++)
            {
                *ptr2 = word[i];
                ptr2++;
            }
            i = 0;
        }
        else
        {
            word[i++] = *ptr;
            len++;
        }
    }
    len += i;
    if (len < 1900)
    {
        word[i++] = ' ';
        word[i] = '\0';
        if (hsInterface.GetRandom(100) < decay)
            strcpy(word, "...");
        for (i = 0; word[i]; i++)
        {
            *ptr2 = word[i];
            ptr2++;
        }
    }
    *ptr2 = '\0';

    return newmsg;
}

HSPACE_FUNC_HDR(hsfCalcXYAngle)
{
    double x1, y1;
    double x2, y2;

    // Grab coordinate input values
    x1 = atof(args[0]);
    y1 = atof(args[1]);
    x2 = atof(args[2]);
    y2 = atof(args[3]);

    static char rval[8];

    sprintf(rval, "%d", XYAngle(x1, y1, x2, y2));
    return rval;
}

HSPACE_FUNC_HDR(hsfCalcZAngle)
{
    double x1, y1, z1;
    double x2, y2, z2;

    // Grab coordinate input values
    x1 = atof(args[0]);
    y1 = atof(args[1]);
    z1 = atof(args[2]);
    x2 = atof(args[3]);
    y2 = atof(args[4]);
    z2 = atof(args[5]);

    static char rval[8];

    sprintf(rval, "%d", ZAngle(x1, y1, z1, x2, y2, z2));
    return rval;
}

HSPACE_FUNC_HDR(hsfGetMissile)
{
    HS_DBREF dbObj;
    static char rval[32];

    // Find the object in question
    dbObj = hsInterface.NoisyMatchThing(executor, args[0]);

    if (dbObj == HSNOTHING)
        return "#-1";

    if (!hsInterface.IsNearby(executor, dbObj)
        && !hsInterface.CanSeeAll(executor))
    {
        hsInterface.Notify(executor, "I don't see that here.");
        return "#-1";
    }

    // Determine type of object
    if (!hsInterface.HasFlag(dbObj, TYPE_THING, THING_HSPACE_OBJECT))
        return "#-1 Not an HSpace object.";

    CHS3DObject *cObj;

    cObj = dbHSDB.FindObject(dbObj);
    if (!cObj)
        return "#-1 Invalid HSpace object.";

    if (cObj->GetType() != HST_SHIP)
        return "#-1 Not an HSpace ship.";


    CHSShip *cShip;
    cShip = (CHSShip *) cObj;

    // Grab the missile bay from the ship
    CHSMissileBay *mBay;
    mBay = cShip->GetMissileBay();

    // Ask the missile bay for the maximum and current values.
    HS_UINT32 max;
    HS_UINT32 left;
    int iClass;
    iClass = atoi(args[1]);
    max = mBay->GetMaxMissiles(iClass);
    left = mBay->GetMissilesLeft(iClass);
    sprintf(rval, "%d/%d", left, max);
    return rval;
}

HSPACE_FUNC_HDR(hsfGetAttr)
{
    HS_DBREF dbObj = HSNOTHING;
    char *ptr = NULL;

    // Find the object in question
    dbObj = hsInterface.NoisyMatchThing(executor, args[0]);

    if (dbObj == HSNOTHING)
        return "#-1";

    if (!hsInterface.IsNearby(executor, dbObj)
        && !hsInterface.CanSeeAll(executor))
    {
        hsInterface.Notify(executor, "I don't see that here.");
        return "#-1";
    }

    // Determine type of object
    if (hsInterface.HasFlag(dbObj, TYPE_THING, THING_HSPACE_OBJECT))
    {
        CHS3DObject *cObj;

        cObj = dbHSDB.FindObject(dbObj);
        if (!cObj)
            return "#-1 Invalid HSpace object.";

        // Disable access to some attrs
        if (!strcasecmp(args[1], "BOARDING CODE") &&
            !hsInterface.CanSeeAll(executor)
            && !hsInterface.ControlsObject(executor, dbObj))
        {
            return "#-1 Permission Denied.";
        }
        ptr = cObj->GetAttributeValue(args[1]);
    }
    else if (hsInterface.HasFlag(dbObj, TYPE_THING, THING_HSPACE_CONSOLE))
    {
        CHSConsole *cConsole;

        cConsole = dbHSDB.FindConsole(dbObj);
        if (!cConsole)
            return "#-1 Invalid HSpace console.";

        // Disable access to some attrs
        if (!strcasecmp(args[1], "BOARDING CODE") &&
            !hsInterface.CanSeeAll(executor)
            && !hsInterface.ControlsObject(executor, dbObj))
        {
            return "#-1 Permission Denied.";
        }

        // Try to get the attribute from the console.
        ptr = cConsole->GetAttributeValue(args[1]);

        // If attribute not found, try the ship object
        // the console belongs to.
        if (!ptr)
        {
            CHS3DObject *cObj;
            cObj = dbHSDB.FindObjectByConsole(dbObj);
            if (cObj)
                ptr = cObj->GetAttributeValue(args[1]);
        }
    }
    else if (hsInterface.HasFlag(dbObj, TYPE_ROOM, ROOM_HSPACE_LANDINGLOC))
    {
        CHSLandingLoc *cLoc;

        cLoc = dbHSDB.FindLandingLoc(dbObj);
        if (!cLoc)
            return "#-1 Invalid HSpace landing location.";

        ptr = cLoc->GetAttributeValue(args[1]);
    }
    else if (hsInterface.HasFlag(dbObj, TYPE_EXIT, EXIT_HSPACE_HATCH))
    {
        CHSHatch *cHatch;

        cHatch = dbHSDB.FindHatch(dbObj);
        if (!cHatch)
            return "#-1 Invalid HSpace hatch.";

        ptr = cHatch->GetAttributeValue(args[1]);
    }
    else if (hsInterface.HasFlag(dbObj, TYPE_THING, THING_HSPACE_TERRITORY))
    {
        CHSTerritory *cTerritory;

        cTerritory = taTerritories.FindTerritory(dbObj);

        if (!cTerritory)
        {
            return "#-1 Invalid HSpace Territory";
        }
        ptr = cTerritory->GetAttributeValue(args[1]);

    }
    else
    {
        return "#-1 Not an HSpace object.";
    }

    if (NULL == ptr)
    {
        return "#-1 Attribute not found.";
    }

    return ptr;
}

HSPACE_FUNC_HDR(hsfSetAttr)
{
    HS_DBREF dbObj;
    char *ptr, *dptr;
    char strName[32];
    char strAttr[32];

    // The object/attribute pair is specified by a slash
    // in between.  Separate them.
    dptr = strName;
    for (ptr = args[0]; *ptr; ptr++)
    {
        if ((dptr - strName) > 31)
            break;

        if (*ptr == '/')
            break;

        *dptr++ = *ptr;
    }
    *dptr = '\0';

    if (!*ptr)
        return "#-1 Invalid object/attribute pair specified.";

    dptr = strAttr;
    ptr++;
    while (*ptr)
    {
        if ((dptr - strAttr) > 31)
            break;

        *dptr++ = *ptr++;
    }
    *dptr = '\0';

    // Find the object in question
    dbObj = hsInterface.NoisyMatchThing(executor, strName);

    if (dbObj == HSNOTHING)
        return "#-1";

    // Executor must control the object
    if (!hsInterface.ControlsObject(executor, dbObj))
    {
        return "#-1 Permission denied.";
    }

    if (!hsInterface.IsNearby(executor, dbObj)
        && !hsInterface.CanSeeAll(executor))
    {
        hsInterface.Notify(executor, "I don't see that here.");
        return "#-1";
    }

    // Determine type of object
    if (hsInterface.HasFlag(dbObj, TYPE_THING, THING_HSPACE_OBJECT))
    {
        HS_BOOL8 val;
        CHS3DObject *cObj;

        cObj = dbHSDB.FindObject(dbObj);
        if (!cObj)
            return "#-1 Invalid HSpace object.";

        if (cObj->GetType() == HST_MISSILE)
            val =
                static_cast < CHSMissile * >(cObj)->SetAttributeValue(strAttr,
                                                                      args
                                                                      [1]);
        else
            val = cObj->SetAttributeValue(strAttr, args[1]);

        if (val)
            return "Attribute - set.";
        else
            return "Attribute - failed.";
    }
    else if (hsInterface.HasFlag(dbObj, TYPE_THING, THING_HSPACE_CONSOLE))
    {
        CHSConsole *cConsole;

        cConsole = dbHSDB.FindConsole(dbObj);
        if (!cConsole)
            return "#-1 Invalid HSpace console.";

        // Try to get the attribute from the console.
        if (cConsole->SetAttributeValue(strAttr, args[1]))
            return "Attribute - set.";
        else
        {
            // If attribute not found, try the ship object
            // the console belongs to.
            CHS3DObject *cObj;
            cObj = dbHSDB.FindObjectByConsole(dbObj);
            if (cObj)
            {
                if (cObj->SetAttributeValue(strAttr, args[1]))
                {
                    static char rbuf[32];
                    sprintf(rbuf, "%s Attribute - set.", cObj->GetName());
                    return rbuf;
                }
                else
                    return "Attribute - failed.";
            }
            else
                return "Attribute - failed.";
        }
    }
    else if (hsInterface.HasFlag(dbObj, TYPE_ROOM, ROOM_HSPACE_LANDINGLOC))
    {
        CHSLandingLoc *cLoc;

        cLoc = dbHSDB.FindLandingLoc(dbObj);
        if (!cLoc)
            return "#-1 Invalid HSpace landing location.";

        if (cLoc->SetAttributeValue(strAttr, args[1]))
            return "Attribute - set.";
        else
            return "Attribute - failed.";
    }
    else if (hsInterface.HasFlag(dbObj, TYPE_EXIT, EXIT_HSPACE_HATCH))
    {
        CHSHatch *cHatch;

        cHatch = dbHSDB.FindHatch(dbObj);
        if (!cHatch)
            return "#-1 Invalid HSpace hatch.";

        if (cHatch->SetAttributeValue(strAttr, args[1]))
            return "Attribute - set.";
        else
            return "Attribute - failed.";
    }
    else if (hsInterface.HasFlag(dbObj, TYPE_THING, THING_HSPACE_TERRITORY))
    {
        CHSTerritory *cTerritory;

        cTerritory = taTerritories.FindTerritory(dbObj);

        if (!cTerritory)
        {
            return "#-1 Invalid HSpace Territory";
        }
        if (cTerritory->SetAttributeValue(strAttr, args[1]))
        {
            return "Attribute - set.";
        }
        else
        {
            return "Attribute - failed.";
        }
    }
    else
        return "#-1 Not an HSpace object.";
}

// Sends a message into space by 1 of 3 possible ways.
HSPACE_FUNC_HDR(hsfSpaceMsg)
{
    CHS3DObject *cObj;

    // Check flags on the object to see if it's an
    // actual object or maybe a console.
    if (hsInterface.HasFlag(executor, TYPE_THING, THING_HSPACE_OBJECT))
        cObj = dbHSDB.FindObject(executor);
    else if (hsInterface.HasFlag(executor, TYPE_THING, THING_HSPACE_CONSOLE))
        cObj = dbHSDB.FindObjectByConsole(executor);
    else
        cObj = NULL;
    if (!hsInterface.IsWizard(executor) && !cObj)
    {
        return "#-1 Permission denied.";
    }


    // Methods of sending:
    //
    // 0 = Blind message sent to all ships within a specified
    //     distance of the given coordinates.
    // 1 = Message sent to all ships who have the source object
    //     on sensors.
    // 2 = Message sent to all ships who have the source object
    //     identified on sensors.
    int how = atoi(args[0]);
    CHSUniverse *uDest;
    switch (how)
    {
    case 0:
        if (!args[1] || !isdigit(*args[1]))
            return "#-1 Invalid universe ID specified.";

        int uid;
        uid = atoi(args[1]);
        uDest = CHSUniverseDB::GetInstance().FindUniverse(uid);
        if (!uDest)
            return "#-1 Invalid universe ID specified.";


        if (!args[2] || !args[3] || !args[4])
        {

            return "#-1 Invalid coordinate parameters supplied.";
        }

        double sX, sY, sZ;      // Source coords
        sX = atof(args[2]);
        sY = atof(args[3]);
        sZ = atof(args[4]);

        if (!args[5] || !isdigit(*args[5]))
            return "#-1 Invalid distance parameter value.";

        double dMaxDist;
        dMaxDist = atof(args[5]);
        double tX, tY, tZ;      // Target object coords

        if (!args[6])
            return "#-1 No message to send.";

        double dDistance;
        CHS3DObject *pObject;
        for (pObject = uDest->GetFirstActiveObject(); pObject;
             pObject = uDest->GetNextActiveObject())
        {
            tX = pObject->GetX();
            tY = pObject->GetY();
            tZ = pObject->GetZ();

            dDistance = Dist3D(sX, sY, sZ, tX, tY, tZ);

            if (dDistance > dMaxDist)
                continue;

            pObject->HandleMessage(args[6], MSG_GENERAL, NULL);
        }
        break;

    case 1:
    case 2:
        if (!cObj)
            return "#-1 You are not a HSpace object.";

        // Grab the source universe
        uDest = cObj->GetUniverse();
        if (!uDest)
            return "#-1 Object universe could not be found.";

        CHSSysSensors *cSensors;
        SENSOR_CONTACT *cContact;
        for (pObject = uDest->GetFirstActiveObject(); pObject;
             pObject = uDest->GetNextActiveObject())
        {
            cSensors = (CHSSysSensors *) pObject->GetEngSystem(HSS_SENSORS);
            if (!cSensors)
                continue;

            cContact = cSensors->GetContact(cObj);
            if (!cContact)
                continue;

            if ((cContact->status == DETECTED ||
                 cContact->status == IDENTIFIED) && how == 1)
                pObject->HandleMessage(args[1], MSG_SENSOR, (long *) cObj);
            else if (cContact->status == IDENTIFIED && how == 2)
                pObject->HandleMessage(args[1], MSG_SENSOR, (long *) cObj);
        }
        break;

    default:
        return "#-1 Invalid sending method specified.";
    }
    return "Message sent.";
}

// Returns a comma delimited list of engineering systems
// located on the object.
HSPACE_FUNC_HDR(hsfGetEngSystems)
{
    CHS3DObject *cObj;
    HS_DBREF dbObj;

    // Find the object in question
    dbObj = hsInterface.NoisyMatchThing(executor, args[0]);

    if (dbObj == HSNOTHING)
        return "#-1";

    if (!hsInterface.IsNearby(executor, dbObj)
        && !hsInterface.CanSeeAll(executor))
    {
        hsInterface.Notify(executor, "I don't see that here.");
        return "#-1";
    }

    if (hsInterface.HasFlag(dbObj, TYPE_THING, THING_HSPACE_OBJECT))
        cObj = dbHSDB.FindObject(dbObj);
    else if (hsInterface.HasFlag(dbObj, TYPE_THING, THING_HSPACE_CONSOLE))
        cObj = dbHSDB.FindObjectByConsole(dbObj);
    else
        cObj = NULL;

    if (!cObj)
        return "#-1 Target object is not an HSpace object.";

    CHSShip *dObj;

    if (cObj->GetType() != HST_SHIP)
        return "#-1 Target object does not have systems.";
    else
        dObj = (CHSShip *) cObj;

    // We've got the list, now translate the types into
    // names, and put them in the return buffer.
    static char rbuf[512];
    const char *ptr;
    CHSEngSystem *cSys;
    char tbuf[32];

    *rbuf = '\0';

    for (cSys = dObj->GetSystems().GetHead(); cSys; cSys = cSys->GetNext())
    {
        // Find the system name
        if (cSys->GetType() != HSS_FICTIONAL)
            ptr = hsGetEngSystemName(cSys->GetType());
        else
            ptr = cSys->GetName();

        if (!*rbuf)
            sprintf(tbuf, "%s", ptr ? ptr : "Unknown");
        else
            sprintf(tbuf, ",%s", ptr ? ptr : "Unknown");

        strcat(rbuf, tbuf);
    }
    return rbuf;
}

HSPACE_FUNC_HDR(hsfGetSensorContacts)
{
    HS_DBREF dbObj;
    int oType;                  // Type of object to report

    dbObj = hsInterface.NoisyMatchThing(executor, args[0]);
    if (dbObj == HSNOTHING)
        return "#-1 No object found.";

    CHS3DObject *cObj;
    if (hsInterface.HasFlag(dbObj, TYPE_THING, THING_HSPACE_CONSOLE))
        cObj = dbHSDB.FindObjectByConsole(dbObj);
    else
        cObj = dbHSDB.FindObject(dbObj);

    if (!cObj)
        return "#-1 Not a HSpace object.";

    // Grab the sensors from the object
    CHSSysSensors *cSensors;
    cSensors = (CHSSysSensors *) cObj->GetEngSystem(HSS_SENSORS);
    if (!cSensors)
        return "#-1 Object has no sensors.";

    int nContacts;
    nContacts = cSensors->NumContacts();
    if (!nContacts)
        return "";

    oType = atoi(args[1]);
    int idx;
    SENSOR_CONTACT *cContact;
    CHS3DObject *cTarget;
    static char rbuf[1024];
    char tbuf[128];
    *rbuf = '\0';

    double sX, sY, sZ;          // Source coords
    double tX, tY, tZ;          // Contact coords

    sX = cObj->GetX();
    sY = cObj->GetY();
    sZ = cObj->GetZ();
    for (idx = 0; idx < HS_MAX_CONTACTS; idx++)
    {
        // Grab the next contact
        cContact = cSensors->GetContact(idx);
        if (!cContact)
            continue;

        // Does the type match what is requested?
        cTarget = cContact->m_obj;
        if (oType != HST_NOTYPE && (cTarget->GetType() != oType))
            continue;

        CHSShip *cShip;
        cShip = NULL;
        if (cTarget->GetType() == HST_SHIP)
            cShip = (CHSShip *) cTarget;


        // Grab target coords
        tX = cTarget->GetX();
        tY = cTarget->GetY();
        tZ = cTarget->GetZ();

        // Print out object info
        if (!*rbuf)
        {
            if (!cShip)
                sprintf(tbuf, "%d:#%d:%d:%d:%s:%d:%d:%.4f",
                        cContact->m_id,
                        cTarget->GetDbref(),
                        cTarget->GetType(),
                        cTarget->GetSize(),
                        cContact->status ==
                        DETECTED ? "Unknown" : cTarget->GetName(), XYAngle(sX,
                                                                           sY,
                                                                           tX,
                                                                           tY),
                        ZAngle(sX, sY, sZ, tX, tY, tZ), Dist3D(sX, sY, sZ, tX,
                                                               tY, tZ));
            else
                sprintf(tbuf, "%d:#%d:%d:%d:%s:%d:%d:%.4f:%d:%d:%d",
                        cContact->m_id,
                        cTarget->GetDbref(),
                        cTarget->GetType(),
                        cTarget->GetSize(),
                        cContact->status ==
                        DETECTED ? "Unknown" : cTarget->GetName(), XYAngle(sX,
                                                                           sY,
                                                                           tX,
                                                                           tY),
                        ZAngle(sX, sY, sZ, tX, tY, tZ), Dist3D(sX, sY, sZ, tX,
                                                               tY, tZ),
                        cShip->GetXYHeading(), cShip->GetZHeading(),
                        cShip->GetSpeed());
        }
        else
        {
            if (!cShip)
                sprintf(tbuf, "|%d:#%d:%d:%d:%s:%d:%d:%.4f",
                        cContact->m_id,
                        cTarget->GetDbref(),
                        cTarget->GetType(),
                        cTarget->GetSize(),
                        cContact->status ==
                        DETECTED ? "Unknown" : cTarget->GetName(), XYAngle(sX,
                                                                           sY,
                                                                           tX,
                                                                           tY),
                        ZAngle(sX, sY, sZ, tX, tY, tZ), Dist3D(sX, sY, sZ, tX,
                                                               tY, tZ));
            else
                sprintf(tbuf, "|%d:#%d:%d:%d:%s:%d:%d:%.4f:%d:%d:%d",
                        cContact->m_id,
                        cTarget->GetDbref(),
                        cTarget->GetType(),
                        cTarget->GetSize(),
                        cContact->status ==
                        DETECTED ? "Unknown" : cTarget->GetName(), XYAngle(sX,
                                                                           sY,
                                                                           tX,
                                                                           tY),
                        ZAngle(sX, sY, sZ, tX, tY, tZ), Dist3D(sX, sY, sZ, tX,
                                                               tY, tZ),
                        cShip->GetXYHeading(), cShip->GetZHeading(),
                        cShip->GetSpeed());
        }

        strcat(rbuf, tbuf);
    }
    return rbuf;
}


HSPACE_FUNC_HDR(hsfSysAttr)
{
    HS_BOOL8 bAdjusted = true;
    HS_DBREF dbObj;
    char *ptr;
    char lpstrAttr[32];

    // Args[0] should be obj/attr, so drop a null in the 
    // split.
    ptr = strchr(args[0], '/');
    if (!ptr)
        return "#-1 Invalid object/system combination.";

    *ptr = '\0';
    // Find the object in question
    dbObj = hsInterface.NoisyMatchThing(executor, args[0]);

    if (dbObj == HSNOTHING)
        return "#-1";

    if (!hsInterface.IsNearby(executor, dbObj)
        && !hsInterface.CanSeeAll(executor))
    {
        hsInterface.Notify(executor, "I don't see that here.");
        return "#-1";
    }

    CHS3DObject *cObj;
    // If it's a console, search for the object based on
    // the console.
    if (hsInterface.HasFlag(dbObj, TYPE_THING, THING_HSPACE_CONSOLE))
        cObj = dbHSDB.FindObjectByConsole(dbObj);
    else
        cObj = dbHSDB.FindObject(dbObj);

    if (!cObj)
        return "#-1 Invalid HSpace object.";

    // The rest of the string (after the /) should be
    // the attribute name.
    ptr++;
    strncpy(lpstrAttr, ptr, 31);
    lpstrAttr[31] = '\0';

    // Determine what type of system is being queried.
    HSS_TYPE type;
    CHSEngSystem *cSys;
    type = hsGetEngSystemType(lpstrAttr);
    if (type == HSS_NOTYPE && cObj->GetType() == HST_SHIP
        || type == HSS_FICTIONAL && cObj->GetType() == HST_SHIP)
    {
        CHSShip *dObj;
        dObj = (CHSShip *) cObj;
        cSys = dObj->GetSystems().GetSystemByName(lpstrAttr);
        if (!cSys)
            return "#-1 Invalid system specified.";
    }
    else if (type != HSS_NOTYPE)
    {
        cSys = cObj->GetEngSystem(type);
    }
    else
        return "#-1 Invalid system specified.";

    // Ask the object for the system.
    if (!cSys)
        return "#-1 System not present.";

    if (args[2] && *args[2])
        bAdjusted = *args[2] == '0' ? false : true;

    // Query the system for the attribute.
    CHSVariant varValue;
    if (cSys->GetAttributeValue(args[1], varValue, bAdjusted, false))
    {
        // Determine attribute return type.
        static HS_INT8 cBuffer[256];

        switch (varValue.GetType())
        {
        case CHSVariant::VT_BOOL:
            sprintf(cBuffer, "%s", varValue.GetBool() == true ? "1" : "0");
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
            return "#-1 Unknown variant value returned from attribute query.";
        }

        return cBuffer;
    }
    else
    {
        return "#-1 Failed to query system attribute.";
    }
}

HSPACE_FUNC_HDR(hsfSysSet)
{
    HS_DBREF dbObj;
    char *ptr;
    char strSysName[32];

    // The object/attribute pair is specified by a slash
    // in between.  Separate them.
    ptr = strchr(args[0], '/');
    if (!ptr)
        return "#-1 Invalid object/system pair specified.";

    *ptr = '\0';

    // Find the object in question
    dbObj = hsInterface.NoisyMatchThing(executor, args[0]);

    if (dbObj == HSNOTHING)
        return "#-1";

    // Executor must control the object
    if (!hsInterface.ControlsObject(executor, dbObj))
    {
        return "#-1 Permission denied.";
    }

    CHS3DObject *cObj;

    cObj = dbHSDB.FindObject(dbObj);
    if (NULL == cObj)
    {
        cObj = dbHSDB.FindObjectByConsole(dbObj);

        if (NULL == cObj)
        {
            return "#-1 Invalid HSpace object.";
        }
    }

    // The rest of the string (after the /) should be
    // the system name.
    ptr++;
    strncpy(strSysName, ptr, 31);
    strSysName[31] = '\0';

    // Determine what type of system is being queried.
    HSS_TYPE type;
    CHSEngSystem *cSys;
    type = hsGetEngSystemType(strSysName);
    if (type == HSS_NOTYPE && cObj->GetType() == HST_SHIP ||
        type == HSS_FICTIONAL && cObj->GetType() == HST_SHIP)
    {
        CHSShip *dObj;
        dObj = (CHSShip *) cObj;
        cSys = dObj->GetSystems().GetSystemByName(strSysName);
        if (!cSys)
            return "#-1 Invalid system specified.";
    }
    else if (type != HSS_NOTYPE)
    {
        cSys = cObj->GetEngSystem(type);
    }
    else
        return "#-1 Invalid system specified.";

    if (!cSys)
        return "#-1 System not present.";

    // Now set the attribute value
    char strAttr[32];
    char strValue[32];

    // Look for a colon separating the attr:value pair.  It's
    // ok to have an empty value.
    ptr = strchr(args[1], ':');
    if (!ptr)
    {
        strncpy(strAttr, args[1], 31);
        strAttr[31] = '\0';
        strValue[0] = '\0';
    }
    else
    {
        *ptr = '\0';
        strncpy(strAttr, args[1], 31);
        strAttr[31] = '\0';

        ptr++;
        strncpy(strValue, ptr, 31);
        strValue[31] = '\0';
    }

    // Tell the system to set the attribute
    if (!cSys->SetAttributeValue(strAttr, strValue))
    {
        return "Attribute - failed.";
    }
    else
    {
        return "Attribute - set.";
    }
}

HSPACE_FUNC_HDR(hsfCommMsg)
{
    // See if the executor is a COMM console
    if (!hsInterface.HasFlag(executor, TYPE_THING, THING_HSPACE_COMM)
        && !hsInterface.HasFlag(executor, TYPE_THING, THING_HSPACE_CONSOLE))
        return "#-1 Must have HSPACE_COMM or HSPACE_CONSOLE flag to do that.";

    // Grab an HSCOMM struct and fill in the info.
    HSCOMM commdata;

    // Try to find a source HSpace obj based on the executor.
    CHS3DObject *cObj;
    cObj = dbHSDB.FindObjectByConsole(executor);
    if (cObj)
    {
        commdata.cObj = cObj;
        commdata.dbSource = cObj->GetDbref();
    }
    else
    {
        commdata.cObj = NULL;
        commdata.dbSource = executor;
    }

    // Source UID
    commdata.suid = atoi(args[0]);

    // Dest UID
    commdata.duid = atoi(args[1]);

    // Source coords
    commdata.sX = atoi(args[2]);
    commdata.sY = atoi(args[3]);
    commdata.sZ = atoi(args[4]);

    // Max range
    commdata.dMaxDist = atoi(args[5]);

    // Frequency
    commdata.frq = atof(args[6]);

    // Message
    commdata.msg = args[7];

    if (cmRelay.RelayMessage(&commdata))
        return "Sent.";
    else
        return "Failed.";
}

HSPACE_FUNC_HDR(hsfSetMissile)
{
    HS_DBREF dbObj;

    // Find the object in question
    dbObj = hsInterface.NoisyMatchThing(executor, args[0]);

    if (dbObj == HSNOTHING)
        return "#-1";

    // Executor must control the object
    if (!hsInterface.ControlsObject(executor, dbObj))
    {
        return "#-1 Permission denied.";
    }

    CHS3DObject *cObj;

    cObj = dbHSDB.FindObject(dbObj);
    if (!cObj)
        return "#-1 Invalid HSpace object.";

    // The object must be a ship.
    if (cObj->GetType() != HST_SHIP)
        return "#-1 Not an HSpace ship.";

    CHSShip *cShip;
    cShip = (CHSShip *) cObj;
    if (cShip->SetNumMissiles(executor, atoi(args[1]), atoi(args[2])))
        return "Value - set.";
    else
        return "Value - failed.";
}

HSPACE_FUNC_HDR(hsfClone)
{
    HS_DBREF dbObj;

    // Find the object in question
    dbObj = hsInterface.NoisyMatchThing(executor, args[0]);

    if (dbObj == HSNOTHING)
        return "#-1";

    // Executor must be wizard
    if (!hsInterface.IsWizard(executor))
    {
        return "#-1 Permission denied.";
    }

    CHS3DObject *cObj;

    cObj = dbHSDB.FindObject(dbObj);
    if (!cObj)
        return "#-1 Invalid HSpace object.";

    // The object must be a ship.
    if (cObj->GetType() != HST_SHIP)
        return "#-1 Not an HSpace ship.";

    CHSShip *cShip;
    cShip = (CHSShip *) cObj;

    // Clone the ship.
    dbObj = cShip->Clone();

    static char tbuf[64];
    sprintf(tbuf, "#%d", dbObj);
    return tbuf;
}

HSPACE_FUNC_HDR(hsfDelSys)
{
    HS_DBREF dbObj;
    CHSShip *pShip;
    CHS3DObject *cObj;
    CHSEngSystem *cSys;

    // Parse out the parts of the command.
    // Command format is:
    //
    // @space/sysset obj:system/attr=value
    if (!args[0] || !args[1])
        return "You must specify an object and system name.";

    // Pull out the object of interest

    dbObj = hsInterface.NoisyMatchThing(executor, args[0]);
    if (dbObj == HSNOTHING)
        return "#-1";

    if (hsInterface.HasFlag(dbObj, TYPE_THING, THING_HSPACE_CONSOLE))
        cObj = dbHSDB.FindObjectByConsole(dbObj);
    else
        cObj = dbHSDB.FindObject(dbObj);


    if (!cObj)
        return "#-1 That thing does not appear to be an HSpace object.";


    // Currently only support ships
    if (cObj->GetType() != HST_SHIP)
        return
            "#-1 That HSpace object does not currently support that operation.";

    pShip = (CHSShip *) cObj;

    // Find the system based on the name
    HSS_TYPE type;

    type = hsGetEngSystemType(args[1]);
    if (type == HSS_NOTYPE || type == HSS_FICTIONAL)
    {
        cSys = pShip->GetSystems().GetSystemByName(args[1]);
        if (!cSys)
        {
            return "#-1 Invalid system name specified.";
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
            return "#-1 That system does not exists for that ship.";
        }
    }


    char tmp[64];
    sprintf(tmp, "%s", cSys->GetName());

    // Delete the system
    if (pShip->GetSystems().DelSystem(cSys))
        return hsInterface.HSPrintf("%s system deleted from the %s.", tmp,
                                    pShip->GetName());
    else
        return "Failed to delete system.";
}

HSPACE_FUNC_HDR(hsfAddSys)
{
    HS_DBREF dbObj;
    CHSShip *pShip;
    CHS3DObject *cObj;

    // Parse out the parts of the command.
    // Command format is:
    //
    // @space/sysset obj:system/attr=value
    if (!args[0] || !args[1])
        return "#-1 You must specify an object and system name.";

    // Pull out the object of interest

    dbObj = hsInterface.NoisyMatchThing(executor, args[0]);
    if (dbObj == HSNOTHING)
        return "#-1";

    // Find the system based on the name
    HSS_TYPE type;

    type = hsGetEngSystemType(args[1]);
    if (type == HSS_NOTYPE)
        return "#-1 Invalid system name specified.";

    if (hsInterface.HasFlag(dbObj, TYPE_THING, THING_HSPACE_CONSOLE))
        cObj = dbHSDB.FindObjectByConsole(dbObj);
    else
        cObj = dbHSDB.FindObject(dbObj);

    if (!cObj)
        return "#-1 That thing does not appear to be an HSpace object.";

    // Currently only support ships
    if (cObj->GetType() != HST_SHIP)
        return
            "#-1 That HSpace object does not currently support that operation.";


    pShip = (CHSShip *) cObj;

    // Try to find the system already on the class
    CHSEngSystem *cSys;

    if (type != HSS_FICTIONAL)
    {
        cSys = pShip->GetSystems().GetSystem(type);
        if (cSys)
            return "#-1 That system already exists for that ship.";
    }

    // Add the system
    cSys = CHSEngSystem::CreateFromType(type);
    if (!cSys)
        return "#-1 Failed to add the system to the specified ship.";

    pShip->GetSystems().AddSystem(cSys);
    cSys->SetOwner(pShip);
    return hsInterface.HSPrintf("%s system added to the %s.", cSys->GetName(),
                                pShip->GetName());
}

HSPACE_FUNC_HDR(hsfList)
{
    int uid;
    int cType;
    int check_active = 1;

    static std::string mStr;
    mStr.clear();

    // Executor must be a wizard
    if (!hsInterface.IsWizard(executor))
        return "#-1 Permission denied.";

    uid = atoi(args[0]);
    CHSUniverse *uDest = CHSUniverseDB::GetInstance().FindUniverse(uid);
    if (!uDest)
    {
        // no such universe
        return "#-1 No such universe.";
    }

    cType = atoi(args[1]);
    if (cType < 0)
    {
        return "#-1 Invalid HS-TYPE.";
    }

    check_active = atoi(args[2]);

    THSObjectIterator tIterator;
    HS_BOOL8 bContinue;
    for (bContinue = uDest->GetFirstObject(tIterator, HST_NOTYPE); bContinue;
         bContinue = uDest->GetNextObject(tIterator, HST_NOTYPE))
    {
        CHS3DObject *obj = static_cast < CHS3DObject * >(tIterator.pValue);

        if (check_active)
        {
            if (obj->IsActive() == false)
            {
                continue;
            }
        }

        if (cType != 0)         // 0 means all objects
        {
            // skip objects not matching the specified type
            if (obj->GetType() != cType)
            {
                continue;
            }
        }
        // add to list
        if(true == mStr.empty())
        {
            mStr += hsInterface.HSPrintf("#%d", obj->GetDbref());
        }
        else
        {
            mStr += hsInterface.HSPrintf(" #%d", obj->GetDbref());
        }
    }

    return (char *) mStr.c_str();

}

// hs_nearby(uid, x, y, z, range)
//   Return all objects from the specified universe that are within max_range 
//   distance of the provided XYZ coordinates. This includes ships that may
//   be cloaked or jumping and inactive objects.
HSPACE_FUNC_HDR(hsfNearby)
{
    double dX, dY, dZ;
    double tX, tY, tZ;
    double dDist;
    double max_range;
    int uid;
    CHSUniverse *uDest;

    static std::string mStr;
    // Make sure it is clear
    mStr.clear();

    // Executor must be a wizard
    if (!hsInterface.IsWizard(executor))
        return "#-1 Permission denied.";

    uid = atoi(args[0]);
    uDest = CHSUniverseDB::GetInstance().FindUniverse(uid);
    if (!uDest)
    {
        // no such universe
        return "#-1 No such universe.";
    }

    dX = atof(args[1]);
    dY = atof(args[2]);
    dZ = atof(args[3]);
    max_range = atof(args[4]);

    if (max_range < 0)
    {
        return "#-1 Invalid range.";
    }

    THSObjectIterator tIterator;
    HS_BOOL8 bContinue;
    for (bContinue = uDest->GetFirstObject(tIterator, HST_NOTYPE); bContinue;
         bContinue = uDest->GetNextObject(tIterator, HST_NOTYPE))
    {
        CHS3DObject *obj = static_cast < CHS3DObject * >(tIterator.pValue);

        // Skip over inactive space objects...
        if (obj->IsActive() == false)
        {
            continue;
        }

        tX = obj->GetX();
        tY = obj->GetY();
        tZ = obj->GetZ();

        dDist = Dist3D(dX, dY, dZ, tX, tY, tZ);

        if (dDist <= max_range)
        {
            // add to list
            if(true == mStr.empty())
            {
                mStr = hsInterface.HSPrintf("#%d", obj->GetDbref());
            }
            else
            {
                mStr += hsInterface.HSPrintf(" #%d", obj->GetDbref());
            }
        }
    }

    return (char *) mStr.c_str();
}

HSPACE_FUNC_HDR(hsfExplode)
{
    double sX, sY, sZ;
    double tX, tY, tZ;
    double dDist;
    double max_range;
    int uid;
    int strength;
    CHSUniverse *uDest;

    // Executor must be a wizard
    if (!hsInterface.IsWizard(executor))
        return "#-1 Permission denied.";

    uid = atoi(args[0]);
    uDest = CHSUniverseDB::GetInstance().FindUniverse(uid);
    if (!uDest)
    {
        // no such universe
        return "#-1 No such universe.";
    }

    sX = atof(args[1]);
    sY = atof(args[2]);
    sZ = atof(args[3]);
    max_range = atof(args[4]);
    strength = atoi(args[5]);

    if (max_range < 0)
    {
        return "#-1 Invalid range.";
    }

    THSObjectIterator tIterator;
    HS_BOOL8 bContinue;
    for (bContinue = uDest->GetFirstObject(tIterator, HST_NOTYPE); bContinue;
         bContinue = uDest->GetNextObject(tIterator, HST_NOTYPE))
    {
        CHS3DObject *obj = static_cast < CHS3DObject * >(tIterator.pValue);

        // Skip over inactive space objects...
        if (obj->IsActive() == false)
        {
            continue;
        }

        tX = obj->GetX();
        tY = obj->GetY();
        tZ = obj->GetZ();

        dDist = Dist3D(sX, sY, sZ, tX, tY, tZ);

        if (dDist <= max_range)
        {
            // Dole out damage based on the distance from the center
            // point of the explosion. Objects at greater distances
            // take less damage.
            if (obj->GetType() == HST_SHIP)
            {
                CHSShip *ship = static_cast < CHSShip * >(obj);
                ship->HandleDamage(sX, sY, sZ,
                                   (HS_INT32) (strength *
                                               (dDist / max_range)));
            }
            else if (obj->GetType() == HST_MISSILE)
            {
                // damage missile here
                CHSMissile *missile = static_cast < CHSMissile * >(obj);
                missile->HandleDamage(sX, sY, sZ,
                                      (HS_INT32) (strength *
                                                  (dDist / max_range)));
            }
        }
    }

    return "";
}

HSPACE_FUNC_HDR(hsfConsoleCommand)
{
    CHSShip *cShip;
    HSPACE_COMMAND *hCmdArray;
    HSPACE_COMMAND *hCmd;

    hsEnterMutex();

    cShip = (CHSShip *) dbHSDB.FindObjectByConsole(executor);
    if (!cShip || (cShip->GetType() != HST_SHIP))
    {
        hsLeaveMutex();
        return "#-1 PERMISSION DENIED";
    }

    if (!strcasecmp(args[0], "nav"))
        hCmdArray = hsNavCommandArray;
    else if (!strcasecmp(args[0], "eng"))
        hCmdArray = hsEngCommandArray;
    else if (!strcasecmp(args[0], "con"))
        hCmdArray = hsConCommandArray;
    else
    {
        hsLeaveMutex();
        return "#-1 INVALID COMMAND";
    }

    hCmd = hsFindCommand(args[1], hCmdArray);
    if (!hCmd)
    {
        hsLeaveMutex();
        return "#-1 INVALID SWITCH";
    }

    hCmd->func(executor, args[2], args[3]);
    hsLeaveMutex();
    return "";
}
