// -----------------------------------------------------------------------
// $Id: hsthrusters.cpp,v 1.7 2006/04/04 12:41:11 mark Exp $
// -----------------------------------------------------------------------

#include "pch.h"

#include "hsobjects.h"
#include "hsship.h"
#include "hsconf.h"
#include "hsinterface.h"
#include "hsthrusters.h"

CHSSysThrusters::CHSSysThrusters()
{
    m_puiTurningRate = NULL;
    SetType(HSS_THRUSTERS);
}

CHSSysThrusters::~CHSSysThrusters()
{
    if (m_puiTurningRate)
    {
        delete m_puiTurningRate;
    }
}

void CHSSysThrusters::PowerUp(int level)
{
    if (!GetOwnerObject())
    {
        return;
    }

    if (GetOwnerObject()->GetType() == HST_SHIP)
    {
        CHSShip *cShip;

        cShip = (CHSShip *) GetOwnerObject();

        if (hsInterface.
            AtrGet(cShip->GetDbref(), "HSMSG_THRUSTERS_ACTIVATING"))
        {
            char *msg = hsInterface.EvalExpression(hsInterface.m_buffer,
                                                   cShip->GetDbref(),
                                                   cShip->GetDbref(),
                                                   cShip->GetDbref());
            cShip->NotifySrooms(msg);
        }
        else
        {
            cShip->NotifySrooms(HSCONF.thrusters_activating);
        }
    }
}

// Sets a specific attribute value for the system.  This
// also allows system default values to be overridden at the
// ship level.
HS_BOOL8 CHSSysThrusters::SetAttributeValue(const HS_INT8 * pcAttrName,
                                            const HS_INT8 * strValue)
{
    int iVal;

    // Match the name .. set the value
    if (!_stricmp(pcAttrName, "TURNING RATE"))
    {
        // If strValue contains a null, clear our local setting
        if (!*strValue)
        {
            if (m_puiTurningRate)
            {
                delete m_puiTurningRate;
                m_puiTurningRate = NULL;
            }
        }
        else
        {
            iVal = atoi(strValue);
            if (iVal < 0)
                return false;

            if (!m_puiTurningRate)
                m_puiTurningRate = new HS_UINT32;

            *m_puiTurningRate = iVal;
        }
        return true;
    }
    else
    {
        return CHSEngSystem::SetAttributeValue(pcAttrName, strValue);
    }
}

void CHSSysThrusters::GetAttributeList(CHSAttributeList & rlistAttrs)
{
    // Call the base class first.
    CHSEngSystem::GetAttributeList(rlistAttrs);

    // Push our own attributes.
    rlistAttrs.push_back("TURNING RATE");
}

HS_BOOL8
    CHSSysThrusters::GetAttributeValue(const HS_INT8 * pcAttrName,
                                       CHSVariant & rvarValue,
                                       HS_BOOL8 bAdjusted,
                                       HS_BOOL8 bLocalOnly)
{
    // Determine attribute, and return the value.
    if (!_stricmp(pcAttrName, "TURNING RATE"))
    {
        if (m_puiTurningRate)
        {
            rvarValue = *m_puiTurningRate;
        }
        else if (!bLocalOnly)
        {
            rvarValue = GetRate(bAdjusted);
        }
        else
        {
            return false;
        }
        return true;
    }
    else
    {
        // See if the base class knows about this attr.
        return CHSEngSystem::GetAttributeValue(pcAttrName, rvarValue,
                                               bAdjusted, bLocalOnly);
    }
}

// Returns the turning rate for the thrusters system.  If bAdjusted
// is set to false, then the maximum raw turning rate is returned,
// otherwise, it is adjusted for damage and power levels.
HS_UINT32 CHSSysThrusters::GetRate(HS_BOOL8 bAdjusted)
{
    double dmgperc;
    CHSSysThrusters *ptr;
    HS_UINT32 uVal;
    int iOptPower;

    // Use some logic here.
    if (!m_puiTurningRate)
    {
        // Go to the parent's setting?
        if (!GetParent())
        {
            // No.  We are the default values.
            return 0;
        }
        else
        {
            // Yes, this system exists on a ship, so
            // find the default values on the parent.
            ptr = (CHSSysThrusters *) GetParent();
            uVal = ptr->GetRate(false);
        }
    }
    else
        uVal = *m_puiTurningRate;


    // Make overloading and damage adjustments?
    if (bAdjusted)
    {
        float fVal;
        iOptPower = GetOptimalPower(false);
        fVal = (float) GetCurrentPower();
        fVal /= (float) iOptPower;
        uVal = (HS_UINT32) (uVal * fVal);

        // Figure in damage.  1/4 reduction per level of damage
        dmgperc = 1 - (.25 * GetDamageLevel());
        uVal = (HS_UINT32) (uVal * dmgperc);
    }
    return uVal;
}

// Outputs system information to a file.  This could be
// the class database or another database of objects that
// contain systems.
void CHSSysThrusters::SaveToFile(FILE * fp)
{
    // Call base class FIRST
    CHSEngSystem::SaveToFile(fp);

    // Now output our local values.
    if (m_puiTurningRate)
        fprintf(fp, "TURNING RATE=%d\n", *m_puiTurningRate);
}
