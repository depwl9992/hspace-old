// -----------------------------------------------------------------------
//! $Id: hsshipnav.cpp,v 1.29 2007/01/26 01:45:37 worsel Exp $
// -----------------------------------------------------------------------
#include "pch.h"

#include <stdlib.h>
#include <ctype.h>              // needed for isdigit()
#include <cstring>
#include "hsobjects.h"
#include "hscelestial.h"
#include "hsinterface.h"
#include "hsutils.h"
#include "hsconf.h"
#include "hsengines.h"
#include "hsjumpdrive.h"
#include "hswarpdrive.h"
#include "hsdb.h"
#include "hspace.h"
#include "hsansi.h"
#include "hsuniverse.h"
#include "hsterritory.h"
#include "hsclass.h"
#include "hssensors.h"
#include "hsthrusters.h"
#include "hscomputer.h"
#include "hsshields.h"
#include "hsreactor.h"
#include "hslifesupport.h"
#include "hscloaking.h"
#include "hsautopilot.h"

extern double d2cos_table[];
extern double d2sin_table[];

// Allows a player to set a desired velocity for the ship.
void CHSShip::SetVelocity(HS_DBREF player, int iVel)
{
    CHSSysEngines *cEngines;
    int iMaxVelocity;
    double dCurrentSpeed;
    char tbuf[256];

    // Check to see if we're dropping or lifting off
    if (m_drop_status)
    {
        hsStdError(player, "Unable to set speed during surface exchange.");
        return;
    }

    if (m_dock_status)
    {
        hsStdError(player, "Unable to set speed while docking.");
        return;
    }

    // Find the engines
    cEngines = (CHSSysEngines *) m_systems.GetSystem(HSS_ENGINES);
    if (!cEngines)
    {
        hsStdError(player, "This vessel has no engines.");
        return;
    }

    if (!cEngines->GetCurrentPower())
    {
        hsStdError(player, HSCONF.engines_offline);
        return;
    }

    if (cEngines->GetDamageLevel() == DMG_INOPERABLE)
    {
        hsStdError(player, "The engines have been destroyed!");
        return;
    }

    if (m_docked)
    {
        hsStdError(player, HSCONF.ship_is_docked);
        return;
    }

    if (m_dock_status)
    {
        hsStdError(player, HSCONF.ship_is_docking);
        return;
    }

    // Is the ship afterburning?
    if (cEngines->GetAfterburning())
    {
        hsStdError(player,
                   "Cannot set speed while afterburners are engaged.");
        return;
    }
    iMaxVelocity = cEngines->GetMaxVelocity();
    if ((iVel > iMaxVelocity) || (iVel < (-1 * (iMaxVelocity / 2.0))))
    {
        sprintf_s(tbuf,
                "Specified velocity must range from %d to %d.",
                (int) (-1 * (iMaxVelocity / 2.0)), iMaxVelocity);
        hsStdError(player, tbuf);
        return;
    }

    if (cEngines->SetDesiredSpeed(iVel))
    {
        sprintf_s(tbuf,
                "Desired velocity now set to %d %cph.", iVel,
                HSCONF.unit_name[0]);
        hsStdError(player, tbuf);
    }
    else
    {
        hsStdError(player, "Failed to set desired velocity.");
        return;
    }
    dCurrentSpeed = cEngines->GetCurrentSpeed();

    // if the new velocity is not 0 and the ship has boardlinks established,
    // disconnect them
    if (iVel != 0)
    {
        unsigned int idx = 0;
        CHSHatch *cHatch;
        for (idx = 0; idx < m_hatches.size(); idx++)
        {
            cHatch = GetHatch(idx);
            if (cHatch != NULL)
            {
                // break connection, idx must be 1 greater due to the
                // mechanism in DoBreakBoardLink
                if (cHatch->TargetObject() != HSNOTHING)
                    DoBreakBoardLink(player, idx + 1);
            }
        }
    }

    /*
     * Give some effects messages
     */
    if ((iVel > 0) && (dCurrentSpeed < 0))
    {
        if (hsInterface.AtrGet(m_objnum, "HSMSG_ENGINE_FORWARD"))
        {
            char *msg = hsInterface.EvalExpression(hsInterface.m_buffer,
                                                   m_objnum, m_objnum,
                                                   m_objnum);
            NotifySrooms(msg);
        }
        else
        {
            NotifySrooms(HSCONF.engine_forward);
        }
    }
    else if ((iVel < 0) && (dCurrentSpeed > 0))
    {
        if (hsInterface.AtrGet(m_objnum, "HSMSG_ENGINE_REVERSE"))
        {
            char *msg = hsInterface.EvalExpression(hsInterface.m_buffer,
                                                   m_objnum, m_objnum,
                                                   m_objnum);
            NotifySrooms(msg);
        }
        else
        {
            NotifySrooms(HSCONF.engine_reverse);
        }
    }
    else if (iVel > dCurrentSpeed)
    {
        if (hsInterface.AtrGet(m_objnum, "HSMSG_SPEED_INCREASE"))
        {
            char *msg = hsInterface.EvalExpression(hsInterface.m_buffer,
                                                   m_objnum, m_objnum,
                                                   m_objnum);
            NotifySrooms(msg);
        }
        else
        {
            NotifySrooms(HSCONF.speed_increase);
        }
    }
    else
    {
        if (hsInterface.AtrGet(m_objnum, "HSMSG_SPEED_DECREASE"))
        {
            char *msg = hsInterface.EvalExpression(hsInterface.m_buffer,
                                                   m_objnum, m_objnum,
                                                   m_objnum);
            NotifySrooms(msg);
        }
        else
        {
            NotifySrooms(HSCONF.speed_decrease);
        }
    }

}

// Sets the heading of the ship to a desired XY and Z angle.
void CHSShip::SetHeading(HS_DBREF player, int iXYAngle, int iZAngle)
{
    CHSSysThrusters *cThrust;

    if (m_hyperspace)
    {
        hsStdError(player, HSCONF.ship_is_jumping);
        return;
    }

    if (m_drop_status)
    {
        hsStdError(player,
                   "Unable to change course during surface exchange.");
        return;
    }

    if (m_dock_status)
    {
        hsStdError(player, "Unable to change course while docking.");
        return;
    }

    // Find the thrusters system
    cThrust = (CHSSysThrusters *) m_systems.GetSystem(HSS_THRUSTERS);
    if (!cThrust)
    {
        hsStdError(player, "This vessel has no maneuvering thrusters.");
        return;
    }

    if (!cThrust->GetCurrentPower())
    {
        hsStdError(player, "Thrusters are not currently online.");
        return;
    }

    if (cThrust->GetDamageLevel() == DMG_INOPERABLE)
    {
        hsStdError(player, "The maneuvering thrusters have been destroyed!");
        return;
    }

    if ((iXYAngle < 0) || (iXYAngle > 359))
    {
        hsInterface.Notify(player, "Valid XY headings range from 0 - 359.");
        return;
    }

    if ((iZAngle < -90) || (iZAngle > 90))
    {
        hsInterface.Notify(player, "Valid Z headings range from -90 - 90.");
        return;
    }

    m_desired_xyheading = iXYAngle;
    m_desired_zheading = iZAngle;

    char tbuf[256];

    sprintf_s(tbuf, "Course heading changed to %d mark %d.", iXYAngle, iZAngle);
    hsStdError(player, tbuf);

    //  increase_training(player, tship, .01);

}

void CHSShip::SetRoll(HS_DBREF player, int iRoll)
{
    CHSSysThrusters *cThrust;

    if (m_hyperspace)
    {
        hsStdError(player, HSCONF.ship_is_jumping);
        return;
    }

    if (m_drop_status)
    {
        hsStdError(player,
                   "Unable to change course during surface exchange.");
        return;
    }

    if (m_dock_status)
    {
        hsStdError(player, "Unable to change course while docking.");
        return;
    }

    // Find the thrusters system
    cThrust = (CHSSysThrusters *) m_systems.GetSystem(HSS_THRUSTERS);
    if (!cThrust)
    {
        hsStdError(player, "This vessel has no maneuvering thrusters.");
        return;
    }

    if (cThrust->GetDamageLevel() == DMG_INOPERABLE)
    {
        hsStdError(player, "The maneuvering thrusters have been destroyed!");
        return;
    }

    if (!cThrust->GetCurrentPower())
    {
        hsStdError(player, "Thrusters are not currently online.");
        return;
    }

    if (iRoll == HSNOTHING)
        iRoll = m_desired_roll;

    if (iRoll < 0 || iRoll > 359)
    {
        hsStdError(player, "Valid rolls reach from 0 to 359 degrees.");
        return;
    }

    m_desired_roll = iRoll;
    hsStdError(player,
               hsInterface.HSPrintf("Ship roll now set to %i degrees.",
                                    iRoll));

}


// Cyclicly changes the heading of the ship.
void CHSShip::ChangeHeading(void)
{
    CHSSysThrusters *cThrust;
    int iTurnRate;
    int iZChange;
    int iXYChange;
    int iDiff;
    HS_BOOL8 bChanged;

    cThrust = (CHSSysThrusters *) m_systems.GetSystem(HSS_THRUSTERS);
    if (!cThrust || !cThrust->GetCurrentPower())
        return;

    iTurnRate = cThrust->GetRate();

    bChanged = false;

    iXYChange = iZChange = 0;
    // Check the zheading first because this will affect the
    // trig for the XY plane.
    if (m_current_zheading != m_desired_zheading)
    {
        bChanged = true;
        if (m_desired_zheading > m_current_zheading)
        {
            if ((m_desired_zheading - m_current_zheading) > iTurnRate)
            {
                m_current_zheading += iTurnRate;
                iZChange = iTurnRate;
            }
            else
            {
                iZChange = m_desired_zheading - m_current_zheading;
                m_current_zheading = m_desired_zheading;
            }
        }
        else
        {
            if ((m_current_zheading - m_desired_zheading) > iTurnRate)
            {
                m_current_zheading -= iTurnRate;
                iZChange = -iTurnRate;
            }
            else
            {
                iZChange = m_desired_zheading - m_current_zheading;
                m_current_zheading = m_desired_zheading;
            }
        }
    }

    // Now handle any changes in the XY plane.
    if (m_desired_xyheading != m_current_xyheading)
    {
        bChanged = true;
        if (abs(m_current_xyheading - m_desired_xyheading) < 180)
        {
            if (abs(m_current_xyheading - m_desired_xyheading) < iTurnRate)
            {
                iXYChange = m_desired_xyheading - m_current_xyheading;
                m_current_xyheading = m_desired_xyheading;
            }
            else if (m_current_xyheading > m_desired_xyheading)
            {
                iXYChange = -iTurnRate;
                m_current_xyheading -= iTurnRate;
            }
            else
            {
                iXYChange = iTurnRate;
                m_current_xyheading += iTurnRate;
            }
        }
        else if (((360 - m_desired_xyheading) + m_current_xyheading) < 180)
        {
            iDiff = (360 - m_desired_xyheading) + m_current_xyheading;
            if (iDiff < iTurnRate)
            {
                iXYChange = -iDiff;
                m_current_xyheading = m_desired_xyheading;
            }
            else
            {
                iXYChange = -iTurnRate;
                m_current_xyheading -= iTurnRate;
            }
        }
        else if (((360 - m_current_xyheading) + m_desired_xyheading) < 180)
        {
            iDiff = (360 - m_current_xyheading) + m_desired_xyheading;
            if (iDiff < iTurnRate)
            {
                iXYChange = iDiff;
                m_current_xyheading = m_desired_xyheading;
            }
            else
            {
                iXYChange = iTurnRate;
                m_current_xyheading += iTurnRate;
            }
        }
        else                    // This should never be true, but just in case.
        {
            iXYChange = iTurnRate;
            m_current_xyheading += iTurnRate;
        }

        // Check to make sure angles are 0-359
        if (m_current_xyheading > 359)
            m_current_xyheading -= 360;
        else if (m_current_xyheading < 0)
            m_current_xyheading += 360;
    }

    // Now handle any changes in the XY plane.
    if (m_desired_roll != m_current_roll)
    {
        if (abs(m_current_roll - m_desired_roll) < 180)
        {
            if (abs(m_current_roll - m_desired_roll) < iTurnRate)
            {
                iXYChange = m_desired_roll - m_current_roll;
                m_current_roll = m_desired_roll;
            }
            else if (m_current_roll > m_desired_roll)
            {
                iXYChange = -iTurnRate;
                m_current_roll -= iTurnRate;
            }
            else
            {
                iXYChange = iTurnRate;
                m_current_roll += iTurnRate;
            }
        }
        else if (((360 - m_desired_roll) + m_current_roll) < 180)
        {
            iDiff = (360 - m_desired_roll) + m_current_roll;
            if (iDiff < iTurnRate)
            {
                iXYChange = -iDiff;
                m_current_roll = m_desired_roll;
            }
            else
            {
                iXYChange = -iTurnRate;
                m_current_roll -= iTurnRate;
            }
        }
        else if (((360 - m_current_roll) + m_desired_roll) < 180)
        {
            iDiff = (360 - m_current_roll) + m_desired_roll;
            if (iDiff < iTurnRate)
            {
                iXYChange = iDiff;
                m_current_roll = m_desired_roll;
            }
            else
            {
                iXYChange = iTurnRate;
                m_current_roll += iTurnRate;
            }
        }
        else                    // This should never be true, but just in case.
        {
            iXYChange = iTurnRate;
            m_current_roll += iTurnRate;
        }

        // Check to make sure angles are 0-359
        if (m_current_roll > 359)
            m_current_roll -= 360;
        else if (m_current_roll < 0)
            m_current_roll += 360;
    }


    // If nothing was changed, do nothing, else update stuff.
    if (bChanged)
    {
        // Run through any consoles, changing their headings
        int idx;
        for (idx = 0; idx < MAX_SHIP_CONSOLES; idx++)
        {
            if (m_console_array[idx])
                m_console_array[idx]->AdjustHeading(iXYChange, iZChange);
        }

        // Recompute heading vector.  Necessary for any heading change.
        RecomputeVectors();
    }
}

// Returns the current speed of the ship.
int CHSShip::GetSpeed(void)
{
    CHSSysEngines *cEngines;
    CHSWarpDrive* cWarp;

    // Find the engines
    cEngines = (CHSSysEngines *) m_systems.GetSystem(HSS_ENGINES);
    if (!cEngines)
        return 0;
    else
    {
        int speed;
        speed = (int) cEngines->GetCurrentSpeed();

        if((cWarp = (CHSWarpDrive*) m_systems.GetSystem(HSS_WARP_DRIVE)))
        {
            speed = (int) cWarp->CalculateSpeed();
        }
        return speed;
    }
}

// Returns the XY heading of the ship.
HS_UINT32 CHSShip::GetXYHeading(void)
{
    return m_current_xyheading;
}

HS_UINT32 CHSShip::GetDesiredXYHeading(void)
{
    return m_desired_xyheading;
}

// Returns the XY heading of the ship.
HS_UINT32 CHSShip::GetRoll(void)
{
    return m_current_roll;
}

// Returns the current Z heading of the ship
HS_INT32 CHSShip::GetZHeading(void)
{
    return m_current_zheading;
}

HS_INT32 CHSShip::GetDesiredZHeading(void)
{
    return m_desired_zheading;
}

// Moves the ship according to engine settings, heading vectors,
// all that good stuff.
void CHSShip::Travel(void)
{
    double speed;
    double dCurrentSpeed;
    CHSSysEngines *cEngines;

    // Find the engines
    cEngines = (CHSSysEngines *) m_systems.GetSystem(HSS_ENGINES);
    if (!cEngines)
        dCurrentSpeed = 0;
    else
        dCurrentSpeed = cEngines->GetCurrentSpeed();
    /*
     * Speed is measured in Hetramere per hour
     */
    if (dCurrentSpeed || m_warpengaged)
    {
        double oldx, oldy, oldz;        // New coords after movement.

        // Save current coords in case we need them.
        oldx = m_x;
        oldy = m_y;
        oldz = m_z;

        // Bring speed down to the unit per second level
        // The .0002778 is actually 1/3600.0 precomputed
        // to save time.
        speed = (dCurrentSpeed) * (.0002778 * HSCONF.cyc_interval);

        /*
         * If the ship is jumping, everything is accelerated
         */
        if (m_hyperspace)
        {
            CHSJumpDrive *cJumpers = (CHSJumpDrive *)
                m_systems.GetSystem(HSS_JUMP_DRIVE);

            if (cJumpers)
            {
                speed *= cJumpers->GetJumpSpeedMultiplier();
            }
            else
            {
                speed *= HSCONF.jump_speed_multiplier;
            }
        }

        if(true == m_warpengaged)
        {
            CHSWarpDrive* cWarp = (CHSWarpDrive*)
                m_systems.GetSystem(HSS_WARP_DRIVE);

            HS_FLOAT32 warp_speed = cWarp->CalculateSpeed() *
                (.0002778 * HSCONF.cyc_interval);

            // If warp speed is less than the current sublight speed,
            // use the latter until the warp drive catches up
            if(warp_speed > speed)
            {
                speed = warp_speed;
            }
        }

        // Add to the current position .. heading vector * speed.
        m_x += m_motion_vector.i() * speed;
        m_y += m_motion_vector.j() * speed;
        m_z += m_motion_vector.k() * speed;

        // See if we've moved into a new territory.
        CHSTerritory *newterritory;
        newterritory = taTerritories.InTerritory(this);

        HS_DBREF oldterr, newterr;

        // Check to see if we've moved in or out of a territory
        if (!m_territory)
            oldterr = HSNOTHING;
        else
            oldterr = m_territory->GetDbref();

        if (newterritory)
            newterr = newterritory->GetDbref();
        else
            newterr = HSNOTHING;

        if (oldterr != newterr)
        {
            // We've crossed borders.

            // If entering a new territory, only give that
            // message.  Else, give a leave message for the
            // old one.
            char *s;
            if (newterr && hsInterface.ValidObject(newterr))
            {
                if (hsInterface.AtrGet(newterr, "ENTER"))
                {
                    s = hsInterface.EvalExpression(hsInterface.m_buffer,
                                                   newterr,
                                                   m_objnum, m_objnum);
                    NotifyConsoles(s, MSG_GENERAL);
                }
                hsInterface.InvokeResponse(m_objnum, newterr, NULL, NULL,
                                           "AENTER");
            }

            if (hsInterface.ValidObject(oldterr))
            {
                if (hsInterface.AtrGet(oldterr, "LEAVE"))
                {
                    s = hsInterface.EvalExpression(hsInterface.m_buffer,
                                                   oldterr,
                                                   m_objnum, m_objnum);
                    NotifyConsoles(s, MSG_GENERAL);
                }
                hsInterface.InvokeResponse(m_objnum, oldterr,
                                           NULL, NULL, "ALEAVE");
            }

        }

        // Set our new territory
        m_territory = newterritory;
    }
}

// Handles recomputing the current heading vectors.  Handy function.
void CHSShip::RecomputeVectors(void)
{
    int zhead;
    int xyhead;

    zhead = m_current_zheading;
    if (zhead < 0)
        zhead += 360;

    xyhead = m_current_xyheading;
    CHSVector tvec(d2sin_table[xyhead] * d2cos_table[zhead],
                   d2cos_table[xyhead] * d2cos_table[zhead],
                   d2sin_table[zhead]);

    m_motion_vector = tvec;
}

// Gives the big, navigation status report for the vessel.
void CHSShip::GiveNavigationStatus(HS_DBREF player)
{
    CHSSysComputer *cComputer;
    CHSSysSensors *cSensors;
    CHSSysShield *cShield;
    CHSSysEngines *cEngines;

    // Lines that will get printed in the map portion
    char mLine1[128];
    char mLine2[128];
    char mLine3[128];
    char mLine4[128];
    char mLine5[128];
    char mLine6[128];
    char mLine7[128];
    char mLine8[128];

    char charcolors[64][64];    // Used to store object chars and colors

    char tbuf[256];

    int insX, insY;             // Where to insert into the map
    int idx;
    int tdx;
    char *ptr;

    // Find a variety of systems we'll need.
    cComputer = (CHSSysComputer *) m_systems.GetSystem(HSS_COMPUTER);
    if (!cComputer)
    {
        hsStdError(player, "This vessel has no internal computer.");
        return;
    }

    cSensors = (CHSSysSensors *) m_systems.GetSystem(HSS_SENSORS);
    cEngines = (CHSSysEngines *) m_systems.GetSystem(HSS_ENGINES);

    charcolors[0][0] = '\0';

    // We have to spend the majority of this function
    // just shoving characters into the map, so that's what we'll
    // start with.
    memset(mLine1, '\0', 128);
    memset(mLine2, '\0', 128);
    for (idx = 0; idx < 32; idx++)
        mLine1[idx] = ' ';
    for (idx = 0; idx < 34; idx++)
        mLine2[idx] = ' ';
    for (idx = 0; idx < 35; idx++)
    {
        mLine3[idx] = ' ';
        mLine4[idx] = ' ';
        mLine5[idx] = ' ';
        mLine6[idx] = ' ';
        mLine7[idx] = ' ';
    }
    mLine3[idx] = '\0';
    mLine4[idx] = '\0';
    mLine5[idx] = '\0';
    mLine6[idx] = '\0';
    mLine7[idx] = '\0';
    for (idx = 0; idx < 34; idx++)
        mLine8[idx] = ' ';
    mLine8[idx] = '\0';

    // Ok, now we need to run through sensor contacts and
    // insert them into the map.  Each object has a certain
    // character and color, and we need to map it's real world
    // coordinates into the map.  The map range specifies the
    // scale of the map, so if the object maps to a location
    // outside of the scale, it's excluded.
    if (cSensors)
    {
        SENSOR_CONTACT *cContact;
        CHS3DObject *cObj;
        double dX, dY, dZ;
        char filler;

        for (idx = 0; idx < HS_MAX_CONTACTS; idx++)
        {
            cContact = cSensors->GetContact(idx);
            if (!cContact)
                continue;

            cObj = cContact->m_obj;

            // Grab it's coordinates
            dX = cObj->GetX();
            dY = cObj->GetY();
            dZ = cObj->GetZ();

            // Check to see if it's within our map range
            if ((m_x - dX) > m_map_range ||
                (m_x - dX) < -m_map_range ||
                (m_y - dY) > m_map_range || (m_y - dY) < -m_map_range)
                continue;

            filler = cObj->GetObjectCharacter();

            // See if we need to store color and character
            // info for this object type.
            for (tdx = 0; charcolors[tdx][0]; tdx++)
            {
                if (charcolors[tdx][0] == filler)
                    break;
            }

            if (!charcolors[tdx][0])
            {
                // Store it
                ptr = charcolors[tdx];

                charcolors[tdx][0] = filler;

                charcolors[tdx][1] = '\0';
                strcat_s(charcolors[tdx], cObj->GetObjectColor());
                charcolors[tdx + 1][0] = '\0';
            }

            // Determine where to put the object in the ASCII map.
            insX = (int) ((((dX - m_x) / (m_map_range * 2)) * 34) + 17);
            insY = (int) ((((m_y - dY) / (m_map_range * 2)) * 8) + 4);

            switch (insY)
            {
            case 0:
                if (insX > 2 && insX < 32)
                    mLine1[insX] = filler;
                break;
            case 1:
                if (insX > 0 && insX < 34)
                    mLine2[insX] = filler;
                break;
            case 2:
                mLine3[insX] = filler;
                break;
            case 3:
                mLine4[insX] = filler;
                break;
            case 4:
                mLine5[insX] = filler;
                break;
            case 5:
                mLine6[insX] = filler;
                break;
            case 6:
                mLine7[insX] = filler;
                break;
            default:
                if (insX > 2 && insX < 34)
                    mLine8[insX] = filler;
                break;
            }
        }
    }


    // Now do HUD stuff

    // XY HUD first
    int x1, x2, x3;
    x2 = (int) ((m_current_xyheading / 10.0) * 10);
    x1 = x2 - 10;
    x3 = x2 + 10;
    if (x1 < 0)
        x1 += 360;
    else if (x1 > 359)
        x1 -= 360;
    if (x2 < 0)
        x2 += 360;
    else if (x2 > 359)
        x2 -= 360;
    if (x3 < 0)
        x3 += 360;
    else if (x3 > 359)
        x3 -= 360;

    // Now Z HUD stuff
    int z1, z2, z3;             // This are the little ticks on the heading bar
    z2 = (int) ((m_current_zheading / 10.0) * 10);
    z1 = z2 + 10;
    z3 = z2 - 10;

    // Now print the display.
    char tbuf2[256];
    sprintf_s(tbuf,
            "%s%s.---------------------------------------------------------------------------.%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    sprintf_s(tbuf2, "%s (%s)", GetName(), m_ident ? m_ident : "--");
    sprintf_s(tbuf, "%s%s|%s %-29s %s%s|%s %40s  %s%s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
            tbuf2,
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
            m_classinfo ? m_classinfo->ClassName() : "Unknown Class",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    sprintf_s(tbuf,
            "%s%s >---%sNavigation Status Report%s---%s+%s------------------------------------------<%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_WHITE, ANSI_BLUE,
            ANSI_WHITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    char tnum1[32];
    char tnum2[32];
    char tnum3[32];
    if (x1 < 10)
        sprintf_s(tnum1, " %d ", x1);
    else
        sprintf_s(tnum1, "%-3d", x1);
    if (x2 < 10)
        sprintf_s(tnum2, " %d ", x2);
    else
        sprintf_s(tnum2, "%3d", x2);
    if (x3 < 10)
        sprintf_s(tnum3, " %d ", x3);
    else
        sprintf_s(tnum3, "%3d", x3);
    sprintf_s(tbuf,
            "%s%s|%s                   %s        %s        %s     %s%s| %sX:%s %10.0f           %s%s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
            tnum1, tnum2, tnum3,
            ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
            m_x, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    sprintf_s(tbuf,
            "%s%s|%s %4d%s%s__             |____%s%s.%s_____|_____%s%s.%s____|      %s| %sY:%s %10.0f           %s%s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
            z1,
            ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
            ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
            ANSI_HILITE, ANSI_GREEN, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
            m_y, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    if (m_current_xyheading < 10)
        sprintf_s(tnum1, " %d ", m_current_xyheading);
    else
        sprintf_s(tnum1, "%3d", m_current_xyheading);
    sprintf_s(tbuf,
            "%s%s|       %s|%s                    > %s <              %s%s| %sZ:%s %10.0f           %s%s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
            tnum1,
            ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
            m_z, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    sprintf_s(tbuf,
            "%s%s|%s      %s-%s|          %s___________________________    |%s %s+%s- Course -%s%s+            %s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
            ANSI_HILITE, ANSI_GREEN, ANSI_BLUE, ANSI_NORMAL,
            ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    // Now we have to print out the map lines.  Phew.
    char tbuf3[256];

    // Line 1
    *tbuf2 = '\0';
    for (idx = 3; mLine1[idx]; idx++)
    {
        if (mLine1[idx] != ' ')
        {
            // Find the character type and color
            ptr = NULL;
            for (tdx = 0; charcolors[tdx][0]; tdx++)
            {
                if (charcolors[tdx][0] == mLine1[idx])
                {
                    ptr = charcolors[tdx];
                    ptr++;
                    break;
                }
            }
            if (ptr)
                sprintf_s(tbuf3, "%s%c%s", ptr, mLine1[idx], ANSI_NORMAL);
            else
                sprintf_s(tbuf3,
                        "%s%c%s", ANSI_HILITE, mLine1[idx], ANSI_NORMAL);
        }
        else
            sprintf_s(tbuf3, " ");
        strcat_s(tbuf2, tbuf3);
    }
    sprintf_s(tbuf,
            "%s%s|%s %4d%s%s-->%s %-3d    %s%s/%s%s%s%s\\  | %sC:%s %3d/%-3d  %s%sD:%s %3d/%-3d  %s%s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
            z2,
            ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
            m_current_zheading,
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
            tbuf2,
            ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
            m_current_xyheading, m_current_zheading,
            ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
            m_desired_xyheading, m_desired_zheading,
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);


    // Line 2
    *tbuf3 = '\0';
    for (idx = 1; mLine2[idx]; idx++)
    {
        if (mLine2[idx] != ' ')
        {
            // Find the character type and color
            ptr = NULL;
            for (tdx = 0; charcolors[tdx][0]; tdx++)
            {
                if (charcolors[tdx][0] == mLine2[idx])
                {
                    ptr = charcolors[tdx];
                    ptr++;
                    break;
                }
            }
            if (ptr)
                sprintf_s(tnum1, "%s%c%s", ptr, mLine2[idx], ANSI_NORMAL);
            else
                sprintf_s(tnum1,
                        "%s%c%s", ANSI_HILITE, mLine2[idx], ANSI_NORMAL);
        }
        else
            sprintf_s(tnum1, " ");
        strcat_s(tbuf3, tnum1);
    }
    if (cEngines && cEngines->CanBurn())
    {
        sprintf_s(tbuf2, "%.0f/%s%.0f%s (%.0f)",
                cEngines ? cEngines->GetCurrentSpeed() : 0.0,
                cEngines->GetAfterburning()? "*" : "",
                cEngines ? cEngines->GetDesiredSpeed() : 0.0,
                cEngines->GetAfterburning()? "*" : "",
                cEngines ? cEngines->GetMaxVelocity() : 0.0);
    }
    else
    {
        sprintf_s(tbuf2, "%.0f/%.0f (%.0f)",
                cEngines ? cEngines->GetCurrentSpeed() : 0.0,
                cEngines ? cEngines->GetDesiredSpeed() : 0.0,
                cEngines ? cEngines->GetMaxVelocity() : 0.0);

    }

    // change display if we are in warp
    if(m_warpengaged)
    {
        CHSWarpDrive* cWarp = (CHSWarpDrive*)
            m_systems.GetSystem(HSS_WARP_DRIVE);
        sprintf_s(tbuf2, "%.2e %s%sW:%s %.1f/%.1f  ",
                cWarp->CalculateSpeed(),
                ANSI_HILITE,ANSI_GREEN,ANSI_NORMAL,
                cWarp->GetCurrentWarp(),
                cWarp->GetDesiredWarp());

    }

    // change display if we are in hyperspace
    if(m_hyperspace)
    {
        float speed = cEngines ? cEngines->GetCurrentSpeed() : 0.0;

        CHSJumpDrive *cJumpers = (CHSJumpDrive *)
            m_systems.GetSystem(HSS_JUMP_DRIVE);

        // if this ship has it's own jump engine
        if (cJumpers)
        {
            sprintf_s(tbuf2, "%.2e %s%s-Hyperspace%s ",
            (speed *= cJumpers->GetJumpSpeedMultiplier()),
            ANSI_HILITE,ANSI_GREEN,ANSI_NORMAL
            );
        }
        else
        // ship used a jump beacon to get into hyperspace
        {
            sprintf_s(tbuf2, "%.2e %s%s[Gate]%s      ",
            (speed *= HSCONF.jump_speed_multiplier),
            ANSI_HILITE,ANSI_GREEN,ANSI_NORMAL
            );
        }

    }

    sprintf_s(tbuf,
            "%s%s|%s      %s-%s|%s      %s%s/%s%s%s%s\\| %sV:%s %-21s%s%s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
            ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
            tbuf3, ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
            tbuf2, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    // Line 3
    *tbuf3 = '\0';
    for (idx = 0; mLine3[idx]; idx++)
    {
        if (mLine3[idx] != ' ')
        {
            // Find the character type and color
            ptr = NULL;
            for (tdx = 0; charcolors[tdx][0]; tdx++)
            {
                if (charcolors[tdx][0] == mLine3[idx])
                {
                    ptr = charcolors[tdx];
                    ptr++;
                    break;
                }
            }
            if (ptr)
                sprintf_s(tnum1, "%s%c%s", ptr, mLine3[idx], ANSI_NORMAL);
            else
                sprintf_s(tnum1,
                        "%s%c%s", ANSI_HILITE, mLine3[idx], ANSI_NORMAL);
        }
        else
            sprintf_s(tnum1, " ");
        strcat_s(tbuf3, tnum1);
    }
    sprintf_s(tbuf,
            "%s%s|     %s__|     %s|%s%s%s%s|                         |%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_BLUE, ANSI_NORMAL,
            tbuf3, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    // Line 4
    *tbuf3 = '\0';
    for (idx = 0; mLine4[idx]; idx++)
    {
        if (mLine4[idx] != ' ')
        {
            // Find the character type and color
            ptr = NULL;
            for (tdx = 0; charcolors[tdx][0]; tdx++)
            {
                if (charcolors[tdx][0] == mLine4[idx])
                {
                    ptr = charcolors[tdx];
                    ptr++;
                    break;
                }
            }
            if (ptr)
                sprintf_s(tnum1, "%s%c%s", ptr, mLine4[idx], ANSI_NORMAL);
            else
                sprintf_s(tnum1,
                        "%s%c%s", ANSI_HILITE, mLine4[idx], ANSI_NORMAL);
        }
        else
            sprintf_s(tnum1, " ");
        strcat_s(tbuf3, tnum1);
    }

    cShield = (CHSSysShield *) m_systems.GetSystem(HSS_FORE_SHIELD);
    if (!cShield)
        strcpy_s(tnum1, "  * ");
    else
        sprintf_s(tnum1, "%.0f%%", cShield->GetShieldPerc());
    sprintf_s(tbuf,
            "%s%s|%s %4d        %s%s|%s%s%s%s| %sShields%s  %4s           %s%s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
            z3,
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
            tbuf3,
            ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
            tnum1, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    // Line 5
    *tbuf3 = '\0';
    for (idx = 0; mLine5[idx]; idx++)
    {
        if (idx == 17)
            sprintf_s(tnum1, "%s+%s", ANSI_HILITE, ANSI_NORMAL);
        else if (mLine5[idx] != ' ')
        {
            // Find the character type and color
            ptr = NULL;
            for (tdx = 0; charcolors[tdx][0]; tdx++)
            {
                if (charcolors[tdx][0] == mLine5[idx])
                {
                    ptr = charcolors[tdx];
                    ptr++;
                    break;
                }
            }
            if (ptr)
                sprintf_s(tnum1, "%s%c%s", ptr, mLine5[idx], ANSI_NORMAL);
            else
                sprintf_s(tnum1,
                        "%s%c%s", ANSI_HILITE, mLine5[idx], ANSI_NORMAL);
        }
        else
            sprintf_s(tnum1, " ");
        strcat_s(tbuf3, tnum1);
    }
    sprintf_s(tbuf,
            "%s%s|             |%s%s%s%s|%s            |            %s%s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, tbuf3, ANSI_HILITE,
            ANSI_BLUE, ANSI_NORMAL, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    // Line 6
    *tbuf3 = '\0';
    for (idx = 0; mLine6[idx]; idx++)
    {
        if (mLine6[idx] != ' ')
        {
            // Find the character type and color
            ptr = NULL;
            for (tdx = 0; charcolors[tdx][0]; tdx++)
            {
                if (charcolors[tdx][0] == mLine6[idx])
                {
                    ptr = charcolors[tdx];
                    ptr++;
                    break;
                }
            }
            if (ptr)
                sprintf_s(tnum1, "%s%c%s", ptr, mLine6[idx], ANSI_NORMAL);
            else
                sprintf_s(tnum1,
                        "%s%c%s", ANSI_HILITE, mLine6[idx], ANSI_NORMAL);
        }
        else
            sprintf_s(tnum1, " ");
        strcat_s(tbuf3, tnum1);
    }

    cShield = (CHSSysShield *) m_systems.GetSystem(HSS_PORT_SHIELD);
    if (cShield)
        sprintf_s(tnum1, "%.0f%%", cShield->GetShieldPerc());
    else
        strcpy_s(tnum1, "   *");

    cShield = (CHSSysShield *) m_systems.GetSystem(HSS_STARBOARD_SHIELD);
    if (cShield)
        sprintf_s(tnum2, "%.0f%%", cShield->GetShieldPerc());
    else
        strcpy_s(tnum2, "*   ");

    double perc;
    perc = GetMaxHullPoints();
    perc = 100 * (GetHullPoints() / perc);
    sprintf_s(tbuf,
            "%s%s| %sHP:%s %3.0f%%    %s%s|%s%s%s%s|%s      %4s -%s+%s- %-4s      %s%s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
            perc,
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
            tbuf3,
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
            tnum1, ANSI_HILITE, ANSI_NORMAL,
            tnum2, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    // Line 7
    *tbuf3 = '\0';
    for (idx = 0; mLine7[idx]; idx++)
    {
        if (mLine7[idx] != ' ')
        {
            // Find the character type and color
            ptr = NULL;
            for (tdx = 0; charcolors[tdx][0]; tdx++)
            {
                if (charcolors[tdx][0] == mLine7[idx])
                {
                    ptr = charcolors[tdx];
                    ptr++;
                    break;
                }
            }
            if (ptr)
                sprintf_s(tnum1, "%s%c%s", ptr, mLine7[idx], ANSI_NORMAL);
            else
                sprintf_s(tnum1,
                        "%s%c%s", ANSI_HILITE, mLine7[idx], ANSI_NORMAL);
        }
        else
            sprintf_s(tnum1, " ");
        strcat_s(tbuf3, tnum1);
    }

    sprintf_s(tbuf,
            "%s%s| %sMR:%s %-8d%s%s|%s%s%s%s|%s            |            %s%s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
            m_map_range,
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
            tbuf3,
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    // Finally, line 8.
    *tbuf3 = '\0';
    for (idx = 1; mLine8[idx]; idx++)
    {
        if (mLine8[idx] != ' ')
        {
            // Find the character type and color
            ptr = NULL;
            for (tdx = 0; charcolors[tdx][0]; tdx++)
            {
                if (charcolors[tdx][0] == mLine8[idx])
                {
                    ptr = charcolors[tdx];
                    ptr++;
                    break;
                }
            }
            if (ptr)
                sprintf_s(tnum1, "%s%c%s", ptr, mLine8[idx], ANSI_NORMAL);
            else
                sprintf_s(tnum1,
                        "%s%c%s", ANSI_HILITE, mLine8[idx], ANSI_NORMAL);
        }
        else
            sprintf_s(tnum1, " ");
        strcat_s(tbuf3, tnum1);
    }

    cShield = (CHSSysShield *) m_systems.GetSystem(HSS_AFT_SHIELD);
    if (!cShield)
        strcpy_s(tnum1, "  * ");
    else
        sprintf_s(tnum1, "%.0f%%", cShield->GetShieldPerc());
    sprintf_s(tbuf,
            "%s%s| %s             %s\\%s%s%s%s/%s           %4s           %s%s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_GREEN,
            ANSI_BLUE, ANSI_NORMAL,
            tbuf3,
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
            tnum1, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    sprintf_s(tbuf,
            "%s%s`---------------\\\\.___________________________.//--------------------------`%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);

    hsInterface.Notify(player, tbuf);
}

// Sets the map range of the ship to a given distance.
// This distance is like a scale.  It specifies, basically,
// a clipping region such that objects further than this
// distance are not included in the map.  At the same time,
// it maps objects within the clipping range to the map
// on the navigation display.
HS_BOOL8 CHSShip::SetMapRange(HS_DBREF player, int range)
{
    // Have to have a real range
    if (range <= 0)
        return false;

    // We can handle up to 8 digit ranges
    if (range > 99999999)
        return false;

    m_map_range = range;

    return true;
}

// Attempts to land the ship in another ship or on a
// celestial surface.
void CHSShip::LandVessel(HS_DBREF player, int id,
                         const HS_INT8 * pcLocation, char *lpstrCode)
{
    SENSOR_CONTACT *cContact;
    CHSSysSensors *cSensors;
    CHSSysEngines *cEngines;
    CHS3DObject *cObj;
    char tbuf[256];
    HS_TYPE tType;
    CHSLandingLoc *cLocation;

    // Perform some situational error checking
    if (m_drop_status)
    {
        hsStdError(player, "You may not do that during surface exchange.");
        return;
    }

    if (m_dock_status)
    {
        hsStdError(player,
                   "Ship is currently in docking/undocking procedures.");
        return;
    }

    // Find the sensors, so we can grab the contact
    cSensors = (CHSSysSensors *) m_systems.GetSystem(HSS_SENSORS);
    if (!cSensors)
    {
        hsStdError(player, "This vessel has no sensors.  Contact not found.");
        return;
    }

    // Find the sensor contact based on the given number.
    cContact = cSensors->GetContactByID(id);
    if (!cContact)
    {
        sprintf_s(tbuf, "%s%s[%s%4d%s%s]%s - No such contact on sensors.",
                ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
                id, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL);
        hsInterface.Notify(player, tbuf);
        return;
    }

    // Are our engines online?
    cEngines = (CHSSysEngines *) m_systems.GetSystem(HSS_ENGINES);
    if (!cEngines)
    {
        hsStdError(player, "This vessel has no engines.  Unable to land.");
        return;
    }

    if (cEngines->GetCurrentPower() == 0)
    {
        hsStdError(player, "Engines are currently offline.");
        return;
    }

    // Contact found, so let's see if it's an object
    // we can land on.  We support ships and planets.
    cObj = cContact->m_obj;
    tType = cObj->GetType();
    if (tType != HST_SHIP && tType != HST_PLANET)
    {
        sprintf_s(tbuf,
                "%s%s[%s%4d%s%s]%s - Cannot land on that type of object.",
                ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
                id, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL);
        hsInterface.Notify(player, tbuf);
        return;
    }

    // Do some ship to ship error checking.
    if (cObj->GetType() == HST_SHIP)
    {
        CHSShip *cShip;
        cShip = (CHSShip *) cObj;

        // Are we too big?
        if ((int) GetSize() > HSCONF.max_dock_size && !cShip->IsSpacedock())
        {
            hsStdError(player,
                       "Our vessel is too large to dock in another vessel.");
            return;
        }

        // Are they too small?
        if (GetSize() >= cObj->GetSize())
        {
            hsStdError(player, "That vessel is too small to accomodate us.");
            return;
        }

        // Is it within range?
        double dDist;

        dDist = Dist3D(m_x, m_y, m_z,
                       cObj->GetX(), cObj->GetY(), cObj->GetZ());
        if (dDist > HSCONF.max_dock_dist)
        {
            sprintf_s(tbuf,
                    "Location must be within %d %s to commence docking.",
                    HSCONF.max_dock_dist, HSCONF.unit_name);
            hsStdError(player, tbuf);
            return;
        }

        // Must be at a full stop to dock.
        if (cEngines && cEngines->GetCurrentSpeed())
        {
            hsStdError(player, "Must be at a full stop to commence docking.");
            return;
        }


        // Find the landing location

        if (!isdigit(*pcLocation))
        {
            // Find location by name.
            cLocation = cShip->FindLandingLocByName(pcLocation);
        }
        else
        {
            // The landing locations are specified by the player as
            // 1 .. n, but we store them in array notation 0..n-1.
            // Thus, subtract 1 from location when retrieving.
            cLocation = cShip->GetLandingLoc(atoi(pcLocation) - 1);
        }
    }
    else                        // Must be a planet
    {
        // Do we have drop rockets?
        if (!CanDrop())
        {
            hsStdError(player, "This vessel does not have drop capability.");
            return;
        }

        // Is it within range?
        double dDist;

        dDist = Dist3D(m_x, m_y, m_z,
                       cObj->GetX(), cObj->GetY(), cObj->GetZ());
        if (dDist > HSCONF.max_drop_dist)
        {
            sprintf_s(tbuf,
                    "Location must be within %d %s to commence landing.",
                    HSCONF.max_drop_dist, HSCONF.unit_name);
            hsStdError(player, tbuf);
            return;
        }

        // Are we too fast to land?
        if (cEngines && cEngines->GetCurrentSpeed() > HSCONF.max_land_speed)
        {
            sprintf_s(tbuf,
                    "Must be traveling at less than %d %s to commence landing.",
                    HSCONF.max_land_speed, HSCONF.unit_name);
            hsStdError(player, tbuf);
            return;
        }


        // Find the landing location
        CHSPlanet *cPlanet;
        cPlanet = (CHSPlanet *) cObj;

        if (!isdigit(*pcLocation))
        {
            cLocation = cPlanet->FindLandingLocByName(pcLocation);
        }
        else
        {
            cLocation = cPlanet->GetLandingLoc(atoi(pcLocation) - 1);
            // If the location exists, but is not visible and was accessed by
            // number, just report that it doesn't exist.
            if (cLocation && !cLocation->IsVisible())
            {
                hsStdError(player, "That landing location does not exist.");
                return;
            }
        }
    }

    // Location exists?
    if (!cLocation)
    {
        hsStdError(player, "That landing location does not exist.");
        return;
    }

    // See if the landing site is active.
    if (!cLocation->IsActive())
    {
        if (tType == HST_PLANET)
        {
            hsStdError(player,
                       "That landing site is currently not open for landing.");
            return;
        }
        else
        {
            hsStdError(player, "The bay doors to that bay are closed.");
            return;
        }
    }

    // See if we have code clearance?
    if (!cLocation->CodeClearance(lpstrCode))
    {
        hsStdError(player, "Invalid landing code -- permission denied.");
        return;
    }

    // See if the landing site can accomodate us.
    if (!cLocation->CanAccomodate(this))
    {
        hsStdError(player, "Landing site cannot accomodate this vessel.");
        return;
    }

    // Set desired and current speed to 0 to stop the ship.
    cEngines->SetDesiredSpeed(0);
    cEngines->SetAttributeValue("CURRENT SPEED", "0");

    // Ok.  Everything checks out, so begin landing procedures.
    InitLanding(player, cContact, cLocation);
}

// Initializes the landing procedures, be it for a planet or ship.
void CHSShip::InitLanding(HS_DBREF player, SENSOR_CONTACT * cContact,
                          CHSLandingLoc * cLocation)
{
    HS_TYPE tType;
    CHS3DObject *cObj;
    char tbuf[256];
    CHSUniverse *uDest;

    if (NULL == cContact)
    {
        hs_log("Invalid contact pointer passed to CHSShip::InitLanding().");
        return;
    }

    if (NULL == cLocation)
    {
        hs_log("Invalid location pointer passed to CHSShip::InitLanding()");
        return;
    }

    // Grab a few variables from the contact
    cObj = cContact->m_obj;
    tType = cObj->GetType();

    // Get our universe
    uDest = GetUniverse();

    // Now, what type of object is it?  We need to set some
    // variables.
    m_drop_status = 0;
    m_dock_status = 0;
    switch (tType)
    {
    case HST_SHIP:
        CHSShip * cShip;
        cShip = (CHSShip *) cObj;

        // Give some messages
        sprintf_s(tbuf,
                "%s[%s%s%d%s%s]%s - Docking request accepted .. beginning docking procedures.",
                cObj->GetObjectColor(), ANSI_NORMAL,
                ANSI_HILITE, cContact->m_id, ANSI_NORMAL,
                cObj->GetObjectColor(), ANSI_NORMAL);
        NotifyConsoles(tbuf, MSG_GENERAL);

        sprintf_s(tbuf,
                "The %s is beginning docking procedures at this location.",
                GetName());
        cShip->HandleMessage(tbuf, MSG_SENSOR, (long *) this);

        sprintf_s(tbuf,
                "In the distance, the %s begins docking with the %s.",
                GetName(), cShip->GetName());

        if (uDest)
            uDest->SendContactMessage(tbuf, IDENTIFIED, this);

        // Set the dock status to 8, which is the number
        // of seconds it takes to dock.  Undocking is a
        // negative number, so sign matters!
        m_dock_status = 8;
        m_docked = false;
        m_landing_target = cLocation;
        break;

    case HST_PLANET:
        // Give some messages
        sprintf_s(tbuf,
                "%s-%s Beginning descent to the surface of %s ...",
                cObj->GetObjectColor(), ANSI_NORMAL, cObj->GetName());
        NotifyConsoles(tbuf, MSG_GENERAL);

        // To Ship rooms
        if (hsInterface.AtrGet(m_objnum, "HSMSG_BEGIN_DESCENT"))
        {
            char *s = hsInterface.EvalExpression(hsInterface.m_buffer,
                                                 m_objnum, m_objnum,
                                                 m_objnum);
            hsInterface.NotifyContents(m_landing_target->Object(), s);
        }
        else
        {
            NotifySrooms(HSCONF.begin_descent);
        }

        sprintf_s(tbuf,
                "In the distance, the %s begins its descent toward the surface of %s.",
                GetName(), cObj->GetName());

        if (uDest)
            uDest->SendContactMessage(tbuf, IDENTIFIED, this);

        // Set the drop status to the positive number
        // of seconds it takes to drop.  This is specified
        // in the config file.
        m_drop_status = HSCONF.seconds_to_drop;
        m_docked = false;
        m_landing_target = cLocation;
        break;

    default:                   // Who knows!
        hsInterface.Notify(player,
                           "Help!  What type of HSpace object are you landing on?");
        return;
    }
}

// Handles landing the ship, be it to a planet or another ship.
void CHSShip::HandleLanding()
{
    CHSSysShield *cShield;
    CHSUniverse *uSource;
    char tbuf[256];

    // Determine which procedure we're in.
    if (m_dock_status > 0)
    {
        // We're landing in a ship.
        m_dock_status--;

        switch (m_dock_status)
        {
        case 6:                // Bay doors open
            NotifyConsoles("The enormous bay doors slowly begin to slide \
						open in front of the ship.\n", MSG_GENERAL);
            m_landing_target->HandleMessage("The enormous bay doors slowly \
						begin to slide open ...", MSG_GENERAL);
            break;

        case 3:                // Shields are dropped
            HS_BOOL8 bShields;
            HS_BOOL8 bShieldsUp;

            bShields = false;
            bShieldsUp = false;

            // If no shields present, go to docking
            cShield = (CHSSysShield *) m_systems.GetSystem(HSS_FORE_SHIELD);
            if (cShield)
            {
                if (cShield->GetCurrentPower() > 0)
                    bShieldsUp = true;
                cShield->SetCurrentPower(0);
                bShields = true;
            }
            cShield = (CHSSysShield *) m_systems.GetSystem(HSS_AFT_SHIELD);
            if (cShield)
            {
                if (cShield->GetCurrentPower() > 0)
                    bShieldsUp = true;
                cShield->SetCurrentPower(0);
                bShields = true;
            }
            cShield = (CHSSysShield *) m_systems.GetSystem(HSS_PORT_SHIELD);
            if (cShield)
            {
                if (cShield->GetCurrentPower() > 0)
                    bShieldsUp = true;
                cShield->SetCurrentPower(0);
                bShields = true;
            }
            cShield = (CHSSysShield *)
                m_systems.GetSystem(HSS_STARBOARD_SHIELD);
            if (cShield)
            {
                if (cShield->GetCurrentPower() > 0)
                    bShieldsUp = true;
                cShield->SetCurrentPower(0);
                bShields = true;
            }
            if (!bShields)
            {
                m_dock_status = 1;      // Skip quickly to docking
            }
            else
            {
                sprintf_s(tbuf, "%s%s-%s Shield check ...", ANSI_HILITE,
                        ANSI_GREEN, ANSI_NORMAL);
                NotifyConsoles(tbuf, MSG_GENERAL);
                if (bShieldsUp)
                {
                    sprintf_s(tbuf, "  %s%s-%s lowering shields ...",
                            ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL);
                    NotifyConsoles(tbuf, MSG_GENERAL);
                }
                else
                {
                    sprintf_s(tbuf, "  %s%s-%s shields down.", ANSI_HILITE,
                            ANSI_GREEN, ANSI_NORMAL);
                    NotifyConsoles(tbuf, MSG_GENERAL);
                }
            }
            break;

        case 0:                // Ship docks
            if (hsInterface.AtrGet(m_objnum, "HSMSG_DOCKING_DOCKS"))
            {
                char *s = hsInterface.EvalExpression(hsInterface.m_buffer,
                                                     m_objnum, m_objnum,
                                                     m_objnum);
                m_landing_target->HandleMessage(s, MSG_GENERAL);
            }
            else
            {
                sprintf_s(tbuf,
                        "Through the bay doors, the %s comes in and docks.",
                        GetName());
                m_landing_target->HandleMessage(tbuf, MSG_GENERAL);
            }

            sprintf_s(tbuf,
                    "The %s pushes forward as it glides in and docks.",
                    GetName());
            NotifyConsoles(tbuf, MSG_GENERAL);
            m_docked = true;
            m_in_space = false;
            MoveShipObject(m_landing_target->Object());

            // Set our location
            CHS3DObject *cOwner;
            cOwner = m_landing_target->GetOwnerObject();
            if (cOwner)
                m_objlocation = cOwner->GetDbref();
            else
                m_objlocation = HSNOTHING;

            // Remove us from active space
            uSource = GetUniverse();
            if (uSource)
                uSource->RemoveActiveObject(this);

            // Deduct the capacity from the landing loc
            m_landing_target->DeductCapacity(GetSize());
            break;
        }
    }
    else if (m_drop_status > 0)
    {
        int iHalfMarker;

        // We're dropping to a planet.
        m_drop_status--;

        iHalfMarker = (int) (HSCONF.seconds_to_drop / 2.0);

        // At the half way point, give another message
        if (m_drop_status == iHalfMarker)
        {
            CHSSysSensors *cSensors;

            sprintf_s(tbuf,
                    "%s%s-%s Surface contact in %d seconds ...",
                    ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, iHalfMarker);
            NotifyConsoles(tbuf, MSG_GENERAL);

            if (hsInterface.AtrGet(m_objnum, "HSMSG_LANDING_APPEARS"))
            {
                char *s = hsInterface.EvalExpression(hsInterface.m_buffer,
                                                     m_objnum, m_objnum,
                                                     m_objnum);
                m_landing_target->HandleMessage(s, MSG_GENERAL);

            }
            else
            {
                sprintf_s(tbuf,
                        "In the sky above, a d)ropship comes into view as it descends toward the surface.");
                m_landing_target->HandleMessage(tbuf, MSG_GENERAL);

            }

            // Clear the sensors if needed
            cSensors = (CHSSysSensors *) m_systems.GetSystem(HSS_SENSORS);
            if (cSensors)
                cSensors->ClearContacts();

            // Remove us from the active universe.  We're in
            // the atmosphere now.
            uSource = GetUniverse();
            if (uSource)
                uSource->RemoveActiveObject(this);

            // Indicate that we're no longer in space.  We're
            // somewhere between heaven and hell!
            m_in_space = false;
        }
        else if (m_drop_status == 0)
        {
            // To Ship rooms
            if (hsInterface.AtrGet(m_objnum, "HSMSG_LANDING_MSG"))
            {
                char *s = hsInterface.EvalExpression(hsInterface.m_buffer,
                                                     m_objnum, m_objnum,
                                                     m_objnum);
                hsInterface.NotifyContents(m_landing_target->Object(), s);
            }
            else
            {
                NotifySrooms(HSCONF.landing_msg);
            }

            if (hsInterface.AtrGet(m_objnum, "HSMSG_LANDING_LANDS"))
            {
                char *s = hsInterface.EvalExpression(hsInterface.m_buffer,
                                                     m_objnum, m_objnum,
                                                     m_objnum);
                m_landing_target->HandleMessage(s, MSG_GENERAL);

            }
            else
            {

                sprintf_s(tbuf,
                        "You take a step back as the %s fires its drop rockets and lands before you.",
                        GetName());
                m_landing_target->HandleMessage(tbuf, MSG_GENERAL);
            }
            MoveShipObject(m_landing_target->Object());
            m_docked = true;

            // Set our location
            CHS3DObject *cOwner;
            cOwner = m_landing_target->GetOwnerObject();
            if (cOwner)
                m_objlocation = cOwner->GetDbref();
            else
                m_objlocation = HSNOTHING;

            m_landing_target->DeductCapacity(GetSize());
        }
    }
}

// Allows a player to undock, or lift off, a ship that is
// docked or dropped.
void CHSShip::UndockVessel(HS_DBREF player)
{
    CHSSysEngines *cEngines;
    CHSLandingLoc *cLocation;
    CHSSysComputer *cComputer;
    CHS3DObject *cObj;
    HS_DBREF dbRoom;
    char tbuf[256];

    // Determine our status.
    if (!m_docked)
    {
        // We're not docked or dropped.
        hsStdError(player, "This vessel is not currently docked or landed.");
        return;
    }

    // Make sure we're not docking or dropping
    if (m_dock_status)
    {
        if (m_dock_status < 0)
            hsStdError(player, HSCONF.ship_is_undocking);
        else
            hsStdError(player, HSCONF.ship_is_docking);
        return;
    }
    if (m_drop_status)
    {
        if (m_drop_status > 0)
            hsStdError(player,
                       "Vessel is currently descending to the surface.");
        else
            hsStdError(player,
                       "Vessel is already lifting off from the surface.");
        return;
    }

    // Find the current landing location based on the
    // location of the ship object.
    if (m_objnum == HSNOTHING)
    {
        hsInterface.Notify(player,
                           "Help!  I can't find the ship object for your vessel.");
        return;
    }

    dbRoom = hsInterface.GetLocation(m_objnum);
    cLocation = dbHSDB.FindLandingLoc(dbRoom);
    if (!cLocation)
    {
        hsStdError(player, "You may not undock from this location.");
        return;
    }

    // Determine the owner of the landing location.
    cObj = cLocation->GetOwnerObject();
    if (!cObj)
    {
        hsInterface.Notify(player,
                           "Help!  I can't figure out which planet/ship this landing location belongs to.");
        return;
    }

    // Make sure engines and computer are online.
    cEngines = (CHSSysEngines *) m_systems.GetSystem(HSS_ENGINES);
    cComputer = (CHSSysComputer *) m_systems.GetSystem(HSS_COMPUTER);

    if (!cEngines || !cComputer)
    {
        hsStdError(player,
                   "This vessel is either missing the engines or computer needed to take off.");
        return;
    }

    if (!cEngines->GetCurrentPower() || !cComputer->GetCurrentPower())
    {
        hsStdError(player,
                   "Engines and computer systems must first be online.");
        return;
    }
    // We've got the location.
    // We've got the CHS3DObject that owns the location.
    // Figure out the type of object and undock/lift off.

    // See if the location we're docking on is a vessel and if so if bay is active.
    if (!cLocation->IsActive())
    {
        if (cObj->GetType() == HST_SHIP)
        {
            hsStdError(player, "The bay doors are closed.");
            return;
        }
    }

    if (cObj->GetType() == HST_SHIP)
    {
        CHSShip *cShip;

        cShip = (CHSShip *) cObj;

        sprintf_s(tbuf, "The %s is undocking from this location.", GetName());
        cShip->HandleMessage(tbuf, MSG_SENSOR, (long *) this);


        // Takes 15 seconds to undock, so set that variable.
        m_dock_status = -15;
        sprintf_s(tbuf,
                "%s%s-%s Undocking .. systems check initiating ...",
                ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL);
        NotifyConsoles(tbuf, MSG_GENERAL);
    }
    else if (cObj->GetType() == HST_PLANET)
    {
        // Set the lift off time plus 5 seconds systems check
        m_drop_status = -(HSCONF.seconds_to_drop) - 5;
        sprintf_s(tbuf,
                "%s%s-%s Commencing lift off procedures ...",
                ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL);
        NotifyConsoles(tbuf, MSG_GENERAL);
        // We don't really need 4 messages output to the location of
        // departure.  Skip this one.
        //
        //sprintf_s(tbuf,
        //      "Smoke begins to trickle from the lift rockets of the %s ...",
        //      GetName());
        //hsInterface.NotifyContents(dbRoom, tbuf);
    }
    else
    {
        hsInterface.Notify(player,
                           "What the ..?  This landing location isn't on a planet or ship.");
        return;
    }

    // We'll use the landing target variable to indicate
    // the location we're coming from.
    m_landing_target = cLocation;

}

// Handles the undocking/lift off continuation procedures such
// as systems checking, putting the ship into space, etc.
void CHSShip::HandleUndocking(void)
{
    char tbuf[256];
    double dVal;

    // Determine our status
    if (m_dock_status < 0)
    {
        m_dock_status++;

        switch (m_dock_status)
        {
        case -13:              // Hull check
            dVal = GetMaxHullPoints();
            dVal = 100 * (GetHullPoints() / dVal);
            sprintf_s(tbuf,
                    "  %s%s-%s Hull at %.0f%% integrity ...",
                    ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, dVal);
            NotifyConsoles(tbuf, MSG_GENERAL);
            break;

        case -10:              // Reactor check
            CHSReactor * cReactor;
            cReactor = (CHSReactor *) m_systems.GetSystem(HSS_REACTOR);
            if (!cReactor)
                sprintf_s(tbuf,
                        "  %s%s-%s Reactor not present .. ?",
                        ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL);
            else
            {
                dVal = cReactor->GetMaximumOutput(false);
                dVal = 100 * cReactor->GetOutput() / dVal;
                sprintf_s(tbuf,
                        "  %s%s-%s Reactor online at %.0f%% power ...",
                        ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, dVal);
            }
            NotifyConsoles(tbuf, MSG_GENERAL);
            break;

        case -7:               // Life support check
            CHSSysLifeSupport * cLife;

            cLife =
                (CHSSysLifeSupport *) m_systems.GetSystem(HSS_LIFE_SUPPORT);
            if (!cLife || !cLife->GetCurrentPower())
                sprintf_s(tbuf,
                        "  %s%s%s*%s %s%sWARNING%s %s%s%s*%s Life support systems are not online.",
                        ANSI_HILITE, ANSI_BLINK, ANSI_YELLOW, ANSI_NORMAL,
                        ANSI_HILITE, ANSI_YELLOW, ANSI_NORMAL,
                        ANSI_HILITE, ANSI_BLINK, ANSI_YELLOW, ANSI_NORMAL);
            else
                sprintf_s(tbuf,
                        "  %s%s-%s Life support systems -- online.",
                        ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL);
            NotifyConsoles(tbuf, MSG_GENERAL);
            break;

        case -4:               // Engine status report
            CHSSysEngines * cEngines;
            cEngines = (CHSSysEngines *) m_systems.GetSystem(HSS_ENGINES);
            if (!cEngines)
                sprintf_s(tbuf,
                        "  %s%s-%s Engines not present .. ?",
                        ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL);
            else
            {
                dVal = cEngines->GetOptimalPower(false);
                dVal = 100 * cEngines->GetCurrentPower() / dVal;
                sprintf_s(tbuf,
                        "  %s%s-%s Engines online at %.0f%% power ...",
                        ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, dVal);
            }
            NotifyConsoles(tbuf, MSG_GENERAL);
            break;

        case -2:               // Bay doors open
            sprintf_s(tbuf,
                    "\nThe bay doors begin to slide open as the %s prepares for departure ...\n",
                    GetName());
            NotifyConsoles(tbuf, MSG_GENERAL);
            if (hsInterface.AtrGet(m_objnum, "HSMSG_UNDOCK_START"))
            {
                char *s = hsInterface.EvalExpression(hsInterface.m_buffer,
                                                     m_objnum, m_objnum,
                                                     m_objnum);
                hsInterface.NotifyContents(m_landing_target->Object(), s);
            }
            else
            {
                sprintf_s(tbuf,
                        "The bay doors begin to slide open as the %s prerares for departure ...",
                        GetName());
                hsInterface.NotifyContents(m_landing_target->Object(), tbuf);
            }
            break;

        case 0:
            sprintf_s(tbuf,
                    "You feel a sudden lift as the %s glides forth from the docking bay.",
                    GetName());
            NotifySrooms(tbuf);
            if (hsInterface.AtrGet(m_objnum, "HSMSG_UNDOCK_FINAL"))
            {
                char *s = hsInterface.EvalExpression(hsInterface.m_buffer,
                                                     m_objnum, m_objnum,
                                                     m_objnum);
                hsInterface.NotifyContents(m_landing_target->Object(), s);
            }
            else
            {
                sprintf_s(tbuf,
                        "The %s fires its engines as it departs through the docking bay doors.",
                        GetName());
                hsInterface.NotifyContents(m_landing_target->Object(), tbuf);
            }

            // Find the object we're undocking from.
            CHS3DObject *cUndockingFrom;
            cUndockingFrom =
                dbHSDB.FindObjectByLandingLoc(m_landing_target->Object());
            CHSUniverse *uDest;
            if (cUndockingFrom)
            {
                uDest = cUndockingFrom->GetUniverse();
                m_x = cUndockingFrom->GetX();
                m_y = cUndockingFrom->GetY();
                m_z = cUndockingFrom->GetZ();
                sprintf_s(tbuf,
                        "In the distance, the %s undocks from the %s.",
                        GetName(), cUndockingFrom->GetName());
                if (uDest)
                    uDest->SendContactMessage(tbuf, IDENTIFIED,
                                              cUndockingFrom);
            }
            else
            {
                NotifyConsoles
                    ("Unable to find source ship.  Putting you where you docked at.",
                     MSG_GENERAL);
                uDest = GetUniverse();
            }
            // Add us to the univeres's active list.
            if (!uDest)
            {
                NotifyConsoles("Help!  I can't find your universe!",
                               MSG_GENERAL);
                return;
            }

            uDest->AddActiveObject(this);


            // We're in space now
            m_in_space = true;
            m_docked = false;

            // Move the ship object to the
            // universe room.
            MoveShipObject(uDest->GetID());
            m_uid = uDest->GetID();

            // Clear our location
            m_objlocation = HSNOTHING;

            // Add to the landing loc capacity
            int iSize;
            iSize = GetSize();
            m_landing_target->DeductCapacity(-iSize);
            break;
        }
    }
    else if (m_drop_status < 0)
    {
        m_drop_status++;

        int iLifeCheck;
        int iTestLifters;
        int iHalfWay;

        iHalfWay = (int) (-HSCONF.seconds_to_drop / 2.0);
        iLifeCheck = -HSCONF.seconds_to_drop - 4;
        iTestLifters = -HSCONF.seconds_to_drop - 2;
        if (m_drop_status == iLifeCheck)
        {
            CHSSysLifeSupport *cLife;

            // Check life support.
            cLife =
                (CHSSysLifeSupport *) m_systems.GetSystem(HSS_LIFE_SUPPORT);
            if (!cLife)
            {
                sprintf_s(tbuf,
                        "  %s%s%s*%s %s%sWARNING%s %s%s%s*%s Life support systems non-existant!",
                        ANSI_HILITE, ANSI_BLINK, ANSI_YELLOW, ANSI_NORMAL,
                        ANSI_HILITE, ANSI_YELLOW, ANSI_NORMAL,
                        ANSI_HILITE, ANSI_BLINK, ANSI_YELLOW, ANSI_NORMAL);
                NotifyConsoles(tbuf, MSG_GENERAL);
            }
            else if (!cLife->GetCurrentPower())
            {
                sprintf_s(tbuf,
                        "  %s%s%s*%s %s%sWARNING%s %s%s%s*%s Life support systems are not online.",
                        ANSI_HILITE, ANSI_BLINK, ANSI_YELLOW, ANSI_NORMAL,
                        ANSI_HILITE, ANSI_YELLOW, ANSI_NORMAL,
                        ANSI_HILITE, ANSI_BLINK, ANSI_YELLOW, ANSI_NORMAL);
                NotifyConsoles(tbuf, MSG_GENERAL);
            }
            else
            {
                sprintf_s(tbuf,
                        "  %s%s-%s Life support systems check - OK.",
                        ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL);
                NotifyConsoles(tbuf, MSG_GENERAL);
            }
        }
        else if (m_drop_status == iTestLifters)
        {
            // Test lift rockets.
            sprintf_s(tbuf,
                    "  %s%s-%s Testing lift rockets ...",
                    ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL);
            NotifyConsoles(tbuf, MSG_GENERAL);

            if (hsInterface.AtrGet(m_objnum, "HSMSG_TAKEOFF_START"))
            {
                char *s = hsInterface.EvalExpression(hsInterface.m_buffer,
                                                     m_objnum, m_objnum,
                                                     m_objnum);
                hsInterface.NotifyContents(m_landing_target->Object(), s);
            }
            else
            {
                sprintf_s(tbuf,
                        "Flames spurt intermittently from the lift rockets of the %s ...",
                        GetName());
                hsInterface.NotifyContents(m_landing_target->Object(), tbuf);
            }
        }
        else if (m_drop_status == -HSCONF.seconds_to_drop)
        {
            // LIFT OFF!
            sprintf_s(tbuf,
                    "%s%s-%s Lift off procedures complete .. %d seconds to orbit.",
                    ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
                    HSCONF.seconds_to_drop);
            NotifyConsoles(tbuf, MSG_GENERAL);

            // To Ship rooms
            if (hsInterface.AtrGet(m_objnum, "HSMSG_LIFT_OFF"))
            {
                char *s = hsInterface.EvalExpression(hsInterface.m_buffer,
                                                     m_objnum, m_objnum,
                                                     m_objnum);
                hsInterface.NotifyContents(m_landing_target->Object(), s);
            }
            else
            {
                NotifySrooms(HSCONF.lift_off);
            }

            if (hsInterface.AtrGet(m_objnum, "HSMSG_TAKEOFF_LIFTOFF"))
            {
                char *s = hsInterface.EvalExpression(hsInterface.m_buffer,
                                                     m_objnum, m_objnum,
                                                     m_objnum);
                hsInterface.NotifyContents(m_landing_target->Object(), s);
            }
            else
            {
                sprintf_s(tbuf,
                        "The wind suddenly picks up as the %s fires its lift rockets and begins its climb upward.",
                        GetName());
                hsInterface.NotifyContents(m_landing_target->Object(), tbuf);
            }

            // We're not dropped right now, but we're not in
            // space.
            m_docked = false;
            m_in_space = false;

            // Clear our location
            m_objlocation = HSNOTHING;

            // Move the ship object to the
            // universe room.
            MoveShipObject(m_uid);
        }
        else if (m_drop_status == iHalfWay)
        {
            sprintf_s(tbuf,
                    "%s%s-%s Orbit in %d seconds ...",
                    ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, -iHalfWay);
            NotifyConsoles(tbuf, MSG_GENERAL);
            if (hsInterface.AtrGet(m_objnum, "HSMSG_TAKEOFF_FINAL"))
            {
                char *s = hsInterface.EvalExpression(hsInterface.m_buffer,
                                                     m_objnum, m_objnum,
                                                     m_objnum);
                hsInterface.NotifyContents(m_landing_target->Object(), s);
            }
            else
            {
                sprintf_s(tbuf,
                        "The %s disappears in the sky above as it continues its climb into orbit.",
                        GetName());
                hsInterface.NotifyContents(m_landing_target->Object(), tbuf);
            }
        }
        else if (!m_drop_status)
        {
            int iSize;

            sprintf_s(tbuf,
                    "%s%s-%s Ship is now in orbit above the celestial surface.",
                    ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL);
            NotifyConsoles(tbuf, MSG_GENERAL);

            // Find the object we're taking off from and use the current
            // coordinates in case the "planet" has moved.
            CHS3DObject *cUndockingFrom;
            CHSUniverse *uDest;
            cUndockingFrom =
                dbHSDB.FindObjectByLandingLoc(m_landing_target->Object());
            if (cUndockingFrom)
            {
                uDest = cUndockingFrom->GetUniverse();
                m_x = cUndockingFrom->GetX();
                m_y = cUndockingFrom->GetY();
                m_z = cUndockingFrom->GetZ();
            }

            // If we don't have an object, use our universe
            if (!uDest)
            {
                uDest = GetUniverse();
            }

            if (!uDest)
            {
                NotifyConsoles("Help!  I can't find your universe!",
                               MSG_GENERAL);
                return;
            }
            uDest->AddActiveObject(this);
            sprintf_s(tbuf,
                    "In the distance, the %s undocks from %s.",
                    GetName(), m_landing_target->GetOwnerObject()->GetName());

            if (uDest)
                uDest->SendContactMessage(tbuf, IDENTIFIED,
                                          m_landing_target->GetOwnerObject());

            // We're in space now.
            m_in_space = true;


            // Add to the landing loc capacity
            iSize = GetSize();
            m_landing_target->DeductCapacity(-iSize);
        }
    }
}

// Returns the current location where the ship is docked/dropped,
// or HSNOTHING if not applicable.
HS_DBREF CHSShip::GetDockedLocation(void)
{
    if (m_objnum == HSNOTHING)
        return HSNOTHING;

    if (m_docked)
    {
        return hsInterface.GetLocation(m_objnum);
    }
    else
        return HSNOTHING;
}

// Attempts to engage or disengage the afterburners.
// Set bStat to true to engage, false to disengage.
void CHSShip::EngageAfterburn(HS_DBREF player, HS_BOOL8 bStat)
{
    CHSSysEngines *cEngines;

    // Look for the engines
    cEngines = (CHSSysEngines *) m_systems.GetSystem(HSS_ENGINES);
    if (!cEngines)
    {
        hsStdError(player, "This vessel has no engines.");
        return;
    }

    // Can't afterburn while not in space
    if (!m_in_space && bStat)
    {
        hsStdError(player,
                   "Unable to engage afterburners when not actively in space.");
        return;
    }

    // Can't afterburn when docking/dropping
    if (bStat && (m_drop_status || m_dock_status))
    {
        hsStdError(player, "Unable to engage afterburners at this time.");
        return;
    }

    // Can the engines burn?
    if (!cEngines->CanBurn())
    {
        hsStdError(player, "Our engines are not capable of afterburning.");
        return;
    }

    // Are engines online?
    if (!cEngines->GetCurrentPower())
    {
        hsStdError(player, "Engines are not currently online.");
        return;
    }

    // Are the engines burning already?
    if (bStat && cEngines->GetAfterburning())
    {
        hsStdError(player, "Afterburners already engaged.");
        return;
    }
    else if (!bStat && !cEngines->GetAfterburning())
    {
        hsStdError(player, "The afterburners are not currently engaged.");
        return;
    }

    // Find our universe to give some messages
    CHSUniverse *uDest;
    char tbuf[256];

    uDest = GetUniverse();

    // What change do we want to make?
    if (bStat)
    {
        // Engage.
        if (!cEngines->SetAfterburn(true))
            hsStdError(player, "Failed to engage afterburners.");
        else
        {
            sprintf_s(tbuf,
                    "%s%s-%s Afterburners engaged.",
                    ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL);
            NotifyConsoles(tbuf, MSG_ENGINEERING);

            if (hsInterface.AtrGet(m_objnum, "HSMSG_AFTERBURN_ENGAGE"))
            {
                char *msg = hsInterface.EvalExpression(hsInterface.m_buffer,
                                                       m_objnum, m_objnum,
                                                       m_objnum);
                NotifySrooms(msg);
            }
            else
            {
                NotifySrooms(HSCONF.afterburn_engage);
            }

            // Give some effects messages
            if (uDest)
            {
                sprintf_s(tbuf,
                        "Flames roar from the engines of the %s as it engages its afterburners.",
                        GetName());
                uDest->SendContactMessage(tbuf, IDENTIFIED, this);
            }
        }
    }
    else
    {
        // Disengage
        if (!cEngines->SetAfterburn(false))
            hsStdError(player, "Failed to disengage afterburners.");
        else
        {
            sprintf_s(tbuf,
                    "%s%s-%s Afterburners disengaged.",
                    ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL);
            NotifyConsoles(tbuf, MSG_ENGINEERING);

            if (hsInterface.AtrGet(m_objnum, "HSMSG_AFTERBURN_DISENGAGE"))
            {
                char *msg = hsInterface.EvalExpression(hsInterface.m_buffer,
                                                       m_objnum, m_objnum,
                                                       m_objnum);
                NotifySrooms(msg);
            }
            else
            {
                NotifySrooms(HSCONF.afterburn_disengage);
            }

            // Give some effects messages
            if (uDest)
            {
                sprintf_s(tbuf,
                        "The roaring flames from the engines of the %s cease as it disengages afterburners.",
                        GetName());
                uDest->SendContactMessage(tbuf, IDENTIFIED, this);
            }
        }
    }
}

void CHSShip::EngageWarpDrive(HS_DBREF player, HS_FLOAT32 level)
{
    CHSWarpDrive *cWarp;

    cWarp = (CHSWarpDrive*) m_systems.GetSystem(HSS_WARP_DRIVE);
    if(NULL == cWarp)
    {
        hsStdError(player, "This vessel has no warp drive.");
        return;
    }

    if(false == m_in_space)
    {
        hsStdError(player, "Ship must be in space to engage warp drive.");
        return;
    }

    if(true == m_dock_status)
    {
        hsStdError(player, "Cannot engage warp drive while docked!");
        return;
    }

    if(true == m_drop_status)
    {
        hsStdError(player, "Warp drive can only be used in space.");
        return;
    }

    if(0 == cWarp->GetCurrentPower())
    {
        hsStdError(player, "The warp drive is not online.");
        return;
    }


    cWarp->SetDesiredWarp(level);
    cWarp->DoCycle();

    if(level > 0.0001)
    {
        m_warpengaged = true;
    }
    else
    {
        m_warpengaged = false;
    }

}

// Attempts to engage or disengage the jump drive.
// Set bStat to true to engage, false to disengage.
void CHSShip::EngageJumpDrive(HS_DBREF player, HS_BOOL8 bStat)
{
    CHSJumpDrive *cJumpers;
    CHSSysEngines *cEngines;
    char tbuf[128];

    // Look for the jumpers
    cJumpers = (CHSJumpDrive *) m_systems.GetSystem(HSS_JUMP_DRIVE);
    if (!cJumpers)
    {
        hsStdError(player, "This vessel has no jump drive.");
        return;
    }

    // Can't jump while not in space
    if (!m_in_space && bStat)
    {
        hsStdError(player,
                   "Unable to engage jump drive when not actively in space.");
        return;
    }

    // Can't jump when docking/dropping
    if (bStat && (m_drop_status || m_dock_status))
    {
        hsStdError(player, "Unable to engage jump drive at this time.");
        return;
    }

    // Are jumpers online?
    if (!cJumpers->GetCurrentPower())
    {
        hsStdError(player, "Jump drive is not currently online.");
        return;
    }

    // Are the jumpers engaged already?
    if (bStat && cJumpers->GetEngaged())
    {
        hsStdError(player, "Jump drive already engaged.");
        return;
    }
    else if (!bStat && !cJumpers->GetEngaged())
    {
        hsStdError(player, "The jump drive is not currently engaged.");
        return;
    }

    // What change do we want to make?
    if (bStat)
    {
        // Make sure we're at a good speed.
        cEngines = (CHSSysEngines *) m_systems.GetSystem(HSS_ENGINES);
        if (!cEngines)
        {
            hsStdError(player, "This vessel doesn't even have engines.");
            return;
        }

        if (cEngines->GetCurrentSpeed() < cJumpers->GetMinJumpSpeed())
        {
            sprintf_s(tbuf,
                    "Minimum sublight speed of %d %s/hr required to engage jump drive.",
                    cJumpers->GetMinJumpSpeed(), HSCONF.unit_name);
            hsStdError(player, tbuf);
            return;
        }
        // Engage.
        if (!cJumpers->SetEngaged(true))
            hsStdError(player, "Failed to engage jump drive.");
        else
        {
            // Put us in hyperspace
            EnterHyperspace();
        }
    }
    else
    {
        // Disengage
        if (!cJumpers->SetEngaged(false))
            hsStdError(player, "Failed to disengage jump drive");
        else
        {
            ExitHyperspace();
        }
    }
}

// Handles putting the ship into hyperspace, including effects
// messages.
void CHSShip::EnterHyperspace(void)
{
    char tbuf[128];
    CHSUniverse *uSource;

    if (m_hyperspace)
        return;

    m_hyperspace = true;

    // Effects messages
    sprintf_s(tbuf,
            "%s%s-%s Jump drive engaged.",
            ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL);
    NotifyConsoles(tbuf, MSG_ENGINEERING);

    if (hsInterface.AtrGet(m_objnum, "HSMSG_SHIP_JUMPS"))
    {
        char *msg = hsInterface.EvalExpression(hsInterface.m_buffer,
                                               m_objnum, m_objnum, m_objnum);
        NotifySrooms(msg);
    }
    else
    {
        NotifySrooms(HSCONF.ship_jumps);
    }

    // Notify ships with us on sensors that
    // we've jumped.
    uSource = GetUniverse();
    if (uSource)
    {
        sprintf_s(tbuf,
                "You see a flash of blue light as the %s engages its jump drives.",
                GetName());
        uSource->SendContactMessage(tbuf, IDENTIFIED, this);
    }
}

// Handles taking the ship out of hyperspace, including effects
// messages.
void CHSShip::ExitHyperspace(void)
{
    CHSUniverse *uSource;
    char tbuf[128];

    if (!m_hyperspace)
        return;

    m_hyperspace = false;

    sprintf_s(tbuf,
            "%s%s-%s Jump drive disengaged.",
            ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL);
    NotifyConsoles(tbuf, MSG_ENGINEERING);

    if (hsInterface.AtrGet(m_objnum, "HSMSG_END_JUMP"))
    {
        char *msg = hsInterface.EvalExpression(hsInterface.m_buffer,
                                               m_objnum, m_objnum, m_objnum);
        NotifySrooms(msg);
    }
    else
    {
        NotifySrooms(HSCONF.end_jump);
    }

    // Notify ships with us on sensors that
    // we've come out of hyperspace.
    uSource = GetUniverse();
    if (uSource)
    {
        sprintf_s(tbuf,
                "You see a flash of blue light as a vessel comes out of hyperspace.");
        uSource->SendMessage(tbuf, 1000, this);
    }
}

// Returns true or false for whether the ship is in hyperspace
// or not.
HS_BOOL8 CHSShip::InHyperspace(void)
{
    return m_hyperspace;
}

// Allows a player to break a boarding link with a ship
// in the specified boarding link slot.
void CHSShip::DoBreakBoardLink(HS_DBREF player, int slot)
{
    // Decrement slot for array notation.
    slot--;

    CHSHatch *lHatch;

    lHatch = GetHatch(slot);
    // Link exists?
    if (!lHatch)
    {
        hsStdError(player, "No hatch with that notation found.");
        return;
    }
    int port;
    port = lHatch->TargetHatch();

    CHSShip *cShip;

    cShip = (CHSShip *) dbHSDB.FindObject(lHatch->TargetObject());

    if (!cShip)
    {
        hsStdError(player, "No vessel docked on that port.");
        return;
    }

    CHSHatch *cHatch;
    cHatch = cShip->GetHatch(port);
    if (!cHatch)
    {
        hsStdError(player, "No vessel docked on that port.");
        return;
    }

    if (lHatch->Clamped() == 1)
    {
        hsStdError(player, "Local hatch is clamped can't break link.");
        return;
    }

    if (cHatch->Clamped() == 1)
    {
        hsStdError(player, "Remote hatch is clamped can't break link.");
        return;
    }

    cHatch->TargetObject(HSNOTHING);
    cHatch->TargetHatch(HSNOTHING);
    lHatch->TargetObject(HSNOTHING);
    lHatch->TargetHatch(HSNOTHING);
    hsInterface.UnlinkExits(cHatch->Object(), lHatch->Object());
    cHatch->HandleMessage(hsInterface.HSPrintf("%s is disconnected.",
                                               hsInterface.GetName(cHatch->
                                                                   Object())),
                          MSG_GENERAL);
    lHatch->HandleMessage(hsInterface.
                          HSPrintf("%s is disconnected.",
                                   hsInterface.GetName(lHatch->Object())),
                          MSG_GENERAL);

    char tbuf[256];
    sprintf_s(tbuf, "The %s disengages docking couplings.", GetName());
    cShip->NotifyConsoles(tbuf, MSG_GENERAL);

    hsStdError(player, "Docking couplings disengaged.");
}

// Attempts to establish a boarding connection with
// another vessel.
void CHSShip::DoBoardLink(HS_DBREF player, int id, int lhatch, int dhatch)
{
    CHSJumpDrive *cJumpers;
    CHSWarpDrive *cWarp;
    CHSSysEngines *cEngines;
    CHSSysSensors *cSensors;
    SENSOR_CONTACT *cContact;
    CHS3DObject *cObj;
    CHSShip *cShip;
    char tbuf[128];


    // Look for the jumpers
    cJumpers = (CHSJumpDrive *) m_systems.GetSystem(HSS_JUMP_DRIVE);
    cWarp = (CHSWarpDrive *) m_systems.GetSystem(HSS_WARP_DRIVE);
    cEngines = (CHSSysEngines *) m_systems.GetSystem(HSS_ENGINES);
    cSensors = (CHSSysSensors *) m_systems.GetSystem(HSS_SENSORS);

    lhatch--;
    dhatch--;

    // Are they on sensors?
    cContact = NULL;
    if (cSensors)
    {
        cContact = cSensors->GetContactByID(id);
    }
    if (!cContact)
    {
        hsStdError(player, "No such contact id on sensors.");
        return;
    }
    cObj = cContact->m_obj;

    // Other object must be capable of board linking.
    if (cObj->GetType() != HST_SHIP)
    {
        hsStdError(player,
                   "You may not establish a boarding link with that.");
        return;
    }
    cShip = (CHSShip *) cObj;

    // Are the jumpers engaged?
    if (cJumpers && cJumpers->GetEngaged())
    {
        hsStdError(player,
                   "Cannot make a boarding link while in hyperspace.");
        return;
    }

    if(cWarp && cWarp->GetEngaged())
    {
        hsStdError(player, "Cannot make a boarding link with warp engaged.");
        return;
    }

    // Are the afterburners engaged?
    if (cEngines && cEngines->GetAfterburning())
    {
        hsStdError(player, "Cannot make a boarding link while afterburning.");
        return;
    }


    // See if the distance is ok
    if (Dist3D(GetX(), GetY(), GetZ(),
               cObj->GetX(), cObj->GetY(), cObj->GetZ()) >
        HSCONF.max_board_dist)
    {
        sprintf_s(tbuf,
                "Vessel must be within %d %s to establish boarding link.",
                HSCONF.max_board_dist, HSCONF.unit_name);
        hsStdError(player, tbuf);
        return;
    }

    CHSHatch *cHatch;
    cHatch = cShip->GetHatch(dhatch);
    if (!cHatch)
    {
        hsStdError(player, "No such hatch on target vessel.");
        return;
    }
    CHSHatch *lHatch;
    lHatch = GetHatch(lhatch);
    if (!lHatch)
    {
        hsStdError(player, "No such hatch on this vessel.");
        return;
    }
    if (cHatch->TargetObject() != HSNOTHING)
    {
        hsStdError(player, "Target hatch already taken.");
        return;
    }
    if (lHatch->TargetObject() != HSNOTHING)
    {
        hsStdError(player, "Local hatch already taken.");
        return;
    }
    if (lHatch->Clamped() == 1)
    {
        hsStdError(player, "Local hatch is clamped can't estabilish link.");
        return;
    }

    if (cHatch->Clamped() == 1)
    {
        hsStdError(player, "Remote hatch is clamped can't estabilish link.");
        return;
    }


    cHatch->TargetObject(GetDbref());
    lHatch->TargetObject(cShip->GetDbref());
    cHatch->TargetHatch(lhatch);
    lHatch->TargetHatch(dhatch);
    hsInterface.LinkExits(lHatch->Object(), cHatch->Object());
    sprintf_s(tbuf, "%s connects with %s's %s.",
            hsInterface.GetName(lHatch->Object()), cShip->GetName(),
            hsInterface.GetName(cHatch->Object()));
    lHatch->HandleMessage(tbuf, MSG_GENERAL);
    sprintf_s(tbuf, "%s connects with %s's %s.",
            hsInterface.GetName(cHatch->Object()),
            GetName(), hsInterface.GetName(lHatch->Object()));
    cHatch->HandleMessage(tbuf, MSG_GENERAL);
    sprintf_s(tbuf, "A loud clang is heard as docking couplings are engaged.");
    NotifySrooms(tbuf);
    cShip->NotifySrooms(tbuf);

    sprintf_s(tbuf,
            "The %s has engaged docking couplings on our hatch %d.",
            GetName(), lhatch);
    cShip->NotifyConsoles(tbuf, MSG_GENERAL);

    hsStdError(player, "Hatches connected.");
}

// Scans a target ID on sensors and gives the player a
// scan report.
void CHSShip::ScanObjectID(HS_DBREF player, int id)
{
    CHSSysSensors *cSensors;
    SENSOR_CONTACT *cContact;

    // Do we have sensors?
    cSensors = (CHSSysSensors *) m_systems.GetSystem(HSS_SENSORS);
    if (!cSensors)
    {
        hsStdError(player, "This vessel is not equipped with sensors.");
        return;
    }

    // Are sensors online?
    if (!cSensors->GetCurrentPower())
    {
        hsStdError(player, "Sensors are currently offline.");
        return;
    }

    // Find the contact.
    cContact = cSensors->GetContactByID(id);
    if (!cContact)
    {
        hsStdError(player, "No such contact ID on sensors.");
        return;
    }

    // Scan the object
    if (cContact->status == IDENTIFIED)
        cContact->m_obj->GiveScanReport(this, player, true);
    else
        cContact->m_obj->GiveScanReport(this, player, false);
}

// Displays the description of a target vessel.
void CHSShip::ViewObjectID(HS_DBREF player, int id)
{
    CHSSysSensors *cSensors;
    SENSOR_CONTACT *cContact;

    // Do we have sensors?
    cSensors = (CHSSysSensors *) m_systems.GetSystem(HSS_SENSORS);
    if (!cSensors)
    {
        hsStdError(player, "This vessel is not equipped with sensors.");
        return;
    }

    // Are sensors online?
    if (!cSensors->GetCurrentPower())
    {
        hsStdError(player, "Sensors are currently offline.");
        return;
    }

    // Find the contact.
    cContact = cSensors->GetContactByID(id);
    if (!cContact)
    {
        hsStdError(player, "No such contact ID on sensors.");
        return;
    }

    // Show the object
    if (cContact->status == IDENTIFIED)
        if (hsInterface.AtrGet(cContact->m_obj->GetDbref(), "DESCRIBE"))
            hsInterface.Notify(player,
                               hsInterface.HSPrintf("%s",
                                                    hsInterface.
                                                    EvalExpression
                                                    (hsInterface.m_buffer,
                                                     cContact->m_obj->
                                                     GetDbref(), m_objnum,
                                                     m_objnum)));
        else
            hsInterface.Notify(player, "No description set on that object.");
    else
    {
        hsStdError(player, "Contact outside visual range.");
        return;
    }
}

void CHSShip::GateObjectID(HS_DBREF player, int id)
{
    CHSSysSensors *cSensors;
    SENSOR_CONTACT *cContact;

    // Do we have sensors?
    cSensors = (CHSSysSensors *) m_systems.GetSystem(HSS_SENSORS);
    if (!cSensors)
    {
        hsStdError(player, "This vessel is not equipped with sensors.");
        return;
    }

    // Are sensors online?
    if (!cSensors->GetCurrentPower())
    {
        hsStdError(player, "Sensors are currently offline.");
        return;
    }

    // Find the contact.
    cContact = cSensors->GetContactByID(id);
    if (!cContact)
    {
        hsStdError(player, "No such contact ID on sensors.");
        return;
    }

    CHS3DObject *cObj;

    cObj = cContact->m_obj;

    if (Dist3D
        (GetX(), GetY(), GetZ(), cObj->GetX(), cObj->GetY(),
         cObj->GetZ()) > HSCONF.max_gate_dist)
    {
        hsStdError(player,
                   hsInterface.
                   HSPrintf("You must be within %d %s to gate an object.",
                            HSCONF.max_gate_dist, HSCONF.unit_name));
        return;
    }

    if (cObj->GetType() != HST_WORMHOLE)
    {
        hsStdError(player, "You cannot gate that type of object.");
        return;
    }

    if (cObj->GetType() == HST_WORMHOLE)
    {
        CHSWormHole *cWorm;

        cWorm = (CHSWormHole *) cObj;

        if (cWorm->GetSize() / 2 < GetSize())
        {
            hsStdError(player,
                       "This ship is too large to gate the wormhole.");
            return;
        }

        cWorm->GateShip(this);
    }
    else
    {
        hsStdError(player, "Error, cannot gate target.");
    }

}

// Attempts to engage or disengage the autopilot.
// bStat == true for engage, false for disengage.
void CHSShip::EngageAutoPilot(HS_DBREF player, HS_BOOL8 bStat)
{
    CHSSysAutoPilot *cAutoPilot;

    // Look for the auto pilot.
    cAutoPilot = (CHSSysAutoPilot *) m_systems.GetSystem(HSS_AUTOPILOT);
    if (!cAutoPilot)
    {
        hsStdError(player, "This vessel has no autopilot.");
        return;
    }

    // Is the autopilot already engaged?
    if (bStat && cAutoPilot->IsEngaged())
    {
        hsStdError(player, "Autopilot is already engaged.");
        return;
    }
    else if (!bStat && !cAutoPilot->IsEngaged())
    {
        hsStdError(player, "The autopilot is already disengaged.");
        return;
    }

    // What change do we want to make?
    if (bStat)
        cAutoPilot->SetEngaged(true);
    else
        cAutoPilot->SetEngaged(false);
}

// Attempts to engage or disengage the cloaking device.
// Set bStat to true to engage, false to disengage.
void CHSShip::EngageCloak(HS_DBREF player, HS_BOOL8 bStat)
{
    CHSSysCloak *cCloak;
    char tbuf[128];

    // Look for the cloaking device.
    cCloak = (CHSSysCloak *) m_systems.GetSystem(HSS_CLOAK);
    if (!cCloak)
    {
        hsStdError(player, "This vessel has no cloaking device.");
        return;
    }

    // Can't jump while not in space
    if (!m_in_space && bStat)
    {
        hsStdError(player,
                   "Unable to engage cloaking device when not actively in space.");
        return;
    }

    // Can't cloak when docking/dropping
    if (bStat && (m_drop_status || m_dock_status))
    {
        hsStdError(player, "Unable to engage cloaking device at this time.");
        return;
    }

    // Is the cloaking system engaged already?
    if ( (bStat && cCloak->GetEngaged()) || (bStat && cCloak->IsEngaging()))
    {
        hsStdError(player, "Cloaking Device already engaged.");
        return;
    }
    else if (false == bStat && 0 == cCloak->GetEngaged())
    {
        hsStdError(player, "The cloaking device is not currently engaged.");
        return;
    }

    // What change do we want to make?
    if (bStat)
    {
        CHSUniverse *uSource;
        cCloak->SetEngaged(true);

        sprintf_s(tbuf,
                "%s%s-%s Cloaking Device engaged.",
                ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL);
        NotifyConsoles(tbuf, MSG_ENGINEERING);
        // Notify ships with us on sensors that
        // we've decloaked.
        uSource = GetUniverse();
        if (uSource)
        {
            sprintf_s(tbuf,
                    "A vessel slowly shifts out of view as it engages its cloaking device.");
            uSource->SendMessage(tbuf, 1000, this);
        }
    }
    else
    {
        CHSUniverse *uSource;
        cCloak->SetEngaged(false);
        sprintf_s(tbuf,
                "%s%s-%s Cloaking Device disengaged.",
                ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL);
        NotifyConsoles(tbuf, MSG_ENGINEERING);
        // Notify ships with us on sensors that
        // we've decloaked.
        uSource = GetUniverse();
        if (uSource)
        {
            sprintf_s(tbuf,
                    "A vessel slowly shifts into view as it disengages its cloaking device.");
            uSource->SendMessage(tbuf, 1000, this);
        }
    }
}

void CHSShip::GiveHatchRep(HS_DBREF player)
{
    unsigned int idx;
    unsigned int hatches = 0;

    CHSHatch *cHatch;
    for (idx = 0; idx < m_hatches.size(); idx++)
    {
        cHatch = m_hatches[idx];

        if (cHatch)
            hatches++;
    }

    if (hatches < 1)
    {
        hsStdError(player, "This vessel has no hatches.");
        return;
    }


    char tbuf[512];
    // Give the header info
    sprintf_s(tbuf,
            "%s%s.----------------------------------------------------------.%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);
    sprintf_s(tbuf,
            "%s%s|%s Hatch Status Report      %30s  %s%s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, GetName(),
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);
    sprintf_s(tbuf,
            "%s%s >--------------------------------------------------------<%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);
    sprintf_s(tbuf,
            "%s%s|%s-%s#%s%s-       - %sShip Linked %s%s-                 - %sRemote Port %s%s- %s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL, ANSI_HILITE,
            ANSI_GREEN, ANSI_NORMAL, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
            ANSI_HILITE, ANSI_GREEN, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);


    char tbuf2[256], tbuf3[256];


    for (idx = 1; idx < hatches + 1; idx++)
    {
        cHatch = m_hatches[idx - 1];

        CHSShip *cShip;
        cShip = dbHSDB.FindShip(cHatch->TargetObject());
        if (cShip)
        {
            sprintf_s(tbuf2, "%s", cShip->GetName());
            sprintf_s(tbuf3, "%i", cHatch->TargetHatch() + 1);
        }
        else
        {
            sprintf_s(tbuf2, "Unconnected");
            sprintf_s(tbuf3, "Unconnected");
        }

        sprintf_s(tbuf, "%s%s|%s[%s%i%s%s]%s  %-36s  %11s    %s%s|%s",
                ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL, idx,
                ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, tbuf2, tbuf3,
                ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
        hsInterface.Notify(player, tbuf);
    }

    sprintf_s(tbuf,
            "%s%s`----------------------------------------------------------'%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

}
