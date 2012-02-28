// -----------------------------------------------------------------------
// $Id: hsutils.cpp,v 1.11 2006/04/22 22:13:00 mark Exp $
// -----------------------------------------------------------------------

#include "pch.h"

#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <cstring>

#include "hsutils.h"
#include "hsansi.h"
#include "hsinterface.h"


#ifdef WIN32
CRITICAL_SECTION g_tHSGlobalMutex;
#else
#include <pthread.h>
pthread_mutex_t g_tHSGlobalMutex;
#endif

bool g_bHSGlobalMutexInitialized = 0;

FILE *spacelog_fp;

int hs_num_keys = -1;

// Key/Value pairs we expect to find in the various
// databases for HSpace.
//
// IF YOU ADD ANYTHING HERE, you need to add it in alphabetical
// order.  The HSFindKey() function uses a binary search algorithm.
struct HSKEYVALUEPAIR hsdbkeypairs[] = {
    {"AGE", HSK_AGE},
    {"BASESTABILITY", HSK_BASESTABILITY},
    {"BOARDING_CODE", HSK_BOARDING_CODE},
    {"CARGOSIZE", HSK_CARGOSIZE},
    {"CLASSID", HSK_CLASSID},
    {"CONSOLE", HSK_CONSOLE},
    {"DBVERSION", HSK_DBVERSION},
    {"DENSITY", HSK_DENSITY},
    {"DESTERROR", HSK_DESTERROR},
    {"DESTROYED", HSK_DESTROYED},
    {"DESTUID", HSK_DESTUID},
    {"DESTX", HSK_DESTX},
    {"DESTY", HSK_DESTY},
    {"DESTZ", HSK_DESTZ},
    {"DOCKED", HSK_DOCKED},
    {"DROP CAPABLE", HSK_DROP_CAPABLE},
    {"DROPPED", HSK_DROPPED},
    {"FLUCTUATION", HSK_FLUCTUATION},
    {"HATCH", HSK_HATCH},
    {"HULL_POINTS", HSK_HULL_POINTS},
    {"IDENT", HSK_IDENT},
    {"LANDINGLOC", HSK_LANDINGLOC},
    {"MAXFUEL", HSK_MAXFUEL},
    {"MAXHULL", HSK_MAXHULL},
    {"MINMANNED", HSK_MINMANNED},
    {"MISSILEBAY", HSK_MISSILEBAY},
    {"NAME", HSK_NAME},
    {"NODAMAGE", HSK_NODAMAGE},
    {"OBJECTEND", HSK_OBJECTEND},
    {"OBJECTTYPE", HSK_OBJECTTYPE},
    {"OBJLOCATION", HSK_OBJLOCATION},
    {"OBJNUM", HSK_OBJNUM},
    {"PRIORITY", HSK_PRIORITY},
    {"ROLL", HSK_ROLL},
    {"ROOM", HSK_ROOM},
    {"SHIELDAFF", HSK_SHIELDAFF},
    {"SIZE", HSK_SIZE},
    {"SPACEDOCK", HSK_SPACEDOCK},
    {"STABILITY", HSK_STABILITY},
    {"SYSTEMDEF", HSK_SYSTEMDEF},
    {"SYSTEMEND", HSK_SYSTEMEND},
    {"SYSTYPE", HSK_SYSTYPE},
    {"TYPENAME", HSK_TYPENAME},
    {"UID", HSK_UID},
    {"VISIBLE", HSK_VISIBLE},
    {"X", HSK_X},
    {"XYHEADING", HSK_XYHEADING},
    {"Y", HSK_Y},
    {"Z", HSK_Z},
    {"ZHEADING", HSK_ZHEADING},
    { NULL }
};

int load_int(FILE * fp)
{
    char word[512];

    if (!fgets(word, 512, fp))
        return SPACE_ERR;
    else
        return atoi(word);
}

float load_float(FILE * fp)
{
    char word[512];

    if (!fgets(word, 512, fp))
        return SPACE_ERR;
    else
        return (float) atof(word);
}


double load_double(FILE * fp)
{
    char word[512];

    if (!fgets(word, 512, fp))
        return SPACE_ERR;
    else
        return atof(word);
}

const char *getstr(FILE * fp)
{
    static char word[4096];
    char *ptr;

    fgets(word, 4096, fp);

    // Strip newlines
    if ((ptr = strchr(word, '\n')) != NULL)
        *ptr = '\0';
    if ((ptr = strchr(word, '\r')) != NULL)
        *ptr = '\0';

    return word;
}

void hs_log(const char *str)
{
    time_t tt;
    struct tm *ttm;
    char timebuf[256];

    if (NULL == spacelog_fp)
    {
        return;
    }

    tt = time(NULL);
    ttm = localtime(&tt);

    sprintf_s(timebuf, "%d%d/%d%d %d%d:%d%d:%d%d",
            (((ttm->tm_mon) + 1) / 10), (((ttm->tm_mon) + 1) % 10),
            (ttm->tm_mday / 10), (ttm->tm_mday % 10),
            (ttm->tm_hour / 10), (ttm->tm_hour % 10),
            (ttm->tm_min / 10), (ttm->tm_min % 10),
            (ttm->tm_sec / 10), (ttm->tm_sec % 10));

    fprintf(spacelog_fp, "%s: %s\n", timebuf, str);
    fflush(spacelog_fp);
}



/*
 * Pulls a delimited number of words out of a string at 
 * a given location and for a given number of delimiters.
 */
void extract(char *string, char *into, int start, int num, char delim)
{
    HS_INT32 uDelimsEncountered;
    char *dptr;
    char *sptr;

    uDelimsEncountered = 0;
    dptr = into;
    *dptr = '\0';
    for (sptr = string; *sptr; sptr++)
    {
        if (*sptr == delim)
        {
            uDelimsEncountered++;
            if ((uDelimsEncountered - start) == num)
            {
                *dptr = '\0';
                return;
            }
            else if (uDelimsEncountered > start)
                *dptr++ = delim;
        }
        else
        {
            if (uDelimsEncountered >= start)
                *dptr++ = *sptr;
        }
    }
    *dptr = '\0';
}

// Takes a string and strips off the leading # sign, turning
// the rest into an integer(dbref)
int strtodbref(const char *string)
{
    char *ptr;

    ptr = (char *) string;

    if (*ptr == '#')
        ptr++;

    return atoi(ptr);
}

// Looks for a key based on the key string.  Since the key
// strings are sorted, we can use some intelligent searching here.
HS_DBKEY HSFindKey(char *strKeyName)
{
    int top, middle, bottom;
    int ival;

    // If hs_num_keys is -1, then we have to initialize it.
    if (hs_num_keys < 0)
    {
        for (ival = 0; hsdbkeypairs[ival].name; ival++)
        {
        };
        hs_num_keys = ival;
    }

    bottom = 0;
    top = hs_num_keys;

    // Look for the key name
    while (bottom < top)
    {
        // Split the array in two
        middle = ((top - bottom) / 2) + bottom;

        // Check to see if the middle is the match
        ival = _stricmp(hsdbkeypairs[middle].name, strKeyName);
        if (!ival)
            return (hsdbkeypairs[middle].key);

        // Not a match, so check to see where we go next.
        // Down or up in the array.
        if (ival > 0)
        {
            // Lower portion
            // Check to see if it's redundant
            if (top == middle)
                break;

            top = middle;
        }
        else
        {
            // Upper portion
            // Check to see if it's redundant
            if (bottom == middle)
                break;

            bottom = middle;
        }
    }
    return HSK_NOKEY;
}

// Returns the 3D distance between two sets of coordinates
double Dist3D(double x1, double y1, double z1,
              double x2, double y2, double z2)
{
    double dist;

    dist = ((x2 - x1) * (x2 - x1)) +
        ((y2 - y1) * (y2 - y1)) + ((z2 - z1) * (z2 - z1));

    return sqrt(dist);
}

// Returns the Z angle between the given coordinates.  Six points
// are needed for this operation.
int ZAngle(double fx, double fy, double fz, double sx, double sy, double sz)
{
    char tbuf[8];               // Used to round the number

    // use atan2, not atan, and remove a LOT of special case code, and guarantee
    // a more accurate result when very close to the axes

    double angle = 0.0;
    angle = RADTODEG * atan2
        ((sz - fz), sqrt(((sx - fx) * (sx - fx)) + ((sy - fy) * (sy - fy))));

    _snprintf_s(tbuf, 7, "%0.f", angle);

    return atoi(tbuf);
}

// Returns the XY angle between the given coordinates.  Four points
// are needed for this operation.
int XYAngle(double fx, double fy, double sx, double sy)
{
    char tbuf[8];

    // recoded to use atan2, not atan, remove a LOT of special case 
    // code, and guarantee a more accurate result when very close to 
    // the axes, as well as removing format inconsistencies in some 
    // edge cases

    _snprintf_s(tbuf, 7, "%.0f", 360 + (RADTODEG * atan2(sx - fx, sy - fy)));
    return (atoi(tbuf) % 360);
}

// Gives a standard format error message to a player.
void hsStdError(int player, const char *strMsg)
{
    char *strNewMsg;

    strNewMsg = new char[strlen(strMsg) + 32];
    sprintf(strNewMsg,
            "%s%s-%s %s", ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, strMsg);
    hsInterface.Notify(player, strNewMsg);
    delete[]strNewMsg;
}

// Returns true if everything worked or false if not.
bool hsInitMutex()
{
    if (g_bHSGlobalMutexInitialized == true)
    {
        return false;
    }

#ifdef WIN32
    InitializeCriticalSection(&g_tHSGlobalMutex);
#else
    if (pthread_mutex_init(&g_tHSGlobalMutex, NULL) != 0)
    {
        return false;
    }
#endif

    g_bHSGlobalMutexInitialized = true;

    return true;
}

// Returns true if the calling thread has a lock on the mutex.
bool hsEnterMutex()
{
    if (!g_bHSGlobalMutexInitialized)
    {
        return false;
    }

#ifdef WIN32
    EnterCriticalSection(&g_tHSGlobalMutex);
    return true;
#else
    if (pthread_mutex_lock(&g_tHSGlobalMutex) == 0)
    {
        return true;
    }
#endif

    return false;
}

// Returns true if the calling thread successfully unlocked the mutex.
bool hsLeaveMutex()
{
    if (!g_bHSGlobalMutexInitialized)
    {
        return false;
    }

#ifdef WIN32
    LeaveCriticalSection(&g_tHSGlobalMutex);
    return true;
#else
    if (pthread_mutex_unlock(&g_tHSGlobalMutex) == 0)
    {
        return true;
    }
#endif

    return false;
}

// Destroys the global mutex.
void hsDestroyMutex()
{
    if (!g_bHSGlobalMutexInitialized)
    {
        return;
    }

#ifdef WIN32
    DeleteCriticalSection(&g_tHSGlobalMutex);
#else
    pthread_mutex_destroy(&g_tHSGlobalMutex);
#endif
}
