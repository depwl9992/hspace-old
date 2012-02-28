#ifndef __HSFUNCS_INCLUDED__
#define __HSFUNCS_INCLUDED__

typedef struct
{
    char *name;
    char *(*func) (int, char **);
} HSFUNC;

extern HSFUNC *hsFindFunction(char *);

#define HSPACE_FUNC_PROTO(x)	char *x(int, char **);
#define HSPACE_FUNC_HDR(x)	char *x(int executor, char **args)

HSPACE_FUNC_PROTO(hsfGetMissile)
HSPACE_FUNC_PROTO(hsfGetAttr)
HSPACE_FUNC_PROTO(hsfCalcXYAngle)
HSPACE_FUNC_PROTO(hsfCalcZAngle)
HSPACE_FUNC_PROTO(hsfSpaceMsg)
HSPACE_FUNC_PROTO(hsfGetEngSystems)
HSPACE_FUNC_PROTO(hsfSetAttr)
HSPACE_FUNC_PROTO(hsfGetSensorContacts)
HSPACE_FUNC_PROTO(hsfAddWeapon)
HSPACE_FUNC_PROTO(hsfDelWeapon)
HSPACE_FUNC_PROTO(hsfWeaponAttr)
HSPACE_FUNC_PROTO(hsfSysAttr)
HSPACE_FUNC_PROTO(hsfSysSet)
HSPACE_FUNC_PROTO(hsfDelSys)
HSPACE_FUNC_PROTO(hsfAddSys)
HSPACE_FUNC_PROTO(hsfCommMsg)
HSPACE_FUNC_PROTO(hsfSetMissile)
HSPACE_FUNC_PROTO(hsfClone)
HSPACE_FUNC_PROTO(hsfDecayMessage)
HSPACE_FUNC_PROTO(hsfNearby)
HSPACE_FUNC_PROTO(hsfList)
HSPACE_FUNC_PROTO(hsfExplode)
HSPACE_FUNC_PROTO(hsfConsoleWeaponAttr) 
HSPACE_FUNC_PROTO(hsfConsoleCommand)
#endif // __HSFUNCS_INCLUDED__
