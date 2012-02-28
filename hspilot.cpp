#include "pch.h"

#include "hspace.h"
#include "hsinterface.h"
#include "hsobjects.h"
#include "hsutils.h"
#include "hsansi.h"
#include "hsengines.h"
#include "hssensors.h"
#include "hsautopilot.h"
#include "hspilot.h"

#include "math.h"
#include "hsjumpdrive.h"
#include "hswarpdrive.h"

// Headers for our AIs
HSPACE_AI_HDR(navigation_seeker);
HSPACE_AI_HDR(awareness_drone);
HSPACE_AI_HDR(aggression_anybody);
HSPACE_AI_HDR(cowardice_fearless);
HSPACE_AI_HDR(manueaver_seeker);
HSPACE_AI_HDR(ordnance_fireall);

// This is where we register our AIs, attributes, and pilot profiles.
void InitPilots(void)
{
    // Register the pilot profile first
    HSPACE_PILOT_ALLOC(testpilot);
    // Register the navigation AI
    // HSPACE_AI_ALLOC(Pilot, AI name, AI type);
    HSPACE_AI_ALLOC(testpilot, navigation_seeker, CHSAI_NAVIGATION);
    HSPACE_AI_ALLOC(testpilot, manueaver_seeker, CHSAI_MANUEAVER);
    HSPACE_AI_ALLOC(testpilot, cowardice_fearless, CHSAI_COWARDICE);
    HSPACE_AI_ALLOC(testpilot, awareness_drone, CHSAI_AWARENESS);
    HSPACE_AI_ALLOC(testpilot, aggression_anybody, CHSAI_AGGRESSION);
    HSPACE_AI_ALLOC(testpilot, ordnance_fireall, CHSAI_ORDNANCE);
    // Register the trigger to be run when the ship arrives
    // HSPACE_ATTR_ALLOC(AI name, Attr name, Attr type, Default value);
    HSPACE_ATTR_ALLOC(navigation_seeker, "DESTINATION", CHSATTR_TRIGGER, "");
    HSPACE_ATTR_ALLOC(navigation_seeker, "X", CHSATTR_NOEXEC, "0");
    HSPACE_ATTR_ALLOC(navigation_seeker, "Y", CHSATTR_NOEXEC, "0");
    HSPACE_ATTR_ALLOC(navigation_seeker, "Z", CHSATTR_NOEXEC, "0");
    HSPACE_ATTR_ALLOC(navigation_seeker, "RANGE", CHSATTR_NOEXEC, "2");
    HSPACE_ATTR_ALLOC(navigation_seeker, "XYDRIFT", CHSATTR_NOEXEC, "1");
    HSPACE_ATTR_ALLOC(navigation_seeker, "ZDRIFT", CHSATTR_NOEXEC, "1");
    HSPACE_ATTR_ALLOC(navigation_seeker, "MAXSPEED", CHSATTR_NOEXEC, "1");
    HSPACE_ATTR_ALLOC(aggression_anybody, "ISHOSTILE", CHSATTR_FUNCTION, "0");
    HSPACE_ATTR_ALLOC(aggression_anybody, "ISFRIENDLY", CHSATTR_FUNCTION, "0");
    HSPACE_ATTR_ALLOC(cowardice_fearless, "HULLPERC", CHSATTR_NOEXEC, "10");
    HSPACE_ATTR_ALLOC(manueaver_seeker, "RANGE", CHSATTR_NOEXEC, "2");
    HSPACE_ATTR_ALLOC(manueaver_seeker, "XYDRIFT", CHSATTR_NOEXEC, "1");
    HSPACE_ATTR_ALLOC(manueaver_seeker, "ZDRIFT", CHSATTR_NOEXEC, "1");
    HSPACE_ATTR_ALLOC(manueaver_seeker, "INTERCEPT", CHSATTR_NOEXEC, "0");
}

// Navigation AI
// Find waypoints and handle noncombat activity
HSPACE_AI(navigation_seeker)
{
    HS_FLOAT64 dist_to_target, dist_to_decel, dist_per_cycle;
    HS_INT32 xyhead;
    HS_INT32 zhead;
    CHSShip *cShip;
    CHSSysEngines *cEngines;
    CHSConsole *cConsole;
    HS_DBREF dbUser;
    HS_FLOAT64 x, y, z;
    HS_INT32 xydrift, zdrift;
    HS_FLOAT64 time_to_decel;
    HS_FLOAT64 range;
    HS_FLOAT64 speed, new_speed, warp_speed, max_speed;
    CHSAttr *cAtr;
    // allow the autopilot to use warp or jump to get to it's destination
    CHSJumpDrive *cJumpers;
    CHSWarpDrive *cWarp;

    // make sure we have an autopilot!
    if (!cAutoPilot)
        return;

    // retrieve the ship information
    cShip = (CHSShip *) cAutoPilot->GetOwnerObject();
    if (!cShip)
        return;

    // retrieve the user manning the console 
    cConsole = cShip->GetConsole(0);
    if (cConsole)
        dbUser = hsInterface.ConsoleUser(cConsole->m_objnum);
    else
        dbUser = HSNOTHING;

    // get the information about the engines
    cEngines = (CHSSysEngines *) cShip->GetSystems().GetSystem(HSS_ENGINES);
    // quit if we don't have engines
    if (!cEngines)
    {
        return;
    }

    // get the information about the warp drive
    cWarp = (CHSWarpDrive*) cShip->GetSystems().GetSystem(HSS_WARP_DRIVE);

    // get the information about the jump drive
    cJumpers = (CHSJumpDrive *) cShip->GetSystems().GetSystem(HSS_JUMP_DRIVE);


    cAtr = cAI->GetAtr("X");
    if (!cAtr)
        x = 0;
    else
    {
        cAtr->Execute(cAutoPilot->GetObj());
        x = cAtr->ToFloat64();
    }

    cAtr = cAI->GetAtr("Y");
    if (!cAtr)
        y = 0;
    else
    {
        cAtr->Execute(cAutoPilot->GetObj());
        y = cAtr->ToFloat64();
    }

    cAtr = cAI->GetAtr("Z");
    if (!cAtr)
        z = 0;
    else
    {
        cAtr->Execute(cAutoPilot->GetObj());
        z = cAtr->ToFloat64();
    }

    cAtr = cAI->GetAtr("RANGE");
    if (!cAtr)
        range = 2;
    else
    {
        cAtr->Execute(cAutoPilot->GetObj());
        range = cAtr->ToFloat64();
    }

    cAtr = cAI->GetAtr("XYDRIFT");
    if (!cAtr)
        xydrift = 1;
    else
    {
        cAtr->Execute(cAutoPilot->GetObj());
        xydrift = cAtr->ToInt32();
    }

    cAtr = cAI->GetAtr("ZDRIFT");
    if (!cAtr)
        zdrift = 1;
    else
    {
        cAtr->Execute(cAutoPilot->GetObj());
        zdrift = cAtr->ToInt32();
    }

    cAtr = cAI->GetAtr("MAXSPEED");
    if (!cAtr)
        max_speed = 0;
    else
    {
        cAtr->Execute(cAutoPilot->GetObj());
        max_speed = cAtr->ToFloat64();
    }

    // calculate distance to the target
    dist_to_target =
        Dist3D(cShip->GetX(), cShip->GetY(), cShip->GetZ(), x, y, z);

// if we have warp and it is engaged, get the speed from the warp engines
    if(NULL != cWarp && cWarp->GetEngaged())
    {
        speed = cWarp->CalculateSpeed();
        time_to_decel = cWarp->GetCurrentWarp() / cWarp->GetAcceleration();
    }
    else if (NULL != cJumpers && cJumpers->GetEngaged())
    {
        speed = cJumpers->GetJumpSpeedMultiplier() * cEngines->GetCurrentSpeed();
        // deceleration from hyperspace is instantaneous
        time_to_decel = 1;
    }
    else
    {
        speed = cEngines->GetCurrentSpeed();
        time_to_decel = speed / cEngines->GetAcceleration(false);
    }

    // dist_per_cycle speed is per/hour, so adjust
    dist_per_cycle = speed / 3600.0;

    // distance to decel
    dist_to_decel = (.5 * dist_per_cycle * time_to_decel);


    if (!cAutoPilot->IsWaiting())
    {
        // If we have arrived, set mode and trigger effects
        if ((dist_to_target < range) && (speed == 0))
        {
            cAutoPilot->SetMode(CHSAP_NOTHING);
            cShip->NotifyConsoles(hsInterface.
                                  HSPrintf("%s%s-%s Destination reached.",
                                           ANSI_HILITE, ANSI_GREEN,
                                           ANSI_NORMAL), MSG_GENERAL);
            cAtr = cAI->GetAtr("DESTINATION");
            if (cAtr)
                cAtr->Execute(cAutoPilot->GetObj());
            return;
        }
        // Else if our time to stop is within the threshold, stop
        else if ((dist_to_decel > (dist_to_target - range)) &&
                 (dist_to_decel < (dist_to_target + range)))
        {
            //if we are on warp, then disengage warp 
            if(NULL != cWarp && cWarp->GetEngaged())
            {
                cShip->EngageWarpDrive(dbUser, 0.0);
            }
            // if we are in hyperspace, then disengage jump drive
            if (NULL != cJumpers && cJumpers->GetEngaged())
            {
              cJumpers->SetEngaged(false);
            }
            // stop afterburners
            if (cEngines->GetAfterburning())
            {
                cShip->EngageAfterburn(dbUser, false);
            }
            // set speed to zero
            if (cEngines->GetDesiredSpeed() != 0)
            {
                cShip->SetVelocity(dbUser, 0);
            }
            return;
        }
        // Else if we are not in the threshold, but we will pass it on the
        // next cycle, start slowing down
        else if ((dist_to_decel + dist_per_cycle) >
                 (dist_to_target + range))
        {
            // drop out of warp speed as we get closer
            if(NULL != cWarp && cWarp->GetEngaged())
            {
               // if we are on high warp
               if(cWarp->GetCurrentWarp() > 1)
               {
                   // slow down at warp deceleration
                   warp_speed = cWarp->GetCurrentWarp() - cWarp->GetAcceleration();
                   if (warp_speed < 1)
                   {
                       warp_speed = 0.0;
                       cWarp->SetDesiredWarp(warp_speed);
                       cShip->EngageWarpDrive(dbUser, 0.0);
                   }
                   else
                   {
                       cWarp->SetDesiredWarp(warp_speed);
                   }
               }
               else
               {
                   // disengage warp at speeds less than warp 1
                   cShip->EngageWarpDrive(dbUser, 0.0);
               }
            }
            // drop out of hyperspace
            else if (NULL != cJumpers && cJumpers->GetEngaged())
            {
                // jump drive is on or off, no slowing down time
                cJumpers->SetEngaged(false);
            }
            else
            {
                if (cEngines->GetAfterburning())
                    cShip->EngageAfterburn(dbUser, false);
                cShip->SetVelocity(dbUser, 0);
            }
            return;
        }
    }

    // if we are out of range and not in fleeing mode set us as cruising
    if ((dist_to_target >= range) && !cAutoPilot->IsFleeing())
    {
        cAutoPilot->SetMode(CHSAP_CRUISING);
    }

    // unless we are in hyperspace, figure out any course correction
    if ( !(NULL != cJumpers && cJumpers->GetEngaged()))
    {
        // figure out our heading to the target co-ordinates
        xyhead = XYAngle(cShip->GetX(), cShip->GetY(), x, y);
        zhead = ZAngle(cShip->GetX(), cShip->GetY(), cShip->GetZ(), x, y, z);

        // if our current heading is outside the drift allowance, set the
        // new heading
        if ((abs(xyhead - cShip->GetDesiredXYHeading()) > xydrift)
            || (abs(zhead - cShip->GetDesiredZHeading()) > zdrift))
        {
            cShip->SetHeading(dbUser, xyhead, zhead);
        }
    }

    // if the ship is in cruise mode or it is fleeing
    if (cEngines && (cAutoPilot->IsCruising() || cAutoPilot->IsFleeing()))
    {
        // new speed for engines
        new_speed = cEngines->GetMaxVelocity();
        // adjust speed if builder has set the MAXSPEED attribute
        if (max_speed > 1)
            new_speed = max_speed;
        // set the new speed if needed
        if (cEngines->GetDesiredSpeed() != new_speed &&
            !cEngines->GetAfterburning())
        {
            cShip->SetVelocity(dbUser, (HS_INT32) new_speed);
        }
        // engage afterburners if ship is fleeing
        if (cAutoPilot->IsFleeing() && !cEngines->GetAfterburning() &&
            (speed == cEngines->GetDesiredSpeed()))
        {
            cShip->EngageAfterburn(dbUser, true);
        }
    }

/*
// this is for debugging the travel path for the autopilot
if (speed>0.01)
  cShip->NotifyConsoles(hsInterface.HSPrintf("speed %2.2f\n dist_to_target %2.2f\n dist_to_decel %2.2f\n time_to_decel %2.2f\n dist_per_cycle %2.2f",
 speed,
 dist_to_target,
 dist_to_decel,
 time_to_decel,
 dist_per_cycle),
 MSG_GENERAL);
*/

}

// Awareness AI
// Handles which of the hostile targets is the weapons target
HSPACE_AI(awareness_drone)
{
    CHSConsole *cConsole;
    HS_DBREF dbUser;
    CHSShip *cTarget;
    CHSAttr *cAtr;
    HS_FLOAT64 mercy, hullperc;
    CHSShip *cShip;
    HS_FLOAT64 sx, tx, sy, ty, sz, tz, dist;
    HS_UINT32 max_range;
    CHSWeaponArray *cWeaponArray;

    if (!cAutoPilot)
        return;

    cShip = (CHSShip *) cAutoPilot->GetOwnerObject();

    if (!cShip)
    {
        cAutoPilot->SetMode(CHSAP_NOTHING);
        return;
    }

    if (!cAutoPilot->GetFirstHostile())
    {
        if (cAutoPilot->IsFighting())
            cAutoPilot->SetMode(CHSAP_NOTHING);
        return;
    }

    cConsole = cShip->GetConsole(0);
    if (!cConsole)
    {
        cAutoPilot->SetMode(CHSAP_NOTHING);
        return;
    }

    cWeaponArray = cConsole->GetWeaponArray();
    if (!cWeaponArray)
    {
        cAutoPilot->SetMode(CHSAP_NOTHING);
        return;
    }

    max_range = cWeaponArray->GetMaxRange();

    cAtr = cAI->GetAtr("MERCYPERC");
    if (!cAtr)
        mercy = 0;
    else
    {
        cAtr->Execute(cAutoPilot->GetObj());
        mercy = cAtr->ToFloat64();
    }

    dbUser = hsInterface.ConsoleUser(cConsole->m_objnum);
    cTarget = (CHSShip *) cConsole->GetObjectLock();

    // Find out if we already have a target and if it is above the mercy threshold
    if (cTarget
        && ((100 * (cTarget->GetHullPoints() / cTarget->GetMaxHullPoints())) >
            mercy))
        return;

    // If we have a target and it's not above teh mercy threshold, unlock weapons
    if (cTarget)
        cConsole->UnlockWeapons(dbUser);

    sx = cShip->GetX();
    sy = cShip->GetY();
    sz = cShip->GetZ();

    // Cycle through hostiles and find the first target in range that is above mercy
    for (cTarget = cAutoPilot->GetFirstHostile(); cTarget;
         cTarget = cAutoPilot->GetNextHostile())
    {
        hullperc =
            100 * (cTarget->GetHullPoints() / cTarget->GetMaxHullPoints());

        if (hullperc < mercy)
            continue;
        tx = cTarget->GetX();
        tx = cTarget->GetY();
        tx = cTarget->GetZ();
        dist = Dist3D(sx, sy, sz, tx, ty, tz);
        if (dist > max_range)
            continue;

        break;
    }

    // We didn't find any good targets
    if (!cTarget)
    {
        cAutoPilot->SetMode(CHSAP_NOTHING);
        return;
    }

    // The target we found is already locked for some strange reason?
    if (cTarget == cConsole->GetObjectLock())
        return;

    // Lock weapons on the new target
    cConsole->LockTarget(cTarget);
    cAutoPilot->SetMode(CHSAP_FIGHTING);
    cShip->NotifyConsoles(hsInterface.
                          HSPrintf("%s%s-%s Engaging enemy craft!",
                                   ANSI_HILITE, ANSI_RED, ANSI_NORMAL),
                          MSG_GENERAL);
}

// Aggression AI
// Determines whether or not a given contact should be considered a threat
HSPACE_AI(aggression_anybody)
{
    SENSOR_CONTACT *cContact;
    CHSSysSensors *cSensors;
    CHSShip *cShip, *cTarget;
    CHSAttr *cAtr;
    int idx;

    if (!cAutoPilot)
        return;

    cShip = (CHSShip *) cAutoPilot->GetOwnerObject();

    if (!cShip)
        return;

    cSensors = (CHSSysSensors *) cShip->GetSystems().GetSystem(HSS_SENSORS);

    if (!cSensors || !cSensors->GetCurrentPower()
        || !cSensors->NumContacts(HST_SHIP))
        return;

    cAtr = cAI->GetAtr("ISHOSTILE");
    if (!cAtr)
        return;

    cAutoPilot->ClearHostiles();

    // Cycle through sensor contacts and mark hostiles
    for (idx = 0; idx < HS_MAX_CONTACTS; idx++)
    {
        cContact = cSensors->GetContact(idx);
        if (!cContact)
            continue;

        cTarget = (CHSShip *) cContact->m_obj;
        if (!cTarget)
            continue;

        if (cTarget->GetType() != HST_SHIP)
            continue;

        cAtr->SetEnv(0, hsInterface.HSPrintf("#%d", cTarget->GetDbref()));

        cAtr->Execute(cAutoPilot->GetObj());

        if (cAtr->ToBool())
            cAutoPilot->AddHostile(cTarget);
    }
}

// Cowardice AI
// Handles when the ship decides to give up, flee, or pull out the cheap tricks
HSPACE_AI(cowardice_fearless)
{
    CHSShip *cShip;
    CHSAttr *cAtr;
    HS_FLOAT64 hullperc;

    if (!cAutoPilot)
        return;

    cShip = (CHSShip *) cAutoPilot->GetOwnerObject();

    if (!cShip)
        return;

    cAtr = cAI->GetAtr("HULLPERC");
    if (!cAtr)
        return;

    cAtr->Execute(cAutoPilot->GetObj());
    hullperc =
        100.0 * ((HS_FLOAT64) cShip->GetHullPoints() /
                 (HS_FLOAT64) cShip->GetMaxHullPoints());
    if (hullperc < cAtr->ToFloat64())
        cAutoPilot->SetMode(CHSAP_FLEEING);
}

// Manueavering AI
// Handle how the ship moves during combat
HSPACE_AI(manueaver_seeker)
{
    HS_INT32 xyhead;
    HS_INT32 zhead;
    CHSShip *cShip, *cTarget;
    CHSSysEngines *cEngines;
    CHSConsole *cConsole;
    HS_DBREF dbUser;
    HS_INT32 xydrift, zdrift, intercept_mode;
    HS_FLOAT64 range, dist_to_target;
    HS_FLOAT64 speed, new_speed;
    CHSAttr *cAtr;

    if (!cAutoPilot)
        return;

    cShip = (CHSShip *) cAutoPilot->GetOwnerObject();

    if (!cShip)
    {
        cAutoPilot->SetMode(CHSAP_NOTHING);
        return;
    }

    if (!cAutoPilot->GetFirstHostile())
    {
        if (cAutoPilot->IsFighting())
            cAutoPilot->SetMode(CHSAP_NOTHING);
        return;
    }

    cConsole = cShip->GetConsole(0);
    if (!cConsole)
    {
        cAutoPilot->SetMode(CHSAP_NOTHING);
        return;
    }
    dbUser = hsInterface.ConsoleUser(cConsole->m_objnum);

    cEngines = (CHSSysEngines *) cShip->GetSystems().GetSystem(HSS_ENGINES);
    if (cEngines)
        speed = cEngines->GetCurrentSpeed();
    else
        speed = 0;

// Find targetted object
    cTarget = (CHSShip *) cConsole->GetObjectLock();
    if (!cTarget)
    {
        cAutoPilot->SetMode(CHSAP_NOTHING);
        return;
    }
    dist_to_target =
        Dist3D(cShip->GetX(), cShip->GetY(), cShip->GetZ(), cTarget->GetX(),
               cTarget->GetY(), cTarget->GetZ());

// Get intercept_mode - best cone, course matching, best intercept
    cAtr = cAI->GetAtr("INTERCEPT");
    if (!cAtr)
    {
        intercept_mode = 0;
    }
    else
    {
        cAtr->Execute(cAutoPilot->GetObj());
        intercept_mode = cAtr->ToInt32();
        if (intercept_mode > 2)
            intercept_mode = 2;
        else if (intercept_mode < 0)
            intercept_mode = 0;
    }

    cAtr = cAI->GetAtr("XYDRIFT");
    if (!cAtr)
        xydrift = 1;
    else
    {
        cAtr->Execute(cAutoPilot->GetObj());
        xydrift = cAtr->ToInt32();
    }

    cAtr = cAI->GetAtr("ZDRIFT");
    if (!cAtr)
        zdrift = 1;
    else
    {
        cAtr->Execute(cAutoPilot->GetObj());
        zdrift = cAtr->ToInt32();
    }

    cAtr = cAI->GetAtr("RANGE");
    if (!cAtr)
        range = 2;
    else
    {
        cAtr->Execute(cAutoPilot->GetObj());
        range = cAtr->ToFloat64();
    }

// Best cone
// Calculate the lead_range based on the time it will take to turn and the
//   distance the ship and target will travel
    if (intercept_mode == 0)
    {
        xyhead =
            XYAngle(cShip->GetX(), cShip->GetY(), cTarget->GetX(),
                    cTarget->GetY());
        zhead =
            ZAngle(cShip->GetX(), cShip->GetY(), cShip->GetZ(),
                   cTarget->GetX(), cTarget->GetY(), cTarget->GetZ());
        if ((abs(xyhead - cShip->GetXYHeading()) > 135
             || abs(zhead - cShip->GetZHeading()) > 65)
            && dist_to_target < range)
        {
            xyhead = cShip->GetXYHeading();
            zhead = cShip->GetZHeading();
        }
        new_speed = cEngines->GetMaxVelocity() / 2;
    }
// Course matching
// Calculate course matching with lead_range (usually negative number) to
//   find the best track to the target's aft

// Best intercept
// Calculate the lead_range based on arriving at the same point in space
//   at the same time

// Change course to intercept
    if ((abs(xyhead - cShip->GetDesiredXYHeading()) > xydrift)
        || (abs(zhead - cShip->GetDesiredZHeading()) > zdrift))
    {
        cShip->SetHeading(dbUser, xyhead, zhead);
    }

// Set intercept velocity
    if (!cEngines)
        return;

    if (new_speed > cEngines->GetMaxVelocity())
        new_speed = cEngines->GetMaxVelocity();

    if (cEngines->GetDesiredSpeed() != new_speed
        && !cEngines->GetAfterburning())
    {
        cShip->SetVelocity(dbUser, (HS_INT32) new_speed);
    }
}

// Ordnance AI
// Decides what to fire and when
HSPACE_AI(ordnance_fireall)
{
    CHSShip *cShip;
    CHSConsole *cConsole;
    HS_DBREF dbUser;
    CHSWeaponArray *cWeaponArray;
    CHSShip *cTarget;
    CHSWeapon *cWeapon;

    if (!cAutoPilot)
        return;

    cShip = (CHSShip *) cAutoPilot->GetOwnerObject();

    if (!cShip)
    {
        cAutoPilot->SetMode(CHSAP_NOTHING);
        return;
    }

    if (!cAutoPilot->GetFirstHostile())
    {
        if (cAutoPilot->IsFighting())
            cAutoPilot->SetMode(CHSAP_NOTHING);
        return;
    }

    cConsole = cShip->GetConsole(0);
    if (!cConsole)
    {
        cAutoPilot->SetMode(CHSAP_NOTHING);
        return;
    }
    dbUser = hsInterface.ConsoleUser(cConsole->m_objnum);

    cWeaponArray = cConsole->GetWeaponArray();
    if (!cWeaponArray)
    {
        cAutoPilot->SetMode(CHSAP_NOTHING);
        return;
    }

    cTarget = (CHSShip *) cConsole->GetObjectLock();
    if (!cTarget)
    {
        cAutoPilot->SetMode(CHSAP_NOTHING);
        return;
    }

    if (!cConsole->IsInCone(cTarget))
        return;

    for (cWeapon = cWeaponArray->GetFirstWeapon(); cWeapon;
         cWeapon = cWeaponArray->GetNextWeapon())
    {
        if (!cWeapon->IsReady())
            continue;
        cShip->NotifyConsoles("KABOOM!!!!", MSG_GENERAL);
        cWeapon->AttackObject(cShip, cTarget, cConsole, HSS_NOTYPE, 0);
        if (cConsole->GetAutoload() && cWeapon->Loadable())
            cWeapon->Reload();
    }
// Determine gunnery type
}
