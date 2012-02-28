// -----------------------------------------------------------------------
//! $Id: hsengines.h,v 1.6 2006/12/16 01:17:04 mark Exp $
// -----------------------------------------------------------------------
#ifndef __HSENGINES_INCLUDED__
#define __HSENGINES_INCLUDED__

#include "hsfuel.h"
#include "hsconf.h"

//! A system specifically for engines, derived from
//! the base CHSEngSystem class.  The engines aren't like
//! car engines.  They basically handle velocity.
class CHSSysEngines : public CHSEngSystem
{
    public:
        //! Setup all default values
        CHSSysEngines();

        //! Deallocate local memory
        ~CHSSysEngines();

        HS_BOOL8 SetAttributeValue(const HS_INT8 * pcAttrName,
                const HS_INT8 * pcValue);

        void GetAttributeList(CHSAttributeList & rlistAttrs);

        HS_BOOL8 GetAttributeValue(const HS_INT8 * pcAttrName,
                CHSVariant & rvarReturnVal, HS_BOOL8 bAdjusted,
                HS_BOOL8 bLocalOnly = false);

        //! Is this engine capable of afterburning?
        HS_BOOL8 CanBurn()
        {
            // Do we have a local value?
            if (!m_pbCanAfterburn)
            {
                // No local value.  Do we have a parent?
                if (!GetParent())
                {
                    return false;
                }

                // Get the parent's value.
                CHSSysEngines *ptr = (CHSSysEngines *) GetParent();
                return ptr->CanBurn();
            }
            else
            {
                return *m_pbCanAfterburn;
            }
        }

        //! Toggle afterburning on (true) or off (false)
        HS_BOOL8 SetAfterburn(HS_BOOL8 bStat)
        {
            if (!CanBurn())
            {
                return false;
            }

            HS_FLOAT32 uMax = GetDesiredSpeed();
            if (bStat)
            {
                m_desired_speed = (HS_FLOAT32) (uMax * m_iAfterburnRatio);
                m_afterburning = true;
            }
            else
            {
                m_afterburning = false;
                m_desired_speed = (HS_FLOAT32) uMax;
            }
            return true;
        }

        //! Set the desired speed in units/hr
        HS_BOOL8 SetDesiredSpeed(HS_INT32 speed)
        {
            HS_INT32 iMaxVelocity = GetMaxVelocity();
            if ((speed > iMaxVelocity) || (speed < (-1 * (iMaxVelocity / 2.0))))
            {
                return false;
            }

            // If desired speed is 0, turn off afterburners
            if (speed == 0)
            {
                SetAfterburn(false);
            }

            m_desired_speed = (HS_FLOAT32) speed;
            return true;
        }

        //! Setup afterburning capability 
        void SetCanBurn(HS_BOOL8 bValue)
        {
            if (!m_pbCanAfterburn)
            {
                m_pbCanAfterburn = new HS_BOOL8;
            }

            *m_pbCanAfterburn = bValue;
        }

        //! Set the current speed of the system
        void SetCurrentSpeed(HS_FLOAT32 fValue)
        {
            m_current_speed = fValue;
        }

        //! Set acceleration parameter
        void SetAcceleration(HS_UINT32 iValue)
        {
            if (!m_puiAcceleration)
            {
                m_puiAcceleration = new HS_UINT32;
            }

            *m_puiAcceleration = iValue;
        }

        //! Set the efficiency for the engine system
        void SetEfficiency(HS_UINT32 iValue)
        {
            if (!m_puiEfficiency)
            {
                m_puiEfficiency = new HS_UINT32;
            }

            *m_puiEfficiency = iValue;
        }

        //! Set the maximum allowable velocity for the engine
        void SetMaxVelocity(HS_UINT32 iValue)
        {
            if (!m_puiMaxVelocity)
            {
                m_puiMaxVelocity = new HS_UINT32;
            }

            *m_puiMaxVelocity = iValue;
        }

        //! Are the afterburners active?
        HS_BOOL8 GetAfterburning();

        //! Get the current maximum velocity 
        HS_UINT32 GetMaxVelocity(HS_BOOL8 bAdjusted = true);

        //! Get the current acceleration parameter
        HS_UINT32 GetAcceleration(HS_BOOL8 bAdjusted = true);

        //! Get the desired speed
        HS_FLOAT32 GetDesiredSpeed();

        //! Get the current speed
        HS_FLOAT32 GetCurrentSpeed();

        //! Get current efficiency value
        HS_INT32 GetEfficiency();

        //! Set the fuel source for the engine system
        void SetFuelSource(CHSFuelSystem* cFuel);

        //! Get the fuel source for the engine system
        CHSFuelSystem* GetFuelSource();

        //! Write system details to the given file
        void SaveToFile(FILE *fp);

        //! Consume fuel based on the current speed
        void ConsumeFuelBySpeed(HS_INT32);

        //! Process on cycle for the system
        void DoCycle();

        //! Set the current power level
        void PowerUp(HS_INT32 level);

        //! Send notifications of power removal from the engines
        void CutPower(HS_INT32 level);

        //! Return a pointer to the raw member pointer or NULL
        HS_UINT32 *GetRawMaxVelocity()
        {
            return m_puiMaxVelocity;
        }

        //! Return a pointer to the raw member pointer or NULL
        HS_UINT32 *GetRawAcceleration()
        {
            return m_puiAcceleration;
        }

        //! Return a pointer to the raw member pointer or NULL
        HS_UINT32 *GetRawEfficiency()
        {
            return m_puiEfficiency;
        }

        //! Return a pointer to the raw member pointer or NULL
        HS_BOOL8 *GetRawCanAfterburn()
        {
            return m_pbCanAfterburn;
        }

        //! Get the current afterburning ratio
        HS_INT32 GetAfterburnRatio()
        {
            return m_iAfterburnRatio;
        }

    protected:
        //! Process speed changes (called from DoCycle())
        void ChangeSpeed();

        // Source of fuel
        CHSFuelSystem *m_fuel_source;

        //! Current speed in units/hr
        HS_FLOAT32 m_current_speed;

        //! Desired speed in units/hr
        HS_FLOAT32 m_desired_speed;

        //! Are engines are in afterburn mode?
        HS_BOOL8 m_afterburning;

        //! Maximum velocity for this system
        HS_UINT32 *m_puiMaxVelocity;

        //! Maximum acceleration per cycle
        HS_UINT32 *m_puiAcceleration;

        //! Efficiency of this system
        HS_UINT32 *m_puiEfficiency; 

        //! Is this engine afterburn capable
        HS_BOOL8 *m_pbCanAfterburn;

        //! Local afterburning ratio for this engine system
        HS_INT32 m_iAfterburnRatio;
};


#endif // __HSENGINES_INCLUDED__
