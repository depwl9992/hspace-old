// -----------------------------------------------------------------------
// $Id: hsdb.h,v 1.5 2006/04/04 12:56:10 mark Exp $
// -----------------------------------------------------------------------

#ifndef __HSDB_INCLUDED__
#define __HSDB_INCLUDED__

class CHS3DObject;
class CHSLandingLoc;
class CHSShip;
class CHSCelestial;
class CHSConsole;
class CHSHatch;

//! Database storage and handlers
class CHSDB
{
  public:
    //! Constructor -- init variables as needed
    CHSDB();

    //! Deconstructor does nada
    virtual ~CHSDB()
    {
    }

    //! Call load routines for all the various databases
    HS_BOOL8 LoadDatabases();

    //! Call database save routines for all databases
    void DumpDatabases();

    //! Return weapon count in the database
    HS_UINT32 NumWeapons()
    {
        return m_nWeapons;
    }

    //! Return the number of universes in the database
    HS_UINT32 NumUniverses()
    {
        return m_nUniverses;
    }

    //! Create a new universe based on room #
    void CreateNewUniverse(HS_DBREF player, HS_INT8 * strRoom);

    //! Delete a universe based on a specific ID
    void DeleteUniverse(HS_DBREF player, HS_INT8 * strID);

    //! Iterate through all universes, calling ClearObjectAttrs on everything
    void CleanupDBAttrs();

    //! Find a space object from a console dbref
    CHS3DObject *FindObjectByConsole(HS_DBREF obj);

    //! Look through all universes and find the specified object
    CHS3DObject *FindObject(int objnum);

    //! Return the space object that controls the landing location
    CHS3DObject *FindObjectByLandingLoc(int objnum);

    //! Search database for the owner object of a specified room
    CHS3DObject *FindObjectByRoom(HS_DBREF room);

    //! Find a landing location by dbref
    CHSLandingLoc *FindLandingLoc(HS_DBREF dbLocation);

    //! Find a celestrial object by id
    CHSCelestial *FindCelestial(int objnum);

    //! Traverse all universes, looking for the ship 
    CHSShip *FindShip(int objnum);

    //! Traverse all universes, looking for the console
    CHSConsole *FindConsole(int objnum);

    //! Find hatch data by dbref
    CHSHatch *FindHatch(int obj);

  protected:
    //! Load universe database from the specified file
    HS_BOOL8 LoadUniverseDB(HS_INT8 * lpstrDBPath);

    //! Load the class database from the specified file
    HS_BOOL8 LoadClassDB(HS_INT8 * lpstrDBPath);

    //! Load the weapon database from the specified file
    HS_BOOL8 LoadWeaponDB(HS_INT8 * lpstrDBPath);

    //! Load the territory database from the specified file
    HS_BOOL8 LoadTerritoryDB(HS_INT8 * lpstrDBPath);

    //! Load the object database 
    HS_BOOL8 LoadObjectDB();

    //! Write the universe database to the specified file
    void DumpUniverseDB(HS_INT8 * lpstrDBPath);

    //! Write the class database to the specified file
    void DumpClassDB(HS_INT8 * lpstrDBPath);

    //! Write the weapon database to the specified file
    void DumpWeaponDB(HS_INT8 * lpstrDBPath);

    //! Write the territory database to the specified file
    void DumpTerritoryDB(HS_INT8 * lpstrDBPath);

    //! Write the object database
    void DumpObjectDB();

    //! Rename the origfile to newfile
    HS_BOOL8 BackupFile(HS_INT8 * origfile, HS_INT8 * newfile);

    //! Backup all databases to <original>.suffix
    void BackupDatabases(HS_INT8 * suffix);

    //! Recursive lookup for the space object owner of room
    CHS3DObject *FindRoomOwner(HS_DBREF room, int *iRoomsChecked);

    //! # of Weapons in the Database
    HS_UINT32 m_nWeapons;

    //! # of Universes in the Database
    HS_UINT32 m_nUniverses;

};

extern CHSDB dbHSDB;

#endif // __HSDB_INCLUDED__
