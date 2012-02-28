#include <cstdio>
#include <stdarg.h>
#include <cstring>
#include <math.h>

#ifdef WIN32
#include <stdlib.h>
#endif

#include "hsconf.h"
#include "hsutils.h"
#include "hsflags.h"
#include "hscmds.h"

extern "C"
{

#ifdef TM3
#include "../api.h"
    extern void link_exit(dbref, dbref, dbref);
#endif

#ifdef PENNMUSH

#include "config.h"
#include "version.h"

#ifdef HASATTRIBUTE
#define __attribute__(_arg_)
#endif

#define hsrestrict	__restrict__

#include "conf.h"
#include "externs.h"
#include "flags.h"
#include "dbdefs.h"
#include "mushdb.h"
#include "flags.h"
#include "attrib.h"
#include "lock.h"
#include "game.h"
#include "match.h"
#include "command.h"
#include "parse.h"
#include "function.h"
#include "ptab.h"
#include "svninfo.h"
#include "version.h"            // necessary for NUMVERSION checking

    typedef enum look_type HS_LOOK_TYPE;

    void free_object(dbref thing); 

    extern int process_expression(char *, char **, char const **,
                                  dbref, dbref, dbref, int, int, NEW_PE_INFO *pe_info);

    extern void clone_locks(dbref player, dbref orig, dbref clone);

#define DECLARE_HSPACE_FUNCTION(name, func) \
  FUNCTION(name) { safe_format(buff, bp, "%s", HSpace.DoFunction(func, executor, args)); }


    struct pennmush_flag_info
    {
        const char *name;
        char letter;
        int type;
        int perms;
        int negate_perms;
    };

    struct pennmush_flag_info hspace_flag_table[] = {
        {THING_HSPACE_SIM, THING_HSPACE_SIM_CHAR, TYPE_THING, F_INTERNAL,
         F_WIZARD},
        {THING_HSPACE_OBJECT, THING_HSPACE_OBJECT_CHAR, TYPE_THING, F_WIZARD,
         F_WIZARD},
        {THING_HSPACE_CONSOLE, THING_HSPACE_CONSOLE_CHAR, TYPE_THING,
         F_WIZARD, F_WIZARD},
        {THING_HSPACE_TERRITORY, THING_HSPACE_TERRITORY_CHAR, TYPE_THING,
         F_WIZARD, F_WIZARD},
        {THING_HSPACE_C_LOCKED, THING_HSPACE_C_LOCKED_CHAR, TYPE_THING,
         F_WIZARD, F_WIZARD},
        {THING_HSPACE_COMM, THING_HSPACE_COMM_CHAR, TYPE_THING, F_WIZARD,
         F_WIZARD},
        {ROOM_HSPACE_LANDINGLOC, ROOM_HSPACE_LANDINGLOC_CHAR, TYPE_ROOM,
         F_WIZARD, F_WIZARD},
        {ROOM_HSPACE_UNIVERSE, ROOM_HSPACE_UNIVERSE_CHAR, TYPE_ROOM, F_WIZARD,
         F_WIZARD},
        {EXIT_HSPACE_HATCH, EXIT_HSPACE_HATCH_CHAR, TYPE_EXIT, F_WIZARD,
         F_WIZARD},
        {PLAYER_HSPACE_ADMIN, PLAYER_HSPACE_ADMIN_CHAR, TYPE_PLAYER, F_WIZARD,
         F_WIZARD},
        {NULL, '0', 0, 0, 0}
    };

#endif // PENNMUSH

}                               // extern "C"

#include "hscopyright.h"
#include "hstypes.h"
#include "hsinterface.h"
#include "hspace.h"

#if defined(TM3) || defined(MUX)
char *wenv[10];
#else
// this will work only for Penn > 1.7.7p35 through 1.8.4p3
#if (NUMVERSION > 1007007035) && (NUMVERSION < 1008004004)
char **wenv = global_eval_context.wenv;
#endif
#endif


#ifdef MUX
extern void do_chzone(dbref, dbref, dbref, int, int, char *, char *);
extern void link_exit(dbref, dbref, dbref);
extern bool check_pass(dbref, const char *);
#endif


// TINYMUSH SUPPORT FUNCTIONS
#if defined(TM3) || defined(MUX)
const int NUM_BUFFS = 4;

#ifdef TM3
const Delim SpaceSep = { ' ' };
#endif

/* ------------------------------------------------------------------------
*/

// Fetch attribute text. Use a revolving queue of LBUFFS
//
char *my_atr_get(dbref obj, const char *atrname)
{
    int atr, aflags;
    dbref aowner;
    char *value;
    char name[SBUF_SIZE];
    static char buff[LBUF_SIZE][NUM_BUFFS];
    static int x = 0;

    x = (x + 1) % NUM_BUFFS;

    // Crashes and burns without this. Dunno why.
    strncpy(name, atrname, SBUF_SIZE - 1);
#ifdef TM3
    int alen;
    atr = mkattr(name);
    value = atr_pget(obj, atr, &aowner, &aflags, &alen);
#endif

#ifdef MUX
    atr = mkattr(GOD, name);
    value = atr_pget(obj, atr, &aowner, &aflags);
#endif

    strncpy(buff[x], value, LBUF_SIZE - 1);
    free_lbuf(value);

    return buff[x];
}

/* ------------------------------------------------------------------------ */

int my_atr_put(dbref obj, const char *atrname, const char *val)
{
    int atr;
    char name[SBUF_SIZE];

    strcpy(name, atrname);
#ifdef TM3
    atr = mkattr(name);
#endif
#ifdef MUX
    atr = mkattr(GOD, name);
#endif

    if (atr <= 0)
        return 0;
    atr_add_raw(obj, atr, (char *) val);
    return 1;
}

/* ------------------------------------------------------------------------ */

void my_atr_clear(dbref obj, const char *atrname)
{
    int atr;
    char name[SBUF_SIZE];

    strcpy(name, atrname);
#ifdef TM3
    atr = mkattr((char *) atrname);
#endif

#ifdef MUX
    atr = mkattr(GOD, (char *) atrname);
#endif

    atr_clr(obj, atr);

}

#endif /* TM3 */



CHSInterface hsInterface;       // One instance of this.

// Adds an attribute with a value to an object.
void CHSInterface::AtrAdd(int obj, const char *atrname, char *val, int owner,
                          int flags)
{
    if (false == ValidObject(obj))
    {
        hs_log(HSPrintf("AtrAdd() invalid object: %d, atr: %s, val: %s",
                        obj, atrname, val));
        return;
    }
#ifdef PENNMUSH                 // No change in code between versions
    atr_add(obj, atrname, val, owner, flags);
#endif

#if defined(TM3) || defined(MUX)
    my_atr_put(obj, atrname, val);
#endif
}

void CHSInterface::AtrDel(int obj, const char *atrname, int owner)
{
    if (false == ValidObject(obj))
    {
        hs_log(HSPrintf("AtrDel() invalid object: %d, atr: %s",
                        obj, atrname));
        return;
    }

#ifdef PENNMUSH                 // No change in code between versions
    atr_clr(obj, atrname, owner);
#endif

#if defined(TM3) || defined(MUX)
    my_atr_clear(obj, atrname);
#endif
}

HS_DBREF CHSInterface::GetGodDbref()
{
    return GOD;
}

// Clones an object and returns the HS_DBREF of the new clone.
HS_DBREF CHSInterface::CloneThing(HS_DBREF model)
{
    HS_DBREF clone;

#ifdef PENNMUSH                 // No change in code between versions
    clone = new_object();

    // Copy the basic information from the model to the clone.
    memcpy(REFDB(clone), REFDB(model), sizeof(struct object));
    Owner(clone) = Owner(model);
    Name(clone) = NULL;

    // NULL-out some memory pointers we didn't really want copied.
    db[clone].list = NULL;

    // Now copy the pointer information.
    atr_cpy(clone, model);
    Locks(clone) = NULL;
    clone_locks(model, model, clone);
    Zone(clone) = Zone(model);
    Parent(clone) = Parent(model);
    Flags(clone) = clone_flag_bitmask("FLAG", Flags(model));
    set_name(clone, Name(model));
    s_Pennies(clone, Pennies(model));

#ifdef CREATION_TIMES
    /*
     * We give the clone the same modification time that its
     * other clone has, but update the creation time
     */
    db[clone].creation_time = time((time_t *) 0);
#endif

    db[clone].contents = db[clone].location = db[clone].next = NOTHING;
#endif

#if defined(TM3) || defined(MUX)
    clone = create_obj(Owner(model), Typeof(model),
                       Name(model), Pennies(model));

    //atr_free(clone);
    s_Name(clone, Name(model));
    s_Pennies(clone, Pennies(model));
    s_Parent(clone, Parent(model));
#ifdef TM3
    atr_cpy(Owner(clone), clone, model);
    s_Flags(clone, Flags(model));
    s_Flags2(clone, Flags2(model));
    s_Flags3(clone, Flags3(model));
#endif

#ifdef MUX
    atr_cpy(clone, model);
    s_Flags(clone, FLAG_WORD1, Flags(model));
    s_Flags(clone, FLAG_WORD2, Flags2(model));
    s_Flags(clone, FLAG_WORD3, Flags3(model));
    s_Home(clone, Home(model));
#endif
#endif

    return clone;
}

// Gets an attribute from an object and stores the value
// in the buffer.  It returns false if the attribute was
// not found.  Otherwise, true.
HS_BOOL8 CHSInterface::AtrGet(int obj, const char *atrname)
{
#ifdef PENNMUSH
    ATTR *a;

    a = atr_get(obj, atrname);
    if (!a)
    {
        return false;
    }

    strcpy_s(m_buffer, atr_value(a));
    return true;
#endif

#if defined(TM3) || defined(MUX)
    char *a;
    a = my_atr_get(obj, atrname);
    if (a && *a)
    {
        strcpy(m_buffer, a);
        return true;
    }
    else
        return false;
#endif
}

int CHSInterface::MaxObjects(void)
{
#ifdef PENNMUSH                 // No change in code between versions
    return db_top;
#endif

#if defined(TM3) || defined(MUX)
    return mudstate.db_top;
#endif
}

#ifdef PENNMUSH 	// Added by Mongo for UnmanConsole in PennMUSH
HS_DBREF CHSInterface::MatchThing(HS_DBREF player, char *name)
{
    HS_DBREF console;

    console = match_result(player, name, TYPE_THING, MAT_NEAR_THINGS);

    if (console == AMBIGUOUS)
        console = NOTHING;

    return console;
}
#endif

HS_DBREF CHSInterface::NoisyMatchThing(HS_DBREF player, char *name)
{
    HS_DBREF console;

#ifdef PENNMUSH                 // No change in code between versions
    console = noisy_match_result(player, name, TYPE_THING, MAT_NEAR_THINGS);
#endif

#if defined(TM3) || defined(MUX)
    console = match_thing(player, name);
#endif

    if (console == AMBIGUOUS)
        console = NOTHING;

    return console;
}

HS_DBREF CHSInterface::NoisyMatchRoom(HS_DBREF player, char *name)
{
    HS_DBREF room;

#ifdef PENNMUSH                 // No change in code between versions
    room = noisy_match_result(player, name, TYPE_ROOM,
                              MAT_ABSOLUTE | MAT_HERE);
#endif

#if defined(TM3) || defined(MUX)
    init_match(player, name, TYPE_ROOM);
    match_absolute();
    match_here();
    room = noisy_match_result();
#endif

    if (room == AMBIGUOUS)
        room = NOTHING;

    return room;
}

// Specifically matches an exit for a given game driver
HS_DBREF CHSInterface::NoisyMatchExit(HS_DBREF player, char *name)
{
    HS_DBREF exit_m;

#ifdef PENNMUSH                 // No change in code between versions
    exit_m = match_result(player, name, TYPE_EXIT, MAT_EXIT);
#endif

#if defined(TM3) || defined(MUX)
    init_match(player, name, TYPE_EXIT);
    match_exit();
    match_absolute();
    exit_m = noisy_match_result();
#endif

    if (exit_m == AMBIGUOUS)
        exit_m = NOTHING;

    return exit_m;
}

#ifdef PENNMUSH
void CHSInterface::SetToggle(int objnum, const char *flag)
#endif
#if defined(TM3) || defined(MUX)
    void CHSInterface::SetToggle(int objnum, int flag)
#endif
{
#ifdef PENNMUSH
    set_flag_internal(objnum, flag);
#endif

#ifdef TM3
    s_Flags3(objnum, Flags3(objnum) | flag);
#endif

#ifdef MUX
    s_Flags(objnum, FLAG_WORD3, Flags3(objnum) | flag);
#endif
}

#ifdef PENNMUSH
void CHSInterface::SetFlag(HS_DBREF dbObject, HS_INT32 iType,
                           HS_BOOL8 bAbsolute)
{
    if (bAbsolute)
    {
        db[dbObject].type = iType;
    }
}

void CHSInterface::SetFlag(HS_DBREF dbObject,
                           const char *iFlag, HS_BOOL8 bAbsolute)
{
    if (bAbsolute)
    {
        Flags(dbObject) = string_to_bits("FLAG", iFlag);
    }
    else
    {
        char buff[BUFFER_LEN];
        strcpy_s(buff,
               bits_to_string("FLAG", Flags(dbObject), dbObject, dbObject));
        strcat_s(buff, " ");
        strcat_s(buff, iFlag);
        Flags(dbObject) = string_to_bits("FLAG", buff);
    }
}

#endif


#ifdef TM3
void CHSInterface::SetFlag(HS_DBREF dbObject,
                           HS_INT32 iFlag, HS_BOOL8 bAbsolute)
{
    if (bAbsolute)
    {
        s_Flags3(dbObject, iFlag);
    }
    else
    {
        s_Flags3(dbObject, Flags3(dbObject) |= iFlag);
    }
}
#endif

#ifdef MUX
void CHSInterface::SetFlag(HS_DBREF dbObject,
                           HS_INT32 iFlag, HS_BOOL8 bAbsolute)
{
    if (bAbsolute)
    {
        s_Flags(dbObject, FLAG_WORD3, iFlag);
    }
    else
    {
        s_Flags(dbObject, FLAG_WORD3, Flags3(dbObject) |= iFlag);
    }
}
#endif

#ifdef PENNMUSH
void CHSInterface::UnsetToggle(int objnum, const char *flag)
#endif
#if defined(TM3) || defined(MUX)
    void CHSInterface::UnsetToggle(int objnum, int flag)
#endif
{
#ifdef PENNMUSH
    clear_flag_internal(objnum, flag);
#endif

#ifdef TM3
    s_Flags3(objnum, Flags3(objnum) & ~flag);
#endif

#ifdef MUX
    s_Flags(objnum, FLAG_WORD3, Flags3(objnum) & ~flag);
#endif
}

// Checks to see if the specified object exists, isn't garbage,
// etc.
HS_BOOL8 CHSInterface::ValidObject(HS_DBREF objnum)
{
#ifdef PENNMUSH                 // No change in code between versions
    if (GoodObject(objnum) && !IsGarbage(objnum))
        return true;
    else
        return false;
#endif

#if defined(TM3) || defined(MUX)
    if (Good_obj(objnum) && !isGarbage(objnum))
        return true;
    else
        return false;
#endif
}

// Returns true or false depending on whether the object has
// the specified flag or not.
#ifdef PENNMUSH
HS_BOOL8 CHSInterface::HasFlag(HS_DBREF objnum, int type, const char *flag)
#endif
#if defined(TM3) || defined(MUX)
    HS_BOOL8 CHSInterface::HasFlag(HS_DBREF objnum, int type, int flag)
#endif
{
#ifdef PENNMUSH
    return (has_flag_by_name(objnum, flag, type) == 0 ? false : true);
#endif

#if defined(TM3) || defined(MUX)

    // Using only Flags3 is sufficient as HSpace does not look at
    // other flags for TM3.  Thus the first two can be skipped.

    if (Typeof(objnum) != type)
        return false;
    //if (Flags(objnum) & flag)
    //   return true;
    //if (Flags2(objnum) & flag)
    //   return true;
    if (Flags3(objnum) & flag)
        return true;
    return false;
#endif

}

// Can be used to set a type of lock on an object
void CHSInterface::SetLock(int objnum, int lockto, HS_LOCKTYPE lock)
{
#ifdef PENNMUSH                 // No change in code between versions
    char tmp[32];
    sprintf_s(tmp, "#%d", lockto);

    switch (lock)
    {
    case LOCK_USE:
        add_lock(GOD, objnum, Use_Lock, parse_boolexp(lockto, tmp, Use_Lock),
                 -1);
        break;
    case LOCK_ZONE:
        add_lock(GOD, objnum, Zone_Lock,
                 parse_boolexp(lockto, tmp, Zone_Lock), -1);
        break;
    case LOCK_NORMAL:
    default:
        break;
    }
#endif

#if defined(TM3) || defined(MUX)
    char tmp[SBUF_SIZE];
    BOOLEXP *key;

    snprintf(tmp, SBUF_SIZE - 1, "#%d", lockto);
    key = parse_boolexp(objnum, tmp, 1);

    switch (lock)
    {
    case LOCK_USE:
        atr_add_raw(objnum, A_LUSE, unparse_boolexp_quiet(objnum, key));
        break;
    case LOCK_ZONE:
        atr_add_raw(objnum, A_LCONTROL, unparse_boolexp_quiet(objnum, key));
    case LOCK_NORMAL:
        break;
    }
    free_boolexp(key);
#endif
}

HS_DBREF CHSInterface::GetLock(int objnum, HS_LOCKTYPE lock)
{
#ifdef PENNMUSH                 // No change in code between versions
    boolexp boolExp = getlock(objnum, Use_Lock);

    if (boolExp == TRUE_BOOLEXP)
    {
        return NOTHING;
    }
    else
    {
        return strtodbref(unparse_boolexp(objnum, boolExp, UB_DBREF));
    }
#endif

#if defined(TM3) || defined(MUX)
    char *value;
    BOOLEXP *key;
    int aowner, aflags;
    dbref lockobj;

#ifdef TM3
    int alen;
    value = atr_get((dbref) objnum, A_LUSE, &aowner, &aflags, &alen);
#else
    value = atr_get((dbref) objnum, A_LUSE, &aowner, &aflags);
#endif
    key = parse_boolexp((dbref) objnum, value, 1);
    free_lbuf(value);
    if (key == TRUE_BOOLEXP)
    {
        free_boolexp(key);
        return NOTHING;
    }
    else
    {
        lockobj = key->thing;
        free_boolexp(key);
        return lockobj;
    }
#endif
}

HS_DBREF CHSInterface::ConsoleUser(int objnum)
{
    HS_DBREF dbUser;

    dbUser = GetLock(objnum, LOCK_USE);

    if (dbUser == HSNOTHING || objnum == HSNOTHING)
        return HSNOTHING;

#ifndef TM3
    if (IsPlayer(dbUser))
#else
    if (isPlayer(dbUser))
#endif
    {
        // If the user is not in the same location as the object,
        // set the lock to the object and return NOTHING.
#ifdef PENNMUSH
        if (Location(dbUser) != Location(objnum)
            || !HasFlag(dbUser, TYPE_PLAYER, PLAYER_CONNECT)
            && IsPlayer(dbUser))
#else
        if (Location(dbUser) != Location(objnum) || !Connected(dbUser)
            && isPlayer(dbUser))
#endif
        {
            SetLock(objnum, objnum, LOCK_USE);


            // Delete attribute from player.
            hsInterface.AtrDel(dbUser, "MCONSOLE", GOD);
#ifdef PENNMUSH
            notify_except(db[Location(objnum)].contents, dbUser,
                          tprintf("%s unmans the %s.", Name(dbUser),
                                  Name(objnum)), 0);
#else
            notify_except(Location(objnum), dbUser, NOTHING,
                          tprintf("%s unmans the %s.", Name(dbUser),
                                  Name(objnum)), 0);
#endif

            return NOTHING;
        }
    }

    return dbUser;
}

// Sends a message to the contents of an object, which is usually
// a room.
void CHSInterface::NotifyContents(int objnum, const HS_INT8 * strMsg)
{
#ifdef PENNMUSH
    notify_except(db[objnum].contents, NOTHING, strMsg, 0);
#endif

#if defined(TM3) || defined(MUX)
    notify_except(objnum, GOD, NOTHING, strMsg, 0);
#endif
}

// Handles getting the contents of an object, which is often
// specific to each game driver.
void CHSInterface::GetContents(int loc, CSTLDbrefList & rlistDbrefs, int type)
{
    int thing;

#ifdef PENNMUSH                 // No change in code between versions
    for (thing = db[loc].contents; GoodObject(thing); thing = Next(thing))
    {
        if (type != NOTYPE)
        {
            if (Typeof(thing) == type)
            {
                rlistDbrefs.push_back(thing);
            }
        }
        else
        {
            rlistDbrefs.push_back(thing);
        }
    }
#endif


#if defined(TM3) || defined(MUX)
    for (thing = Contents(loc); Good_obj(thing); thing = Next(thing))
    {
        if (type != NOTYPE)
        {
            if (Typeof(thing) == type)
                rlistDbrefs.push_back(thing);
        }
        else
            rlistDbrefs.push_back(thing);
    }
#endif
}

// Call this function to see if a given object can
// pass a type of lock on a target object.
HS_BOOL8 CHSInterface::PassesLock(int obj, int target, HS_LOCKTYPE locktype)
{
    int retval = 0;

#ifdef PENNMUSH
    switch (locktype)
    {
    case LOCK_ZONE:
        retval = eval_lock(obj, target, Zone_Lock);
        break;
    case LOCK_USE:
        retval = eval_lock(obj, target, Use_Lock);
        break;
    case LOCK_NORMAL:
    default:
        retval = eval_lock(obj, target, Basic_Lock);
        break;
    }
#endif

#if defined(TM3) || defined(MUX)
    switch (locktype)
    {
    case LOCK_ZONE:
        retval = could_doit(obj, target, A_LCONTROL);
        break;
    case LOCK_USE:
        retval = could_doit(obj, target, A_LUSE);
        break;
    default:
        retval = could_doit(obj, target, A_LOCK);
        break;
    }
#endif

    return retval ? true : false;
}

char *CHSInterface::EvalExpression(char *input, HS_DBREF executor,
                                   HS_DBREF caller, HS_DBREF enactor)
{
#ifdef PENNMUSH
    static char tbuf[BUFFER_LEN];
#else
    static char tbuf[LBUF_SIZE];
#endif

    *tbuf = '\0';

#ifdef PENNMUSH                 // No change in code between versions
    char *bp;
    const char *p;
    //char *rsaves[10];

    bp = tbuf;
    p = input;
    process_expression(tbuf, &bp, &p, executor, caller, enactor,
                       PE_DEFAULT, PT_DEFAULT, NULL);
    *bp = '\0';
#endif


#if defined(TM3) || defined(MUX)
    char *bp, *str;
    bp = tbuf;
    str = input;

#ifdef TM3
    exec(tbuf, &bp, 0, executor, enactor, EV_EVAL,
         &input, wenv, m_uiEnvVariableCount);
#endif

#ifdef MUX
    mux_exec(tbuf, &bp, 0, executor, enactor, EV_EVAL,
             &input, wenv, m_uiEnvVariableCount);

#endif

    *bp = '\0';

#endif

    return tbuf;
}

void CHSInterface::CHZone(HS_DBREF room, HS_DBREF zone)
{
#ifndef TM3
    char zonename[32];
    char roomname[32];
#else
    char zonename[SBUF_SIZE];
    char roomname[SBUF_SIZE];
#endif

#ifdef PENNMUSH                 // No change in code between versions
    sprintf_s(zonename, "#%d", zone);
    sprintf_s(roomname, "#%d", room);
	NEW_PE_INFO *pe_info = make_pe_info("pe_info-hspace_chzone");
    do_chzone(GOD, roomname, zonename, 0, 0, pe_info);
#endif

#if defined(TM3)
    snprintf(zonename, SBUF_SIZE - 1, "#%d", zone);
    snprintf(roomname, SBUF_SIZE - 1, "#%d", room);

    do_chzone(GOD, GOD, 0, roomname, zonename);
#endif
#if defined(MUX)
    snprintf(zonename, SBUF_SIZE - 1, "#%d", zone);
    snprintf(roomname, SBUF_SIZE - 1, "#%d", room);

    do_chzone(GOD, GOD, GOD, 0, 0, roomname, zonename);
#endif

}

HS_BOOL8 CHSInterface::LinkExits(HS_DBREF sexit, HS_DBREF dexit)
{
    if (!sexit || !dexit)
        return false;

#ifdef PENNMUSH
    if (!IsExit(sexit) || !IsExit(dexit))
        return false;

    Location(dexit) = Home(sexit);
    Location(sexit) = Home(dexit);
#endif

#if defined(TM3) || defined(MUX)
    if (!isExit(sexit) || !isExit(dexit))
        return false;
    s_Location(dexit, where_is(sexit));
    s_Location(sexit, where_is(dexit));
#endif

#ifdef TM3
    s_Modified(dexit);
    s_Modified(sexit);
#endif

    return true;
}

HS_BOOL8 CHSInterface::UnlinkExits(HS_DBREF sexit, HS_DBREF dexit)
{
    if (!sexit || !dexit)
        return false;

#ifdef PENNMUSH
    if (!IsExit(sexit) || !IsExit(dexit))
        return false;

    Location(dexit) = NOTHING;
    Location(sexit) = NOTHING;
#endif

#if defined(TM3) || defined(MUX)
    if (!isExit(sexit) || !isExit(dexit))
        return false;
    s_Location(dexit, NOTHING);
    s_Location(sexit, NOTHING);
#endif

    return true;
}

HS_DBREF CHSInterface::GetHome(HS_DBREF dbObject)
{
    if (dbObject)
        return Home(dbObject);

    return NOTHING;
}

const HS_INT8 *CHSInterface::GetName(HS_DBREF dbObject)
{
    if (dbObject != HSNOTHING)
    {
        return Name(dbObject);
    }

    return NULL;
}

#ifndef MUX
extern "C" void hsCommand(char *, const char *, char *, char *, int);
extern "C" HSPACE_COMMAND_PROTO(hscManConsole)
    extern "C" HSPACE_COMMAND_PROTO(hscUnmanConsole)
    extern "C" HSPACE_COMMAND_PROTO(hscBoardShip)
    extern "C" HSPACE_COMMAND_PROTO(hscDisembark)
#else
void hsCommand(char *, const char *, char *, char *, int);
HSPACE_COMMAND_PROTO(hscManConsole)
HSPACE_COMMAND_PROTO(hscUnmanConsole)
HSPACE_COMMAND_PROTO(hscBoardShip) HSPACE_COMMAND_PROTO(hscDisembark)
#endif
#ifdef PENNMUSH                 // No change in code between versions
dbref player;
COMMAND(cmd_hs_man)
{
    hscManConsole(player, arg_left, arg_right);
}
COMMAND(cmd_hs_unman)
{
    hscUnmanConsole(player, arg_left, arg_right);
}
COMMAND(cmd_hs_board)
{
    hscBoardShip(player, arg_left, arg_right);
}
COMMAND(cmd_hs_disembark)
{
    hscDisembark(player, arg_left, arg_right);
}
COMMAND(cmd_hs_space)
{
    hsCommand("@space", switches, arg_left, arg_right, player);
}
COMMAND(cmd_hs_nav)
{
    hsCommand("@nav", switches, arg_left, arg_right, player);
}
COMMAND(cmd_hs_eng)
{
    hsCommand("@eng", switches, arg_left, arg_right, player);
}
COMMAND(cmd_hs_console)
{
    hsCommand("@console", switches, arg_left, arg_right, player);
}

DECLARE_HSPACE_FUNCTION(hs_get_attr, "GETATTR")
DECLARE_HSPACE_FUNCTION(hs_get_missile, "GETMISSILE")
DECLARE_HSPACE_FUNCTION(hs_set_missile, "SETMISSILE")
DECLARE_HSPACE_FUNCTION(hs_xyangle, "XYANG")
DECLARE_HSPACE_FUNCTION(hs_zangle, "ZANG")
DECLARE_HSPACE_FUNCTION(hs_spacemsg, "SPACEMSG")
DECLARE_HSPACE_FUNCTION(hs_eng_sys, "GETENGSYSTEMS")
DECLARE_HSPACE_FUNCTION(hs_set_attr, "SETATTR")
DECLARE_HSPACE_FUNCTION(hs_srep, "SENSORCONTACTS")
DECLARE_HSPACE_FUNCTION(hs_add_weapon, "ADDWEAPON")
DECLARE_HSPACE_FUNCTION(hs_del_weapon, "DELWEAPON")
DECLARE_HSPACE_FUNCTION(hs_weapon_attr, "WEAPONATTR")
DECLARE_HSPACE_FUNCTION(hs_sys_attr, "SYSATTR")
DECLARE_HSPACE_FUNCTION(hs_sysset, "SYSSET")
DECLARE_HSPACE_FUNCTION(hs_delsys, "DELSYS")
DECLARE_HSPACE_FUNCTION(hs_addsys, "ADDSYS")
DECLARE_HSPACE_FUNCTION(hs_comm_msg, "COMMMSG")
DECLARE_HSPACE_FUNCTION(hs_clone, "CLONE")
DECLARE_HSPACE_FUNCTION(hs_decay_msg, "DECAYMSG")
DECLARE_HSPACE_FUNCTION(hs_nearby, "NEARBY")
DECLARE_HSPACE_FUNCTION(hs_list, "LIST")
DECLARE_HSPACE_FUNCTION(hs_explode, "EXPLODE");
DECLARE_HSPACE_FUNCTION(hs_conweap, "CONWEAP");
DECLARE_HSPACE_FUNCTION(hs_command, "COMMAND");

#endif


    void CHSInterface::SetupInterfaceCommands()
{
#ifdef PENNMUSH

#ifdef CAN_TAKE_ARGS_IN_FP
    command_add("man", CMD_T_ANY | CMD_T_SWITCHES | CMD_T_EQSPLIT, 0, 0, NULL,
                cmd_hs_man);
    command_add("unman", CMD_T_ANY | CMD_T_SWITCHES | CMD_T_EQSPLIT, 0, 0,
                NULL, cmd_hs_unman);
    command_add("board", CMD_T_ANY | CMD_T_SWITCHES | CMD_T_EQSPLIT, 0, 0,
                NULL, cmd_hs_board);
    command_add("disembark", CMD_T_ANY | CMD_T_SWITCHES | CMD_T_EQSPLIT, 0, 0,
                NULL, cmd_hs_disembark);
    command_add("@SPACE", CMD_T_ANY | CMD_T_SWITCHES | CMD_T_EQSPLIT, 0, 0,
                NULL, cmd_hs_space);
    command_add("@NAV", CMD_T_ANY | CMD_T_SWITCHES | CMD_T_EQSPLIT, 0, 0,
                NULL, cmd_hs_nav);
    command_add("@CONSOLE", CMD_T_ANY | CMD_T_SWITCHES | CMD_T_EQSPLIT, 0, 0,
                NULL, cmd_hs_console);
    command_add("@ENG", CMD_T_ANY | CMD_T_SWITCHES | CMD_T_EQSPLIT, 0, 0,
                NULL, cmd_hs_eng);
#else
    command_add("man", CMD_T_ANY | CMD_T_SWITCHES | CMD_T_EQSPLIT, 0, 0, NULL,
                cmd_hs_man);
    command_add("unman", CMD_T_ANY | CMD_T_SWITCHES | CMD_T_EQSPLIT, 0, 0,
                NULL,  cmd_hs_unman);
    command_add("board", CMD_T_ANY | CMD_T_SWITCHES | CMD_T_EQSPLIT, 0, 0,
                NULL,  cmd_hs_board);
    command_add("disembark", CMD_T_ANY | CMD_T_SWITCHES | CMD_T_EQSPLIT, 0, 0,
                NULL,  cmd_hs_disembark);
    command_add("@SPACE", CMD_T_ANY | CMD_T_SWITCHES | CMD_T_EQSPLIT, 0, 0,
                NULL,  cmd_hs_space);
    command_add("@NAV", CMD_T_ANY | CMD_T_SWITCHES | CMD_T_EQSPLIT, 0, 0,
                NULL,  cmd_hs_nav);
    command_add("@CONSOLE", CMD_T_ANY | CMD_T_SWITCHES | CMD_T_EQSPLIT, 0, 0,
                NULL,  cmd_hs_console);
    command_add("@ENG", CMD_T_ANY | CMD_T_SWITCHES | CMD_T_EQSPLIT, 0, 0,
                NULL,  cmd_hs_eng);
#endif

    function_add("HS_GET_ATTR", hs_get_attr, 2, 2, FN_REG);
    function_add("HS_GET_MISSILE", hs_get_missile, 2, 2, FN_REG);
    function_add("HS_SET_MISSILE", hs_set_missile, 3, 3, FN_REG);
    function_add("HS_DECAY_MSG", hs_decay_msg, 2, 2, FN_REG);
    function_add("XYANG", hs_xyangle, 4, 4, FN_REG);
    function_add("ZANG", hs_zangle, 6, 6, FN_REG);
    function_add("HS_SPACEMSG", hs_spacemsg, 2, 7, FN_REG);
    function_add("HS_ENG_SYS", hs_eng_sys, 1, 1, FN_REG);
    function_add("HS_SET_ATTR", hs_set_attr, 2, 2, FN_REG);
    function_add("HS_SREP", hs_srep, 2, 2, FN_REG);
    function_add("HS_ADD_WEAPON", hs_add_weapon, 2, 2, FN_REG);
    function_add("HS_DEL_WEAPON", hs_del_weapon, 2, 2, FN_REG);
    function_add("HS_WEAPON_ATTR", hs_weapon_attr, 2, 2, FN_REG);
    function_add("HS_SYS_ATTR", hs_sys_attr, 2, 3, FN_REG);
    function_add("HS_SYSSET", hs_sysset, 2, 2, FN_REG);
    function_add("HS_DELSYS", hs_delsys, 2, 2, FN_REG);
    function_add("HS_ADDSYS", hs_addsys, 2, 2, FN_REG);
    function_add("HS_COMM_MSG", hs_comm_msg, 8, 8, FN_REG);
    function_add("HS_CLONE", hs_clone, 1, 1, FN_REG);
    function_add("HS_NEARBY", hs_nearby, 5, 5, FN_REG);
    function_add("HS_LIST", hs_list, 3, 3, FN_REG);
    function_add("HS_EXPLODE", hs_explode, 6, 6, FN_REG);
    function_add("HS_CONWEAP", hs_conweap, 2, 2, FN_REG);
    function_add("HS_COMMAND", hs_command, 2, 4, FN_REG);

    struct pennmush_flag_info *pFlagInfo;
    for (pFlagInfo = hspace_flag_table; pFlagInfo->name; pFlagInfo++)
    {
        add_flag(pFlagInfo->name, pFlagInfo->letter, pFlagInfo->type,
                 pFlagInfo->perms, pFlagInfo->negate_perms);
    }

#endif

}

void CHSInterface::Notify(HS_DBREF dbObject, const HS_INT8 * pcMessage)
{
#ifdef PENNMUSH                 // No change in code between versions
    notify(dbObject, pcMessage);
#endif

#if defined(TM3) || defined(MUX)
    notify(dbObject, pcMessage);
#endif
}

void CHSInterface::NotifyExcept(HS_DBREF dbFirst,
                                HS_DBREF dbExcept, const HS_INT8 * pcMessage)
{
#ifdef PENNMUSH
    notify_except(dbFirst, dbExcept, pcMessage, 0);
#endif

#if defined(TM3) || defined(MUX)
    notify_except(Location(dbFirst), GOD, dbExcept, pcMessage, 0);
#endif
}

HS_UINT32 CHSInterface::GetRandom(HS_UINT32 uiMaxValPlusOne)
{
    // Avoid div by 0 error.
    if (uiMaxValPlusOne == 0)
    {
        return 0;
    }

    return (rand() % uiMaxValPlusOne);
}

char *CHSInterface::HSPrintf(const HS_INT8 * pcFormat, ...)
{
    va_list args;
    static char buff[HSPACE_MAX_HSPRINTF_LEN];

    va_start(args, pcFormat);
    vsnprintf(buff, HSPACE_MAX_HSPRINTF_LEN, pcFormat, args);

    buff[HSPACE_MAX_HSPRINTF_LEN - 1] = '\0';
    va_end(args);
    return (buff);
}

HS_BOOL8 CHSInterface::IsWizard(HS_DBREF dbWho)
{
    return Wizard(dbWho);
}

HS_UINT32 CHSInterface::GetType(HS_DBREF dbObject)
{
    return Typeof(dbObject);
}

HS_DBREF CHSInterface::GetContents(HS_DBREF dbItem)
{
#ifdef PENNMUSH                 // No change in code between versions
    return db[dbItem].contents;
#endif

#if defined(TM3) || defined(MUX)
    return Contents(dbItem);
#endif
}

HS_DBREF CHSInterface::GetLocation(HS_DBREF dbItem)
{
#ifdef PENNMUSH                 // No change in code between versions
    return Location(dbItem);
#endif

#if defined(TM3) || defined(MUX)
    return Location(dbItem);
#endif
}

void CHSInterface::MoveObject(HS_DBREF dbObject, HS_DBREF dbTo)
{
#ifdef PENNMUSH                 // No change in code between versions
    moveto(dbObject, dbTo, SYSEVENT, "HSpace MoveObject");
#endif

#if defined(TM3) || defined(MUX)
    move_object(dbObject, dbTo);
#endif
}

HS_DBREF CHSInterface::GetFirstExit(HS_DBREF dbRoom)
{
    return Exits(dbRoom);
}

HS_DBREF CHSInterface::GetNextExit(HS_DBREF dbPrevExit)
{
    return Next(dbPrevExit);
}

HS_BOOL8 CHSInterface::ControlsObject(HS_DBREF dbController,
                                      HS_DBREF dbObject)
{
#ifdef PENNMUSH                 // No change in code between versions
    return controls(dbController, dbObject) == 0 ? false : true;
#endif

#if defined(TM3) || defined(MUX)
    return Controls(dbController, dbObject);
#endif
}


HS_BOOL8 CHSInterface::IsNearby(HS_DBREF dbSource, HS_DBREF dbTarget)
{
#ifdef PENNMUSH                 // No change in code between versions
    return nearby(dbSource, dbTarget) == 0 ? false : true;
#endif

#if defined(TM3) || defined(MUX)
    return nearby(dbSource, dbTarget);
#endif
}

HS_BOOL8 CHSInterface::CanSeeAll(HS_DBREF dbObject)
{
    return See_All(dbObject);
}

HS_BOOL8 CHSInterface::EvalCommand(const HS_INT8 * command, HS_DBREF dbPlayer,
                                   HS_DBREF dbCause)
{
#ifdef PENNMUSH                 //
    //char *rsaves[10];

    if (!ValidObject(dbPlayer) || !ValidObject(dbCause) || !command
        || !*command)
        return false;

	//save_env("hsInterface::EvalCommand", rsaves);
	//restore_env("LocalContext", m_registers);
    //parse_que(dbPlayer, command, dbCause, NULL);
    //save_env("LocalContext", m_registers);
    //restore_env("hsInterface::EvalCommand", rsaves);
#endif //PENNMUSH
    return true;
}

void CHSInterface::LookInRoom(HS_DBREF dbPlayer,
                              HS_DBREF dbRoom, HS_INT32 iHow)
{
#ifdef PENNMUSH
	NEW_PE_INFO *pe_info = make_pe_info("pe_info-HSpace_look_in_room");
    look_room(dbPlayer, dbRoom, (HS_LOOK_TYPE) iHow, pe_info);
#endif

#if defined(TM3) || defined(MUX)
    look_in(dbPlayer, dbRoom, iHow);
#endif
}

void CHSInterface::SetObjectName(HS_DBREF dbObject, const HS_INT8 * pcName)
{
#ifdef PENNMUSH                 // No change in code between versions
    set_name(dbObject, pcName);
#endif

#if defined(TM3) || defined(MUX)
    s_Name(dbObject, (char *) pcName);
#endif
}

#ifdef PENNMUSH
HS_DBREF CHSInterface::CreateNewGameObject()
#endif
#if defined(TM3) || defined(MUX)
    HS_DBREF CHSInterface::CreateNewGameObject(int object_type)
#endif
{
#ifdef PENNMUSH                 // No change in code between versions
    HS_DBREF dbObject;
    dbObject = new_object();
    set_name(dbObject, "NewObj");
    Owner(dbObject) = HSCONF.space_wiz;
    Zone(dbObject) = Zone(HSCONF.space_wiz);
    Type(dbObject) = TYPE_THING;
    SetFlag(dbObject, GetGameOption(HSGO_THING_FLAGS));
    return dbObject;
#endif

#if defined(TM3) || defined(MUX)
    return create_obj(GOD, object_type, "NewObj", 0);
#endif

}

void CHSInterface::DestroyObject(HS_DBREF dbObject)
{
#ifdef PENNMUSH
    free_object(dbObject); 
#else
    destroy_obj(dbObject);
#endif

}


void CHSInterface::SetObjectOwner(HS_DBREF dbObject, HS_DBREF dbOwner)
{
#ifdef PENNMUSH                 // No change in code between versions
    Owner(dbObject) = dbOwner;
#endif

#if defined(TM3) || defined(MUX)
    s_Owner(dbObject, dbOwner);
#endif
}

HS_DBREF CHSInterface::GetOwner(HS_DBREF dbObject)
{
    return Owner(dbObject);
}

void CHSInterface::SetObjectParent(HS_DBREF dbObject, HS_DBREF dbParent)
{
#ifdef PENNMUSH                 // No change in code between versions
    Parent(dbObject) = dbParent;
#endif

#if defined(TM3) || defined(MUX)
    s_Parent(dbObject, dbParent);
#endif
}

HS_DBREF CHSInterface::GetParent(HS_DBREF dbObject)
{
#ifdef PENNMUSH                 // No change in code between versions
    return Parent(dbObject);
#endif

#if defined(TM3) || defined(MUX)
    return Parent(dbObject);
#endif

}

HS_DBREF CHSInterface::GetZone(HS_DBREF dbObject)
{
    return Zone(dbObject);
}

void CHSInterface::SetObjectZone(HS_DBREF dbObject, HS_DBREF dbZone)
{
#ifdef PENNMUSH                 // No change in code between versions
    Zone(dbObject) = dbZone;
#endif

#if defined(TM3) || defined(MUX)
    s_Zone(dbObject, dbZone);
#endif
}

#ifdef PENNMUSH
const char *CHSInterface::GetGameOption(EHSGameOption eOption)
#endif
#if defined(TM3) || defined(MUX)
    HS_INT32 CHSInterface::GetGameOption(EHSGameOption eOption)
#endif
{
#if defined(TM3) || defined(MUX)
    // Look at this in TM3 but its done via the creation process
    return 0;
#else
    switch (eOption)
    {
    case HSGO_ROOM_FLAGS:
#ifdef PENNMUSH
        return options.room_flags;
#endif

    case HSGO_EXIT_FLAGS:
#ifdef PENNMUSH
        return options.exit_flags;
#endif

    case HSGO_EXIT_TOGGLES:
#ifdef PENNMUSH
        return options.exit_flags;
#endif

    case HSGO_ROOM_TOGGLES:
#ifdef PENNMUSH
        return options.room_flags;
#endif
    case HSGO_THING_FLAGS:
    case HSGO_THING_TOGGLES:
#ifdef PENNMUSH
        return options.thing_flags;
    case HSGO_INVALID:
    default:
        return 0;
#endif
    }

#endif
    return 0;
}

HS_DBREF CHSInterface::GetFirstContent(HS_DBREF dbObject)
{
    return Contents(dbObject);
}

HS_DBREF CHSInterface::GetNextContent(HS_DBREF dbPrevContent)
{
#ifdef PENNMUSH
    if (!GoodObject(dbPrevContent))
    {
        return HSNOTHING;
    }
    else
    {
        HS_DBREF dbNext = Next(dbPrevContent);

        return GoodObject(dbNext) ? dbNext : HSNOTHING;
    }
#endif // PENNMUSH

#if defined(TM3) || defined(MUX)
    HS_DBREF dbNext;
    if (!Good_obj(dbPrevContent))
    {
        return HSNOTHING;
    }
    else
    {
        dbNext = Next(dbPrevContent);
    }
    return Good_obj(dbNext) ? dbNext : HSNOTHING;
#endif


}

void CHSInterface::LinkExitToRoom(HS_DBREF dbExit, HS_DBREF dbRoom)
{
#ifdef PENNMUSH                 // No change in code between versions
    Exits(dbExit) = dbRoom;
    PUSH(dbExit, Exits(dbRoom));
#endif

#if defined(TM3) || defined(MUX)
    s_Exits(dbRoom, insert_first(Exits(dbRoom), dbExit));
    s_Exits(dbExit, dbRoom);
    s_Location(dbExit, NOTHING);
    link_exit(GOD, dbExit, dbRoom);
#endif

}

void CHSInterface::SetObjectLocation(HS_DBREF dbObject, HS_DBREF dbLocation)
{
#ifdef PENNMUSH                 // No change in code between versions
    Location(dbObject) = dbLocation;
#endif

#if defined(TM3) || defined(MUX)
    s_Location(dbObject, dbLocation);
#endif

}

void CHSInterface::CopyAttributes(HS_DBREF dbFrom, HS_DBREF dbTo)
{
#ifdef PENNMUSH                 // No change in code between versions
    atr_cpy(dbTo, dbFrom);
#endif

#if defined(TM3)
    atr_cpy(GOD, dbTo, dbFrom);
#endif
#if defined(MUX)
    atr_cpy(dbTo, dbFrom);
#endif
}

void CHSInterface::InvokeResponse(HS_DBREF dbCause,
                                  HS_DBREF dbTarget,
                                  const HS_INT8 * pcMessage,
                                  const HS_INT8 * pcOMessage,
                                  const HS_INT8 * pcAction,
                                  HS_DBREF dbLocation)
{
#ifdef PENNMUSH                 // No change in code between versions
    did_it(dbCause, dbTarget, pcMessage, NULL, pcOMessage, NULL, pcAction,
           dbLocation);
#endif

#if defined(TM3) || defined(MUX)
    ATTR *a_atr = NULL, *o_atr = NULL, *aa_atr = NULL;
    int anum, onum, aanum;

    anum = onum = aanum = 0;

    if (pcMessage != NULL)
    {
        a_atr = atr_str((char *) pcMessage);
        if (a_atr != NULL)
            anum = a_atr->number;
    }
    if (pcOMessage != NULL)
    {
        o_atr = atr_str((char *) pcOMessage);
        if (o_atr != NULL)
            onum = o_atr->number;
    }
    if (pcAction != NULL)
    {
        aa_atr = atr_str((char *) pcAction);
        if (aa_atr != NULL)
            aanum = aa_atr->number;
    }

#ifdef TM3
    did_it(dbCause, dbTarget, anum, NULL, onum,
           NULL, aanum, 0, (char **) NULL, 0, 0);
#endif
#ifdef MUX
    did_it(dbCause, dbTarget, anum, NULL, onum,
           NULL, aanum, (char **) NULL, 0);
#endif

#endif
}

#ifdef PENNMUSH
void CHSInterface::BroadcastWithFlags(const char *iFlags,
                                      const char *iToggles,
                                      const char *pcFormat, ...)
#endif
#if defined(TM3) || defined(MUX)
    void CHSInterface::BroadcastWithFlags(HS_INT32 iFlags,
                                          HS_INT32 iToggles,
                                          const HS_INT8 * pcFormat, ...)
#endif
{
    va_list args;

    static char buff[HSPACE_MAX_HSPRINTF_LEN];

    va_start(args, pcFormat);
    vsnprintf(buff, HSPACE_MAX_HSPRINTF_LEN, pcFormat, args);

    buff[HSPACE_MAX_HSPRINTF_LEN - 1] = '\0';
    va_end(args);


#ifdef PENNMUSH
    flag_broadcast(iFlags, iToggles, buff);
#endif
#if defined(TM3) || defined(MUX)
    raw_broadcast(iToggles, buff);
#endif
}

HS_DBREF CHSInterface::LookupPlayer(const HS_INT8 * pcName)
{
#ifdef PENNMUSH                 // No change in code between versions
    return lookup_player(pcName);
#endif

#if defined(TM3) || defined(MUX)
    return lookup_player(GOD, (char *) pcName, 0);
#endif

    return HSNOTHING;
}

HS_BOOL8 CHSInterface::ValidatePlayerPassword(HS_DBREF dbPlayer,
                                              const HS_INT8 * pcPassword)
{
#ifdef PENNMUSH                 // No change in code between versions
    if (password_check(dbPlayer, pcPassword) != 0)
    {
        return true;
    }

#endif

#if defined(TM3) || defined(MUX)
    if (check_pass(dbPlayer, pcPassword) != 0)
    {
        return true;
    }
#endif

    return false;
}


void CHSInterface::SetEnvironmentVariable(HS_INT8 * pcValue)
{

    //wenv[m_uiEnvVariableCount++] = pcValue;
}
