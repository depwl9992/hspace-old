// -----------------------------------------------------------------------
// $Id: hstractor.cpp,v 1.10 2006/04/29 12:26:33 mark Exp $
// -----------------------------------------------------------------------

#include "pch.h"

#include "hsobjects.h"
#include "hsuniverse.h"
#include "hsinterface.h"
#include "hsutils.h"
#include "hsansi.h"
#include "hspace.h"
#include "hssensors.h"
#include "hsconf.h"
#include "hsengines.h"

#include "hstractor.h"

CHSSysTractor::CHSSysTractor(void)
{
    SetType(HSS_TRACTOR);
    m_strength = 0;
    m_lock = NULL;
    m_mode = HSTM_HOLD;
}

CHSSysTractor::~CHSSysTractor(void)
{
    if (m_strength)
        delete m_strength;
}

HS_BOOL8 CHSSysTractor::SetAttributeValue(const HS_INT8 * pcAttrName,
                                          const HS_INT8 * strValue)
{
    // Match the name .. set the value
    if (!strcasecmp(pcAttrName, "STRENGTH"))
    {
        if (!*strValue)
        {
            delete m_strength;
            m_strength = NULL;
        }
        else
        {
            if (!m_strength)
                m_strength = new float;

            *m_strength = atoi(strValue);
        }
        return true;
    }
    else if (!strcasecmp(pcAttrName, "MODE"))
    {
        if (!*strValue)
        {
            return false;
        }
        else
        {
            if (atoi(strValue) > 2 || atoi(strValue) < 0)
                return false;

            m_mode = (HS_TMODE) atoi(strValue);
        }
        return true;
    }
    else
    {
        return CHSEngSystem::SetAttributeValue(pcAttrName, strValue);
    }
}


void CHSSysTractor::GetAttributeList(CHSAttributeList & rlistAttrs)
{
    CHSEngSystem::GetAttributeList(rlistAttrs);

    rlistAttrs.push_back("STRENGTH");
    rlistAttrs.push_back("MODE");
}

HS_BOOL8
    CHSSysTractor::GetAttributeValue(const HS_INT8 * pcAttrName,
                                     CHSVariant & rvarValue,
                                     HS_BOOL8 bAdjusted, HS_BOOL8 bLocalOnly)
{
    // Determine attribute, and return the value.
    if (!strcasecmp(pcAttrName, "STRENGTH"))
    {
        if (m_strength)
        {
            rvarValue = *m_strength;
        }
        else if (!bLocalOnly)
        {
            rvarValue = GetStrength(bAdjusted);
        }
        else
        {
            return false;
        }
        return true;
    }
    else if (!strcasecmp(pcAttrName, "MODE"))
    {
        rvarValue = (HS_INT8) m_mode;
        return true;
    }
    else
    {
        return CHSEngSystem::GetAttributeValue(pcAttrName, rvarValue,
                                               bAdjusted, bLocalOnly);
    }
}

float CHSSysTractor::GetStrength(HS_BOOL8 bAdjusted)
{
    float rval;


    // Do we have a local value?
    if (!m_strength)
    {
        // Do we have a parent?
        if (GetParent())
        {
            CHSSysTractor *ptr;
            ptr = (CHSSysTractor *) GetParent();
            rval = ptr->GetStrength(false);
        }
        else
            return 0;           // default of 1000 (1k)
    }
    else
        rval = *m_strength;

    if (bAdjusted)
    {
        float fVal;
        int iOptPower;
        iOptPower = GetOptimalPower(false);
        fVal = (double) GetCurrentPower();
        if (iOptPower)
            fVal /= (float) iOptPower;
        else
            fVal = 1;
        rval *= fVal;
    }



    return rval;

}

void CHSSysTractor::SaveToFile(FILE * fp)
{
    // Save the base first
    CHSEngSystem::SaveToFile(fp);

    // Save our stuff
    fprintf(fp, "MODE=%d\n", m_mode);
    if (m_strength)
        fprintf(fp, "STRENGTH=%.4f\n", *m_strength);
}

void CHSSysTractor::ReleaseLock(HS_DBREF player)
{
    if (m_lock == NULL)
    {
        hsStdError(player, "The tractor beam is not locked on a target.");
        return;
    }

    CHSShip *cShip;
    CHS3DObject *tObj;
    tObj = m_lock;
    m_lock = NULL;

    if (!tObj)
        return;
    cShip = (CHSShip *) GetOwnerObject();
    if (!cShip)
        return;

    cShip->NotifyConsoles(hsInterface.
                          HSPrintf("%s%s-%s Tractor lock released.",
                                   ANSI_GREEN, ANSI_HILITE, ANSI_NORMAL),
                          MSG_GENERAL);

    tObj->HandleMessage(hsInterface.HSPrintf("%s has released tractor lock.",
                                             cShip->GetName()), MSG_GENERAL,
                        (long *) cShip);

}

void CHSSysTractor::DoCycle(void)
{
    CHSEngSystem::DoCycle();

    CHSShip *cShip;
    CHS3DObject *tObj;
    tObj = m_lock;
    if (!tObj)
        return;
    cShip = (CHSShip *) GetOwnerObject();
    if (!cShip)
        return;

    double cDist;

    cDist =
        Dist3D(tObj->GetX(), tObj->GetY(), tObj->GetZ(), cShip->GetX(),
               cShip->GetY(), cShip->GetZ());

    if (cDist > (GetEffect() * 10))
    {
        m_lock = NULL;
        cShip->NotifyConsoles(hsInterface.
                              HSPrintf
                              ("%s%s-%s Tractor lock target out of range, releasing lock.",
                               ANSI_GREEN, ANSI_HILITE, ANSI_NORMAL),
                              MSG_GENERAL);
        tObj->HandleMessage("Tractor lock has been released.", MSG_GENERAL,
                            (long *) cShip);
        return;
    }

    if (!cShip->IsActive() || cShip->IsDestroyed() || !tObj->IsActive() ||
        cShip->GetDockedLocation() != HSNOTHING)
    {
        m_lock = NULL;
        cShip->NotifyConsoles(hsInterface.
                              HSPrintf("%s%s-%s Tractor lock lost.",
                                       ANSI_GREEN, ANSI_HILITE, ANSI_NORMAL),
                              MSG_GENERAL);
        tObj->HandleMessage("Tractor lock has been released.", MSG_GENERAL,
                            (long *) cShip);
        return;
    }


    if (m_mode == HSTM_TRACTOR)
    {
        if (cDist > GetEffect())
        {
            tObj->
                MoveInDirection(XYAngle
                                (tObj->GetX(), tObj->GetY(), cShip->GetX(),
                                 cShip->GetY()), ZAngle(tObj->GetX(),
                                                        tObj->GetY(),
                                                        tObj->GetZ(),
                                                        cShip->GetX(),
                                                        cShip->GetY(),
                                                        cShip->GetZ()),
                                GetEffect());
        }
        else
        {
            tObj->SetX(cShip->GetX());
            tObj->SetY(cShip->GetY());
            tObj->SetZ(cShip->GetZ());
        }
    }
    else if (m_mode == HSTM_REPULSE)
    {
        tObj->
            MoveInDirection(XYAngle
                            (cShip->GetX(), cShip->GetY(), tObj->GetX(),
                             tObj->GetY()), ZAngle(cShip->GetX(),
                                                   cShip->GetY(),
                                                   cShip->GetZ(),
                                                   tObj->GetX(), tObj->GetY(),
                                                   tObj->GetZ()),
                            GetEffect());
    }
    else if (m_mode == HSTM_HOLD)
    {
        if (cShip->GetSpeed() == 0)
            return;

        float mvAmt;
        if ((cShip->GetSpeed() / 3600.00) < GetEffect())
            mvAmt = (float) (cShip->GetSpeed() / 3600.00);
        else
            mvAmt = GetEffect();

        tObj->MoveInDirection(cShip->GetXYHeading(), cShip->GetZHeading(),
                              mvAmt);
    }
    cDist =
        Dist3D(tObj->GetX(), tObj->GetY(), tObj->GetZ(), cShip->GetX(),
               cShip->GetY(), cShip->GetZ());

    if (cDist > (GetEffect() * 10))
    {
        m_lock = NULL;
        cShip->NotifyConsoles(hsInterface.
                              HSPrintf
                              ("%s%s-%s Tractor lock target out of range, releasing lock.",
                               ANSI_GREEN, ANSI_HILITE, ANSI_NORMAL),
                              MSG_GENERAL);
        tObj->HandleMessage("Tractor lock has been released.", MSG_GENERAL,
                            (long *) cShip);
        return;
    }

}

CHS3DObject *CHSSysTractor::GetLock(void)
{
    return m_lock;
}

float CHSSysTractor::GetEffect(void)
{
    float rval;

    if (!GetLock())
        return 0.00;

    CHS3DObject *tObj;

    tObj = GetLock();

    rval = (float) (GetStrength(true) / tObj->GetSize());

    return rval;
}

HS_TMODE CHSSysTractor::GetMode(void)
{
    return m_mode;
}

void CHSSysTractor::SetMode(int mode)
{
    CHSShip *cShip;
    cShip = (CHSShip *) GetOwnerObject();

    if (cShip)
    {
        char tbuf[128];
        if (mode == 0)
            sprintf(tbuf, "tractor");
        else if (mode == 1)
            sprintf(tbuf, "repulse");
        else if (mode == 2)
            sprintf(tbuf, "hold");
        cShip->NotifyConsoles(hsInterface.
                              HSPrintf("%s%s-%s Tractor beam mode set to %s.",
                                       ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
                                       tbuf), MSG_GENERAL);
    }

    m_mode = (HS_TMODE) mode;
}

void CHSSysTractor::SetLock(HS_DBREF player, int id)
{
    CHSSysSensors *cSensors;
    SENSOR_CONTACT *cContact;

    CHSShip *cShip;
    cShip = (CHSShip *) GetOwnerObject();

    if (!cShip)
        return;

    if (id == 0)
    {
        if (m_lock)
        {
            cShip->NotifyConsoles(hsInterface.
                                  HSPrintf("%s%s-%s Tractor beam unlocked.",
                                           ANSI_GREEN, ANSI_HILITE,
                                           ANSI_NORMAL), MSG_GENERAL);
            m_lock = NULL;
            return;
        }
        else
        {
            hsStdError(player, "Tractor beam is not locked.");
            return;
        }
    }

    // Do we have sensors?
    cSensors = (CHSSysSensors *) cShip->GetSystems().GetSystem(HSS_SENSORS);
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

    if (Dist3D(cShip->GetX(), cShip->GetY(), cShip->GetZ(),
               cObj->GetX(), cObj->GetY(), cObj->GetZ())
        > GetStrength(true) / cObj->GetSize() * 10.00)
    {
        hsStdError(player, "Target outside of tractor beam range.");
        return;
    }

    if (cObj->GetType() != HST_SHIP)
    {
        hsStdError(player, "You cannot tractor that type of object.");
        return;
    }

    m_lock = cObj;

    cShip->HandleMessage("Tractor beam locked onto target.", MSG_GENERAL,
                         (long *) cObj);
    cObj->HandleMessage(hsInterface.
                        HSPrintf("Tractor beam lock detected from the %s.",
                                 cShip->GetName()), MSG_GENERAL,
                        (long *) cShip);
    if (cObj->GetType() == HST_SHIP)
    {
        CHSShip *tShip;
        tShip = (CHSShip *) cObj;
        tShip->
            NotifySrooms
            ("The ship shakes lightly as something locks onto the hull.");
    }
}

void CHSSysTractor::DockShip(HS_DBREF player, int id, int loc)
{
    CHSSysSensors *cSensors;
    SENSOR_CONTACT *cContact;

    CHSShip *cShip;
    cShip = (CHSShip *) GetOwnerObject();

    if (!cShip)
        return;

    // Do we have sensors?
    cSensors = (CHSSysSensors *) cShip->GetSystems().GetSystem(HSS_SENSORS);
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

    if (cObj->GetType() != HST_SHIP)
    {
        hsStdError(player, "Cannot tractor dock that kind of object.");
        return;
    }

    if (Dist3D
        (cShip->GetX(), cShip->GetY(), cShip->GetZ(), cObj->GetX(),
         cObj->GetY(), cObj->GetZ()) > 2)
    {
        hsStdError(player,
                   hsInterface.
                   HSPrintf("Target outside of tractor docking range(2 %s).",
                            HSCONF.unit_name));
        return;
    }

    CHSShip *tShip;
    tShip = (CHSShip *) cObj;

    // Are they too big?
    if ((int) tShip->GetSize() > HSCONF.max_dock_size &&
        !cShip->IsSpacedock())
    {
        hsStdError(player,
                   "That vessel is too large to dock in another vessel.");
        return;
    }

    // Are we too small?
    if (tShip->GetSize() >= cShip->GetSize())
    {
        hsStdError(player, "We are too small to accomodate that vessel.");
        return;
    }

    CHSSysEngines *cEngines;
    cEngines = (CHSSysEngines *) tShip->GetSystems().GetSystem(HSS_ENGINES);

    // Are they at full stop?
    if (cEngines && cEngines->GetCurrentSpeed())
    {
        hsStdError(player,
                   "Target must be at a full stop to commence docking.");
        return;
    }

    CHSLandingLoc *cLocation;

    // Find the landing location
    // The landing locations are specified by the player as
    // 1 .. n, but we store them in array notation 0..n-1.
    // Thus, subtract 1 from location when retrieving.
    cLocation = cShip->GetLandingLoc(loc - 1);

    // Location exists?
    if (!cLocation)
    {
        hsStdError(player, "That landing location does not exist.");
        return;
    }

    // See if the landing site is active.
    if (!cLocation->IsActive())
    {
        hsStdError(player, "The bay doors to that bay are closed.");
        return;
    }

    // See if the landing site can accomodate us.
    if (!cLocation->CanAccomodate(tShip))
    {
        hsStdError(player, "Landing site cannot accomodate this vessel.");
        return;
    }

    // Set desired and current speed to 0 to stop the ship.
    cEngines->SetDesiredSpeed(0);
    cEngines->SetAttributeValue("CURRENT SPEED", "0");

    char tbuf[256];

    sprintf(tbuf,
            "Through the bay doors, the %s is tractored in and docks.",
            tShip->GetName());
    cLocation->HandleMessage(tbuf, MSG_GENERAL);
    sprintf(tbuf,
            "The %s pushes forward as it is tractored in and docks.",
            tShip->GetName());
    tShip->NotifyConsoles(tbuf, MSG_GENERAL);
    tShip->SetDocked();
    tShip->MoveShipObject(cLocation->Object());
    cShip->HandleMessage(hsInterface.
                         HSPrintf
                         ("The %s is being tractored into docking bay %d.",
                          tShip->GetName(), loc), MSG_SENSOR, (long *) tShip);

    // Set our location
    CHS3DObject *tOwner;
    tOwner = cLocation->GetOwnerObject();
    if (tOwner)
        tShip->m_objlocation = tOwner->GetDbref();
    else
        tShip->m_objlocation = HSNOTHING;

    // Remove us from active space
    CHSUniverse *uSource;
    uSource = tShip->GetUniverse();
    if (uSource)
        uSource->RemoveActiveObject(tShip);

    // Deduct the capacity from the landing loc
    cLocation->DeductCapacity(tShip->GetSize());
}
