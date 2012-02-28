#include "pch.h"

#include <math.h>
#include <cstring>
#include <stdlib.h>

#include "hsobjects.h"
#include "hsinterface.h"
#include "hsutils.h"
#include "hsansi.h"
#include "hsuniverse.h"
#include "hsuniversedb.h"
#include "hsconf.h"
#include "hsflags.h"

extern double d2sin_table[];
extern double d2cos_table[];

CHS3DObject::CHS3DObject():m_bLoading(false)
{
    m_x = 0;
    m_y = 0;
    m_z = 0;

    m_objnum = HSNOTHING;
    m_uid = 0;
    m_visible = true;
    m_size = 1;
    m_name = NULL;
    m_current_speed = 0;        // No speed

    m_type = HST_NOTYPE;

    m_next = m_prev = NULL;
}

CHS3DObject::~CHS3DObject()
{
    if (m_name)
        delete[]m_name;

    if (m_objnum != HSNOTHING)
    {
        // Reset object flag
        if (hsInterface.ValidObject(m_objnum))
            hsInterface.UnsetToggle(m_objnum, THING_HSPACE_OBJECT);
    }

    // Delete landing locations.
    while (!m_listLandingLocs.empty())
    {
        CHSLandingLoc *pLoc = m_listLandingLocs.front();

        delete pLoc;
        m_listLandingLocs.pop_front();
    }
}

const HS_INT8 *CHS3DObject::GetTypeName()
{
    if (mTypeName.size() > 0)
    {
        return mTypeName.c_str();
    }

    return "";
}

void CHS3DObject::SetTypeName(HS_INT8 * newname)
{
    mTypeName = newname;
}



// Indicates whether the object is actively in space or not.
HS_BOOL8 CHS3DObject::IsActive()
{
    // By default, all objects are active
    return true;
}

// Indicates whether the object is visible or not.
HS_BOOL8 CHS3DObject::IsVisible()
{
    return m_visible;
}

// A general message handler for all CHS3DObjects.  It does
// absosmurfly nothing.  If you implement this function in your
// derived class, the long pointer points to whatever data are
// passed in.  It's up to you to know what is being passed.
void CHS3DObject::HandleMessage(const char *lpstrMsg, int msgType, long *data)
{
    // Do nothing.
}

CHSUniverse *CHS3DObject::GetUniverse()
{
    return (CHSUniverseDB::GetInstance().FindUniverse(m_uid));
}

// Can be overridden in derived classes to retrieve
// engineering systems.  Generic CHS3DObjects have no
// engineering systems.
CHSEngSystem *CHS3DObject::GetEngSystem(int type)
{
    return NULL;
}

// Can be overridden in derived classes to retrieve
// the engineering systems array.  Generic CHS3DObjects have
// no engineering systems array.
CHSSystemArray *CHS3DObject::GetEngSystemArray()
{
    return NULL;
}

// Can be overridden in derived classes to retrieve
// a list of engineering system types available on
// the object.  Generic CHS3DObjects have no engineering
// systems.
//
// The return value is the number of systems returned.
HS_UINT32 CHS3DObject::GetEngSystemTypes(int *iBuff)
{
    return 0;
}

// ATTEMPTS to set a given attribute with the specified value.
// If the attribute doesn't exist or the value is invalid, it
// return false.  Otherwise, true.
HS_BOOL8 CHS3DObject::SetAttributeValue(char *strName, char *strValue)
{
    int iVal;

    // Look for the attribute
    if (!strcasecmp(strName, "X"))
    {
        m_x = atof(strValue);
        return true;
    }
    else if (!strcasecmp(strName, "Y"))
    {
        m_y = atof(strValue);
        return true;
    }
    else if (!strcasecmp(strName, "Z"))
    {
        m_z = atof(strValue);
        return true;
    }
    else if (!strcasecmp(strName, "NAME"))
    {
        if (!strValue || !*strValue)
            return false;

        if (m_name)
            delete[]m_name;

        m_name = new char[strlen(strValue) + 1];
        strcpy(m_name, strValue);

        // Change the name of the MUSH object
        if (m_objnum != HSNOTHING)
        {
            hsInterface.SetObjectName(m_objnum, strValue);
        }
        return true;

    }
    else if (!strcasecmp(strName, "UID"))
    {
        iVal = atoi(strValue);

        // Check to see if the universe exists.
        CHSUniverse *uSource;
        CHSUniverse *uDest;

        uDest = CHSUniverseDB::GetInstance().FindUniverse(iVal);
        if (!uDest)
            return false;

        // Grab the source universe
        uSource = GetUniverse();

        // Now pull it from one, and put it in another
        uSource->RemoveObject(this);
        m_uid = iVal;
        return (uDest->AddObject(this));
    }
    else if (!strcasecmp(strName, "TYPENAME"))
    {
        mTypeName = strValue;
        return true;
    }
    else if (!strcasecmp(strName, "SIZE"))
    {
        iVal = atoi(strValue);
        if (iVal < 1)
            return false;

        m_size = iVal;
        return true;
    }
    else if (!strcasecmp(strName, "VISIBLE"))
    {
        iVal = atoi(strValue);
        if (!iVal)
            m_visible = 0;
        else
            m_visible = 1;
        return true;
    }
    else
        return false;
}

void CHS3DObject::GetAttributeList(CHSAttributeList & rlistAttributes)
{
    rlistAttributes.push_back("X");
    rlistAttributes.push_back("Y");
    rlistAttributes.push_back("Z");
    rlistAttributes.push_back("NAME");
    rlistAttributes.push_back("UID");
    rlistAttributes.push_back("SIZE");
    rlistAttributes.push_back("LANDINGLOCS");
    rlistAttributes.push_back("TYPENAME");
    rlistAttributes.push_back("TYPE");
    rlistAttributes.push_back("VISIBLE");
}

// Returns a character value for the requested object
// attribute.  If the attribute is not valid, the string
// contains a single null character.
char *CHS3DObject::GetAttributeValue(char *strName)
{
    static char rval[128];

    *rval = '\0';

    if (!strcasecmp(strName, "X"))
    {
        sprintf(rval, "%.2f", m_x);
    }
    else if (!strcasecmp(strName, "Y"))
    {
        sprintf(rval, "%.2f", m_y);
    }
    else if (!strcasecmp(strName, "Z"))
    {
        sprintf(rval, "%.2f", m_z);
    }
    else if (!strcasecmp(strName, "NAME"))
    {
        strcpy(rval, GetName());
    }
    else if (!strcasecmp(strName, "UID"))
    {
        sprintf(rval, "%d", m_uid);
    }
    else if (!strcasecmp(strName, "SIZE"))
    {
        sprintf(rval, "%d", GetSize());
    }
    else if (!strcasecmp(strName, "TYPENAME"))
    {
        strncpy(rval, mTypeName.c_str(), 127);
    }
    else if(!strcasecmp(strName, "TYPE"))
    {
        sprintf(rval, "%d", GetType());
    }
    else if (!strcasecmp(strName, "LANDINGLOCS"))
    {
        CSTLLandingLocList::iterator iter;
        for (iter = m_listLandingLocs.begin();
             iter != m_listLandingLocs.end(); iter++)
        {
            CHSLandingLoc *pLoc = *iter;

            char cBuf[32];
            if (!*rval)
                sprintf(cBuf, "#%d", pLoc->Object());
            else
                sprintf(cBuf, " #%d", pLoc->Object());

            strcat(rval, cBuf);
        }
    }
    else if (!strcasecmp(strName, "VISIBLE"))
    {
        sprintf(rval, "%d", m_visible);
    }

    else
        return NULL;

    return rval;
}

// This function does nothing at present, but it
// can be overridden in the derived classes.
void CHS3DObject::ClearObjectAttrs()
{
}

// Given a key and a value, the function tries to
// load the value.  If the key is not recognized by the
// object, it returns false.
//
// If you implement the WriteToFile() function, you MUST
// have this function present to handle loading the values
// in the database.  They are in the form of key=value.
HS_BOOL8 CHS3DObject::HandleKey(int key, char *strValue, FILE * fp)
{
    // Determine key, load value
    switch (key)
    {
    case HSK_X:
        m_x = atof(strValue);
        return true;
    case HSK_Y:
        m_y = atof(strValue);
        return true;
    case HSK_Z:
        m_z = atof(strValue);
        return true;
    case HSK_NAME:
        if (m_name)
            delete[]m_name;
        m_name = new char[strlen(strValue) + 1];
        strcpy(m_name, strValue);
        return true;

    case HSK_UID:
        m_uid = atoi(strValue);
        return true;
    case HSK_OBJNUM:
        m_objnum = atoi(strValue);
        return true;

    case HSK_SIZE:
        m_size = atoi(strValue);
        return true;

    case HSK_TYPENAME:
        mTypeName = strValue;
        return true;
        break;

    case HSK_VISIBLE:
        m_visible = atoi(strValue) == 0 ? false : true;
        return true;
    case HSK_LANDINGLOC:
        {
            CHSLandingLoc *pLoc = new CHSLandingLoc;

            if (!pLoc)
            {
                hs_log("ERROR: Couldn't allocate a new landing location.");
            }
            else
            {
                HS_DBREF objnum = atoi(strValue);
                if (!hsInterface.ValidObject(objnum))
                    return true;

                pLoc->SetOwnerObject(this);
                pLoc->LoadFromObject(objnum);
                // Set the planet attribute
                hsInterface.AtrAdd(objnum, "HSDB_OBJECT",
                                   hsInterface.HSPrintf("#%d", m_objnum),
                                   hsInterface.GetGodDbref());

                m_listLandingLocs.push_back(pLoc);
            }
        }
        return true;
    }
    return false;
}

// You can override this function in your base class IF YOU WANT.
// However, if you implement the HandleKey(), that's enough.
// Override it if you want some extra initialization when your
// derived object is loaded from the database.
//
// This function is called by some database loading routine, and passed
// a file pointer which it uses to load variables from the database.
// The variables are in key=value form, and this function just passes
// the key and value to the HandleKey() function.  If you haven't
// overridden it, then the CHS3DObject handles everything that it can.
// If you have overridden the HandleKey(), then the key and values
// are passed to your function first.  You can then pass unhandled
// keys to the base object.
HS_BOOL8 CHS3DObject::LoadFromFile(FILE * fp)
{
    char tbuf[256];
    char strKey[64];
    char strValue[64];
    char *ptr;
    HS_DBKEY key;
    CHSUniverse *uDest;

    if (NULL == fp)
    {
        hs_log("Invalid file pointer passed to CHS3DObject::LoadFromFile()");
        return false;
    }

    m_bLoading = true;

    // Load until end of file or, more likely, an OBJECTEND
    // key.  If we don't find the OBJECTEND key, the object
    // is incomplete.
    while (fgets(tbuf, 256, fp))
    {
        // Strip newlines
        if ((ptr = strchr(tbuf, '\n')) != NULL)
            *ptr = '\0';
        if ((ptr = strchr(tbuf, '\r')) != NULL)
            *ptr = '\0';

        // Extract key/value pairs
        extract(tbuf, strKey, 0, 1, '=');
        extract(tbuf, strValue, 1, 1, '=');


        // Determine key, and handle it.
        key = HSFindKey(strKey);
        if (key == HSK_OBJECTEND)
        {
            // Exit loading
            EndLoadObject();
            break;
        }
        else if (!HandleKey(key, strValue, fp))
        {
            sprintf(tbuf,
                    "WARNING: Object key \"%s\" (%d) found but not handled.",
                    strKey, key);
            hs_log(tbuf);
        }
    }

    // Ensure that object is complete
    if (key != HSK_OBJECTEND)
    {
        m_bLoading = false;
        return false;
    }

    // Check to be sure the objnum is valid.
    if (!hsInterface.ValidObject(m_objnum))
    {
        m_bLoading = false;
        return false;
    }

    // Set flags
    hsInterface.SetToggle(m_objnum, THING_HSPACE_OBJECT);

    // Set type
    hsInterface.AtrAdd(m_objnum, "HSDB_TYPE", hsInterface.HSPrintf("%d",
                                                                   m_type),
                       hsInterface.GetGodDbref());

    // Add to universe
    uDest = GetUniverse();
    if (!uDest)
    {
        sprintf(tbuf, "WARNING: HSpace object #%d has an invalid UID.  \
			Adding to default universe.", m_objnum);
        hs_log(tbuf);

        THSUniverseIterator tIter;

        if (CHSUniverseDB::GetInstance().GetFirstUniverse(tIter))
        {
            m_bLoading = false;
            hs_log("ERROR: Unable to find default universe.");
            return false;
        }

        CHSUniverse *pUniverse = tIter.pValue;
        if (!pUniverse->AddObject(this))
        {
            m_bLoading = false;
            hs_log("ERROR: Failed to add to default universe.");
            return false;
        }

        // Set the new ID of the valid universe.
        m_uid = pUniverse->GetID();
    }
    else
    {
        if (!uDest->AddObject(this))
        {
            sprintf(tbuf,
                    "WARNING: HSpace object #%d was not be added to universe.",
                    m_objnum);
            hs_log(tbuf);
        }
    }

    m_bLoading = false;
    return true;
}

// This function is called by some database saving routine.  If
// you have derived an object from CHS3DObject and have attributes
// that you want written to the database, you MUST override this
// in your derived class.  You must also, then, implement the
// HandleKey() function to handle loading those values.
void CHS3DObject::WriteToFile(FILE * fp)
{
    if (NULL == fp)
    {
        hs_log("Invalid file pointer passed to CHS3DObject::WriteToFile()");
        return;
    }

    // Write attributes to file
    fprintf(fp, "OBJNUM=%d\n", m_objnum);
    if (m_name)
        fprintf(fp, "NAME=%s\n", m_name);
    fprintf(fp, "X=%.2f\n", m_x);
    fprintf(fp, "Y=%.2f\n", m_y);
    fprintf(fp, "Z=%.2f\n", m_z);
    fprintf(fp, "UID=%d\n", m_uid);
    fprintf(fp, "VISIBLE=%d\n", m_visible);
    fprintf(fp, "SIZE=%d\n", m_size);
    if (mTypeName.size())
    {
        fprintf(fp, "TYPENAME=%s\n", mTypeName.c_str());
    }

    // Landing locs
    CSTLLandingLocList::iterator iterLocs;
    for (iterLocs = m_listLandingLocs.begin();
         iterLocs != m_listLandingLocs.end(); iterLocs++)
    {
        CHSLandingLoc *pLoc = *iterLocs;

        fprintf(fp, "LANDINGLOC=%d\n", pLoc->Object());
    }
}

// Returns the name of the object.
char *CHS3DObject::GetName()
{
    if (!m_name)
        return "No Name";

    return m_name;
}

// Sets the name of the object.
void CHS3DObject::SetName(const HS_INT8 * pcName)
{
    if (!pcName)
        return;

    if (m_name)
        delete[]m_name;

    m_name = new char[strlen(pcName) + 1];
    strcpy(m_name, pcName);
}

// This function does nothing, but it can be overridden by 
// derived objects to handle HSpace cycles.  For example, ships
// want to travel, do sensor sweeps, etc.  The generic CHS3DObject
// doesn't have any cyclic stuff to do.
void CHS3DObject::DoCycle()
{
    // Do nothing
}

void CHS3DObject::MoveTowards(double x, double y, double z, float speed)
{
    int zhead;
    int xyhead;

    xyhead = XYAngle(m_x, m_y, x, y);
    zhead = ZAngle(m_x, m_y, m_z, x, y, z);
    if (zhead < 0)
        zhead += 360;

    CHSVector tvec(d2sin_table[xyhead] * d2cos_table[zhead],
                   d2cos_table[xyhead] * d2cos_table[zhead],
                   d2sin_table[zhead]);
    double oldx, oldy, oldz;    // New coords after movement.

    // Save current coords in case we need them.
    oldx = m_x;
    oldy = m_y;
    oldz = m_z;


    // Add to the current position .. heading vector * speed.
    m_x += tvec.i() * speed;
    m_y += tvec.j() * speed;
    m_z += tvec.k() * speed;

}

void CHS3DObject::MoveInDirection(int xyhead, int zhead, float speed)
{
    if (zhead < 0)
        zhead += 360;

    CHSVector tvec(d2sin_table[xyhead] * d2cos_table[zhead],
                   d2cos_table[xyhead] * d2cos_table[zhead],
                   d2sin_table[zhead]);
    double oldx, oldy, oldz;    // New coords after movement.

    // Save current coords in case we need them.
    oldx = m_x;
    oldy = m_y;
    oldz = m_z;


    // Add to the current position .. heading vector * speed.
    m_x += tvec.i() * speed;
    m_y += tvec.j() * speed;
    m_z += tvec.k() * speed;

}

// Returns true if the two objects are equal.  That is to say that
// their object dbrefs are equal.
HS_BOOL8 CHS3DObject::operator ==(CHS3DObject & cRVal)
{
    if (cRVal.GetDbref() == m_objnum)
        return true;

    return false;
}

HS_BOOL8 CHS3DObject::operator ==(CHS3DObject * cRVal)
{
    if (cRVal->GetDbref() == m_objnum)
        return true;

    return false;
}

HS_UINT32 CHS3DObject::GetSize()
{
    return m_size;
}

HS_TYPE CHS3DObject::GetType()
{
    return m_type;
}

double CHS3DObject::GetX()
{
    return m_x;
}

double CHS3DObject::GetY()
{
    return m_y;
}

double CHS3DObject::GetZ()
{
    return m_z;
}

// Returns the ANSI character string used to define the
// color of the object.  Each object should have its own color ..
// or none at all.  Basic objects have no color.  Override this
// member function in your base class to provide a color.
char *CHS3DObject::GetObjectColor()
{
    return ANSI_HILITE;
}

// Returns the character representation of the object.  This can
// be used in maps and whatever.
char CHS3DObject::GetObjectCharacter()
{
    return '.';
}

HS_UINT32 CHS3DObject::GetUID()
{
    return m_uid;
}

HS_DBREF CHS3DObject::GetDbref()
{
    return m_objnum;
}

void CHS3DObject::SetDbref(HS_DBREF objnum)
{
    m_objnum = objnum;

    // Set the type attribute on the object
    if (hsInterface.ValidObject(objnum))
        hsInterface.AtrAdd(objnum, "HSDB_TYPE",
                           hsInterface.HSPrintf("%d", m_type),
                           hsInterface.GetGodDbref());
}

void CHS3DObject::SetUID(HS_UINT32 uid)
{
    m_uid = uid;
}

void CHS3DObject::SetX(double x)
{
    m_x = x;
}

void CHS3DObject::SetY(double y)
{
    m_y = y;
}

void CHS3DObject::SetZ(double z)
{
    m_z = z;
}

// This function returns NULL because the function should
// be implemented specifically on the derived class.
CHSConsole *CHS3DObject::FindConsole(HS_DBREF obj)
{
    return NULL;
}

//
// Linked list functions
//
CHS3DObject *CHS3DObject::GetNext()
{
    return m_next;
}

CHS3DObject *CHS3DObject::GetPrev()
{
    return m_prev;
}

void CHS3DObject::SetNext(CHS3DObject * cObj)
{
    m_next = cObj;
}

void CHS3DObject::SetPrev(CHS3DObject * cObj)
{
    m_prev = cObj;
}

// Override this function in your derived object class to handle
// when another object locks onto it (or unlocks).
void CHS3DObject::HandleLock(CHS3DObject * cLocker, HS_BOOL8 bLocking)
{
    // cLocker is the other object that is locking onto
    // us.
    //
    // bLocking is true if the object is locking.  false,
    // if unlocking.

    // Do nothing
}

// Override this function in your derived object class to
// handle damage from another object that is attacking.
void CHS3DObject::HandleDamage(CHS3DObject * cSource,
                               CHSWeaponData * cWeapon,
                               int strength,
                               CHSConsole * cConsole, int iSysType)
{
    // Strength is the amount of damage inflicted.  It is
    // up to you to handle that damage in some way.

    // You can use cConsole to get the console that
    // fired the shot.

    // iSysType is the type of system preferred to damage.

    hs_log("CHS3DObject::HandleDamage() -- Invalid HandleDamage called on \
            generic object.");
}

// Can be used to explode the object, for whatever reason.
void CHS3DObject::ExplodeMe()
{
    CHSUniverse *uSource;

    // Find my universe
    uSource = GetUniverse();
    if (uSource)
    {
        // Remove me from active space.
        uSource->RemoveActiveObject(this);
    }
}

//! Override this function in your derived class to handle
//! the end of an object loading from the database.  When
//! the loader for the object database encounters the end
//! of the object definition, it'll call this routine to
//! let your object handle any final changes.
void CHS3DObject::EndLoadObject()
{
    // Nothing to do for base objects
}

//! This function can be called by a scanning object to
//! get a scan report.  Because scan reports can be different
//! for each derived type of object, you should override this
//! function to give custom scanning information.  The
//! information should be sent to the player variable
//! supplied.  The id variable indicates whether the object
//! being scanned is on the scanning objects sensors and how
//! well it has been identified.
void CHS3DObject::GiveScanReport(CHS3DObject * cScanner,
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

    // Finish the report
    sprintf(tbuf,
            "%s%s`------------------------------------------------'%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    hsInterface.Notify(player, tbuf);

}

// Returns the motion vector for the object.
CHSVector & CHS3DObject::GetMotionVector()
{
    return m_motion_vector;
}

// Returns the current velocity for the object
int CHS3DObject::GetSpeed()
{
    return m_current_speed;
}


// Adds a landing location (a bay) to the object.
HS_BOOL8 CHS3DObject::AddLandingLoc(HS_DBREF room)
{
    CHSLandingLoc *cLoc;

    // Grab a new landing loc
    cLoc = new CHSLandingLoc;
    if (!cLoc)
    {
        hs_log("ERROR: Failed to allocate a new CHSLandingLoc memory block.");
        return false;
    }

    // Initialize some attrs
    cLoc->Object(room);
    cLoc->SetActive(true);
    cLoc->SetOwnerObject(this);

    m_listLandingLocs.push_back(cLoc);

    // Set the flag
    hsInterface.SetToggle(room, ROOM_HSPACE_LANDINGLOC);

    // Set the planet attribute
    hsInterface.AtrAdd(room, "HSDB_SHIP",
                       hsInterface.HSPrintf("#%d", m_objnum),
                       hsInterface.GetGodDbref());

    return true;
}

HS_BOOL8 CHS3DObject::DelLandingLoc(HS_DBREF dbLocation)
{
    // Find the landing location in question.
    CSTLLandingLocList::iterator iter;
    for (iter = m_listLandingLocs.begin(); iter != m_listLandingLocs.end();
         iter++)
    {
        CHSLandingLoc *pLoc = *iter;

        if (pLoc->Object() == dbLocation)
        {
            // This is the one.
            delete pLoc;
            m_listLandingLocs.erase(iter);
            return true;
        }
    }

    // Never did find that thing.
    return false;
}


// Call this function to find a landing location based on its
// room number.
CHSLandingLoc *CHS3DObject::FindLandingLoc(HS_DBREF objnum)
{
    CSTLLandingLocList::iterator iter;
    for (iter = m_listLandingLocs.begin(); iter != m_listLandingLocs.end();
         iter++)
    {
        CHSLandingLoc *pLoc = *iter;

        if (pLoc->Object() == objnum)
        {
            return pLoc;
        }
    }

    return NULL;
}


// Retrieves a landing location (a bay) given a slot number (0 - n)
CHSLandingLoc *CHS3DObject::GetLandingLoc(int which)
{
    if (which < 0)
        return NULL;

    // Iterate the list until our index matches the slot we're looking for.
    CSTLLandingLocList::iterator iter;
    int idx = 0;
    for (iter = m_listLandingLocs.begin(); iter != m_listLandingLocs.end();
         iter++, idx++)
    {
        CHSLandingLoc *pLoc = *iter;

        if (idx == which)
        {
            return pLoc;
        }
    }

    // Never found the slot we were looking for.
    return NULL;
}

CHSLandingLoc *CHS3DObject::FindLandingLocByName(const HS_INT8 * pcName)
{
    CSTLLandingLocList::iterator iter;
    for (iter = m_listLandingLocs.begin(); iter != m_listLandingLocs.end();
         iter++)
    {
        CHSLandingLoc *pLoc = *iter;

        // Match based on number of characters specified in the input name.
        if (!strncasecmp(pcName,
                         hsInterface.GetName(pLoc->Object()), strlen(pcName)))
        {
            return pLoc;
        }
    }

    // Never found the slot we were looking for.
    return NULL;
}

HS_UINT32 CHS3DObject::GetNumVisibleLandingLocs()
{
    HS_UINT32 uiNumVisible = 0;

    CSTLLandingLocList::iterator iter;
    for (iter = m_listLandingLocs.begin(); iter != m_listLandingLocs.end();
         iter++)
    {
        CHSLandingLoc *pLoc = *iter;

        if (pLoc->IsVisible())
        {
            uiNumVisible++;
        }
    }

    return uiNumVisible;
}

//
// CHSVector implementation
//
CHSVector::CHSVector()
{
    ci = cj = ck = 0.0000000;
}

CHSVector::CHSVector(double ii, double jj, double kk)
{
    ci = ii;
    cj = jj;
    ck = kk;
}

HS_BOOL8 CHSVector::operator ==(CHSVector & vec)
{
    if (vec.ci == ci && vec.cj == cj && vec.ck == ck)
        return true;

    return false;
}

void CHSVector::operator =(CHSVector & vec)
{
    ci = vec.ci;
    cj = vec.cj;
    ck = vec.ck;
}

CHSVector & CHSVector::operator +(CHSVector & vec)
{
    int ti, tj, tk;

    ti = (int) (ci + vec.ci);
    tj = (int) (cj + vec.cj);
    tk = (int) (ck + vec.ck);

    static CHSVector newVec(ti, tj, tk);

    return newVec;
}

double CHSVector::i()
{
    return ci;
}

double CHSVector::j()
{
    return cj;
}

double CHSVector::k()
{
    return ck;
}

void CHSVector::operator +=(CHSVector & vec)
{
    ci += vec.ci;
    cj += vec.cj;
    ck += vec.ck;
}

double CHSVector::DotProduct(CHSVector & vec)
{
    double rval;

    rval = (ci * vec.ci) + (cj * vec.cj) + (ck * vec.ck);
    if (rval > 1)
        rval = 1;
    else if (rval < -1)
        rval = -1;

    return rval;
}
