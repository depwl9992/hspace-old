// -----------------------------------------------------------------------
//! $Id: hsjumpdrive.h,v 1.2 2006/12/16 01:25:01 mark Exp $
// -----------------------------------------------------------------------

#ifndef __HSJUMPDRIVE_INCLUDED__
#define __HSJUMPDRIVE_INCLUDED__

#include "hsfuel.h"
#include "hsconf.h"
#include "hsengines.h"
// Hyperspace jump drives.  They're basically like the
// generic CHSEngSystem with some enhancements.
class CHSJumpDrive:public CHSEngSystem
{
  public:
    CHSJumpDrive();

    void GetAttributeList(CHSAttributeList & rlistAttrs);

    HS_BOOL8 GetAttributeValue(const HS_INT8 * pcAttrName,
                               CHSVariant & rvarReturnVal,
                               HS_BOOL8 bAdjusted,
                               HS_BOOL8 bLocalOnly = false);

    HS_BOOL8 SetAttributeValue(const HS_INT8 * pcAttrName,
                               const HS_INT8 * pcValue);

    HS_BOOL8 GetEngaged();
    HS_BOOL8 SetEngaged(HS_BOOL8);
    HS_INT32 GetMinJumpSpeed();
    HS_INT32 GetChargePerc();
    HS_INT32 GetEfficiency();
    HS_INT8 *GetStatus();
    HS_INT32 GetJumpSpeedMultiplier()
    {
        return m_iJumpSpeedMultiplier;
    };
    void SaveToFile(FILE *);
    void DoCycle();
    void ConsumeFuelBySpeed(HS_INT32);
    void SetFuelSource(CHSFuelSystem *);
    CHSFuelSystem* GetFuelSource();
    void SetSublightSpeed(HS_INT32);
    void GiveSystemInfo(HS_INT32);
    void CutPower(HS_INT32);

    void SetEfficiency(HS_INT32 iValue)
    {
        if (!m_puiEfficiency)
        {
            m_puiEfficiency = new HS_UINT32;
        }

        *m_puiEfficiency = iValue;
    }

    void SetMinJumpSpeed(HS_INT32 iValue)
    {
        if (!m_puiMinJumpSpeed)
        {
            m_puiMinJumpSpeed = new HS_UINT32;
        }

        *m_puiMinJumpSpeed = iValue;
    }

    void SetChargeLevel(HS_FLOAT32 dLevel)
    {
        m_fChargeLevel = dLevel;
    }

    HS_FLOAT32 GetChargeRate(HS_BOOL8 bAdjust = true);

    HS_UINT32 *GetRawMinJumpSpeed()
    {
        return m_puiMinJumpSpeed;
    }

    HS_UINT32 *GetRawEfficiency()
    {
        return m_puiEfficiency;
    }

  protected:
    // Source of fuel
    CHSFuelSystem * m_fuel_source;

    HS_BOOL8 m_bEngaged;
    HS_UINT32 *m_puiMinJumpSpeed;       // Overridable at the ship level
    HS_UINT32 *m_puiEfficiency; // Thousands of distance units per fuel unit
    HS_FLOAT32 m_fChargeLevel;
    HS_UINT32 m_uiSublightSpeed;        // Set this to indicate fuel consumption
    HS_INT32 m_iJumpSpeedMultiplier;
};


#endif // __HSJUMPDRIVE_INCLUDED__

