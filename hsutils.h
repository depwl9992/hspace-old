// -----------------------------------------------------------------------
// $Id: hsutils.h,v 1.7 2006/04/04 12:41:36 mark Exp $
// -----------------------------------------------------------------------

#ifndef __HSUTILS_INCLUDED__
#define __HSUTILS_INCLUDED__

#include <cstdio>

#define SPACE_ERR	-534234 // Just some random number
#define RADTODEG 	57.296  //  180 / pi

//! Read a string from a file and convert it to an integer
int load_int(FILE * fp);

//! Read a string from a file and convert it to a double
double load_double(FILE * fp);

//! Read string from file and convert to float
float load_float(FILE * fp);

//! Read a string from the file
const char *getstr(FILE * fp);

//! Write a string to the logfile
void hs_log(const char *msg);

//! Pulls a delimited number of words out of a string at a given locaiton
//! and for a given number of delimiters
void extract(char *string, char *into, int start, int num, char delim);

//! convert a dbref of #XXX values and converts it to an integer
int strtodbref(const char *);

//! Calculate a distance between two 3D coordinates
double Dist3D(double x1, double y1, double z1,
              double x2, double y2, double z2);

//! Calculate the angle between two coordinates
int XYAngle(double fx, double fy, double sx, double sy);

//! Return the Z angle between a pair of coordinates
int ZAngle(double fx, double fy, double fz, double sx, double sy, double sz);

//! Give a standard format error message to the specified player
void hsStdError(int player, const char *strMsg);

//! Return true if the calling thread has a lock on the mutex
bool hsEnterMutex();

//! Return true if the calling thread successfully released the muted
bool hsLeaveMutex();

//! Return true if the mutex is successfully initialized 
bool hsInitMutex();

//! Destroys the global mutex
void hsDestroyMutex();

//! Various keys found in our databases used for loading attribute values
enum HS_DBKEY
{
    HSK_NOKEY = 0,
    HSK_BOARDING_CODE,
    HSK_CARGOSIZE,
    HSK_CLASSID,
    HSK_CONSOLE,
    HSK_DBVERSION,
    HSK_DESTROYED,
    HSK_DOCKED,
    HSK_DROP_CAPABLE,
    HSK_DROPPED,
    HSK_HATCH,
    HSK_HULL_POINTS,
    HSK_IDENT,
    HSK_LANDINGLOC,
    HSK_MAXHULL,
    HSK_MAXFUEL,
    HSK_MISSILEBAY,
    HSK_NAME,
    HSK_OBJECTEND,
    HSK_OBJECTTYPE,
    HSK_OBJLOCATION,
    HSK_OBJNUM,
    HSK_PRIORITY,
    HSK_ROLL,
    HSK_ROOM,
    HSK_SIZE,
    HSK_SYSTEMDEF,
    HSK_SYSTEMEND,
    HSK_SYSTYPE,
    HSK_UID,
    HSK_VISIBLE,
    HSK_X,
    HSK_XYHEADING,
    HSK_Y,
    HSK_Z,
    HSK_ZHEADING,
    HSK_DENSITY,
    HSK_SPACEDOCK,
    HSK_MINMANNED,
    HSK_BASESTABILITY,
    HSK_STABILITY,
    HSK_FLUCTUATION,
    HSK_DESTERROR,
    HSK_NODAMAGE,
    HSK_DESTX,
    HSK_DESTY,
    HSK_DESTZ,
    HSK_DESTUID,
    HSK_AGE,
    HSK_SHIELDAFF,
    HSK_TYPENAME
};

//! Lookup a key based on thekey string
HS_DBKEY HSFindKey(char *strKeyName);

// A key/value pair structure

//! Structure for storing keyname / key values
struct HSKEYVALUEPAIR
{
    char *name;
    HS_DBKEY key;
};

#endif
