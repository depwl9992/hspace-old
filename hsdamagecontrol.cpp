// -----------------------------------------------------------------------
//! $Id: hsdamagecontrol.cpp,v 1.9 2006/04/26 23:53:15 mark Exp $
// -----------------------------------------------------------------------

#include "pch.h"

#include "hsobjects.h"
#include "hsship.h"
#include "hsinterface.h"
#include "hsutils.h"
#include "hsansi.h"
#include "hsconf.h"
#include "hspace.h"

#include "hsdamagecontrol.h"

CHSDamCon::CHSDamCon():
m_puiNumCrews(NULL)
{
    SetType(HSS_DAMCON);
}

CHSDamCon::~CHSDamCon()
{
}

void CHSDamCon::SetParentSystem(CHSEngSystem * pParent)
{
    // Call the base class first.
    CHSEngSystem::SetParentSystem(pParent);

    // Find out how many damage crews this parent has.
    HS_INT32 iNumCrews = static_cast < CHSDamCon * >(pParent)->GetNumCrews();

    m_vecCrews.resize(iNumCrews + 1);
}

void CHSDamCon::GetAttributeList(CHSAttributeList & rlistAttrs)
{
    // Base class first.
    CHSEngSystem::GetAttributeList(rlistAttrs);

    // Now our attributes.
    rlistAttrs.push_back("NUMCREWS");
}

HS_BOOL8 CHSDamCon::GetAttributeValue(const HS_INT8 * pcAttrName,
                                      CHSVariant & rvarValue,
                                      HS_BOOL8 bAdjusted, HS_BOOL8 bLocalOnly)
{
    // Determine attribute, and return the value.
    if (!strcasecmp(pcAttrName, "NUMCREWS"))
    {
        if (m_puiNumCrews)
        {
            rvarValue = *m_puiNumCrews;
        }
        else if (!bLocalOnly)
        {
            rvarValue = GetNumCrews();
        }
        else
        {
            return false;
        }
        return true;
    }
    else
    {
        return CHSEngSystem::GetAttributeValue(pcAttrName, rvarValue,
                                               bAdjusted, bLocalOnly);
    }
}

void CHSDamCon::SaveToFile(FILE * fp)
{
    // Save the base first
    CHSEngSystem::SaveToFile(fp);

    // Save our stuff
    if (m_puiNumCrews)
    {
        fprintf(fp, "NUMCREWS=%i\n", *m_puiNumCrews);
    }
}

HS_UINT32 CHSDamCon::GetNumCrews()
{
    HS_UINT32 rval;

    // Do we have a local value?
    if (!m_puiNumCrews)
    {
        // Do we have a parent?
        if (!GetParent())
        {
            return 0;
        }
        else
        {
            CHSDamCon *ptr;
            ptr = (CHSDamCon *) GetParent();
            rval = ptr->GetNumCrews();
        }
    }
    else
        rval = *m_puiNumCrews;

    return rval;

}

HS_BOOL8 CHSDamCon::SetAttributeValue(const HS_INT8 * pcAttrName,
                                      const HS_INT8 * strValue)
{
    // Match the name .. set the value
    if (!strcasecmp(pcAttrName, "NUMCREWS"))
    {
        if (atoi(strValue) < 0)
        {
            return false;
        }
        else
        {
            // If no value is to be set, remove all crews.
            if (!*strValue)
            {
                if (m_puiNumCrews)
                {
                    delete m_puiNumCrews;
                    m_puiNumCrews = NULL;
                }
            }
            else
            {
                if (!m_puiNumCrews)
                {
                    m_puiNumCrews = new HS_UINT32;
                }
                *m_puiNumCrews = atoi(strValue);

                // Reserve m_puiNumCrews vector slots in the STL vector.
                m_vecCrews.resize(*m_puiNumCrews + 1);
            }
        }
        return true;
    }
    else
    {
        return CHSEngSystem::SetAttributeValue(pcAttrName, strValue);
    }
}

CHSEngSystem *CHSDamCon::GetWorkingCrew(int crew)
{
    // Be certain this is a valid crew ID.
    if ((crew < 1) || (crew > (int) GetNumCrews()))
    {
        return NULL;
    }

    THSDamageCrew & rtCrew = m_vecCrews[crew];
    return rtCrew.pAssignedTo;
}

int CHSDamCon::GetCyclesLeft(int crew)
{
    // Be certain this is a valid crew ID.
    if ((crew < 1) || (crew >= (int) GetNumCrews()))
    {
        return 0;
    }

    THSDamageCrew & rtCrew = m_vecCrews[crew];
    return rtCrew.iSecondsLeft;
}

void CHSDamCon::DoCycle()
{
    // First we do the basic cycle, stress and such.
    CHSEngSystem::DoCycle();

    // Now we'll just go through our damage crews.
    HS_INT32 iEfficiency;
    HS_INT32 iRandNum;

    // Calculate percentage of power allocated to damage control.
    if (GetCurrentPower() != 0 && GetOptimalPower(false) != 0)
    {
        iEfficiency = (int) (100 * ((GetCurrentPower() * 1.00) /
                                    (GetOptimalPower(false) * 1.00)));
    }
    else
    {
        iEfficiency = 0;
    }

    iRandNum = hsInterface.GetRandom(99) + 1;

    // If we roll a random number greater than the power allocated to damage
    // control, then this round of damage control goes into the toilet.
    if (iRandNum > iEfficiency)
    {
        return;
    }

    // Insure that our number of crews matches the vector size.
    m_vecCrews.resize(GetNumCrews() + 1);

    // Run through all of our damage crews.
    // Those that are assigned get their time remaining reduced.
    unsigned int idx;
    for (idx = 1; idx <= GetNumCrews(); idx++)
    {
        THSDamageCrew & rtCrew = m_vecCrews[idx];

        // Is this crew working on anything?
        if (!rtCrew.pAssignedTo)
        {
            continue;
        }

        // This crew's time decreases.
        if (rtCrew.iSecondsLeft > 0)
        {
            rtCrew.iSecondsLeft--;
        }

        // If the crew is done working, reduce the damage on the system.
        if (rtCrew.iSecondsLeft <= 0)
        {
            rtCrew.pAssignedTo->ReduceDamage();

            // If this system is fully repaired, notify everyone, and pull this
            // crew off duty.
            if (rtCrew.pAssignedTo->GetDamageLevel() == DMG_NONE)
            {
                // If this system is part of a ship,
                // notify all consoles that repairs are complete.
                if (GetOwnerObject()->GetType() == HST_SHIP)
                {
                    CHSShip *pOwnerShip =
                        static_cast < CHSShip * >(GetOwnerObject());

                    pOwnerShip->NotifyConsoles(hsInterface.
                                               HSPrintf
                                               ("%s%s-%s %s repairs complete.",
                                                ANSI_HILITE, ANSI_GREEN,
                                                ANSI_NORMAL,
                                                rtCrew.pAssignedTo->
                                                GetName()), MSG_ENGINEERING);
                }

                // This crew is no longer assigned.
                rtCrew.pAssignedTo = NULL;
            }
            else
            {
                // This system is not repaired.  Reassign the crew to another
                // round of repairs.
                rtCrew.iSecondsLeft = HSCONF.damage_repair_time;

                if (0 == rtCrew.iSecondsLeft)
                {
                    rtCrew.iSecondsLeft = 1;
                }

                if (GetOwnerObject()->GetType() == HST_SHIP)
                {
                    CHSShip *pOwnerShip =
                        static_cast < CHSShip * >(GetOwnerObject());

                    pOwnerShip->NotifyConsoles(hsInterface.
                                               HSPrintf
                                               ("%s%s-%s %s repairs incomplete, continuing repairs.",
                                                ANSI_HILITE, ANSI_GREEN,
                                                ANSI_NORMAL,
                                                rtCrew.pAssignedTo->
                                                GetName()), MSG_ENGINEERING);
                }
            }
        }
    }
}
void CHSDamCon::UnassignCrew(HS_DBREF player, int iCrew)
{
    // Be certain this is a valid crew ID.
    if ((iCrew < 1) || (iCrew > (int) GetNumCrews()))
    {
        return;
    }
    
    // Grab this crew's structure from the vector.
    THSDamageCrew & rtCrew = m_vecCrews[iCrew];
    // Only one crew can be assigned to a system at any one time.
    HS_INT32 iNumCrews = GetNumCrews();

    for (int idx = 0; idx < iNumCrews; idx++)
    {
        // If this crew is the one we're assigning, ignore it.
        if (idx == iCrew)
        {
            rtCrew.pAssignedTo = NULL;
            rtCrew.iSecondsLeft = 0;
            if (GetOwnerObject()->GetType() == HST_SHIP)
            {
                CHSShip *pOwnerShip = static_cast <CHSShip * >
                    (GetOwnerObject());

                pOwnerShip->NotifyConsoles(hsInterface.
                        HSPrintf(
                            "%s%s-%s Damage Control crew %i now available.",
                            ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, iCrew), 
                        MSG_ENGINEERING);
            }
        }
    }
}

void CHSDamCon::AssignCrew(HS_DBREF player, int iCrew, CHSEngSystem * pSystem)
{
    // Be certain this is a valid crew ID.
    if ((iCrew < 1) || (iCrew > (int) GetNumCrews()))
    {
        return;
    }

    // Grab this crew's structure from the vector.
    THSDamageCrew & rtCrew = m_vecCrews[iCrew];

    // If we're assigning to a valid system, try that.  
    // Otherwise, unassign this crew.
    if (pSystem)
    {
        // Only one crew can be assigned to a system at any one time.
        HS_INT32 iNumCrews = GetNumCrews();

        for (int idx = 0; idx < iNumCrews; idx++)
        {
            // If this crew is the one we're assigning, ignore it.
            if (idx == iCrew)
            {
                continue;
            }

            THSDamageCrew & rtAssignedCrew = m_vecCrews[idx];

            // Is this crew assigned to the system we want?
            if (rtAssignedCrew.pAssignedTo == pSystem)
            {
                hsStdError(player,
                           "A repair crew is already working on that system.");
                return;
            }
        }

        // This crew is now assigned.
        rtCrew.pAssignedTo = pSystem;
        rtCrew.iSecondsLeft = HSCONF.damage_repair_time;

        if (0 == rtCrew.iSecondsLeft)
        {
            rtCrew.iSecondsLeft = 1;
        }

        if (GetOwnerObject()->GetType() == HST_SHIP)
        {
            CHSShip *pOwnerShip = static_cast < CHSShip * >(GetOwnerObject());

            pOwnerShip->NotifyConsoles(hsInterface.
                                       HSPrintf
                                       ("%s%s-%s Damage Control crew %i assigned to %s.",
                                        ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
                                        iCrew, pSystem->GetName()),
                                       MSG_ENGINEERING);
        }
    }
    else
    {
        // This crew is unassigned.
        rtCrew.pAssignedTo = NULL;
        rtCrew.iSecondsLeft = 0;

        if (GetOwnerObject()->GetType() == HST_SHIP)
        {
            CHSShip *pOwnerShip = static_cast < CHSShip * >(GetOwnerObject());

            pOwnerShip->NotifyConsoles(hsInterface.
                                       HSPrintf
                                       ("%s%s-%s Damage Control crew %i now idle.",
                                        ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
                                        iCrew), MSG_ENGINEERING);
        }
    }
}
