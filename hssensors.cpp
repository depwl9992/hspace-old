// -----------------------------------------------------------------------
// $Id: hssensors.cpp,v 1.9 2006/04/04 12:41:11 mark Exp $
// -----------------------------------------------------------------------

#include "pch.h"

#include "hsobjects.h"
#include "hsinterface.h"
#include "hsconf.h"
#include "hsutils.h"
#include "hsuniverse.h"
#include "hscelestial.h"
#include "hsautopilot.h"
#include "hssensors.h"

CHSSysSensors::CHSSysSensors(void)
{
    int idx;

    SetType(HSS_SENSORS);

    m_puiSensorRating = NULL;
    m_nContacts = 0;

    // Initialize contact arrays
    for (idx = 0; idx < HS_MAX_CONTACTS; idx++)
        m_contact_list[idx] = NULL;

    for (idx = 0; idx < HS_MAX_NEW_CONTACTS; idx++)
    {
        m_new_contacts[idx] = NULL;
        m_lost_contacts[idx] = NULL;
    }
}

// Outputs system information to a file.  This could be
// the class database or another database of objects that
// contain systems.
void CHSSysSensors::SaveToFile(FILE * fp)
{
    // Call base class FIRST
    CHSEngSystem::SaveToFile(fp);

    // Ok to output, so we print out our information
    // specific to this system.
    if (m_puiSensorRating)
        fprintf(fp, "SENSOR RATING=%d\n", *m_puiSensorRating);
}

// This just sets the index to the first lost contact slot.
// Always use GetNextLostContact() to retrieve the first and
// all subsequent contacts.
SENSOR_CONTACT *CHSSysSensors::GetFirstLostContact(void)
{
    for (uContactSlot = 0; uContactSlot < HS_MAX_NEW_CONTACTS; uContactSlot++)
    {
        if (m_lost_contacts[uContactSlot])
            return m_lost_contacts[uContactSlot];
    }
    return NULL;
}

SENSOR_CONTACT *CHSSysSensors::GetNextLostContact(void)
{
    while (uContactSlot < HS_MAX_NEW_CONTACTS)
    {
        if (m_lost_contacts[uContactSlot])
        {
            uContactSlot++;
            return m_lost_contacts[uContactSlot - 1];
        }
        uContactSlot++;
    }
    return NULL;
}

// This just sets the index to the first new contact slot.
// Always use GetNextNewContact() to retrieve the first and
// all subsequent contacts.
SENSOR_CONTACT *CHSSysSensors::GetFirstNewContact(void)
{
    for (uContactSlot = 0; uContactSlot < HS_MAX_NEW_CONTACTS; uContactSlot++)
    {
        if (m_new_contacts[uContactSlot])
            return m_new_contacts[uContactSlot];
    }
    return NULL;
}

SENSOR_CONTACT *CHSSysSensors::GetNextNewContact(void)
{
    while (uContactSlot < HS_MAX_NEW_CONTACTS)
    {
        if (m_new_contacts[uContactSlot])
        {
            uContactSlot++;
            return m_new_contacts[uContactSlot - 1];
        }
        uContactSlot++;
    }
    return NULL;
}

void CHSSysSensors::ClearContacts(void)
{
    int idx;

    // Clear out our current contact array.
    for (idx = 0; idx < HS_MAX_CONTACTS; idx++)
    {
        m_contact_list[idx] = NULL;

        if (m_contact_list[idx])
        {
            delete m_contact_list[idx];
            m_contact_list[idx] = NULL;
        }
    }

    // Clear out any lost contacts that were moved
    // out of the current contact array.
    for (idx = 0; idx < HS_MAX_NEW_CONTACTS; idx++)
    {
        m_lost_contacts[idx] = NULL;

        if (m_lost_contacts[idx])
        {
            delete m_lost_contacts[idx];
            m_lost_contacts[idx] = NULL;
        }
    }

    m_nContacts = 0;
}

// This will handle sensor sweeping.
void CHSSysSensors::DoCycle(void)
{
    int idx;
    SENSOR_CONTACT *cContact;
    int iDetectLevel;           // 0, 1, or 2 for how to detect it.
    double MyX, MyY, MyZ;
    double NebX, NebY, NebZ;
    double TargetX, TargetY, TargetZ;
    double dIdentDist, dDetectDist, dDistance;
    float fCloakEffect;
    float fNebulaEffect;
    double dOverload;

    // Call the base class first to handle stress
    CHSEngSystem::DoCycle();

    // No power? Don't cycle.
    if (!GetCurrentPower())
    {
        // Check to be sure the user didn't turn off power to
        // save sensor contacts for a later time.
        if (m_nContacts > 0)
            ClearContacts();

        return;
    }

    // If we have no owner, then we're not sweeping
    // for contacts.
    if (!GetOwnerObject())
        return;

    // If the owner is not active, then we're not sweeping.
    if (!GetOwnerObject()->IsActive())
    {
        if (m_nContacts > 0)
        {
            ClearContacts();
        }
        return;
    }

    // If we belong to a ship, and we're in hyperspace,
    // figure out what to do.
    int iDetectCap;
    iDetectCap = 2;             // Fully sense by default
    if (GetOwnerObject() && GetOwnerObject()->GetType() == HST_SHIP)
    {
        CHSShip *cShip;
        cShip = (CHSShip *) GetOwnerObject();

        // Figure out if we should sweep at all
        if (cShip->InHyperspace())
        {
            if (HSCONF.sense_hypervessels == 0)
                iDetectCap = 0;
            else if (HSCONF.sense_hypervessels == 1)
                iDetectCap = 1;
        }
    }

    // Calculate overload (or underload)
    int iOptPower = GetOptimalPower(false);
    dOverload = GetCurrentPower() / (float) (iOptPower + .00001);

    // Clear out our new and lost contact arrays.
    for (idx = 0; idx < HS_MAX_NEW_CONTACTS; idx++)
    {
        m_new_contacts[idx] = NULL;

        if (m_lost_contacts[idx])
        {
            delete m_lost_contacts[idx];
            m_lost_contacts[idx] = NULL;
        }
    }

    // Flag all of our current contacts as unverified
    m_nContacts = 0;
    for (idx = 0; idx < HS_MAX_CONTACTS; idx++)
    {
        if (m_contact_list[idx])
        {
            m_nContacts++;
            m_contact_list[idx]->bVerified = false;

            // Change updated statuses to IDENTIFIED
            if (m_contact_list[idx]->status == UPDATED)
                m_contact_list[idx]->status = IDENTIFIED;
        }
    }

    // Look for objects in our universe

    CHSUniverse *pUniverse = GetOwnerObject()->GetUniverse();
    if (!pUniverse)
        return;

    // Run through all active objects, checking distance,
    // and seeing if they're on sensors.
    MyX = GetOwnerObject()->GetX();
    MyY = GetOwnerObject()->GetY();
    MyZ = GetOwnerObject()->GetZ();

    // Compile a list of objects that we might possibly detect.
    CHS3DObject *pTargetObj;
    CSTLHSObjectList listNebulae;
    for (pTargetObj = pUniverse->GetFirstActiveObject();
         iDetectCap && pTargetObj;
         pTargetObj = pUniverse->GetNextActiveObject())
    {
        if (pTargetObj->GetType() == HST_NEBULA)
        {
            listNebulae.push_back(pTargetObj);
        }
    }

    for (pTargetObj = pUniverse->GetFirstActiveObject();
         iDetectCap && pTargetObj;
         pTargetObj = pUniverse->GetNextActiveObject())
    {
        iDetectLevel = iDetectCap;      // Sense only to our ability

        // Object visible?
        if (!pTargetObj->IsVisible())
            continue;

        // Is it us?
        if (pTargetObj->GetDbref() == GetOwnerObject()->GetDbref())
            continue;

        // Grab target coordinates
        TargetX = pTargetObj->GetX();
        TargetY = pTargetObj->GetY();
        TargetZ = pTargetObj->GetZ();


        // If it's a ship, check for cloaking.
        if (pTargetObj->GetType() == HST_SHIP)
        {
            CHSShip *pTargetShip = static_cast < CHSShip * >(pTargetObj);
            CHSShip *pMyShip = static_cast < CHSShip * >(GetOwnerObject());

            // Check if the ship is in hyperspace and
            // how we should handle it.
            if (pTargetShip->InHyperspace())
            {
                // Determine what to do with the
                // contact.
                if (HSCONF.sense_hypervessels == 0)
                    continue;
                else if (HSCONF.sense_hypervessels == 1)
                    iDetectLevel = 1;
            }

            // Grab cloaking effect, where 1 is complete
            // visibility.
            fCloakEffect = pTargetShip->CloakingEffect();
            fCloakEffect += pMyShip->TachyonEffect();
            if (fCloakEffect > 1)
            {
                fCloakEffect = 1;
            }
        }
        else
        {
            // Just some object
            fCloakEffect = 1;
        }

        // Check to see if any of our detected targets are in a nebula.
        fNebulaEffect = 1;

        // See if there are any Nebulae near the vessel.
        CSTLHSObjectList::iterator iter;
        for (iter = listNebulae.begin(); iter != listNebulae.end(); iter++)
        {
            CHSNebula *pNebula = static_cast < CHSNebula * >(*iter);

            NebX = pNebula->GetX();
            NebY = pNebula->GetY();
            NebZ = pNebula->GetZ();

            dDistance = Dist3D(NebX, NebY, NebZ, TargetX, TargetY, TargetZ);

            if (dDistance > pNebula->GetSize() * 100)
                continue;

            // Calculate effect of Nebula on sensor report for our ship.
            fNebulaEffect =
                (float) (1 /
                         (static_cast <
                          CHSNebula * >(pNebula)->GetDensity() * 1.00));
        }

        // Calculate distance to object
        dDistance = Dist3D(MyX, MyY, MyZ, TargetX, TargetY, TargetZ);

        // Calculate ranges, including cloaking.
        dDetectDist =
            pTargetObj->GetSize() / 2.0 * fCloakEffect * fNebulaEffect *
            (HSCONF.detectdist * 2.0) * GetSensorRating();
        dIdentDist =
            pTargetObj->GetSize() / 2.0 * fCloakEffect * fNebulaEffect *
            (HSCONF.identdist * 2.0) * GetSensorRating();

        // Range is increased (or decreased) based on overloading.
        dDetectDist *= dOverload;
        dIdentDist *= dOverload;

        // Object out of range?
        if ((dDistance > dDetectDist) ||
            (dDistance > HSCONF.max_sensor_range))
        {
            // Do nothing. It'll get lost from sensors
            // simply because it hasn't been flagged
            // as verified.
        }
        else
        {
            // Now see if the object is currently on sensors.
            cContact = GetContact(pTargetObj);

            if (cContact)
            {
                // Check to see if we need to update its status.
                if ((cContact->status == DETECTED) &&
                    (dDistance <= dIdentDist) && (iDetectLevel == 2))
                {
                    UpdateContact(cContact);
                }

                // This contact is a sure bet.
                cContact->bVerified = true;
            }
            else
            {
                // New contact
                // Is it immediately in identification range?
                if ((dDistance <= dIdentDist) && (iDetectLevel == 2))
                    NewContact(pTargetObj, IDENTIFIED);
                else
                    NewContact(pTargetObj, DETECTED);

                m_nContacts++;
            }
        }
    }

    // Check for any unverified contacts
    for (idx = 0; idx < HS_MAX_CONTACTS; idx++)
    {
        if (m_contact_list[idx] && (!m_contact_list[idx]->bVerified))
        {
            LoseContact(m_contact_list[idx]);
            m_nContacts--;
        }
    }
}

// Returns the number of contacts on sensors.
HS_UINT32 CHSSysSensors::NumContacts(void)
{
    return m_nContacts;
}

HS_UINT32 CHSSysSensors::NumContacts(int tType)
{
    HS_UINT32 count = 0;
    int idx;

    for (idx = 0; idx < HS_MAX_CONTACTS; idx++)
    {
        if (m_contact_list[idx] != NULL &&
            m_contact_list[idx]->m_obj != NULL &&
            m_contact_list[idx]->m_obj->GetType() == tType)
            count++;
    }

    return count;
}

// Returns a 4-digit, random sensor id for a new contact.
// This id will not match any other on sensors.
HS_UINT32 CHSSysSensors::RandomSensorID(void)
{
    HS_UINT32 idx;
    HS_UINT32 num;
    HS_BOOL8 bInUse;

    bInUse = true;
    while (bInUse)
    {
        num = hsInterface.GetRandom(8999) + 1000;

        bInUse = false;
        // Make sure it's not in use
        for (idx = 0; idx < HS_MAX_CONTACTS; idx++)
        {
            if (m_contact_list[idx] && m_contact_list[idx]->m_id == num)
            {
                bInUse = true;
                break;
            }
        }
    }

    // It's ok.
    return num;
}

// Adds a new CHS3DObject to the list of contacts.
void CHSSysSensors::NewContact(CHS3DObject * cObj, CONTACT_STATUS stat)
{
    CHSShip *cShip;
    CHSSysAutoPilot *cAutoPilot;
    SENSOR_CONTACT *cNewContact;
    int idx;

    // Allocate a new contact object
    cNewContact = new SENSOR_CONTACT;

    cNewContact->m_obj = cObj;
    cNewContact->status = stat;
    cNewContact->m_id = RandomSensorID();

    // Now add it to the contact list and new contact list
    for (idx = 0; idx < HS_MAX_CONTACTS; idx++)
    {
        if (!m_contact_list[idx])
        {
            m_contact_list[idx] = cNewContact;
            break;
        }
    }

    // Error check
    if (idx == HS_MAX_CONTACTS)
    {
        hs_log
            ("WARNING: Sensor array has reached maximum number of contacts.");
        delete cNewContact;
    }

    // New contacts list
    for (idx = 0; idx < HS_MAX_NEW_CONTACTS; idx++)
    {
        if (!m_new_contacts[idx])
        {
            m_new_contacts[idx] = cNewContact;
            break;
        }
    }

    // The contact is definitely on sensors.
    cNewContact->bVerified = true;

    cShip = (CHSShip *) GetOwnerObject();
    if (!cShip || !(cShip->GetType() == HST_SHIP))
        return;

    cAutoPilot =
        (CHSSysAutoPilot *) cShip->GetSystems().GetSystem(HSS_AUTOPILOT);
    if (!cAutoPilot)
        return;

    cAutoPilot->ControlShip(CHSAI_AGGRESSION);
}

// Updates the status of an existing contact to indicate
// identified.  This is similar to obtaining a new contact.
void CHSSysSensors::UpdateContact(SENSOR_CONTACT * cContact)
{
    CHSShip *cShip;
    CHSSysAutoPilot *cAutoPilot;
    int idx;

    cContact->status = UPDATED;

    // Add it to the new list.  It's not really new, but
    // other functions can query it for the UPDATED status
    // to see that it's not really a new contact.
    for (idx = 0; idx < HS_MAX_NEW_CONTACTS; idx++)
    {
        if (!m_new_contacts[idx])
        {
            m_new_contacts[idx] = cContact;
            break;
        }
    }

    cShip = (CHSShip *) GetOwnerObject();
    if (!cShip || !(cShip->GetType() == HST_SHIP))
        return;

    cAutoPilot =
        (CHSSysAutoPilot *) cShip->GetSystems().GetSystem(HSS_AUTOPILOT);
    if (!cAutoPilot)
        return;

    cAutoPilot->ControlShip(CHSAI_AGGRESSION);

}

// Can be called externally to force the removal of an
// object from sensors.
void CHSSysSensors::LoseObject(CHS3DObject * cObj)
{
    CHSShip *cShip;
    CHSSysAutoPilot *cAutoPilot;
    int idx;

    // Run down the contact list, looking for this object.
    for (idx = 0; idx < HS_MAX_CONTACTS; idx++)
    {
        if (m_contact_list[idx])
        {
            CHS3DObject *cTmp;
            cTmp = m_contact_list[idx]->m_obj;
            if (cTmp->GetDbref() == cObj->GetDbref())
            {
                delete m_contact_list[idx];
                m_contact_list[idx] = NULL;
                break;
            }
        }
    }

    cShip = (CHSShip *) GetOwnerObject();
    if (!cShip || !(cShip->GetType() == HST_SHIP))
        return;

    cAutoPilot = (CHSSysAutoPilot *)
        cShip->GetSystems().GetSystem(HSS_AUTOPILOT);
    if (!cAutoPilot)
        return;

    cAutoPilot->ControlShip(CHSAI_AGGRESSION);

}

// Removes a contact from sensors and indicates that it's lost.
void CHSSysSensors::LoseContact(SENSOR_CONTACT * cContact)
{
    int idx;

    // Run down the contact list, looking for it.
    for (idx = 0; idx < HS_MAX_CONTACTS; idx++)
    {
        if (m_contact_list[idx] &&
            (m_contact_list[idx]->m_id == cContact->m_id))
        {
            m_contact_list[idx] = NULL;
            break;
        }
    }

    // Now add it to the lost contact list
    for (idx = 0; idx < HS_MAX_NEW_CONTACTS; idx++)
    {
        if (!m_lost_contacts[idx])
        {
            m_lost_contacts[idx] = cContact;
            break;
        }
    }
}

// Tries to find a sensor contact given a contact id
SENSOR_CONTACT *CHSSysSensors::GetContactByID(int id)
{
    int idx;

    for (idx = 0; idx < HS_MAX_CONTACTS; idx++)
    {
        if (!m_contact_list[idx])
            continue;

        if ((int) m_contact_list[idx]->m_id == id)
            return m_contact_list[idx];
    }

    // Not found
    return NULL;
}

// Tries to find a sensor contact given an object.
SENSOR_CONTACT *CHSSysSensors::GetContact(CHS3DObject * cObj)
{
    int idx = 0;

    for (idx = 0; idx < HS_MAX_CONTACTS; idx++)
    {
        if (!m_contact_list[idx])
            continue;

        // Compare dbrefs
        if (m_contact_list[idx]->m_obj->GetDbref() == cObj->GetDbref())
            return m_contact_list[idx];
    }
    return NULL;
}

// Returns the sensor contact in a given array slot.
SENSOR_CONTACT *CHSSysSensors::GetContact(int slot)
{
    if ((slot < 0) || (slot >= HS_MAX_CONTACTS))
        return NULL;

    return m_contact_list[slot];
}

// Sets a specific attribute value for the system.  This
// also allows system default values to be overridden at the
// ship level.
HS_BOOL8 CHSSysSensors::SetAttributeValue(const HS_INT8 * pcAttrName,
                                          const HS_INT8 * strValue)
{
    int iVal;

    // Match the name .. set the value
    if (!strcasecmp(pcAttrName, "SENSOR RATING"))
    {
        // If strValue contains a null, clear our local setting
        if (!*strValue)
        {
            if (m_puiSensorRating)
            {
                delete m_puiSensorRating;
                m_puiSensorRating = NULL;
            }
        }
        else
        {
            iVal = atoi(strValue);
            if (iVal < 0)
                return false;

            if (!m_puiSensorRating)
                m_puiSensorRating = new HS_UINT32;

            *m_puiSensorRating = iVal;
        }
        return true;
    }
    else
    {
        return CHSEngSystem::SetAttributeValue(pcAttrName, strValue);
    }
}


void CHSSysSensors::GetAttributeList(CHSAttributeList & rlistAttrs)
{
    // Call the base class first.
    CHSEngSystem::GetAttributeList(rlistAttrs);

    // Push our own attributes.
    rlistAttrs.push_back("SENSOR RATING");
}

HS_BOOL8
    CHSSysSensors::GetAttributeValue(const HS_INT8 * pcAttrName,
                                     CHSVariant & rvarValue,
                                     HS_BOOL8 bAdjusted, HS_BOOL8 bLocalOnly)
{
    // Determine attribute, and return the value.
    if (!strcasecmp(pcAttrName, "SENSOR RATING"))
    {
        if (m_puiSensorRating)
        {
            rvarValue = *m_puiSensorRating;
        }
        else if (!bLocalOnly)
        {
            rvarValue = GetSensorRating();
        }
        else
        {
            return false;
        }
        return true;
    }
    else
    {
        return CHSEngSystem::GetAttributeValue(pcAttrName, rvarValue,
                                               bAdjusted, bLocalOnly);
    }
}

HS_UINT32 CHSSysSensors::GetSensorRating(void)
{
    // Use some logic here.
    if (!m_puiSensorRating)
    {
        // Go to the parent's setting?
        if (!GetParent())
        {
            // No.  We are the default values.
            return 0;
        }
        else
        {
            CHSSysSensors *ptr = (CHSSysSensors *) GetParent();

            // Yes, this system exists on a ship, so
            // find the default values on the parent.
            return ptr->GetSensorRating();
        }
    }
    else
        return *m_puiSensorRating;
}
