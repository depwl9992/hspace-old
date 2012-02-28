// -----------------------------------------------------------------------
// $Id: hslifesupport.h,v 1.5 2006/04/04 12:56:10 mark Exp $
// -----------------------------------------------------------------------
#if !defined(__HSLIFESUPPORT_H__)
#define __HSLIFESUPPORT_H__

#include "hseng.h"

//! The life support system, which provides life supporting
//! goodies to a vessel.
class CHSSysLifeSupport:public CHSEngSystem
{
  public:
    //! Set system type to HSS_LIFE_SUPPORT & m_perc_air to 100
    CHSSysLifeSupport();

    //! Default deconstructor does nothing.
    virtual ~CHSSysLifeSupport()
    {
    }

    //! Cycle the system once, echoing messages of increasing or decreasing air
    void DoCycle();

    //! Return a floating point level of air left from 0 to 100
    float GetAirLeft();

    //! Set the current level or air left from 0 to 100;
    void SetAirLeft(float fValue);

    //! @brief Power up the life support system for the ship
    //! @param level -- currently ignored
    void PowerUp(int level);


    //! @brief Cut power to the life support system
    //! @param level - if 0, life support is shut off otherwise it is ignored
    void CutPower(int level);

  protected:
    //! Current percentage of air remaining in the vessel
    float m_perc_air;
};

#endif // __HSLIFESUPPORT_H__
