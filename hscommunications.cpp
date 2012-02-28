// -----------------------------------------------------------------------
// $Id: hscommunications.cpp,v 1.4 2006/04/04 12:41:11 mark Exp $
// -----------------------------------------------------------------------

#include "pch.h"

#include <cstdio>
#include "hsinterface.h"
#include "hscommunications.h"

CHSSysComm::CHSSysComm(void):
m_puiCommRange(NULL)
{
    SetType(HSS_COMM);
}

CHSSysComm::~CHSSysComm(void)
{
    if (m_puiCommRange)
    {
        delete m_puiCommRange;
    }
}

void CHSSysComm::GetAttributeList(CHSAttributeList & rlistAttrs)
{
    // Push the base class first.
    CHSEngSystem::GetAttributeList(rlistAttrs);

    // Now push our attributes.
    rlistAttrs.push_back("MAX RANGE");
}

// Sets a specific attribute value for the system.  This
// also allows system default values to be overridden at the
// ship level.
HS_BOOL8 CHSSysComm::SetAttributeValue(const HS_INT8 * pcAttrName,
                                       const HS_INT8 * strValue)
{
    int iVal;

    // Match the name .. set the value
    if (!_stricmp(pcAttrName, "MAX RANGE"))
    {
        // If strValue contains a null, clear our local setting
        if (!*strValue)
        {
            if (m_puiCommRange)
            {
                delete m_puiCommRange;
                m_puiCommRange = NULL;
            }
        }
        else
        {
            iVal = atoi(strValue);
            if (iVal < 0)
                return false;

            if (!m_puiCommRange)
                m_puiCommRange = new HS_UINT32;

            *m_puiCommRange = iVal;
        }
        return true;
    }
    else
    {
        return CHSEngSystem::SetAttributeValue(pcAttrName, strValue);
    }
}

// Returns the value for the specified attribute, or NULL if
// invalid attribute.
HS_BOOL8 CHSSysComm::GetAttributeValue(const HS_INT8 * pcAttrName,
                                       CHSVariant & rvarValue,
                                       HS_BOOL8 bAdjusted,
                                       HS_BOOL8 bLocalOnly)
{
    // Determine attribute, return the value.
    if (!_stricmp(pcAttrName, "MAX RANGE"))
    {
        if (m_puiCommRange)
        {
            rvarValue = *m_puiCommRange;
        }
        else if (!bLocalOnly)
        {
            rvarValue = GetMaxRange();
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

// Returns the maximum transmission range for the system.  If bAdjusted
// is set to false, then the maximum raw turning rate is returned,
// otherwise, it is adjusted for damage and power levels.
HS_UINT32 CHSSysComm::GetMaxRange(HS_BOOL8 bAdjusted)
{
    CHSSysComm *ptr;
    HS_UINT32 uVal;
    int iOptPower;

    // Use some logic here.
    if (!m_puiCommRange)
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
            ptr = (CHSSysComm *) GetParent();
            uVal = ptr->GetMaxRange(false);
        }
    }
    else
        uVal = *m_puiCommRange;


    // Make overloading and damage adjustments?  No need
    // to figure in damage, cause that's figured into 
    // optimal power and overloading.  If they overload
    // a damaged system, it stresses, so we just need to
    // check current power over undamaged optimal power.
    if (bAdjusted)
    {
        float fVal;
        iOptPower = GetOptimalPower(false);
        fVal = (float) GetCurrentPower();
        fVal /= (float) iOptPower;
        uVal = (HS_UINT32) (uVal * fVal);
    }
    return uVal;
}

// Outputs system information to a file.  This could be
// the class database or another database of objects that
// contain systems.
void CHSSysComm::SaveToFile(FILE * fp)
{
    // Call base class FIRST
    CHSEngSystem::SaveToFile(fp);

    // Now output our local values.
    if (m_puiCommRange)
    {
        fprintf(fp, "MAX RANGE=%d\n", *m_puiCommRange);
    }
}
