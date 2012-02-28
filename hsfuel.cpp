// -----------------------------------------------------------------------
// @$Id: hsfuel.cpp,v 1.8 2006/04/04 12:41:11 mark Exp $
// -----------------------------------------------------------------------

#include "pch.h"

#include <cstring>
#include <stdlib.h>
#include <cstdio>

#include "hstypes.h"
#include "hsinterface.h"
#include "hsfuel.h"


CHSFuelSystem::CHSFuelSystem():
m_fBurnableFuel(0.0),
m_fReactableFuel(0.0), m_puiMaxBurnableFuel(NULL), m_puiMaxReactableFuel(NULL)
{
    SetType(HSS_FUEL_SYSTEM);

    // Fuel systems are not visible as other
    // systems are.  They are more of an 
    // interal use system.
    SetVisible(false);
}

CHSFuelSystem::~CHSFuelSystem()
{
    if (NULL != m_puiMaxBurnableFuel)
    {
        delete m_puiMaxBurnableFuel;
    }

    if (NULL != m_puiMaxReactableFuel)
    {
        delete m_puiMaxReactableFuel;
    }
}


HS_BOOL8 CHSFuelSystem::SetAttributeValue(const HS_INT8 * pcAttrName,
                                          const HS_INT8 * strValue)
{
    HS_INT32 iVal;

    if (!_stricmp(pcAttrName, "BURNABLE FUEL"))
    {
        m_fBurnableFuel = (float) atof(strValue);
        return true;
    }
    else if (!_stricmp(pcAttrName, "REACTABLE FUEL"))
    {
        m_fReactableFuel = (float) atof(strValue);
        return true;
    }
    else if (!_stricmp(pcAttrName, "MAX REACTABLE FUEL"))
    {
        // If strValue contains a null, clear our local setting
        if (!*strValue)
        {
            if (m_puiMaxReactableFuel)
            {
                delete m_puiMaxReactableFuel;
                m_puiMaxReactableFuel = NULL;
                return true;
            }
        }
        else
        {
            iVal = atoi(strValue);
            if (NULL == m_puiMaxReactableFuel)
            {
                m_puiMaxReactableFuel = new HS_UINT32;
            }

            if (NULL != m_puiMaxReactableFuel)
            {
                *m_puiMaxReactableFuel = iVal;
            }
            else                // just in case allocation fails
            {
                return false;
            }

        }
        return true;
    }
    else if (!_stricmp(pcAttrName, "MAX BURNABLE FUEL"))
    {
        // If strValue contains a null, clear our local setting
        if (!*strValue)
        {
            if (m_puiMaxBurnableFuel)
            {
                delete m_puiMaxBurnableFuel;
                m_puiMaxBurnableFuel = NULL;
            }
        }
        else
        {
            iVal = atoi(strValue);
            if (NULL == m_puiMaxBurnableFuel)
            {
                m_puiMaxBurnableFuel = new HS_UINT32;
            }

            if (NULL != m_puiMaxBurnableFuel)
            {
                *m_puiMaxBurnableFuel = iVal;
            }
            else
            {
                return false;
            }
        }
        return true;
    }
    else
    {
        return CHSEngSystem::SetAttributeValue(pcAttrName, strValue);
    }
}

void CHSFuelSystem::GetAttributeList(CHSAttributeList & rlistAttrs)
{
    CHSEngSystem::GetAttributeList(rlistAttrs);

    rlistAttrs.push_back("MAX BURNABLE FUEL");
    rlistAttrs.push_back("MAX REACTABLE FUEL");
    rlistAttrs.push_back("BURNABLE FUEL");
    rlistAttrs.push_back("REACTABLE FUEL");
}

HS_BOOL8 CHSFuelSystem::GetAttributeValue(const HS_INT8 * pcAttrName,
                                          CHSVariant & rvarValue,
                                          HS_BOOL8 bAdjusted,
                                          HS_BOOL8 bLocalOnly)
{
    // Determine attribute, and return the value.
    if (!_stricmp(pcAttrName, "MAX BURNABLE FUEL"))
    {
        if (m_puiMaxBurnableFuel)
        {
            rvarValue = *m_puiMaxBurnableFuel;
        }
        else if (!bLocalOnly)
        {
            rvarValue = GetMaxFuel(FL_BURNABLE);
        }
        else
        {
            return false;
        }
        return true;
    }
    else if (!_stricmp(pcAttrName, "MAX REACTABLE FUEL"))
    {
        if (m_puiMaxReactableFuel)
        {
            rvarValue = *m_puiMaxReactableFuel;
        }
        else if (!bLocalOnly)
        {
            rvarValue = GetMaxFuel(FL_REACTABLE);
        }
        else
        {
            return false;
        }
        return true;
    }
    else if (!_stricmp(pcAttrName, "REACTABLE FUEL"))
    {
        rvarValue = m_fReactableFuel;
        return true;
    }
    else if (!_stricmp(pcAttrName, "BURNABLE FUEL"))
    {
        rvarValue = m_fBurnableFuel;
        return true;
    }
    else
    {
        return CHSEngSystem::GetAttributeValue(pcAttrName, rvarValue,
                                               bAdjusted, bLocalOnly);
    }
}

// Returns the maximum fuel of a given type.  If no type
// is specified, it defaults to burnable fuel.
HS_INT32 CHSFuelSystem::GetMaxFuel(HS_FUELTYPE tType)
{
    // We have to determine type and then see if 
    // we have to return the value from the parent or
    // not.
    if (tType == FL_BURNABLE)
    {
        if (NULL == m_puiMaxBurnableFuel)
        {
            if (!GetParent())
                return 0;
            else
            {
                CHSFuelSystem *cFuel;
                cFuel = (CHSFuelSystem *) GetParent();
                return cFuel->GetMaxFuel(tType);
            }
        }
        else
        {
            return *m_puiMaxBurnableFuel;
        }
    }
    else if (tType == FL_REACTABLE)
    {
        if (NULL == m_puiMaxReactableFuel)
        {
            if (!GetParent())
                return 0;
            else
            {
                CHSFuelSystem *cFuel;
                cFuel = (CHSFuelSystem *) GetParent();
                return cFuel->GetMaxFuel(tType);
            }
        }
        else
        {
            return *m_puiMaxReactableFuel;
        }
    }

    // An unsupported fuel type
    return 0;
}

// ATTEMPTS to extract a given amount of fuel from the
// system for a certain type.  If type is not specified,
// it defaults to burnable fuel.  The return value is 
// the level of fuel actually given, which may not be
// the same as requested.
float CHSFuelSystem::ExtractFuelUnit(float amt, HS_FUELTYPE tType)
{
    float rval;

    if (amt < 0)
        return 0;

    if (tType == FL_BURNABLE)
    {
        if (amt > m_fBurnableFuel)
        {
            rval = m_fBurnableFuel;
            m_fBurnableFuel = 0;
            return rval;
        }
        else
        {
            m_fBurnableFuel -= amt;
            return amt;
        }
    }
    else if (tType == FL_REACTABLE)
    {
        if (amt > m_fReactableFuel)
        {
            rval = m_fReactableFuel;
            m_fReactableFuel = 0;
            return rval;
        }
        else
        {
            m_fReactableFuel -= amt;
            return amt;
        }
    }

    return 0;
}

// Writes the fuel system attributes to the specified
// file stream.
void CHSFuelSystem::SaveToFile(FILE * fp)
{
    // Save base class info first
    CHSEngSystem::SaveToFile(fp);

    // Output RAW values.  Do not call any Get() functions,
    // because those will return local or parent values.  We
    // ONLY want to output local values.
    if (NULL != m_puiMaxBurnableFuel)
    {
        fprintf(fp, "MAX BURNABLE FUEL=%d\n", *m_puiMaxBurnableFuel);
    }

    if (NULL != m_puiMaxReactableFuel)
    {
        fprintf(fp, "MAX REACTABLE FUEL=%d\n", *m_puiMaxReactableFuel);
    }

    fprintf(fp, "REACTABLE FUEL=%.2f\n", m_fReactableFuel);
    fprintf(fp, "BURNABLE FUEL=%.2f\n", m_fBurnableFuel);
}

void CHSFuelSystem::Refuel()
{
    if (NULL != m_puiMaxBurnableFuel)
    {
        m_fBurnableFuel = *m_puiMaxBurnableFuel;
    }
    else
    {
        if (!GetParent())
        {
            return;
        }
        else
        {
            CHSFuelSystem *cFuel;
            cFuel = (CHSFuelSystem *) GetParent();
            if (cFuel)
            {
                m_fBurnableFuel = cFuel->GetMaxFuel(FL_BURNABLE);
            }
        }
    }

    if (NULL != m_puiMaxReactableFuel)
    {
        m_fReactableFuel = *m_puiMaxReactableFuel;
    }
    else
    {
        if (!GetParent())
        {
            return;
        }
        else
        {
            CHSFuelSystem *cFuel;
            cFuel = (CHSFuelSystem *) GetParent();
            if (cFuel)
            {
                m_fReactableFuel = cFuel->GetMaxFuel(FL_REACTABLE);
            }
        }
    }
}
