// -----------------------------------------------------------------------
//! $Id: hsmissile.cpp,v 1.29 2006/06/17 02:36:54 mark Exp $
// -----------------------------------------------------------------------


#include "pch.h"
#include <stdlib.h>
#include <cstring>

#include "hsmissile.h"
#include "hsuniverse.h"
#include "hsutils.h"
#include "hsinterface.h"
#include "hspace.h"
#include "hsconf.h"
#include "hsansi.h"
#include "hscloaking.h"
#include "hsflags.h"
#include "HSFileDatabase.h"

extern double d2cos_table[];
extern double d2sin_table[];

// Default constructor
CHSMTube::CHSMTube():m_time_to_reload(0),
m_loaded(false), m_reloading(false), m_missile_bay(NULL)
{
    m_reload_time = 1;
    m_strength = 1;
    m_turnrate = 1;
    m_speed = 1;
}

const HS_INT8 *CHSMTube::GetAttributeValue(const HS_INT8 * strName)
{
    static char rval[HS_BUF_64];

    *rval = '\0';
    if (!_strnicmp(strName, "RELOAD TIME", strlen(strName)))
    {
        _snprintf_s(rval, HS_BUF_64_CPY, "%d", m_reload_time);
    }
    else if (!_strnicmp(strName, "RANGE", strlen(strName)))
    {
        _snprintf_s(rval, HS_BUF_64_CPY, "%d", m_range);
    }
    else if (!_strnicmp(strName, "STRENGTH", strlen(strName)))
    {
        _snprintf_s(rval, HS_BUF_64_CPY, "%d", m_strength);
    }
    else if (!_strnicmp(strName, "TURNRATE", strlen(strName)))
    {
        _snprintf_s(rval, HS_BUF_64_CPY, "%d", m_turnrate);
    }
    else if (!_strnicmp(strName, "SPEED", strlen(strName)))
    {
        _snprintf_s(rval, HS_BUF_64_CPY, "%d", m_speed);
    }
    else
    {
        return CHSWeapon::GetAttributeValue(strName);
    }

    return rval;
}


HS_BOOL8 CHSMTube::WriteToDatabase(CHSFileDatabase * pDatabase)
{
    HS_INT8 cBuffer[HS_BUF_64];

    m_pWeaponData->WriteToDatabase(pDatabase);

    _snprintf_s(cBuffer, HS_BUF_64_CPY, "%d", m_reload_time);
    pDatabase->AddSectionAttribute("RELOAD TIME", cBuffer);

    _snprintf_s(cBuffer, HS_BUF_64_CPY, "%d", m_range);
    pDatabase->AddSectionAttribute("RANGE", cBuffer);

    _snprintf_s(cBuffer, HS_BUF_64_CPY, "%d", m_strength);
    pDatabase->AddSectionAttribute("STRENGTH", cBuffer);

    _snprintf_s(cBuffer, HS_BUF_64_CPY, "%d", m_turnrate);
    pDatabase->AddSectionAttribute("TURNRATE", cBuffer);

    _snprintf_s(cBuffer, HS_BUF_64_CPY, "%d", m_speed);
    pDatabase->AddSectionAttribute("SPEED", cBuffer);

    _snprintf_s(cBuffer, HS_BUF_64_CPY, "%3.0f", m_shield_percentage);
    pDatabase->AddSectionAttribute("SHIELD PERCENTAGE", cBuffer);

    _snprintf_s(cBuffer, HS_BUF_64_CPY, "%3.0f", m_hull_percentage);
    pDatabase->AddSectionAttribute("HULL PERCENTAGE", cBuffer);

    return true;
}

HS_BOOL8 CHSMTube::ReadFromDatabase(CHSFileDatabase * pDatabase)
{
    CHSFileDatabase::THSDBEntry tEntry;

    while (pDatabase->GetNextAttribute(tEntry))
    {
        SetAttributeValue(tEntry.strAttributeName.c_str(),
                          tEntry.strValue.c_str());
    }

    return true;
}


// Attempts to set the value of the given attribute.  If successful,true 
// is returned, else false.
HS_BOOL8 CHSMTube::SetAttributeValue(const HS_INT8 * strName,
                                     const HS_INT8 * strValue)
{
    int iVal = -1;

    // Try to match the name
    if (!_strnicmp(strName, "RELOAD TIME", strlen(strName)))
    {
        iVal = atoi(strValue);
        if (iVal < 0)
            iVal = 0;

        m_reload_time = iVal;
        return true;
    }
    else if (!_strnicmp(strName, "RANGE", strlen(strName)))
    {
        iVal = atoi(strValue);
        if (iVal < 1)
            iVal = 1;

        m_range = iVal;
        return true;
    }
    else if (!_strnicmp(strName, "STRENGTH", strlen(strName)))
    {
        iVal = atoi(strValue);
        if (iVal < 1)
            iVal = 1;

        m_strength = iVal;
        return true;
    }
    else if (!_strnicmp(strName, "TURNRATE", strlen(strName)))
    {
        iVal = atoi(strValue);
        if (iVal < 1)
            iVal = 1;

        m_turnrate = iVal;
        return true;
    }
    else if (!_strnicmp(strName, "SPEED", strlen(strName)))
    {
        iVal = atoi(strValue);
        if (iVal < 0)
            iVal = 0;

        m_speed = iVal;
        return true;
    }
    else
    {
        return CHSWeapon::SetAttributeValue(strName, strValue);
    }

    return false;
}

void CHSMTube::GetAttributeList(CHSAttributeList & rlistAttrs)
{
    m_pWeaponData->GetAttributeList(rlistAttrs);
}

// Returns an integer indicating the status of the tube.
HS_INT32 CHSMTube::GetStatusInt()
{
    // Loaded?
    if (m_loaded)
    {
        return 0;
    }
    else if (m_reloading)       // Reloading?
    {
        return m_time_to_reload;
    }

    // Must be empty and not loading
    return -1;
}

// Missiles require a target lock to fire
HS_BOOL8 CHSMTube::RequiresLock()
{
    return true;
}

HS_BOOL8 CHSMTube::CheckTargetSize(CHS3DObject * cObj)
{
    if (cObj->GetSize() < m_pWeaponData->GetMinimumTargetSize())
    {
        return false;
    }
    return true;
}

// Indicates whether the missile tube can fire upon the
// given type of CHS3DObject.
HS_BOOL8 CHSMTube::CanAttackObject(CHS3DObject * cObj)
{
    // Only attack ships
    if (cObj->GetType() == HST_SHIP || cObj->GetType() == HST_MISSILE)
    {
        return true;
    }

    return false;
}

HS_BOOL8 CHSMTube::IsReady()
{
    if (m_reloading)
        return false;

    if (!m_loaded)
        return false;

    return true;
}

// Handles the status integer sent to the weapon from an
// outside function.
void CHSMTube::SetStatus(HS_INT32 stat)
{
    // 0 indicates loaded.
    // -1 indicates not loaded.
    // Any other number indicates loading time
    if (!stat)
    {
        m_loaded = true;
    }
    else if (stat < 0)
    {
        m_loaded = false;
    }
    else
    {
        m_loaded = false;
        m_reloading = true;
        m_time_to_reload = stat;
    }
}

// Cyclic stuff for missile tubes
void CHSMTube::DoCycle()
{
    m_change = STAT_NOCHANGE;

    if (m_reloading)
        Reload();
}

// Missile types are configurable
HS_BOOL8 CHSMTube::Configurable()
{
    return true;
}

// Returns the status of the missile tube
const HS_INT8 *CHSMTube::GetStatus()
{
    static char tbuf[HS_BUF_64];

    if (m_loaded)
        return "Loaded";

    if (m_reloading)
    {
        _snprintf_s(tbuf, HS_BUF_64_CPY, "(%d) Loading", m_time_to_reload);
        return tbuf;
    }

    return "Empty";
}

void CHSMTube::SetData(CHSMissileData * pData)
{
    m_reload_time = pData->ReloadTime();
    m_range = pData->Range();
    m_turnrate = pData->Turnrate();
    m_strength = pData->Strength();
    m_speed = pData->Speed();
}

// Configures the missile tube to a given type of
// weapon.
HS_BOOL8 CHSMTube::Configure(HS_INT32 type)
{
    // Are we loaded?
    if (m_loaded)
    {
        return false;
    }

    // Are we reloading?
    if (m_reloading)
    {
        return false;
    }

    // Find the weapon in the global array.  It has to
    // be a missile.
    CHSWeaponData *pData = waWeapons.GetWeapon(type);
    if (!pData || (pData->WeaponClass() != WC_MISSILE))
    {
        return false;
    }

    // Set our data, which is the missile info we retrieved.
    SetData((CHSMissileData *) pData);

    return true;
}

// If the missile tube has no parent, it is not configured
// for a specific type of missile, so handle this for the
// name.
const HS_INT8 *CHSMTube::GetName()
{
    if (m_strName.length())
        return m_strName.c_str();
    else if (!m_pWeaponData)
    {
        return "Not Configured";
    }

    return static_cast < CHSMissileData * >(m_pWeaponData)->Name();
}

// Unload a missile from the tube
HS_BOOL8 CHSMTube::Unload()
{
    if (!m_loaded || !m_pWeaponData)
    {
        return false;
    }

    // If reloading, cancel it, and give the missile back
    // to storage.
    if (m_reloading)
    {
        m_time_to_reload = 0;
        m_reloading = false;
        m_loaded = false;

        // Give the missile back
        if (m_missile_bay)
        {
            m_missile_bay->GetMissile(m_pWeaponData->TypeID(), -1);
        }
        return true;
    }
    else
    {
        m_loaded = false;
        if (m_missile_bay)
        {
            m_missile_bay->GetMissile(m_pWeaponData->TypeID(), -1);
        }

        return true;
    }
}

void CHSMTube::AttackObject(CHS3DObject * cSource,
                            CHS3DObject * cTarget,
                            CHSConsole * cConsole,
                            HS_INT32 iSysType, HS_INT32 hit_flag)
{
    HS_DBREF dbUser;

    // Grab the user of the console.
    dbUser = hsInterface.ConsoleUser(cConsole->m_objnum);

    if (cSource->GetType() == HST_SHIP)
    {
        CHSSysCloak *cCloak;
        CHSShip *ptr;

        ptr = (CHSShip *) cSource;

        // Look for the cloaking device.
        cCloak = (CHSSysCloak *) ptr->GetEngSystem(HSS_CLOAK);
        if (cCloak)
            if (cCloak->GetEngaged())
            {
                if (dbUser != HSNOTHING)
                    hsStdError(dbUser, "You cannot fire while cloaked.");
                return;
            }
    }

    // Can we attack that object?
    if (!CanAttackObject(cTarget))
    {
        if (dbUser != HSNOTHING)
            hsStdError(dbUser,
                       "You cannot attack that target with that weapon.");
    }

    // Create a missile object, and put it in space
    CHSMissile *cMiss;
    cMiss = new CHSMissile;
    cMiss->SetUID(cSource->GetUID());

    if (hit_flag == 0)
    {
        cMiss->SetAutoMiss();
    }

    // Add it to the universe
    CHSUniverse *uDest;
    uDest = cMiss->GetUniverse();
    if (!uDest)
    {
        if (dbUser != HSNOTHING)
        {
            hsInterface.Notify(dbUser,
                               "Error finding a universe to put the missile in.");
        }
        delete cMiss;
        return;
    }
    uDest->AddObject(cMiss);

    // Set missile coords
    cMiss->SetX(cSource->GetX());
    cMiss->SetY(cSource->GetY());
    cMiss->SetZ(cSource->GetZ());

    // Set missile heading
    cMiss->SetHeading(cConsole->GetXYHeading(), cConsole->GetZHeading());

    // Set missile type
    cMiss->SetWeaponData(this);

    // Set source info
    cMiss->SetSourceConsole(cConsole);
    cMiss->SetSourceObject(cSource);
    cMiss->SetTargetObject(cTarget);


#ifdef PENNMUSH
    HS_DBREF obj_num = hsInterface.CreateNewGameObject();
#else
    HS_DBREF obj_num = hsInterface.CreateNewGameObject(TYPE_THING);
#endif

    if (hsInterface.ValidObject(obj_num))
    {
        cMiss->SetDbref(obj_num);

        // Try to set the name of the missile properly but default
        // just in case
        if (NULL == m_pWeaponData)
        {
            hsInterface.SetObjectName(obj_num, "Missile");
        }
        else
        {
            hsInterface.SetObjectName(obj_num,
              static_cast < CHSMissileData * >(m_pWeaponData)->Name());
        }

        hsInterface.MoveObject(obj_num, uDest->GetID());
        hsInterface.SetToggle(obj_num, THING_HSPACE_OBJECT);
        hsInterface.SetObjectOwner(obj_num, hsInterface.GetGodDbref());

        // Missile objects are temporary, clear them if the game
        // is restarted while the missile is still in existance
        hsInterface.AtrAdd(obj_num, "STARTUP", "@destroy me",
                           hsInterface.GetGodDbref());
    }
    else
    {
        // Set missile HS_DBREF to some very high number that's
        // probably not duplicated.
        cMiss->SetDbref(hsInterface.GetRandom(10000) + 28534);
        hs_log("CHSMTube::AttackObject() -- Deprecated fake dbref method \
                utilized.");
    }

    if (dbUser != HSNOTHING)
    {
        hsStdError(dbUser, "Missile launched.");
    }

    m_loaded = false;
}

HS_BOOL8 CHSMTube::Reload()
{
    if (!m_pWeaponData)
    {
        return false;
    }

    // Check to see if we're reloading.  If not, then begin it.
    if (!m_reloading)
    {
        // Are we already loaded?
        if (m_loaded)
        {
            return false;
        }

        // Pull a missile from storage .. or try to.
        if (!m_missile_bay)
        {
            return false;
        }

        if (!m_missile_bay->GetMissile(m_pWeaponData->TypeID()))
        {
            return false;
        }

        m_reloading = true;
        m_time_to_reload = GetReloadTime();
    }
    else
    {
        m_time_to_reload--;

        if (m_time_to_reload <= 0)
        {
            m_change = STAT_READY;
            m_reloading = false;
            m_loaded = true;
        }
    }
    return true;
}

// Returns attribute information in a string.
const HS_INT8 *CHSMTube::GetAttrInfo()
{
    static char rval[HS_BUF_128];

    _snprintf_s(rval, HS_BUF_128_CPY, "R: %-5d S: %-3d V : %d",
             GetRange(), GetStrength(), GetSpeed());

    return rval;
}

HS_BOOL8 CHSMTube::Loadable()
{
    return true;
}

HS_INT32 CHSMTube::Reloading()
{
    if (!m_reloading)
    {
        return 0;
    }

    return m_time_to_reload;
}

HS_UINT32 CHSMTube::GetReloadTime()
{
    return m_reload_time;
}

HS_UINT32 CHSMTube::GetRange()
{
    return m_range;
}

HS_UINT32 CHSMTube::GetStrength()
{
    return m_strength;
}

HS_UINT32 CHSMTube::GetSpeed()
{
    return m_speed;
}

HS_UINT32 CHSMTube::GetTurnRate()
{
    return m_turnrate;
}

// Sets the missile bay source for the tube, which is where
// the tube will draw its missiles from.
void CHSMTube::SetMissileBay(CHSMissileBay * mBay)
{
    m_missile_bay = mBay;
}


//
// CHSMissileBay stuff
//
CHSMissileBay::CHSMissileBay()
{
}

CHSMissileBay::~CHSMissileBay()
{
}

HS_BOOL8 CHSMissileBay::HasType(HS_UINT32 uiWeaponTypeID)
{
    CSTLMissileStorageMap::iterator iter = m_mapMissiles.find(uiWeaponTypeID);
    if (iter == m_mapMissiles.end())
    {
        return false;
    }
    return true;
}

// Tries to set a remaining missile storage value for
// the given type of missile.  If the type isn't 
// already shown in storage, the type is added IF the
// weapon type is valid.  If the remaining value is greater
// than the maximum storage value, then the maximum is raised.
HS_BOOL8 CHSMissileBay::SetRemaining(HS_UINT32 uiWeaponTypeID,
                                     HS_UINT32 uiNumRemaining)
{
    THSMissileStorage *pStorage = NULL;

    CSTLMissileStorageMap::iterator iter = m_mapMissiles.find(uiWeaponTypeID);
    if (iter == m_mapMissiles.end())
    {
        // We don't currently have any missiles of this type.

        // Does the type exist at all?
        CHSWeaponData *pData = waWeapons.GetWeapon(uiWeaponTypeID);
        if (!pData)
        {
            // This weapon type doesn't exist.
            return false;
        }

        // Add a new entry in the map for this weapon type.
        THSMissileStorage tStorage;

        tStorage.uiWeaponTypeID = uiWeaponTypeID;

        m_mapMissiles[uiWeaponTypeID] = tStorage;

        iter = m_mapMissiles.find(uiWeaponTypeID);
        if (iter == m_mapMissiles.end())
        {
            // Map failure.  Really never happens.
            return false;
        }
    }

    pStorage = &(iter->second);

    if (!pStorage)
    {
        // This shouldn't happen.
        return false;
    }

    pStorage->uiNumRemaining = uiNumRemaining;
    if (uiNumRemaining > pStorage->uiMaximum)
    {
        pStorage->uiMaximum = uiNumRemaining;
    }

    return true;
}

HS_BOOL8 CHSMissileBay::RemoveMissileType(HS_UINT32 uiWeaponTypeID)
{
    CSTLMissileStorageMap::iterator iter = m_mapMissiles.find(uiWeaponTypeID);
    if (iter != m_mapMissiles.end())
    {
        m_mapMissiles.erase(iter);
        return true;
    }

    return false;
}

// Tries to pull a missile from storage.  If the missile
// doesn't exist or the slot is empty, false is returned.
// Otherwise, a value of true indicates success, and the
// storage slot is decremented.
HS_BOOL8 CHSMissileBay::GetMissile(HS_UINT32 uiWeaponTypeID, HS_INT32 amt)
{
    CSTLMissileStorageMap::iterator iter = m_mapMissiles.find(uiWeaponTypeID);
    if (iter == m_mapMissiles.end())
    {
        // We don't have this munitions type.
        return false;
    }

    THSMissileStorage & rtStorage = iter->second;

    // Got enough missiles left?
    if (amt > 0)
    {
        if (amt > (int) rtStorage.uiNumRemaining)
        {
            return false;
        }
    }

    // If amt is negative, this adds missiles.  Otherwise, it removes.
    rtStorage.uiNumRemaining -= amt;

    return true;
}

// Saves missile bay information to the specified file stream.
void CHSMissileBay::SaveToFile(FILE * fp)
{

    if (NULL == fp)
    {
        hs_log("Invalid file pointer passed to CHSMissileBay::SaveToFile()");
        return;
    }

    // Write out our missile storage infos
    CSTLMissileStorageMap::iterator iter;
    for (iter = m_mapMissiles.begin(); iter != m_mapMissiles.end(); iter++)
    {
        THSMissileStorage & rtStorage = iter->second;

        fprintf(fp, "MISSILETYPE=%d\n", rtStorage.uiWeaponTypeID);
        fprintf(fp, "MISSILEMAX=%d\n", rtStorage.uiMaximum);
        fprintf(fp, "MISSILEREMAINING=%d\n", rtStorage.uiNumRemaining);
    }

    fprintf(fp, "MISSILEBAYEND\n");
}

// Loads missile bay information from the specified file stream.
void CHSMissileBay::LoadFromFile(FILE * fp)
{
    if (NULL == fp)
    {
        hs_log("NULL filepointer passed to CHSMissileBay::LoadFromFile()");
        return;
    }

    char tbuf[HS_BUF_256];
    char strKey[HS_BUF_128];
    char strValue[HS_BUF_128];
    char *ptr;
    HS_INT32 type;

    // Load from the file stream until end of file or, more
    // like, MISSILEBAYEND indicator.
    type = -1;
    while (fgets(tbuf, HS_BUF_256_CPY, fp))
    {
        // Strip newline chars
        if ((ptr = strchr(tbuf, '\n')) != NULL)
            *ptr = '\0';
        if ((ptr = strchr(tbuf, '\r')) != NULL)
            *ptr = '\0';

        // Extract key/value pairs
        extract(tbuf, strKey, 0, 1, '=');
        extract(tbuf, strValue, 1, 1, '=');

        // Check for key match
        if (!strncmp(strKey, "MISSILEBAYEND", 13))
        {
            // This is the end.
            return;
        }
        else if (!strncmp(strKey, "MISSILETYPE", 11))
        {
            type = atoi(strValue);
        }
        else if (!strncmp(strKey, "MISSILEMAX", 10))
        {
            // Set the maximum value for the type.
            // No need to error check.
            SetRemaining(type, atoi(strValue));
        }
        else if (!strncmp(strKey, "MISSILEREMAINING", 16))
        {
            // Set missiles remaining for the type.
            SetRemaining(type, atoi(strValue));
        }
    }
}

// Returns the number of missiles left for the given type of missile.
HS_UINT32 CHSMissileBay::GetMissilesLeft(HS_UINT32 uiWeaponTypeID)
{
    CSTLMissileStorageMap::iterator iter = m_mapMissiles.find(uiWeaponTypeID);
    if (iter == m_mapMissiles.end())
    {
        return 0;
    }

    return iter->second.uiNumRemaining;
}

// Reteurns the maximum storage value for the given type of missile.
HS_UINT32 CHSMissileBay::GetMaxMissiles(HS_UINT32 uiWeaponTypeID)
{
    CSTLMissileStorageMap::iterator iter = m_mapMissiles.find(uiWeaponTypeID);
    if (iter == m_mapMissiles.end())
    {
        return 0;
    }

    return iter->second.uiMaximum;
}

//
// CHSMissile implementation
//
CHSMissile::CHSMissile()
{
    m_pData = NULL;
    m_target = NULL;
    m_source_object = NULL;
    m_source_console = NULL;
    m_timeleft = -1;
    m_delete_me = false;
    m_target_hit = false;

    m_type = HST_MISSILE;

    CHSVector cvTmp(1, 0, 0);
    m_motion_vector = cvTmp;

    m_xyheading = m_zheading = 0;

    m_speed = 0;
    m_turnrate = 0;
    m_strength = 0;
    m_range = 0;
}

CHSMissile::~CHSMissile()
{
}

void CHSMissile::SetWeaponData(CHSMTube * pData)
{
    if (!pData)
    {
        m_pData = NULL;
        m_pTube = NULL;
        return;
    }

    m_pTube = pData;
    m_pData = (CHSMissileData *) pData->GetData();

    if (m_pTube)
    {
        SetName(m_pTube->GetName());
    }
    else
    {
        SetName("Missile");
    }

    m_speed = pData->GetSpeed();
    m_range = pData->GetRange();
    m_turnrate = pData->GetTurnRate();
    m_strength = pData->GetStrength();
}

HS_INT32 CHSMissile::GetSourceDbref()
{
    if (NULL != m_source_object)
    {
        return m_source_object->GetDbref();
    }

    return HSNOTHING;
}

HS_INT32 CHSMissile::GetTargetDbref()
{
    if (NULL != m_target)
    {
        return m_target->GetDbref();
    }

    return -1;
}

void CHSMissile::SetSourceObject(CHS3DObject * cSource)
{
    m_source_object = cSource;
}

void CHSMissile::SetSourceConsole(CHSConsole * cConsole)
{
    m_source_console = cConsole;
}

void CHSMissile::SetTargetObject(CHS3DObject * cObj)
{
    m_target = cObj;
}

void CHSMissile::DoCycle()
{
    // Do we need to delete ourselves?
    if (m_delete_me)
    {
        // Remove us from space
        CHSUniverse *cSource;
        cSource = GetUniverse();
        if (cSource)
        {
            cSource->RemoveObject(this);
        }

        // Purge the object representing the missile prior to deallocating
        // memory
        if (hsInterface.ValidObject(GetDbref()))
        {
            hsInterface.DestroyObject(GetDbref());
        }

        return;
    }

    // If we have no target or no parent, remove us from space.
    if (!m_target || !m_pData || !m_target->IsActive())
    {
        hs_log("CHSMissile::DoCycle() Missile Data Invalid - Removing Object.");
        m_delete_me = true;
        return;
    }

    // Do we know how much time is left until we die?
    if (m_timeleft < 0)
    {
        CalculateTimeLeft();
    }
    else
    {
        m_timeleft--;
    }

    // Missile depleted?
    if (0 == m_timeleft)
    {

        m_delete_me = true;
        return;
    }

    // Change the missile heading toward the target
    ChangeHeading();

    // Move us closer to the target.
    MoveTowardTarget();

    // The MoveTowardTarget() checks to see if the missile hits
    // so we just have to check our flag.
    if (m_target_hit)
    {

        // If we aren't designated to miss, apply damage
        if (m_specified_miss != true)
        {
            // BOOM!
            HitTarget();
        }

        m_delete_me = true;     // Delete next time around
    }
}

HS_INT32 CHSMissile::GetSpeed()
{
    return m_speed;
}

// Moves the missile along the current trajectory.
void CHSMissile::MoveTowardTarget()
{
    if (!m_pData)
    {
        return;
    }

    // Calculate per second speed
    HS_FLOAT32 fSpeed;

    fSpeed = m_speed * (.0002778f * HSCONF.cyc_interval);

    static double tX = 0.0, tY = 0.0, tZ = 0.0;
    double dist;

    if(NULL != m_target && HST_SHIP == m_target->GetType())
    {

        // get value from 0.0 - 1.0 where 1.0 is 100% visible
        float cloak_effect = 
            static_cast<CHSShip* >(m_target)->CloakingEffect() * 100;

        // If the random value is less than the cloaking effect,
        // the missile still sees the ship and can update its
        // coordinates, otherwise, leave them at the previous 
        // location
        if(hsInterface.GetRandom(100) <= (HS_UINT32) cloak_effect)
        {
            tX = m_target->GetX();
            tY = m_target->GetY();
            tZ = m_target->GetZ();
        }
    }
    else
    {
        tX = m_target->GetX();
        tY = m_target->GetY();
        tZ = m_target->GetZ();
    }
    
    dist = Dist3D(m_x, m_y, m_z, tX, tY, tZ);

    // Determine if speed is > distance, which might indicate
    // that the missile is within hitting range.  If this is
    // true, move the missile just the distance to the target
    // and see if the coordinates are relatively close.  It's
    // possible that the missile is close, but the target is
    // evading, in which case we just have to move the missile
    // it's regular distance.
    if (fSpeed > dist)
    {
        // Move just the distance
        double n_x, n_y, n_z;
        n_x = m_x + m_motion_vector.i() * dist;
        n_y = m_y + m_motion_vector.j() * dist;
        n_z = m_z + m_motion_vector.k() * dist;

        // Is new distance within 1 unit?
        dist = Dist3D(n_x, n_y, n_z, m_target->GetX(), m_target->GetY(), 
                m_target->GetZ());
        if (dist <= 1)
        {
            m_target_hit = true;
            return;
        }
    }

    // At this point we know we didn't hit, so just move regularly.
    m_x += m_motion_vector.i() * fSpeed;
    m_y += m_motion_vector.j() * fSpeed;
    m_z += m_motion_vector.k() * fSpeed;
}

// Changes the heading of the missile to head toward the target.
void CHSMissile::ChangeHeading()
{
    HS_BOOL8 bChange;           // Indicates if any turning has occurred;

    if (!m_pData)
    {
        return;
    }

    // Calculate X, Y, and Z difference vector
    static double tX = 0.0, tY = 0.0, tZ = 0.0;

    if(NULL != m_target && HST_SHIP == m_target->GetType())
    {

        // get value from 0.0 - 1.0 where 1.0 is 100% visible
        float cloak_effect = 
            static_cast<CHSShip* >(m_target)->CloakingEffect() * 100;

        // If the random value is less than the cloaking effect,
        // the missile still sees the ship and can update its
        // coordinates, otherwise, leave them at the previous 
        // location
        if(hsInterface.GetRandom(100) <= (HS_UINT32) cloak_effect)
        {
            tX = m_target->GetX();
            tY = m_target->GetY();
            tZ = m_target->GetZ();
        }
    }
    else
    {
        tX = m_target->GetX();
        tY = m_target->GetY();
        tZ = m_target->GetZ();
    }

    HS_INT32 xyang = XYAngle(m_x, m_y, tX, tY);
    HS_INT32 zang = ZAngle(m_x, m_y, m_z, tX, tY, tZ);

    // Get the turn rate
    HS_INT32 iTurnRate = m_turnrate;

    bChange = false;

    // Check for change in zheading
    if (zang != m_zheading)
    {
        if (zang > m_zheading)
        {
            if ((zang - m_zheading) > iTurnRate)
                m_zheading += iTurnRate;
            else
                m_zheading = zang;
        }
        else
        {
            if ((m_zheading - zang) > iTurnRate)
                m_zheading -= iTurnRate;
            else
                m_zheading = zang;
        }
        bChange = true;
    }

    // Now handle any changes in the XY plane.
    HS_INT32 iDiff;
    if (xyang != m_xyheading)
    {
        if (abs(m_xyheading - xyang) < 180)
        {
            if (abs(m_xyheading - xyang) < iTurnRate)
                m_xyheading = xyang;
            else if (m_xyheading > xyang)
            {
                m_xyheading -= iTurnRate;

                if (m_xyheading < 0)
                    m_xyheading += 360;
            }
            else
            {
                m_xyheading += iTurnRate;

                if (m_xyheading > 359)
                    m_xyheading -= 360;
            }
        }
        else if (((360 - xyang) + m_xyheading) < 180)
        {
            iDiff = (360 - xyang) + m_xyheading;
            if (iDiff < iTurnRate)
            {
                m_xyheading = xyang;
            }
            else
            {
                m_xyheading -= iTurnRate;

                if (m_xyheading < 0)
                    m_xyheading += 360;
            }
        }
        else if (((360 - m_xyheading) + xyang) < 180)
        {
            iDiff = (360 - m_xyheading) + xyang;
            if (iDiff < iTurnRate)
            {
                m_xyheading = xyang;
            }
            else
            {
                m_xyheading += iTurnRate;

                if (m_xyheading > 359)
                    m_xyheading -= 360;
            }
        }
        else                    // This should never be true, but just in case.
        {
            m_xyheading += iTurnRate;

            if (m_xyheading > 359)
                m_xyheading -= 360;
        }
        bChange = true;
    }

    // Check to see if we need to recompute trajectory.
    if (bChange)
    {
        zang = m_zheading;
        if (zang < 0)
            zang += 360;

        CHSVector tvec(d2sin_table[m_xyheading] * d2cos_table[zang],
                       d2cos_table[m_xyheading] * d2cos_table[zang],
                       d2sin_table[zang]);
        m_motion_vector = tvec;
    }
}

// Sets the current heading of the missile, which is usually
// used when the missile is launched toward a target.
void CHSMissile::SetHeading(HS_INT32 xy, HS_INT32 z)
{
    m_xyheading = xy;
    m_zheading = z;
}

// Handles hitting the target and contacting the target
// object to negotiate damage.
void CHSMissile::HitTarget()
{
    if (!m_pData)
    {
        return;
    }

    // Tell the target it's damaged
    m_target->HandleDamage(m_source_object, m_pData, m_strength,
                           m_source_console, HSS_NOTYPE);
}

void CHSMissile::CalculateTimeLeft()
{
    HS_FLOAT32 fSpeed;
    HS_UINT32 uiRange;

    if (!m_pData)
    {
        return;
    }

    // Time until our death is calculated by
    // speed and range.
    fSpeed = m_speed;

    // Sanity check to avoid division by 0 later
    if(fSpeed < 1.0)
    {
        fSpeed = 1.0;
    }

    uiRange = m_range;

    // Bring speed down to per second level.
    fSpeed = fSpeed * .0002778f * HSCONF.cyc_interval;

    m_timeleft = (HS_UINT32) ((HS_FLOAT32) uiRange / fSpeed);

    // Missile must exist for at least 2 iterations: 
    // Then they get 1 cycle to interact and then are removed the next
    if (m_timeleft < 2)
    {
        m_timeleft = 2;
    }

}

char CHSMissile::GetObjectCharacter()
{
    return 'x';
}

char *CHSMissile::GetObjectColor()
{
    static char tbuf[HS_BUF_64];

    _snprintf_s(tbuf, HS_BUF_64_CPY, "%s%s", ANSI_HILITE, ANSI_YELLOW);
    return tbuf;
}

void CHSMissile::HandleDamage(HS_FLOAT64 x, HS_FLOAT64 y,
                              HS_FLOAT64 z, HS_INT32 strength)
{
    // Any incoming damage destroys the missile
    m_delete_me = true;

}

void CHSMissile::HandleDamage(CHS3DObject * cSource,
                              CHSWeaponData * cWeapon,
                              HS_INT32 strength,
                              CHSConsole * cConsole, HS_INT32 iSysType)
{
    HS_INT32 player = hsInterface.ConsoleUser(cConsole->m_objnum);

    // Any hit destroys the missile.
    hsStdError(player, "Your shot explodes the target missile!");
    
    // Delete me 
    m_delete_me = true;
}

void CHSMissile::GetAttributeList(CHSAttributeList & rlistAttributes)
{
    rlistAttributes.push_back("TARGET");
    rlistAttributes.push_back("SOURCE");
    rlistAttributes.push_back("SPEED");
    rlistAttributes.push_back("STRENGTH");
    rlistAttributes.push_back("TURNRATE");
    rlistAttributes.push_back("RANGE");
}

HS_INT8 *CHSMissile::GetAttributeValue(HS_INT8 * strName)
{
    static char rval[HS_BUF_64];
    *rval = 0;

    if (!_strnicmp(strName, "TARGET", strlen(strName)))
    {
        _snprintf_s(rval, HS_BUF_64_CPY, "#%d", GetTargetDbref());
    }
    else if (!_strnicmp(strName, "SOURCE", strlen(strName)))
    {
        _snprintf_s(rval, HS_BUF_64_CPY, "#%d", GetSourceDbref());
    }
    else if (!_strnicmp(strName, "SPEED", strlen(strName)))
    {
        _snprintf_s(rval, HS_BUF_64_CPY, "%d", m_speed);
    }
    else if (!_strnicmp(strName, "STRENGTH", strlen(strName)))
    {
        _snprintf_s(rval, HS_BUF_64_CPY, "%d", m_strength);
    }
    else if (!_strnicmp(strName, "TURNRATE", strlen(strName)))
    {
        _snprintf_s(rval, HS_BUF_64_CPY, "#%d", m_turnrate);
    }
    else if (!_strnicmp(strName, "RANGE", strlen(strName)))
    {
        _snprintf_s(rval, HS_BUF_64_CPY, "#%d", m_range);
    }
    else
    {
        return CHS3DObject::GetAttributeValue(strName);
    }

    return rval;
}

// Attempts to set the value of the given attribute.  If successful,true 
// is returned, else false.
HS_BOOL8 CHSMissile::SetAttributeValue(HS_INT8 * strName, HS_INT8 * strValue)
{
    int iVal = -1;

    if (!_strnicmp(strName, "STRENGTH", strlen(strName)))
    {
        iVal = atoi(strValue);
        if (iVal < 1)
            iVal = 1;

        m_strength = iVal;
        return true;
    }
    else if (!_strnicmp(strName, "TURNRATE", strlen(strName)))
    {
        iVal = atoi(strValue);
        if (iVal < 1)
            iVal = 1;

        m_turnrate = iVal;
        return true;
    }
    else if (!_strnicmp(strName, "SPEED", strlen(strName)))
    {
        iVal = atoi(strValue);
        if (iVal < 0)
            iVal = 0;

        m_speed = iVal;
        return true;
    }
    else if (!_strnicmp(strName, "RANGE", strlen(strName)))
    {
        iVal = atoi(strValue);
        if (iVal < 0)
            iVal = 0;

        m_range = iVal;
        return true;
    }
    else
    {
        return CHS3DObject::SetAttributeValue(strName, strValue);
    }

    return false;
}
