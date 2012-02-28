// -----------------------------------------------------------------------
//! $Id: hscelestial.cpp,v 1.19 2006/04/04 12:41:11 mark Exp $
// -----------------------------------------------------------------------

#include "pch.h"

#include <cstdio>
#include <stdlib.h>
#include <cstring>

#include "hspace.h"
#include "hsconf.h"
#include "hsuniverse.h"
#include "hscelestial.h"
#include "hsinterface.h"
#include "hsutils.h"
#include "hsansi.h"
#include "hsobjects.h"
#include "hsuniversedb.h"
#include "hsflags.h"
#include "hssensors.h"


// Overridden from the CHS3DObject class
HS_BOOL8 CHSCelestial::HandleKey(HS_INT32 key, HS_INT8 * strValue, FILE * fp)
{
    // Find the key and handle it
    switch (key)
    {
    default:                   // Pass it up to CHS3DObject
        return (CHS3DObject::HandleKey(key, strValue, fp));
    }
}

// Basic celestial objects are cyan
HS_INT8 *CHSCelestial::GetObjectColor()
{
    static HS_INT8 tbuf[32];

    sprintf_s(tbuf, "%s%s", ANSI_HILITE, ANSI_CYAN);
    return tbuf;
}

// Returns the character representing celestials in general
HS_INT8 CHSCelestial::GetObjectCharacter()
{
    // How about a simple star
    return '*';
}


// Overridden from the CHS3DObject class
void CHSCelestial::WriteToFile(FILE * fp)
{
    // Write base class info first, then our's.
    CHS3DObject::WriteToFile(fp);
}

void CHSCelestial::ClearObjectAttrs()
{
    CHS3DObject::ClearObjectAttrs();

    // Clear the attributes
    hsInterface.AtrAdd(m_objnum, "HSDB_X", NULL, hsInterface.GetGodDbref());
    hsInterface.AtrAdd(m_objnum, "HSDB_Y", NULL, hsInterface.GetGodDbref());
    hsInterface.AtrAdd(m_objnum, "HSDB_Z", NULL, hsInterface.GetGodDbref());
    hsInterface.AtrAdd(m_objnum, "HSDB_UID", NULL, hsInterface.GetGodDbref());
}

HS_UINT32 CHSCelestial::GetType()
{
    return m_type;
}

// Constructor
CHSNebula::CHSNebula()
{
    m_type = HST_NEBULA;
    m_visible = true;
    m_density = 1;
    m_shieldaff = 100.00;
}

// Nebulas are little ~'s
HS_INT8 CHSNebula::GetObjectCharacter()
{
    return '~';
}


// Nebulae are magenta
HS_INT8 *CHSNebula::GetObjectColor()
{
    static HS_INT8 tbuf[32];

    sprintf_s(tbuf, "%s%s", ANSI_HILITE, ANSI_MAGENTA);
    return tbuf;

}

HS_INT32 CHSNebula::GetDensity()
{
    return m_density;
}

HS_FLOAT32 CHSNebula::GetShieldaff()
{
    return m_shieldaff;
}

// Overridden from the CHSCelestial class
void CHSNebula::WriteToFile(FILE * fp)
{
    // Write base class info first, then our's.
    CHSCelestial::WriteToFile(fp);
    fprintf(fp, "DENSITY=%d\n", m_density);
    fprintf(fp, "SHIELDAFF=%f\n", m_shieldaff);
}

void CHSNebula::ClearObjectAttrs()
{
    CHSCelestial::ClearObjectAttrs();
}

void CHSNebula::GetAttributeList(CHSAttributeList & rlistAttributes)
{
    rlistAttributes.push_back("DENSITY");
    rlistAttributes.push_back("SHIELDAFF");

    CHSCelestial::GetAttributeList(rlistAttributes);
}

HS_INT8 *CHSNebula::GetAttributeValue(HS_INT8 * strName)
{
    static HS_INT8 rval[64];
    *rval = '\0';

    if (!_stricmp(strName, "DENSITY"))
    {
        sprintf_s(rval, "%d", m_density);
    }
    else if (!_stricmp(strName, "SHIELDAFF"))
    {
        sprintf_s(rval, "%f", m_shieldaff);
    }
    else
        return CHSCelestial::GetAttributeValue(strName);

    return rval;
}

HS_BOOL8 CHSNebula::HandleKey(HS_INT32 key, HS_INT8 * strValue, FILE * fp)
{
    // Find the key and handle it
    switch (key)
    {
    case HSK_DENSITY:
        m_density = atoi(strValue);
        return true;
    case HSK_SHIELDAFF:
        m_shieldaff = (HS_FLOAT32) atof(strValue);
        return true;
    default:                   // Pass it up to CHSCelestial
        return (CHSCelestial::HandleKey(key, strValue, fp));
    }
}

HS_BOOL8 CHSNebula::SetAttributeValue(HS_INT8 * strName, HS_INT8 * strValue)
{

    if (!_stricmp(strName, "DENSITY"))
    {
        m_density = atoi(strValue);
        return true;
    }
    else if (!_stricmp(strName, "SHIELDAFF"))
    {
        if (atof(strValue) < 0.00)
            return false;

        m_shieldaff = (HS_FLOAT32) atof(strValue);
        return true;
    }
    else
    {
        return (CHSCelestial::SetAttributeValue(strName, strValue));
    }
}

void CHSNebula::GiveScanReport(CHS3DObject * cScanner,
                               HS_DBREF player, HS_BOOL8 id)
{
    HS_INT8 tbuf[256];

    // Print a header
    sprintf_s(tbuf,
            "%s%s.------------------------------------------------.%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);
    sprintf_s(tbuf,
            "%s%s|%s Scan Report    %30s  %s%s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
            id ? GetName() : "Unknown", ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);
    sprintf_s(tbuf,
            "%s%s >----------------------------------------------<%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    // Give Nebula info
    sprintf_s(tbuf,
            "%s%s| %sX:%s %9.0f              %s%sDiameter:%s %-7d %s %s%s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
            GetX(),
            ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
            GetSize() * HSCONF.nebula_size_multiplier, HSCONF.unit_name,
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    sprintf_s(tbuf,
            "%s%s| %sY:%s %9.0f               %s%sDensity:%s %-3d        %s%s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
            GetY(),
            ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
            GetDensity(), ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    sprintf_s(tbuf,
            "%s%s| %sZ:%s %9.0f     %s%sShield Efficiency:%s %3.2f%%    %s%s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
            GetZ(), ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
            GetShieldaff(), ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);
    // Finish the report
    sprintf_s(tbuf,
            "%s%s`------------------------------------------------'%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);
}


// Constructor
CHSAsteroid::CHSAsteroid():
m_density(1)
{
    m_type = HST_ASTEROID;
    m_visible = true;
}

// Asteroid Belts are little :'s
HS_INT8 CHSAsteroid::GetObjectCharacter()
{
    return ':';
}

CHSAsteroid::~CHSAsteroid()
{

}

// Asteroid Belts are black
HS_INT8 *CHSAsteroid::GetObjectColor()
{
    static HS_INT8 tbuf[32];

    sprintf_s(tbuf, "%s%s", ANSI_HILITE, ANSI_BLACK);
    return tbuf;
}

HS_INT32 CHSAsteroid::GetDensity()
{
    return m_density;
}

// Overridden from the CHSCelestial class
void CHSAsteroid::WriteToFile(FILE * fp)
{
    if (NULL == fp)
    {
        hs_log("Invalid file pointer passed to CHSAsteroid::WriteToFile()");
        return;
    }

    // Write base class info first, then our's.
    CHSCelestial::WriteToFile(fp);
    fprintf(fp, "DENSITY=%d\n", m_density);
}

void CHSAsteroid::ClearObjectAttrs()
{
    CHSCelestial::ClearObjectAttrs();

    m_density = 0;
}

HS_INT8 *CHSAsteroid::GetAttributeValue(HS_INT8 * strName)
{
    static HS_INT8 rval[64];
    *rval = '\0';

    if (!_stricmp(strName, "DENSITY"))
    {
        sprintf_s(rval, "%d", m_density);
    }
    else
        return CHSCelestial::GetAttributeValue(strName);

    return rval;
}

void CHSAsteroid::GetAttributeList(CHSAttributeList & rlistAttributes)
{
    rlistAttributes.push_back("DENSITY");
    rlistAttributes.push_back("SHIELDAFF");

    CHSCelestial::GetAttributeList(rlistAttributes);
}

HS_BOOL8 CHSAsteroid::HandleKey(HS_INT32 key, HS_INT8 * strValue, FILE * fp)
{

    if (NULL == fp)
    {
        hs_log("Invalid file pointer passed to CHSAsteroid::HandleKey()");
        return false;
    }

    // Find the key and handle it
    switch (key)
    {
    case HSK_DENSITY:
        m_density = atoi(strValue);
        return true;
    default:                   // Pass it up to CHSCelestial
        return (CHSCelestial::HandleKey(key, strValue, fp));
    }
}

HS_BOOL8 CHSAsteroid::SetAttributeValue(HS_INT8 * strName, HS_INT8 * strValue)
{

    if (!_stricmp(strName, "DENSITY"))
    {
        m_density = atoi(strValue);
        return true;
    }
    else
    {
        return (CHSCelestial::SetAttributeValue(strName, strValue));
    }
}

void CHSAsteroid::GiveScanReport(CHS3DObject * cScanner,
                                 HS_DBREF player, HS_BOOL8 id)
{
    HS_INT8 tbuf[256];

    // Print a header
    sprintf_s(tbuf,
            "%s%s.------------------------------------------------.%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);
    sprintf_s(tbuf,
            "%s%s|%s Scan Report    %30s  %s%s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
            id ? GetName() : "Unknown", ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);
    sprintf_s(tbuf,
            "%s%s >----------------------------------------------<%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    // Give Nebula info
    sprintf_s(tbuf,
            "%s%s| %sX:%s %9.0f              %s%sDiameter:%s %7d %s %s%s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
            GetX(),
            ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
            GetSize() * HSCONF.asteroid_size_multiplier, HSCONF.unit_name,
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    sprintf_s(tbuf,
            "%s%s| %sY:%s %9.0f               %s%sDensity:%s %3d/100m2  %s%s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
            GetY(),
            ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
            GetDensity(), ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    sprintf_s(tbuf,
            "%s%s| %sZ:%s %9.0f%35s%s%s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
            GetZ(), " ", ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);
    // Finish the report
    sprintf_s(tbuf,
            "%s%s`------------------------------------------------'%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);
}

void CHSAsteroid::DoCycle()
{
    CHSUniverse *pSourceUniverse;
    HS_FLOAT64 dDistance;


    pSourceUniverse = GetUniverse();
    if (!pSourceUniverse)
    {
        return;
    }

    // Grab all of the objects in the universe, and see if they're in the area.
    THSObjectIterator tIterator;
    HS_BOOL8 bContinue;
    for (bContinue = pSourceUniverse->GetFirstObject(tIterator, HST_SHIP);
         bContinue;
         bContinue = pSourceUniverse->GetNextObject(tIterator, HST_SHIP))
    {
        CHSShip *pTarget = static_cast < CHSShip * >(tIterator.pValue);
        dDistance = Dist3D(GetX(), GetY(), GetZ(),
                           pTarget->GetX(), pTarget->GetY(), pTarget->GetZ());

        if (dDistance > GetSize() * HSCONF.asteroid_size_multiplier)
        {
            continue;
        }

        if (GetDensity() < (int) hsInterface.GetRandom(50 / pTarget->GetSize()
                                                       *
                                                       ((pTarget->GetSpeed() +
                                                         1) / 1000)))
        {
            continue;
        }

        HS_INT32 strength;

        strength = hsInterface.GetRandom(GetDensity() * pTarget->GetSize() *
                                         ((pTarget->GetSpeed() + 1) / 1000));

        pTarget->HullPoints(pTarget->HullPoints() - strength);

        pTarget->
            NotifySrooms("The ship shakes as asteroids impact on the hull");

        // Is hull < 0?
        if (pTarget->HullPoints() < 0)
        {
            pTarget->ExplodeMe();
            if (!hsInterface.HasFlag(m_objnum, TYPE_THING, THING_HSPACE_SIM))
            {
                pTarget->KillShipCrew("THE SHIP EXPLODES!!");
            }
        }
    }

}

CHSBlackHole::CHSBlackHole()
{
    m_type = HST_BLACKHOLE;
    m_visible = true;
}

// Black holes are big O's
HS_INT8 CHSBlackHole::GetObjectCharacter()
{
    return 'O';
}

CHSBlackHole::~CHSBlackHole()
{

}

// Nebulae are magenta
HS_INT8 *CHSBlackHole::GetObjectColor()
{
    static HS_INT8 tbuf[32];

    sprintf_s(tbuf, "%s", ANSI_BLUE);
    return tbuf;
}

void CHSBlackHole::DoCycle()
{
    CHSUniverse *pSourceUniverse;
    HS_FLOAT64 dDistance;


    pSourceUniverse = GetUniverse();
    if (!pSourceUniverse)
    {
        return;
    }

    // Grab all of the objects in the universe, and see if they're in the area.
    THSObjectIterator tIterator;
    HS_BOOL8 bContinue;
    for (bContinue = pSourceUniverse->GetFirstObject(tIterator, HST_SHIP);
         bContinue;
         bContinue = pSourceUniverse->GetNextObject(tIterator, HST_SHIP))
    {
        CHSShip *pTarget = static_cast < CHSShip * >(tIterator.pValue);
        dDistance = Dist3D(GetX(), GetY(), GetZ(),
                           pTarget->GetX(), pTarget->GetY(), pTarget->GetZ());

        if (dDistance > GetSize() * 100 - 0.01)
        {
            continue;
        }

        HS_FLOAT32 strength;

        strength = (HS_FLOAT32) ((GetSize() * 100) / dDistance * 100 - 100);

        pTarget->MoveTowards(m_x, m_y, m_z, strength);

        pTarget->HullPoints(pTarget->HullPoints() - (HS_INT32) strength);

        pTarget->
            NotifySrooms("The hull buckles from the black hole's gravity.");

        // Is hull < 0?
        if (pTarget->HullPoints() < 0)
        {
            pTarget->ExplodeMe();
            if (!hsInterface.HasFlag(m_objnum, TYPE_THING, THING_HSPACE_SIM))
                pTarget->KillShipCrew("THE SHIP EXPLODES!!");
        }

    }

}

// Constructor
CHSWormHole::CHSWormHole()
{
    m_type = HST_WORMHOLE;
    m_visible = true;
    m_stability = 0.0;
    m_fluctuation = 0.0;
    m_basestability = 0.0;
    m_destx = m_x;
    m_desty = m_y;
    m_destz = m_z;
    m_destuid = m_uid;
    m_nodamage = false;
    m_desterror = 0;
}

// Asteroid Belts are little :'s
HS_INT8 CHSWormHole::GetObjectCharacter()
{
    return '0';
}

CHSWormHole::~CHSWormHole()
{

}

// Asteroid Belts are black
HS_INT8 *CHSWormHole::GetObjectColor()
{
    static HS_INT8 tbuf[32];

    sprintf_s(tbuf, "%s", ANSI_RED);
    return tbuf;
}

HS_FLOAT32 CHSWormHole::GetStability()
{
    return m_stability;
}

HS_FLOAT32 CHSWormHole::GetFluctuation()
{
    return m_fluctuation;
}

HS_FLOAT32 CHSWormHole::GetBaseStability()
{
    return m_basestability;
}

// Overridden from the CHSCelestial class
void CHSWormHole::WriteToFile(FILE * fp)
{
    // Write base class info first, then our's.
    CHSCelestial::WriteToFile(fp);
    fprintf(fp, "STABILITY=%f\n", m_stability);
    fprintf(fp, "FLUCTUATION=%f\n", m_fluctuation);
    fprintf(fp, "BASESTABILITY=%f\n", m_basestability);
    fprintf(fp, "DESTX=%.0f\n", m_destx);
    fprintf(fp, "DESTY=%.0f\n", m_desty);
    fprintf(fp, "DESTZ=%.0f\n", m_destz);
    fprintf(fp, "DESTUID=%d\n", m_destuid);
    fprintf(fp, "DESTERROR=%d\n", m_desterror);
    fprintf(fp, "NODAMAGE=%d\n", m_nodamage ? 1 : 0);
}

void CHSWormHole::ClearObjectAttrs()
{
    CHSCelestial::ClearObjectAttrs();
}


void CHSWormHole::GetAttributeList(CHSAttributeList & rlistAttributes)
{
    rlistAttributes.push_back("STABILITY");
    rlistAttributes.push_back("FLUCTUATION");
    rlistAttributes.push_back("BASESTABILITY");
    rlistAttributes.push_back("DESTX");
    rlistAttributes.push_back("DESTY");
    rlistAttributes.push_back("DESTZ");
    rlistAttributes.push_back("DESTUID");
    rlistAttributes.push_back("DESTERROR");
    rlistAttributes.push_back("NODAMAGE");

    CHSCelestial::GetAttributeList(rlistAttributes);
}

HS_INT8 *CHSWormHole::GetAttributeValue(HS_INT8 * strName)
{
    static HS_INT8 rval[64];
    *rval = '\0';

    if (!_stricmp(strName, "STABILITY"))
    {
        sprintf_s(rval, "%f", m_stability);
    }
    else if (!_stricmp(strName, "FLUCTUATION"))
    {
        sprintf_s(rval, "%f", m_fluctuation);
    }
    else if (!_stricmp(strName, "BASESTABILITY"))
    {
        sprintf_s(rval, "%f", m_basestability);
    }
    else if (!_stricmp(strName, "DESTX"))
    {
        sprintf_s(rval, "%.0f", m_destx);
    }
    else if (!_stricmp(strName, "DESTY"))
    {
        sprintf_s(rval, "%.0f", m_desty);
    }
    else if (!_stricmp(strName, "DESTZ"))
    {
        sprintf_s(rval, "%.0f", m_destz);
    }
    else if (!_stricmp(strName, "DESTUID"))
    {
        sprintf_s(rval, "%d", m_destuid);
    }
    else if (!_stricmp(strName, "DESTERROR"))
    {
        sprintf_s(rval, "%d", m_desterror);
    }
    else if (!_stricmp(strName, "NODAMAGE"))
    {
        sprintf_s(rval, "%d", m_nodamage ? 1 : 0);
    }
    else
        return CHSCelestial::GetAttributeValue(strName);

    return rval;
}

HS_BOOL8 CHSWormHole::HandleKey(HS_INT32 key, HS_INT8 * strValue, FILE * fp)
{
    // Find the key and handle it
    HS_INT8 tmp[64];
    sprintf_s(tmp, "%d", key);
    switch (key)
    {
    case HSK_STABILITY:
        m_stability = (HS_FLOAT32) atof(strValue);
        return true;
    case HSK_FLUCTUATION:
        m_fluctuation = (HS_FLOAT32) atof(strValue);
        return true;
    case HSK_BASESTABILITY:
        m_basestability = (HS_FLOAT32) atof(strValue);
        return true;
    case HSK_DESTX:
        m_destx = (HS_FLOAT32) atof(strValue);
        return true;
    case HSK_DESTY:
        m_desty = (HS_FLOAT32) atof(strValue);
        return true;
    case HSK_DESTZ:
        m_destz = (HS_FLOAT32) atof(strValue);
        return true;
    case HSK_DESTUID:
        m_destuid = atoi(strValue);
        return true;
    case HSK_DESTERROR:
        m_desterror = atoi(strValue);
        return true;
    case HSK_NODAMAGE:
        m_nodamage = atoi(strValue) ? true : false;
        return true;
    default:                   // Pass it up to CHSCelestial
        return (CHSCelestial::HandleKey(key, strValue, fp));
    }
}

HS_BOOL8 CHSWormHole::SetAttributeValue(HS_INT8 * strName, HS_INT8 * strValue)
{

    if (!_stricmp(strName, "STABILITY"))
    {
        m_stability = (HS_FLOAT32) atof(strValue);
        return true;
    }
    else if (!_stricmp(strName, "FLUCTUATION"))
    {
        m_fluctuation = (HS_FLOAT32) atof(strValue);
        return true;
    }
    else if (!_stricmp(strName, "BASESTABILITY"))
    {
        m_basestability = (HS_FLOAT32) atof(strValue);
        return true;
    }
    else if (!_stricmp(strName, "DESTX"))
    {
        m_destx = (HS_FLOAT32) atof(strValue);
        return true;
    }
    else if (!_stricmp(strName, "DESTY"))
    {
        m_desty = (HS_FLOAT32) atof(strValue);
        return true;
    }
    else if (!_stricmp(strName, "DESTZ"))
    {
        m_destz = (HS_FLOAT32) atof(strValue);
        return true;
    }
    else if (!_stricmp(strName, "DESTUID"))
    {
        m_destuid = atoi(strValue);
        return true;
    }
    else if (!_stricmp(strName, "DESTERROR"))
    {
        m_desterror = atoi(strValue);
        return true;
    }
    else if (!_stricmp(strName, "NODAMAGE"))
    {
        m_nodamage = atoi(strValue) ? true : false;
        return true;
    }
    else
    {
        return (CHSCelestial::SetAttributeValue(strName, strValue));
    }
}

void CHSWormHole::GiveScanReport(CHS3DObject * cScanner,
                                 HS_DBREF player, HS_BOOL8 id)
{
    HS_INT8 tbuf[256];

    // Print a header
    sprintf_s(tbuf,
            "%s%s.------------------------------------------------.%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);
    sprintf_s(tbuf,
            "%s%s|%s Scan Report    %30s  %s%s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
            id ? GetName() : "Unknown", ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);
    sprintf_s(tbuf,
            "%s%s >----------------------------------------------<%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    // Give Nebula info
    sprintf_s(tbuf,
            "%s%s| %sX:%s %9.0f                  %s%sSize:%s %-3d        %s%s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
            GetX(),
            ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
            GetSize(), ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    sprintf_s(tbuf,
            "%s%s| %sY:%s %9.0f             %s%sStability:%s %-10s %s%s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
            GetY(),
            ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
            hsInterface.HSPrintf("%3.6f%%", GetStability()),
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    sprintf_s(tbuf,
            "%s%s| %sZ:%s %9.0f %34s%s%s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
            GetZ(), " ", ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);
    // Finish the report
    sprintf_s(tbuf,
            "%s%s`------------------------------------------------'%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);
}

void CHSWormHole::DoCycle()
{
    if (m_fluctuation <= 0)
        return;

    m_stability += (HS_FLOAT32)
        ((hsInterface.GetRandom(20000) - 10000) / 1000000.00);

    if (m_stability < (m_basestability - m_fluctuation))
    {
        m_stability = m_basestability - m_fluctuation;
    }

    if (m_stability > (m_basestability + m_fluctuation))
    {
        m_stability = m_basestability + m_fluctuation;
    }
}

void CHSWormHole::GateShip(CHSShip * cShip)
{
    if (!cShip)
        return;

    HS_BOOL8 succeed;

    if (hsInterface.GetRandom(100) <= m_stability)
        succeed = true;
    else
        succeed = false;

    // always succeed if nodamage is flagged to true
    if (m_nodamage == true)
    {
        succeed = true;
    }

    if (hsInterface.AtrGet(GetDbref(), "HSWH_ENTER_CONSOLES"))
    {
        char *msg = hsInterface.EvalExpression(hsInterface.m_buffer,
                                               GetDbref(), GetDbref(),
                                               GetDbref());
        cShip->NotifyConsoles(msg, MSG_SENSOR);
    }
    else
    {
        cShip->NotifyConsoles("The ship enters the wormhole and an infinite \
				amount of colors can be seen.", MSG_SENSOR);
    }

    if (hsInterface.AtrGet(GetDbref(), "HSWH_ENTER_SROOMS"))
    {
        char *msg = hsInterface.EvalExpression(hsInterface.m_buffer,
                                               GetDbref(), GetDbref(),
                                               GetDbref());
        cShip->NotifySrooms(msg);
    }
    else
    {
        cShip->
            NotifySrooms("The ship shakes slightly as it enters a wormhole.");
    }

    CHSUniverse *uDest;
    uDest = cShip->GetUniverse();
    if (uDest)
    {
        uDest->SendContactMessage("In the distance a ship gates a wormhole.",
                                  DETECTED, cShip);
        uDest->SendContactMessage(hsInterface.
                                  HSPrintf
                                  ("In the distance the %s gates a wormhole.",
                                   cShip->GetName()), IDENTIFIED, cShip);
    }

    if (succeed)
    {
        // If the destination error is more than 0, randomly calculate
        // coordinate offsets up to the maximum about based on the
        // current stability
        float xerr = 0, yerr = 0, zerr = 0;
        if (m_desterror > 0)
        {
            // Calculate the maximal offset based on the current
            // stability of the wormhole
            xerr = m_desterror * ((100 - m_stability) / 100);

            // Same calc, copy the value
            yerr = xerr;
            zerr = xerr;

            // Generate random offsets based on the maximum values
            xerr = hsInterface.GetRandom((HS_UINT32) xerr);
            yerr = hsInterface.GetRandom((HS_UINT32) yerr);
            zerr = hsInterface.GetRandom((HS_UINT32) zerr);

            // Randomly move the offset +/-
            xerr *= hsInterface.GetRandom(2) ? 1 : -1;
            yerr *= hsInterface.GetRandom(2) ? 1 : -1;
            zerr *= hsInterface.GetRandom(2) ? 1 : -1;
        }

        // Change the destination coordinates
        if (m_destx + xerr)
            cShip->SetX(m_destx + xerr);
        if (m_desty + yerr)
            cShip->SetY(m_desty + yerr);
        if (m_destz + zerr)
            cShip->SetZ(m_destz + zerr);

        CHSUniverse *uSource;

        uDest = CHSUniverseDB::GetInstance().FindUniverse(m_destuid);
        if (uDest)
        {
            // Grab the source universe
            uSource = cShip->GetUniverse();

            // Now pull it from one, and put it in another
            uSource->RemoveObject(cShip);
            cShip->SetUID(m_destuid);
            uDest->AddObject(cShip);
        }

        if (hsInterface.AtrGet(GetDbref(), "HSWH_EXIT_CONSOLES"))
        {
            char *msg = hsInterface.EvalExpression(hsInterface.m_buffer,
                                                   GetDbref(), GetDbref(),
                                                   GetDbref());
            cShip->NotifyConsoles(msg, MSG_SENSOR);
        }
        else
        {
            cShip->
                NotifyConsoles
                ("The ship safely emerges on the other side of the wormhole.",
                 MSG_SENSOR);
        }

        if (hsInterface.AtrGet(GetDbref(), "HSWH_EXIT_SROOMS"))
        {
            char *msg = hsInterface.EvalExpression(hsInterface.m_buffer,
                                                   GetDbref(), GetDbref(),
                                                   GetDbref());
            cShip->NotifySrooms(msg);
        }
    }
    else
    {
        if (hsInterface.AtrGet(GetDbref(), "HSWH_COLLAPSE_CONSOLES"))
        {
            char *msg = hsInterface.EvalExpression(hsInterface.m_buffer,
                                                   GetDbref(), GetDbref(),
                                                   GetDbref());
            cShip->NotifyConsoles(msg, MSG_SENSOR);
        }
        else
        {
            cShip->
                NotifyConsoles
                ("The wormhole collapses and the structural integrity is comprimised.",
                 MSG_SENSOR);
        }

        cShip->ExplodeMe();
        if (!hsInterface.HasFlag(cShip->GetDbref(), TYPE_THING,
                                 THING_HSPACE_SIM))
        {
            if (hsInterface.AtrGet(GetDbref(), "HSWH_KILL_CREW"))
            {
                char *msg = hsInterface.EvalExpression(hsInterface.m_buffer,
                                                       GetDbref(), GetDbref(),
                                                       GetDbref());
                cShip->KillShipCrew(msg);
            }

            else
            {
                cShip->
                    KillShipCrew
                    ("The ship explodes as the structural integrity fails!");
            }
        }
    }
}
