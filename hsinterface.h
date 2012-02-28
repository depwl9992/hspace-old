// -----------------------------------------------------------------------
// $Id: hsinterface.h,v 1.11 2007/08/19 17:41:27 grapenut_org Exp $
// -----------------------------------------------------------------------

#ifndef __HSINTERFACE_INCLUDED__
#define __HSINTERFACE_INCLUDED__

#include <list>

enum HS_LOCKTYPE
{
    LOCK_NORMAL = 0,
    LOCK_USE,
    LOCK_ZONE
};

#ifdef TM3
#include "../api.h"
#endif

#ifdef MUX
#include "../autoconf.h"
#include "../config.h"
#include "../externs.h"
#include "../attrs.h"
#include "../powers.h"
#define IsPlayer isPlayer
#endif


#ifdef PENNMUSH

typedef struct pe_info PE_Info;
#define PE_NOTHING              0
#define PE_COMPRESS_SPACES      0x00000001
#define PE_STRIP_BRACES         0x00000002
#define PE_COMMAND_BRACES       0x00000004
#define PE_EVALUATE             0x00000010
#define PE_FUNCTION_CHECK       0x00000020
#define PE_FUNCTION_MANDATORY   0x00000040
#define PE_LITERAL              0x00000100
#define PE_DOLLAR               0x00000200

#ifndef PE_DEFAULT
#define PE_DEFAULT (PE_COMPRESS_SPACES | PE_STRIP_BRACES | \
                    PE_EVALUATE | PE_FUNCTION_CHECK)
#endif

#define AF_ODARK        0x1U     /* OBSOLETE! Leave here but don't use */
#define AF_INTERNAL     0x2U     /* no one can see it or set it */
#define AF_WIZARD       0x4U     /* Wizard only can change it */
#define AF_NUKED        0x8U     /* OBSOLETE! Leave here but don't use */
#define AF_LOCKED       0x10U    /* Only creator of attrib can change it. */
#define AF_NOPROG       0x20U    /* won't be searched for $ commands. */
#define AF_MDARK        0x40U    /* Only wizards can see it */
#define AF_PRIVATE      0x80U    /* Children don't inherit it */
#define AF_NOCOPY       0x100U   /* atr_cpy (for @clone) doesn't copy it */
#define AF_VISUAL       0x200U   /* Everyone can see this attribute */
#define AF_REGEXP       0x400U   /* Match $/^ patterns using regexps */
#define AF_CASE         0x800U   /* Match $/^ patterns case-sensitive */
#define AF_SAFE         0x1000U  /* This attribute may not be modified */
#define AF_STATIC       0x10000U /* OBSOLETE! Leave here but don't use */
#define AF_COMMAND      0x20000U /* INTERNAL: value starts with $ */
#define AF_LISTEN       0x40000U /* INTERNAL: value starts with ^ */
#define AF_NODUMP       0x80000U /* INTERNAL: attribute is not saved */
#define AF_LISTED       0x100000U        /* INTERNAL: Used in @list attribs */
#define AF_PREFIXMATCH  0x200000U       /* Subject to prefix-matching */
#define AF_VEILED       0x400000U      /* On ex, show presence, not value */

#define PT_NOTHING      0
#define PT_BRACE        0x00000001
#define PT_BRACKET      0x00000002
#define PT_PAREN        0x00000004
#define PT_COMMA        0x00000008
#define PT_SEMI         0x00000010
#define PT_EQUALS       0x00000020
#define PT_SPACE        0x00000040

#define TYPE_ROOM       0x1
#define TYPE_THING      0x2
#define TYPE_EXIT       0x4
#define TYPE_PLAYER     0x8
#define TYPE_GARBAGE    0x10
#define TYPE_MARKED     0x20
#define NOTYPE          0xFFFF

#endif // PENNMUSH

#define HSNOTHING		-1
#define HSPACE_MAX_HSPRINTF_LEN				1024

typedef enum EHSGameOption
{
    HSGO_INVALID = 0,
    HSGO_ROOM_FLAGS,
    HSGO_EXIT_FLAGS,
    HSGO_THING_FLAGS,
    HSGO_EXIT_TOGGLES,
    HSGO_ROOM_TOGGLES,
    HSGO_THING_TOGGLES
};

typedef std::list < HS_DBREF > CSTLDbrefList;


// The CHSInterface class is used to allow HSpace to easily
// port to other types of game drivers.  Any game driver specific
// access methods should go in here whenever possible.
class CHSInterface
{
  public:
    void CHZone(HS_DBREF, HS_DBREF);
    void AtrAdd(HS_INT32, const HS_INT8 *, HS_INT8 *, HS_INT32,
                HS_INT32 flags = HSNOTHING);
    void AtrDel(HS_INT32, const HS_INT8 *, HS_INT32);

#ifdef PENNMUSH
    void SetToggle(HS_INT32, const HS_INT8 *);
    void UnsetToggle(HS_INT32, const HS_INT8 *);
    const char *GetGameOption(EHSGameOption eOption);
    HS_BOOL8 HasFlag(HS_INT32, HS_INT32, const HS_INT8 *);
    void BroadcastWithFlags(const char *iFlags, const char *iToggles,
                            const char *pcFormat, ...);
    void SetFlag(HS_DBREF, const HS_INT8 *, HS_BOOL8 bAbsolute = false);
#else                           // This works for TM3
    void SetToggle(HS_INT32, HS_INT32);
    void UnsetToggle(HS_INT32, HS_INT32);
    HS_INT32 GetGameOption(EHSGameOption eOption);
    HS_BOOL8 HasFlag(HS_INT32, HS_INT32, HS_INT32);
    void BroadcastWithFlags(HS_INT32 iFlags, HS_INT32 iToggles,
                            const HS_INT8 * pcFormat, ...);
#endif

    void SetFlag(HS_DBREF, HS_INT32, HS_BOOL8 bAbsolute = false);
    void SetLock(HS_INT32, HS_INT32, HS_LOCKTYPE);      // Sets an object lock
    void GetContents(HS_INT32, CSTLDbrefList &, HS_INT32);
    void SetupInterfaceCommands();
    void NotifyContents(HS_INT32, const HS_INT8 *);
    void Notify(HS_DBREF, const HS_INT8 *);
    void NotifyExcept(HS_DBREF, HS_DBREF, const HS_INT8 *);
    void MoveObject(HS_DBREF, HS_DBREF);

    HS_UINT32 GetRandom(HS_UINT32 uiMaxValPlusOne);
    HS_UINT32 GetType(HS_DBREF);

    HS_INT32 MaxObjects(void);

    HS_DBREF CloneThing(HS_DBREF);
    HS_DBREF NoisyMatchThing(HS_DBREF, HS_INT8 *);
#ifdef PENNMUSH //  Added by Mongo For New UnmanConsole functionality in PennMUSH
	HS_DBREF MatchThing(HS_DBREF, HS_INT8 *);
#endif
    HS_DBREF NoisyMatchRoom(HS_DBREF, HS_INT8 *);
    HS_DBREF NoisyMatchExit(HS_DBREF, HS_INT8 *);
    HS_DBREF GetLock(HS_INT32, HS_LOCKTYPE);
    HS_DBREF ConsoleUser(HS_INT32);
    HS_DBREF GetHome(HS_DBREF);
    HS_DBREF GetGodDbref();
    HS_DBREF GetContents(HS_DBREF);
    HS_DBREF GetLocation(HS_DBREF);
#ifdef PENNMUSH
    HS_DBREF CreateNewGameObject();
#else
    HS_DBREF CreateNewGameObject(int object_type);
#endif
    void DestroyObject(HS_DBREF);
    HS_DBREF GetOwner(HS_DBREF);
    HS_DBREF GetFirstExit(HS_DBREF);
    HS_DBREF GetNextExit(HS_DBREF);
    HS_DBREF GetParent(HS_DBREF);
    HS_DBREF GetZone(HS_DBREF);
    HS_DBREF GetFirstContent(HS_DBREF);
    HS_DBREF GetNextContent(HS_DBREF);
    HS_DBREF LookupPlayer(const HS_INT8 * pcName);

    char *HSPrintf(const HS_INT8 * pcFormat, ...);

    HS_BOOL8 ValidatePlayerPassword(HS_DBREF, const HS_INT8 * pcPassword);
    HS_BOOL8 PassesLock(HS_INT32, HS_INT32, HS_LOCKTYPE);
    HS_BOOL8 ValidObject(HS_DBREF);
    HS_BOOL8 AtrGet(HS_INT32, const HS_INT8 *);
    HS_BOOL8 LinkExits(HS_DBREF, HS_DBREF);
    HS_BOOL8 UnlinkExits(HS_DBREF, HS_DBREF);
    HS_BOOL8 ControlsObject(HS_DBREF, HS_DBREF);
    HS_BOOL8 IsWizard(HS_DBREF);
    HS_BOOL8 IsNearby(HS_DBREF, HS_DBREF);
    HS_BOOL8 CanSeeAll(HS_DBREF);
    HS_BOOL8 EvalCommand(const HS_INT8 *, HS_DBREF, HS_DBREF);

    HS_INT8 *m_registers[10];
    HS_INT8 *EvalExpression(HS_INT8 *, HS_INT32, HS_INT32, HS_INT32);
    HS_INT8 m_buffer[4096];
    const HS_INT8 *GetName(HS_DBREF);

    void LookInRoom(HS_DBREF, HS_DBREF, HS_INT32);
    void SetObjectName(HS_DBREF, const HS_INT8 *);
    void SetObjectOwner(HS_DBREF, HS_DBREF);
    void SetObjectParent(HS_DBREF, HS_DBREF);
    void SetObjectZone(HS_DBREF, HS_DBREF);
    void LinkExitToRoom(HS_DBREF dbExit, HS_DBREF dbRoom);
    void SetObjectLocation(HS_DBREF, HS_DBREF);
    void CopyAttributes(HS_DBREF, HS_DBREF);
    void InvokeResponse(HS_DBREF dbCause,
                        HS_DBREF dbTarget,
                        const HS_INT8 * pcMessage,
                        const HS_INT8 * pcOMessage,
                        const HS_INT8 * pcAction,
                        HS_DBREF dbLocation = HSNOTHING);

    inline void ClearEnvironmentVariables()
    {
        m_uiEnvVariableCount = 0;
    }

    void SetEnvironmentVariable(HS_INT8 * pcValue);

  private:

    HS_UINT32 m_uiEnvVariableCount;
};

extern CHSInterface hsInterface;

#endif // __HSINTERFACE_INCLUDED__
