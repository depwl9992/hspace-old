// -----------------------------------------------------------------------
// $Id: hsclass.cpp,v 1.12 2006/06/17 03:31:48 mark Exp $
// -----------------------------------------------------------------------

#include "pch.h"

#include <cstdio>
#include <stdlib.h>

#include "hscopyright.h"
#include "hspace.h"
#include "hsclass.h"
#include "hsengines.h"
#include "hsutils.h"
#include "hsconf.h"
#include "hsinterface.h"

const HS_UINT32 HSCONST_DEFAULT_MAX_CLASSIDS = 64;

CHSClassDB myCHSClassDB;        // Instantiate to generate singleton

//CHSClassDB::InstPtr   CHSClassDB::m_pTheInstance;     // The singleton

CHSClassDBDef::CHSClassDBDef(void):m_uiMaxClassID(0), m_pucClassIDs(NULL)
{
}

HS_BOOL8 CHSClassDBDef::RemoveClass(HS_UINT32 uiClassID)
{
    CSTLShipClassMap::iterator iter;

    // Find the class.
    if ((iter = m_mapShipClasses.find(uiClassID)) != m_mapShipClasses.end())
    {
        // Found the class.
        CHSShipClass *pClass = (*iter).second;

        // Erase it from the map.
        m_mapShipClasses.erase(iter);

        // Free the class' ID.
        UnlockClassID(pClass->Id());

        // Delete the class.
        delete pClass;
        return true;
    }

    return false;
}

HS_UINT32 CHSClassDBDef::GetNextClassID(HS_BOOL8 bLock)
{
    // Do we have any class IDs at all?
    if (!m_pucClassIDs)
    {
        HS_UINT32 uiNumBytes = 1 + (HSCONST_DEFAULT_MAX_CLASSIDS / 8);

        m_pucClassIDs = new HS_UINT8[uiNumBytes];
        memset(m_pucClassIDs, 0, uiNumBytes);
        m_uiMaxClassID = uiNumBytes * 8 - 1;
    }

    // Find the first bit in the class IDs array that is 0.
    for (HS_UINT32 i = 0; i <= m_uiMaxClassID; i++)
    {
        HS_UINT32 uiByte = i / 8;

        if (m_pucClassIDs[uiByte] & (1 << (i % 8)))
        {
            // This bit is turned on.  It's not available.
            continue;
        }
        else
        {
            // Here's a free ID.
            if (bLock)
            {
                LockClassID(i);
            }
            return i;
        }
    }

    // There are no more free class IDs in our array.  Double it, and get the next ID.
    HS_UINT32 uiNewSize = 1 + (m_uiMaxClassID * 2) / 8;
    HS_UINT32 uiPrevSize = (m_uiMaxClassID + 1) / 8;
    HS_UINT8 *pucNewArray = new HS_UINT8[uiNewSize];
    memset(pucNewArray, 0, uiNewSize);
    memcpy(pucNewArray, m_pucClassIDs, uiPrevSize);
    delete[]m_pucClassIDs;
    m_pucClassIDs = pucNewArray;

    m_uiMaxClassID = uiNewSize * 8 - 1;

    return GetNextClassID(bLock);
}

void CHSClassDBDef::LockClassID(HS_UINT32 uiClassID)
{
    // Is this class ID bigger than our array?
    if ((m_uiMaxClassID == 0) || (uiClassID > m_uiMaxClassID))
    {
#ifndef WIN32
        HS_UINT32 uiSize =
            1 + (std::max(uiClassID, HSCONST_DEFAULT_MAX_CLASSIDS) / 8);
#else
        HS_UINT32 uiSize =
            1 + (_cpp_max(uiClassID, HSCONST_DEFAULT_MAX_CLASSIDS) / 8);
#endif

        HS_UINT8 *pucNewArray;
        pucNewArray = new HS_UINT8[uiSize];
        memset(pucNewArray, 0, uiSize);

        // Should we copy the previous settings?
        if (m_pucClassIDs)
        {
            HS_UINT32 uiNumPrevBytes = (m_uiMaxClassID + 1) / 8;

            memcpy(pucNewArray, m_pucClassIDs, uiNumPrevBytes);

            delete[]m_pucClassIDs;
        }

        m_pucClassIDs = pucNewArray;

        m_uiMaxClassID = uiSize * 8 - 1;
    }

    // Flip the bit to 1.
    HS_UINT32 uiByte = uiClassID / 8;
    m_pucClassIDs[uiByte] |= (1 << (uiClassID % 8));
}

void CHSClassDBDef::UnlockClassID(HS_UINT32 uiClassID)
{
    if (uiClassID > m_uiMaxClassID)
    {
        // This is an invalid operation, so we don't care.
        return;
    }

    // Flip the bit to 0.
    HS_UINT32 uiByte = uiClassID / 8;
    m_pucClassIDs[uiByte] &= ~(1 << (uiClassID % 8));
}

HS_BOOL8 CHSClassDBDef::AddClass(CHSShipClass * pClass)
{
    if (!pClass)
    {
        hs_log("NULL class pointer sent to AddClass().");
        return false;
    }

    // See if we already have this class registered.
    CSTLShipClassMap::iterator iter;

    if ((iter =
         m_mapShipClasses.find(pClass->Id())) != m_mapShipClasses.end())
    {
        // We already have a class with this id.
        return false;
    }

    m_mapShipClasses[pClass->Id()] = pClass;

    // Mark this class ID as being used.
    LockClassID(pClass->Id());

    return true;
}

HS_BOOL8 CHSClassDBDef::LoadFromFile(const char *pcPath)
{
    FILE *fp;
    int ok;
    char tbuf[256];
    char lbuf[256];
    char strKey[64];
    char strValue[64];
    CHSShipClass *pNewClass = NULL;
    CHSEngSystem *cSys;

    sprintf(tbuf, "LOADING: %s", pcPath);
    hs_log(tbuf);

    fp = fopen(pcPath, "r");
    if (!fp)
    {
        sprintf(tbuf, "ERROR: Couldn't open %s for loading.", pcPath);
        hs_log(tbuf);
        return false;
    }

    // Load database version for possible conversion.
    strcpy(tbuf, getstr(fp));
    extract(tbuf, strKey, 0, 1, '=');
    extract(tbuf, strValue, 1, 1, '=');

    if (strcasecmp(strKey, "DBVERSION"))
    {
        // Pre-4.0 database
    }
    else
    {
        // 4.0 and later
        strcpy(tbuf, getstr(fp));       // Beginning ! mark
    }

    if (*tbuf == EOF)
    {
        ok = 0;
    }
    else
    {
        ok = 1;
        pNewClass = new CHSShipClass(0);
    }

    HS_BOOL8 bHasID = false;
    while (ok)
    {
        strcpy(tbuf, getstr(fp));
        if (*tbuf == '!')
        {
            // Insert the previously loaded class.
            if (pNewClass)
            {
                // If this class doesn't have an ID, then we need
                // to assign one.  A missing ID is either a corrupt
                // database, or an older one that doesn't have IDs
                // written to it.
                if (!bHasID)
                {
                    pNewClass->Id(GetNextClassID(true));
                    hs_log(hsInterface.
                           HSPrintf
                           ("WARNING: Ship class loaded with ID, assigning ID %d.",
                            pNewClass->Id()));
                }
                m_mapShipClasses[pNewClass->Id()] = pNewClass;
            }

            pNewClass = new CHSShipClass(0);
            bHasID = false;
            continue;
        }
        else if (!strcasecmp(tbuf, "*END*"))
        {
            if (pNewClass)
            {
                m_mapShipClasses[pNewClass->Id()] = pNewClass;
            }

            ok = 0;
            continue;
        }

        // Must be a key/value pair, so work with that.
        extract(tbuf, strKey, 0, 1, '=');
        extract(tbuf, strValue, 1, 1, '=');

        // Figure out the key type, and see what we can
        // do with it.
        if (!strcasecmp(strKey, "ID"))
        {
            pNewClass->Id(atoi(strValue));

            // Lock this class ID.
            LockClassID(pNewClass->Id());
            bHasID = true;
        }
        else if (!strcasecmp(strKey, "NAME"))
        {
            pNewClass->ClassName(strValue);
        }
        else if (!strcasecmp(strKey, "DROP CAPABLE"))
        {
            pNewClass->CanDrop(atoi(strValue) == 0 ? false : true);
        }
        else if (!strcasecmp(strKey, "SPACEDOCK"))
        {
            pNewClass->SpaceDock(atoi(strValue) == 0 ? false : true);
        }
        else if (!strcasecmp(strKey, "CARGOSIZE"))
        {
            pNewClass->CargoSize(atoi(strValue));
        }
        else if (!strcasecmp(strKey, "MINMANNED"))
        {
            pNewClass->MinCrew(atoi(strValue));
        }
        else if (!strcasecmp(strKey, "MAXHULL"))
        {
            pNewClass->MaxHull(atoi(strValue));
        }
        else if (!strcasecmp(strKey, "SIZE"))
        {
            pNewClass->Size(atoi(strValue));
        }
        else if (!strcasecmp(strKey, "SYSTEMDEF"))
        {
            if (!pNewClass->m_pSystems)
            {
                pNewClass->m_pSystems = new CHSSystemArray;
            }

            cSys = LoadSystem(fp, pNewClass);
            if (cSys)
            {
                pNewClass->m_pSystems->AddSystem(cSys);
            }
        }
    }
    fclose(fp);

    sprintf(lbuf, "LOADING: %s - %d classes loaded (done)", pcPath,
            m_mapShipClasses.size());
    hs_log(lbuf);

    return true;
}

// Prints information about all of the loaded classes to
// the specified player.
void CHSClassDBDef::PrintInfo(int player)
{
    if (GetNumClasses() == 0)
    {
        hsInterface.Notify((int) player, (char *) "No ship classes loaded.");
        return;
    }

    // Print the header
    hsInterface.Notify(player, "[ID ] Name");

    // Print these out in numerical order.
    unsigned int idx;
    for (idx = 0; idx < m_uiMaxClassID; idx++)
    {
        CSTLShipClassMap::iterator iter;

        if ((iter = m_mapShipClasses.find(idx)) != m_mapShipClasses.end())
        {
            CHSShipClass *pClass = (*iter).second;

            hsInterface.Notify(player,
                               hsInterface.HSPrintf("[%3d] %s", idx,
                                                    pClass->ClassName()));
        }
    }
}


// When the LoadFromFile() encounters a SYSTEMDEF key, it
// calls upon this function to load the system information
// from the file, construct a CHSEngSystem of some type, load
// it with information, and return that system.
CHSEngSystem *CHSClassDBDef::LoadSystem(FILE * fp, CHSShipClass * pClass)
{
    CHSEngSystem *cSys;
    char strKey[64];
    char strValue[64];
    char *ptr;
    char tbuf[256];
    HSS_TYPE type;

    cSys = NULL;

    // We start here, after LoadFromFile() already encountered
    // the SYSTEMDEF keyword.  We figure out the type of system,
    // which should be the first keyword, and then we load
    // according to that.
    // 
    // We have NO WAY of knowing what kind of information each
    // system needs.  We ONLY know what types of systems exist
    // because this is defined in hseng.h.  Thus, we simply
    // figure out the type of system, and then pass the key/value
    // information to the system as if it were setting attributes.
    while (fgets(tbuf, 256, fp))
    {
        // Truncate newlines
        if ((ptr = strchr(tbuf, '\n')) != NULL)
            *ptr = '\0';
        if ((ptr = strchr(tbuf, '\r')) != NULL)
            *ptr = '\0';

        extract(tbuf, strKey, 0, 1, '=');
        extract(tbuf, strValue, 1, 1, '=');

        // Check for end of database .. would be an error
        if (!strcmp(strKey, "*END*"))
        {
            hs_log
                ("ERROR: Encountered end of classdb before loading specified system list.");
            break;
        }

        if (!strcmp(strKey, "SYSTYPE"))
        {
            type = (HSS_TYPE) atoi(strValue);

            cSys = CHSEngSystem::CreateFromType(type);

            if (!cSys)
            {
                sprintf(tbuf,
                        "ERROR: Invalid system type %d encountered.", type);
                hs_log(tbuf);
            }
        }
        else if (!strcmp(strKey, "SYSTEMEND"))
        {
            return cSys;
        }
        else
        {
            // Simply call upon the object to set it's
            // own attributes.  We don't know what they
            // are.
            if (cSys)
            {
                // Check to see if this system supports this attribute.
                if (!cSys->SetAttributeValue(strKey, strValue))
                {
                    char cError[128];

                    sprintf(cError,
                            "Failed attribute load on system.  Attribute: %s\n",
                            strKey);
                    hs_log(cError);
                }
            }
        }
    }

    return cSys;
}

// Saves class information to the class database.  To do this,
// we just run down the list of classes, outputting some standard
// variables, and then running down the system array for each
// class, using the Sys->SaveToFile() to output system information.
void CHSClassDBDef::SaveToFile(char *lpstrPath)
{
    FILE *fp;
    char tbuf[512];

    // Is the file path good?
    if (!lpstrPath || strlen(lpstrPath) == 0)
    {
        hs_log
            ("Attempt to save the ship class database, but no path specified.  Could be a config file problem.");
        return;
    }

    // Verify we have any classes
    if (m_mapShipClasses.size() == 0)
    {
        return;
    }

    // Open output file.
    fp = fopen(lpstrPath, "w");
    if (!fp)
    {
        sprintf(tbuf, "ERROR: Unable to write classes to %s.", lpstrPath);
        hs_log(tbuf);
        return;
    }

    // Write out the database version
    fprintf(fp, "DBVERSION=4.1\n");

    // Ok.  Now just run through the classes, outputting information.
    CSTLShipClassMap::iterator iter;
    for (iter = m_mapShipClasses.begin(); iter != m_mapShipClasses.end();
         iter++)
    {
        CHSShipClass *pClass = (*iter).second;

        // Output standard variables for each class
        fprintf(fp, "!\n");
        fprintf(fp, "ID=%d\n", pClass->Id());
        fprintf(fp, "NAME=%s\n", pClass->ClassName());
        fprintf(fp, "CARGOSIZE=%d\n", pClass->CargoSize());
        fprintf(fp, "MINMANNED=%d\n", pClass->MinCrew());
        fprintf(fp, "SIZE=%d\n", pClass->Size());
        fprintf(fp, "MAXHULL=%d\n", pClass->MaxHull());
        fprintf(fp, "DROP CAPABLE=%d\n", pClass->CanDrop());
        fprintf(fp, "SPACEDOCK=%d\n", pClass->SpaceDock());

        // Now output system information
        if (pClass->m_pSystems)
        {
            pClass->m_pSystems->SaveToFile(fp);
        }
    }
    fprintf(fp, "*END*\n");
    fclose(fp);
}

CHSShipClass *CHSClassDBDef::GetClass(HS_UINT32 uiClassID)
{
    CSTLShipClassMap::iterator iter;

    if ((iter = m_mapShipClasses.find(uiClassID)) != m_mapShipClasses.end())
    {
        return (*iter).second;
    }

    return NULL;
}

// Attempts to load a picture from the account into the
// supplied buffer.
HS_BOOL8 CHSClassDBDef::LoadClassPicture(int iClass, char **buff)
{
    FILE *fp;
    char tbuf[512];
    char *ptr;
    int idx;

    sprintf(tbuf, "%s/class_%d.pic", HSCONF.picture_dir, iClass);
    fp = fopen(tbuf, "r");
    if (!fp)
        return false;

    // Load the file
    idx = 0;
    while (fgets(tbuf, 512, fp))
    {
        if ((ptr = strchr(tbuf, '\n')) != NULL)
            *ptr = '\0';
        if ((ptr = strchr(tbuf, '\r')) != NULL)
            *ptr = '\0';
        buff[idx] = new char[strlen(tbuf) + 1];
        strcpy(buff[idx], tbuf);
        idx++;
    }
    buff[idx] = NULL;

    return true;
}


HS_BOOL8 CHSClassDBDef::GetFirstClass(THSShipClassIterator & rtIter)
{
    rtIter.iter = m_mapShipClasses.begin();

    return (GetNextClass(rtIter));
}

HS_BOOL8 CHSClassDBDef::GetNextClass(THSShipClassIterator & rtIter)
{
    if (rtIter.iter == m_mapShipClasses.end())
    {
        return false;
    }

    rtIter.pValue = rtIter.iter->second;

    rtIter.iter++;

    return true;
}
