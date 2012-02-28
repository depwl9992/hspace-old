// -----------------------------------------------------------------------
//! $Id: hsweapon.h,v 1.14 2006/06/17 02:36:26 mark Exp $
// -----------------------------------------------------------------------

#ifndef __HSWEAPON_INCLUDED__
#define __HSWEAPON_INCLUDED__

#include "hsobject.h"

#include <cstdio>
#include <map>
#include <string>
#include <list>


class CHSFileDatabase;
//
// To implement a new type of weapon:
//
// Derive a weapon from the generic CHSDBWeapon.  These are stored
// in the global CHSWeaponDBArray and contain the information loaded
// from the weapon database.
//
// Modify the CHSWeaponDB to implement a Create.. function
// to handle the creation of your weapon.  Use CreateLaser() or
// CreateMissile() as examples.
//
// Derive a console weapon class from CHSWeapon, which is the
// weapon object that gets added to a console.  This is different
// from CHSDBWeapon because it has a more diverse interface, whereas
// CHSDBWeapon simply contains database information for the weapon.
// Administrators can add multiple CHSWeapon's to consoles.
//
// Overload the necessary functions to make the weapon work as
// you want.

//! Weapon class types defined as an enumeration
enum EHSWeaponClass
{
    WC_LASER = 0,
    WC_MISSILE
};

//! Weapon Status changes as enumeration
enum WEAPSTAT_CHANGE
{
    STAT_NOCHANGE = 0,
    STAT_READY
};

class CHSConsole;

//! The base weapon data structure, which is ultimately associated
//! with an instance of a weapon.
class CHSWeaponData:public CHSObject
{
  public:
    //! Default constructor
    CHSWeaponData()
    {
    }
    //! Default deconstructor
    virtual ~CHSWeaponData()
    {
    }

    //! Return minimum sized of target objects
    HS_UINT32 GetMinimumTargetSize()
    {
        return m_mintargetsize;
    }

    //! Return of the getabble attribute values
    virtual const HS_INT8 *GetAttributeValue(const HS_INT8 * pcAttributeName);

    //! Set a specified attribute to a value
    virtual HS_BOOL8 SetAttributeValue(const HS_INT8 * pcAttributeName,
                                       const HS_INT8 * pcValue);

    //! Return the list of attributes that can be changed for this object
    virtual void GetAttributeList(CHSAttributeList & rlistAttributes);

    //! Create a default weapon of the specified class
    static CHSWeaponData *CreateFromClass(EHSWeaponClass eClass);

    //! Write weapon data to the specified database
    virtual HS_BOOL8 WriteToDatabase(CHSFileDatabase * pDatabase);
    //! Read weapon information from the specified database
    virtual HS_BOOL8 ReadFromDatabase(CHSFileDatabase * pDatabase);

    // This is for an old-style load.
    virtual HS_BOOL8 LoadFromBuffer(const HS_INT8 * pcAttributeBuffer) = 0;

    //! Return the weapon class name
    virtual const HS_INT8 *GetWeaponClassName() const
    {
        return "Unknown";
    }

    //! Get the Weapon Name
    const HS_INT8* Name()
    {
        return m_strName.c_str();
    }
    //! Set the weapon Name
    void Name(const char *val)
    {
        m_strName = val;
    }

    //! Get the Weapon Type ID
    HS_UINT32 TypeID()
    {
        return m_uiWeaponTypeID;
    }
    //! Set the Weapon Type ID
    void TypeID(HS_UINT32 val)
    {
        m_uiWeaponTypeID = val;
    }

    //! Get the Weapon Class
    EHSWeaponClass WeaponClass()
    {
        return m_eWeaponClass;
    }
    //! Set the Weapon Class
    void WeaponClass(EHSWeaponClass val)
    {
        m_eWeaponClass = val;
    }

    double ShieldPercentage(void)
    {
        return m_shield_percentage;
    }
    void ShieldPercentage(double val)
    {
        m_shield_percentage = val;
    }

    double HullPercentage(void)
    {
        return m_hull_percentage;
    }
    void HullPercentage(double val)
    {
        m_hull_percentage = val;
    }

  protected:

    double m_shield_percentage;
    double m_hull_percentage;

    //! Class for the Weapon
    EHSWeaponClass m_eWeaponClass;
    //! Type Identifier for the Weapon
    HS_UINT32 m_uiWeaponTypeID;
    //! Name of this weapon
    std::string m_strName;
    //! Minimum size of target that can be attacked by this weapon
    HS_UINT32 m_mintargetsize;
};

// Laser attributes.
class CHSLaserData:public CHSWeaponData
{
  public:
    //! Default constructor
    CHSLaserData();
    //! Default deconstructor
    virtual ~CHSLaserData()
    {
    }

    //! Get value of the specified attribute
    const HS_INT8 *GetAttributeValue(const HS_INT8 * pcAttributeName);

    //! Set the specified attribute value
    HS_BOOL8 SetAttributeValue(const HS_INT8 * pcAttributeName,
                               const HS_INT8 * pcValue);

    //! Return list of modifiable attributes
    void GetAttributeList(CHSAttributeList & rlistAttributes);

    // This is for an old-style load.
    HS_BOOL8 LoadFromBuffer(const HS_INT8 * pcAttributeBuffer);

    //! Write laser data to the specified database
    HS_BOOL8 WriteToDatabase(CHSFileDatabase * pDatabase);

    //! Load laser data from the specified database
    HS_BOOL8 ReadFromDatabase(CHSFileDatabase * pDatabase);

    //! Return the weapon class name
    const HS_INT8 *GetWeaponClassName() const
    {
        return "Laser";
    }

    //! Get the strength rating of the laser
    HS_UINT32 Strength()
    {
        return m_strength;
    }
    //! Set the strength rating of the laser
    void Strength(HS_UINT32 val)
    {
        m_strength = val;
    }

    //! Get the maximum range of the missile
    HS_UINT32 Range()
    {
        return m_range;
    }
    //! Set the max range of the missile to val
    void Range(HS_UINT32 val)
    {
        m_range = val;
    }

    //! Get the time, in number of cycles, to recharge
    HS_UINT32 RegenTime()
    {
        return m_regen_time;
    }
    //! Set the time, in number of cycles, to recharge
    void RegenTime(HS_UINT32 val)
    {
        m_regen_time = val;
    }

    //! Get the accuracy rating for the laser 
    HS_UINT32 Accuracy()
    {
        return m_accuracy;
    }
    //! Set accuracy to val
    void Accuracy(HS_UINT32 val)
    {
        m_accuracy = val;
    }

    //! Get the Power Usage  for the laser 
    HS_UINT32 PowerUsage()
    {
        return m_powerusage;
    }
    //! Set PowerUsage to val
    void PowerUsage(HS_UINT32 val)
    {
        m_powerusage = val;
    }

    //! Does the weapon cause hull damage
    HS_BOOL8 NoHull()
    {
        return m_nohull;
    }

  protected:
    //! Time to recharge the laser, in # of cycles
    HS_UINT32 m_regen_time;
    //! Maximum range of this weapon
    HS_UINT32 m_range;
    //! Strength value of the weapon
    HS_UINT32 m_strength;
    //! Accuracy rating of the laser
    HS_UINT32 m_accuracy;
    //! Power required to operate the laser
    HS_UINT32 m_powerusage;
    //! Does this weapon cause hull damage?
    HS_BOOL8 m_nohull;

};

//! Missile attributes storage class
class CHSMissileData:public CHSWeaponData
{
  public:
    //! Default Constructor
    CHSMissileData();

    //! Default Deconstructor
    virtual ~CHSMissileData()
    {
    }

    //! Return the value of the specified attribute
    const HS_INT8 *GetAttributeValue(const HS_INT8 * pcAttributeName);
    //! Set the specified attribute to pcValue
    HS_BOOL8 SetAttributeValue(const HS_INT8 * pcAttributeName,
                               const HS_INT8 * pcValue);
    //! Get the list of attributes that can be manipulated
    void GetAttributeList(CHSAttributeList & rlistAttributes);

    // This is for an old-style load.
    HS_BOOL8 LoadFromBuffer(const HS_INT8 * pcAttributeBuffer);

    //! Write MissileData to the database
    HS_BOOL8 WriteToDatabase(CHSFileDatabase * pDatabase);
    //! Read Missle data from the specified database
    HS_BOOL8 ReadFromDatabase(CHSFileDatabase * pDatabase);

    //! Return the classname 
    const HS_INT8 *GetWeaponClassName() const
    {
        return "Missile";
    }

    //! Get the max speed of the missile
    HS_UINT32 Speed()
    {
        return m_speed;
    }
    //! Set the max speed of the missile to val
    void Speed(HS_UINT32 val)
    {
        m_speed = val;
    }

    //! Get the turn rate of the missile
    HS_UINT32 Turnrate()
    {
        return m_turnrate;
    }
    //! Set the turn rate of the missile to val
    void Turnrate(HS_UINT32 val)
    {
        m_turnrate = val;
    }

    //! Get the strength rating of the missile
    HS_UINT32 Strength()
    {
        return m_strength;
    }
    //! Set the strength rating of the missile
    void Strength(HS_UINT32 val)
    {
        m_strength = val;
    }

    //! Get the maximum range of the missile
    HS_UINT32 Range()
    {
        return m_range;
    }
    //! Set the max range of the missile to val
    void Range(HS_UINT32 val)
    {
        m_range = val;
    }

    //! Get the time, in number of cycles, to reload the missile
    HS_UINT32 ReloadTime()
    {
        return m_reload_time;
    }
    //! Set the time, in number of cycles, to reload missile to val
    void ReloadTime(HS_UINT32 val)
    {
        m_reload_time = val;
    }

  protected:
    //! Maximum speed of the missile
    HS_UINT32 m_speed;
    //! Turning rate of the missile per cycle
    HS_UINT32 m_turnrate;
    //! Strength rating of the weapon
    HS_UINT32 m_strength;
    //! Number of cycles required to load a missile 
    HS_UINT32 m_reload_time;
    //! Maximum range of the missile
    HS_UINT32 m_range;


};

//! The CHSWeaponDB is the repository for all weapon data in the game.
class CHSWeaponDB
{
  public:
    //! Default constructor
    CHSWeaponDB();
    //! Default deconstructor
    ~CHSWeaponDB();

    //! Save weapon to the specified file 
    void SaveToFile(HS_INT8 * pcFilePath);

    //! Load weapon data from the specified file
    HS_BOOL8 LoadFromFile(HS_INT8 * pcFilePath);

    //! Display current weapon information to the specified player
    //! @param player: dbref of player
    void PrintInfo(HS_INT32 player);

    //! Return the number of weapons in the database
    HS_UINT32 NumWeapons()
    {
        return m_mapWeaponData.size();
    }

    //! Retrieve weapon information for the specified weapon id
    //! @return NULL on error or CHSWeaponData pointer 
    CHSWeaponData *GetWeapon(HS_UINT32 uiWeaponID);

    //! @brief Add specified weapon to the database
    //! @param pData : weapon data to be added
    //! @param bAssignID : does weapon need to have an ID assigned?
    //! @return true on success, false on failure
    HS_BOOL8 AddWeapon(CHSWeaponData * pData, HS_BOOL8 bAssignID = true);

    //! Get weapon information for the first weapon in the list
    //! @return NULL if no weapons present or pointer to CHSWeaponData 
    CHSWeaponData *GetFirstWeapon()
    {
        m_iterCurWeapon = m_mapWeaponData.begin();
        return GetNextWeapon();
    }

    //! Return weapon data for the next weapon after the current iter pos
    //! @return NULL on errror, CHSWeaponData pointer on success
    CHSWeaponData *GetNextWeapon()
    {
        if (m_iterCurWeapon == m_mapWeaponData.end())
        {
            return NULL;
        }

        CHSWeaponData *pData = m_iterCurWeapon->second;
        m_iterCurWeapon++;
        return pData;
    }

  protected:
    typedef std::map < HS_UINT32, CHSWeaponData * >CSTLWeaponDataMap;

    //! Pointer to current weapon being processed
    CSTLWeaponDataMap::iterator m_iterCurWeapon;
    //! Map of weapon id to weapon data for db storage
    CSTLWeaponDataMap m_mapWeaponData;
    //! Largest weapon id in the current system for internal id tracking
    HS_UINT32 m_uiMaxWeaponID;
    //! Array of bits representing used class id's
    HS_UINT8 *m_pucWeaponIDs;

    //! Call this to flag a given weapon ID as available for use.
    //! @param uiWeaponID: id to unlock
    void UnlockWeaponID(HS_UINT32 uiWeaponID);

    //! Call this to flag a given weapon ID as taken.
    //! @param uiWeaponID: id to lock
    void LockWeaponID(HS_UINT32 uiWeaponID);

    //! Finds the next available weapon ID and marks it as used.
    HS_UINT32 GetNextWeaponID();
};

class CHS3DObject;

//! The CHSWeapon is the base class for all weapons that can be fired 
class CHSWeapon
{
  public:
    //! Default constructor
    CHSWeapon();
    //! Default deconstructor
    virtual ~CHSWeapon();

    //! Create a weapon of the specified class
    static CHSWeapon *CreateWeapon(EHSWeaponClass eClass);

    // Overridable functions.
    //! Power usage requirements for the weapon
    virtual HS_UINT32 GetPowerUsage(void);
    //! Maximum range of the weapon
    virtual HS_UINT32 GetRange(void);
    //! name of the weapon
    virtual const HS_INT8 *GetName(void);
    //! Return attribute information in a formatted string 
    virtual const char *GetAttrInfo(void);
    //! Return status value as a string
    virtual const char *GetStatus(void);
    //! Set underlying weapon data to passed pointer
    virtual void SetData(CHSWeaponData * pData);
    //! Return a pointer to the weapon data for this weapon
    CHSWeaponData *GetData()
    {
        return m_pWeaponData;
    }
    //! Get value of the specified attribute
    virtual const HS_INT8 *GetAttributeValue(const HS_INT8 * pcAttributeName);

    //! Set the specified attribute value
    virtual HS_BOOL8 SetAttributeValue(const HS_INT8 * pcAttributeName,
                                       const HS_INT8 * pcValue);

    //! Return list of modifiable attributes
    virtual void GetAttributeList(CHSAttributeList & rlistAttributes);

    //! Write laser data to the specified database
    virtual HS_BOOL8 WriteToDatabase(CHSFileDatabase * pDatabase);

    //! Load laser data from the specified database
    virtual HS_BOOL8 ReadFromDatabase(CHSFileDatabase * pDatabase);

    //! Process a single cyle for the weapon
    virtual void DoCycle(void);
    //! Set the weapon status
    virtual void SetStatus(HS_INT32);
    //! Invoke an attack by this weapon 
    virtual void AttackObject(CHS3DObject *, CHS3DObject *, CHSConsole *,
                              HS_INT32, HS_INT32);

    //! Does this weapon require a target lock before firing?
    virtual HS_BOOL8 RequiresLock(void);

    //! Can this weapon be configured?
    virtual HS_BOOL8 Configurable(void);
    //! Configure weapon
    virtual HS_BOOL8 Configure(HS_INT32);
    //! Is this weapon loadable?
    virtual HS_BOOL8 Loadable(void);
    //! Reload the weapon 
    virtual HS_BOOL8 Reload(void);
    //! Unload the weapon
    virtual HS_BOOL8 Unload(void);
    //! Can this weapon attack the specified object?
    virtual HS_BOOL8 CanAttackObject(CHS3DObject *);
    //! Is the target size greater than the minimum target size?
    virtual HS_BOOL8 CheckTargetSize(CHS3DObject *);
    //! Is the weapon ready to be fired?
    virtual HS_BOOL8 IsReady(void);

    //! Is the weapon reloading?
    virtual HS_INT32 Reloading(void);
    //! Return weapon status as an integer
    virtual HS_INT32 GetStatusInt(void);
    //! Check for a weapon status change
    virtual HS_INT32 GetStatusChange(void);

    //! Get the weapon class 
    EHSWeaponClass GetWeaponClass(void);

    double ShieldPercentage(void)
    {
        return m_shield_percentage;
    }
    void ShieldPercentage(double val)
    {
        m_shield_percentage = val;
    }

    double HullPercentage(void)
    {
        return m_hull_percentage;
    }
    void HullPercentage(double val)
    {
        m_hull_percentage = val;
    }

    HS_UINT32 MinTargetSize(void)
    {
        return m_mintargetsize;
    }
    void MinTargetSize(HS_UINT32 val)
    {
        m_mintargetsize = val;
    }

    const HS_INT8* Name(void)
    {
        return m_strName.c_str();
    }
    void Name(const char *newName)
    {
        m_strName = newName;
    }

  protected:
    //! Weapon data pointer to weapon params
    CHSWeaponData * m_pWeaponData;
    //! State monitoring variable
    WEAPSTAT_CHANGE m_change;

    std::string m_strName;
    HS_UINT32 m_mintargetsize;
    double m_shield_percentage;
    double m_hull_percentage;
};

class CHSLaser:public CHSWeapon
{
  public:
    //! Default constructor
    CHSLaser();

    //! Default deconstructor
    virtual ~CHSLaser()
    {
    }

    //! Get required time to regenerate a charge before able to fire
    HS_UINT32 GetRegenTime();

    //! Return the maximum range of the laser
    HS_UINT32 GetRange();

    //! Return the strength rating of the laser
    HS_UINT32 GetStrength();

    //! Return the accuracy rating of the laser
    HS_UINT32 GetAccuracy();

    //! Return the power required to operate the laser
    HS_UINT32 GetPowerUsage();

    //! Can this weapon damage hulls?
    HS_BOOL8 NoHull();

    //! Checks if the specified object is larger than the minimum target size
    //! @return false if target is smaller than min target size, else true
    HS_BOOL8 CheckTargetSize(CHS3DObject * cObj);

    //! Checks if the laser can attack the specified object
    //! Return true if it can; false if not
    HS_BOOL8 CanAttackObject(CHS3DObject * cObj);

    //! Is the laser ready to fire?
    //! @return true if not regenerating otherwise false
    HS_BOOL8 IsReady();

    //! Must a target lock be present to fire the laser?
    HS_BOOL8 RequiresLock();

    //! @brief Cause this weapon to attack a target
    //! @param cSource : attacking vessel
    //! @param cTarget : target vessel
    //! @param cConsole : attacking console
    //! @param iSysType : targetted system
    //! @param hit_flag :  specify if the system should auto hit or miss
    void AttackObject(CHS3DObject * cSource, CHS3DObject * cTarget,
                      CHSConsole * cConsole, HS_INT32 iSysType,
                      HS_INT32 hit_flag);

    //! Process regeneration / recharging of the laster
    void Regenerate();

    //! Process a single cycle for the laser
    void DoCycle();

    //! Set the current status of the laser
    void SetStatus(HS_INT32);

    //! Get the current status of the laser
    const char *GetStatus();

    //! Return attribute information as a string
    const char *GetAttrInfo();

    //! @brief Return status as an integer
    //! @return 0 if not recharging or number of cycles to finish charging
    HS_INT32 GetStatusInt();

    //! Get value of the specified attribute
    const HS_INT8 *GetAttributeValue(const HS_INT8 * pcAttributeName);

    //! Set the specified attribute value
    HS_BOOL8 SetAttributeValue(const HS_INT8 * pcAttributeName,
                               const HS_INT8 * pcValue);

    //! Return list of modifiable attributes
    void GetAttributeList(CHSAttributeList & rlistAttributes);

    //! Write laser data to the specified database
    HS_BOOL8 WriteToDatabase(CHSFileDatabase * pDatabase);

    //! Load laser data from the specified database
    HS_BOOL8 ReadFromDatabase(CHSFileDatabase * pDatabase);

    void SetData(CHSLaserData * pData);

  protected:

    // Overload all the variables of CHSLaserData
    HS_UINT32 m_regen_time;
    HS_UINT32 m_range;
    HS_UINT32 m_strength;
    HS_UINT32 m_accuracy;
    HS_UINT32 m_power;
    HS_BOOL8 m_nohull;

    //! Number of cycles remaining to finish recharging
    HS_UINT32 m_time_to_regen;
    //! Boolean flag to indicate weapon is recharging
    HS_BOOL8 m_regenerating;
};

// Can be used on an object to store/retrieve missles.
class CHSMissileBay
{
  public:
    //! A missile storage slot in the missile bay
    struct THSMissileStorage
    {
        HS_UINT32 uiMaximum;
        HS_UINT32 uiWeaponTypeID;
        HS_UINT32 uiNumRemaining;

            THSMissileStorage():uiMaximum(0),
            uiWeaponTypeID(0), uiNumRemaining(0)
        {
        }
    };

    //! Default constructor
    CHSMissileBay();
    //! Default deconstructor
    ~CHSMissileBay();

    //! Pull a new missile from storage
    HS_BOOL8 GetMissile(HS_UINT32 uiWeaponTypeID, HS_INT32 amt = 1);
    //! Set the number of missiles remaining
    HS_BOOL8 SetRemaining(HS_UINT32 uiWeaponTypeID, HS_UINT32 uiNumRemaining);
    //! Remove the specified missile type from the bay
    HS_BOOL8 RemoveMissileType(HS_UINT32 uiWeaponTypeID);
    //! Get the max num of specified type that can be stored in the bay
    HS_UINT32 GetMaxMissiles(HS_UINT32 uiWeaponTypeID);
    //! Return the number of missiles of the indicated type in the bay
    HS_UINT32 GetMissilesLeft(HS_UINT32 uiWeaponTypeID);
    //! Does this bay contain missiles of the specified type?
    //! @return true if found, false if not
    HS_BOOL8 HasType(HS_UINT32 uiWeaponTypeID);

    //! Return the type of the first missile in the bay
    HS_INT32 GetFirstType()
    {
        m_itrCurMissile = m_mapMissiles.begin();
        return GetNextType();
    }

    //! Return the type of the missile after the current iterator position
    HS_INT32 GetNextType()
    {
        if (m_itrCurMissile == m_mapMissiles.end())
        {
            return -1;
        }

        HS_INT32 iType = m_itrCurMissile->first;
        m_itrCurMissile++;
        return iType;
    }

    //! Save the missile bay information to the specified file
    void SaveToFile(FILE *);
    //! Load a missile bay from the given file 
    void LoadFromFile(FILE *);

    typedef std::map < HS_UINT32, THSMissileStorage > CSTLMissileStorageMap;


  protected:
    //! Missile storage mapped by weapon type
    CSTLMissileStorageMap m_mapMissiles;
    //! Iterator pointing to the current missile
    CSTLMissileStorageMap::iterator m_itrCurMissile;
};

// A weapons array that can exist on any console
class CHSWeaponArray
{
  public:
    //! Default constructor
    CHSWeaponArray(void);
    //! Default deconstructor
        virtual ~CHSWeaponArray()
    {
    }

    //! Add the specified weapon to the array
    //! @return true on succcess, false on failure
    HS_BOOL8 AddWeapon(CHSWeapon * pWeapon);

    //! Delete the weapon at the specified slot
    //! @return true on succes, false on failure
    HS_BOOL8 DeleteWeapon(HS_UINT32);

    //! Remove all current weapons from the array
    void ClearArray(void);

    //! Set a pointer to the missile bay used by the weapon array
    void SetMissileBay(CHSMissileBay *);

    //! Process one cycle for the weapon array
    void DoCycle(void);

    //! Return the total power needed to run all weapons in the array
    HS_UINT32 GetTotalPower(void);

    //! Return the maximum range of the weapon
    HS_UINT32 GetMaxRange(void);

    //! Get a pointer to the weapon in the specified slot.
    //! @return NULL if invalid or CHSWeapon pointer
    CHSWeapon *GetWeapon(HS_UINT32);    // Returns the weapon in a given slot

    //! Get the first weapon in the weapon array
    //! @return NULL if weapon array is empty or CHSWeapon pointer
    CHSWeapon *GetFirstWeapon()
    {
        m_iterCurWeapon = m_listWeapons.begin();
        return GetNextWeapon();
    }

    //! Get a pointer to the next weapon in the array
    CHSWeapon *GetNextWeapon()
    {
        if (m_iterCurWeapon == m_listWeapons.end())
        {
            return NULL;
        }
        CHSWeapon *pWeapon = *m_iterCurWeapon;
        m_iterCurWeapon++;
        return pWeapon;
    }


    typedef std::list < CHSWeapon * >CSTLWeaponList;

  protected:
    //! Create a new laser in the array
    void NewLaser(HS_INT32);
    //! Create a new missile in the array
    void NewMTube(HS_INT32);

    //! Pointer to the current weapon in the list
    std::list < CHSWeapon * >::iterator m_iterCurWeapon;
    //! The weapons array as a list
    CSTLWeaponList m_listWeapons;
    //! Pointer to the associated missile bay, if appropriate
    CHSMissileBay *m_missile_bay;
};


extern CHSWeaponDB waWeapons;
#endif // __HSWEAPON_INCLUDED__
