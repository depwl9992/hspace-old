// -----------------------------------------------------------------------
// $Id: hsuniverse.cpp,v 1.8 2006/04/04 12:41:11 mark Exp $
// -----------------------------------------------------------------------

#include "pch.h"

#include <stdlib.h>
#include <cstring>

#include "hsuniverse.h"
#include "hsutils.h"
#include "hspace.h"
#include "hsconf.h"
#include "hsflags.h"
#include "hscelestial.h"
#include "hsinterface.h"
#include "hssensors.h"


CHSUniverse::CHSUniverse()
{
}

CHSUniverse::~CHSUniverse()
{
    CSTLHSObjectList::iterator iter;

    // Free up all of the objects.
    for (iter = m_listObjects.begin(); iter != m_listObjects.end(); iter++)
    {
        CHS3DObject *pObject = (*iter);

        if (pObject)
        {
            pObject->Release();
        }
    }
}

// Used to add a celestial to the list of celestials.  The
// celestial first exists in the object array.  It's here 
// so that we can specifically search for just a celestial
// if we want.
HS_BOOL8 CHSUniverse::AddCelestial(CHSCelestial * cObj)
{
    m_listCelestials.push_back(static_cast < CHS3DObject * >(cObj));
    return true;
}

// This function is protected because any caller should call
// the generic AddObject() function, which will then call this
// function.  This function, then, adds a ship (if possible)
// to the universe ship list.
HS_BOOL8 CHSUniverse::AddShip(CHSShip * cObj)
{
    m_listShips.push_back(cObj);
    return true;
}

// Finds a given object with a HS_DBREF number
CHS3DObject *CHSUniverse::FindObject(int objnum, HS_TYPE type)
{
    CSTLHSObjectList *pList = NULL;

    switch (type)
    {
    case HST_SHIP:
        {
            pList = &m_listShips;
        }
        break;

    case HST_PLANET:
    case HST_WORMHOLE:
    case HST_NEBULA:
    case HST_ASTEROID:
    case HST_BLACKHOLE:
        {
            pList = &m_listCelestials;
        }
        break;

    default:                   // Generic object
        {
            pList = &m_listObjects;
        }
        break;
    }

    if (pList)
    {
        CSTLHSObjectList::iterator iter;

        for (iter = pList->begin(); iter != pList->end(); iter++)
        {
            CHS3DObject *pObject = *iter;

            if (pObject && pObject->GetDbref() == objnum)
            {
                return pObject;
            }
        }
    }
    return NULL;
}

// Call this function to remove an object from the
// "active" list for the universe, which stores all
// objects that aren't docked, aren't dropped, etc.
void CHSUniverse::RemoveActiveObject(CHS3DObject * cObj)
{
    CSTLHSObjectList::iterator iter;

    // Simply run down the list until we find the
    // object.  Then yank it.
    iter = m_listActiveObjects.begin();
    while (iter != m_listActiveObjects.end())
    {
        CHS3DObject *pObject = *iter;

        // If this is the object being removed, remove it.
        // Otherwise, force its removal from the sensors
        // of other objects.
        if (pObject == cObj)
        {
            m_listActiveObjects.erase(iter);

            // Have to start over so we don't kill the iteration.
            iter = m_listActiveObjects.begin();
            continue;
        }
        else
        {
            CHSSysSensors *pSensors;
            pSensors = (CHSSysSensors *) pObject->GetEngSystem(HSS_SENSORS);
            if (pSensors)
            {
                pSensors->LoseObject(cObj);
            }
        }

        iter++;
    }
}

// This is a public function that should be called to
// remove ANY object from the universe.
void CHSUniverse::RemoveObject(CHS3DObject * cObj)
{
    CSTLHSObjectList::iterator iter;

    // Determine the type of object, then remove from the appropriate
    // list.
    switch (cObj->GetType())
    {
    case HST_SHIP:
        {
            for (iter = m_listShips.begin(); iter != m_listShips.end();
                 iter++)
            {
                CHS3DObject *pObject = *iter;

                if (pObject == cObj)
                {
                    m_listShips.erase(iter);
                    break;
                }
            }
        }
        break;

    case HST_PLANET:
        {
            for (iter = m_listCelestials.begin();
                 iter != m_listCelestials.end(); iter++)
            {
                CHS3DObject *pObject = *iter;

                if (pObject == cObj)
                {
                    m_listCelestials.erase(iter);
                    break;
                }
            }
        }
        break;
        // Do nothing with these types
    case HST_NOTYPE:
    case HST_MISSILE:
    case HST_WORMHOLE:
    case HST_BLACKHOLE:
    case HST_NEBULA:
    case HST_ASTEROID:
    default:
        break;
    }

    for (iter = m_listObjects.begin(); iter != m_listObjects.end(); iter++)
    {
        CHS3DObject *pObject = *iter;

        if (pObject == cObj)
        {
            // Here's our man.
            RemoveActiveObject(pObject);

            m_listObjects.erase(iter);
            break;
        }
    }
}

// Adds an object to the active list of objects.
HS_BOOL8 CHSUniverse::AddActiveObject(CHS3DObject * cObj)
{
    // Be sure the object isn't already in the active list.
    CSTLHSObjectList::iterator iter;

    for (iter = m_listActiveObjects.begin();
         iter != m_listActiveObjects.end(); iter++)
    {
        if (*iter == cObj)
        {
            return true;        // It's already in there, so what.
        }
    }

    m_listActiveObjects.push_back(cObj);

    return true;
}

// Allows any object to be generically added to the universe.
// The function determines the type of object and then adds
// to the appropriate list.
HS_BOOL8 CHSUniverse::AddObject(CHS3DObject * cObj)
{
    // Add it to the object list first.
    m_listObjects.push_back(cObj);

    // Add it to the active list?
    if (cObj->IsActive())
    {
        AddActiveObject(cObj);
    }

    // Add it to the type-specific list for higher efficiency.
    switch (cObj->GetType())
    {
    case HST_SHIP:
        {
            m_listShips.push_back(cObj);
        }
        break;

    case HST_PLANET:
        {
            m_listCelestials.push_back(cObj);
        }
        break;
    case HST_NOTYPE:
    case HST_MISSILE:
    case HST_WORMHOLE:
    case HST_BLACKHOLE:
    case HST_NEBULA:
    case HST_ASTEROID:
    default:
        break;
    }
    return true;
}

// Return number of objects, perhaps for a specific type.
HS_UINT32 CHSUniverse::GetNumObjects(HS_TYPE type)
{
    switch (type)
    {
    case HST_SHIP:
        return m_listShips.size();

    case HST_PLANET:
    case HST_WORMHOLE:
    case HST_NEBULA:
    case HST_ASTEROID:
    case HST_BLACKHOLE:
        return m_listCelestials.size();

    default:
        return m_listObjects.size();
    }
}

HS_BOOL8 CHSUniverse::GetFirstObject(THSObjectIterator & rtValue,
                                     HS_TYPE eType)
{
    switch (eType)
    {
    case HST_SHIP:
        rtValue.pSearchList = &m_listShips;
        break;

    case HST_PLANET:
    case HST_WORMHOLE:
    case HST_NEBULA:
    case HST_BLACKHOLE:
    case HST_ASTEROID:
        rtValue.pSearchList = &m_listCelestials;
        break;

    default:
        rtValue.pSearchList = &m_listObjects;
        break;
    }

    rtValue.iter = rtValue.pSearchList->begin();
    rtValue.eSearchType = eType;

    return (GetNextObject(rtValue, eType));
}

HS_BOOL8 CHSUniverse::GetNextObject(THSObjectIterator & rtValue,
                                    HS_TYPE eType)
{
    if (eType != rtValue.eSearchType)
    {
        // Trying to call GetNextObject() on a different type than GetFirstObject().
        return false;
    }

    if (rtValue.iter == rtValue.pSearchList->end())
    {
        return false;
    }

    // If the current object is not of the specified type, advance to the next
    // object of the type.
    CHS3DObject *pObject = *rtValue.iter;
    if (eType != HST_NOTYPE)
    {
        while (pObject->GetType() != eType)
        {
            rtValue.iter++;

            if (rtValue.iter == rtValue.pSearchList->end())
            {
                break;
            }

            pObject = *rtValue.iter;
        }
    }

    if (rtValue.iter == rtValue.pSearchList->end())
    {
        return false;
    }

    rtValue.pValue = *rtValue.iter;

    rtValue.iter++;

    return true;
}

CHS3DObject *CHSUniverse::GetFirstActiveObject()
{
    m_iterCurActiveObject = m_listActiveObjects.begin();

    return (GetNextActiveObject());
}

CHS3DObject *CHSUniverse::GetNextActiveObject()
{
    if (m_iterCurActiveObject == m_listActiveObjects.end())
    {
        return NULL;
    }

    CHS3DObject *pObject = *m_iterCurActiveObject;

    m_iterCurActiveObject++;

    return pObject;
}

// Returns the number of active objects, perhaps for a specific type.
HS_UINT32 CHSUniverse::GetNumActiveObjects(HS_TYPE eType)
{
    CSTLHSObjectList::iterator iter;
    HS_UINT32 num;

    num = 0;
    for (iter = m_listActiveObjects.begin();
         iter != m_listActiveObjects.end(); iter++)
    {
        if (eType == HST_NOTYPE)
        {
            num++;
        }
        else if ((*iter)->GetType() == eType)
        {
            num++;
        }
    }
    return num;
}


// Sends a message to every object within a given distance
// from the supplied CHS3DObject.
void CHSUniverse::SendMessage(const char *strMsg,
                              int iDist, CHS3DObject * pSource)
{
    double sX, sY, sZ;          // Source coords
    double tX, tY, tZ;          // Target coords
    double dDistance;

    // If a CHS3DObject was supplied, get its coords.
    if (pSource)
    {
        sX = pSource->GetX();
        sY = pSource->GetY();
        sZ = pSource->GetZ();
    }
    else
        sX = sY = sZ = 0;

    // Run through all active objects, checking distance
    CSTLHSObjectList::iterator iter;
    for (iter = m_listActiveObjects.begin();
         iter != m_listActiveObjects.end(); iter++)
    {
        CHS3DObject *pObject = *iter;

        if (!pObject)
        {
            continue;
        }

        // Make sure it's not the source object
        if (pObject->GetDbref() == pObject->GetDbref())
        {
            continue;
        }

        // Calculate distance between coords, if needed
        if (iDist > 0)
        {
            tX = pObject->GetX();
            tY = pObject->GetY();
            tZ = pObject->GetZ();

            dDistance = Dist3D(sX, sY, sZ, tX, tY, tZ);

            if (dDistance > iDist)
                continue;
        }

        // At this point, the distance is ok.  Give the
        // message
        pObject->HandleMessage(strMsg, MSG_GENERAL, (long *) pSource);
    }
}

// Sends a message to every ship that has the supplied
// CHS3DObject on sensors with the given status.
void CHSUniverse::SendContactMessage(const char *strMsg,
                                     int status, CHS3DObject * pSource)
{
    CHSShip *pShip;
    SENSOR_CONTACT *pContact;

    // Run through all active objects, checking distance
    CSTLHSObjectList::iterator iter;
    for (iter = m_listActiveObjects.begin();
         iter != m_listActiveObjects.end(); iter++)
    {
        CHS3DObject *pObject = *iter;

        if (!pObject)
        {
            continue;
        }

        // Is it an object with sensors?
        if (pObject->GetType() != HST_SHIP)
        {
            continue;
        }

        pShip = static_cast < CHSShip * >(pObject);

        // Grab the sensor contact for the source object
        // in the sensor array of the target object.
        pContact = pShip->GetSensorContact(pSource);

        if (pContact && (pContact->status == status))
        {
            pObject->HandleMessage(strMsg, MSG_GENERAL, (long *) pSource);
        }
    }
}

// Allows an attribute with a given name to be set to
// a specified value.
HS_BOOL8 CHSUniverse::SetAttributeValue(const char *pcKey,
                                        const char *pcValue)
{
    // Try to match the attribute name
    if (!_stricmp(pcKey, "NAME"))
    {
        if (pcValue)
        {
            m_strName = pcValue;
        }
        else
        {
            m_strName = "";
        }
        return true;
    }
    else if (!_stricmp(pcKey, "OBJNUM"))
    {
        m_uiID = strtodbref(pcValue);
        return true;
    }
    return false;
}

// Tells the universe to save its information to the
// specified file stream.
void CHSUniverse::SaveToFile(FILE * fp)
{
    if (!fp)
    {
        return;
    }

    if (!hsInterface.HasFlag(m_uiID, TYPE_ROOM, ROOM_HSPACE_UNIVERSE))
    {
        return;
    }

    // Write out our attrs
    fprintf(fp, "OBJNUM=%d\n", m_uiID);
    fprintf(fp, "NAME=%s\n", m_strName.c_str());
}
