// -----------------------------------------------------------------------
//! $Id: hstypes.h,v 1.4 2006/04/04 12:41:36 mark Exp $
// -----------------------------------------------------------------------

#ifndef __HSTYPES_INCLUDED__
#define __HSTYPES_INCLUDED__

#include <list>
#include <string>

typedef char HS_INT8;
typedef unsigned char HS_UINT8;
typedef bool HS_BOOL8;
typedef unsigned short HS_UINT16;
typedef short HS_INT16;
typedef unsigned int HS_UINT32;
typedef int HS_INT32;
typedef float HS_FLOAT32;
typedef double HS_FLOAT64;
typedef int HS_DBREF;

typedef std::list < std::string > CHSAttributeList;

#ifdef WIN32
#ifndef strcasecmp
#define strcasecmp		stricmp
#define strncasecmp		strnicmp
#endif
#endif

const int HS_BUF_64 = 64;
const int HS_BUF_128 = 128;
const int HS_BUF_256 = 256;
const int HS_BUF_512 = 512;
const int HS_BUF_1024 = 1024;

const int HS_BUF_64_CPY = HS_BUF_64 - 1;
const int HS_BUF_128_CPY = HS_BUF_128 - 1;
const int HS_BUF_256_CPY = HS_BUF_256 - 1;
const int HS_BUF_512_CPY = HS_BUF_512 - 1;
const int HS_BUF_1024_CPY = HS_BUF_1024 - 1;

#endif // __HSTYPES_INCLUDED__
