// -----------------------------------------------------------------------
//! $Id: hsthrusters.h,v 1.3 2006/04/04 12:41:36 mark Exp $
// -----------------------------------------------------------------------
#if !defined(__HSTHRUSTERS_H__)
#define __HSTHRUSTERS_H__

#include "hseng.h"


//! Thrusters, which control steering for the ship.
class CHSSysThrusters:public CHSEngSystem
{
  public:
    //! Set system type to HSS_THRUSTERS
    CHSSysThrusters();

    //! Deallocate memory if needed
    ~CHSSysThrusters();

    //! Set the specified attribute, pcAttrName, to pcValue if possible
    HS_BOOL8 SetAttributeValue(const HS_INT8 * pcAttrName,
                               const HS_INT8 * pcValue);

    //! Return the current list of manipulatable attributes on the system
    void GetAttributeList(CHSAttributeList & rlistAttrs);

    //! Get an attribute value from the thruster system
    HS_BOOL8 GetAttributeValue(const HS_INT8 * pcAttrName,
                               CHSVariant & rvarReturnVal,
                               HS_BOOL8 bAdjusted,
                               HS_BOOL8 bLocalOnly = false);

    //! Returns the turning rate for the thrusters system.  If bAdjusted
    //! is set to false, then the maximum raw turning rate is returned,
    //! otherwise, it is adjusted for damage and power levels.
    HS_UINT32 GetRate(HS_BOOL8 bAdjusted = true);

    //! Write system data to the specified file
    void SaveToFile(FILE * fp);

    //! Power thrusters to the specified level
    void PowerUp(int level);

    //! Set the current turning rate, allocate memory if needed
    void SetTurningRate(HS_UINT32 iValue)
    {
        if (!m_puiTurningRate)
        {
            m_puiTurningRate = new HS_UINT32;
        }

           *m_puiTurningRate = iValue;
    }


    //! Return a pointer to the raw turning rate for this ship
    HS_UINT32 *GetRawTurningRate()
    {
        return m_puiTurningRate;
    }

  protected:
    //! Internal turning rate variable used to override ship class parent
    HS_UINT32 * m_puiTurningRate;
};

#endif // __HSTHRUSTERS_H__
