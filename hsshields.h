// -----------------------------------------------------------------------
// $Id: hsshields.h,v 1.5 2006/04/04 12:56:10 mark Exp $
// -----------------------------------------------------------------------
#if !defined(__HSSHIELDS_H__)
#define __HSSHIELDS_H__

#include "hseng.h"


//! There are currently two types of shields.  
enum HS_SHIELDTYPE
{
    ST_DEFLECTOR = 0,
    ST_ABSORPTION
};

//! The base shield class derived from the generic engineering system
class CHSSysShield:public CHSEngSystem
{
  public:
    //! Default constructor set type and init variables to defaults
    CHSSysShield();

    //! Default deconstructor has no effect
    virtual ~CHSSysShield()
    {
    }

    //! Overloaded set operator to copy essential shield data
    void operator=(CHSEngSystem & rSystem)
    {
        CHSSysShield & rShield = static_cast < CHSSysShield & >(rSystem);

        if (rShield.m_puiMaxStrength)
        {
            SetMaxStrength(*rShield.m_puiMaxStrength);
        }
        if (rShield.m_pfRegenRate)
        {
            SetRegenRate(*rShield.m_pfRegenRate);
        }

        m_fStrength = rShield.m_fStrength;
    }

    //! Return type of shield or parent's type if derived
    HS_SHIELDTYPE GetShieldType();

    //! @brief Handle incoming damage to the shield
    //! @param iPts - number of points of incoming damage
    //! @return #of Damage Points not handled by the shield
    HS_UINT32 DoDamage(int iPts);

    //! Process a single cycle, regenerate absorption shield points
    void DoCycle();

    //! Save shield system data to the given file
    void SaveToFile(FILE * fp);

    //! Set the specified attribute name to the given pcValue if appropriate
    HS_BOOL8 SetAttributeValue(const HS_INT8 * pcAttrName,
                               const HS_INT8 * pcValue);

    //! Return list of manipulatable attributes for the shield system
    void GetAttributeList(CHSAttributeList & rlistAttrs);

    //! Return the current value of the specified attribute
    HS_BOOL8 GetAttributeValue(const HS_INT8 * pcAttrName,
                               CHSVariant & rvarReturnVal,
                               HS_BOOL8 bAdjusted,
                               HS_BOOL8 bLocalOnly = false);

    //! @brief Set the current power level for the shield system.  
    //! For deflector shields, the current power / optimal power determines
    //! the shield strength value
    HS_BOOL8 SetCurrentPower(int level);

    //! Returns the maximum strength of the shield.  There is NO
    //! adjustment that can be made to this.  The shield regenerators
    //! ALWAYS recharge to full value, just maybe more slowly if damaged.
    unsigned int GetMaxStrength();

    //! @brief Returns the regen rate for the shield system.  If bAdjusted
    //! is set to false, then the maximum regen rate is returned,
    //! otherwise, it is adjusted for damage and power levels.
    //! Rates are stored as points per second.
    float GetRegenRate(HS_BOOL8 bAdjusted = true);

    //! Returns the current shield percentage from 0 - 100.
    float GetShieldPerc();

    //! Set the maximum strength rating, allocating memory if needed
    void SetMaxStrength(unsigned int uiValue)
    {
        if (!m_puiMaxStrength)
        {
            m_puiMaxStrength = new unsigned int;
        }

        *m_puiMaxStrength = uiValue;
    }

    //! Set the shield type to the specified value
    void SetShieldType(HS_SHIELDTYPE eType)
    {
        if (!m_pshield_type)
        {
            m_pshield_type = new HS_SHIELDTYPE;
        }
        *m_pshield_type = eType;
    }

    //! Set the strength value for the shield
    void SetStrength(float fValue)
    {
        m_fStrength = fValue;
    }

    //! Sets the Regneration Rate for the Shield
    void SetRegenRate(float dValue)
    {
        if (!m_pfRegenRate)
        {
            m_pfRegenRate = new float;
        }

        *m_pfRegenRate = dValue;
    }

    //! Get the raw pointer to the Max Strength variable
    unsigned int *GetRawMaxStrength()
    {
        return m_puiMaxStrength;
    }

    //! Get the raw regeneration rate pointer
    float *GetRawRegenRate()
    {
        return m_pfRegenRate;
    }

    //! Get the current strength value for the shield
    float GetStrength()
    {
        return m_fStrength;
    }

  protected:
    HS_UINT32 * m_puiMaxStrength;
    HS_FLOAT32 m_fStrength;
    HS_FLOAT32 *m_pfRegenRate;
    HS_SHIELDTYPE *m_pshield_type;      // Type of shield
};

#endif // __HSSHIELDS_H__
