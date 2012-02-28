// -----------------------------------------------------------------------
//! $Id: hsjumpdrive.cpp,v 1.4 2006/12/16 01:25:01 mark Exp $
// -----------------------------------------------------------------------
#include "pch.h"

#ifndef WIN32
#include <strings.h>
#endif
#include <cstdio>
#include <stdlib.h>

#include "hstypes.h"
#include "hsobjects.h"
#include "hsship.h"
#include "hspace.h"
#include "hsansi.h"
#include "hsconf.h"
#include "hsinterface.h"
#include "hsjumpdrive.h"

//
// CHSJumpDrive stuff
//
CHSJumpDrive::CHSJumpDrive()
{
    SetType(HSS_JUMP_DRIVE);
    m_fChargeLevel = 0;
    m_bEngaged = false;
    m_puiMinJumpSpeed = NULL;
    m_puiEfficiency = NULL;
    m_uiSublightSpeed = 0;
    m_fuel_source = NULL;
    m_iJumpSpeedMultiplier = HSCONF.jump_speed_multiplier;
}

HS_BOOL8 CHSJumpDrive::SetAttributeValue(const HS_INT8 * pcAttrName,
                                         const HS_INT8 * strValue)
{
    if (!strcasecmp(pcAttrName, "CHARGE"))
    {
        m_fChargeLevel = (float) atof(strValue);
        return true;
    }
    else if (!strcasecmp(pcAttrName, "ENGAGED"))
    {
        m_bEngaged = atoi(strValue) == 0 ? false : true;
        return true;
    }
    else if (!strcasecmp(pcAttrName, "JUMPSPEED MULTIPLIER"))
    {
        m_iJumpSpeedMultiplier = atoi(strValue);
        return true;
    }
    else if (!strcasecmp(pcAttrName, "MIN SPEED"))
    {
        if (!m_puiMinJumpSpeed)
        {
            m_puiMinJumpSpeed = new HS_UINT32;
        }

        *m_puiMinJumpSpeed = atoi(strValue);
        return true;
    }
    else if (!strcasecmp(pcAttrName, "EFFICIENCY"))
    {
        if (!m_puiEfficiency)
        {
            m_puiEfficiency = new HS_UINT32;
        }

        *m_puiEfficiency = atoi(strValue);
        return true;
    }

    return CHSEngSystem::SetAttributeValue(pcAttrName, strValue);
}

void CHSJumpDrive::GetAttributeList(CHSAttributeList & rlistAttrs)
{
    // Call the base class first.
    CHSEngSystem::GetAttributeList(rlistAttrs);

    // Push our own attributes.
    rlistAttrs.push_back("ENGAGED");
    rlistAttrs.push_back("EFFICIENCY");
    rlistAttrs.push_back("MIN SPEED");
    rlistAttrs.push_back("CHARGE");
    rlistAttrs.push_back("JUMPSPEED MULTIPLIER");
}

HS_BOOL8 CHSJumpDrive::GetAttributeValue(const HS_INT8 * pcAttrName,
                                         CHSVariant & rvarValue,
                                         HS_BOOL8 bAdjusted,
                                         HS_BOOL8 bLocalOnly)
{
    // Determine attribute, and return the value.
    if (!strcasecmp(pcAttrName, "ENGAGED"))
    {
        rvarValue = m_bEngaged;
        return true;
    }
    else if (!strcasecmp(pcAttrName, "JUMPSPEED MULTIPLIER"))
    {
        rvarValue = m_iJumpSpeedMultiplier;
        return true;
    }
    else if (!strcasecmp(pcAttrName, "EFFICIENCY"))
    {
        if (m_puiEfficiency)
        {
            rvarValue = *m_puiEfficiency;
        }
        else if (!bLocalOnly)
        {
            rvarValue = GetEfficiency();
        }
        else
        {
            return false;
        }
        return true;
    }
    else if (!strcasecmp(pcAttrName, "MIN SPEED"))
    {
        if (m_puiMinJumpSpeed)
        {
            rvarValue = *m_puiMinJumpSpeed;
        }
        else if (!bLocalOnly)
        {
            rvarValue = GetMinJumpSpeed();
        }
        else
        {
            return false;
        }
        return true;
    }
    else if (!strcasecmp(pcAttrName, "CHARGE"))
    {
        rvarValue = m_fChargeLevel;
        return true;
    }
    else
    {
        return CHSEngSystem::GetAttributeValue(pcAttrName, rvarValue,
                                               bAdjusted, bLocalOnly);
    }
}

void CHSJumpDrive::SaveToFile(FILE * fp)
{
    // Save the base first
    CHSEngSystem::SaveToFile(fp);

    // Save our stuff
    fprintf(fp, "CHARGE=%.2f\n", m_fChargeLevel);
    fprintf(fp, "ENGAGED=%d\n", m_bEngaged);

    if (m_puiMinJumpSpeed)
    {
        fprintf(fp, "MIN SPEED=%d\n", *m_puiMinJumpSpeed);
    }
    if (m_puiEfficiency)
    {
        fprintf(fp, "EFFICIENCY=%d\n", *m_puiEfficiency);
    }

    fprintf(fp, "JUMPSPEED MULTIPLIER=%d\n", m_iJumpSpeedMultiplier);
}

// Returns the charge rate for the jump drives.  If bAdjusted
// is set to true, damage and power level adjustements are
// made to the value before returning it.
float CHSJumpDrive::GetChargeRate(HS_BOOL8 bAdjusted)
{
    float rate;

    // Base rate of 1/2 percent per second
    rate = 0.5;

    // Adjust?
    if (bAdjusted)
    {
        // Damage adjustment
        rate *= 1 - ((float) .25 * GetDamageLevel());

        // Power adjustment.  Add .00001 just in case
        // optimal power is 0.
        float fPower;
        fPower = (float) (GetOptimalPower(false) + .00001);
        fPower = GetCurrentPower() / fPower;

        rate *= fPower;
    }
    return rate;
}

void CHSJumpDrive::DoCycle()
{
    float rate;

    // Do base stuff first
    CHSEngSystem::DoCycle();

    // Charge jumpers?
    if ((GetCurrentPower() > 0) && (m_fChargeLevel < 100))
    {
        rate = GetChargeRate(true);

        if (rate > 0)
        {
            m_fChargeLevel += rate;

            // Make sure we don't overcharge
            if (m_fChargeLevel > 100)
            {
                m_fChargeLevel = 100;
            }

            if (m_fChargeLevel == 100)
            {
                if (GetOwnerObject()
                    && GetOwnerObject()->GetType() == HST_SHIP)
                {
                    CHSShip *cShip;
                    cShip = (CHSShip *) GetOwnerObject();
                    if (cShip)
                    {
                        cShip->NotifyConsoles(hsInterface.
                                              HSPrintf
                                              ("%s%s-%s Jump Drive charged.",
                                               ANSI_HILITE, ANSI_GREEN,
                                               ANSI_NORMAL), MSG_ENGINEERING);
                    }
                }
            }

        }
    }

    // If engaged, consume fuel.
    if (m_bEngaged)
    {
        ConsumeFuelBySpeed(m_uiSublightSpeed * m_iJumpSpeedMultiplier);
    }
}

HS_INT8 *CHSJumpDrive::GetStatus()
{
    static HS_INT8 tbuf[32];

    if (m_fChargeLevel == 100)
    {
        return "Charged";
    }
    else if (GetCurrentPower() <= 0)
    {
        return "Offline";
    }
    else
    {
        sprintf(tbuf, "%.0f/100%%", m_fChargeLevel);
        return tbuf;
    }
}

void CHSJumpDrive::CutPower(HS_INT32 level)
{
    if (level == 0)
    {
        if (GetOwnerObject() && (GetOwnerObject()->GetType() == HST_SHIP))
        {
            CHSShip *cShip;

            cShip = (CHSShip *) GetOwnerObject();

            if (hsInterface.AtrGet(cShip->GetDbref(), "HSMSG_JUMPERS_CUT"))
            {
                char *msg = hsInterface.EvalExpression(hsInterface.m_buffer,
                                                       cShip->GetDbref(),
                                                       cShip->GetDbref(),
                                                       cShip->GetDbref());
                cShip->NotifySrooms(msg);
            }
            else
            {
                cShip->NotifySrooms(HSCONF.jumpers_cut);
            }
        }

        // Set charge level to 0.
        m_fChargeLevel = 0;

        // If jumpers are engaged and power is set to 0, then
        // disengage.
        if (GetEngaged())
        {
            HS_INT8 tbuf[128];

            m_bEngaged = false;
            sprintf(tbuf, "%s%s-%s Jump drive disengaged.",
                    ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL);
            if (GetOwnerObject())
            {
                GetOwnerObject()->HandleMessage(tbuf, MSG_ENGINEERING);
            }
        }

    }
}

// Indicates whether jump drive is engaged or not.
HS_BOOL8 CHSJumpDrive::GetEngaged()
{
    return m_bEngaged;
}

// Attempts to set the engaged status
HS_BOOL8 CHSJumpDrive::SetEngaged(HS_BOOL8 bStat)
{
    if (bStat)
    {
        // Already engaged?
        if (GetEngaged())
            return false;

        // Not enough power?
        if (m_fChargeLevel < 100)
            return false;

        // Engage em.
        m_bEngaged = true;

        // Drain power
        m_fChargeLevel = 0;

        return true;
    }
    else
    {
        // Not engaged?
        if (!GetEngaged())
            return false;

        // Disengage
        m_bEngaged = false;
        return true;
    }
}

// Returns the current charge percentage from 0 - 100.
HS_INT32 CHSJumpDrive::GetChargePerc()
{
    return (HS_INT32) (m_fChargeLevel);
}

// Returns the minimum jump speed for the drive.
HS_INT32 CHSJumpDrive::GetMinJumpSpeed()
{
    // Do we have a local value?
    if (!m_puiMinJumpSpeed)
    {
        // Do we have a parent?
        if (!GetParent())
        {
            return 0;
        }
        else
        {
            CHSJumpDrive *ptr;
            ptr = (CHSJumpDrive *) GetParent();
            return ptr->GetMinJumpSpeed();
        }
    }

    return *m_puiMinJumpSpeed;
}

// Tells the engines that they need to gobble up some fuel
// from the fuel source.  The engines know their efficiency
// and such, so they just drain the fuel from the fuel system.
// Speed here should be given in units per hour.
void CHSJumpDrive::ConsumeFuelBySpeed(HS_INT32 speed)
{
    float fConsume;
    float fRealized;

    // Do we have a fuel system?
    if (!GetFuelSource() || (GetFuelSource()->GetFuelRemaining() <= 0))
    {
        return;
    }

    // Do not consume fuel if efficiency is 0
    if (0 == GetEfficiency())
    {
        return;
    }

    // Calculate per second travel rate
    fConsume = (float) (speed * .0002778);

    // Divide by efficiency
    fConsume = (float) (fConsume / (1000.0 * GetEfficiency()));

    fRealized = GetFuelSource()->ExtractFuelUnit(fConsume);
    if (fRealized < fConsume)
    {
        // Out of gas.  Disengage.
        m_bEngaged = false;
        HS_INT8 tbuf[128];
        sprintf(tbuf,
                "%s%s-%s A warning light flashing, indicating jump drives have \
			run out of fuel.",
                ANSI_HILITE, ANSI_YELLOW, ANSI_NORMAL);

        if (GetOwnerObject())
        {
            GetOwnerObject()->HandleMessage(tbuf, MSG_ENGINEERING, NULL);
        }
    }
}

// Returns the efficiency of the jump drive, which is a 
// number specifying thousands of units of distance travel
// per unit of fuel consumption .. just like gas mileage on
// a car.  For example 1000km/gallon.  This would be returned
// here as just 1.
HS_INT32 CHSJumpDrive::GetEfficiency()
{
    // Do we have a local value?
    if (!m_puiEfficiency)
    {
        // Do we have a parent?
        if (GetParent())
        {
            CHSJumpDrive *ptr;
            ptr = (CHSJumpDrive *) GetParent();
            return ptr->GetEfficiency();
        }
        else
        {
            return 1;           // default of 1000 (1k)
        }
    }

    return *m_puiEfficiency;
}

// Sets the fuel system source for the engines.  If this
// is NULL, engines don't consume fuel .. duh.
void CHSJumpDrive::SetFuelSource(CHSFuelSystem * cFuel)
{
    m_fuel_source = cFuel;
}

// Gets the fuel system for the engines. Replaces the m_fuel_source variable.
CHSFuelSystem* CHSJumpDrive::GetFuelSource(void)
{
	CHSShip *cShip = (CHSShip *) GetOwnerObject();
	if (!cShip)
    {
		return NULL;
    }
	return (CHSFuelSystem *)cShip->GetSystems().GetSystem(HSS_FUEL_SYSTEM);
}



// Tells the jump drive the current sublight speed so that
// it can calculate fuel consumption and hyperspace speed.
void CHSJumpDrive::SetSublightSpeed(HS_INT32 iSpeed)
{
    // Speed must be positive
    m_uiSublightSpeed = abs(iSpeed);
}
