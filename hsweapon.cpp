// -----------------------------------------------------------------------
//! $Id: hsweapon.cpp,v 1.23 2006/06/17 02:36:26 mark Exp $
// -----------------------------------------------------------------------

#include "pch.h"
#include <math.h>
#ifdef I_STDLIB
#include <stdlib.h>
#endif

#ifdef WIN32
#include <ctype.h>
#include <cstring>
#else
#include <strings.h>
#include <ctype.h>
#endif

#include "hstypes.h"
#include "hsuniverse.h"
#include "hsobjects.h"
#include "hsinterface.h"
#include "hspace.h"
#include "hsdb.h"
#include "hsansi.h"
#include "hsutils.h"
#include "hsmissile.h"
#include "hssensors.h"
#include "HSFileDatabase.h"
#include "hscloaking.h"
#include "hsweapon.h"

CHSWeaponDB waWeapons;          // One instance of this .. right here.

const HS_UINT32 HSCONST_DEFAULT_MAX_WEAPONIDS = 64;

const char *g_pszWeaponTypeNames[] = {
    "Laser",
    "Missile"
};


// Returns the request attribute value, or NULL if the attribute
// name is not valid.
const HS_INT8 *CHSWeaponData::GetAttributeValue(const HS_INT8 *
                                                pcAttributeName)
{
    static char rval[HS_BUF_64];

    *rval = '\0';
    if (!_strnicmp(pcAttributeName, "NAME", strlen(pcAttributeName)))
    {
        return m_strName.c_str();
    }
    else if (!_strnicmp(pcAttributeName, "TYPE", strlen(pcAttributeName)))
    {
        _snprintf_s(rval, HS_BUF_64_CPY, "%d", m_uiWeaponTypeID);
    }
    else if (!_strnicmp(pcAttributeName, "WEAPON CLASS",
                          strlen(pcAttributeName)))
    {
        strncpy_s(rval, GetWeaponClassName(), HS_BUF_64_CPY);
    }
    else if (!_strnicmp(pcAttributeName, "MIN TARGET SIZE",
                          strlen(pcAttributeName)))
    {
        _snprintf_s(rval, HS_BUF_64_CPY, "%d", m_mintargetsize);
    }
    else if (!_strnicmp(pcAttributeName, "SHIELD PERCENTAGE",
                          strlen(pcAttributeName)))
    {
        _snprintf_s(rval, HS_BUF_64_CPY, "%f", m_shield_percentage);
    }
    else if (!_strnicmp(pcAttributeName, "HULL PERCENTAGE",
                          strlen(pcAttributeName)))
    {
        _snprintf_s(rval, HS_BUF_64_CPY, "%f", m_hull_percentage);
    }
    else
    {
        return NULL;
    }

    return rval;
}

HS_BOOL8 CHSWeaponData::WriteToDatabase(CHSFileDatabase * pDatabase)
{
    pDatabase->AddSectionAttribute("NAME", m_strName.c_str());
    pDatabase->AddSectionAttribute("MIN TARGET SIZE",
                                   hsInterface.HSPrintf("%d",
                                                        m_mintargetsize));

    return true;
}

HS_BOOL8 CHSWeaponData::ReadFromDatabase(CHSFileDatabase * pDatabase)
{
    return true;
}

void CHSWeaponData::GetAttributeList(CHSAttributeList & rlistAttributes)
{
    rlistAttributes.push_back("WEAPON CLASS");
    rlistAttributes.push_back("TYPE");
    rlistAttributes.push_back("NAME");
    rlistAttributes.push_back("MIN TARGET SIZE");
    rlistAttributes.push_back("SHIELD PERCENTAGE");
    rlistAttributes.push_back("HULL PERCENTAGE");
}

// Attempts to set the value of the given attribute.  If successful,true
// is returned, else false.
HS_BOOL8 CHSWeaponData::SetAttributeValue(const HS_INT8 * strName,
                                          const HS_INT8 * strValue)
{
    // Try to match the name
    if (!_strnicmp(strName, "NAME", strlen(strName)))
    {
        m_strName = strValue;
        return true;
    }
    else if (!_strnicmp(strName, "MIN TARGET SIZE", strlen(strName)))
    {
        m_mintargetsize = atoi(strValue);
        return true;
    }
    else if (!_strnicmp(strName, "SHIELD PERCENTAGE", strlen(strName)))
    {
        m_shield_percentage = strtod(strValue, NULL);
        return true;
    }
    else if (!_strnicmp(strName, "HULL PERCENTAGE", strlen(strName)))
    {
        m_hull_percentage = strtod(strValue, NULL);
        return true;
    }

    return false;
}

CHSLaserData::CHSLaserData(void)
{
    m_eWeaponClass = WC_LASER;
    m_regen_time = 1;
    m_range = 1;
    m_strength = 1;
    m_accuracy = 1;
    m_powerusage = 1;
    m_nohull = 0;
    m_mintargetsize = 0;
    m_shield_percentage = 100;
    m_hull_percentage = 100;
}

// Returns the request attribute value, or NULL if the attribute
// name is not valid.
const HS_INT8 *CHSLaserData::GetAttributeValue(const HS_INT8 * strName)
{
    static char rval[HS_BUF_64];

    *rval = '\0';
    if (!_strnicmp(strName, "REGEN TIME", strlen(strName)))
    {
        _snprintf_s(rval, HS_BUF_64_CPY, "%d", m_regen_time);
    }
    else if (!_strnicmp(strName, "RANGE", strlen(strName)))
    {
        _snprintf_s(rval, HS_BUF_64_CPY, "%d", m_range);
    }
    else if (!_strnicmp(strName, "STRENGTH", strlen(strName)))
    {
        _snprintf_s(rval, HS_BUF_64_CPY, "%d", m_strength);
    }
    else if (!_strnicmp(strName, "ACCURACY", strlen(strName)))
    {
        _snprintf_s(rval, HS_BUF_64_CPY, "%d", m_accuracy);
    }
    else if (!_strnicmp(strName, "POWER", strlen(strName)))
    {
        _snprintf_s(rval, HS_BUF_64_CPY, "%d", m_powerusage);
    }
    else if (!_strnicmp(strName, "NOHULL", strlen(strName)))
    {
        _snprintf_s(rval, HS_BUF_64_CPY, "%d", m_nohull);
    }
    else
    {
        return CHSWeaponData::GetAttributeValue(strName);
    }

    return rval;
}


HS_BOOL8 CHSLaserData::WriteToDatabase(CHSFileDatabase * pDatabase)
{
    HS_INT8 cBuffer[HS_BUF_64];

    CHSWeaponData::WriteToDatabase(pDatabase);

    _snprintf_s(cBuffer, HS_BUF_64_CPY, "%d", m_regen_time);
    pDatabase->AddSectionAttribute("REGEN TIME", cBuffer);

    _snprintf_s(cBuffer, HS_BUF_64_CPY, "%d", m_range);
    pDatabase->AddSectionAttribute("RANGE", cBuffer);

    _snprintf_s(cBuffer, HS_BUF_64_CPY, "%d", m_strength);
    pDatabase->AddSectionAttribute("STRENGTH", cBuffer);

    _snprintf_s(cBuffer, HS_BUF_64_CPY, "%d", m_accuracy);
    pDatabase->AddSectionAttribute("ACCURACY", cBuffer);

    _snprintf_s(cBuffer, HS_BUF_64_CPY, "%d", m_powerusage);
    pDatabase->AddSectionAttribute("POWER", cBuffer);

    _snprintf_s(cBuffer, HS_BUF_64_CPY, "%d", m_nohull);
    pDatabase->AddSectionAttribute("NOHULL", cBuffer);

    return true;
}

HS_BOOL8 CHSLaserData::ReadFromDatabase(CHSFileDatabase * pDatabase)
{
    CHSFileDatabase::THSDBEntry tEntry;

    while (pDatabase->GetNextAttribute(tEntry))
    {
        SetAttributeValue(tEntry.strAttributeName.c_str(),
                          tEntry.strValue.c_str());
    }

    return true;
}


void CHSLaserData::GetAttributeList(CHSAttributeList & rlistAttributes)
{
    CHSWeaponData::GetAttributeList(rlistAttributes);

    rlistAttributes.push_back("REGEN TIME");
    rlistAttributes.push_back("RANGE");
    rlistAttributes.push_back("STRENGTH");
    rlistAttributes.push_back("ACCURACY");
    rlistAttributes.push_back("POWER");
    rlistAttributes.push_back("NOHULL");
}

// Attempts to set the value of the given attribute.  If successful,true
// is returned, else false.
HS_BOOL8 CHSLaserData::SetAttributeValue(const HS_INT8 * strName,
                                         const HS_INT8 * strValue)
{
    int iVal = -1;

    // Try to match the name
    if (!_strnicmp(strName, "REGEN TIME", strlen(strName)))
    {
        iVal = atoi(strValue);
        if (iVal < 0)
            iVal = 0;

        m_regen_time = iVal;
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
    else if (!_strnicmp(strName, "ACCURACY", strlen(strName)))
    {
        iVal = atoi(strValue);
        if (iVal < 1)
            iVal = 1;

        m_accuracy = iVal;
        return true;
    }
    else if (!_strnicmp(strName, "POWER", strlen(strName)))
    {
        iVal = atoi(strValue);
        if (iVal < 0)
            iVal = 0;

        m_powerusage = iVal;
        return true;
    }
    else if (!_strnicmp(strName, "NOHULL", strlen(strName)))
    {
        iVal = atoi(strValue);
        m_nohull = iVal == 0 ? false : true;
        return true;
    }

    return CHSWeaponData::SetAttributeValue(strName, strValue);
}

CHSMissileData::CHSMissileData(void)
{
    m_eWeaponClass = WC_MISSILE;
    m_reload_time = 1;
    m_range = 1;
    m_strength = 1;
    m_turnrate = 0;
    m_speed = 1;
    m_shield_percentage = 100;
    m_hull_percentage = 100;
}

// Returns the request attribute value, or NULL if the attribute
// name is not valid.
const HS_INT8 *CHSMissileData::GetAttributeValue(const HS_INT8 * strName)
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
    else if (!_strnicmp(strName, "TURN RATE", strlen(strName)))
    {
        _snprintf_s(rval, HS_BUF_64_CPY, "%d", m_turnrate);
    }
    else if (!_strnicmp(strName, "SPEED", strlen(strName)))
    {
        _snprintf_s(rval, HS_BUF_64_CPY, "%d", m_speed);
    }
    else
    {
        return CHSWeaponData::GetAttributeValue(strName);
    }

    return rval;
}

void CHSMissileData::GetAttributeList(CHSAttributeList & rlistAttributes)
{
    CHSWeaponData::GetAttributeList(rlistAttributes);

    rlistAttributes.push_back("RELOAD TIME");
    rlistAttributes.push_back("RANGE");
    rlistAttributes.push_back("STRENGTH");
    rlistAttributes.push_back("TURN RATE");
    rlistAttributes.push_back("SPEED");
}

HS_BOOL8 CHSMissileData::WriteToDatabase(CHSFileDatabase * pDatabase)
{
    HS_INT8 cBuffer[32];

    CHSWeaponData::WriteToDatabase(pDatabase);

    _snprintf_s(cBuffer, 31, "%d", m_reload_time);
    pDatabase->AddSectionAttribute("RELOAD TIME", cBuffer);

    _snprintf_s(cBuffer, 31, "%d", m_range);
    pDatabase->AddSectionAttribute("RANGE", cBuffer);

    _snprintf_s(cBuffer, 31, "%d", m_strength);
    pDatabase->AddSectionAttribute("STRENGTH", cBuffer);

    _snprintf_s(cBuffer, 31, "%d", m_turnrate);
    pDatabase->AddSectionAttribute("TURN RATE", cBuffer);

    _snprintf_s(cBuffer, 31, "%d", m_speed);
    pDatabase->AddSectionAttribute("SPEED", cBuffer);

    return true;
}

HS_BOOL8 CHSMissileData::ReadFromDatabase(CHSFileDatabase * pDatabase)
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
HS_BOOL8 CHSMissileData::SetAttributeValue(const HS_INT8 * strName,
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
    else if (!_strnicmp(strName, "TURN RATE", strlen(strName)))
    {
        iVal = atoi(strValue);
        m_turnrate = iVal;
        return true;
    }
    else if (!_strnicmp(strName, "SPEED", strlen(strName)))
    {
        iVal = atoi(strValue);
        if (iVal < 1)
            iVal = 1;

        m_speed = iVal;
        return true;
    }

    return CHSWeaponData::SetAttributeValue(strName, strValue);
}

CHSWeaponDB::CHSWeaponDB()
{
}

CHSWeaponDB::~CHSWeaponDB()
{
    CSTLWeaponDataMap::iterator iter;
    for (iter = m_mapWeaponData.begin(); iter != m_mapWeaponData.end();
         iter++)
    {
        CHSWeaponData *pData = iter->second;
        pData->Release();
    }
}

HS_BOOL8 CHSWeaponDB::AddWeapon(CHSWeaponData * pData, HS_BOOL8 bAssignID)
{
    if (!pData)
    {
        return false;
    }

    if (bAssignID)
    {
        pData->TypeID(GetNextWeaponID());
    }

    LockWeaponID(pData->TypeID());
    m_mapWeaponData[pData->TypeID()] = pData;

    return true;
}

HS_UINT32 CHSWeaponDB::GetNextWeaponID()
{
    // Do we have any weapon IDs at all?
    if (!m_pucWeaponIDs)
    {
        HS_UINT32 uiNumBytes = 1 + (HSCONST_DEFAULT_MAX_WEAPONIDS / 8);

        m_pucWeaponIDs = new HS_UINT8[uiNumBytes];
        memset(m_pucWeaponIDs, 0, uiNumBytes);
        m_uiMaxWeaponID = uiNumBytes * 8 - 1;
    }

    // Find the first bit in the weapon IDs array that is 0.
    for (HS_UINT32 i = 0; i <= m_uiMaxWeaponID; i++)
    {
        HS_UINT32 uiByte = i / 8;

        if (m_pucWeaponIDs[uiByte] & (1 << (i % 8)))
        {
            // This bit is turned on.  It's not available.
            continue;
        }
        else
        {
            // Here's a free ID.
            return i;
        }
    }

    // There are no more free weapon IDs in our array.
    // Double it, and get the next ID.
    HS_UINT32 uiNewSize = 1 + (m_uiMaxWeaponID * 2) / 8;
    HS_UINT32 uiPrevSize = (m_uiMaxWeaponID + 1) / 8;
    HS_UINT8 *pucNewArray = new HS_UINT8[uiNewSize];
    memset(pucNewArray, 0, uiNewSize);
    memcpy(pucNewArray, m_pucWeaponIDs, uiPrevSize);
    delete[]m_pucWeaponIDs;
    m_pucWeaponIDs = pucNewArray;

    m_uiMaxWeaponID = uiNewSize * 8 - 1;

    return GetNextWeaponID();
}

void CHSWeaponDB::LockWeaponID(HS_UINT32 uiWeaponID)
{
    // Is this weapon ID bigger than our array?
    if ((m_uiMaxWeaponID == 0) || (uiWeaponID > m_uiMaxWeaponID))
    {
#ifndef WIN32
        HS_UINT32 uiSize = 1 +
            (std::max(uiWeaponID, HSCONST_DEFAULT_MAX_WEAPONIDS) / 8);
#else
        HS_UINT32 uiSize = 1 +
            ((std::max)(uiWeaponID, HSCONST_DEFAULT_MAX_WEAPONIDS) / 8);
#endif

        HS_UINT8 *pucNewArray;
        pucNewArray = new HS_UINT8[uiSize];
        memset(pucNewArray, 0, uiSize);

        // Should we copy the previous settings?
        if (m_pucWeaponIDs)
        {
            HS_UINT32 uiNumPrevBytes = (m_uiMaxWeaponID + 1) / 8;

            memcpy(pucNewArray, m_pucWeaponIDs, uiNumPrevBytes);

            delete[]m_pucWeaponIDs;
        }

        m_pucWeaponIDs = pucNewArray;

        m_uiMaxWeaponID = uiSize * 8 - 1;
    }

    // Flip the bit to 1.
    HS_UINT32 uiByte = uiWeaponID / 8;
    m_pucWeaponIDs[uiByte] |= (1 << (uiWeaponID % 8));
}

void CHSWeaponDB::UnlockWeaponID(HS_UINT32 uiWeaponID)
{
    if (uiWeaponID > m_uiMaxWeaponID)
    {
        // This is an invalid operation, so we don't care.
        return;
    }

    // Flip the bit to 0.
    HS_UINT32 uiByte = uiWeaponID / 8;
    m_pucWeaponIDs[uiByte] &= ~(1 << (uiWeaponID % 8));
}

// Prints info to a given player for all of the weapons
// in the database.
void CHSWeaponDB::PrintInfo(HS_INT32 player)
{
    if (m_mapWeaponData.empty())
    {
        hsInterface.Notify(player, "No weapons currently loaded.");
        return;
    }

    hsInterface.Notify(player, "[ID] Weapon Name                     Type");

    CSTLWeaponDataMap::iterator iter;
    for (iter = m_mapWeaponData.begin(); iter != m_mapWeaponData.end();
         iter++)
    {
        CHSWeaponData *pWeaponData = iter->second;

        hsInterface.Notify(player,
                           hsInterface.HSPrintf("[%2d] %-32s%s",
                                                pWeaponData->TypeID(),
                                                pWeaponData->Name(),
                                                g_pszWeaponTypeNames
                                                [pWeaponData->
                                                 WeaponClass()]));
    }
}

// Gets the CSHDBWeapon structure from the WeaponDB.
CHSWeaponData *CHSWeaponDB::GetWeapon(HS_UINT32 uiWeaponID)
{
    CSTLWeaponDataMap::iterator iter = m_mapWeaponData.find(uiWeaponID);

    if (iter == m_mapWeaponData.end())
    {
        return NULL;
    }
    return iter->second;
}

// Load the weapons database file, creating all of the weapons from it.
HS_BOOL8 CHSWeaponDB::LoadFromFile(char *lpstrPath)
{
    // Determine the type of database in use.
    FILE *fp;
    char tbuf[HS_BUF_256];

    _snprintf_s(tbuf, HS_BUF_256_CPY, "LOADING: %s", lpstrPath);
    hs_log(tbuf);

    fopen_s(&fp, lpstrPath, "r");
    if (!fp)
    {
        _snprintf_s(tbuf, HS_BUF_256_CPY, "ERROR: Couldn't open %s for reading.",
                 lpstrPath);
        hs_log(tbuf);
        return false;
    }

    fgets(tbuf, HS_BUF_256_CPY, fp);
    if (isdigit(*tbuf) && (*(tbuf + 1) == '"'))
    {
        // Old style database.  Gonna have to do this the hard way.
        HS_UINT32 idx = 0;
        char *ptr;

        // Ok, start loading lines from the file.
        do
        {
            // Chop the newline
            if ((ptr = strchr(tbuf, '\n')) != NULL)
                *ptr = '\0';
            if ((ptr = strchr(tbuf, '\r')) != NULL)
                *ptr = '\0';

            // Check for the end of the file.
            if (!_strnicmp(tbuf, "*END*", 5))
                break;

            // Create the weapon specified by the first character.
            EHSWeaponClass eClass = (EHSWeaponClass) (*tbuf - '0');

            CHSWeaponData *pWeaponData =
                CHSWeaponData::CreateFromClass(eClass);
            if (!pWeaponData)
            {
                _snprintf_s(tbuf, HS_BUF_256_CPY,
                         "ERROR: Invalid weapon specification at line %d.\n",
                         idx);
                hs_log(tbuf);
            }
            else
            {
                // Have the weapon parse the data from the buffer.
                if (!pWeaponData->LoadFromBuffer(tbuf + 1))
                {
                    _snprintf_s(tbuf, HS_BUF_256_CPY,
                             "ERROR: Invalid weapon specification at line %d.\n",
                             idx);
                    hs_log(tbuf);
                    pWeaponData->Release();
                }
                else
                {
                    AddWeapon(pWeaponData, true);
                }
            }
            idx++;
        }
        while (fgets(tbuf, HS_BUF_256_CPY, fp));

        fclose(fp);
    }
    else
    {
        fclose(fp);

        // New style database!  Woot!
        CHSFileDatabase *pLoader = new CHSFileDatabase;

        if (pLoader->OpenFile(lpstrPath, CHSFileDatabase::OPENMODE_READ) !=
            CHSFileDatabase::DB_OK)
        {
            _snprintf_s(tbuf, HS_BUF_256_CPY,
                     "ERROR: Couldn't open %s for reading.", lpstrPath);
            hs_log(tbuf);
        }
        else
        {
            // Load all of the sections.
            HS_UINT32 uiSection = 0;
            while (pLoader->LoadNextSection() == CHSFileDatabase::DB_OK)
            {
                uiSection++;

                // We know the first attribute should be weapon class.
                CHSFileDatabase::THSDBEntry tEntry;

                if (pLoader->GetFirstAttribute(tEntry))
                {
                    EHSWeaponClass eClass =
                        (EHSWeaponClass) atoi(tEntry.strValue.c_str());

                    if (!pLoader->GetNextAttribute(tEntry))
                    {
                        _snprintf_s(tbuf, HS_BUF_256_CPY,
                                 "ERROR: Corrupt weapon database entry at section %d.",
                                 uiSection);
                        hs_log(tbuf);
                    }
                    else
                    {
                        HS_UINT32 uiTypeID = atoi(tEntry.strValue.c_str());

                        CHSWeaponData *pData =
                            CHSWeaponData::CreateFromClass(eClass);
                        if (!pData)
                        {
                            _snprintf_s(tbuf, HS_BUF_256_CPY,
                                     "ERROR: Invalid weapon class entry at section %d.",
                                     uiSection);
                            hs_log(tbuf);
                        }
                        else
                        {
                            pData->TypeID(uiTypeID);

                            if (!pData->ReadFromDatabase(pLoader))
                            {
                                _snprintf_s(tbuf, HS_BUF_256_CPY,
                                         "ERROR: Invalid weapon data at section %d.",
                                         uiSection);
                                hs_log(tbuf);
                                pData->Release();
                            }
                            else
                            {
                                // Add it to our database.
                                AddWeapon(pData, false);
                            }
                        }
                    }
                }
                else
                {
                    _snprintf_s(tbuf, HS_BUF_256_CPY,
                             "ERROR: Corrupt weapon database entry at section %d.",
                             uiSection);
                    hs_log(tbuf);
                }
            }
        }

        pLoader->Release();
    }


    _snprintf_s(tbuf, HS_BUF_256_CPY, "LOADING: %s - %d %s loaded (done)",
             lpstrPath,
             m_mapWeaponData.size(), m_mapWeaponData.size() == 1
             ? "weapon" : "weapons");
    hs_log(tbuf);

    return true;
}

void CHSWeaponDB::SaveToFile(char *lpstrPath)
{
    char tbuf[HS_BUF_512];

    // Is the file path good?
    if (!lpstrPath || strlen(lpstrPath) == 0)
    {
        hs_log
            ("Attempt to save the weapons database, but no path specified.  Could be a config file problem.");
        return;
    }

    if (m_mapWeaponData.empty())
    {
        return;
    }

    CHSFileDatabase *pDatabase = new CHSFileDatabase;

    if (pDatabase->OpenFile(lpstrPath, CHSFileDatabase::OPENMODE_WRITE)
        != CHSFileDatabase::DB_OK)
    {
        _snprintf_s(tbuf, HS_BUF_512_CPY,
                 "ERROR: Unable to write weapons to %s.", lpstrPath);
        hs_log(tbuf);
        return;
    }

    CSTLWeaponDataMap::iterator iter;
    for (iter = m_mapWeaponData.begin(); iter != m_mapWeaponData.end();
         iter++)
    {
        CHSWeaponData *pWeaponData = iter->second;

        pDatabase->StartSection();

        char cBuffer[HS_BUF_64];

        _snprintf_s(cBuffer, HS_BUF_64_CPY, "%d", pWeaponData->WeaponClass());
        pDatabase->AddSectionAttribute("WEAPON CLASS", cBuffer);

        _snprintf_s(cBuffer, HS_BUF_64_CPY, "%d", pWeaponData->TypeID());
        pDatabase->AddSectionAttribute("TYPE", cBuffer);

        pWeaponData->WriteToDatabase(pDatabase);

        pDatabase->EndSection();
    }

    pDatabase->CloseFile();
    pDatabase->Release();
}


HS_BOOL8 CHSLaserData::LoadFromBuffer(const HS_INT8 * pcBuffer)
{
    // Extract the name.
    HS_INT8 *ptr = (HS_INT8 *) pcBuffer;
    if (*ptr != '"')
    {
        // Bad format.
        hs_log("ERROR: Bad laser name format.");
        return false;
    }
    ptr++;
    while (*ptr && *ptr != '\n' && *ptr != '"')
    {
        m_strName.append(ptr, 1);
        ptr++;
    }

    if (*ptr != '"')
    {
        // Bad format.
        hs_log("ERROR: Bad laser name format.");
        return false;
    }
    ptr += 2;

    // Extract various attributes.
    HS_INT8 cAttrBuf[HS_BUF_256];

    // Strength.
    extract(ptr, cAttrBuf, 0, 1, ' ');
    m_strength = atoi(cAttrBuf);

    // Regen time.
    extract(ptr, cAttrBuf, 1, 1, ' ');
    m_regen_time = atoi(cAttrBuf);

    // Range.
    extract(ptr, cAttrBuf, 2, 1, ' ');
    m_range = atoi(cAttrBuf);

    // Accuracy.
    extract(ptr, cAttrBuf, 3, 1, ' ');
    m_accuracy = atoi(cAttrBuf);

    // Power consumption.
    extract(ptr, cAttrBuf, 4, 1, ' ');
    m_powerusage = atoi(cAttrBuf);

    // No hull.
    extract(ptr, cAttrBuf, 5, 1, ' ');
    m_nohull = atoi(cAttrBuf) == 0 ? false : true;

    return true;
}

HS_BOOL8 CHSMissileData::LoadFromBuffer(const HS_INT8 * pcBuffer)
{
    // Extract the name.
    HS_INT8 *ptr = (HS_INT8 *) pcBuffer;
    if (*ptr != '"')
    {
        // Bad format.
        hs_log("ERROR: Bad missile name format.");
        return false;
    }
    ptr++;
    while (*ptr && *ptr != '\n' && *ptr != '"')
    {
        m_strName.append(ptr, 1);
        ptr++;
    }

    if (*ptr != '"')
    {
        // Bad format.
        hs_log("ERROR: Bad missile name format.");
        return false;
    }
    ptr += 2;

    // Extract various attributes.
    HS_INT8 cAttrBuf[256];

    // Strength.
    extract(ptr, cAttrBuf, 0, 1, ' ');
    m_strength = atoi(cAttrBuf);

    // Reload time.
    extract(ptr, cAttrBuf, 1, 1, ' ');
    m_reload_time = atoi(cAttrBuf);

    // Range.
    extract(ptr, cAttrBuf, 2, 1, ' ');
    m_range = atoi(cAttrBuf);

    // Turn rate.
    extract(ptr, cAttrBuf, 3, 1, ' ');
    m_turnrate = atoi(cAttrBuf);

    // Speed.
    extract(ptr, cAttrBuf, 4, 1, ' ');
    m_speed = atoi(cAttrBuf);

    return true;
}

//
// CHSWeaponArray stuff
//
//
CHSWeaponArray::CHSWeaponArray(void):m_missile_bay(NULL)
{
}

// Attempts to remove the weapon in the specified slot.
HS_BOOL8 CHSWeaponArray::DeleteWeapon(HS_UINT32 uiSlot)
{
    if (uiSlot >= m_listWeapons.size())
    {
        return false;
    }

    // Find the weapon in question.
    HS_UINT32 idx = 0;
    CSTLWeaponList::iterator iter;
    for (iter = m_listWeapons.begin(); iter != m_listWeapons.end(); iter++)
    {
        if (idx == uiSlot)
        {
            break;
        }
        idx++;
    }

    if (iter == m_listWeapons.end())
    {
        return false;
    }

    CHSWeapon *pWeapon = *iter;
    delete pWeapon;

    m_listWeapons.erase(iter);

    return true;
}

// Returns the weapon in a given slot.
CHSWeapon *CHSWeaponArray::GetWeapon(HS_UINT32 uiSlot)
{
    if (uiSlot >= m_listWeapons.size())
    {
        return NULL;
    }

    // Find the weapon in question.
    HS_UINT32 idx = 0;
    CSTLWeaponList::iterator iter;
    for (iter = m_listWeapons.begin(); iter != m_listWeapons.end(); iter++)
    {
        if (idx == uiSlot)
        {
            break;
        }
        idx++;
    }

    if (iter == m_listWeapons.end())
    {
        return NULL;
    }

    CHSWeapon *pWeapon = *iter;
    return pWeapon;
}

// Takes a missile bay object and sets that bay as the
// source for all missile weapons in the array.
void CHSWeaponArray::SetMissileBay(CHSMissileBay * cBay)
{
    CHSMTube *mTube;

    // Run through the array, looking for missiles.
    CSTLWeaponList::iterator iter;
    for (iter = m_listWeapons.begin(); iter != m_listWeapons.end(); iter++)
    {
        CHSWeapon *pWeapon = *iter;

        if (pWeapon->GetWeaponClass() == WC_MISSILE)
        {
            mTube = static_cast < CHSMTube * >(pWeapon);
            mTube->SetMissileBay(cBay);
        }
    }
    m_missile_bay = cBay;
}

// Returns the total power usage for all weapons in the array.
HS_UINT32 CHSWeaponArray::GetTotalPower(void)
{
    HS_UINT32 uPower;

    uPower = 0;
    CSTLWeaponList::iterator iter;
    for (iter = m_listWeapons.begin(); iter != m_listWeapons.end(); iter++)
    {
        CHSWeapon *pWeapon = *iter;

        uPower += pWeapon->GetPowerUsage();
    }
    return uPower;
}

void CHSWeaponArray::ClearArray(void)
{
    CSTLWeaponList::iterator iter;
    for (iter = m_listWeapons.begin(); iter != m_listWeapons.end(); iter++)
    {
        CHSWeapon *pWeapon = *iter;

        delete pWeapon;
    }
    m_listWeapons.clear();
}

HS_BOOL8 CHSWeaponArray::AddWeapon(CHSWeapon * pWeapon)
{
    if (pWeapon == NULL)
    {
        return false;
    }

    m_listWeapons.push_back(pWeapon);

    return true;
}

// Runs through the list of weapons, searching out the maximum
// weapon range.
HS_UINT32 CHSWeaponArray::GetMaxRange(void)
{
    HS_UINT32 uiMaxRange = 0;

    CSTLWeaponList::iterator iter;
    for (iter = m_listWeapons.begin(); iter != m_listWeapons.end(); iter++)
    {
        CHSWeapon *pWeapon = *iter;

        HS_UINT32 uiRange = pWeapon->GetRange();
        if (uiRange > uiMaxRange)
        {
            uiMaxRange = uiRange;
        }
    }
    return uiMaxRange;
}

// Handles cyclic stuff for the weapons in the array
void CHSWeaponArray::DoCycle(void)
{
    // Run through our weapons, telling them to cycle.
    CSTLWeaponList::iterator iter;
    for (iter = m_listWeapons.begin(); iter != m_listWeapons.end(); iter++)
    {
        CHSWeapon *pWeapon = *iter;
        pWeapon->DoCycle();
    }
}

//
// CHSWeapon stuff
//
CHSWeapon::CHSWeapon(void):m_pWeaponData(NULL), m_change(STAT_NOCHANGE)
{
    m_shield_percentage = 100;
    m_hull_percentage = 100;
    m_mintargetsize = 0;
}

CHSWeapon::~CHSWeapon()
{
    if (m_pWeaponData)
    {
        m_pWeaponData->Release();
        m_pWeaponData = NULL;
    }
}

CHSWeapon *CHSWeapon::CreateWeapon(EHSWeaponClass eClass)
{
    // Determine class of weapon, and create the appropriate type.
    switch (eClass)
    {
    case WC_LASER:
        return static_cast < CHSWeapon * >(new CHSLaser);
        break;
    case WC_MISSILE:
        return static_cast < CHSWeapon * >(new CHSMTube);
        break;
    default:
        return NULL;
        break;
    }
}

HS_UINT32 CHSWeapon::GetPowerUsage(void)
{
    // Generic weapons require no power
    return 0;
}

HS_BOOL8 CHSWeapon::CheckTargetSize(CHS3DObject * cObj)
{
    return false;
}

// Indicates whether the object can attack the specified
// type of CHS3DObject.  Override this in your derived weapon
// class to have your weapon respond to different types of
// objects.
HS_BOOL8 CHSWeapon::CanAttackObject(CHS3DObject * cObj)
{
    // Generic weapons can't attack anything.
    hs_log("CHSWeapon::CanAttackObject() called for generic weapon.");
    return false;
}

// Returns the recent status change of the weapon.
int CHSWeapon::GetStatusChange(void)
{
    return m_change;
}

// Sets the status of the weapon by an integer identifier.
// It's up to the weapon to determine what it should do with
// that integer.
void CHSWeapon::SetStatus(int stat)
{
    // Generic weapons don't handle this.
    hs_log("CHSWeapon::SetStatus() called for generic weapon.");
}

// The generic weapon has no cyclic stuff
void CHSWeapon::DoCycle(void)
{
    hs_log("CHSWeapon::DoCycle() called for generic weapon.");
}

HS_UINT32 CHSWeapon::GetRange(void)
{
    // Generic weapons have no range
    return 0;
}

HS_BOOL8 CHSWeapon::IsReady(void)
{
    return true;
}

// Returns the type of the weapon, which is taken from
// the parent.
EHSWeaponClass CHSWeapon::GetWeaponClass(void)
{
    if (!m_pWeaponData)
    {
        return WC_LASER;
    }

    return m_pWeaponData->WeaponClass();
}

// Sets the parent of the weapon to a CHSDBWeapon object
void CHSWeapon::SetData(CHSWeaponData * pData)
{
    CHSLaser *cLaser;
    CHSMTube *cMissile;

    if (pData->WeaponClass() == WC_LASER)
    {
        cLaser = (CHSLaser *) this;
        m_pWeaponData = (CHSLaserData *) pData;
        ((CHSLaserData *) m_pWeaponData)->AddRef();
        cLaser->SetData((CHSLaserData *) pData);
    }
    else if (pData->WeaponClass() == WC_MISSILE)
    {
        cMissile = (CHSMTube *) this;
        m_pWeaponData = (CHSMissileData *) pData;
        ((CHSMissileData *) m_pWeaponData)->AddRef();
        cMissile->SetData((CHSMissileData *) pData);
    }

    m_shield_percentage = pData->ShieldPercentage();
    m_hull_percentage = pData->HullPercentage();
}

// Override this function in your derived weapon class
// to return a custom name.
const HS_INT8 *CHSWeapon::GetName(void)
{
    if (m_strName.length())
        return m_strName.c_str();
    else if (!m_pWeaponData)
        return "No Name";

    // Return the name from the parent
    return m_pWeaponData->Name();
}

// Indicates whether the weapon needs a target to be
// fired.
HS_BOOL8 CHSWeapon::RequiresLock(void)
{
    return false;
}

// Override this function in your derived weapon class
// to return custom attribute information about the weapon,
// such as strength, regeneration time, etc.
const char *CHSWeapon::GetAttrInfo(void)
{
    // Generic weapons have no attributes
    return "No information available.";
}

// Override this function in your derived weapon class
// to return custom status info such as time left to
// recharge, etc.
const char *CHSWeapon::GetStatus(void)
{
    // Generic weapons have unknown status
    return "Unknown";
}

// Returns an integer that is meaningful only to the weapon.
// The status is saved during shutdown and passed back to
// the weapon on reboot for handling.
int CHSWeapon::GetStatusInt(void)
{
    return 0;
}

// Indicates whether the weapon is configurable or not.
HS_BOOL8 CHSWeapon::Configurable(void)
{
    // Generic weapons are not configurable
    return false;
}

HS_BOOL8 CHSWeapon::Configure(int type)
{

    // Generic weapons cannot be configured.
    return false;
}

HS_BOOL8 CHSWeapon::Loadable(void)
{
    // Generic weapons are not loadable
    return false;
}

HS_BOOL8 CHSWeapon::Reload(void)
{
    // Generic weapons are not loadable
    return false;
}

HS_BOOL8 CHSWeapon::Unload(void)
{
    // Generic weapons can't unload
    return false;
}

// Returns the number of seconds until reload, or 0 if
// not reloading.
int CHSWeapon::Reloading(void)
{
    // Generic weapons are never reloading
    return 0;
}

// Use this routine to instruct the weapon to attack a given
// object.  Supply the HS_DBREF of the console the weapon should
// send messages back to as well.
void CHSWeapon::AttackObject(CHS3DObject * cSource,
                             CHS3DObject * cTarget,
                             CHSConsole * cConsole,
                             int iSysType, HS_INT32 hit_flag)
{
    HS_DBREF dbUser;

    dbUser = hsInterface.ConsoleUser(cConsole->m_objnum);
    if (dbUser != HSNOTHING)
    {
        hsStdError(dbUser,
                   "I don't know how to attack.  I'm just a plain old weapon.");
    }
}

const HS_INT8 *CHSWeapon::GetAttributeValue(const HS_INT8 * strName)
{
    static char rval[HS_BUF_64];

    *rval = '\0';
    if (!_strnicmp(strName, "SHIELD PERCENTAGE", strlen(strName)))
    {
        _snprintf_s(rval, HS_BUF_64_CPY, "%.2f", m_shield_percentage);
    }
    else if (!_strnicmp(strName, "HULL PERCENTAGE", strlen(strName)))
    {
        _snprintf_s(rval, HS_BUF_64_CPY, "%.2f", m_hull_percentage);
    }
    else if (!_strnicmp(strName, "MIN TARGET SIZE", strlen(strName)))
    {
        _snprintf_s(rval, HS_BUF_64_CPY, "%d", m_mintargetsize);
    }
    else if (!_strnicmp(strName, "NAME", strlen(strName)))
    {
        _snprintf_s(rval, HS_BUF_64_CPY, "%s", m_strName.c_str());
    }
    else
    {
        return m_pWeaponData->GetAttributeValue(strName);
    }

    return rval;
}


HS_BOOL8 CHSWeapon::WriteToDatabase(CHSFileDatabase * pDatabase)
{
    HS_INT8 cBuffer[HS_BUF_64];

    m_pWeaponData->WriteToDatabase(pDatabase);

    _snprintf_s(cBuffer, HS_BUF_64_CPY, "%3.0f", m_shield_percentage);
    pDatabase->AddSectionAttribute("SHIELD PERCENTAGE", cBuffer);

    _snprintf_s(cBuffer, HS_BUF_64_CPY, "%3.0f", m_hull_percentage);
    pDatabase->AddSectionAttribute("HULL PERCENTAGE", cBuffer);

    return true;
}

HS_BOOL8 CHSWeapon::ReadFromDatabase(CHSFileDatabase * pDatabase)
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
HS_BOOL8 CHSWeapon::SetAttributeValue(const HS_INT8 * strName,
                                      const HS_INT8 * strValue)
{
    if (!_strnicmp(strName, "SHIELD PERCENTAGE", strlen(strName)))
    {
        m_shield_percentage = strtod(strValue, NULL);
        return true;
    }
    else if (!_strnicmp(strName, "HULL PERCENTAGE", strlen(strName)))
    {
        m_hull_percentage = strtod(strValue, NULL);
        return true;
    }
    else if (!_strnicmp(strName, "MIN TARGET SIZE", strlen(strName)))
    {
        m_mintargetsize = strtol(strValue, NULL, 10);
        return true;
    }
    else if (!_strnicmp(strName, "NAME", strlen(strName)))
    {
        m_strName = strValue;
        return true;
    }

    return false;
}

void CHSWeapon::GetAttributeList(CHSAttributeList & rlistAttrs)
{
    m_pWeaponData->GetAttributeList(rlistAttrs);
}

//
// CHSLaser stuff
//
CHSLaser::CHSLaser(void)
{
    m_regenerating = false;
    m_regen_time = 1;
    m_range = 1;
    m_strength = 1;
    m_accuracy = 1;
    m_power = 1;
    m_nohull = 0;
    m_mintargetsize = 0;
    m_shield_percentage = 100;
    m_hull_percentage = 100;
}

// Cyclic stuff for lasers
void CHSLaser::DoCycle(void)
{
    m_change = STAT_NOCHANGE;

    if (m_regenerating)
        Regenerate();
}

void CHSLaser::SetData(CHSLaserData * pData)
{
    m_regen_time = pData->RegenTime();
    m_range = pData->Range();
    m_accuracy = pData->Accuracy();
    m_strength = pData->Strength();
    m_power = pData->PowerUsage();
    m_nohull = pData->NoHull();
}

const HS_INT8 *CHSLaser::GetAttributeValue(const HS_INT8 * strName)
{
    static char rval[HS_BUF_64];

    *rval = '\0';
    if (!_strnicmp(strName, "REGEN TIME", strlen(strName)))
    {
        _snprintf_s(rval, HS_BUF_64_CPY, "%d", m_regen_time);
    }
    else if (!_strnicmp(strName, "RANGE", strlen(strName)))
    {
        _snprintf_s(rval, HS_BUF_64_CPY, "%d", m_range);
    }
    else if (!_strnicmp(strName, "STRENGTH", strlen(strName)))
    {
        _snprintf_s(rval, HS_BUF_64_CPY, "%d", m_strength);
    }
    else if (!_strnicmp(strName, "ACCURACY", strlen(strName)))
    {
        _snprintf_s(rval, HS_BUF_64_CPY, "%d", m_accuracy);
    }
    else if (!_strnicmp(strName, "POWER", strlen(strName)))
    {
        _snprintf_s(rval, HS_BUF_64_CPY, "%d", m_power);
    }
    else if (!_strnicmp(strName, "NOHULL", strlen(strName)))
    {
        _snprintf_s(rval, HS_BUF_64_CPY, "%d", m_nohull);
    }
    else
    {
        return CHSWeapon::GetAttributeValue(strName);
    }

    return rval;
}


HS_BOOL8 CHSLaser::WriteToDatabase(CHSFileDatabase * pDatabase)
{
    HS_INT8 cBuffer[HS_BUF_64];

    m_pWeaponData->WriteToDatabase(pDatabase);

    _snprintf_s(cBuffer, HS_BUF_64_CPY, "%d", m_regen_time);
    pDatabase->AddSectionAttribute("REGEN TIME", cBuffer);

    _snprintf_s(cBuffer, HS_BUF_64_CPY, "%d", m_range);
    pDatabase->AddSectionAttribute("RANGE", cBuffer);

    _snprintf_s(cBuffer, HS_BUF_64_CPY, "%d", m_strength);
    pDatabase->AddSectionAttribute("STRENGTH", cBuffer);

    _snprintf_s(cBuffer, HS_BUF_64_CPY, "%d", m_accuracy);
    pDatabase->AddSectionAttribute("ACCURACY", cBuffer);

    _snprintf_s(cBuffer, HS_BUF_64_CPY, "%d", m_power);
    pDatabase->AddSectionAttribute("POWER", cBuffer);

    _snprintf_s(cBuffer, HS_BUF_64_CPY, "%d", m_nohull);
    pDatabase->AddSectionAttribute("NOHULL", cBuffer);

    _snprintf_s(cBuffer, HS_BUF_64_CPY, "%3.0f", m_shield_percentage);
    pDatabase->AddSectionAttribute("SHIELD PERCENTAGE", cBuffer);

    _snprintf_s(cBuffer, HS_BUF_64_CPY, "%3.0f", m_hull_percentage);
    pDatabase->AddSectionAttribute("HULL PERCENTAGE", cBuffer);

    return true;
}

HS_BOOL8 CHSLaser::ReadFromDatabase(CHSFileDatabase * pDatabase)
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
HS_BOOL8 CHSLaser::SetAttributeValue(const HS_INT8 * strName,
                                     const HS_INT8 * strValue)
{
    int iVal = -1;

    // Try to match the name
    if (!_strnicmp(strName, "REGEN TIME", strlen(strName)))
    {
        iVal = atoi(strValue);
        if (iVal < 0)
            iVal = 0;

        m_regen_time = iVal;
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
    else if (!_strnicmp(strName, "ACCURACY", strlen(strName)))
    {
        iVal = atoi(strValue);
        if (iVal < 1)
            iVal = 1;

        m_accuracy = iVal;
        return true;
    }
    else if (!_strnicmp(strName, "POWER", strlen(strName)))
    {
        iVal = atoi(strValue);
        if (iVal < 0)
            iVal = 0;

        m_power = iVal;
        return true;
    }
    else if (!_strnicmp(strName, "NOHULL", strlen(strName)))
    {
        iVal = atoi(strValue);
        m_nohull = iVal == 0 ? false : true;
        return true;
    }
    else
    {
        return CHSWeapon::SetAttributeValue(strName, strValue);
    }

    return false;
}

void CHSLaser::GetAttributeList(CHSAttributeList & rlistAttrs)
{
    m_pWeaponData->GetAttributeList(rlistAttrs);
}

// Attacks a target object.
void CHSLaser::AttackObject(CHS3DObject * cSource,
                            CHS3DObject * cTarget,
                            CHSConsole * cConsole,
                            int iSysType, HS_INT32 hit_flag)
{
    HS_DBREF dbUser;
    int iAttackRoll = 0;
    int iDefendRoll = 0;
    double sX = 0.0, sY = 0.0, sZ = 0.0;        // Source object coords
    double tX = 0.0, tY = 0.0, tZ = 0.0;        // Target object coords;

    // Grab the user of the console.
    dbUser = hsInterface.ConsoleUser(cConsole->m_objnum);

    // Cannot attack while cloaked
    if (cSource->GetType() == HST_SHIP)
    {
        CHSSysCloak *cCloak;
        CHSShip *ptr;

        ptr = (CHSShip *) cSource;

        // Look for the cloaking device.
        cCloak = (CHSSysCloak *) ptr->GetEngSystem(HSS_CLOAK);
        if (cCloak)
        {
            if (cCloak->GetEngaged())
            {
                if (dbUser != HSNOTHING)
                    hsStdError(dbUser, "You cannot fire while cloaked.");
                return;
            }
        }
    }

    if (!CanAttackObject(cTarget))
    {
        if (dbUser != HSNOTHING)
            hsStdError(dbUser,
                       "You cannot attack that target with that weapon.");
    }

    // Calculate distance to object
    sX = cSource->GetX();
    sY = cSource->GetY();
    sZ = cSource->GetZ();
    tX = cTarget->GetX();
    tY = cTarget->GetY();
    tZ = cTarget->GetZ();

    double dDistance = 0.0;
    dDistance = Dist3D(sX, sY, sZ, tX, tY, tZ) + .00001;

    // Size of a target ship matters relative to distance.
    // The closer a target gets to the ship, the larger
    // it effectively is.  That is to say it takes up more
    // of the view angle.  When the target is right next
    // to the ship, in front of the gun, it is essentially
    // the broad side of a barn, which everyone can hit.
    // Thus, to handle this we'll calculate the size of
    // the target and the viewing angle it takes up.

    double dSize = 0.0;         // Size of the side of the target
    double dAngle = 0.0;        // Amount of viewing angle taken up by size

    dSize = cTarget->GetSize();
    dSize = (.7 * dSize) * (.7 * dSize);

    // Halve the size, and divide by distance.  This
    // gives us the tangent of the angle taken up by
    // the target.
    dSize = (dSize * .5) / dDistance;

    // Take the inverse tangent to get angle.
    dAngle = atan(dSize);

    // Double the angle because we used half of the size
    // to get the angle of a right triangle.
    dAngle *= 2;

    // We now have the viewing angle consumed by the
    // target.  There's a maximum possible value of 180,
    // so divide by that to determine how much of the viewing
    // angle is taken up by the target.
    dSize = dAngle * .005555;

    // Subtract from 1 to get maximum values of 1 when the
    // angle is small.
    dSize = 1 - dSize;

    // Now multiply by 6 to get relative difficulty of hitting
    // target.
    iDefendRoll = (int) (6 * dSize) + hsInterface.GetRandom(6);
    iAttackRoll = GetAccuracy() + hsInterface.GetRandom(6);

    // Simulate difficulty when a target is moving.
    // If the target is moving toward or away from the
    // attacker, it's not very difficult.  Thus, we
    // calculate the change in angle for the target
    // during one cycle.  The maximum change is 180
    // degrees.
    CHSVector tVec;
    CHSVector aVec;
    tVec = cTarget->GetMotionVector();
    aVec = cSource->GetMotionVector();

    // Calculate vector to target now.
    double dx, dy, dz;
    dx = tX - sX;
    dy = tY - sY;
    dz = tZ - sZ;

    // Make a unit vector
    dx /= dDistance;
    dy /= dDistance;
    dz /= dDistance;

    CHSVector nowVec(dx, dy, dz);

    // Now calculate coordinate for source and target
    // in one cycle.
    double sX2, sY2, sZ2;
    double tX2, tY2, tZ2;
    double aSpeed, tSpeed;

    // Grab both object speeds, and bring them down
    // to per-second levels.
    aSpeed = cSource->GetSpeed() * .0002778;
    tSpeed = cTarget->GetSpeed() * .0002778;

    // Calculate coordinates for next cycle.
    sX2 = sX + (aVec.i() * aSpeed);
    sY2 = sY + (aVec.j() * aSpeed);
    sZ2 = sZ + (aVec.k() * aSpeed);
    tX2 = tX + (tVec.i() * tSpeed);
    tY2 = tY + (tVec.j() * tSpeed);
    tZ2 = tZ + (tVec.k() * tSpeed);

    // Calculate vector to target after next cycle
    dx = tX2 - sX2;
    dy = tY2 - sY2;
    dz = tZ2 - sZ2;

    // Divide by distance to make a unit vector
    double dDistance2;
    dDistance2 = Dist3D(sX2, sY2, sZ2, tX2, tY2, tZ2);
    dx /= dDistance2;
    dy /= dDistance2;
    dz /= dDistance2;

    CHSVector nextVec(dx, dy, dz);

    // Calculate the dot product between the previous
    // and the next cycle vectors.
    double dp;
    dp = nowVec.DotProduct(nextVec);

    // Calculate the angle change.  This is in radians.
    dAngle = acos(dp);

    // Now divide angle change by 2pi to get change in angle
    // from 0 to 1, where 1 is a huge change in angle and,
    // therefore, high difficulty.
    dAngle *= .15915;

    // Add up to 6 points of defense for "evasion" by angle
    // change.
    iDefendRoll += (int) (6 * dAngle);


    // If distance is farther than our range, the shot always
    // misses.

    double range;
    range = GetRange();

    CHSUniverse *uDest;
    char tbuf[HS_BUF_256];
    char fstat1[HS_BUF_128];
    char fstat2[HS_BUF_128];

    if (dDistance >= range || iDefendRoll > iAttackRoll || hit_flag == 0)
    {
        _snprintf_s(fstat1, HS_BUF_128_CPY,
                 "%s%smisses%s", ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL);
        _snprintf_s(fstat2, HS_BUF_128_CPY,
                 "%s%smissed%s", ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL);
    }
    else
    {
        _snprintf_s(fstat1, HS_BUF_128_CPY,
                 "%s%shits%s", ANSI_HILITE, ANSI_RED, ANSI_NORMAL);
        _snprintf_s(fstat2, HS_BUF_128_CPY,
                 "%s%shit%s", ANSI_HILITE, ANSI_RED, ANSI_NORMAL);
    }

    uDest = cSource->GetUniverse();

    CHSSysSensors *cSensors;
    SENSOR_CONTACT *cContactS;
    SENSOR_CONTACT *cContactD;
    CHS3DObject *pObject;
    for (pObject = uDest->GetFirstActiveObject(); pObject;
         pObject = uDest->GetNextActiveObject())
    {
        if (pObject == cSource || pObject == cTarget)
        {
            continue;
        }

        cSensors = (CHSSysSensors *) pObject->GetEngSystem(HSS_SENSORS);
        if (!cSensors)
        {
            continue;
        }

        cContactS = cSensors->GetContact(cSource);
        cContactD = cSensors->GetContact(cTarget);


        if (!cContactS && !cContactD)
        {
            continue;
        }

        if (!cContactS && cContactD)
        {
            if (cContactD->status == DETECTED)
            {
                _snprintf_s(tbuf, HS_BUF_256_CPY,
                         "%s[%s%s%d%s%s]%s - Unknown contact is being fired upon and %s",
                         cTarget->GetObjectColor(),
                         ANSI_NORMAL, ANSI_HILITE,
                         cContactD->m_id, ANSI_NORMAL,
                         cTarget->GetObjectColor(), ANSI_NORMAL, fstat2);
                pObject->HandleMessage(tbuf, MSG_SENSOR, (long *) pObject);
            }
            else if (cContactD->status == IDENTIFIED)
            {
                _snprintf_s(tbuf, HS_BUF_256_CPY,
                         "%s[%s%s%d%s%s]%s - The %s is being fired upon and %s",
                         cTarget->GetObjectColor(),
                         ANSI_NORMAL, ANSI_HILITE,
                         cContactD->m_id, ANSI_NORMAL,
                         cTarget->GetObjectColor(),
                         ANSI_NORMAL, cSource->GetName(), fstat2);
                pObject->HandleMessage(tbuf, MSG_SENSOR, (long *) pObject);
            }
            continue;
        }


        if (cContactS && !cContactD)
        {
            if (cContactS->status == DETECTED)
            {
                _snprintf_s(tbuf, HS_BUF_256_CPY,
                         "%s[%s%s%d%s%s]%s - Unknown contact is firing upon something",
                         cSource->GetObjectColor(),
                         ANSI_NORMAL, ANSI_HILITE,
                         cContactS->m_id, ANSI_NORMAL,
                         cSource->GetObjectColor(), ANSI_NORMAL);
                pObject->HandleMessage(tbuf, MSG_SENSOR, (long *) pObject);
            }
            else if (cContactS->status == IDENTIFIED)
            {
                _snprintf_s(tbuf, HS_BUF_256_CPY,
                         "%s[%s%s%d%s%s]%s - The %s is firing upon something",
                         cSource->GetObjectColor(),
                         ANSI_NORMAL, ANSI_HILITE,
                         cContactS->m_id, ANSI_NORMAL,
                         cSource->GetObjectColor(),
                         ANSI_NORMAL, cSource->GetName());
                pObject->HandleMessage(tbuf, MSG_SENSOR, (long *) pObject);
            }
            continue;
        }

        if (cContactS && cContactD)
            if (cContactS->status == DETECTED
                && cContactD->status == DETECTED)
            {
                _snprintf_s(tbuf, HS_BUF_256_CPY,
                         "%s[%s%s%d%s%s]%s - Unknown contact fires and %s unknown contact %s[%s%s%d%s%s]%s",
                         cSource->GetObjectColor(),
                         ANSI_NORMAL, ANSI_HILITE,
                         cContactS->m_id,
                         ANSI_NORMAL, cSource->GetObjectColor(),
                         ANSI_NORMAL, fstat1, cTarget->GetObjectColor(),
                         ANSI_NORMAL, ANSI_HILITE,
                         cContactD->m_id, ANSI_NORMAL,
                         cTarget->GetObjectColor(), ANSI_NORMAL);
                pObject->HandleMessage(tbuf, MSG_SENSOR, (long *) pObject);
            }
            else if (cContactS->status == IDENTIFIED &&
                     cContactD->status == IDENTIFIED)
            {
                _snprintf_s(tbuf, HS_BUF_256_CPY,
                         "%s[%s%s%d%s%s]%s - The %s fires and %s the %s",
                         cSource->GetObjectColor(),
                         ANSI_NORMAL, ANSI_HILITE,
                         cContactS->m_id, ANSI_NORMAL,
                         cSource->GetObjectColor(),
                         ANSI_NORMAL,
                         cSource->GetName(), fstat1, cTarget->GetName());
                pObject->HandleMessage(tbuf, MSG_SENSOR, (long *) pObject);
            }
            else if (cContactS->status == IDENTIFIED &&
                     cContactD->status == DETECTED)
            {
                _snprintf_s(tbuf, HS_BUF_256_CPY,
                         "%s[%s%s%d%s%s]%s - The %s fires and %s unknown contact %s[%s%s%d%s%s]%s",
                         cSource->GetObjectColor(),
                         ANSI_NORMAL, ANSI_HILITE,
                         cContactS->m_id, ANSI_NORMAL,
                         cSource->GetObjectColor(), ANSI_NORMAL,
                         cSource->GetName(), fstat1,
                         cTarget->GetObjectColor(), ANSI_NORMAL, ANSI_HILITE,
                         cContactD->m_id, ANSI_NORMAL,
                         cTarget->GetObjectColor(), ANSI_NORMAL);
                pObject->HandleMessage(tbuf, MSG_SENSOR, (long *) pObject);
            }
            else if (cContactS->status == DETECTED &&
                     cContactD->status == IDENTIFIED)
            {
                _snprintf_s(tbuf, HS_BUF_256_CPY,
                         "%s[%s%s%d%s%s]%s - Unknown contact fires and %s the %s",
                         cSource->GetObjectColor(), ANSI_NORMAL, ANSI_HILITE,
                         cContactS->m_id, ANSI_NORMAL,
                         cSource->GetObjectColor(), ANSI_NORMAL, fstat1,
                         cTarget->GetName());
                pObject->HandleMessage(tbuf, MSG_SENSOR, (long *) pObject);
            }

    }


    if (dDistance >= range)
    {
        if (dbUser != HSNOTHING)
        {
            hsStdError(dbUser, "Your shot dissipates short of its target.");
        }

        strncpy_s(tbuf, "An incoming energy shot has missed us.",
                HS_BUF_256_CPY);
        cTarget->HandleMessage(tbuf, MSG_COMBAT, (long *) cSource);
    }
    else if (hit_flag == 0)
    {
        // The weapon misses. :(
        if (dbUser != HSNOTHING)
        {
            hsStdError(dbUser,
                       "Your shot skims past your target and out into space.");
        }

        strncpy_s(tbuf, "An incoming energy shot has missed us.",
                HS_BUF_256_CPY);
        cTarget->HandleMessage(tbuf, MSG_COMBAT, (long *) cSource);
    }
    else if (iAttackRoll > iDefendRoll || hit_flag == 1)
    {
        // The weapon hits!
        // Determine strength based on base weapon
        // strength and range to target.
        int strength;

        strength = GetStrength();
        if (dDistance > (range * .333))
        {
            strength = (int) (strength *
                              (.333 + (1 - (dDistance / (range + .0001)))));
        }

        // If iSysType is not HSS_NOTYPE, then do a roll
        // against the accuracy of the weapon to see if
        // the system gets hit.
        if (iSysType != HSS_NOTYPE)
        {
            HS_UINT32 ARoll, SRoll;

            ARoll = hsInterface.GetRandom(GetAccuracy());
            SRoll = hsInterface.GetRandom(10);

            if (SRoll > ARoll)
            {
                iSysType = HSS_NOTYPE;  // Didn't succeed
            }
        }

        // Tell the target to take damage
        cTarget->HandleDamage(cSource, m_pWeaponData, strength,
                              cConsole, iSysType);

    }
    else
    {
        // The weapon misses. :(
        if (dbUser != HSNOTHING)
        {
            hsStdError(dbUser,
                       "Your shot skims past your target and out into space.");
        }

        strncpy_s(tbuf, "An incoming energy shot has missed us.",
                HS_BUF_256_CPY);
        cTarget->HandleMessage(tbuf, MSG_COMBAT, (long *) cSource);
    }

    Regenerate();
}

// Lasers require a target lock
HS_BOOL8 CHSLaser::RequiresLock(void)
{
    return true;
}

HS_BOOL8 CHSLaser::NoHull(void)
{
    if (!m_pWeaponData)
    {
        return false;
    }

    return static_cast < CHSLaserData * >(m_pWeaponData)->NoHull();
}

// Handles the status integer sent to the weapon from
// some calling functions.
void CHSLaser::SetStatus(int stat)
{
    // Any integer indicates a reloading
    // status.
    if (stat > 0)
    {
        m_regenerating = true;
        m_time_to_regen = stat;
    }
}

// Returns attribute information in a string.
const char *CHSLaser::GetAttrInfo(void)
{
    static char rval[HS_BUF_128];

    _snprintf_s(rval, HS_BUF_128_CPY, "R: %-5d S: %-3d Rt: %d",
             GetRange(), GetStrength(), GetRegenTime());
    return rval;
}

HS_BOOL8 CHSLaser::IsReady(void)
{
    if (!m_regenerating)
    {
        return true;
    }

    return false;
}

void CHSLaser::Regenerate(void)
{
    // Check to see if we're regenerating.
    // If not, then begin it.
    if (!m_regenerating)
    {
        m_regenerating = true;
        m_time_to_regen = GetRegenTime();
    }
    else
    {
        m_time_to_regen--;

        if (m_time_to_regen <= 0)
        {
            m_change = STAT_READY;
            m_regenerating = false;
        }
    }
}

HS_BOOL8 CHSLaser::CheckTargetSize(CHS3DObject * cObj)
{
    if (cObj->GetSize() < m_pWeaponData->GetMinimumTargetSize())
    {
        return false;
    }
    return true;
}

// Indicates whether the laser can attack the given type
// of space object.
HS_BOOL8 CHSLaser::CanAttackObject(CHS3DObject * cObj)
{
    // Lasers can attack ships or missiles but can
    // only attack missiles if they cause hull damage
    if (cObj->GetType() == HST_SHIP ||
        (cObj->GetType() == HST_MISSILE && false == NoHull()))

    {
        return true;
    }

    return false;
}

// Returns an integer representing the status of the laser.
// This integer is meaningful only to the laser code.
int CHSLaser::GetStatusInt(void)
{
    // Regenerating?
    if (m_regenerating)
    {
        return m_time_to_regen;
    }

    return 0;                   // Not regenerating
}

// Returns the status of the laser
const char *CHSLaser::GetStatus(void)
{
    static char tbuf[HS_BUF_64];

    if (m_regenerating)
    {
        _snprintf_s(tbuf, HS_BUF_64_CPY, "(%d) Charging", m_time_to_regen);
        return tbuf;
    }


    return "Ready";
}

HS_UINT32 CHSLaser::GetRegenTime(void)
{
    return m_regen_time;
}

HS_UINT32 CHSLaser::GetRange(void)
{
    return m_range;
}

HS_UINT32 CHSLaser::GetStrength(void)
{
    return m_strength;
}

HS_UINT32 CHSLaser::GetPowerUsage(void)
{
    return m_power;
}

HS_UINT32 CHSLaser::GetAccuracy(void)
{
    return m_accuracy;
}

CHSWeaponData *CHSWeaponData::CreateFromClass(EHSWeaponClass eClass)
{
    // Determine class of weapon, and create the right type.
    switch (eClass)
    {
    case WC_LASER:
        return new CHSLaserData;
        break;
    case WC_MISSILE:
        return new CHSMissileData;
        break;
    default:
        hs_log("Invalid class passed to CHSWeaponData::CreateFromClass()");
        return NULL;
        break;
    }
}
