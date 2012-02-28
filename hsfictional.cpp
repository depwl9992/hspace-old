// -----------------------------------------------------------------------
//  $Id: hsfictional.cpp,v 1.2 2006/04/04 12:41:11 mark Exp $
// -----------------------------------------------------------------------

#include "hscopyright.h"
#include "hsfictional.h"

CHSFictional::CHSFictional()
{
    m_fChargeLevel = 0.0;
    m_fChargeRate = 0.0;
    SetType(HSS_FICTIONAL);
}

void CHSFictional::DoCycle()
{
    if (m_fChargeRate > 0 && GetCurrentPower() > 0 && m_fChargeLevel < 100)
    {
        float increase = (GetCurrentPower() / GetOptimalPower()) *
            m_fChargeRate;

        m_fChargeLevel += increase;

        if (m_fChargeLevel > 100)
        {
            m_fChargeLevel = 100.0;
        }
        else if (m_fChargeLevel < 0)
        {
            m_fChargeLevel = 0.0;
        }
    }
}

void CHSFictional::GetAttributeList(CHSAttributeList & rlistAttrs)
{
    CHSEngSystem::GetAttributeList(rlistAttrs);
    rlistAttrs.push_back("CHARGE");
    rlistAttrs.push_back("CHARGE RATE");
}

HS_BOOL8 CHSFictional::SetAttributeValue(const HS_INT8 * pcAttrName,
                                         const HS_INT8 * strValue)
{
    if (!strcasecmp(pcAttrName, "CHARGE RATE"))
    {
        m_fChargeRate = (float) atof(strValue);
        return true;
    }
    else if (!strcasecmp(pcAttrName, "CHARGE"))
    {
        m_fChargeLevel = (float) atof(strValue);
        return true;
    }
    else
    {
        return CHSEngSystem::SetAttributeValue(pcAttrName, strValue);
    }
}

HS_BOOL8
    CHSFictional::GetAttributeValue(const HS_INT8 * pcAttrName,
                                    CHSVariant & rvarValue,
                                    HS_BOOL8 bAdjusted, HS_BOOL8 bLocalOnly)
{
    // Determine attribute, and return the value.
    if (!strcasecmp(pcAttrName, "CHARGE RATE"))
    {
        rvarValue = m_fChargeRate;
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

void CHSFictional::SaveToFile(FILE * fp)
{
    CHSEngSystem::SaveToFile(fp);
    fprintf(fp, "CHARGE RATE=%f\n", m_fChargeRate);
    fprintf(fp, "CHARGE=%.2f\n", m_fChargeLevel);
}
