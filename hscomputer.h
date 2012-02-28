// -----------------------------------------------------------------------
// $Id: hscomputer.h,v 1.3 2006/04/04 12:41:36 mark Exp $
// -----------------------------------------------------------------------
#if !defined(__HSCOMPUTER_H__)
#define __HSCOMPUTER_H__

#include "hseng.h"

//! The computer system, which handles a lot of internal
//! ship power allocation and information handling.
class CHSSysComputer:public CHSEngSystem
{
  public:
    //! Set power use to 0 and system type to HSS_COMPUTER
    CHSSysComputer();

    //! Returns the optimal power usage for the computer system
    int GetOptimalPower(HS_BOOL8 bDamage = true);

    //! Returns the power surplus available
    int GetPowerSurplus();

    //! Returns the current number of consoles installed on the ship
    int GetConsoles();

    //! Return the current total power consumed
    int GetUsedPower();

    //! Return the number of consoles that are powered up on the ship
    int GetPoweredConsoles();

    void SetPowerUsed(int iPower)
    {
        m_power_used = iPower;
    }

    //! @brief Power up the computer for the ship
    //! @param level - level of power for the computer to consume (max 5)
    void PowerUp(int level);


    //! @brief Cut power level from the computer and power off consoles
    //! @param level - cut power to this level
    void CutPower(int level);

    //! @brief Used to drain power from the computer to the ship systems.
    //! @param amt - amount of power to drain
    //! @return true if sufficient power is available, else false
    HS_BOOL8 DrainPower(int amt);


    //! @brief Called to release power that was drained via DrainPower() to 
    //! ensure the current power use is correct
    //! @param amt - amount of power to release
    void ReleasePower(int amt);

  protected:
    //! Storage value for current amount of power consumed
    int m_power_used;

    //! Queries all ship consoles and returns the total required power
    HS_UINT32 TotalShipPower();
};

#endif // __HSCOMPUTER_H__
