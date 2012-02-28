// -----------------------------------------------------------------------
// $Id: hsdb.cpp,v 1.8 2006/04/04 12:41:11 mark Exp $
// -----------------------------------------------------------------------

#include "pch.h"

#include <cstring>
#include <stdlib.h>
#include <cstdio>

#include "hscopyright.h"
#include "hsdb.h"
#include "hsconf.h"
#include "hsweapon.h"
#include "hsinterface.h"
#include "hsutils.h"
#include "hsuniverse.h"
#include "hsuniversedb.h"
#include "hsclass.h"
#include "hsterritory.h"
#include "hsflags.h"
#include "hscelestial.h"

// The HSpace database object instance
CHSDB dbHSDB;

CHSDB::CHSDB():
m_nWeapons(0), m_nUniverses(0)
{
}

HS_BOOL8 CHSDB::LoadDatabases()
{
    char tbuf[256];
    int cnt;


    // Load classes
    if (!LoadClassDB(HSCONF.classdb))
        hs_log("ERROR: No class DB found.");
    // Load weapons
    if (!LoadWeaponDB(HSCONF.weapondb))
        hs_log("ERROR: No weapon DB found.");
    m_nWeapons = waWeapons.NumWeapons();

    // Load universes
    if (!LoadUniverseDB(HSCONF.univdb))
        hs_log("ERROR: No universe DB found.");
    m_nUniverses = CHSUniverseDB::GetInstance().GetNumUniverses();

    // Load planets
    LoadObjectDB();

    // Load Territories
    sprintf_s(tbuf, "LOADING: %s", HSCONF.territorydb);
    hs_log(tbuf);
    LoadTerritoryDB(HSCONF.territorydb);
    cnt = taTerritories.NumTerritories();
    sprintf_s(tbuf, "LOADING: %d %s loaded.", cnt,
            cnt == 1 ? "territory" : "territories");
    hs_log(tbuf);

    // Rename the input files and then dump the loaded set to disk
    BackupDatabases(".prev");
    DumpDatabases();

    return true;
}

HS_BOOL8 CHSDB::LoadUniverseDB(char *lpstrDBPath)
{
    return CHSUniverseDB::GetInstance().LoadFromFile(lpstrDBPath);
}

void CHSDB::DumpUniverseDB(char *lpstrDBPath)
{

    CHSUniverseDB::GetInstance().SaveToFile(lpstrDBPath);
}

HS_BOOL8 CHSDB::LoadClassDB(char *lpstrDBPath)
{
    return CHSClassDB::GetInstance().LoadFromFile(lpstrDBPath);
}

void CHSDB::DumpClassDB(char *lpstrDBPath)
{
    CHSClassDB::GetInstance().SaveToFile(lpstrDBPath);
}

HS_BOOL8 CHSDB::LoadWeaponDB(char *lpstrDBPath)
{
    return waWeapons.LoadFromFile(lpstrDBPath);
}

void CHSDB::DumpWeaponDB(char *lpstrDBPath)
{
    waWeapons.SaveToFile(lpstrDBPath);
}

HS_BOOL8 CHSDB::LoadTerritoryDB(char *lpstrDBPath)
{
    return taTerritories.LoadFromFile(lpstrDBPath);
}

void CHSDB::DumpTerritoryDB(char *lpstrDBPath)
{
    taTerritories.SaveToFile(lpstrDBPath);
}

HS_BOOL8 CHSDB::LoadObjectDB()
{
    FILE *fp;
    char tbuf[256];
    char strKey[64];
    char strValue[64];
    char *ptr;
    HS_UINT32 type;
    HS_DBKEY key;
    HS_UINT32 nShips, nCelestials, nObjs;
    CHS3DObject *newObj;

    sprintf_s(tbuf, "LOADING: %s ...", HSCONF.objectdb);
    hs_log(tbuf);

    // Load the object database, pulling out key/value pairs.
    // There should be an OBJECTTYPE attribute after each
    // new object definition.  Thus, we just allocate that
    // type of object, and tell it to load from the file.
    nShips = nCelestials = nObjs = 0;
    fopen_s(&fp, HSCONF.objectdb, "r");
    if (!fp)
    {
        hs_log("ERROR: Unable to open object database for loading!");
        return false;
    }

    while (fgets(tbuf, 256, fp))
    {
        // Truncate newline chars
        if ((ptr = strchr(tbuf, '\n')) != NULL)
            *ptr = '\0';
        if ((ptr = strchr(tbuf, '\r')) != NULL)
            *ptr = '\0';

        // Check for end of DB
        if (!strcmp(tbuf, "*END*"))
            break;

        // Extract key and value
        extract(tbuf, strKey, 0, 1, '=');
        extract(tbuf, strValue, 1, 1, '=');


        // Determine key type, then do something.
        key = HSFindKey(strKey);
        switch (key)
        {
        case HSK_NOKEY:
            sprintf_s(tbuf, "WARNING: Invalid key \"%s\" encountered.", strKey);
            hs_log(tbuf);
            break;

        case HSK_DBVERSION:    // This should be the first line, if present
            break;

        case HSK_OBJECTTYPE:   // NEW OBJECT!
            // Determine type, allocate object, and tell
            // it to load.
            type = atoi(strValue);
            switch (type)
            {
            case HST_SHIP:
                newObj = new CHSShip;
                if (newObj->LoadFromFile(fp))
                    nShips++;
                break;

            case HST_PLANET:
                newObj = new CHSPlanet;
                if (newObj->LoadFromFile(fp))
                    nCelestials++;
                break;

            case HST_NEBULA:
                newObj = new CHSNebula;
                if (newObj->LoadFromFile(fp))
                    nCelestials++;
                break;

            case HST_ASTEROID:
                newObj = new CHSAsteroid;
                if (newObj->LoadFromFile(fp))
                    nCelestials++;
                break;

            case HST_BLACKHOLE:
                newObj = new CHSBlackHole;
                if (newObj->LoadFromFile(fp))
                    nCelestials++;
                break;

            case HST_WORMHOLE:
                newObj = new CHSWormHole;
                if (newObj->LoadFromFile(fp))
                    nCelestials++;
                break;

            default:           // Generic CHS3DObject
                newObj = new CHS3DObject;
                if (newObj->LoadFromFile(fp))
                    nObjs++;
                break;
            }
            break;

        default:
            sprintf_s(tbuf,
                    "WARNING: Key \"%s\" encountered but not handled.",
                    strKey);
            hs_log(tbuf);
            break;
        }
    }
    fclose(fp);
    sprintf_s(tbuf, "LOADING: %d generic objects loaded.", nObjs);
    hs_log(tbuf);
    sprintf_s(tbuf, "LOADING: %d celestial objects loaded.", nCelestials);
    hs_log(tbuf);
    sprintf_s(tbuf, "LOADING: %d ship objects loaded (Done).", nShips);
    hs_log(tbuf);

    return true;
}

void CHSDB::DumpObjectDB()
{
    FILE *fp;
    char tbuf[256];

    // Is the file path good?
    if (strlen(HSCONF.objectdb) == 0)
    {
        hs_log
            ("Attempt to save the object database, but no path specified.  Could be a config file problem.");
        return;
    }

    fopen_s(&fp, HSCONF.objectdb, "w");
    if (!fp)
    {
        sprintf_s(tbuf, "ERROR: Unable to open object db \"%s\" for writing.",
                HSCONF.objectdb);
        hs_log(tbuf);
        return;
    }

    // DBVERSION marker
    fprintf(fp, "DBVERSION=4.0\n");

    // Now write out all of the objects in all of the universes
    THSUniverseIterator tIter;
    HS_BOOL8 bIter;
    for (bIter = CHSUniverseDB::GetInstance().GetFirstUniverse(tIter); bIter;
         bIter = CHSUniverseDB::GetInstance().GetNextUniverse(tIter))
    {
        CHSUniverse *pUniverse = tIter.pValue;

        // Now grab all of the objects in the universe, and
        // call upon them to dump their attributes.
        THSObjectIterator tIterator;
        HS_BOOL8 bContinue;
        for (bContinue = pUniverse->GetFirstObject(tIterator); bContinue;
             bContinue = pUniverse->GetNextObject(tIterator))
        {
            CHS3DObject *pObject = tIterator.pValue;
            fprintf(fp, "OBJECTTYPE=%d\n", pObject->GetType());
            pObject->WriteToFile(fp);
            fprintf(fp, "OBJECTEND\n");
        }
    }
    fprintf(fp, "*END*\n");
    fclose(fp);
}

void CHSDB::DumpDatabases()
{
    char ostr[256];

    // Save classes
    sprintf_s(ostr, "Saving class database '%s'.", HSCONF.classdb);
    hs_log(ostr);
    DumpClassDB(HSCONF.classdb);

    // Save weapons
    sprintf_s(ostr, "Saving weapon database '%s'.", HSCONF.weapondb);
    hs_log(ostr);
    DumpWeaponDB(HSCONF.weapondb);

    // Save universes
    sprintf_s(ostr, "Saving universe database '%s'.", HSCONF.univdb);
    hs_log(ostr);
    DumpUniverseDB(HSCONF.univdb);

    // Save planets
    sprintf_s(ostr, "Saving object database '%s'.", HSCONF.objectdb);
    hs_log(ostr);
    DumpObjectDB();

    // Territories
    sprintf_s(ostr, "Saving territory database '%s'.", HSCONF.territorydb);
    hs_log(ostr);
    taTerritories.SaveToFile(HSCONF.territorydb);

    hs_log("Database save complete.");
}

// Call this member function to clean up attributes that are 
// written to objects during a database save.
void CHSDB::CleanupDBAttrs()
{
    // Loop through all universes and all objects in those
    // universes, clearing attributes.
    THSUniverseIterator tIter;
    HS_BOOL8 bIter;
    for (bIter = CHSUniverseDB::GetInstance().GetFirstUniverse(tIter); bIter;
         bIter = CHSUniverseDB::GetInstance().GetNextUniverse(tIter))
    {
        CHSUniverse *pUniverse = tIter.pValue;

        // Grab all objects, and tell them to cleanup their acts!
        THSObjectIterator tIterator;
        HS_BOOL8 bContinue;
        for (bContinue = pUniverse->GetFirstObject(tIterator); bContinue;
             bContinue = pUniverse->GetNextObject(tIterator))
        {
            CHS3DObject *pObject = tIterator.pValue;
            pObject->ClearObjectAttrs();
        }
    }
}

CHSShip *CHSDB::FindShip(int objnum)
{
    CHSShip *rShip;

    // Traverse all universes, looking for the ship
    THSUniverseIterator tIter;
    HS_BOOL8 bIter;
    for (bIter = CHSUniverseDB::GetInstance().GetFirstUniverse(tIter); bIter;
         bIter = CHSUniverseDB::GetInstance().GetNextUniverse(tIter))
    {
        CHSUniverse *pUniverse = tIter.pValue;
        if ((rShip = static_cast < CHSShip * >
             (pUniverse->FindObject(objnum, HST_SHIP))))
            return rShip;
    }

    // No ship found
    return NULL;
}

CHSCelestial *CHSDB::FindCelestial(int objnum)
{
    CHSCelestial *rCel;

    // Traverse all universes, looking for the celestial
    THSUniverseIterator tIter;
    HS_BOOL8 bIter;
    for (bIter = CHSUniverseDB::GetInstance().GetFirstUniverse(tIter); bIter;
         bIter = CHSUniverseDB::GetInstance().GetNextUniverse(tIter))
    {
        CHSUniverse *pUniverse = tIter.pValue;
        // We can use HST_PLANET here because the FindObject()
        // returns a generic celestial given any of the celestial
        // types.
        if ((rCel = static_cast < CHSCelestial * >
             (pUniverse->FindObject(objnum, HST_PLANET))))
            return rCel;
    }

    // No celestial found
    return NULL;
}

// Generically looks for an object in the universes.  It is
// slightly optimized to save time by looking at the HSDB_TYPE
// attribute of the object and only looking through those types
// of objects.
CHS3DObject *CHSDB::FindObject(int objnum)
{
    HS_TYPE hstType;
    CHS3DObject *pObject = NULL;

    // Grab the TYPE attribute
    if (!hsInterface.AtrGet(objnum, "HSDB_TYPE"))
        return NULL;
    hstType = (HS_TYPE) atoi(hsInterface.m_buffer);

    // Depending on the type of attribute, call a variety of
    // functions.
    switch (hstType)
    {
    case HST_SHIP:
        pObject = FindShip(objnum);
        break;

    case HST_PLANET:
        pObject = FindCelestial(objnum);
        break;

    default:
        // Traverse all universes, looking for the object
        THSUniverseIterator tIter;
        HS_BOOL8 bIter;
        for (bIter = CHSUniverseDB::GetInstance().GetFirstUniverse(tIter);
             bIter;
             bIter = CHSUniverseDB::GetInstance().GetNextUniverse(tIter))
        {
            CHSUniverse *pUniverse = tIter.pValue;
            if ((pObject = pUniverse->FindObject(objnum)))
            {
                return pObject;
            }
        }
        return NULL;
    }
    return pObject;
}

// Call this function to allow a player to create a new universe
// with a given room #.
void CHSDB::CreateNewUniverse(HS_DBREF player, HS_INT8 * strRoom)
{
    HS_DBREF dbRoom;            // Room to use as universe

    // arg_left is the HS_DBREF of the room to represent the
    // universe.
    dbRoom = hsInterface.NoisyMatchRoom(player, strRoom);
    if (dbRoom == HSNOTHING)
        return;
    if (hsInterface.GetType(dbRoom) != TYPE_ROOM)
    {
        hsInterface.Notify(player,
                           "You must specify a room to represent this universe.");
        return;
    }

    // Grab a new universe, but check that it was created
    CHSUniverse *pNewUniverse;

    pNewUniverse = new CHSUniverse;

    // Set the objnum
    pNewUniverse->SetID(dbRoom);

    CHSUniverseDB::GetInstance().AddUniverse(pNewUniverse);

    // Set the name
    pNewUniverse->SetName((char *) hsInterface.GetName(dbRoom));

    // Set the flag on the room
    hsInterface.SetToggle(dbRoom, ROOM_HSPACE_UNIVERSE);

    // Increment number of universes
    m_nUniverses++;

    hsInterface.Notify(player,
                       hsInterface.
                       HSPrintf("Universe %s (#%d) created.  Behold!",
                                pNewUniverse->GetName(),
                                pNewUniverse->GetID()));
}

// Call this function to allow a player to delete an existing
// universe with a given universe ID.
void CHSDB::DeleteUniverse(HS_DBREF player, char *strID)
{
    int uid;

    if (!strID || !*strID)
    {
        hsInterface.Notify(player, "You must specify the uid of a universe.");
        return;
    }

    uid = atoi(strID);

    /* Check to see if the universe even exists */
    if (!CHSUniverseDB::GetInstance().FindUniverse(uid))
    {
        hsInterface.Notify(player,
                           hsInterface.
                           HSPrintf("No such universe with id %d.", uid));
        return;
    }

    if (CHSUniverseDB::GetInstance().DeleteUniverse(uid))
    {
        m_nUniverses--;
        hsInterface.Notify(player, "BOOM!  You have destroyed the universe!");
        hsInterface.UnsetToggle(uid, ROOM_HSPACE_UNIVERSE);
    }
    else
        hsInterface.Notify(player,
                           "Unable to delete that universe at this time.");
}

// Returns the CHS3DObject that controls the specified landing
// location. 
CHS3DObject *CHSDB::FindObjectByLandingLoc(int loc)
{
    int objnum;

    // If the location is a landing location, it should have a 
    // HSDB_OBJECT attribute.
    if (hsInterface.AtrGet(loc, "HSDB_OBJECT"))
    {
        CHS3DObject *pObject;

        objnum = strtodbref(hsInterface.m_buffer);
        pObject = FindObject(objnum);
        return pObject;
    }
    else
    {
        return NULL;            // Landing location of .. ?
    }
}

// Call this function to find a landing location based on object
// HS_DBREF.
CHSLandingLoc *CHSDB::FindLandingLoc(HS_DBREF dbLocation)
{
    // Find the object based on the landing location.
    CHS3DObject *pObject = FindObjectByLandingLoc(dbLocation);
    if (pObject)
    {
        // Find the CHSLandingLoc from the object.
        return pObject->FindLandingLoc(dbLocation);
    }

    return NULL;
}


CHSHatch *CHSDB::FindHatch(int obj)
{
    int objnum;

    if (hsInterface.AtrGet(obj, "HSDB_SHIP"))
    {
        CHSShip *cShip;

        objnum = strtodbref(hsInterface.m_buffer);
        cShip = (CHSShip *) FindObject(objnum);
        if (!cShip)
            return NULL;

        // Now pull the hatch exit from the ship.
        return (cShip->FindHatch(obj));
    }

    return NULL;                // Landing location of .. ?
}

// Call this function to find a console object based on the
// object HS_DBREF.
CHSConsole *CHSDB::FindConsole(int obj)
{
    HS_DBREF owner;
    CHS3DObject *cObj;

    // Make sure it's a console
    if (!hsInterface.AtrGet(obj, "HSDB_OWNER"))
        return NULL;

    // Grab the HS_DBREF of the owner
    owner = strtodbref(hsInterface.m_buffer);

    // Now we can use FindObject() to locate the object
    // that owns the console.
    cObj = FindObject(owner);
    if (!cObj)
        return NULL;

    return (cObj->FindConsole(obj));
}

// Call this function to find an HSpace object based on
// a given console.  Generally used to find ships based
// on a ship console.
CHS3DObject *CHSDB::FindObjectByConsole(HS_DBREF obj)
{
    HS_DBREF owner;
    CHS3DObject *cObj;

    // Make sure it's a console
    if (!hsInterface.AtrGet(obj, "HSDB_OWNER"))
        return NULL;

    // Grab the HS_DBREF of the owner
    owner = strtodbref(hsInterface.m_buffer);

    // Now we can use FindObject() to locate the object
    // that owns the console.
    cObj = FindObject(owner);

    return cObj;
}

#define HS_MAX_ROOM_SEARCH	256     // Max rooms to check for ownership

// Given a room #, this function will attempt to find the space
// object it belongs to.  Why?  This is useful for determining
// which planet a general room may belong to or which ship a room
// is on.
CHS3DObject *CHSDB::FindObjectByRoom(HS_DBREF room)
{
    int iRoomsChecked[HS_MAX_ROOM_SEARCH];

    iRoomsChecked[0] = HSNOTHING;
    return FindRoomOwner(room, iRoomsChecked);
}

CHS3DObject *CHSDB::FindRoomOwner(HS_DBREF room, int *iRoomsChecked)
{
    CHS3DObject *cOwner;

    // Did we check this room already?
    int idx;
    for (idx = 0; idx < HS_MAX_ROOM_SEARCH; idx++)
    {
        if (iRoomsChecked[idx] == room)
            return NULL;
    }

    // Check attrs to see if this room indicates who
    // it belongs to.
    HS_DBREF dbOwner;
    if (hsInterface.AtrGet(room, "HSDB_PLANET"))
    {
        dbOwner = strtodbref(hsInterface.m_buffer);
        return FindObject(dbOwner);
    }

    if (hsInterface.AtrGet(room, "HSDB_SHIP"))
    {
        dbOwner = strtodbref(hsInterface.m_buffer);
        return FindObject(dbOwner);
    }

    // We're gonna have to search for another room that
    // As the given attribute.
    for (idx = 0; idx < HS_MAX_ROOM_SEARCH; idx++)
    {
        if (iRoomsChecked[idx] == HSNOTHING)
        {
            iRoomsChecked[idx] = room;
            break;
        }
    }
    // Did we reach our limit?
    idx++;
    if (idx == HS_MAX_ROOM_SEARCH)
        return NULL;

    // Ok to look for more rooms, so check the exits
    // and check their destination rooms.
    HS_DBREF exit_m;
    HS_DBREF dbDest;
    for (exit_m = hsInterface.GetFirstExit(room); exit_m &&
         exit_m != HSNOTHING; exit_m = hsInterface.GetNextExit(exit_m))
    {
        // Get the destination of the exit.
        dbDest = hsInterface.GetLocation(exit_m);
        if (dbDest == HSNOTHING)
            continue;

        // Call ourself with the new room.
        cOwner = FindRoomOwner(dbDest, iRoomsChecked);
        if (cOwner)
            return cOwner;
    }

    // Nothing found
    return NULL;
}

HS_BOOL8 CHSDB::BackupFile(HS_INT8 * origfile, HS_INT8 * newfile)
{
    int result = 0;
#ifdef WIN32                    // Windows cannot rename over an existing file
    _unlink(newfile);
#endif

    result = rename(origfile, newfile);

    if (result >= 0)
    {
        return true;
    }

    return false;
}

void CHSDB::BackupDatabases(HS_INT8 * suffix)
{
    char tmpfname[180];

    // Class Database
    strncpy_s(tmpfname, HSCONF.classdb, 179);
    if (BackupFile(HSCONF.classdb, strncat(tmpfname, suffix, 179)) == false)
    {
        hs_log("WARNING: Failed to backup Class DB file.");
    }

    // Weapon DB
    strncpy_s(tmpfname, HSCONF.weapondb, 179);
    if (BackupFile(HSCONF.weapondb, strncat(tmpfname, suffix, 179)) == false)
    {
        hs_log("WARNING: Failed to backup Weapon DB file.");
    }

    // Universe DB
    strncpy_s(tmpfname, HSCONF.univdb, 179);
    if (BackupFile(HSCONF.univdb, strncat(tmpfname, suffix, 179)) == false)
    {
        hs_log("WARNING: Failed to backup Universe DB file.");
    }

    // Territory DB
    strncpy_s(tmpfname, HSCONF.territorydb, 179);
    if (BackupFile(HSCONF.territorydb, strncat(tmpfname, suffix, 179))
        == false)
    {
        hs_log("WARNING: Failed to backup Territory DB file.");
    }

    // Object DB
    strncpy_s(tmpfname, HSCONF.objectdb, 179);
    if (BackupFile(HSCONF.objectdb, strncat(tmpfname, suffix, 179)) == false)
    {
        hs_log("WARNING: Failed to backup Object DB file.");
    }

}
