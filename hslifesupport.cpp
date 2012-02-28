// -----------------------------------------------------------------------
//! $Id: hslifesupport.cpp,v 1.5 2006/04/04 12:41:11 mark Exp $
// -----------------------------------------------------------------------

#include "pch.h"

#include "hsobjects.h"
#include "hsship.h"
#include "hsconf.h"
#include "hsinterface.h"
#include "hslifesupport.h"

CHSSysLifeSupport::CHSSysLifeSupport()
{
    SetType(HSS_LIFE_SUPPORT);

    // Initially 100% air left
    m_perc_air = 100;
}

void CHSSysLifeSupport::PowerUp(int level)
{
    if (!GetOwnerObject())
    {
        return;
    }

    if (GetOwnerObject()->GetType() == HST_SHIP)
    {
        CHSShip *cShip;

        cShip = (CHSShip *) GetOwnerObject();

        if (hsInterface.AtrGet(cShip->GetDbref(), "HSMSG_LIFE_ACTIVATING"))
        {
            char *msg = hsInterface.EvalExpression(hsInterface.m_buffer,
                                                   cShip->GetDbref(),
                                                   cShip->GetDbref(),
                                                   cShip->GetDbref());
            cShip->NotifySrooms(msg);
        }
        else
        {
            cShip->NotifySrooms(HSCONF.life_activating);
        }
    }
}

void CHSSysLifeSupport::CutPower(int level)
{
    if (level == 0)
    {
        if (GetOwnerObject() && GetOwnerObject()->GetType() == HST_SHIP)
        {
            CHSShip *cShip;

            cShip = (CHSShip *) GetOwnerObject();

            if (hsInterface.AtrGet(cShip->GetDbref(), "HSMSG_LIFE_CUT"))
            {
                char *msg = hsInterface.EvalExpression(hsInterface.m_buffer,
                                                       cShip->GetDbref(),
                                                       cShip->GetDbref(),
                                                       cShip->GetDbref());
                cShip->NotifySrooms(msg);
            }
            else
            {
                cShip->NotifySrooms(HSCONF.life_cut);
            }
        }
    }
}


// The DoCycle() for life support will handle increasing or
// decreasing life support percentages (e.g. air).
void CHSSysLifeSupport::DoCycle()
{
    double dLifePerc;

    // Do base cycle stuff
    CHSEngSystem::DoCycle();

    // Based on power availability, determine what
    // percentage of life support should be maintained.
    dLifePerc = (double) GetOptimalPower() + .00001;
    dLifePerc = (GetCurrentPower() / dLifePerc) * 100;

    // Handle life support goodies
    if (m_perc_air < dLifePerc)
    {
        // Increase air availability by 1 percent
        m_perc_air += 1;

        // Did we increase too much?
        if (m_perc_air > dLifePerc)
            m_perc_air = (float) dLifePerc;
    }
    else if (m_perc_air > dLifePerc)
    {
        // Decrease air availability by 1/3 percent because
        // of people breathing.
        m_perc_air = (float) (m_perc_air - .33333);

        // Did we decrease too much?
        if (m_perc_air < dLifePerc)
            m_perc_air = (float) dLifePerc;
    }
}

// Returns the current 0 - 100 percentage of air in the system
float CHSSysLifeSupport::GetAirLeft()
{
    return m_perc_air;
}


void CHSSysLifeSupport::SetAirLeft(float fValue)
{
    if (fValue > 100)
    {
        fValue = 100;
    }
    else if (fValue < 0)
    {
        fValue = 0.0;
    }

    m_perc_air = fValue;
}
