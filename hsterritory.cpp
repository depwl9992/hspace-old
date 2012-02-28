#include "pch.h"

#include <cstdio>
#include <cstring>
#include <stdlib.h>

#include "hsobjects.h"
#include "hsterritory.h"
#include "hsutils.h"
#include "hsinterface.h"
#include "hsflags.h"

CHSTerritoryArray taTerritories;        // Global instance


//
// CHSTerritoryArray
//
CHSTerritoryArray::CHSTerritoryArray()
{
    m_territories.clear();
}

CHSTerritoryArray::~CHSTerritoryArray()
{
    for(unsigned int i=0; i < m_territories.size(); i++)
    {
        if(NULL != m_territories[i])
        {
            delete m_territories[i];
        }
    }
}

// Attempts to allocate a new territory in the array of the
// specified type.
CHSTerritory *CHSTerritoryArray::NewTerritory(int objnum, TERRTYPE type)
{

    CHSTerritory* territory;
    // Determine type of territory
    switch (type)
    {
    case T_RADIAL:
        territory = new CHSRadialTerritory;
        break;

    case T_CUBIC:
        territory = new CHSCubicTerritory;
        break;

    default:                   // What is this territory type?
        return NULL;
    }

    // Set object num
    territory->SetDbref(objnum);

    // Set the territory flag
    if (objnum != HSNOTHING)
    {
        hsInterface.SetToggle(territory->GetDbref(), THING_HSPACE_TERRITORY);
        hs_log(hsInterface.HSPrintf("%s Territory %s (#%d) added.",
                    type == 0 ? "Radial" : "Cubic",
                    hsInterface.GetName(objnum), objnum));
    }

    m_territories.push_back(territory);
    return territory;
}

// Saves territories to the specified file path.
void CHSTerritoryArray::SaveToFile(const char *lpstrPath)
{
    FILE *fp = NULL;

    // Is the file path good?
    if (!lpstrPath || strlen(lpstrPath) == 0)
    {
        hs_log
            ("Attempt to save the territory database, but no path specified. \
             Could be a config file problem.");
        return;
    }

    // Try to open the specified file for writing.
    fopen_s(&fp, lpstrPath, "w");
    if (!fp)
    {
        hs_log(hsInterface.HSPrintf("Failed to open territory db '%s'.",
                    lpstrPath));
        return;                 // bummer
    }

    // Run through our territories, telling them to write
    // to the file.
    for (unsigned int idx = 0; idx < m_territories.size(); idx++)
    {
        fprintf(fp, "TERRITORY=%d\n", m_territories.at(idx)->GetType());
        m_territories.at(idx)->SaveToFile(fp);
    }

    fclose(fp);
}

// Returns the number of territories loaded.
HS_UINT32 CHSTerritoryArray::NumTerritories()
{
    return m_territories.size();
}

// Prints information about the loaded territories to the
// specified player.
void CHSTerritoryArray::PrintInfo(int player)
{
    unsigned int idx;

    hsInterface.Notify(player,
                       "[Dbref] Name               Type    UID Specifications");

    for (idx = 0; idx < m_territories.size(); idx++)
    {
        switch (m_territories.at(idx)->GetType())
        {
            case T_RADIAL:
                CHSRadialTerritory * cRadial;
                cRadial = (CHSRadialTerritory *) m_territories.at(idx);
                hsInterface.Notify(player,
                        hsInterface.HSPrintf
                        ("[%5d] %-18s RADIAL  %-3d Center: %.0f,%.0f,%.0f  Radius: %d",
                         m_territories.at(idx)->GetDbref(),
                         hsInterface.GetName(m_territories.at(idx)->GetDbref()),
                         m_territories.at(idx)->GetUID(),
                         cRadial->GetX(), cRadial->GetY(),
                         cRadial->GetZ(), cRadial->GetRadius()));
                break;

            case T_CUBIC:
                CHSCubicTerritory * cCubic;
                cCubic = (CHSCubicTerritory *) m_territories.at(idx);
                hsInterface.Notify(player,
                        hsInterface.HSPrintf("[%5d] %-18s CUBIC   %-3d Min: %f,%f,%f  Max: %f,%f,%f",
                         m_territories.at(idx)->GetDbref(),
                         hsInterface.GetName(m_territories.at(idx)->GetDbref()),
                         m_territories.at(idx)->GetUID(),
                         cCubic->GetMinX(), cCubic->GetMinY(),
                         cCubic->GetMinZ(), cCubic->GetMaxX(),
                         cCubic->GetMaxY(), cCubic->GetMaxZ()));
                break;
        }
    }
}

// Loads territories from the specified file.
HS_BOOL8 CHSTerritoryArray::LoadFromFile(const char *lpstrPath)
{
    FILE *fp;

    // Try to open the specified file.
    fopen_s(&fp, lpstrPath, "r");
    if (!fp)
    {
        hs_log(hsInterface.HSPrintf("Failed to open %s for writing.",
                    lpstrPath));
        return false;           // Drat
    }

    // Read in the territories file, looking for TERRITORY
    // keywords.
    char key[128];
    char value[128];
    char tbuf[256];
    char *ptr;
    CHSTerritory *cTerritory = NULL;
    while (fgets(tbuf, 128, fp))
    {
        // Strip returns
        if ((ptr = strchr(tbuf, '\r')))
        {
            *ptr = '\0';
        }

        if ((ptr = strchr(tbuf, '\n')))
        {
            *ptr = '\0';
        }

        // Pull out the key and value.
        extract(tbuf, key, 0, 1, '=');
        extract(tbuf, value, 1, 1, '=');

        // Is it a new territory?
        if (!_stricmp(key, "TERRITORY"))
        {
            // Grab a new territory.
            cTerritory = NewTerritory(HSNOTHING, (TERRTYPE) atoi(value));
            if (NULL == cTerritory)
            {
                hs_log("ERROR: Error encountered while loading territories.");
                hs_log("ERROR: Could not allocate memory for new territory.");
                break;
            }
        }
        else
        {
            // Assumed to be a territory key=value pair.
            // Pass it to the current territory.
            if (cTerritory)
            {
                if (!cTerritory->SetAttributeValue(key, value))
                {
                    hs_log(hsInterface.HSPrintf("WARNING: Failed to set \
                                attribute \"%s\" on territory.", key));

                }
            }
        }
    }
    fclose(fp);
    return true;
}

// Returns the territory that the given object falls within,
// or NULL if none matched.
CHSTerritory *CHSTerritoryArray::InTerritory(CHS3DObject * cObj)
{
    // Run through our territories, asking them for a match.
    HS_FLOAT64 x, y, z;
    int uid;

    x = cObj->GetX();
    y = cObj->GetY();
    z = cObj->GetZ();
    uid = cObj->GetUID();
    for (unsigned int idx = 0; idx < m_territories.size(); idx++)
    {
        if (m_territories.at(idx)->PtInTerritory(uid, x, y, z))
        {
            return m_territories.at(idx);
        }
    }
    return NULL;                // No match
}

// Returns a CHSTerritory given the object number representing
// the territory.
CHSTerritory *CHSTerritoryArray::FindTerritory(int objnum)
{
    for (unsigned int idx = 0; idx < m_territories.size(); idx++)
    {
        if (m_territories.at(idx)->GetDbref() == objnum)
        {
            return m_territories.at(idx);
        }
    }
    // No match
    return NULL;
}

// Removes the territory with the specified object number
HS_BOOL8 CHSTerritoryArray::RemoveTerritory(int objnum)
{
    for (unsigned int idx = 0; idx < m_territories.size(); idx++)
    {
        if (m_territories.at(idx)->GetDbref() == objnum)
        {
            // Remove the flag if the object is good.
            if (hsInterface.ValidObject(m_territories.at(idx)->GetDbref()))
            {
                hsInterface.UnsetToggle(m_territories.at(idx)->GetDbref(),
                                        THING_HSPACE_TERRITORY);
                hs_log(hsInterface.HSPrintf("Territory %s (#%d) deleted.",
                        hsInterface.GetName(m_territories.at(idx)->GetDbref()),
                        m_territories.at(idx)->GetDbref()));
            }

            // Free the memory from the allocation
            delete m_territories[idx];

            // Remove the pointer from the vector
            m_territories.erase(m_territories.begin() + idx);

            return true;
        }
    }
    return false;               // Not found
}

//
// CHSTerritory
//
CHSTerritory::CHSTerritory() :
    m_uid(-1),
    m_objnum(-1),
    m_type(T_CUBIC)
{
}

// Returns true if the given point is in this territory.
// Base class returns false always.
HS_BOOL8 CHSTerritory::PtInTerritory(int uid, HS_FLOAT64 x, HS_FLOAT64 y, HS_FLOAT64 z)
{
    return false;
}

// Sets the object HS_DBREF representing the territory
void CHSTerritory::SetDbref(HS_INT32 objnum)
{
    m_objnum = objnum;
}

// Returns the HS_DBREF of the object representing the territory
int CHSTerritory::GetDbref()
{
    return m_objnum;
}

// Returns the territory type for the territory.
TERRTYPE CHSTerritory::GetType()
{
    return m_type;
}

// Returns the UID of the territory.
int CHSTerritory::GetUID()
{
    return m_uid;
}

void CHSTerritory::GetAttributeList(CHSAttributeList & rlistAttributes)
{
    rlistAttributes.push_back("UID");
}

HS_INT8 *CHSTerritory::GetAttributeValue(HS_INT8 * strName)
{
    static HS_INT8 rval[64];
    *rval = '\0';

    if (!_stricmp(strName, "UID"))
    {
        sprintf_s(rval, "%d", m_uid);
    }
    else
    {
        return "#-1 Unknown Attribute";
    }

    return rval;
}

// Attempts to set the given attribute name to the specified
// value.  If successful, true is returned, otherwise false.
HS_BOOL8 CHSTerritory::SetAttributeValue(char *strName, char *strValue)
{
    int iVal;

    // Match the name
    if (!_stricmp(strName, "UID"))
    {
        iVal = atoi(strValue);
        m_uid = iVal;
        return true;
    }
    else if (!_stricmp(strName, "OBJNUM"))
    {
        iVal = atoi(strValue);
        m_objnum = iVal;

        hsInterface.SetToggle(m_objnum, THING_HSPACE_TERRITORY);
        return true;
    }
    return false;
}

// Writes attributes out for the territory to the given file pointer.
void CHSTerritory::SaveToFile(FILE * fp)
{
    fprintf(fp, "UID=%d\n", m_uid);
    fprintf(fp, "OBJNUM=%d\n", m_objnum);
}

//
// CHSRadialTerritory
//
CHSRadialTerritory::CHSRadialTerritory() :
    m_cx(0),
    m_cy(0),
    m_cz(0),
    m_radius(0)

{
    m_type = T_RADIAL;
}

void CHSRadialTerritory::GetAttributeList(CHSAttributeList & rlistAttributes)
{
    rlistAttributes.push_back("CX");
    rlistAttributes.push_back("CY");
    rlistAttributes.push_back("CZ");
    rlistAttributes.push_back("RADIUS");
    CHSTerritory::GetAttributeList(rlistAttributes);
}

HS_INT8 *CHSRadialTerritory::GetAttributeValue(HS_INT8 * strName)
{
    static HS_INT8 rval[64];
    *rval = '\0';

    if (!_stricmp(strName, "CX"))
    {
        sprintf_s(rval, "%.2f", m_cx);
    }
    else if (!_stricmp(strName, "CY"))
    {
        sprintf_s(rval, "%.2f", m_cy);
    }
    else if (!_stricmp(strName, "CZ"))
    {
        sprintf_s(rval, "%.2f", m_cz);
    }
    else if (!_stricmp(strName, "RADIUS"))
    {
        sprintf_s(rval, "%.0f", m_radius);
    }
    else
        return CHSTerritory::GetAttributeValue(strName);

    return rval;
}

// Returns the central X value for the territory.
HS_FLOAT64 CHSRadialTerritory::GetX()
{
    return m_cx;
}

// Returns the central Y value for the territory.
HS_FLOAT64 CHSRadialTerritory::GetY()
{
    return m_cy;
}

// Returns the central Z value for the territory.
HS_FLOAT64 CHSRadialTerritory::GetZ()
{
    return m_cz;
}

// Returns the radius of the territory
HS_FLOAT64 CHSRadialTerritory::GetRadius()
{
    return m_radius;
}

// Saves all radial territory attrs to the file stream.
void CHSRadialTerritory::SaveToFile(FILE * fp)
{
    // Write base attrs first
    CHSTerritory::SaveToFile(fp);

    // Now ours
    fprintf(fp, "CX=%f\n", m_cx);
    fprintf(fp, "CY=%f\n", m_cy);
    fprintf(fp, "CZ=%f\n", m_cz);
    fprintf(fp, "RADIUS=%.0f\n", m_radius);
}

// Attempts to set the given attribute name to the specified
// value.  If successful, true is returned, otherwise false.
HS_BOOL8 CHSRadialTerritory::SetAttributeValue(char *strName, char *strValue)
{
    HS_FLOAT64 iVal;

    // Match the name
    if (!_stricmp(strName, "CX"))
    {
        iVal = atof(strValue);
        m_cx = iVal;
        return true;
    }
    else if (!_stricmp(strName, "CY"))
    {
        iVal = atof(strValue);
        m_cy = iVal;
        return true;
    }
    else if (!_stricmp(strName, "CZ"))
    {
        iVal = atof(strValue);
        m_cz = iVal;
        return true;
    }
    else if (!_stricmp(strName, "RADIUS"))
    {
        iVal = atof(strValue);
        m_radius = (HS_FLOAT64) iVal;
        return true;
    }
    else
        return CHSTerritory::SetAttributeValue(strName, strValue);
}

// Returns true if the given point is in this territory.
HS_BOOL8 CHSRadialTerritory::PtInTerritory(int uid,
                                           HS_FLOAT64 x, HS_FLOAT64 y, HS_FLOAT64 z)
{
    // In our universe?
    if (uid != m_uid)
    {
        return false;
    }

    // true will be returned if the distance to the point is
    // less than our radius.
    double objDist;

    // Calculate squared distance from our center to point.
    objDist = ((m_cx - x) * (m_cx - x) +
               (m_cy - y) * (m_cy - y) + (m_cz - z) * (m_cz - z));

    // Square radius
    double rad = (m_radius * m_radius);

    // Compare
    if (objDist <= rad)
        return true;
    else
        return false;
}

//
// CHSCubicTerritory
//
CHSCubicTerritory::CHSCubicTerritory() :
    m_minx(0),
    m_miny(0),
    m_minz(0),
    m_maxx(0),
    m_maxy(0),
    m_maxz(0)
{

    m_type = T_CUBIC;
}

// Saves all cubic territory attrs to the file stream.
void CHSCubicTerritory::SaveToFile(FILE * fp)
{
    // Write base attrs first
    CHSTerritory::SaveToFile(fp);

    // Now ours
    fprintf(fp, "MINX=%f\n", m_minx);
    fprintf(fp, "MINY=%f\n", m_miny);
    fprintf(fp, "MINZ=%f\n", m_minz);
    fprintf(fp, "MAXX=%f\n", m_maxx);
    fprintf(fp, "MAXY=%f\n", m_maxy);
    fprintf(fp, "MAXZ=%f\n", m_maxz);
}

void CHSCubicTerritory::GetAttributeList(CHSAttributeList & rListAttributes)
{
    rListAttributes.push_back("MINX");
    rListAttributes.push_back("MINY");
    rListAttributes.push_back("MINZ");
    rListAttributes.push_back("MAXX");
    rListAttributes.push_back("MAXY");
    rListAttributes.push_back("MAXZ");
    CHSTerritory::GetAttributeList(rListAttributes);
}

HS_INT8 *CHSCubicTerritory::GetAttributeValue(HS_INT8 * strName)
{
    static HS_INT8 rval[64];
    *rval = '\0';

    if (!_stricmp(strName, "MAXX"))
    {
        sprintf_s(rval, "%.2f", m_maxx);
    }
    else if (!_stricmp(strName, "MAXY"))
    {
        sprintf_s(rval, "%.2f", m_maxy);
    }
    else if (!_stricmp(strName, "MAXZ"))
    {
        sprintf_s(rval, "%.2f", m_maxz);
    }
    if (!_stricmp(strName, "MINX"))
    {
        sprintf_s(rval, "%.2f", m_minx);
    }
    else if (!_stricmp(strName, "MINY"))
    {
        sprintf_s(rval, "%.2f", m_miny);
    }
    else if (!_stricmp(strName, "MINZ"))
    {
        sprintf_s(rval, "%.2f", m_minz);
    }
    else
        return CHSTerritory::GetAttributeValue(strName);

    return rval;
}


// Attempts to set the given attribute name to the specified
// value.  If successful, true is returned, otherwise false.
HS_BOOL8 CHSCubicTerritory::SetAttributeValue(char *strName, char *strValue)
{
    HS_FLOAT64 iVal;

    // Match the name
    if (!_stricmp(strName, "MINX"))
    {
        iVal = atof(strValue);
        m_minx = iVal;
        return true;
    }
    else if (!_stricmp(strName, "MINY"))
    {
        iVal = atof(strValue);
        m_miny = iVal;
        return true;
    }
    else if (!_stricmp(strName, "MINZ"))
    {
        iVal = atof(strValue);
        m_minz = iVal;
        return true;
    }
    else if (!_stricmp(strName, "MAXX"))
    {
        iVal = atof(strValue);
        m_maxx = iVal;
        return true;
    }
    else if (!_stricmp(strName, "MAXY"))
    {
        iVal = atof(strValue);
        m_maxy = iVal;
        return true;
    }
    else if (!_stricmp(strName, "MAXZ"))
    {
        iVal = atof(strValue);
        m_maxz = iVal;
        return true;
    }
    else
        return CHSTerritory::SetAttributeValue(strName, strValue);
}

// Returns true if the given point is in this territory.
HS_BOOL8 CHSCubicTerritory::PtInTerritory(int uid,
                                          HS_FLOAT64 x, HS_FLOAT64 y, HS_FLOAT64 z)
{
    // In our universe?
    if (uid != m_uid)
        return false;

    // true will be returned if the point is between the min
    // and max coordinates of us.
    if ((x <= m_maxx) && (x >= m_minx) &&
        (y <= m_maxy) && (y >= m_miny) && (z <= m_maxz) && (z >= m_minz))
        return true;

    return false;
}

// Returns the specified minimum value for the territory.
HS_FLOAT64 CHSCubicTerritory::GetMinX()
{
    return m_minx;
}

// Returns the specified minimum value for the territory.
HS_FLOAT64 CHSCubicTerritory::GetMinY()
{
    return m_miny;
}

// Returns the specified minimum value for the territory.
HS_FLOAT64 CHSCubicTerritory::GetMinZ()
{
    return m_minz;
}

// Returns the specified maximum value for the territory.
HS_FLOAT64 CHSCubicTerritory::GetMaxX()
{
    return m_maxx;
}

// Returns the specified maximum value for the territory.
HS_FLOAT64 CHSCubicTerritory::GetMaxY()
{
    return m_maxy;
}

// Returns the specified maximum value for the territory.
HS_FLOAT64 CHSCubicTerritory::GetMaxZ()
{
    return m_maxz;
}
