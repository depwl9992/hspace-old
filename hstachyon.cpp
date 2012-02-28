// -----------------------------------------------------------------------
// $Id: hstachyon.cpp,v 1.5 2006/04/04 12:41:11 mark Exp $
// -----------------------------------------------------------------------
#include "pch.h"

#include <cstdio>
#include "hstachyon.h"


CHSSysTach::CHSSysTach(void)
{
    SetType(HSS_TACHYON);
    m_pfEfficiency = NULL;
}

CHSSysTach::~CHSSysTach(void)
{
    if (NULL != m_pfEfficiency)
    {
        delete m_pfEfficiency;
    }
}

HS_BOOL8 CHSSysTach::SetAttributeValue(const HS_INT8 * pcAttrName,
                                       const HS_INT8 * strValue)
{
    // Match the name .. set the value
    if (!strcasecmp(pcAttrName, "EFFICIENCY"))
    {
        if (atof(strValue) < 0 || 100 < atof(strValue))
        {
            return false;
        }
        else
        {
            if (!*strValue)
            {
                delete m_pfEfficiency;
                m_pfEfficiency = NULL;
            }
            else
            {
                if (!m_pfEfficiency)
                    m_pfEfficiency = new float;

                *m_pfEfficiency = (float) atof(strValue);
            }
        }
        return true;
    }
    else
    {
        return CHSEngSystem::SetAttributeValue(pcAttrName, strValue);
    }
}

float CHSSysTach::GetEfficiency(HS_BOOL8 bAdjusted)
{
    float rval;


    // Do we have a local value?
    if (!m_pfEfficiency)
    {
        // Do we have a parent?
        if (GetParent())
        {
            CHSSysTach *ptr;
            ptr = (CHSSysTach *) GetParent();
            rval = ptr->GetEfficiency(false);
        }
        else
            return 0;           // default of 1000 (1k)
    }
    else
        rval = *m_pfEfficiency;

    if (bAdjusted)
    {
        float fVal;
        int iOptPower;
        iOptPower = GetOptimalPower(false);
        fVal = (float) GetCurrentPower();
        if (iOptPower)
            fVal /= (float) iOptPower;
        else
            fVal = 1;
        rval *= fVal;
        if (rval > 100)
            rval = (float) 99.999999;
    }

    return rval;
}

void CHSSysTach::SaveToFile(FILE * fp)
{
    // Save the base first
    CHSEngSystem::SaveToFile(fp);

    // Save our stuff
    if (m_pfEfficiency)
        fprintf(fp, "EFFICIENCY=%.4f\n", *m_pfEfficiency);
}

void CHSSysTach::GetAttributeList(CHSAttributeList & rlistAttrs)
{
    // Call the base class first.
    CHSEngSystem::GetAttributeList(rlistAttrs);

    // Push our own attributes.
    rlistAttrs.push_back("EFFICIENCY");
}

HS_BOOL8 CHSSysTach::GetAttributeValue(const HS_INT8 * pcAttrName,
                                       CHSVariant & rvarValue,
                                       HS_BOOL8 bAdjusted,
                                       HS_BOOL8 bLocalOnly)
{
    // Determine attribute, and return the value.
    if (!strcasecmp(pcAttrName, "EFFICIENCY"))
    {
        if (m_pfEfficiency)
        {
            rvarValue = *m_pfEfficiency;
        }
        else if (!bLocalOnly)
        {
            rvarValue = GetEfficiency(bAdjusted);
        }
        else
        {
            return false;
        }
        return true;
    }

    // See if the base class knows about this attr.
    return CHSEngSystem::GetAttributeValue(pcAttrName, rvarValue,
                                           bAdjusted, bLocalOnly);
}
