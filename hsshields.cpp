// -----------------------------------------------------------------------
// $Id: hsshields.cpp,v 1.6 2006/04/04 12:41:11 mark Exp $
// -----------------------------------------------------------------------

#ifdef WIN32
#include "pch.h"
#endif

#include "hsobjects.h"
#include "hsuniverse.h"
#include "hsutils.h"
#include "hsinterface.h"
#include "hscelestial.h"
#include "hsshields.h"

CHSSysShield::CHSSysShield()
{
    SetType(HSS_FORE_SHIELD);
    m_fStrength = 0;
    m_puiMaxStrength = NULL;
    m_pfRegenRate = NULL;
    m_pshield_type = NULL;
}

void CHSSysShield::DoCycle()
{
    double rate;
    int max;

    // Do base stuff first
    CHSEngSystem::DoCycle();

    // Do anything?
    if (!GetCurrentPower())
    {
        m_fStrength = 0;
        return;
    }

    // Depending on the type of shield, we have
    // to do different things.
    if (GetShieldType() == ST_ABSORPTION)
    {
        // Regen if we have power.
        rate = GetRegenRate();
        max = GetMaxStrength();
        if ((m_fStrength < max) && (rate > 0))
        {
            // Increase by the given point rate
            m_fStrength += rate;

            if (m_fStrength > max)
                m_fStrength = max;
        }
    }
    // Don't do anything for deflector shields
}

// Does damage to the shield (maybe), and returns the number
// of points not handled by the shield.
HS_UINT32 CHSSysShield::DoDamage(int iPts)
{
    int rval;
    float strAffect = 100;
    double NebX, NebY, NebZ, dDistance;
    int Strength2;
    CHSUniverse *pUniverse;

    pUniverse = GetOwnerObject()->GetUniverse();
    if (!pUniverse)
        return 0;

    // See if there are any Nebulae near the vessel.
    CHS3DObject *pObject;
    for (pObject = pUniverse->GetFirstActiveObject(); pObject;
         pObject = pUniverse->GetNextActiveObject())
    {
        if (pObject->GetType() != HST_NEBULA)
        {
            continue;
        }

        NebX = pObject->GetX();
        NebY = pObject->GetY();
        NebZ = pObject->GetZ();

        dDistance = Dist3D(NebX, NebY, NebZ, GetOwnerObject()->GetX(),
                           GetOwnerObject()->GetY(),
                           GetOwnerObject()->GetZ());

        if (dDistance > pObject->GetSize() * 100)
        {
            continue;
        }

        // Calculate effect of Nebula on sensor report for our ship.
        strAffect = static_cast < CHSNebula * >(pObject)->GetShieldaff();
    }

    Strength2 = (int) (m_fStrength * (strAffect / 100));

    // What type of shield are we?
    if (GetShieldType() == ST_DEFLECTOR)
    {
        // Subtract our shield strength from the damage
        // points.
        rval = iPts - Strength2;
        if (rval < 0)
        {
            rval = 0;
        }

        return rval;
    }
    else if (GetShieldType() == ST_ABSORPTION)
    {
        // Knock off points from the shields.
        Strength2 -= iPts;

        m_fStrength = Strength2;

        // If the strength has gone negative, that means
        // the damage points were too much for the shield
        // to handle.  Thus, return the absolute value of
        // the shield strength, and set it to zero.
        if (m_fStrength < 0)
        {
            rval = (int) (m_fStrength * -1);
            m_fStrength = 0;
            return rval;
        }

        return 0;
    }

    // Who knows what type of shield we are.  Let's just
    // say we have none.
    return iPts;
}

// Returns the type of shield.
HS_SHIELDTYPE CHSSysShield::GetShieldType()
{
    if (!m_pshield_type)
    {
        if (GetParent())
            return static_cast <
                CHSSysShield * >(GetParent())->GetShieldType();
        else
            return ST_ABSORPTION;

    }
    else
        return *m_pshield_type;
}

void CHSSysShield::SaveToFile(FILE * fp)
{
    // Call base class next
    CHSEngSystem::SaveToFile(fp);

    // Ok to output, so we print out our information
    // specific to this system.
    fprintf(fp, "STRENGTH=%.2f\n", m_fStrength);

    if (m_pfRegenRate)
    {
        fprintf(fp, "REGEN RATE=%.4f\n", *m_pfRegenRate);
    }

    if (m_puiMaxStrength)
    {
        fprintf(fp, "MAX STRENGTH=%d\n", *m_puiMaxStrength);
    }
    if (m_pshield_type)
    {
        fprintf(fp, "SHIELD TYPE=%d\n", (int) (*m_pshield_type));
    }
}

// Returns the regen rate for the shield system.  If bAdjusted
// is set to false, then the maximum regen rate is returned,
// otherwise, it is adjusted for damage and power levels.
// Rates are stored as points per second.
float CHSSysShield::GetRegenRate(HS_BOOL8 bAdjusted)
{
    float dmgperc;
    CHSSysShield *ptr;
    float rate;
    int iOptPower;

    // Use some logic here.
    if (!m_pfRegenRate)
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
            ptr = (CHSSysShield *) GetParent();
            rate = ptr->GetRegenRate(false);
        }
    }
    else
    {
        rate = *m_pfRegenRate;
    }

    // Make overloading and damage adjustments?
    if (bAdjusted)
    {
        float fVal;
        iOptPower = GetOptimalPower(false);
        fVal = (float) GetCurrentPower();
        fVal /= (float) iOptPower;
        rate *= fVal;

        // Figure in damage.  1/4 reduction per level of damage
        dmgperc = 1 - (.25 * GetDamageLevel());
        rate = (rate * dmgperc);
    }

    return rate;
}

// Returns the current shield percentage from 0 - 100.
float CHSSysShield::GetShieldPerc()
{
    float perc;

    // If we don't have at least 1 MW charge, shields are
    // down.
    if (GetCurrentPower() <= 0)
    {
        return 0;
    }

    perc = GetMaxStrength() + .00001;
    perc = m_fStrength / perc;
    perc *= 100;

    return perc;
}

// Returns the maximum strength of the shield.  There is NO
// adjustment that can be made to this.  The shield regenerators
// ALWAYS recharge to full value, just maybe more slowly if damaged.
unsigned int CHSSysShield::GetMaxStrength()
{
    CHSSysShield *ptr;

    // Use some logic here.
    if (!m_puiMaxStrength)
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
            ptr = (CHSSysShield *) GetParent();
            return (ptr->GetMaxStrength());
        }
    }

    return (*m_puiMaxStrength);
}

// Sets a specific attribute value for the system.  This
// also allows system default values to be overridden at the
// ship level.
HS_BOOL8 CHSSysShield::SetAttributeValue(const HS_INT8 * pcAttrName,
                                         const HS_INT8 * strValue)
{
    int iVal;
    double dVal;

    // Match the name .. set the value
    if (!_stricmp(pcAttrName, "REGEN RATE"))
    {
        // If strValue contains a null, clear our local setting
        if (!*strValue)
        {
            if (m_pfRegenRate)
            {
                delete m_pfRegenRate;
                m_pfRegenRate = NULL;
            }
        }
        else
        {
            dVal = atof(strValue);
            if (dVal < 0)
            {
                return false;
            }

            if (NULL == m_pfRegenRate)
            {
                m_pfRegenRate = new float;
            }

            *m_pfRegenRate = dVal;
        }
        return true;
    }
    else if (!_stricmp(pcAttrName, "STRENGTH"))
    {
        dVal = atof(strValue);
        if (dVal < 0)
        {
            return false;
        }

        m_fStrength = dVal;
        return true;
    }
    else if (!_stricmp(pcAttrName, "SHIELD TYPE"))
    {
        // If strValue contains a null, clear our local setting
        if (!*strValue)
        {
            if (m_pshield_type)
            {
                delete m_pshield_type;
                m_pshield_type = NULL;
            }
        }
        else
        {
            iVal = atoi(strValue);
            if (iVal != 0 && iVal != 1 && iVal != 2)
            {
                return false;
            }

            if (NULL == m_pshield_type)
            {
                m_pshield_type = new HS_SHIELDTYPE;
            }

            *m_pshield_type = (HS_SHIELDTYPE) iVal;
        }
        return true;
    }
    else if (!_stricmp(pcAttrName, "MAX STRENGTH"))
    {
        // If strValue contains a null, clear our local setting
        if (!*strValue)
        {
            if (m_puiMaxStrength)
            {
                delete m_puiMaxStrength;
                m_puiMaxStrength = NULL;
            }
        }
        else
        {
            dVal = atoi(strValue);
            if (dVal < 0)
            {
                return false;
            }

            if (NULL == m_puiMaxStrength)
            {
                m_puiMaxStrength = new HS_UINT32;
            }

            *m_puiMaxStrength = (HS_UINT32) dVal;
        }

        return true;
    }
    else
    {
        return CHSEngSystem::SetAttributeValue(pcAttrName, strValue);
    }
}

void CHSSysShield::GetAttributeList(CHSAttributeList & rlistAttrs)
{
    // Call the base class first.
    CHSEngSystem::GetAttributeList(rlistAttrs);

    // Push our own attributes.
    rlistAttrs.push_back("REGEN RATE");
    rlistAttrs.push_back("STRENGTH");
    rlistAttrs.push_back("SHIELD TYPE");
    rlistAttrs.push_back("MAX STRENGTH");
}

HS_BOOL8
    CHSSysShield::GetAttributeValue(const HS_INT8 * pcAttrName,
                                    CHSVariant & rvarValue,
                                    HS_BOOL8 bAdjusted, HS_BOOL8 bLocalOnly)
{
    // Determine attribute, and return the value.
    if (!_stricmp(pcAttrName, "REGEN RATE"))
    {
        if (m_pfRegenRate)
        {
            rvarValue = *m_pfRegenRate;
        }
        else if (!bLocalOnly)
        {
            rvarValue = GetRegenRate();
        }
        else
        {
            return false;
        }

        return true;
    }
    else if (!_stricmp(pcAttrName, "STRENGTH"))
    {
        rvarValue = m_fStrength;
        return true;
    }
    else if (!_stricmp(pcAttrName, "SHIELD TYPE"))
    {
        rvarValue = (HS_INT8) GetShieldType();
        return true;
    }
    else if (!_stricmp(pcAttrName, "MAX STRENGTH"))
    {
        if (m_puiMaxStrength)
        {
            rvarValue = *m_puiMaxStrength;
        }
        else if (!bLocalOnly)
        {
            rvarValue = GetMaxStrength();
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

// We override the SetCurrentPower() function so that we can
// do different types of shield things.
HS_BOOL8 CHSSysShield::SetCurrentPower(int level)
{
    HS_UINT32 max;

    max = GetOptimalPower();

    // Don't allow levels below 0.  Level 0 is just deactivation.
    if (level < 0)
    {
        return false;
    }

    // Only allow overloading to 150%.
    if (level > (max * 1.5))
    {
        return false;
    }

    CHSEngSystem::SetCurrentPower(level);

    // Now, depending on the type of shield we are, set
    // the strength.
    if (GetShieldType() == ST_DEFLECTOR)
    {
        HS_FLOAT32 dPerc;

        // Grab the absolute maximum power level
        dPerc = (float) GetOptimalPower(false);

        dPerc = (float) level / dPerc;

        // Deflector shield strength is determined by
        // maximum possible strength multiplied by current
        // power settings.  If shield generators get damaged,
        // this doesn't allow as much power to be allocated,
        // and thus results in less shield strength.
        m_fStrength = (float) GetMaxStrength() * dPerc;
    }
    // Don't do anything for absorption shields.

    return true;
}
