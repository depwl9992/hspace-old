// -----------------------------------------------------------------------
//! $Id: hsreactor.cpp,v 1.9 2007/01/31 13:33:12 worsel Exp $
// -----------------------------------------------------------------------

#include "pch.h"

#include "hsobjects.h"
#include "hsconf.h"
#include "hsinterface.h"
#include "hsfuel.h"
#include "hsansi.h"
#include "hspace.h"
#include "hsship.h"

#include "hsreactor.h"

CHSReactor::CHSReactor():
m_uiCurrentOutput(0),
m_uiDesiredOutput(0),
m_puiMaximumOutput(NULL), m_puiEfficiency(NULL), m_fuel_source(NULL)
{
    SetType(HSS_REACTOR);

    // The reactor is not visible as other systems are
    SetVisible(false);
}

//
// CHSReactor implementation
CHSReactor::~CHSReactor()
{
    if (m_puiMaximumOutput)
    {
        delete m_puiMaximumOutput;
    }

    if (m_puiEfficiency)
    {
        delete m_puiEfficiency;
    }
}

void CHSReactor::SetFuelSource(CHSFuelSystem * cSource)
{
    m_fuel_source = cSource;
}

// Returns the desired output setting for the reactor
HS_UINT32 CHSReactor::GetDesiredOutput()
{
    return m_uiDesiredOutput;
}

// Returns the current output for the reactor
HS_UINT32 CHSReactor::GetOutput()
{
    return m_uiCurrentOutput;
}

// Returns the status of the reactor
HS_INT8 *CHSReactor::GetStatus()
{
    if (IsOnline())
    {
        return "Online";
    }

    return "Offline";
}

// Returns efficiency of reactor
int CHSReactor::GetEfficiency()
{
    // Do we have a local value?
    if (NULL == m_puiEfficiency)
    {
        // Do we have a parent?
        if (GetParent())
        {
            CHSReactor *ptr = (CHSReactor *) GetParent();

            // This should never be NULL
            if (NULL != ptr)
            {
                return ptr->GetEfficiency();
            }
            else
            {
                return 100;     // 100% Efficient
            }
        }
        else
        {
            return 100;         // 100% Efficient  
        }
    }

    return *m_puiEfficiency;
}

// Sets a specific attribute value for the system.  This
// also allows system default values to be overridden at the
// ship level.
HS_BOOL8 CHSReactor::SetAttributeValue(const HS_INT8 * pcAttrName,
                                       const HS_INT8 * strValue)
{
    int iVal;

    // Match the name .. set the value
    if (!_stricmp(pcAttrName, "MAX OUTPUT"))
    {
        // If strValue contains a null, clear our local setting
        if (!*strValue)
        {
            if (m_puiMaximumOutput)
            {
                delete m_puiMaximumOutput;
                m_puiMaximumOutput = NULL;
            }
        }
        else
        {
            iVal = atoi(strValue);
            if (iVal < 0)
                return false;

            if (!m_puiMaximumOutput)
                m_puiMaximumOutput = new HS_UINT32;

            *m_puiMaximumOutput = iVal;
        }
        return true;
    }
    else if (!_stricmp(pcAttrName, "DESIRED OUTPUT"))
    {
        iVal = atoi(strValue);
        m_uiDesiredOutput = iVal;
        return true;
    }
    else if (!_stricmp(pcAttrName, "CURRENT OUTPUT"))
    {
        iVal = atoi(strValue);
        m_uiCurrentOutput = iVal;
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
                iVal = 0;
            }
            else if (iVal > 100)
            {
                iVal = 100;
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

void CHSReactor::GetAttributeList(CHSAttributeList & rlistAttrs)
{
    CHSEngSystem::GetAttributeList(rlistAttrs);

    rlistAttrs.push_back("EFFICIENCY");
    rlistAttrs.push_back("MAX OUTPUT");
    rlistAttrs.push_back("CURRENT OUTPUT");
    rlistAttrs.push_back("DESIRED OUTPUT");
}

HS_BOOL8
    CHSReactor::GetAttributeValue(const HS_INT8 * pcAttrName,
                                  CHSVariant & rvarValue,
                                  HS_BOOL8 bAdjusted, HS_BOOL8 bLocalOnly)
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
    else if (!_stricmp(pcAttrName, "MAX OUTPUT"))
    {
        if (m_puiMaximumOutput)
        {
            rvarValue = *m_puiMaximumOutput;
        }
        else if (!bLocalOnly)
        {
            rvarValue = GetMaximumOutput();
        }
        else
        {
            return false;
        }
        return true;
    }
    else if (!_stricmp(pcAttrName, "CURRENT OUTPUT"))
    {
        rvarValue = m_uiCurrentOutput;
        return true;
    }
    else if (!_stricmp(pcAttrName, "DESIRED OUTPUT"))
    {
        rvarValue = m_uiDesiredOutput;
        return true;
    }
    else
    {
        return CHSEngSystem::GetAttributeValue(pcAttrName, rvarValue,
                                               bAdjusted, bLocalOnly);
    }
}

// Outputs system information to a file.  This could be
// the class database or another database of objects that
// contain systems.
void CHSReactor::SaveToFile(FILE * fp)
{
    // Call base class FIRST
    CHSEngSystem::SaveToFile(fp);

    // Ok to output, so we print out our information
    // specific to this system.
    if (m_puiMaximumOutput)
        fprintf(fp, "MAX OUTPUT=%d\n", *m_puiMaximumOutput);

    fprintf(fp, "DESIRED OUTPUT=%d\n", m_uiDesiredOutput);
    fprintf(fp, "CURRENT OUTPUT=%d\n", m_uiCurrentOutput);

    if (m_puiEfficiency)
        fprintf(fp, "EFFICIENCY=%d\n", *m_puiEfficiency);
}

// Attempts to set the reactor to a given output level
HS_BOOL8 CHSReactor::SetOutputLevel(int level)
{
    HS_UINT32 max;

    max = GetMaximumOutput();

    // Don't allow levels below 0.  Level 0 is just deactivation.
    if (level < 0)
        return false;

    // Only allow overloading to 150%.
    if (level > (max * 1.5))
        return false;

    m_uiDesiredOutput = level;

    // If desired output is less than level, just cut
    // the power right to that level.  Cyclic update is
    // only for powering up, not powering down.
    if (m_uiDesiredOutput < m_uiCurrentOutput)
    {
        m_uiCurrentOutput = m_uiDesiredOutput;
    }

    return true;
}

// Returns the maximum output capability of the reactor,
// accounting for damage that has occurred.
HS_UINT32 CHSReactor::GetMaximumOutput(HS_BOOL8 bWithDamage)
{
    double dmgperc;
    HS_UINT32 uVal;

    // Use some logic here.
    if (!m_puiMaximumOutput)
    {
        // Go to the parent's setting?
        if (!GetParent())
        {
            // No.  We are the default values.
            return 0;
        }
        else
        {
            CHSReactor *ptr = (CHSReactor *) GetParent();

            // Yes, this system exists on a ship, so
            // find the default values on the parent.
            uVal = ptr->GetMaximumOutput();
        }
    }
    else
        uVal = *m_puiMaximumOutput;

    if (bWithDamage)
    {
        // Figure in damage.  1/4 reduction per level of damage
        dmgperc = 1 - (.25 * GetDamageLevel());
        uVal = (HS_UINT32) (uVal * dmgperc);
    }
    return uVal;
}

// This is overridden from the base class because reactors
// work much different than other systems.  We have to power
// up, calculate stress based on output (not usage), etc.
void CHSReactor::DoCycle()
{
    HS_UINT32 max;
    float fVal;

    // Check to see if we need to do anything at all.
    if (!m_uiCurrentOutput && !m_uiDesiredOutput && (GetStress() == 0))
        return;

    // Maybe we incurred damage, so check to
    // see if we should cut power.  Remember, we can
    // overload to 150%, so allow for that.
    max = GetMaximumOutput();
    max = (HS_UINT32) (max * 1.5);      // 150% allowance
    if (m_uiCurrentOutput > max)
    {
        // Cut power
        m_uiCurrentOutput = max;
    }

    // Check to see if we should increase or decrease
    // power output.
    if (m_uiCurrentOutput < m_uiDesiredOutput)
    {
        if (m_uiCurrentOutput < max)
        {
            // It's ok to increase
            m_uiCurrentOutput++;
        }
    }
    else if (m_uiCurrentOutput > m_uiDesiredOutput)
    {
        // Decrease output to desired level.
        m_uiCurrentOutput = m_uiDesiredOutput;
    }

    // Calculate stress.  In this case we don't
    // allow for 150% overload.  If it's anything over
    // 100%, we stress it.
    max = GetMaximumOutput();
    if ((m_uiCurrentOutput > max) || (GetStress() > 0))
    {
        HS_FLOAT32 fStress = GetStress();

        // Calculate percent overload/underload.  Have to
        // use some extra float variables here because of
        // some funky problems with subtracting unsigned ints.
        float fVal2;
        fVal = (float) max;
        fVal2 = (float) m_uiCurrentOutput;
        fVal2 = fVal2 - max;
        fVal = fVal2 / fVal;
        fVal *= 100;

        // If overloaded, tolerance hinders stress.
        // If underloaded, tolerance reduces stress faster.
        if (fVal > 0)
            fVal *= (float) (HS_STRESS_INCREASE / (float) GetTolerance());
        else
        {
            // Make sure stress is reduced at least minimally
            // at 100% power allocation.
            if (!fVal)
                fVal = -1;
            fVal *= (float) (HS_STRESS_INCREASE * GetTolerance());
        }
        fStress += fVal;
        if (fStress > 100)
        {
            fStress = 100;
        }
        if (fStress < 0)
        {
            fStress = 0;
        }

        SetStress(fStress);
    }

    // Figure out fuel usage, and consume it.
    if (NULL != GetFuelSource())
    {
        float fRealized;
        float fNeeded;


        // Get efficiency of the reactor, validate it is between 1 and 100
        // to avoid division by 0 later
        int efficiency = GetEfficiency();
        if (efficiency < 1)
        {
            return;
        }
        else if (efficiency > 100)
        {
            efficiency = 100;
        }

        // Calculate # of units of fuel needed to generate the current
        // output of the reactor
        fNeeded = (float) m_uiCurrentOutput / (float) HSCONF.fuel_ratio;

        // Modify the units required based on the efficiency of the reactor
        // Fuel consumption goes up as efficiency goes down
        fNeeded = fNeeded / ((float) efficiency / 100.0);

        // Apply the time slice component
        fNeeded *= (float) .0002778 * (float) HSCONF.cyc_interval;

        if (fNeeded > 0)
        {
            // Model two fuels?
            if (HSCONF.use_two_fuels)
                fRealized = GetFuelSource()->ExtractFuelUnit(fNeeded,
                                                           FL_REACTABLE);
            else
                fRealized = GetFuelSource()->ExtractFuelUnit(fNeeded);

            // If the fuel realized (expended) is less than
            // requested, the fuel tank is "empty."  Thus, 
            // set the desired reactor power to 0.
            if (fRealized < fNeeded)
            {
                char tbuf[128];

                sprintf_s(tbuf,
                        "%s%s-%s A warning light flashes, indicating that reactable \
					fuel supplies are depleted.",
                        ANSI_HILITE, ANSI_YELLOW, ANSI_NORMAL);
                if (GetOwnerObject())
                    GetOwnerObject()->HandleMessage(tbuf, MSG_ENGINEERING,
                                                    NULL);

                m_uiDesiredOutput = 0;
            }
        }
    }
}

// Indicates if the reactor is "online" or not.  This doesn't
// mean whether it's powering or not.  The state of being 
// online is whether the reactor can now begin to allocate
// power to other systems.  It is defined as 5% powered.
HS_BOOL8 CHSReactor::IsOnline()
{
    HS_UINT32 max;

    // Get max reactor output, not with damage
    max = GetMaximumOutput(false);

    if (m_uiCurrentOutput >= (max * .05))
    {
        return true;
    }

    return false;
}

// Gets the fuel system for the reactor. Replaces the m_fuel_source variable.
CHSFuelSystem* CHSReactor::GetFuelSource(void)
{
        CHSShip *cShip = (CHSShip *) GetOwnerObject();
        if (!cShip)
    {
                return NULL;
    }
        return (CHSFuelSystem *)cShip->GetSystems().GetSystem(HSS_FUEL_SYSTEM);
}

