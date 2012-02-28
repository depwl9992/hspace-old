//
// hsplanet.cpp
//
// Handles all of the methods for the CHSPlanet object.
//
//
#include "pch.h"

#include <cstring>
#include <stdlib.h>

#include "hscopyright.h"
#include "hsobjects.h"
#include "hsuniverse.h"
#include "hsinterface.h"
#include "hscelestial.h"
#include "hsutils.h"
#include "hsansi.h"
#include "hsflags.h"

// Constructor
CHSPlanet::CHSPlanet()
{
    m_type = HST_PLANET;
}

CHSPlanet::~CHSPlanet()
{
}

// Planets are green
char *CHSPlanet::GetObjectColor()
{
    static char tbuf[32];

    sprintf(tbuf, "%s%s", ANSI_HILITE, ANSI_GREEN);
    return tbuf;
}

// Planets are little o's
char CHSPlanet::GetObjectCharacter()
{
    return 'o';
}

// Overridden from the CHSCelestial class
HS_BOOL8 CHSPlanet::HandleKey(int key, char *strValue, FILE * fp)
{
    // Find the key and handle it
    switch (key)
    {
    case HSK_SIZE:
        m_size = atoi(strValue);
        return true;
    default:                   // Pass it up to CHSCelestial
        return (CHSCelestial::HandleKey(key, strValue, fp));
    }
}

// Overridden from the CHSCelestial class
void CHSPlanet::WriteToFile(FILE * fp)
{
    // Write base class info first, then our's.
    CHSCelestial::WriteToFile(fp);

}


// Clears attributes specific to the planet object
void CHSPlanet::ClearObjectAttrs()
{
    CHSCelestial::ClearObjectAttrs();
}

void CHSPlanet::GetAttributeList(CHSAttributeList & rlistAttributes)
{
    CHSCelestial::GetAttributeList(rlistAttributes);
}

HS_INT8 *CHSPlanet::GetAttributeValue(HS_INT8 * strName)
{
    return CHSCelestial::GetAttributeValue(strName);

}

HS_BOOL8 CHSPlanet::SetAttributeValue(HS_INT8 * strName, HS_INT8 * strValue)
{
    return (CHSCelestial::SetAttributeValue(strName, strValue));
}


void CHSPlanet::GiveScanReport(CHS3DObject * cScanner,
                               HS_DBREF player, HS_BOOL8 id)
{
    char tbuf[256];

    // Print a header
    sprintf(tbuf,
            "%s%s.------------------------------------------------.%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);
    sprintf(tbuf,
            "%s%s|%s Scan Report    %30s  %s%s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
            id ? GetName() : "Unknown", ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);
    sprintf(tbuf,
            "%s%s >----------------------------------------------<%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    // Give object info
    sprintf(tbuf,
            "%s%s| %sX:%s %9.0f                   %s%sSize:%s %-3d       %s%s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
            GetX(),
            ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
            GetSize(), ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    sprintf(tbuf,
            "%s%s| %sY:%s %9.0f%35s%s%s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
            GetY(), " ", ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    sprintf(tbuf,
            "%s%s| %sZ:%s %9.0f%35s%s%s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
            GetZ(), " ", ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

    // Do we have any landing locs to report?
    if (GetNumVisibleLandingLocs() > 0)
    {
        // Print em baby.
        hsInterface.Notify(player,
                           hsInterface.HSPrintf("%s%s|%48s|%s",
                                                ANSI_HILITE, ANSI_BLUE, " ",
                                                ANSI_NORMAL));
        sprintf(tbuf, "%s%s| %sLanding Locations:%s %-2d%26s%s%s|%s",
                ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
                GetNumVisibleLandingLocs(), " ", ANSI_HILITE, ANSI_BLUE,
                ANSI_NORMAL);
        hsInterface.Notify(player, tbuf);

        sprintf(tbuf,
                "%s%s| %s[%s##%s] Name                     Active  Code     %s|%s",
                ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_WHITE, ANSI_GREEN,
                ANSI_BLUE, ANSI_NORMAL);
        hsInterface.Notify(player, tbuf);

        char strPadName[32];
        CSTLLandingLocList::iterator iter;
        HS_UINT32 idx = 0;
        for (iter = m_listLandingLocs.begin();
             iter != m_listLandingLocs.end(); iter++)
        {
            CHSLandingLoc *pLoc = *iter;

            // Not visible?  Don't print it.
            if (!pLoc->IsVisible())
            {
                idx++;
                continue;
            }

            strncpy(strPadName, hsInterface.GetName(pLoc->Object()), 32);
            strPadName[23] = '\0';
            sprintf(tbuf,
                    "%s%s|%s  %2d  %-25s  %3s   %3s      %s%s|%s",
                    ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
                    idx + 1,
                    strPadName,
                    pLoc->IsActive()? "Yes" : "No",
                    pLoc->CodeRequired()? "Yes" : "No",
                    ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
            hsInterface.Notify(player, tbuf);
            idx++;
        }
    }

    // Finish the report
    sprintf(tbuf,
            "%s%s`------------------------------------------------'%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

}
