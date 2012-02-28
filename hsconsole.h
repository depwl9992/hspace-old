// -----------------------------------------------------------------------
//!
//! @$Id: hsconsole.h,v 1.9 2006/04/21 13:53:19 mark Exp $
//!
//! @par Copyright:
//! Copyright (c) 2005 by HSpace Development Team -- see hscopyright.h
//!
//! @brief CHSConsole class declaration
//!
// -----------------------------------------------------------------------

#ifndef __HSCONSOLE_INCLUDED__
#define __HSCONSOLE_INCLUDED__

#include "hsobjects.h"
#include "hsweapon.h"
#include "hssensors.h"
#include "hscomputer.h"
#include "hspace.h"

class CHSVector;


// A generic console object
class CHSConsole
{
  public:
    CHSConsole(void);
        virtual ~CHSConsole(void);

    void SetOwner(HS_DBREF);
    void HandleMessage(const HS_INT8 *, HS_INT32);
    void AdjustHeading(HS_INT32, HS_INT32);
    void SetOwnerObj(CHS3DObject *);
    void SetMissileBay(CHSMissileBay *);
    void DoCycle(void);
    CHSSysComputer *GetComputer(void);

    HS_INT8 *GetAttributeValue(HS_INT8 *);
    HS_BOOL8 SetAttributeValue(HS_INT8 *, HS_INT8 *);
    HS_UINT32 GetMaximumPower(void);
    HS_BOOL8 LoadFromObject(HS_DBREF);
    HS_BOOL8 IsOnline(void);
    HS_BOOL8 OnFrq(HS_FLOAT64); // On a given frequency or not.
    HS_BOOL8 GetMessage(HS_INT32);
    HS_BOOL8 AddMessage(HS_INT32);
    HS_BOOL8 DelMessage(HS_INT32);
    HS_BOOL8 GetAutoload(void)
    {
        return m_autoload;
    }

    HS_INT32 GetXYHeading(void);
    HS_INT32 GetZHeading(void);

    HS_DBREF GetOwner(void);

    CHS3DObject *GetObjectLock(void);
    CHSVector & GetHeadingVector(void);
    CHSWeaponArray *GetWeaponArray(void);

    // These commands interact with players in the game.
    HS_BOOL8 AddWeapon(HS_DBREF, HS_INT32);
    void LockTargetID(HS_DBREF, HS_INT32);
    void LockTarget(CHS3DObject *);
    void UnlockTarget(CHS3DObject *);
    void UnlockWeapons(HS_DBREF);
    void GiveGunneryReport(HS_DBREF);
    void DeleteWeapon(HS_DBREF, HS_INT32);
    void ConfigureWeapon(HS_DBREF, HS_INT32, HS_INT32);
    void LoadWeapon(HS_DBREF, HS_INT32);
    void ChangeHeading(HS_DBREF, HS_INT32, HS_INT32);
    void FireWeapon(HS_DBREF, HS_INT32, HS_INT32);
    void PowerUp(HS_DBREF);
    void PowerDown(HS_DBREF);
    void GiveTargetReport(HS_DBREF);
    void UnloadWeapon(HS_DBREF, HS_INT32);
    void SetAutoload(HS_DBREF, HS_BOOL8);
    void SetAutoRotate(HS_DBREF, HS_BOOL8);
    void SetSystemTarget(HS_DBREF, HS_INT32);

    // Attributes
    HS_DBREF m_objnum;

    HS_BOOL8 IsInCone(CHS3DObject *);
    void ClearObjectAttrs(void);

  protected:
    // Member functions
    void LoadWeapons(HS_INT8 *);

    void WriteFiringArcAttr(void);
    void WriteCanRotateAttr(void);
    void WriteOffsetAttr(void);
    void WriteXYHeadingAttr(void);
    void WriteZHeadingAttr(void);
    void WriteMessageTypeAttrs(void);
    void WriteWeaponAttr(void);

    // Attributes
    HS_DBREF m_owner;
    HS_BOOL8 m_online;
    HS_INT32 m_xyheading;
    HS_INT32 m_xyoffset;        // XY offset from front of ship
    HS_INT32 m_zoffset;         // Z offset from front of ship
    HS_INT32 m_zheading;
    HS_INT32 m_arc;             // Firing arc
    HS_INT32 m_targetting;      // Which system is being targetted
    HS_INT32 m_msgtypes[NUM_MESSAGE_TYPES];

    HS_BOOL8 m_can_rotate;      // Console can rotate like a turret
    HS_BOOL8 m_autoload;
    HS_BOOL8 m_autorotate;

    CHS3DObject *m_target_lock;
    CHSWeaponArray *m_weapon_array;
    CHS3DObject *m_ownerObj;
    CHSMissileBay *m_missile_bay;
};

#endif // __HSCONSOLE_INCLUDED__
