// -----------------------------------------------------------------------
//  $Id: hseng.cpp,v 1.13 2006/04/27 16:54:43 mark Exp $
// -----------------------------------------------------------------------
#include "pch.h"

#include <stdlib.h>
#include <cstdio>
#include <time.h>
#include <assert.h>

#ifndef WIN32
#include <strings.h>
#endif

#include "hstypes.h"
#include "hsutils.h"
#include "hsuniverse.h"
#include "hscelestial.h"
#include "hsansi.h"
#include "hsengines.h"
#include "hsjumpdrive.h"
#include "hssensors.h"
#include "hsthrusters.h"
#include "hscomputer.h"
#include "hsshields.h"
#include "hsreactor.h"
#include "hscommunications.h"
#include "hsconf.h"
#include "hspace.h"
#include "hsinterface.h"
#include "hslifesupport.h"
#include "hscloaking.h"
#include "hstachyon.h"
#include "hsdamagecontrol.h"
#include "hsjammer.h"
#include "hstractor.h"
#include "hsautopilot.h"
#include "hswarpdrive.h"

#include "hseng.h"
#include "hsfictional.h"


//! Definition for defining available engineering systems
struct SYSTEMTYPE
{
    char *name;
    HSS_TYPE type;
};

//! An array of available systems that can exist on ships.
//! If you create a new system type, you need to add its name
//! here.
const SYSTEMTYPE hs_system_list[] = {
    {"Reactor", HSS_REACTOR},
    {"Life Support", HSS_LIFE_SUPPORT},
    {"Internal Computer", HSS_COMPUTER},
    {"Engines", HSS_ENGINES},
    {"Sensor Array", HSS_SENSORS},
    {"Maneuv. Thrusters", HSS_THRUSTERS},
    {"Fore Shield", HSS_FORE_SHIELD},
    {"Aft Shield", HSS_AFT_SHIELD},
    {"Starboard Shield", HSS_STARBOARD_SHIELD},
    {"Port Shield", HSS_PORT_SHIELD},
    {"Fuel System", HSS_FUEL_SYSTEM},
    {"Jump Drive", HSS_JUMP_DRIVE},
    {"Comm. Array", HSS_COMM},
    {"Cloaking Device", HSS_CLOAK},
    {"Tachyon Sensor Array", HSS_TACHYON},
    {"Fictional System", HSS_FICTIONAL},
    {"Damage Control", HSS_DAMCON},
    {"Comm. Jammer", HSS_JAMMER},
    {"Tractor Beam", HSS_TRACTOR},
    {"AutoPilot", HSS_AUTOPILOT},
    {"Warp Drive", HSS_WARP_DRIVE},
    {NULL}
};

const HS_INT8 *hsGetEngSystemName(int type)
{

    for (int idx = 0; hs_system_list[idx].name; idx++)
    {
        if (hs_system_list[idx].type == type)
            return hs_system_list[idx].name;
    }
    return NULL;
}

HSS_TYPE hsGetEngSystemType(const char *name)
{
    int len = strlen(name);

    for (int idx = 0; hs_system_list[idx].name; idx++)
    {
        if (!_strnicmp(name, hs_system_list[idx].name, len))
        {
            return hs_system_list[idx].type;
        }
    }
    return HSS_NOTYPE;
}

//
// CHSSystemArray stuff
//
CHSSystemArray::CHSSystemArray():
m_SystemHead(NULL), m_SystemTail(NULL), m_nSystems(0), m_uPowerUse(0)
{
}

// Returns one of the random systems
CHSEngSystem *CHSSystemArray::GetRandomSystem()
{
    int iRoll;
    CHSEngSystem *ptr;
    int idx;

    // Roll the dice from 1 .. nSystems
    iRoll = hsInterface.GetRandom(m_nSystems);

    // Resulting iRoll is 0 .. n-1.  Find the system.
    idx = 0;
    for (ptr = m_SystemHead; NULL != ptr; ptr = ptr->GetNext())
    {
        if (idx == iRoll)
        {
            return ptr;
        }

        idx++;
    }

    // Hrm.  No system found, so return NULL.
    return NULL;
}

HS_BOOL8 CHSSystemArray::AddSystem(CHSEngSystem * cSys)
{

    if (NULL == cSys)
    {
        return false;
    }

    // Add system to the list or start new list.
    if (!m_SystemHead)
    {
        m_SystemHead = cSys;
        m_SystemTail = cSys;
    }
    else
    {
        m_SystemTail->SetNext(cSys);
        cSys->SetPrev(m_SystemTail);
        m_SystemTail = cSys;
    }

    m_nSystems++;
    return true;
}

HS_BOOL8 CHSSystemArray::DelSystem(CHSEngSystem * cSys)
{
    // Delete the system from the list and close the ends again :).  
    CHSEngSystem *pSys;
    CHSEngSystem *nSys;
    pSys = cSys->GetPrev();
    nSys = cSys->GetNext();


    if (!nSys && !pSys)
    {
        m_SystemHead = m_SystemTail = NULL;
        return true;
    }
    else if (!nSys)
    {
        pSys->SetNext(NULL);
        m_SystemTail = pSys;
    }
    else if (!pSys)
    {
        nSys->SetPrev(NULL);
        m_SystemHead = nSys;
    }
    else
    {
        pSys->SetNext(nSys);
        nSys->SetPrev(pSys);
    }

    delete cSys;
    m_nSystems--;
    return true;
}



// This operator can be used to duplicate the contents
// of this array into another array that was passed in.
void CHSSystemArray::operator =(CHSSystemArray & cCopyFrom)
{
    CHSEngSystem *ptr;
    CHSEngSystem *tmp;

    // Delete any systems we have loaded already.
    ptr = m_SystemHead;
    while (ptr)
    {
        tmp = ptr;
        ptr = ptr->GetNext();
        delete tmp;
    }
    m_SystemHead = m_SystemTail = NULL;

    // Copy number of systems
    m_nSystems = cCopyFrom.m_nSystems;

    // Now duplicate the systems
    for (ptr = cCopyFrom.m_SystemHead; ptr; ptr = ptr->GetNext())
    {
        // Dup the system
        tmp = ptr->Dup();

        // If the system we're cloning doesn't have a parent,
        // then it's a default system, so it's now our parent.
        tmp->SetParentSystem(ptr);

        // Add it to our linked list.
        if (!m_SystemHead)
        {
            m_SystemHead = m_SystemTail = tmp;
        }
        else
        {
            tmp->SetPrev(m_SystemTail);
            m_SystemTail->SetNext(tmp);
            m_SystemTail = tmp;
        }
    }
}

// Handles cycling systems, powering them down, etc.
void CHSSystemArray::DoCycle()
{
    CHSEngSystem *cSys;

    // Tell the systems to cycle.  At the same time,
    // calculate power used.
    m_uPowerUse = 0;
    for (cSys = m_SystemHead; cSys; cSys = cSys->GetNext())
    {
        cSys->DoCycle();
        m_uPowerUse += cSys->GetCurrentPower();
    }
}

// Updates the current power usage.  Call this after
// changing a power setting on a system.
void CHSSystemArray::UpdatePowerUse()
{
    CHSEngSystem *cSys;

    m_uPowerUse = 0;
    for (cSys = m_SystemHead; cSys; cSys = cSys->GetNext())
    {
        m_uPowerUse += cSys->GetCurrentPower();
    }
}

// Returns current power usage
HS_UINT32 CHSSystemArray::GetPowerUse()
{
    return m_uPowerUse;
}

// Returns the head of the linked list of systems
CHSEngSystem *CHSSystemArray::GetHead()
{
    return m_SystemHead;
}

// Returns number of systems
HS_UINT32 CHSSystemArray::NumSystems()
{
    return m_nSystems;
}

void CHSSystemArray::SaveToFile(FILE * fp)
{

    if (NULL == fp)
    {
        return;
    }

    HS_UINT32 priority = 0;
    CHSEngSystem *ptr;
    for (ptr = GetHead(); NULL != ptr; ptr = ptr->GetNext())
    {
        fprintf(fp, "SYSTEMDEF\n");
        fprintf(fp, "SYSTYPE=%d\n", ptr->GetType());
        ptr->SaveToFile(fp);

        // Record the system priority for rebuilding priority
        // on reload.
        if (ptr->GetType() != HSS_REACTOR)
        {
            fprintf(fp, "PRIORITY=%d\n", priority++);
        }
        fprintf(fp, "SYSTEMEND\n");
    }
}

// Looks for, and returns, a system given a specific system
// type.
CHSEngSystem *CHSSystemArray::GetSystem(HSS_TYPE systype)
{
    CHSEngSystem *ptr;

    // Run down the list, looking for a match on the type
    for (ptr = m_SystemHead; ptr; ptr = ptr->GetNext())
    {
        if (ptr->GetType() == systype)
            return ptr;
    }

    // Not found.
    return NULL;
}

// Looks for, and returns, a system by name.  That name
// must only match the first n characters of the string
// passed in.  The first system name to at least match the
// characters in the given string is returned.
CHSEngSystem *CHSSystemArray::GetSystemByName(const HS_INT8 * strName)
{

    CHSEngSystem *ptr = NULL;
    int len = strlen(strName);

    for (ptr = m_SystemHead; ptr; ptr = ptr->GetNext())
    {
        if (!_strnicmp(strName, ptr->GetName(), len))
        {
            return ptr;
        }
    }
    return NULL;
}

// Allows a given system to be bumped up or down in the
// array.  If iDir is negative, the system moves down, otherwise
// up.
HS_BOOL8 CHSSystemArray::BumpSystem(CHSEngSystem * cSys, int iDir)
{
    CHSEngSystem *ptr;
    CHSEngSystem *ptr2;

    if (NULL == m_SystemHead)
    {
        return false;
    }

    if (0 == iDir)
    {
        return true;
    }
    else if (iDir < 0)
    {
        // Move the system down

        if (!cSys->GetNext())   // System is tail?
        {
            return false;
        }

        ptr = cSys->GetNext();
        if (!ptr->GetNext())    // Next system is tail?
        {
            m_SystemTail = cSys;
            cSys->SetNext(NULL);
        }
        else
        {
            ptr2 = ptr->GetNext();
            ptr2->SetPrev(cSys);
            cSys->SetNext(ptr2);
        }

        if (!cSys->GetPrev())   // System is head?
        {
            m_SystemHead = ptr;
            ptr->SetPrev(NULL);
        }
        else
        {
            ptr2 = cSys->GetPrev();
            ptr2->SetNext(ptr);
            ptr->SetPrev(ptr2);
        }

        // Swap the two systems.
        ptr->SetNext(cSys);
        cSys->SetPrev(ptr);
    }
    else if (iDir > 0)
    {
        // Move the system up

        if (!cSys->GetPrev())   // System is head?
            return false;

        ptr = cSys->GetPrev();
        if (!ptr->GetPrev())    // Prev system is head?
        {
            m_SystemHead = cSys;
            cSys->SetPrev(NULL);
        }
        else
        {
            ptr2 = ptr->GetPrev();
            ptr2->SetNext(cSys);
            cSys->SetPrev(ptr2);
        }

        if (!cSys->GetNext())   // System is tail?
        {
            m_SystemTail = ptr;
            ptr->SetNext(NULL);
        }
        else
        {
            ptr2 = cSys->GetNext();
            ptr2->SetPrev(ptr);
            ptr->SetNext(ptr2);
        }

        // Swap the two systems.
        ptr->SetPrev(cSys);
        cSys->SetNext(ptr);
    }

    // System was either moved, or iDir was 0.
    return true;
}

//
// Generic CHSEngSystem stuff
//
CHSEngSystem::CHSEngSystem():
m_eType(HSS_NOTYPE),
m_bVisible(true),
m_pcName(NULL),
m_fStress(0),
m_uiCurrentPower(0),
m_eDamageLevel(DMG_NONE),
m_puiOptimalPower(NULL),
m_puiTolerance(NULL),
m_ownerObj(NULL), m_parent(NULL), m_next(NULL), m_prev(NULL)
{
    // This is NULL, indicating a default system with default
    // values.
    m_parent = NULL;
    m_ownerObj = NULL;          // No one owns us right now
    m_next = m_prev = NULL;
}

// Destructor
CHSEngSystem::~CHSEngSystem()
{
    if (m_pcName)
    {
        delete[]m_pcName;
    }
    if (m_puiOptimalPower)
    {
        delete m_puiOptimalPower;
    }
    if (m_puiTolerance)
    {
        delete m_puiTolerance;
    }
}

CHSEngSystem *CHSEngSystem::CreateFromType(HSS_TYPE eType)
{
    // Determine type, and allocate system.
    switch (eType)
    {
    case HSS_ENGINES:
        return new CHSSysEngines;
    case HSS_SENSORS:
        return new CHSSysSensors;
    case HSS_COMPUTER:
        return new CHSSysComputer;
    case HSS_THRUSTERS:
        return new CHSSysThrusters;
    case HSS_COMM:
        return new CHSSysComm;
    case HSS_LIFE_SUPPORT:
        return new CHSSysLifeSupport;
    case HSS_REACTOR:
        return new CHSReactor;
    case HSS_FUEL_SYSTEM:
        return new CHSFuelSystem;
    case HSS_AFT_SHIELD:
    case HSS_PORT_SHIELD:
    case HSS_STARBOARD_SHIELD:
    case HSS_FORE_SHIELD:
        {
            CHSEngSystem *pSys = new CHSSysShield;
            pSys->SetType(eType);
            return pSys;
        }
    case HSS_JUMP_DRIVE:
        return new CHSJumpDrive;
    case HSS_CLOAK:
        return new CHSSysCloak;
    case HSS_TACHYON:
        return new CHSSysTach;
    case HSS_FICTIONAL:
        return new CHSFictional;
    case HSS_DAMCON:
        return new CHSDamCon;
    case HSS_JAMMER:
        return new CHSSysJammer;
    case HSS_TRACTOR:
        return new CHSSysTractor;
    case HSS_AUTOPILOT:
        return new CHSSysAutoPilot;
    case HSS_WARP_DRIVE:
        return new CHSWarpDrive;
    default:
        return NULL;
    }
}

// We can use this function to have the system force a power check
// to be the power levels are wacky.
void CHSEngSystem::CheckSystemPower()
{
    HS_UINT32 max;

    max = GetOptimalPower();

    // Only allow overloading to 150%.
    if (GetCurrentPower() > (max * 1.5))
    {
        SetCurrentPower((HS_UINT32) (max * 1.5));
        return;

    }
}

void CHSEngSystem::SetOwner(CHS3DObject * cOwner)
{
    m_ownerObj = cOwner;
}


// Returns the status of the system, which is usually Online
// or Offline.  If you overload this, don't return damage and
// stuff like that.  This is just a general system status
// string.
char *CHSEngSystem::GetStatus()
{
    if (GetCurrentPower() > 0)
        return "Online";
    else
        return "Offline";
}

// Call this function when you want the system to update itself
// during a cycle (usually each second).  It handles recharging,
// stress calculation, etc.
//
// If you're deriving a new system from CHSEngSystem, you _can_
// override this to handle your own cyclic stuff.
void CHSEngSystem::DoCycle()
{
    float fVal;
    int iOptPower;
    char tbuf[128];

    // If the system is inoperable .. do nothing.
    if (GetDamageLevel() == DMG_INOPERABLE)
        return;

    // Do Anything?
    if ((GetStress() == 0) && (GetCurrentPower() == 0))
    {
        return;
    }

    // Calculate stress
    iOptPower = GetOptimalPower();      // Optimal power including damage
    if (((int) GetCurrentPower() > iOptPower) || (GetStress() > 0))
    {
        // Calculate percent overload/underload
        fVal = (GetCurrentPower() - (float) iOptPower) / (float) iOptPower;
        fVal *= 100;

        // If overloaded, tolerance hinders stress.
        // If underloaded, tolerance reduces stress faster.
        if (fVal > 0)
        {
            fVal *= (float) (HS_STRESS_INCREASE / (float) GetTolerance());
        }
        else
        {
            // Make sure stress is reduced at least minimally
            // at 100% power allocation.
            if (!fVal)
            {
                fVal = -1;
            }
            fVal *= (float) (HS_STRESS_INCREASE * GetTolerance());
        }

        HS_FLOAT32 fNewStress = GetStress() + fVal;

        if (fNewStress > 100)
        {
            fNewStress = 100;
        }
        else if (fNewStress < 0)
        {
            fNewStress = 0;
        }

        SetStress(fNewStress);

        // If the system is currently stressed, check to see if system gets damaged. HAHAHAHA!
        if (fNewStress > 0)
        {
            int iDmgRoll;
            int iStressRoll;
            time_t tCurrentSecs;
            tCurrentSecs = time(NULL);

            // We check every 15 seconds
            if (!(tCurrentSecs % 15))
            {
                // Roll the dice for damage
                iDmgRoll = hsInterface.GetRandom(100);

                // Roll for stress .. mama needs a new car!
                iStressRoll = (int) fNewStress;
                iStressRoll = hsInterface.GetRandom(iStressRoll);

                // Did we damage?
                if (iDmgRoll < iStressRoll)
                {
                    // Ouch.  Hurt.
                    HS_DAMAGE prev;
                    HS_DAMAGE newlvl;

                    prev = GetDamageLevel();
                    newlvl = DoDamage();
                    if (newlvl != prev)
                    {
                        // Damage changed, so give some messages.
                        if (m_ownerObj && (m_ownerObj->GetType() == HST_SHIP))
                        {
                            CHSShip *cShip;
                            cShip = (CHSShip *) m_ownerObj;
                            sprintf_s(tbuf,
                                    "%s%s-%s A warning light flashes, indicating that the %s system has taken damage.",
                                    ANSI_HILITE, ANSI_YELLOW, ANSI_NORMAL,
                                    GetName());
                            cShip->HandleMessage(tbuf, MSG_ENGINEERING, NULL);

                            // Force a power check
                            CheckSystemPower();
                        }
                    }
                }
            }
        }
    }
}

// Allocates a new CHSEngSystem object, copies in some
// important information from this object, and gives it
// back to the calling function.
CHSEngSystem *CHSEngSystem::Dup()
{
    return CHSEngSystem::CreateFromType(m_eType);
}

//
// Linked list stuff with CHSEngSystem
CHSEngSystem *CHSEngSystem::GetNext()
{
    return m_next;
}

CHSEngSystem *CHSEngSystem::GetPrev()
{
    return m_prev;
}

void CHSEngSystem::SetNext(CHSEngSystem * cObj)
{
    m_next = cObj;
}

void CHSEngSystem::SetPrev(CHSEngSystem * cObj)
{
    m_prev = cObj;
}
//
// End linked list stuff
//

void CHSEngSystem::SetParentSystem(CHSEngSystem * cSys)
{
    m_parent = cSys;
}

//
// Value access functions
//

// Sets the name of the system.
void CHSEngSystem::SetName(const HS_INT8 * strName)
{
    // If strValue contains a null, clear our local setting
    if (!strName || !*strName)
    {
        if (m_pcName)
        {
            delete m_pcName;
            m_pcName = NULL;
        }

        return;
    }

    if (m_pcName)
    {
        delete[]m_pcName;
    }

    m_pcName = new char[strlen(strName) + 1];

    strcpy(m_pcName, strName);
    return;
}

// Does one level of damage to the system and returns the
// new damage level.
HS_DAMAGE CHSEngSystem::DoDamage()
{
    HS_DAMAGE lvl;

    // Get the current level
    lvl = GetDamageLevel();

    // If it's not inoperable, increase it; if inoperable, kill power
    switch (lvl)
    {
    case DMG_NONE:
        SetDamage(DMG_LIGHT);
        break;
    case DMG_LIGHT:
        SetDamage(DMG_MEDIUM);
        break;
    case DMG_MEDIUM:
        SetDamage(DMG_HEAVY);
        break;
    case DMG_HEAVY:
        SetDamage(DMG_INOPERABLE);
        break;
    case DMG_INOPERABLE:
        SetCurrentPower(0);
        break;
    }
    return GetDamageLevel();
}

// Reduces the damage on the system one level and returns the
// new damage level.
HS_DAMAGE CHSEngSystem::ReduceDamage()
{
    HS_DAMAGE lvl;

    // Get the current level.
    lvl = GetDamageLevel();

    switch (lvl)
    {
    case DMG_NONE:
        // Do nothing
        break;
    case DMG_LIGHT:
        SetDamage(DMG_NONE);
        break;
    case DMG_MEDIUM:
        SetDamage(DMG_LIGHT);
        break;
    case DMG_HEAVY:
        SetDamage(DMG_MEDIUM);
        break;
    case DMG_INOPERABLE:
        SetDamage(DMG_HEAVY);
        break;
    }

    return GetDamageLevel();
}

// Reduces the damage on the system to the specified level
// and returns the previous damage level.
HS_DAMAGE CHSEngSystem::ReduceDamage(HS_DAMAGE lvl)
{
    HS_DAMAGE prev;

    // Store previous level
    prev = GetDamageLevel();

    // Set current level .. we perform no error checking.
    SetDamage(lvl);

    return prev;
}

char *CHSEngSystem::GetName()
{
    int idx;

    if (!m_pcName)
    {
        // Do we have a parent?
        if (m_parent)
        {
            return m_parent->GetName();
        }
        else
        {
            // Search for the name in the global list
            for (idx = 0; hs_system_list[idx].name; idx++)
            {
                if (hs_system_list[idx].type == GetType())
                {
                    return hs_system_list[idx].name;
                }
            }
            return "No Name";
        }
    }
    else
    {
        return m_pcName;
    }

    // System not found in the list
    return "No Name";
}

// Returns the optimal (maximum, no damage) power level for
// the system.  The bAdjusted variable can be set to true to return
// the optimal power for the current damage level.  If this variable
// is false, then the optimal power not including damage is returned.
int CHSEngSystem::GetOptimalPower(HS_BOOL8 bAdjusted)
{
    int lvl;

    // Use some logic here.
    if (!m_puiOptimalPower)
    {
        // Go to the parent's setting?
        if (!m_parent)
        {
            // No.  We are the default values.
            lvl = 0;
        }
        else
        {
            // Yes, this system exists on a ship, so
            // find the default values on the parent.
            lvl = m_parent->GetOptimalPower();
        }
    }
    else
    {
        lvl = *m_puiOptimalPower;
    }

    // Calculate damage?
    if (bAdjusted)
    {
        lvl = (int) (lvl * (1 - (.25 * GetDamageLevel())));
    }

    return lvl;
}

HS_UINT32 CHSEngSystem::GetTolerance()
{
    // Use some logic here.
    if (!m_puiTolerance)
    {
        // Go to the parent's setting?
        if (!m_parent)
        {
            // No.  We are the default values.
            return 0;
        }
        else
        {
            // Yes, this system exists on a ship, so
            // find the default values on the parent.
            return m_parent->GetTolerance();
        }
    }
    else
    {
        return *m_puiTolerance;
    }
}

HS_FLOAT32 CHSEngSystem::GetStress()
{
    return m_fStress;
}

HSS_TYPE CHSEngSystem::GetType()
{
    return m_eType;
}
//
// End value access functions
//

void CHSEngSystem::CutPower(int level)
{
    // Do nothing, overridable.
}

void CHSEngSystem::PowerUp(int level)
{
    // Do nothing, overridable.
}

// Sets the current power level for the system.
HS_BOOL8 CHSEngSystem::SetCurrentPower(HS_UINT32 uiLevel)
{
    HS_UINT32 max;

    max = GetOptimalPower();

    // Don't allow levels below 0.  Level 0 is just deactivation.
    if (uiLevel < 0)
        return false;

    // Only allow overloading to 150%.
    if (uiLevel > (max * 1.5))
        return false;

    if (uiLevel < GetCurrentPower())
        CutPower(uiLevel);

    if (GetCurrentPower() == 0 && uiLevel > 0)
        PowerUp(uiLevel);

    m_uiCurrentPower = uiLevel;

    return true;
}

// Sets a specific attribute value for the system.  This
// also allows system default values to be overridden at the
// ship level.
HS_BOOL8 CHSEngSystem::SetAttributeValue(const HS_INT8 * pcAttrName,
                                         const HS_INT8 * strValue)
{
    int iVal;
    float fVal;

    // Match the name .. set the value
    if (!_stricmp(pcAttrName, "CURRENT POWER"))
    {
        iVal = atoi(strValue);
        if (iVal < 0)
            return false;

        SetCurrentPower(iVal);
        return true;
    }
    else if (!_stricmp(pcAttrName, "NAME"))
    {
        SetName(strValue);
        return true;
    }
    else if (!_stricmp(pcAttrName, "DAMAGE"))
    {
        iVal = atoi(strValue);
        SetDamage((HS_DAMAGE) iVal);
        return true;
    }
    else if (!_stricmp(pcAttrName, "OPTIMAL POWER"))
    {
        // If strValue contains a null, clear our local setting
        if (!*strValue)
        {
            if (m_puiOptimalPower)
            {
                delete m_puiOptimalPower;
                m_puiOptimalPower = NULL;
                return true;
            }

            return false;
        }

        iVal = atoi(strValue);
        if (iVal < 0)
            return false;

        SetOptimalPower(iVal);
        return true;
    }
    else if (!_stricmp(pcAttrName, "TOLERANCE"))
    {
        // If strValue contains a null, clear our local setting
        if (!*strValue)
        {
            if (m_puiTolerance)
            {
                delete m_puiTolerance;
                m_puiTolerance = NULL;
                return true;
            }

            return false;
        }
        iVal = atoi(strValue);
        if (iVal < 0)
            return false;

        SetTolerance(iVal);
        return true;
    }
    else if (!_stricmp(pcAttrName, "STRESS"))
    {
        fVal = (float) atof(strValue);
        if ((fVal < 0) || (fVal > 100))
            return false;

        SetStress(fVal);
        return true;
    }
    else if (!_stricmp(pcAttrName, "VISIBLE"))
    {
        SetVisible(atoi(strValue) == 0 ? false : true);
        return true;
    }
    else if (!_stricmp(pcAttrName, "PRIORITY"))
    {
        // If we get here, do nothing.
        return true;
    }

    return false;               // Attr name not matched
}

// Returns the value of the specified attribute or NULL if
// not a valid attribute.
HS_BOOL8
    CHSEngSystem::GetAttributeValue(const HS_INT8 * pcAttrName,
                                    CHSVariant & rvarReturnVal,
                                    HS_BOOL8 bAdjusted, HS_BOOL8 bLocalOnly)
{
    // Determine the attribute, and return the value.
    if (!_stricmp(pcAttrName, "CURRENT POWER"))
    {
        rvarReturnVal = m_uiCurrentPower;
        return true;
    }
    else if (!_stricmp(pcAttrName, "DAMAGE"))
    {
        rvarReturnVal = (HS_INT8) m_eDamageLevel;
        return true;
    }
    else if (!_stricmp(pcAttrName, "NAME"))
    {
        if (m_pcName)
        {
            rvarReturnVal = m_pcName;
        }
        else if (!bLocalOnly)
        {
            rvarReturnVal = GetName();
        }
        else
        {
            return false;
        }
        return true;
    }
    else if (!_stricmp(pcAttrName, "OPTIMAL POWER"))
    {
        if (m_puiOptimalPower)
        {
            rvarReturnVal = *m_puiOptimalPower;
        }
        else if (!bLocalOnly)
        {
            rvarReturnVal = GetOptimalPower();
        }
        else
        {
            return false;
        }
        return true;
    }
    else if (!_stricmp(pcAttrName, "STRESS"))
    {
        rvarReturnVal = m_fStress;
        return true;
    }
    else if (!_stricmp(pcAttrName, "TOLERANCE"))
    {
        if (m_puiTolerance)
        {
            rvarReturnVal = *m_puiTolerance;
        }
        else if (!bLocalOnly)
        {
            rvarReturnVal = GetTolerance();
        }
        else
        {
            return false;
        }
        return true;
    }
    else if (!_stricmp(pcAttrName, "VISIBLE"))
    {
        rvarReturnVal = m_bVisible;
        return true;
    }

    // Unknown attr.
    return false;
}


// Outputs system information to a file.  This could be
// the class database or another database of objects that
// contain systems.
void CHSEngSystem::SaveToFile(FILE * fp)
{
    // Output RAW values.  Do not call any Get() functions,
    // because those will return local or parent values.  We
    // ONLY want to output local values.
    if (m_pcName)
    {
        fprintf(fp, "NAME=%s\n", m_pcName);
    }
    if (m_puiTolerance)
    {
        fprintf(fp, "TOLERANCE=%d\n", *m_puiTolerance);
    }
    if (m_puiOptimalPower)
    {
        fprintf(fp, "OPTIMAL POWER=%d\n", *m_puiOptimalPower);
    }

    fprintf(fp, "STRESS=%.2f\n", m_fStress);
    fprintf(fp, "DAMAGE=%d\n", m_eDamageLevel);
    fprintf(fp, "CURRENT POWER=%d\n", m_uiCurrentPower);
    fprintf(fp, "VISIBLE=%d\n", m_bVisible);
}

void CHSEngSystem::GetAttributeList(CHSAttributeList & rlistAttributes)
{
    // Push all of our attributes into the list.
    rlistAttributes.push_back("NAME");
    rlistAttributes.push_back("CURRENT POWER");
    rlistAttributes.push_back("OPTIMAL POWER");
    rlistAttributes.push_back("DAMAGE");
    rlistAttributes.push_back("TOLERANCE");
    rlistAttributes.push_back("STRESS");
    rlistAttributes.push_back("VISIBLE");
}
