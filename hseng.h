// -----------------------------------------------------------------------
//  $Id: hseng.h,v 1.9 2006/04/04 12:56:10 mark Exp $
// -----------------------------------------------------------------------
#ifndef __HSENG_INCLUDED__
#define __HSENG_INCLUDED__

#include "HSVariant.h"
#include "HSEngTypes.h"
#include <cstdio>

class CHS3DObject;
class CHSEngSystemData;

extern const HS_INT8 *hsGetEngSystemName(int);
extern HSS_TYPE hsGetEngSystemType(const HS_INT8 *);

//! Damage level declaration applied to engineering systems
enum HS_DAMAGE
{
    DMG_NONE = 0,
    DMG_LIGHT,
    DMG_MEDIUM,
    DMG_HEAVY,
    DMG_INOPERABLE
};

// This is the percent of stress increase per cycle per percent
// overload.  So if you overload systems by 1%, then stress 
// increases .012% per cycle up to 100% stress.  If you overload
// 150%, then it increases .6% each cycle, which gives you about
// 2.8 minutes of heavy overloading before full system stress, 
// assuming cycles are 1 second intervals.  As tolerance increases,
// though, the stress increases more slow.  In fact, it's divided
// by tolerance, so the above information applies only for a tolerance
// of 1.
#define HS_STRESS_INCREASE	.012

//! A specific system, though general, that will be contained in the 
//! system array.  Systems can be derived from this base type.
class CHSEngSystem
{

  public:

    CHSEngSystem();
    virtual ~CHSEngSystem();

    // If you're deriving an object from this base class,
    // you should implement this function.  That's only
    // if you want the classdb to contain default information
    // for your system on the ship classes.  If not, then
    // you need to have your object constructor initialize
    // your variables to something meaningful, cause the
    // classdb loader won't do it without this function.
    virtual void SaveToFile(FILE *);

    // This doesn't have to be overloaded, but if you're
    // providing any strange name handling for your system,
    // you can override it to return the system name.
    virtual HS_INT8 *GetName();

    // If your system has any specific variables not covered
    // by this generic CHSEngSystem, you'll need to implement
    // this in your derived class to handle setting those
    // variables, especially when the system gets loaded
    // from the DB.
    //
    // This function takes values in character form since the caller
    // has no way of knowing what type of value the attribute requires.
    virtual HS_BOOL8 SetAttributeValue(const HS_INT8 * pcAttrName,
                                       const HS_INT8 * pcValue);

    // Call this to retrieve a list of valid attributes for the system.
    // Derived classes should override this to provide their own list,
    // but also call their base class version to provide all attributes
    // up the hierarchy.
    virtual void GetAttributeList(CHSAttributeList & rlistAttrs);

    // Override this to provide information about variables
    // specific to your system.  Refer to one of the other,
    // standard systems for implementation examples.
    // If bLocalOnly is set to true, then only the local value will be
    // checked, and false can be returned if the value is not set locally.
    virtual HS_BOOL8 GetAttributeValue(const HS_INT8 * pcAttrName,
                                       CHSVariant & rvarReturnVal,
                                       HS_BOOL8 bAdjusted,
                                       HS_BOOL8 bLocalOnly = false);

    // If you're doing any cyclic stuff that is different
    // from the cycle in CHSEngSystem, you'll want to 
    // implement this function in your derived class.  Typically,
    // you'll call CHSEngSystem::DoCycle() in your overridden
    // function to handle stress and such, but you don't have to.
    virtual void DoCycle();

    // You can override this in your derived class to
    // return optimal power in a different way than normal.
    virtual int GetOptimalPower(HS_BOOL8 bAdjusted = true);

    // Returns the locally set optimal power and doesn't fall through to the class.
    virtual HS_UINT32 *GetRawOptimalPower()
    {
        return m_puiOptimalPower;
    }

    // You can override this to provide custom status information
    // for your derived system.
    virtual HS_INT8 *GetStatus();

    // You can override this to handle anything specific to
    // your system when the power level is changed.  The
    // basic CHSEngSystem just does some error checking and
    // sets the power.
    virtual HS_BOOL8 SetCurrentPower(HS_UINT32);

    // Override this to provide and action taken
    // when power is taken from this system.
    virtual void CutPower(int);

    // Override this to provide an action taken when the
    // system is powered up(ONLY done when the system
    // was at 0 and now receiving power.)
    virtual void PowerUp(int);

    // These functions are not overridable.

    HS_BOOL8 IsVisible() const
    {
        return m_bVisible;
    }

    // Can be called to return a new copy of this system, whatever it may be.
    CHSEngSystem *Dup();

    // Call this to create a system of the specified type, if you don't
    // already know what type to create.
    static CHSEngSystem *CreateFromType(HSS_TYPE eType);

    void SetName(const HS_INT8 *);
    void SetOwner(CHS3DObject *);
    void SetNext(CHSEngSystem *);
    void SetPrev(CHSEngSystem *);
    HS_INT8 *GetOrigName();
    virtual void SetParentSystem(CHSEngSystem *);

    void SetVisible(HS_BOOL8 bValue)
    {
        m_bVisible = bValue;
    }
    void SetStress(HS_FLOAT32 fValue)
    {
        m_fStress = fValue;
    }
    void SetDamage(HS_DAMAGE eLevel)
    {
        m_eDamageLevel = eLevel;
    }

    // Allows the type of system to be set.
    // Only do this if it's not a derived system.
    void SetType(HSS_TYPE systype)
    {
        m_eType = systype;
    }

    void SetOptimalPower(HS_UINT32 uiValue)
    {
        if (!m_puiOptimalPower)
        {
            m_puiOptimalPower = new HS_UINT32;
        }

        *m_puiOptimalPower = uiValue;
    }

    void SetTolerance(HS_UINT32 uiValue)
    {
        if (!m_puiTolerance)
        {
            m_puiTolerance = new HS_UINT32;
        }

        *m_puiTolerance = uiValue;
    }

    HS_UINT32 GetCurrentPower() const
    {
        return m_uiCurrentPower;
    }

    HS_UINT32 GetTolerance();

    HS_UINT32 *GetRawTolerance()
    {
        return m_puiTolerance;
    }
    float GetStress();

    HSS_TYPE GetType();

    HS_DAMAGE GetDamageLevel() const
    {
        return m_eDamageLevel;
    }

    CHSEngSystem *GetParent()
    {
        return m_parent;
    }

    CHS3DObject *GetOwnerObject()
    {
        return m_ownerObj;
    }
    HS_DAMAGE DoDamage();
    HS_DAMAGE ReduceDamage(HS_DAMAGE lvl);
    HS_DAMAGE ReduceDamage();

    CHSEngSystem *GetPrev();
    CHSEngSystem *GetNext();

  protected:

    void CheckSystemPower();    // Prevents inadvertent overloading

  private:

    HSS_TYPE m_eType;

    HS_BOOL8 m_bVisible;        // Users can see it in the list and use it.

    HS_INT8 *m_pcName;          // System name

    HS_FLOAT32 m_fStress;       // 0 - 100 for amount of stress incurred

    HS_UINT32 m_uiCurrentPower; // Power allocated

    HS_DAMAGE m_eDamageLevel;   // Current damage level

    // These values may or may not be set and may fall through to the parent
    // system if not set.
    HS_UINT32 *m_puiOptimalPower;       // Optimal power to allocate

    HS_UINT32 *m_puiTolerance;  // How quickly the system stresses (1 .. n)

    CHS3DObject *m_ownerObj;    // Indicates the CHS3DObject owning the system

    // These variables are defined as pointers.  Why?  Well,
    // that's so we can override them specifically at the ship
    // level.  When a system tries to return a value to a calling
    // function, it checks to see if these variables are NULL.  If
    // so, it returns the default value from the class for that
    // ship.  Otherwise, we have no way of knowing whether a variable

    CHSEngSystem *m_parent;     // Parent system to retrieve default vals.
    CHSEngSystem *m_next;       // Ptrs for the linked list
    CHSEngSystem *m_prev;       // implementation.

};

// An array of systems that can exist for a ship.
// In fact, the systems are stored in a linked list.
class CHSSystemArray
{
  public:
    CHSSystemArray();

    //! @brief Add the given system to the linked list
    //! @return true on success, false on failure
    HS_BOOL8 AddSystem(CHSEngSystem * cSys);

    //! @brief Add the specified system to the linked list
    //! @param cSys - the engineering system to add
    //! @return true on success, false on failure
    HS_BOOL8 DelSystem(CHSEngSystem * cSys);


    //! @brief Equals operator copies data from the given array into this array
    //! @param cCopyFrom - array to copy
    void operator =(CHSSystemArray & cCopyFrom);        // Duplicates this array

    //! @brief Handles processing a cycle for all systems int he array and 
    //! accumulating total power consumption
    void DoCycle();

    //! Iterates through the array adding all current power consumption
    void UpdatePowerUse();

    //! Handles saving all system data to the given file pointer
    void SaveToFile(FILE * fp);

    //! @brief Moves a system up or down in the array of systems
    //! @param cSys - system to move
    //! @param iDir - direction to move: positive = up, negative = down
    //! @return false on error, true on success
    HS_BOOL8 BumpSystem(CHSEngSystem * cSys, int iDir);

    //! Return the current power consumption of all systems in the array
    HS_UINT32 GetPowerUse();

    //! Return the current number of systems in the array
    HS_UINT32 NumSystems();

    //! @brief Searches the list for a system of the given type
    //! @param systype - engineering system type to return
    //! @return NULL if not found or CHSEngSystem pointer to system data
    CHSEngSystem *GetSystem(HSS_TYPE systype);

    //! @brief Searches the list for a system by name
    //! @param strName - name to search for
    //! @return NULL if no system found or pointer to system data
    CHSEngSystem *GetSystemByName(const HS_INT8 * strName);

    //! Return a pointer to current head of the linked list
    CHSEngSystem *GetHead();

    //! @brief Randomly select a system
    //! @return NULL if no systems present or CHSEngSys pointer to selection
    CHSEngSystem *GetRandomSystem();

  protected:
    //! Pointer to the head of the linked list array
        CHSEngSystem * m_SystemHead;
    //! Pointer to the tail of the linked list array
    CHSEngSystem *m_SystemTail;

    //! Current Number of Systems in the array
    HS_UINT32 m_nSystems;

    //! Current power use of all systems in the array
    HS_UINT32 m_uPowerUse;
};


#endif // __HSENG_INCLUDED__
