// -----------------------------------------------------------------------
// $Id: hscloaking.cpp,v 1.5 2006/04/04 12:41:11 mark Exp $
// -----------------------------------------------------------------------

#include "pch.h"
#include <cstdio>
#include "hsinterface.h"
#include "hscloaking.h"


CHSSysCloak::CHSSysCloak():
m_efficiency(NULL), m_engaging(0.0), m_engaged(0)
{
    SetType(HSS_CLOAK);
}

CHSSysCloak::~CHSSysCloak()
{
    if (NULL != m_efficiency)
    {
        delete m_efficiency;
    }
}

HS_BOOL8 CHSSysCloak::SetAttributeValue(const HS_INT8 * pcAttrName,
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
                delete m_efficiency;
                m_efficiency = NULL;
            }
            else
            {
                if (!m_efficiency)
                {
                    m_efficiency = new HS_FLOAT32;
                }

                *m_efficiency = (HS_FLOAT32) atof(strValue);
            }
        }
        return true;
    }
    else if (!strcasecmp(pcAttrName, "ENGAGED"))
    {
        m_engaged = atoi(strValue);
        return true;
    }
    else
    {
        return CHSEngSystem::SetAttributeValue(pcAttrName, strValue);
    }
}


float CHSSysCloak::GetEfficiency(HS_BOOL8 bAdjusted)
{
    float rval;


    // Do we have a local value?
    if (!m_efficiency)
    {
        // Do we have a parent?
        if (GetParent())
        {
            CHSSysCloak *ptr;
            ptr = (CHSSysCloak *) GetParent();
            rval = ptr->GetEfficiency(false);
        }
        else
            return 0;           // default of 1000 (1k)
    }
    else
    {
        rval = *m_efficiency;
    }

    if (bAdjusted)
    {
        float fVal;
        int iOptPower;

        iOptPower = GetOptimalPower(false);
        fVal = (float) GetCurrentPower();

        if (iOptPower)
        {
            fVal /= (float) iOptPower;
        }
        else
        {
            fVal = 1.00;
        }

        rval *= fVal;

        if (rval > 100)
        {
            rval = (float) 99.999999;
        }
    }

    return rval;

}

void CHSSysCloak::SaveToFile(FILE * fp)
{
    // Save the base first
    CHSEngSystem::SaveToFile(fp);

    // Save our stuff
    fprintf(fp, "ENGAGED=%d\n", m_engaged);

    if (m_efficiency)
    {
        fprintf(fp, "EFFICIENCY=%.4f\n", *m_efficiency);
    }
}

void CHSSysCloak::GetAttributeList(CHSAttributeList & rlistAttrs)
{
    // Call the base class first.
    CHSEngSystem::GetAttributeList(rlistAttrs);

    // Push our own attributes.
    rlistAttrs.push_back("EFFICIENCY");
    rlistAttrs.push_back("ENGAGED");
}

HS_BOOL8
    CHSSysCloak::GetAttributeValue(const HS_INT8 * pcAttrName,
                                   CHSVariant & rvarValue,
                                   HS_BOOL8 bAdjusted, HS_BOOL8 bLocalOnly)
{
    // Determine attribute, and return the value.
    if (!strcasecmp(pcAttrName, "EFFICIENCY"))
    {
        if (m_efficiency)
        {
            rvarValue = *m_efficiency;
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
    else if (!strcasecmp(pcAttrName, "ENGAGED"))
    {
        rvarValue = m_engaged;
        return true;
    }
    else
    {
        // See if the base class knows about this attr.
        return CHSEngSystem::GetAttributeValue(pcAttrName, rvarValue,
                                               bAdjusted, bLocalOnly);
    }
}

int CHSSysCloak::GetEngaged()
{
    return m_engaged;
}

void CHSSysCloak::SetEngaged(HS_BOOL8 bEngage)
{
    if (bEngage)
    {
        m_engaging =
            atoi(hsInterface.HSPrintf("%.0f", GetOptimalPower() / 10.00));

        if (!IsEngaging())
        {
            m_engaged = 1;
        }
    }
    else
    {
        m_engaged = 0;
    }
}

HS_BOOL8 CHSSysCloak::IsEngaging()
{
    if (m_engaging > 0)
    {
        return true;
    }

    return false;
}

void CHSSysCloak::DoCycle()
{
    // Do base cycle stuff
    CHSEngSystem::DoCycle();

    if (IsEngaging())
    {
        m_engaging -= 1;

        if (!IsEngaging())
        {
            m_engaged = 1;
        }
    }

    return;
}
