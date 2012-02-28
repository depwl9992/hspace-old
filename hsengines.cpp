// -----------------------------------------------------------------------
//! $Id: hsengines.cpp,v 1.15 2007/01/31 13:35:03 worsel Exp $
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
#include "hsengines.h"

CHSSysEngines::CHSSysEngines() : 
    m_fuel_source(NULL),
    m_current_speed(0),
    m_desired_speed(0),
    m_afterburning(false),
    m_puiMaxVelocity(NULL),
    m_puiAcceleration(NULL),
    m_puiEfficiency(NULL),
    m_pbCanAfterburn(NULL), 
    m_iAfterburnRatio(HSCONF.afterburn_ratio)
{
    SetType(HSS_ENGINES);
}

// Destructor
CHSSysEngines::~CHSSysEngines()
{
    // Free up our variables
    if (m_puiMaxVelocity)
    {
        delete m_puiMaxVelocity;
    }

    if (m_puiAcceleration)
    {
        delete m_puiAcceleration;
    }

    if (m_puiEfficiency)
    {
        delete m_puiEfficiency;
    }

    if (m_pbCanAfterburn)
    {
        delete m_pbCanAfterburn;
    }
}

void CHSSysEngines::PowerUp(HS_INT32 level)
{
    if (GetOwnerObject() && (GetOwnerObject()->GetType() == HST_SHIP))
    {
        CHSShip *cShip;

        cShip = (CHSShip *) GetOwnerObject();

        if (hsInterface.AtrGet(cShip->GetDbref(), "HSMSG_ENGINES_ACTIVATING"))
        {
            HS_DBREF idx = cShip->GetDbref();

            char *msg = hsInterface.EvalExpression(hsInterface.m_buffer,
                                                   idx, idx, idx);
            cShip->NotifySrooms(msg);
        }
        else
        {
            cShip->NotifySrooms(HSCONF.engines_activating);
        }
    }
}

void CHSSysEngines::CutPower(HS_INT32 level)
{
    if (level == 0)
    {
        if (GetOwnerObject() && GetOwnerObject()->GetType() == HST_SHIP)
        {
            CHSShip *cShip;

            cShip = (CHSShip *) GetOwnerObject();

            if (hsInterface.AtrGet(cShip->GetDbref(), "HSMSG_ENGINES_CUT"))
            {
                HS_DBREF idx = cShip->GetDbref();

                char *msg = hsInterface.EvalExpression(hsInterface.m_buffer,
                                                       idx, idx, idx);
                cShip->NotifySrooms(msg);
            }
            else
            {
                cShip->NotifySrooms(HSCONF.engines_cut);
            }
        }
    }
}



// Sets a specific attribute value for the system.  This
// also allows system default values to be overridden at the
// ship level.
HS_BOOL8 CHSSysEngines::SetAttributeValue(const HS_INT8 * pcAttrName,
                                          const HS_INT8 * strValue)
{
    HS_INT32 iVal;

    // Match the name .. set the value
    if (!_stricmp(pcAttrName, "MAX VELOCITY"))
    {
        // If strValue contains a null, clear our local setting 
        if (!*strValue)
        {
            if (m_puiMaxVelocity)
            {
                delete m_puiMaxVelocity;
                m_puiMaxVelocity = NULL;
            }
        }
        else
        {
            iVal = atoi(strValue);
            if (iVal < 0)
            {
                return false;
            }

            if (!m_puiMaxVelocity)
            {
                m_puiMaxVelocity = new HS_UINT32;
            }

            *m_puiMaxVelocity = iVal;
        }
        return true;
    }
    else if (!_stricmp(pcAttrName, "ACCELERATION"))
    {
        // If strValue contains a null, clear our local setting
        if (!*strValue)
        {
            if (m_puiAcceleration)
            {
                delete m_puiAcceleration;
                m_puiAcceleration = NULL;
            }
        }
        else
        {
            iVal = atoi(strValue);
            if (iVal < 0)
            {
                return false;
            }

            if (!m_puiAcceleration)
            {
                m_puiAcceleration = new HS_UINT32;
            }

            *m_puiAcceleration = iVal;
        }
        return true;
    }
    else if (!_stricmp(pcAttrName, "DESIRED SPEED"))
    {
        m_desired_speed = (float) atof(strValue);
        return true;
    }
    else if (!_stricmp(pcAttrName, "CURRENT SPEED"))
    {
        m_current_speed = (float) atof(strValue);
        return true;
    }
    else if (!_stricmp(pcAttrName, "CAN AFTERBURN"))
    {
        // If strValue contains a null, clear our local setting
        if (!*strValue)
        {
            if (m_pbCanAfterburn)
            {
                delete m_pbCanAfterburn;
                m_pbCanAfterburn = NULL;
            }
        }
        else
        {
            if (!m_pbCanAfterburn)
                m_pbCanAfterburn = new HS_BOOL8;

            *m_pbCanAfterburn = atoi(strValue) == 0 ? false : true;
        }
        return true;
    }
    else if (!_stricmp(pcAttrName, "AFTERBURNING"))
    {
        m_afterburning = atoi(strValue) == 0 ? false : true;
        return true;
    }
    else if (!_stricmp(pcAttrName, "AFTERBURN RATIO"))
    {
        m_iAfterburnRatio = atoi(strValue);
        return true;
    }
    else if (!_stricmp(pcAttrName, "EFFICIENCY"))
    {
        // If strValue contains a null, clear our local setting
        if (!*strValue)
        {
            if (m_puiEfficiency)
            {
                delete m_puiEfficiency;
                m_puiEfficiency = NULL;
            }
        }
        else
        {
            iVal = atoi(strValue);
            if (iVal < 0)
            {
                return false;
            }

            if (!m_puiEfficiency)
            {
                m_puiEfficiency = new HS_UINT32;
            }

            *m_puiEfficiency = iVal;
        }
        return true;
    }
    else
    {
        return CHSEngSystem::SetAttributeValue(pcAttrName, strValue);
    }
}

void CHSSysEngines::GetAttributeList(CHSAttributeList & rlistAttrs)
{
    // Call the base class first.
    CHSEngSystem::GetAttributeList(rlistAttrs);

    // Push our own attributes.
    rlistAttrs.push_back("MAX VELOCITY");
    rlistAttrs.push_back("ACCELERATION");
    rlistAttrs.push_back("DESIRED SPEED");
    rlistAttrs.push_back("CURRENT SPEED");
    rlistAttrs.push_back("CAN AFTERBURN");
    rlistAttrs.push_back("AFTERBURNING");
    rlistAttrs.push_back("AFTERBURN RATIO");
    rlistAttrs.push_back("EFFICIENCY");
}

HS_BOOL8 CHSSysEngines::GetAttributeValue(const HS_INT8 * pcAttrName,
        CHSVariant & rvarValue, HS_BOOL8 bAdjusted, HS_BOOL8 bLocalOnly)
{
    // Determine attribute, and return the value.
    if (!_stricmp(pcAttrName, "EFFICIENCY"))
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
    else if (!_stricmp(pcAttrName, "MAX VELOCITY"))
    {
        if (m_puiMaxVelocity)
        {
            rvarValue = *m_puiMaxVelocity;
        }
        else if (!bLocalOnly)
        {
            rvarValue = GetMaxVelocity();
        }
        else
        {
            return false;
        }
        return true;
    }
    else if (!_stricmp(pcAttrName, "ACCELERATION"))
    {
        if (m_puiAcceleration)
        {
            rvarValue = *m_puiAcceleration;
        }
        else if (!bLocalOnly)
        {
            rvarValue = GetAcceleration();
        }
        else
        {
            return false;
        }

        return true;
    }
    else if (!_stricmp(pcAttrName, "DESIRED SPEED"))
    {
        rvarValue = m_desired_speed;
        return true;
    }
    else if (!_stricmp(pcAttrName, "CURRENT SPEED"))
    {
        rvarValue = m_current_speed;
        return true;
    }
    else if (!_stricmp(pcAttrName, "CAN AFTERBURN"))
    {
        if (m_pbCanAfterburn)
        {
            rvarValue = *m_pbCanAfterburn;
        }
        else if (!bLocalOnly)
        {
            rvarValue = CanBurn();
        }
        else
        {
            return false;
        }

        return true;
    }
    else if (!_stricmp(pcAttrName, "AFTERBURNING"))
    {
        rvarValue = m_afterburning;
        return true;
    }
    else if (!_stricmp(pcAttrName, "AFTERBURN RATIO"))
    {
        rvarValue = m_iAfterburnRatio;
        return true;
    }
    else
    {
        return CHSEngSystem::GetAttributeValue(pcAttrName, rvarValue,
                                               bAdjusted, bLocalOnly);
    }
}

// Returns the current speed (thrust) of the engines
float CHSSysEngines::GetCurrentSpeed()
{
    return m_current_speed;
}

// Indicates whether the engines are afterburning or not
HS_BOOL8 CHSSysEngines::GetAfterburning()
{
    return m_afterburning;
}

// Returns the current, desired speed of the engines
float CHSSysEngines::GetDesiredSpeed()
{
    return m_desired_speed;
}

// Handles cyclic stuff for engines
void CHSSysEngines::DoCycle()
{
    HS_INT32 iFuel;

    // Do base class stuff first
    CHSEngSystem::DoCycle();

    // Do anything?
    if (!m_current_speed && !GetCurrentPower())
    {
        return;
    }

    // Change our speed if needed
    ChangeSpeed();

    // Consume fuel.  Yummy.
    if (m_current_speed)
    {
        iFuel = (HS_INT32) m_current_speed;

        if (GetAfterburning())
        {
            iFuel *= HSCONF.afterburn_fuel_ratio;
        }

        ConsumeFuelBySpeed((HS_INT32) m_current_speed);
    }
}

// Handles slowing or speeding up the engines
void CHSSysEngines::ChangeSpeed()
{
    HS_UINT32 uMax;
    HS_UINT32 uAccel;

    // Grab maximum velocity
    uMax = GetMaxVelocity();

    // If afterburning, max is increased by some ratio.
    if (m_afterburning)
    {
        uMax *= m_iAfterburnRatio;      //HSCONF.afterburn_ratio;
    }

    // Make sure the desired speed doesn't currently
    // exceed the maximum speed limits.
    if (m_desired_speed > uMax)
    {
        m_desired_speed = (float) uMax;
    }
    else if (m_desired_speed < (uMax * -.5))
    {
        m_desired_speed = uMax * (float) -.5;
    }


    // This has to come after the checking of uMax.  Otherwise,
    // the ship could be at top speed, power is taken away from
    // engines, but the speed doesn't change because the desired
    // and current speed are already equal.
    if (m_desired_speed == m_current_speed)
    {
        return;
    }


    // Change the speed closer to the desired speed
    if (m_current_speed > m_desired_speed)
    {
        // Grab deceleration rate, which is always the
        // same as maximum acceleration.  Deceleration is
        // not affected by engine damage or power.
        uAccel = GetAcceleration(false);

        m_current_speed -= uAccel;

        if (m_desired_speed < 0)
        {
            m_current_speed += m_desired_speed;
        }

        // Did we go too far?
        if (m_current_speed < m_desired_speed)
        {
            m_current_speed = m_desired_speed;
        }
    }
    else if (m_current_speed < m_desired_speed)
    {
        // Grab the acceleration rate, which is affected
        // by power and damage.
        uAccel = GetAcceleration(true);

        m_current_speed += uAccel;

        // Did we go too far?
        if (m_current_speed > m_desired_speed)
        {
            m_current_speed = m_desired_speed;
        }
    }
}

// Tells the engines that they need to gobble up some fuel
// from the fuel source.  The engines know their efficiency
// and such, so they just drain the fuel from the fuel system.
// Speed here should be given in units per hour.
void CHSSysEngines::ConsumeFuelBySpeed(HS_INT32 speed)
{
    float fConsume;
    float fRealized;

    // Do we have a fuel system?
    if (!GetFuelSource() ||
        (GetFuelSource()->GetFuelRemaining() <= 0))
    {
        return;
    }

    // If efficiency is 0, do not consume fuel
    if (0 == GetEfficiency())
    {
        return;
    }

    // Calculate per second travel rate
    fConsume = (float) (speed * .0002778);

    // Divide by efficiency
    fConsume = (float) (fConsume / (1000.0 * GetEfficiency()));

    // Engines consume burnable fuel
    fRealized = GetFuelSource()->ExtractFuelUnit(fConsume);

    // check system consumed the ammount we asked for
    if (fRealized < fConsume)
    {
        // Out of gas.  Set desired speed to 0.
        m_desired_speed = 0;
        HS_INT8 tbuf[128];
        sprintf_s(tbuf,
                "%s%s-%s A warning light flashing, indicating engines have run \
			out of fuel.",
                ANSI_HILITE, ANSI_YELLOW, ANSI_NORMAL);
        if (GetOwnerObject())
        {
            GetOwnerObject()->HandleMessage(tbuf, MSG_ENGINEERING, NULL);
        }
    }
}

// Sets the fuel system source for the engines.  If this
// is NULL, engines don't consume fuel .. duh.
void CHSSysEngines::SetFuelSource(CHSFuelSystem * cFuel)
{
    m_fuel_source = cFuel;
}

// Gets the fuel system for the engines. Replaces the m_fuel_source variable.
CHSFuelSystem* CHSSysEngines::GetFuelSource(void)
{
      CHSShip *cShip = (CHSShip *) GetOwnerObject();
      if (!cShip)
              return NULL;
      return (CHSFuelSystem *)cShip->GetSystems().GetSystem(HSS_FUEL_SYSTEM);
}


// Returns the efficiency of the engines, which is a 
// number specifying thousands of units of distance travel
// per unit of fuel consumption .. just like gas mileage on
// a car.  For example 1000km/gallon.  This would be returned
// here as just 1.
HS_INT32 CHSSysEngines::GetEfficiency()
{
    // Do we have a local value?
    if (!m_puiEfficiency)
    {
        // Do we have a parent?
        if (GetParent())
        {
            CHSSysEngines *ptr;
            ptr = (CHSSysEngines *) GetParent();
            return ptr->GetEfficiency();
        }
        else
        {
            return 1;           // default of 1000 (1k)
        }
    }

    return *m_puiEfficiency;
}

// Returns the acceleration rate for the engines.  If the
// bAdjusted variable is set to false, no damage or power level
// adjustments are made.
HS_UINT32 CHSSysEngines::GetAcceleration(HS_BOOL8 bAdjusted)
{
    double dmgperc;
    CHSSysEngines *ptr;
    HS_UINT32 uVal;
    HS_INT32 iOptPower;

    // Use some logic here.
    if (!m_puiAcceleration)
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
            ptr = (CHSSysEngines *) GetParent();
            uVal = ptr->GetAcceleration(false);
        }
    }
    else
    {
        uVal = *m_puiAcceleration;
    }

    // Make overloading and damage adjustments?
    if (bAdjusted)
    {
        // If afterburning, acceleration is increased
        // like speed.
        if (GetAfterburning())
        {
            uVal *= m_iAfterburnRatio;  //HSCONF.afterburn_ratio;
        }

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
void CHSSysEngines::SaveToFile(FILE * fp)
{
    // Call base class FIRST
    CHSEngSystem::SaveToFile(fp);

    // Ok to output, so we print out our information
    // specific to this system.
    if (m_puiMaxVelocity)
        fprintf(fp, "MAX VELOCITY=%d\n", *m_puiMaxVelocity);
    if (m_puiAcceleration)
        fprintf(fp, "ACCELERATION=%d\n", *m_puiAcceleration);
    if (m_puiEfficiency)
        fprintf(fp, "EFFICIENCY=%d\n", *m_puiEfficiency);
    if (m_pbCanAfterburn)
        fprintf(fp, "CAN AFTERBURN=%d\n", *m_pbCanAfterburn);

    fprintf(fp, "DESIRED SPEED=%.0f\n", m_desired_speed);
    fprintf(fp, "CURRENT SPEED=%.0f\n", m_current_speed);
    fprintf(fp, "AFTERBURNING=%d\n", m_afterburning);
    fprintf(fp, "AFTERBURN RATIO=%d\n", m_iAfterburnRatio);
}

HS_UINT32 CHSSysEngines::GetMaxVelocity(HS_BOOL8 bAdjusted)
{
    HS_FLOAT64 dmgperc;
    CHSSysEngines *ptr;
    HS_UINT32 uVal;
    HS_INT32 iOptPower;

    // Use some logic here.
    if (!m_puiMaxVelocity)
    {
        // Go to the parent's setting?
        if (!GetParent())
        {
            return 0;
        }
        else
        {
            // Yes, this system exists on a ship, so
            // find the default values on the parent.
            ptr = (CHSSysEngines *) GetParent();
            uVal = ptr->GetMaxVelocity(false);
        }
    }
    else
        uVal = *m_puiMaxVelocity;


    // Make overloading and damage adjustments?
    if (bAdjusted)
    {
        // At this point, uVal is the maximum velocity assuming
        // 100% power level.  We have to adjust for power level
        // now.  The speed inrcreases proportional to overload
        // percentage.
        HS_FLOAT32 fVal;
        iOptPower = GetOptimalPower(true);
        fVal = (HS_FLOAT32) GetCurrentPower();
        fVal /= (HS_FLOAT32) iOptPower;
        uVal = (HS_UINT32) (fVal * uVal);

        // Figure in damage.  1/4 reduction per level of damage
        dmgperc = 1 - (.25 * GetDamageLevel());
        uVal = (HS_UINT32) (uVal * dmgperc);

        // Do we have a fuel system?
        if (GetFuelSource())
        {
            if (GetFuelSource()->GetFuelRemaining() <= 0)
                return 0;
        }
    }
    return uVal;
}




