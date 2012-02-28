// -----------------------------------------------------------------------
//! $Id: hswarpdrive.cpp,v 1.11 2006/12/16 01:25:01 mark Exp $
// -----------------------------------------------------------------------

#include "hstypes.h"
#include "hsobjects.h"
#include "hsship.h"
#include "hspace.h"
#include "hsansi.h"
#include "hsconf.h"
#include "hsinterface.h"
#include "hswarpdrive.h"

#include <math.h>

CHSWarpDrive::CHSWarpDrive() :
    mFuelSource(NULL),
    mEngaged(false),
    mMaxWarp(MAX_WARP),
    mDesiredWarp(0.0),
    mCurrentWarp(0.0),
    mAcceleration(0.1),
    mFuelConsumption(1.0)
{
    SetType(HSS_WARP_DRIVE);
}

void CHSWarpDrive::GetAttributeList(CHSAttributeList & rlistAttrs)
{
    // Call the base class first.
    CHSEngSystem::GetAttributeList(rlistAttrs);

    // Push our own attributes.
    rlistAttrs.push_back("ACCELERATION");
    rlistAttrs.push_back("CURRENT SPEED");
    rlistAttrs.push_back("CURRENT WARP");
    rlistAttrs.push_back("DESIRED WARP");
    rlistAttrs.push_back("ENGAGED");
    rlistAttrs.push_back("FUEL CONSUMPTION");
    rlistAttrs.push_back("MAX WARP");

}

HS_BOOL8 CHSWarpDrive::GetAttributeValue(const HS_INT8 * pcAttrName,
                                         CHSVariant & rvarValue,
                                         HS_BOOL8 bAdjusted,
                                         HS_BOOL8 bLocalOnly)
{
    // Determine attribute, and return the value.
    if (!strcasecmp(pcAttrName, "ENGAGED"))
    {
        rvarValue = mEngaged ? 1 : 0;
        return true;
    }
    else if (!strcasecmp(pcAttrName, "MAX WARP"))
    {
        if(bAdjusted)
        {
            rvarValue =  GetAdjustedMaxWarp();
        }
        else
        {
            rvarValue = mMaxWarp;
        }
        
        return true;
    }
    else if (!strcasecmp(pcAttrName, "ACCELERATION"))
    {
        rvarValue = mAcceleration;
        return true;
    }
    else if (!strcasecmp(pcAttrName, "FUEL CONSUMPTION"))
    {
        rvarValue = mFuelConsumption;
        return true;
    }
    else if (!strcasecmp(pcAttrName, "CURRENT WARP")) 
    {
        rvarValue = mCurrentWarp;
        return true;
    }
    else if (!strcasecmp(pcAttrName, "DESIRED WARP")) 
    {
        rvarValue = mDesiredWarp;
        return true;
    }
    else if (!strcasecmp(pcAttrName, "CURRENT SPEED")) 
    {
        rvarValue = CalculateSpeed();
        return true;
    }
    else
    {
        return CHSEngSystem::GetAttributeValue(pcAttrName, rvarValue,
                                               bAdjusted, bLocalOnly);
    }
}

HS_BOOL8 CHSWarpDrive::SetAttributeValue(const HS_INT8 * pcAttrName,
                                         const HS_INT8 * strValue)
{
    if (!strcasecmp(pcAttrName, "ENGAGED"))
    {
        mEngaged = atoi(strValue) == 0 ? false : true;
        return true;
    }
    else if (!strcasecmp(pcAttrName, "MAX WARP"))
    {
        mMaxWarp = atof(strValue);
        if(MAX_WARP < mMaxWarp)
        {
            mMaxWarp = MAX_WARP;
        }
        return true;
    }
    else if (!strcasecmp(pcAttrName, "ACCELERATION"))
    {
        mAcceleration = atof(strValue);

        if(mAcceleration > 1.0)
        {
            mAcceleration = 1.0;
        }
        return true;
    }
    else if (!strcasecmp(pcAttrName, "FUEL CONSUMPTION"))
    {
        mFuelConsumption = atof(strValue);

        if(mFuelConsumption < 0)
        {
            mFuelConsumption = 0.0;
        }
        return true;
    }
    else if (!strcasecmp(pcAttrName, "DESIRED WARP")) 
    {
        mDesiredWarp = atof(strValue);
        if(mDesiredWarp > mMaxWarp)
        {
            mDesiredWarp = mMaxWarp;
        }
        return true;
    }
    else if (!strcasecmp(pcAttrName, "CURRENT WARP")) 
    {
        mCurrentWarp = atof(strValue);
        if(mCurrentWarp > mMaxWarp)
        {
            mCurrentWarp = mMaxWarp;
        }
        return true;
    }

    return CHSEngSystem::SetAttributeValue(pcAttrName, strValue);
}

// Gets the fuel system for the engines. Replaces the m_fuel_source variable.
CHSFuelSystem* CHSWarpDrive::GetFuelSource(void)
{
	CHSShip *cShip = (CHSShip *) GetOwnerObject();
	if (!cShip)
    {
		return NULL;
    }
	return (CHSFuelSystem *)cShip->GetSystems().GetSystem(HSS_FUEL_SYSTEM);
}



void CHSWarpDrive::SaveToFile(FILE * fp)
{

    if(NULL == fp)
    {
        hs_log("CHSWarpDrive::SaveToFile() - Invalid file pointer.");
        return;
    }

    // Save the base first
    CHSEngSystem::SaveToFile(fp);

    // Save our stuff
    fprintf(fp, "ENGAGED=%d\n", mEngaged);
    fprintf(fp, "MAX WARP=%f\n", mMaxWarp);
    fprintf(fp, "ACCELERATION=%f\n", mAcceleration);
    fprintf(fp, "FUEL CONSUMPTION=%f\n", mFuelConsumption);
    fprintf(fp, "CURRENT WARP=%f\n", mCurrentWarp);
    fprintf(fp, "DESIRED WARP=%f\n", mDesiredWarp);

}

void CHSWarpDrive::DoCycle()
{
    // Do base stuff first
    CHSEngSystem::DoCycle();

    if(false == mEngaged)
    {
        mCurrentWarp = mDesiredWarp = 0.0;
        return;
    }

    // Process Acceleration if necessary
    if(mCurrentWarp < mDesiredWarp)
    {
        mCurrentWarp += mAcceleration;

        // Limit Current Warp to the desired value
        if(mCurrentWarp > mDesiredWarp)
        {
            mCurrentWarp = mDesiredWarp;
        }

    }
    else if(mCurrentWarp >= mDesiredWarp)
    {
        mCurrentWarp = mDesiredWarp;
    }

    // Limit Current Warp to the Maximum Warp for this system
    HS_FLOAT32 realmax = GetAdjustedMaxWarp();
    if(mCurrentWarp > realmax)
    {
        mCurrentWarp = realmax;
    }

    ConsumeFuel();
}

// Handle all changes to the desired warp level along with appropriate
// messaging to the ship occupants
void CHSWarpDrive::SetDesiredWarp(HS_FLOAT32 level)
{
    HS_FLOAT32 prev_warp = mDesiredWarp;

    HS_FLOAT32 adjMax = GetAdjustedMaxWarp();

    // Fix limit to constant value
    if(level > MAX_WARP)
    {
        level = MAX_WARP;
    }

    // Check to ensure it doesn't exceed the current power
    // limited capabilities
    if(level > adjMax)
    {
        mDesiredWarp = adjMax;
    }
    else
    {
        mDesiredWarp = level;
    }

    // Check for disengage, avoid direct comparison to 0 based on floating
    // point variation among systems
    if(mDesiredWarp < 0.0001 && prev_warp > 0.0001)
    {
        SendChangeNotification(WARP_DISENGAGE);
        mDesiredWarp = 0.00;
        mEngaged = false;
    }
    else if(prev_warp < 0.001 && mDesiredWarp > 0.001) // Engage 
    {
        SendChangeNotification(WARP_ENGAGE);
        mEngaged = true;
    }
    else if(prev_warp < mDesiredWarp)  // Warp level increased
    {
        SendChangeNotification(WARP_INCREASE);
    }
    else // Warp level decrease
    {
        SendChangeNotification(WARP_DECREASE);
    }
}

// Handle various messages
void CHSWarpDrive::SendChangeNotification(WarpLevelChange change)
{
    CHSShip* cShip = NULL;
    if(NULL == GetOwnerObject())
    {
        hs_log(
            "CHSWarpDrive::SendChangeNotification() -- System has no parent.");
        return;
    }

    HS_INT8 tbuf[256];

    switch(change)
    {
        case WARP_INCREASE:
            sprintf(tbuf, "%s%s-%s  Warp Level increased to %.2f.",
                ANSI_HILITE, ANSI_YELLOW, ANSI_NORMAL, mDesiredWarp);
            GetOwnerObject()->HandleMessage(tbuf, MSG_ENGINEERING);
            break;
        case WARP_DECREASE:
            sprintf(tbuf, "%s%s-%s  Warp Level decreased to %.2f.",
                ANSI_HILITE, ANSI_YELLOW, ANSI_NORMAL, mDesiredWarp);
            GetOwnerObject()->HandleMessage(tbuf, MSG_ENGINEERING);
            break;
        case WARP_ENGAGE:
            sprintf(tbuf, 
                    "%s%s-%s  The ship accelerates as the warp drive engages.",
                    ANSI_HILITE, ANSI_YELLOW, ANSI_NORMAL);
            if(GetOwnerObject()->GetType() == HST_SHIP)
            {
                cShip = (CHSShip*) GetOwnerObject();
                cShip->NotifySrooms(tbuf);
            }
            sprintf(tbuf, "%s%s-%s  Warp Level set to %.2f.",
                ANSI_HILITE, ANSI_YELLOW, ANSI_NORMAL, mDesiredWarp);
            GetOwnerObject()->HandleMessage(tbuf, MSG_ENGINEERING);
            break;
        case WARP_DISENGAGE:
            sprintf(tbuf, 
                    "%s%s-%s  The ship drops into sublight speed as the warp drive disengages.",
                    ANSI_HILITE, ANSI_YELLOW, ANSI_NORMAL);
            if(GetOwnerObject()->GetType() == HST_SHIP)
            {
                cShip = (CHSShip*) GetOwnerObject();
                cShip->NotifySrooms(tbuf);
            }
            sprintf(tbuf, "%s%s-%s  Warp Level set to %.2f.",
                ANSI_HILITE, ANSI_YELLOW, ANSI_NORMAL, mDesiredWarp);
            GetOwnerObject()->HandleMessage(tbuf, MSG_ENGINEERING);
            break;
        case WARP_OUT_OF_FUEL:
            sprintf(tbuf,
                    "%s%s-%s A warning light flashes, indicating warp drives \
                    have run out of fuel.",
                    ANSI_HILITE, ANSI_YELLOW, ANSI_NORMAL);
            GetOwnerObject()->HandleMessage(tbuf, MSG_ENGINEERING, NULL);
            SendChangeNotification(WARP_DISENGAGE);
            break;
        default:
            hs_log("CHSWarpDrive::SendChangeNotification() - invalid case!");
    }

}

// speed = warp^(hsconf.warp_exponent) * hsconf.warp_constant
// return raw measurment and let ship methods decrease to the current
// cycle interval for movement calculations
HS_FLOAT32 CHSWarpDrive::CalculateSpeed()
{
    HS_FLOAT32 speed = 0.0;

    if(false == mEngaged || mCurrentWarp < 0.0001)
    {
        return 0.0;
    }

    speed = pow(mCurrentWarp, HSCONF.warp_exponent) * HSCONF.warp_constant;
    return speed;
}

// Consume fuel for a single cycle
void CHSWarpDrive::ConsumeFuel()
{
    // Fuel system doesn't exist or the drive isn't engaged - do nothing
    if(NULL == GetFuelSource() || false == mEngaged)
    {
        return;
    }

    // Check to see if there is fuel to be consumed
    if(GetFuelSource()->GetFuelRemaining() <= 0)
    {
        return;
    }

    // Calculate how much fuel to consume for this single cycle
    float consumed = mCurrentWarp * mFuelConsumption * 0.0002778;
    float actual_consumption = 0.0;

    // Extract the fuel and see if the extracted amount matches
    // what was consumed
    actual_consumption = GetFuelSource()->ExtractFuelUnit(consumed);

    // If actual is less than calculated consumption, the ship
    // has run out of fuel
    if(actual_consumption < consumed) // out of fuel
    {
        mEngaged = false;
        mCurrentWarp = mDesiredWarp = 0.0;
        SendChangeNotification(WARP_OUT_OF_FUEL);
        SendChangeNotification(WARP_DISENGAGE);
    }
}

// Calculate the actual maximum warp value based on the current power
// ratio.  This allows either underpowered or overpowered drives to 
// function based on a linear scale
HS_FLOAT32 CHSWarpDrive::GetAdjustedMaxWarp()
{
    HS_FLOAT32 real_max = 0.0;

    real_max = mMaxWarp * 
        ( (HS_FLOAT32) GetCurrentPower() / (HS_FLOAT32) GetOptimalPower() );

    return real_max;
}
