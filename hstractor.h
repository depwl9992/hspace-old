// -----------------------------------------------------------------------
// $Id: hstractor.h,v 1.7 2006/04/04 12:56:10 mark Exp $
// -----------------------------------------------------------------------

#if !defined(__HSTRACTOR_H__)
#define __HSTRACTOR_H__

// Library Includes
#include "hseng.h"

//! Tractor Beam Mode Types
enum HS_TMODE
{
    HSTM_TRACTOR = 0,           // Bring target toward a vessel
    HSTM_REPULSE,               // Push target away from vessel
    HSTM_HOLD                   // Hold a target in a given position
};


//! Tractor Beams, tractor, repulse or hold ships within the beam range.
class CHSSysTractor:public CHSEngSystem
{
  public:
    //! Default constructor
    CHSSysTractor();
    //! Deault deconstructor
    virtual ~CHSSysTractor();

    //! Set an attribute value for the tractor system
    HS_BOOL8 SetAttributeValue(const HS_INT8 * strName,
                               const HS_INT8 * strValue);
    //! Get the current list of manipulatable attributes
    void GetAttributeList(CHSAttributeList & rlistAttrs);

    //! Get a current value from the tractor system
    HS_BOOL8 GetAttributeValue(const HS_INT8 * pcAttrName,
                               CHSVariant & rvarReturnVal,
                               HS_BOOL8 bAdjusted,
                               HS_BOOL8 bLocalOnly = false);

    //! @brief Set the current beam mode
    //! @param mode - 0: tractor, 1: repulse, 2: hold
    void SetMode(int mode);

    //! Set the current lock to a specified sensor id
    void SetLock(HS_DBREF player, int id);

    //! Releae the current lock
    void ReleaseLock(HS_DBREF player);

    //! Save information to the specified file
    void SaveToFile(FILE * fp);

    //! Perform a single cycle of the tractor beam system
    void DoCycle();

    //! @brief Dock the sensor id'd ship at loc 
    //! @param player - enacting player
    //! @param id - sensor id to dock with
    //! @param loc - docking/landing location on this vessel to use
    void DockShip(HS_DBREF player, int id, int loc);


    //! @brief Get the current strength value, adjusted for current/optimal power
    //! if bAdjusted is true
    float GetStrength(HS_BOOL8 bAdjusted);

    //! Return the current effectiveness rate GetStrength(true)/target size
    float GetEffect();

    //! Get the current mode of the tractor system
    HS_TMODE GetMode();

    //! Return the object information on the current locked vessel
    CHS3DObject *GetLock();

    //! Set the current strength to dValue
    void SetStrength(float dValue)
    {
        if (!m_strength)
        {
            m_strength = new float;
        }
           *m_strength = dValue;
    }


    //! Return a pointer to the allocated raw strength
    float *GetRawStrength()
    {
        return m_strength;
    }

  protected:

    //! Strength of the tractor beam.
    float *m_strength;

    //! Current Mode for the tractor beam
    HS_TMODE m_mode;

    //! Object currently locked by the tractor beam
    CHS3DObject *m_lock;
};

#endif // __HSTRACTOR_H__
