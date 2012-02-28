// -----------------------------------------------------------------------
//! $Id: hsreactor.h,v 1.4 2007/01/31 13:33:12 worsel Exp $
// -----------------------------------------------------------------------
#if !defined(__HSREACTOR_H__)
#define __HSREACTOR_H__


#include "hseng.h"

// Forward Declarations
class CHSFuelSystem;

//! The HSpace Reactor Class.  Power me up, Scotty!
//! This object is found on ships to supply power that
//! can be transferred to various systems.  It is actually
//! a sort of system because it's derived from CHSEngSystem,
//! but we'll ultimately use it differently.  It will provide
//! power, not consume it.
class CHSReactor:public CHSEngSystem
{
  public:
    //! Initialize all variables and set system type
    CHSReactor();

    //! Cleanup all allocated memory
    ~CHSReactor();

    //! @brief Set attribute pcAttrName to pcValue 
    //! @return true on success, false on failure
    HS_BOOL8 SetAttributeValue(const HS_INT8 * pcAttrName,
                               const HS_INT8 * pcValue);

    //! Generate the current list of attributes on this system
    void GetAttributeList(CHSAttributeList & rlistAttrs);


    //! Retrieve an attribute value from the system
    HS_BOOL8 GetAttributeValue(const HS_INT8 * pcAttrName,
                               CHSVariant & rvarReturnVal,
                               HS_BOOL8 bAdjusted,
                               HS_BOOL8 bLocalOnly = false);

    //! Attempt to set the reactor output to the specified level
    HS_BOOL8 SetOutputLevel(int level);

    //! Return true if the reactor is online, false if offline
    HS_BOOL8 IsOnline();

    //! Return the current output of the reactor
    HS_UINT32 GetOutput();

    //! Return the desired output of the reactor
    HS_UINT32 GetDesiredOutput();

    //! Return the maximum output of the reactor, calculate using damage if
    //! bWithDamage is true (default)
    HS_UINT32 GetMaximumOutput(HS_BOOL8 bWithDamage = true);

    //! Return the current efficiency rating for the reactor
    int GetEfficiency();

    //! Save the system information to the specified file pointer
    void SaveToFile(FILE * fp);

    //! Process a single cycle for the reactor including output and stress
    void DoCycle();

    //! Set the optional fuel source for the reactor
    void SetFuelSource(CHSFuelSystem * cSource);

    //! Get the fuel source for the engine system
    CHSFuelSystem* GetFuelSource();

    //! Returns the current "Online" or "Offline" status as a string
    HS_INT8 *GetStatus();


    //! Set the current output level for the reactor
    void SetCurrentOutput(HS_UINT32 iValue)
    {
        m_uiCurrentOutput = iValue;
    }

    //! Set the desired output level for the reactor
    void SetDesiredOutput(HS_UINT32 iValue)
    {
        m_uiDesiredOutput = iValue;
    }

    //! Set the maximum output level for the reactor; allocate memory if 
    //! required
    void SetMaximumOutput(HS_UINT32 iValue)
    {
        if (!m_puiMaximumOutput)
        {
            m_puiMaximumOutput = new HS_UINT32;
        }

        *m_puiMaximumOutput = iValue;
    }

    //! Set the efficiency level of the reactor, allocating memory as needed
    void SetEfficiency(HS_UINT32 iValue)
    {
        if (!m_puiEfficiency)
        {
            m_puiEfficiency = new HS_UINT32;
        }
        *m_puiEfficiency = iValue;
    }

    //! Return a pointer to the allocated maximum output member
    HS_UINT32 *GetRawMaximumOutput()
    {
        return m_puiMaximumOutput;
    }

    //! Return a pointer to the raw efficiency memory
    HS_UINT32 *GetRawEfficiency()
    {
        return m_puiEfficiency;
    }

  protected:
    //! current power output
    HS_UINT32 m_uiCurrentOutput;

    //! desired output level
    HS_UINT32 m_uiDesiredOutput;

    // These variables are overridable at the ship level

    //! Maximum operating output of the reactor
    HS_UINT32 *m_puiMaximumOutput;

    //! Thousdands of distance units per fuel unit
    HS_UINT32 *m_puiEfficiency;

    //! Optional fuel source to consume during operation
    CHSFuelSystem *m_fuel_source;
};

#endif // __HSREACTOR_H__
