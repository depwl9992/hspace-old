// -----------------------------------------------------------------------
//! $Id: hswarpdrive.h,v 1.8 2007/01/22 02:07:47 mhassman Exp $
// -----------------------------------------------------------------------

#ifndef __HSWARPDRIVE_INCLUDED__
#define __HSWARPDRIVE_INCLUDED__

#include "hsfuel.h"
#include "hsconf.h"
#include "hsengines.h"

static const HS_FLOAT32 MAX_WARP = 9.9999;
class CHSWarpDrive : public CHSEngSystem
{
  public:

    //! Setup the default values for the warp drive
    CHSWarpDrive();

    void GetAttributeList(CHSAttributeList & rlistAttrs);

    HS_BOOL8 GetAttributeValue(const HS_INT8 * pcAttrName,
                               CHSVariant & rvarReturnVal,
                               HS_BOOL8 bAdjusted,
                               HS_BOOL8 bLocalOnly = false);

    HS_BOOL8 SetAttributeValue(const HS_INT8 * pcAttrName,
                               const HS_INT8 * pcValue);
    void SaveToFile(FILE *fp);
    void DoCycle();

    //! Is the Warp Drive currently engaged?
    HS_BOOL8 GetEngaged() { return mEngaged; }

    //! return the warp drive acceleration
    HS_FLOAT32 GetAcceleration() { return mAcceleration; }

    //! Return current warp rating
    HS_FLOAT32 GetCurrentWarp() { return mCurrentWarp; }

    //! Return the desired warp rating
    HS_FLOAT32 GetDesiredWarp() { return mDesiredWarp; }

    //! Set the desired warp level
    void SetDesiredWarp(HS_FLOAT32 level);

    //! Set the source for consuming fuel
    void SetFuelSource(CHSFuelSystem* cFuel) { mFuelSource = cFuel; }

    //! @brief Get the current fuel source 
    //! @return NULL if not present or pointer to CHSFuelSystem
    CHSFuelSystem *GetFuelSource(void);

    //! Calculate speed based on current info
    HS_FLOAT32 CalculateSpeed();

    //! Maximum warp value 
    
  protected:

    //! A simple enum to flag what messages to issue
    enum WarpLevelChange
    {
        WARP_INCREASE = 0,
        WARP_DECREASE,
        WARP_ENGAGE,
        WARP_DISENGAGE,
        WARP_OUT_OF_FUEL
    };

    //! Internal message handling
    void SendChangeNotification(WarpLevelChange change);

    //! Consume fuel based on current warp rating
    void ConsumeFuel();

    //! Get the adjusted maximum warp level based on power settings
    HS_FLOAT32 GetAdjustedMaxWarp();

    //! Source of fuel
    CHSFuelSystem* mFuelSource;
    
    //! Is the warp drive engaged?
    HS_BOOL8 mEngaged;

    //! Maximum warp value of this vessel
    HS_FLOAT32 mMaxWarp;

    //! Desired Warp Level
    HS_FLOAT32 mDesiredWarp;

    //! Current Warp Level
    HS_FLOAT32 mCurrentWarp;

    //! Acceleration per cycle in Warp Units
    HS_FLOAT32 mAcceleration;

    //! Fuel Consumption in Units / Hour per Warp Rating
    HS_FLOAT32 mFuelConsumption;
};


#endif // __HSWARPDRIVE_INCLUDED__

