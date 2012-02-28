// -----------------------------------------------------------------------
// $Id: hscelestial.h,v 1.11 2006/04/04 12:56:10 mark Exp $
// -----------------------------------------------------------------------

#ifndef __HSCELESTIAL_INCLUDED__
#define __HSCELESTIAL_INCLUDED__

#include "hsobjects.h"
#include "hsship.h"

#include <string>

//! @brief The CHSCelestial object is derived from the base object class and
//! is used to derive other types of celestial objects.
class CHSCelestial:public CHS3DObject
{
  public:
    //! Default constructor: does nothin
    CHSCelestial()
    {
    }

    // Inherited Methods that must be instantiated
    virtual void WriteToFile(FILE * fp);
    virtual void ClearObjectAttrs();
    virtual HS_INT8 *GetObjectColor();
    virtual HS_INT8 GetObjectCharacter();

    HS_UINT32 GetType();

  protected:
    //! Default deconstructor
    virtual ~CHSCelestial()
    {
    }

    // Inherited method
    virtual HS_BOOL8 HandleKey(HS_INT32 key, HS_INT8 * strValue,
                               FILE * fp = NULL);

};


//! @brief The CHSPlanet object is derived from the celestial object
//! to handle functions and information specific to planets.
class CHSPlanet:public CHSCelestial
{
  public:
    //! Default constructor only sets the type to HST_PLANET
    CHSPlanet();
    //! Default deconstrutor (does nothing)
    ~CHSPlanet();

    // Overriden inherited members
    void ClearObjectAttrs();
    void WriteToFile(FILE * fp);
    void GiveScanReport(CHS3DObject * cScanner, HS_DBREF player, HS_BOOL8 id);
    HS_INT8 *GetObjectColor();
    HS_INT8 GetObjectCharacter();
    HS_INT8 *GetAttributeValue(HS_INT8 * strName);
    HS_BOOL8 SetAttributeValue(HS_INT8 * strName, HS_INT8 * strValue);
    void GetAttributeList(CHSAttributeList & rlistAttributes);

  protected:
        HS_BOOL8 HandleKey(HS_INT32 key, HS_INT8 * strValue, FILE * fp);
};

//! @brief Nebula class derived from the generic celestial class
//! Nebulas cause shield deterioration
class CHSNebula:public CHSCelestial
{
  public:
    //! Constructor sets type to HST_NEBULA, density to 1, and shieldaff to 100
    CHSNebula();

    // Inherited methods
    void WriteToFile(FILE * fp);
    void ClearObjectAttrs();
    void GiveScanReport(CHS3DObject * cScanner, HS_DBREF player, HS_BOOL8 id);
    HS_INT8 GetObjectCharacter();
    HS_INT8 *GetObjectColor();
    HS_INT8 *GetAttributeValue(HS_INT8 * strName);
    HS_BOOL8 SetAttributeValue(HS_INT8 * strName, HS_INT8 * strValue);
    void GetAttributeList(CHSAttributeList & rlistAttributes);

    //! Get the current density value
    HS_INT32 GetDensity();

    //! Get the current shieldaff rating
    HS_FLOAT32 GetShieldaff();

  protected:
    //! Default deconstructor - does nothing
       ~CHSNebula()
    {
    }

    HS_BOOL8 HandleKey(HS_INT32 key, HS_INT8 * strValue, FILE * fp);

    // Attributes
    HS_INT32 m_density;
    HS_FLOAT32 m_shieldaff;
};

//! @brief  The CHSAsteroids class implements asteroid fields in space
class CHSAsteroid:public CHSCelestial
{
  public:
    //! Constructor sets the type, visibility to true and density to 1
    CHSAsteroid();

    //! @brief Checks all objects in the same universe for proximity and damage
    //! Looks at the current list of objects in the same universe as
    //! the asteroid.  If the object is a ship and is within the asteroid
    //! size times the asteroid_size_multiplier in the configuration,
    //! the ship may take damage.   Damage occurs if the density of the
    //! asteroid field is less than a randomly calculated number based
    //! on ship size and speed.
    void DoCycle();

    //! Write asteroid data to the specified file
    void WriteToFile(FILE * fp);

    //! Call parent method and set density to 0
    void ClearObjectAttrs();

    //! Report information to a player when the asteroid is scanned
    void GiveScanReport(CHS3DObject * cScanner, HS_DBREF player, HS_BOOL8 id);

    //! Asteroids are ':'.
    HS_INT8 GetObjectCharacter();

    //! Asteroids are currently black 
    HS_INT8 *GetObjectColor();

    //! Lookup an attribute value on the asteroid
    HS_INT8 *GetAttributeValue(HS_INT8 * strName);

    //! Set the strName attribute to strValue
    HS_BOOL8 SetAttributeValue(HS_INT8 * strName, HS_INT8 * strValue);

    //! Get the current list of settable/gettable attributes
    void GetAttributeList(CHSAttributeList & rlistAttributes);

    //! Return the current density value
    HS_INT32 GetDensity();

  protected:
    //! Deconstructor does nothing
       ~CHSAsteroid();

    //! Handle database keys -- HSK_DENSITY and parent method
    HS_BOOL8 HandleKey(HS_INT32 key, HS_INT8 * strValue, FILE * fp);

    // Attributes
    HS_INT32 m_density;
};

//! @brief Blackholes can suck ships into the nether and cause damage
class CHSBlackHole:public CHSCelestial
{
  public:
    //! Default constructor
    CHSBlackHole();
    //! Default deconstructor
    ~CHSBlackHole();

    //! The blackhole cycle checks for ships within Size*100 units of the
    //! center coordinates of the blackhole.   If within that radius, the
    //! ship is drawn toward the center of the blackhole and damage is 
    //! applied to the ships hull.  Both the speed of the gravity draw and
    //! the amount of damage is based off the proximity of the ship to 
    //! the coordinates of the blackhole
    void DoCycle();

    HS_INT8 GetObjectCharacter();

    HS_INT8 *GetObjectColor();

  protected:
};

//! @brief Wormholes can transfer ships from one universe to another but may 
//! damage the ship in the process.  The stability of the wormhole
//! fluctuates every cycle.
class CHSWormHole:public CHSCelestial
{
  public:
    //! Default constructor
    CHSWormHole();
    //! Default deconstructor
    ~CHSWormHole();

    //! Calculates the current stability measure for the wormhole
    void DoCycle();

    //! Attempt to transfer the target ship through the wormhold to 
    //! the destination universe & coordinates.   Ships may be destroyed
    //! if the stability of the wormhole is low.  Ships may also have
    //! some error upon arriving in the target universe
    void GateShip(CHSShip * cShip);

    // Inherited methods
    void GiveScanReport(CHS3DObject * cScanner, HS_DBREF player, HS_BOOL8 id);
    void ClearObjectAttrs();
    void WriteToFile(FILE * fp);
    HS_INT8 *GetAttributeValue(HS_INT8 * strName);
    HS_INT8 GetObjectCharacter();
    HS_INT8 *GetObjectColor();
    HS_BOOL8 SetAttributeValue(HS_INT8 * strName, HS_INT8 * strValue);
    void GetAttributeList(CHSAttributeList & rlistAttributes);

    //! Get the current fluctuation value
    HS_FLOAT32 GetFluctuation();

    //! Get the current stability rating
    HS_FLOAT32 GetStability();

    //! Get the base stability rating
    HS_FLOAT32 GetBaseStability();

  protected:
        HS_BOOL8 HandleKey(HS_INT32 key, HS_INT8 * strValue, FILE * fp);

    HS_FLOAT64 m_destx;
    HS_FLOAT64 m_desty;
    HS_FLOAT64 m_destz;
    HS_UINT32 m_destuid;
    HS_UINT32 m_desterror;
    HS_BOOL8 m_nodamage;
    HS_FLOAT32 m_fluctuation;
    HS_FLOAT32 m_stability;
    HS_FLOAT32 m_basestability;
};

#endif // __HSCELESTIAL_INCLUDED__
