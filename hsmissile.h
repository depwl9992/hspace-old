// -----------------------------------------------------------------------
//! $Id: hsmissile.h,v 1.17 2006/04/22 22:13:49 mark Exp $
// -----------------------------------------------------------------------

#ifndef __HSMISSILE_INCLUDED__
#define __HSMISSILE_INCLUDED__

#include "hsobjects.h"

class CHSMTube;

//! A missile that gets fired from a missile tube
class CHSMissile:public CHS3DObject
{
  public:
    //! Default constructor
    CHSMissile();
    //! Default deconstructor
    virtual ~CHSMissile();

    //! Process one cycle
    void DoCycle();

    //! @brief Set the space object the missile is targetting
    //! @param  cTarget: Object being targeted by the missile
    void SetTargetObject(CHS3DObject * cTarget);

    //! @brief Set the space object that fired the missile
    //! @param cSource: CHS3DObject that fired the missile
    void SetSourceObject(CHS3DObject * cSource);

    //! @brief Set the console object which launched the missile
    //! @param cConsole: console data 
    void SetSourceConsole(CHSConsole * cConsole);

    //! @brief Handle incoming damage -- explode the missile
    //! @param cSource: source object of the damage (unused)
    //! @param cWeapon: weapon causing the damage  (unused) 
    //! @param strength:  amount of damage (unused)
    //! @param cConsole: firing console 
    //! @param iSysType: targetted system (unused)
    void HandleDamage(CHS3DObject * cSource, CHSWeaponData * cWeapon,
                      HS_INT32 strength, CHSConsole * cConsole,
                      HS_INT32 iSysType);

    //! @brief Handle incoming damage at a coordinate position
    //! @param x:  x coordinate location (unused)
    //! @param y:  y coordinate location (unused)
    //! @param z:  z coordinate location (unused)
    //! @param strength: strength points of damage (unused)
    void HandleDamage(HS_FLOAT64 x, HS_FLOAT64 y, HS_FLOAT64 z,
                      HS_INT32 strength);

    //! @brief Return object color
    //! @return ansi packed char string representing the object color
    char *GetObjectColor();

    //! @brief Return object type character
    //! @return char identifier for this object type
    char GetObjectCharacter();

    //! @brief Set current heading of the missle
    //! @param xy:  XY Heading of the Missile
    //! @param z: Z Heading of the Missile
    void SetHeading(HS_INT32 xy, HS_INT32 z);

    //! @brief Get the current XY heading of the missile
    //! @return XY heading from 0-359 degrees
    HS_UINT32 GetXYHeading() const
    {
        return m_xyheading;
    }

    //! @brief Set weapon data pointer
    //! @param pData: Weapon data pointer
    void SetWeaponData(CHSMTube * pData);

    //! Get the current z heading for the missile
    HS_INT32 GetZHeading() const
    {
        return m_zheading;
    }

    //! Get the current speed of the missile
    HS_INT32 GetSpeed();

    //! Force the missile to miss automatically.
    void SetAutoMiss()
    {
        m_specified_miss = true;
    };

    //! Get the list of manipulatable attributes
    void GetAttributeList(CHSAttributeList & rlistAttributes);

    //! Get the current value of the specified attribute
    HS_INT8 *GetAttributeValue(HS_INT8 * strName);

    //! Set an attribute value.
    HS_BOOL8 SetAttributeValue(HS_INT8 * strName, HS_INT8 * strValue);

    //! Get the dbref of the source (firing vessel) object
    HS_INT32 GetSourceDbref();

    //! Get the dbref of the object being fired upon
    HS_INT32 GetTargetDbref();

  protected:
    //! Calculate how many additional cycles will be processed
    void CalculateTimeLeft();

    //! Process a hit on the target
    void HitTarget();

    //! Change current heading of the missile to intersect with the target
    void ChangeHeading();

    //! Move the missile toward the target along its current heading
    void MoveTowardTarget();

    //! The space object that fired the missile
    CHS3DObject *m_source_object;
    //! The space object being targetted
    CHS3DObject *m_target;
    //! Console data for the firing console
    CHSConsole *m_source_console;

    //! Weapon data for this missile
    CHSMTube *m_pTube;
    CHSMissileData *m_pData;

    // Overloaded from the Weapon Data
    HS_UINT32 m_speed;
    HS_UINT32 m_turnrate;
    HS_UINT32 m_range;
    HS_UINT32 m_strength;

    //! Does this missile need removed?
    HS_BOOL8 m_delete_me;
    //! Was the target hit during the cycle?
    HS_BOOL8 m_target_hit;
    //! Should this missile automatically miss even if it would hit?
    HS_BOOL8 m_specified_miss;
    //! How many cycles remain before the missile expires
    HS_INT32 m_timeleft;
    //! Current xy heading
    HS_INT32 m_xyheading;
    //! Current z heading
    HS_INT32 m_zheading;
};


//! CHSMTube is a strange name for a weapons class, but that's what it
//! really is .. a missile tube, not a missile.  Missiles are stored
//! in the ship's ammunition storage, and each missile tube just loads
//! and fires them.
class CHSMTube:public CHSWeapon
{
  public:
    //! Default Constructor
    CHSMTube();
    //! Default deconstructor
    virtual ~CHSMTube()
    {
    }

    //! @brief How many cycles are required to reload the tube?
    //! @return Number of cycles remaining to finish reloading
    HS_UINT32 GetReloadTime();

    //! @brief Get the maximum range
    //! @return Max range of the missile
    HS_UINT32 GetRange();

    //! Get current strength rating
    HS_UINT32 GetStrength();

    //! Get current turning rate / cycle of the missile
    HS_UINT32 GetTurnRate();

    //! Get maximum speed
    HS_UINT32 GetSpeed();

    //! @brief Is the tube being reloaded
    //! @return 0 if already loaded or # cycles until reloading is complete
    HS_INT32 Reloading();

    //! @brief Return status as an integer
    //! @return 0 if loaded, -1 if empty and not reloading, or time to reload
    HS_INT32 GetStatusInt();

    //! What is the name of this weapon
    const HS_INT8 *GetName();

    //! Get status string
    const HS_INT8 *GetStatus();

    //! Get weapon parameters as a string
    const HS_INT8 *GetAttrInfo();

    //! Set missile bay point that this tube belongs to
    void SetMissileBay(CHSMissileBay *);

    //! Process a single cycle
    void DoCycle();

    //! Set status value
    void SetStatus(HS_INT32);

    //! @brief Perform an attack using this weapon the specified objects
    //! @param cSource -- attacking space object
    //! @param cTarget -- targetted space object
    //! @param cConsole -- console that fired the weapon
    //! @param iSysType -- targetted system type 
    //! @param hit_flag -- user specified autohit or automiss
    void AttackObject(CHS3DObject * cSource, CHS3DObject * cTarget,
                      CHSConsole * cConsole, HS_INT32 iSysType,
                      HS_INT32 hit_flag);

    //! Does this weapon require a targetting lock to be fired?
    HS_BOOL8 RequiresLock();

    //! Is the weapon ready to be fired?
    HS_BOOL8 IsReady();

    //! Can this weapon attack the specified object
    HS_BOOL8 CanAttackObject(CHS3DObject * cObj);

    //! Is the specified object large enough to be attacked?
    HS_BOOL8 CheckTargetSize(CHS3DObject * cObj);

    //! Reload the tube
    HS_BOOL8 Reload();

    //! Is this tube configurable
    HS_BOOL8 Configurable();

    //! configure the tube
    HS_BOOL8 Configure(HS_INT32);

    //! Is the tube loadable?
    HS_BOOL8 Loadable();

    //! Unload a missile from this tube
    HS_BOOL8 Unload();

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

    void SetData(CHSMissileData * pData);
  protected:

    // Overload the variables from CHSMissileData
    HS_UINT32 m_speed;
    HS_UINT32 m_turnrate;
    HS_UINT32 m_strength;
    HS_UINT32 m_reload_time;
    HS_UINT32 m_range;

    //! Numer of cycles required to reload the tube
    HS_UINT32 m_time_to_reload;

    //! flag indicating if the tube is loaded
    HS_BOOL8 m_loaded;

    //! flag indicating if reloading is already inprogress
    HS_BOOL8 m_reloading;

    //! Missible bay where the missiles come from
    CHSMissileBay *m_missile_bay;
};


#endif // __HSMISSILE_INCLUDED__
