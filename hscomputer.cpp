// -----------------------------------------------------------------------
// $Id: hscomputer.cpp,v 1.6 2006/04/04 12:41:11 mark Exp $
// -----------------------------------------------------------------------

#include "pch.h"

#include "hsobjects.h"
#include "hsship.h"
#include "hsconf.h"
#include "hsinterface.h"
#include "hscomputer.h"

CHSSysComputer::CHSSysComputer()
{
    m_power_used = 0;
    SetType(HSS_COMPUTER);
}

// Call this function to drain power from the computer
// to your system.  If enough power is available, true
// is returned, and the power is deducted from the 
// total power available.
HS_BOOL8 CHSSysComputer::DrainPower(int amt)
{
    if ((m_power_used + amt) > (int) GetCurrentPower())
        return false;

    m_power_used += amt;
    return true;
}

int CHSSysComputer::GetUsedPower()
{
    return m_power_used;
}

int CHSSysComputer::GetConsoles()
{
    int nConsoles;
    CHSShip *cShip;
    int idx;
    CHSConsole *cCons;

    cShip = (CHSShip *) GetOwnerObject();
    nConsoles = 0;
    // Query consoles
    for (idx = 0; idx < MAX_SHIP_CONSOLES; idx++)
    {
        cCons = cShip->GetConsole(idx);
        if (cCons)
        {
            nConsoles++;
        }
    }

    return nConsoles;
}

int CHSSysComputer::GetPoweredConsoles()
{
    int nPConsoles;
    CHSShip *cShip;
    int idx;
    CHSConsole *cCons;

    if (!GetOwnerObject())
    {
        return 0;
    }

    cShip = (CHSShip *) GetOwnerObject();

    nPConsoles = 0;
    // Query consoles
    for (idx = 0; idx < MAX_SHIP_CONSOLES; idx++)
    {
        cCons = cShip->GetConsole(idx);
        if (cCons)
        {
            if (cCons->IsOnline())
            {
                nPConsoles++;
            }
        }
    }

    return nPConsoles;
}

void CHSSysComputer::PowerUp(int level)
{
    if (GetOwnerObject() && (GetOwnerObject()->GetType() == HST_SHIP))
    {
        CHSShip *cShip;

        cShip = (CHSShip *) GetOwnerObject();

        if (hsInterface.
            AtrGet(cShip->GetDbref(), "HSMSG_COMPUTER_ACTIVATING"))
        {
            char *msg = hsInterface.EvalExpression(hsInterface.m_buffer,
                                                   cShip->GetDbref(),
                                                   cShip->GetDbref(),
                                                   cShip->GetDbref());
            cShip->NotifySrooms(msg);
        }
        else
        {
            cShip->NotifySrooms(HSCONF.computer_activating);
        }

        if (level > 5)
            m_power_used = 5;
        else
            m_power_used = level;
    }
}

void CHSSysComputer::CutPower(int level)
{
    if (level >= m_power_used)
        return;

    int idx;
    CHSConsole *cCons;
    CHSShip *cShip;

    cShip = (CHSShip *) GetOwnerObject();

    if (!cShip)
        return;


    for (idx = 0; idx < MAX_SHIP_CONSOLES; idx++)
    {
        cCons = cShip->GetConsole(idx);
        if (cCons)
        {
            if (cCons->IsOnline())
            {
                cCons->PowerDown(hsInterface.ConsoleUser(cCons->m_objnum));
            }
        }
        if (level >= m_power_used)
            return;
    }

    if (level < 5)
        m_power_used = level;
}
// Call this function to release power you've drained.
// If you don't, the computer gets confused about how
// much power is used.  Kind of like a memory leak!
void CHSSysComputer::ReleasePower(int amt)
{
    m_power_used -= amt;
}

// Returns the surplus (or deficit) between power allocated
// to the computer and power drained.  If this is negative,
// your system should consider releasing the power.  The
// computer won't do it.
int CHSSysComputer::GetPowerSurplus()
{
    return (GetCurrentPower() - m_power_used);
}

// Returns the power usage for the computer system.  This
// can vary depending on the owner object for the system.
// For example, ships have consoles, weapons, etc.
int CHSSysComputer::GetOptimalPower(HS_BOOL8 bDamage)
{
    HS_UINT32 uPower;

    if (!GetOwnerObject())
    {
        return 0;
    }

    if (GetDamageLevel() == DMG_INOPERABLE)
    {
        return 0;
    }

    // Computer requires 5 MW base power
    uPower = 5;

    // We currently only support ship computers
    if (HST_SHIP == GetOwnerObject()->GetType())
    {
        uPower += TotalShipPower();
    }

    return uPower;
}

// Support for ship objects with computers.  This will
// query various parts of the ship (e.g. consoles) for
// power requirements.
HS_UINT32 CHSSysComputer::TotalShipPower()
{
    HS_UINT32 uPower;
    CHSShip *cShip;
    int idx;
    CHSConsole *cCons;

    cShip = (CHSShip *) GetOwnerObject();
    uPower = 0;
    // Query consoles
    for (idx = 0; idx < MAX_SHIP_CONSOLES; idx++)
    {
        cCons = cShip->GetConsole(idx);
        if (cCons)
            uPower += cCons->GetMaximumPower();
    }

    return uPower;
}
