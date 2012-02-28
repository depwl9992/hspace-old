// -----------------------------------------------------------------------
// @$Id: hsfuel.h,v 1.7 2006/04/04 12:56:10 mark Exp $
// -----------------------------------------------------------------------

#ifndef __HSFUEL_INCLUDED__
#define __HSFUEL_INCLUDED__

#include "hseng.h"

//! Types of fuel.
enum HS_FUELTYPE
{
    FL_BURNABLE = 0,
    FL_REACTABLE,
};

// This is derived an engineering system,
// even though it is quite different.
//
//! The CHSFuelSystem handles fuel storage, transfer, etc.
class CHSFuelSystem:public CHSEngSystem
{
  public:
    //! Setup default values, set system invisible
    CHSFuelSystem();

    //! Cleanup allocated memory, if appropriate
    virtual ~CHSFuelSystem();

    //! @brief Return maximum fuel of specified type, defaulting to burnable
    //! Will query the parent system if no data is present locally
    HS_INT32 GetMaxFuel(HS_FUELTYPE type = FL_BURNABLE);


    //! @brief Attempt to extract amt fuel of the specified type
    //! @return actual amount of fuel extracted
    HS_FLOAT32 ExtractFuelUnit(HS_FLOAT32 amt, HS_FUELTYPE type =
                               FL_BURNABLE);

    //! Top off the tanks, baby.
    void Refuel();

    //! Return the amount of fuel remaining for the given type
    HS_FLOAT32 GetFuelRemaining(HS_FUELTYPE tType = FL_BURNABLE)
    {
        if (tType == FL_BURNABLE)
        {
            return m_fBurnableFuel;
        }
        else if (tType == FL_REACTABLE)
        {
            return m_fReactableFuel;
        }

        return 0;
    }

    HS_BOOL8 SetAttributeValue(const HS_INT8 * pcAttrName,
                               const HS_INT8 * pcValue);

    void GetAttributeList(CHSAttributeList & rlistAttrs);

    HS_BOOL8 GetAttributeValue(const HS_INT8 * pcAttrName,
                               CHSVariant & rvarReturnVal,
                               HS_BOOL8 bAdjusted,
                               HS_BOOL8 bLocalOnly = false);

    void SaveToFile(FILE * fp);

    //! Set a specific fuel type to the given value
    void SetFuelLevel(HS_FLOAT32 fValue, HS_FUELTYPE type)
    {
        if (type == FL_BURNABLE)
        {
            m_fBurnableFuel = fValue;
        }
        else if (type == FL_REACTABLE)
        {
            m_fReactableFuel = fValue;
        }
    }

    //! @brief Set the maximum fuel level of the given type, overriding parent 
    //! if necessary
    void SetMaxFuel(HS_INT32 iValue, HS_FUELTYPE type)
    {
        if (type == FL_BURNABLE)
        {
            if (!m_puiMaxBurnableFuel)
            {
                m_puiMaxBurnableFuel = new HS_UINT32;
            }

            *m_puiMaxBurnableFuel = iValue;
        }
        else if (type == FL_REACTABLE)
        {
            if (!m_puiMaxReactableFuel)
            {
                m_puiMaxReactableFuel = new HS_UINT32;
            }

            *m_puiMaxReactableFuel = iValue;
        }
    }

    //! Return the raw storage pointer
    HS_UINT32 *GetRawMaxBurnable()
    {
        return m_puiMaxBurnableFuel;
    }

    //! Return the raw storage pointer
    HS_UINT32 *GetRawMaxReactable()
    {
        return m_puiMaxReactableFuel;
    }

  protected:
    //! Burnable fuel storage
    HS_FLOAT32 m_fBurnableFuel;

    //! Reactable fuel storage
    HS_FLOAT32 m_fReactableFuel;

    //! Max burnable fuel storage, overrideable at the local level
    HS_UINT32 *m_puiMaxBurnableFuel;

    //! Max reactable fuel storage, overrideable at the local level
    HS_UINT32 *m_puiMaxReactableFuel;
};

#endif // __HSFUEL_INCLUDED__
