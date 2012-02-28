// -----------------------------------------------------------------------
// $Id: hsuniversedb.cpp,v 1.7 2006/04/04 12:41:11 mark Exp $
// -----------------------------------------------------------------------
#include "pch.h"

#include <stdlib.h>

#include "hstypes.h"
#include "hsuniversedb.h"
#include "hsuniverse.h"
#include "hsinterface.h"
#include "hsutils.h"
#include "hsflags.h"

CHSUniverseDB myCHSUniverseDB;

CHSUniverseDBDef::CHSUniverseDBDef()
{
    m_iterCurUniverse = m_mapUniverses.begin();
}

CHSUniverseDBDef::~CHSUniverseDBDef()
{
    CSTLUniverseMap::iterator iter;

    // Free up the universes we've loaded.
    for (iter = m_mapUniverses.begin(); iter != m_mapUniverses.end(); iter++)
    {
        CHSUniverse *pUniverse;

        pUniverse = (*iter).second;

        if (pUniverse)
        {
            pUniverse->Release();
        }
    }
}

// Prints information about all of the universes in the array
// to a given player.
void CHSUniverseDBDef::PrintInfo(int player)
{
    if (m_mapUniverses.empty())
    {
        hsInterface.Notify(player, "No universes currently loaded.");
        return;
    }

    hsInterface.Notify(player,
                       "[Room#] Name                            Objects    Active");

    CSTLUniverseMap::iterator iter;
    for (iter = m_mapUniverses.begin(); iter != m_mapUniverses.end(); iter++)
    {
        CHSUniverse *pUniverse = iter->second;
        if (pUniverse)
        {
            hsInterface.Notify(player,
                               hsInterface.
                               HSPrintf("[%5d] %-32s%4d       %4d",
                                        pUniverse->GetID(),
                                        pUniverse->GetName(),
                                        pUniverse->GetNumObjects(),
                                        pUniverse->GetNumActiveObjects()));
        }
    }
}

HS_BOOL8 CHSUniverseDBDef::GetFirstUniverse(THSUniverseIterator & rtIter)
{
    rtIter.iter = m_mapUniverses.begin();

    return (GetNextUniverse(rtIter));
}

HS_BOOL8 CHSUniverseDBDef::GetNextUniverse(THSUniverseIterator & rtIter)
{
    if (rtIter.iter == m_mapUniverses.end())
    {
        return false;
    }

    rtIter.pValue = rtIter.iter->second;

    rtIter.iter++;

    return true;
}

HS_BOOL8 CHSUniverseDBDef::AddUniverse(CHSUniverse * pUniverse)
{
    // Make sure we don't already have a universe with this ID.
    HS_UINT32 uiUID = pUniverse->GetID();

    CSTLUniverseMap::iterator iter;

    if ((iter = m_mapUniverses.find(uiUID)) != m_mapUniverses.end())
    {
        // Already found a universe with this ID.
        return false;
    }

    // Ok to insert.
    m_mapUniverses[uiUID] = pUniverse;

    return true;
}

// Returns a pointer to a CHSUniverse given a universe HS_DBREF.
CHSUniverse *CHSUniverseDBDef::FindUniverse(HS_UINT32 uiID)
{
    CSTLUniverseMap::iterator iter;
    CHSUniverse *pUniverse;

    if ((iter = m_mapUniverses.find(uiID)) != m_mapUniverses.end())
    {
        pUniverse = (*iter).second;

        // This is the one.
        return pUniverse;
    }
    // Didn't find it.
    return NULL;
}

// Loads universes from the universe db.  It does this by
// loading each line in the db and splitting it up into
// a key/value pair.
HS_BOOL8 CHSUniverseDBDef::LoadFromFile(const char *lpstrPath)
{
    char tbuf[256];
    char strKey[256];
    char strValue[256];
    FILE *fp;
    char *ptr;
    CHSUniverse *pNewUniverse = NULL;

    sprintf(tbuf, "LOADING: %s", lpstrPath);
    hs_log(tbuf);

    fp = fopen(lpstrPath, "r");
    if (!fp)
    {
        hs_log("ERROR: Unable to open object database for loading!");
        return false;
    }

    // Load in the lines from the database, splitting the
    // line into key/value pairs.  Each OBJNUM key specifies
    // the beginning of a new universe definition.
    while (fgets(tbuf, 256, fp))
    {
        // Truncate newline chars
        if ((ptr = strchr(tbuf, '\n')) != NULL)
            *ptr = '\0';
        if ((ptr = strchr(tbuf, '\r')) != NULL)
            *ptr = '\0';

        // Check for end of DB
        if (!strcmp(tbuf, "*END*"))
            break;

        // Extract key and value
        extract(tbuf, strKey, 0, 1, '=');
        extract(tbuf, strValue, 1, 1, '=');

        // Determine key type, then do something.
        if (!strcmp(strKey, "DBVERSION"))
        {
        }
        else if (!strcmp(strKey, "OBJNUM"))
        {
            pNewUniverse = new CHSUniverse;

            HS_UINT32 uiID = atoi(strValue);
            if (hsInterface.ValidObject(uiID) == false)
            {
                return false;
            }

            pNewUniverse->SetID(uiID);
            hsInterface.SetToggle(uiID, ROOM_HSPACE_UNIVERSE);

            m_mapUniverses[uiID] = pNewUniverse;
        }
        else
        {
            // Just try to set the attribute on the universe.
            if (pNewUniverse
                && !pNewUniverse->SetAttributeValue(strKey, strValue))
            {
                sprintf(tbuf,
                        "WARNING: Key \"%s\" encountered but not handled.",
                        strKey);
                hs_log(tbuf);
            }
        }
    }

    fclose(fp);
    sprintf(tbuf, "LOADING: %d universes loaded.", m_mapUniverses.size());
    hs_log(tbuf);

    return true;
}

HS_BOOL8 CHSUniverseDBDef::DeleteUniverse(HS_UINT32 uid)
{
    CSTLUniverseMap::iterator iter;
    CHSUniverse *pUniverse = NULL;

    if ((iter = m_mapUniverses.find(uid)) != m_mapUniverses.end())
    {
        pUniverse = (*iter).second;
    }

    if (!pUniverse)
    {
        return false;
    }

    // Check to see if the universe is empty
    if (!pUniverse->IsEmpty())
    {
        return false;
    }

    // Delete it
    pUniverse->Release();

    m_mapUniverses.erase(iter);

    return true;
}

// Runs through the list of universes, telling them to
// save to the specified file.
void CHSUniverseDBDef::SaveToFile(const char *lpstrPath)
{
    FILE *fp;
    char tbuf[256];

    // Is the file path good?
    if (!lpstrPath || strlen(lpstrPath) == 0)
    {
        hs_log
            ("Attempt to save the universe database, but no path specified.  Could be a config file problem.");
        return;
    }

    // Any universes to save?
    if (m_mapUniverses.size() == 0)
    {
        return;
    }

    // Open the database
    fp = fopen(lpstrPath, "w");
    if (!fp)
    {
        sprintf(tbuf, "ERROR: Unable to write universes to %s.", lpstrPath);
        hs_log(tbuf);
        return;
    }

    // Print dbversion
    fprintf(fp, "DBVERSION=4.0\n");

    // Save all of the universes.
    CSTLUniverseMap::iterator iter;

    for (iter = m_mapUniverses.begin(); iter != m_mapUniverses.end(); iter++)
    {
        CHSUniverse *pUniverse;

        pUniverse = (*iter).second;

        if (pUniverse)
        {
            pUniverse->SaveToFile(fp);
        }
    }
    fprintf(fp, "*END*\n");
    fclose(fp);
}
