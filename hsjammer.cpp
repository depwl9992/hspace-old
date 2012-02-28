// -----------------------------------------------------------------------
// $Id: hsjammer.cpp,v 1.4 2006/04/04 12:41:11 mark Exp $
// -----------------------------------------------------------------------

#include "pch.h"

#include "hsjammer.h"

CHSSysJammer::CHSSysJammer(void):m_puiRange(NULL)
{
    SetType(HSS_JAMMER);
}

CHSSysJammer::~CHSSysJammer(void)
{
    if (NULL != m_puiRange)
    {
        delete m_puiRange;
    }
}

void CHSSysJammer::GetAttributeList(CHSAttributeList & rlistAttrs)
{
    CHSEngSystem::GetAttributeList(rlistAttrs);

    rlistAttrs.push_back("RANGE");
}

HS_BOOL8 CHSSysJammer::SetAttributeValue(const HS_INT8 * pcAttrName,
                                         const HS_INT8 * strValue)
{
    // Match the name .. set the value
    if (!_stricmp(pcAttrName, "RANGE"))
    {
        if (!*strValue)
        {
            delete m_puiRange;
            m_puiRange = NULL;
            return true;
        }
        else
        {
            if (!m_puiRange)
                m_puiRange = new HS_UINT32;

            *m_puiRange = atoi(strValue);
        }
        return true;
    }
    else
    {
        return CHSEngSystem::SetAttributeValue(pcAttrName, strValue);
    }
}

double CHSSysJammer::GetRange(HS_BOOL8 bAdjusted)
{
    double rval;


    // Do we have a local value?
    if (!m_puiRange)
    {
        // Do we have a parent?
        if (GetParent())
        {
            CHSSysJammer *ptr;
            ptr = (CHSSysJammer *) GetParent();
            if (NULL != ptr)
            {
                rval = ptr->GetRange(false);
            }
            else
            {
                return 0;
            }
        }
        else
        {
            return 0;           // default of 1000 (1k)
        }
    }
    else
    {
        rval = *m_puiRange;
    }

    if (bAdjusted)
    {
        double fVal;
        int iOptPower;
        iOptPower = GetOptimalPower(false);
        fVal = (double) GetCurrentPower();
        if (iOptPower)
        {
            fVal /= (double) iOptPower;
        }
        else
        {
            fVal = 1;
        }

        rval *= fVal;
    }



    return rval;

}

void CHSSysJammer::SaveToFile(FILE * fp)
{
    // Save the base first
    CHSEngSystem::SaveToFile(fp);

    // Save our stuff
    if (NULL != m_puiRange)
    {
        fprintf(fp, "RANGE=%d\n", *m_puiRange);
    }
}


HS_BOOL8 CHSSysJammer::GetAttributeValue(const HS_INT8 * pcAttrName,
                                         CHSVariant & rvarValue,
                                         HS_BOOL8 bAdjusted,
                                         HS_BOOL8 bLocalOnly)
{
    // Determine attribute, and return the value.
    if (!_stricmp(pcAttrName, "RANGE"))
    {
        if (NULL != m_puiRange)
        {
            rvarValue = *m_puiRange;
        }
        else if (!bLocalOnly)
        {
            rvarValue = GetRange(bAdjusted);
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
