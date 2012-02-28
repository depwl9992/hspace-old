// -----------------------------------------------------------------------
//! $Id: hsship.h,v 1.18 2006/04/29 12:26:33 mark Exp $
// -----------------------------------------------------------------------

#ifndef HSSHIP_INCLUDED__
#define HSSHIP_INCLUDED__

#include "hsobjects.h"
#include <vector>

#define MAX_SHIP_CONSOLES	 20

class CHSShipClass;
class CHSTerritory;             // From hsterritory.h
class CHSSysComputer;
class CHSSysShield;
class CHSHatch;
struct SENSOR_CONTACT;

//! The generic ship object containing info about its class, weapons, etc.
class CHSShip:public CHS3DObject
{
  public:

    typedef std::set < HS_DBREF > CHSShipRoomSet;

    //! Default constructor
        CHSShip();
    //! Default deconstructor
       ~CHSShip();

    //! Get the cloaking effect measured between 0.0 and 1.00
    HS_FLOAT32 CloakingEffect();        // 0 - 1.00

    //! Get the tachyon effect measured between 0.0 and 1.00
    HS_FLOAT32 TachyonEffect(); // 0 - 1.00

    //! Return the class number of this vessel
    HS_UINT32 ClassNum();

    //! Get the current XYHeading of the ship
    HS_UINT32 GetXYHeading();

    //! Get the ship's Z heading
    HS_INT32 GetZHeading();

    //! Get the roll value of the ship
    HS_UINT32 GetRoll();

    //! Get the maximum hull points for the ship
    HS_UINT32 GetMaxHullPoints();

    //! Get the current hull points remaining
    HS_UINT32 GetHullPoints();

    //! Get the size of the ship or the class size if not set
    HS_UINT32 GetSize();        // Overrides the base class

    //! Get the list of installed engineering system types
    HS_UINT32 GetEngSystemTypes(HS_INT32 *);

    //! Get the desired XY heading for the ship
    HS_UINT32 GetDesiredXYHeading();

    //! Get the desired Z heading for the ship
    HS_INT32 GetDesiredZHeading();

    //! Get the current speed of the ship 
    HS_INT32 GetSpeed();

    //! Return the number of manned consoles
    HS_INT32 GetMannedConsoles();

    //! How many consoles must be manned to operate this ship?
    HS_INT32 GetMinManned();

    //! Get the docking location or HSNOTHING if not docked
    HS_DBREF GetDockedLocation();

    //! Clone this vessel
    HS_DBREF Clone();

    //! Load data from the specified game object????
    HS_BOOL8 LoadFromObject(HS_DBREF);

    //! Is the vessel active?
    HS_BOOL8 IsActive();

    //! Set the baseline class ID for this ship
    HS_BOOL8 SetClassInfo(HS_UINT32);

    //! Add a console to the ship
    HS_BOOL8 AddConsole(HS_DBREF);

    //! Remove a console from the ship
    HS_BOOL8 RemoveConsole(HS_DBREF);

    // Inherited method to set the strName attribute to strValue
    HS_BOOL8 SetAttributeValue(HS_INT8 * strName, HS_INT8 * strValue);

    //! @brief Set an attribute on a specified system
    //! @param lpstrSysName - the system to set the attribute on
    //! @param lpstrAttr - the attribute name to set
    //! @param lpstrValue - the value to place in the attribute
    //! @return true on success, false on invalid attribute or value
    HS_BOOL8 SetSystemAttribute(HS_INT8 * lpstrSysName,
                                HS_INT8 * lpstrAttr, HS_INT8 * lpstrValue);

    //! Add the specified room to the ship
    HS_BOOL8 AddSroom(HS_DBREF room);

    //! Is the specified room a ship room?
    HS_BOOL8 HasSroom(HS_DBREF dbRoom)
    {
        CHSShipRoomSet::iterator iter = m_setRooms.find(dbRoom);
        return (iter != m_setRooms.end());
    }

    //! Add a hatch to the ship
    HS_BOOL8 AddHatch(HS_DBREF objnum);

    //! Delete the specified hatch
    HS_BOOL8 DeleteHatch(HS_DBREF objnum);

    //! Delete the specified room from the ship room list
    HS_BOOL8 DeleteSroom(HS_DBREF room);

    //! Can this vessel drop to the surface of a celestial object?
    HS_BOOL8 CanDrop();

    //! Return the cargo capacity for the vessel
    HS_UINT32 CargoSize();

    //! Is this vessel a space dock?
    HS_BOOL8 IsSpacedock();

    //! Is the vessel travelling in hyperspace
    HS_BOOL8 InHyperspace();

    //! Has this vessel been destroyed
    HS_BOOL8 IsDestroyed();

    //! This appears to be undefined....
    HS_BOOL8 MakeBoardLink(CHSShip *, HS_BOOL8);

    //! Is the ship landed?
    HS_BOOL8 Landed();

    //! Does any console have a target lock on the specified object?
    HS_BOOL8 IsObjectLocked(CHS3DObject * cObj);

    //! Kill all players onboard the ship, notifying them with msg
    void KillShipCrew(const HS_INT8 * msg);

    //! Perform post-database load cleanup for the object
    void EndLoadObject();

    //! Transfer the game object representing the ship to location
    void MoveShipObject(HS_DBREF location);

    //! Totally repair the ship and all systems
    void TotalRepair();         // Repairs everything

    //! Restore fuel to the maximum levels
    void Refuel();              // Maximizes fuel

    //! Return the boarding code for this vessel
    HS_INT8 *GetBoardingCode();

    //! Set attributes on game object as needed
    void WriteObjectAttrs();

    // Inherited methods from parent class
    void DoCycle();
    void ClearObjectAttrs();
    void WriteToFile(FILE * fp);
    HS_INT8 *GetObjectColor();
    HS_INT8 *GetAttributeValue(HS_INT8 * strName);
    void GetAttributeList(CHSAttributeList & rlistAttributes);
    HS_INT8 GetObjectCharacter();
    void HandleMessage(const HS_INT8 * lpstrMsg, HS_INT32 msgType,
                       long *data = NULL);
    void HandleLock(CHS3DObject * cLocker, HS_BOOL8 bLocking);
    void HandleDamage(CHS3DObject * cSource, CHSWeaponData * cWeapon,
                      HS_INT32 strength, CHSConsole * cConsole,
                      HS_INT32 iSysType);
    void HandleDamage(HS_FLOAT64 x, HS_FLOAT64 y, HS_FLOAT64 z,
                      HS_INT32 strength);
    void ExplodeMe();
    void GiveScanReport(CHS3DObject * cScanner, HS_DBREF player, HS_BOOL8 id);

    //! Change the current motion vector to the specified values
    void SetHeadingVector(HS_INT32 i, HS_INT32 j, HS_INT32 k);

    //! Generate a crew report and notify player
    void GiveCrewRep(HS_DBREF player);
    //! Generate a hatch report and notify player
    void GiveHatchRep(HS_DBREF player);

    //! @brief Assign damage repair crews
    //! @param player - player making the assignment
    //! @param iCrew - crew being assigned
    //! @param lpstrSysName - system to repair
    void AssignCrew(HS_DBREF player, HS_INT32 iCrew, HS_INT8 * lpstrSysName);

    //! @brief Unassign a damage repair crew from its current task
    //! @param player - player making the unassign request
    //! @param iCrew - crew number being unassigned
    void UnassignCrew(HS_DBREF player, HS_INT32 iCrew);

    //! Validate all current boarding links
    void ConfirmHatches();

    //! Return the console at the specified slot
    CHSConsole *GetConsole(HS_UINT32 slot);

    //! Find a ship console by the target dbref
    CHSConsole *FindConsole(HS_DBREF objnum);   // Locates a console object

    //! Return the specified hatch at slot in list
    CHSHatch *GetHatch(HS_INT32 slot);

    //! Find a hatch by dbref
    CHSHatch *FindHatch(HS_DBREF objnum);

    //! Get the specified engineering system ot type
    CHSEngSystem *GetEngSystem(HS_INT32 type);

    //! Return the current engineering systems array
    CHSSystemArray *GetEngSystemArray();

    //! Retrieve the SENSOR_CONTACT for the specified object
    SENSOR_CONTACT *GetSensorContact(CHS3DObject * cObj);

    //! Retrieve a SENSOR_CONTACT by contact id
    SENSOR_CONTACT *GetSensorContact(HS_INT32 contact_id);

    // Methods that players interact with
    //! Report Engineering systems information to player
    void GiveEngSysReport(HS_DBREF player);

    //! @brief Allows a player to set system power by level or percent
    //! @param player - player requesting the power set
    //! @param lpstrName - name of system to set power on
    //! @param iLvl - level of power to set
    //! @param bPercent - if true, use iLvl as a percentage of optimal power
    void SetSystemPower(HS_DBREF player, HS_INT8 * lpstrName,
                        HS_INT32 iLvl, HS_BOOL8 bPercent);

    //! @brief Change the system power priority for a specific system
    //! @param player - player requesting the change
    //! @param lpstrName - name of the system to adjust
    //! @param iChange - change system up or down in the list  (???)
    void ChangeSystemPriority(HS_DBREF player, HS_INT8 * lpstrName,
                              HS_INT32 iChange);

    //! @brief Set the desired ship velocity
    //! @param player - player enacting the change
    //! @param iVel - desired velocity
    void SetVelocity(HS_DBREF player, HS_INT32 iVel);


    //! @brief Allow player to specify heading
    //! @param player - enacting player
    //! @param xy - xy heading value
    //! @param z - z heading value
    void SetHeading(HS_DBREF player, HS_INT32 xy = -1, HS_INT32 z = -1);

    //! @brief Allow player to set the desired roll of the ship
    //! @param player - enacting player
    //! @param roll - desired roll
    void SetRoll(HS_DBREF player, HS_INT32 roll = -1);

    //! @brief Generate a sensor report
    //! @param player - player getting the report
    //! @param tType - if specified, report only on that type
    void GiveSensorReport(HS_DBREF player, HS_TYPE tType = HST_NOTYPE);

    //! Report a generic picture of ship with hull and shield percentages
    void GiveVesselStats(HS_DBREF player);

    //! Generate a navigation status report for the player
    void GiveNavigationStatus(HS_DBREF player);

    //! @brief Attempt to land the ship in another ship or on a planet
    //! @param player - player making the landing command
    //! @param id - sensor contact id
    //! @param pcLocation - which landing location slot to use
    //! @param lpstrCode - landing access code, if necessary
    void LandVessel(HS_DBREF player, HS_INT32 id, const HS_INT8 * pcLocation,
                    HS_INT8 * lpstrCode);

    //! Lift off from a planet or undock from another ship
    void UndockVessel(HS_DBREF player);

    //! Send a message to the consoles of the ship
    void NotifyConsoles(HS_INT8 * strMsg, HS_INT32 msgLevel);

    //! Send a message to all the ship rooms
    void NotifySrooms(HS_INT8 * strMsg);

    //! Engage or disengage the afterburner
    void EngageAfterburn(HS_DBREF player, HS_BOOL8 bStat);

    //! Engage or disengage the jump drive
    void EngageJumpDrive(HS_DBREF player, HS_BOOL8 bStat);

    //! Engage the warp drive
    void EngageWarpDrive(HS_DBREF player, HS_FLOAT32 level);

    //! Engage or disengage the autopilot
    void EngageAutoPilot(HS_DBREF player, HS_BOOL8 bStat);

    //! Enagage or disengage the cloaking device
    void EngageCloak(HS_DBREF player, HS_BOOL8 bStat);

    //! @brief Attempt to establish a boarding link with another vessel
    //! @param player - enacting player
    //! @param id - sensor contact id
    //! @param lhatch - local hatch to use
    //! @param dhatch - destination hatch to use
    void DoBoardLink(HS_DBREF player, HS_INT32 id, HS_INT32 lhatch,
                     HS_INT32 dhatch);

    //! @brief Disengage the specified hatch's boarding link
    void DoBreakBoardLink(HS_DBREF player, HS_INT32 slot);

    //! @brief Allow a player to depart the vessel 
    //! @param player - departing player
    //! @param id - totally unused api variable...
    void DisembarkPlayer(HS_DBREF player, HS_INT32 id);


    //! Scan the specified target id and report results to player
    void ScanObjectID(HS_DBREF player, HS_INT32 id);

    //! View the description of the target id
    void ViewObjectID(HS_DBREF player, HS_INT32 id);

    //! Gate ship through the specified object id
    void GateObjectID(HS_DBREF player, HS_INT32 id);

    //! @brief Start the self destruction sequence
    //! @param player - enacting player
    //! @param timer - amount of time to delay destruction
    //! @param boardCode - boarding code for authentication
    void InitSelfDestruct(HS_DBREF player, HS_INT32 timer,
                          HS_INT8 * boardCode);

    //! Set maximum number of type missiles to max count for the ship 
    HS_BOOL8 SetNumMissiles(HS_DBREF player, HS_INT32 type, HS_INT32 max);

    //! Return the current engineering system array
    CHSSystemArray & GetSystems()
    {
        return m_systems;
    }

    //! Set the map range for the navigation reports
    HS_BOOL8 SetMapRange(HS_DBREF player, HS_INT32 range);

    //! Return the missile bay information or NULL if not present
    CHSMissileBay *GetMissileBay();

    //! Set hullpoints to val
    void HullPoints(HS_INT32 val)
    {
        m_hull_points = val;
    }
    //! Get current hull points
    HS_INT32 HullPoints()
    {
        return m_hull_points;
    }

    void SetDocked()
    {
        m_docked = true;
        m_in_space = false;
    }

    //! @todo Fix all these unprotected variables!

    HS_DBREF m_objlocation;     // Dbref where it's docked or dropped

    HS_BOOL8 m_docked;

  protected:

    // Methods
    //! @brief Determine which ship shield is hit for an incoming shot
    //! @param iXYAngle - xy angle of the incoming shot
    //! @param iZAngle - z angle of the incoming shot
    //! @return pointer to shield that was hit or NULL if no shields present
    CHSSysShield * DetermineHitShield(HS_INT32 iXYAngle, HS_INT32 iZAngle);

    //! Recursively clone the rooms of a ship
    //! @param room - dbref of room to clone
    //! @param rmapVisitedRooms - state map of rooms visited for recursion
    //! @param newShip - ship being cloned to add the rooms to
    //! @return dbref of the newly cloned room
    //! @todo This method contains a number of direct inferface calls to
    //! the game server.   If possible, they should be moved into 
    //! hsinterface class.
    HS_DBREF CloneRoom(HS_DBREF room,
                       std::map < HS_DBREF, HS_DBREF > &rmapVisitedRooms,
                       CHSShip * newShip);

    HS_BOOL8 HandleKey(HS_INT32 key, HS_INT8 * strValue, FILE * fp);

    //! When the HandleKey() encounters a SYSTEMDEF key, it
    //! calls this function to load the system information into
    //! a system that already exists on the ship.
    //! @brief Called by HandleKey() to load system information
    //! @param fp - file pointer to use for loading
    //! @return true on success, false on failure
    HS_BOOL8 LoadSystem(FILE * fp);

    //! Transitions the ship into hyperspace including effect messages
    void EnterHyperspace();
    //! Transitions the ship out of hyperspace including effect messages
    void ExitHyperspace();

    //! Move the ship according to the current settings and motion vector
    void Travel();

    //! Performs sensor queries and relays contact information to consoles
    void HandleSensors();

    //! Process system specific routines once per cycle
    void HandleSystems();

    //! Process life support for one cycle (and kill players if needed) 
    void HandleLifeSupport();

    //! Process heading changes once per cycle based on current info
    void ChangeHeading();

    //! Handles recomputing the current heading vectors
    void RecomputeVectors();

    //! @brief Begins the landing process for planets and ships
    //! @param player - player requesting the landing
    //! @param cContact - sensor contact being landed upon
    //! @param cLocation - location being landed on
    void InitLanding(HS_DBREF player, SENSOR_CONTACT * cContact,
                     CHSLandingLoc * cLocation);

    //! Process the landing or docking procedure including messages
    void HandleLanding();

    //! Handles the undocking/liftoff procedures as they progress
    void HandleUndocking();

    //! Restore a previously destroyed ship to an active state
    void ResurrectMe();

    //! @brief Handle any communication message sent to the ship
    //! @param msg - the incoming message
    //! @param data - pointer to an HSCOMM structure with info needed
    void HandleCommMsg(const HS_INT8 * msg, long *data);

    //! @brief Allocate a new hatch object if possible
    //! @return NULL on error, CHSHatch* if successful
    CHSHatch *NewHatch();

    // Attributes
    //! Engineering systems array
    CHSSystemArray m_systems;
    HS_INT32 m_hull_points;
    HS_INT8 *m_ident;
    HS_INT8 *m_boarding_code;
    HS_BOOL8 *m_can_drop;
    HS_BOOL8 *m_spacedock;
    HS_INT32 *m_cargo_size;
    HS_UINT32 *m_maxhull;

    CHSTerritory *m_territory;  // Territory we're in.

    CHSLandingLoc *m_landing_target;    // Valid only during landing

    HS_INT32 m_current_xyheading;
    HS_INT32 m_desired_xyheading;
    HS_INT32 m_current_zheading;
    HS_INT32 m_desired_zheading;
    HS_INT32 m_desired_roll;
    HS_INT32 m_current_roll;
    HS_INT32 m_drop_status;
    HS_INT32 m_dock_status;
    HS_INT32 *m_minmanned;
    HS_INT32 m_class;
    HS_INT32 m_map_range;
    HS_INT32 m_current_speed;
    HS_INT32 m_self_destruct_timer;
    HS_INT32 m_age;

    HS_BOOL8 bGhosted;          // Life support failed?
    HS_BOOL8 bReactorOnline;
    HS_BOOL8 m_destroyed;
    HS_BOOL8 m_in_space;
    HS_BOOL8 m_hyperspace;      // In warp/hyperspace or not.
    HS_BOOL8 m_warpengaged;

    CHS3DObject *m_target;      // Used for locking onto a target.

    // Registered ship rooms
    CHSShipRoomSet m_setRooms;

    // Registered consoles.  NOTE: Ships initially
    // start with NO consoles.  The object representing
    // the ship may or may NOT be a console.
    CHSConsole *m_console_array[MAX_SHIP_CONSOLES];

    // Boarding links with other ships
    std::vector< CHSHatch* > m_hatches;

    // Ship class info
    CHSShipClass *m_classinfo;

    // Missile bay for the ship
    CHSMissileBay m_missile_bay;
};

#endif // HSSHIP_INCLUDED__
