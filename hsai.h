// -----------------------------------------------------------------------
//! $Id: hsai.h,v 1.3 2006/04/04 12:41:36 mark Exp $
// -----------------------------------------------------------------------


/* hspace artificial intelligence */

/*
Pilot Profiles - Master object of an artificial intelligence.
		 These are used to hold all the values that
		 could affect an AIs performance.

	Awareness  - This provides a function used to determine
		     which of the viable targets the pilot should
		     choose as its target. Typical types would be
		     the guy with tunnel vision, that locks on
		     and follows, or the type that picks the closest
		     target, or the pilot that calculates which
		     of the targets it has the best chance of
		     hitting, and will autoswitch.
		
		1 - Stay with the first target you find until it's gone.
		2 - Switch to whichever target is closest, within a
		    tolerance, weighted towards the current ship.
		3 - Calculate which ship it has the best chance of hitting

	
	Aggression - How readily the pilot will go after a target.
		     Again this is a function which characterizes
		     a certain behavior.
	
	Cowardice  - This function determines if/when the pilot
		     will give up and run.
		     *** Fugure out where to go ***
	
	Manuever   - This function determines how the pilot flies.
		     Some pilots will simply turn towards the enemy,
		     some will try to lead the enemy a bit, while
		     others will perform complex patterns.
	
	Ordnance   - This function determines what weapons to fire
		     based on the particular chance to hit or
		     whatever for that particular cycle. Some pilots
		     will blindly fire all guns at the fire cone
		     while others will take into account damage/refire
		     and only fire light weapons on the fringes and
		     reserve the big guns for sure shots. Call on any
		     cycle that the target is 'in cone'

Flight Plan - Every ship with an AI will be able to program that AI
	      with a simple flight plan, with waypoints and commands
	      execute, if any, when the waypoint is reached. When not
	      engaged in combat, the pilot will fall back on it's
	      flight plan to figure out what to do.

If it has no plan
then it deactivate itself, and depending on whether it
is declared a temp object or not, it could disable itself
and remove the ship automatically.
*/

#ifdef _WIN32
#pragma once
#endif

#if !defined(__HSAI_H__)
#define __HSAI_H__

// Library Includes
#include "hseng.h"

// Local Includes

// Forward Declarations
class CHSAI;
class CHSSysAutoPilot;
class CHSPilot;
class CHSRoster;
class CHSAttr;
extern CHSRoster cRoster;

// Types
typedef void (*CHSAIAction) (CHSSysAutoPilot *, CHSAI *);

enum CHSAI_TYPE
{
    CHSAI_NOTHING = 0,
    CHSAI_NAVIGATION,
    CHSAI_AWARENESS,
    CHSAI_AGGRESSION,
    CHSAI_COWARDICE,
    CHSAI_MANUEAVER,
    CHSAI_ORDNANCE
};

enum CHSATTR_TYPE
{
    CHSATTR_NOEXEC = 0,
    CHSATTR_FUNCTION,
    CHSATTR_TRIGGER
};

// Constants

// Prototypes

class CHSAI
{
  public:
    CHSAI(void);
        CHSAI(const char *, CHSAI_TYPE, CHSAIAction);
       ~CHSAI(void);

    void SetAction(CHSAIAction);
    void SetName(const char *);
    void SetType(CHSAI_TYPE);
    void Perform(CHSSysAutoPilot *);
    HS_INT8 *GetName(void);
    CHSAI_TYPE GetType(void);
    const HS_INT8 *GetTypeName(void);
    void AddAtr(const HS_INT8 *, CHSATTR_TYPE, const HS_INT8 *);
    void DelAtr(const HS_INT8 *);
    CHSAttr *GetAtr(const HS_INT8 *);
  protected:
    typedef std::list < CHSAttr * >CSTLAtrList;

    CSTLAtrList m_listAtr;
        CSTLAtrList::iterator m_iterAtr;

    CHSAttr *GetFirstAtr(void);
    CHSAttr *GetNextAtr(void);

    HS_INT8 *m_name;
    CHSAI_TYPE m_type;
    CHSAIAction m_action;
};

class CHSPilot
{
  public:
    CHSPilot(void);
        CHSPilot(const HS_INT8 *);
       ~CHSPilot(void);

    void SetName(const HS_INT8 *);
    void SetAI(CHSAI *);
    void ClearAI(CHSAI_TYPE);
    CHSAI *GetAI(CHSAI_TYPE);
    HS_INT8 *GetName(void);
  protected:
        HS_INT8 * m_name;
    CHSAI *m_navigation;
    CHSAI *m_awareness;
    CHSAI *m_aggression;
    CHSAI *m_cowardice;
    CHSAI *m_manueaver;
    CHSAI *m_ordnance;
};

class CHSRoster
{
  public:
    CHSRoster(void);
       ~CHSRoster(void);

    void DumpRoster(HS_DBREF);

    HS_UINT32 NumAI(void);
    HS_UINT32 NumPilot(void);

    CHSAI *GetAI(const HS_INT8 *);
    CHSPilot *GetPilot(const HS_INT8 *);
    CHSAI *FindAI(const HS_INT8 *);

    HS_BOOL8 Register(CHSAI *);
    HS_BOOL8 Register(CHSPilot *);
    HS_BOOL8 Unregister(CHSAI *);
    HS_BOOL8 Unregister(CHSPilot *);

    CHSAI *GetFirstAI(void);
    CHSAI *GetNextAI(void);
    CHSPilot *GetFirstPilot(void);
    CHSPilot *GetNextPilot(void);
  protected:
    typedef std::list < CHSAI * >CSTLAIList;
    typedef std::list < CHSPilot * >CSTLPilotList;

        CSTLAIList::iterator m_iterAI;
        CSTLPilotList::iterator m_iterPilot;

    CSTLAIList m_listAI;
    CSTLPilotList m_listPilot;
};

class CHSAttr
{
  public:
    CHSAttr(void);
        CHSAttr(const HS_INT8 *, CHSATTR_TYPE, const HS_INT8 *);
       ~CHSAttr(void);

    CHSATTR_TYPE GetType(void);
    void SetType(CHSATTR_TYPE);

    const HS_INT8 *GetName(void);
    void SetName(const HS_INT8 *);

    const HS_INT8 *GetDefault(void);
    void SetDefault(const HS_INT8 *);

    const HS_INT8 *GetBuffer(void);
    void SetBuffer(const HS_INT8 *);

    const HS_INT8 *GetEnv(HS_UINT32);
    void SetEnv(HS_UINT32, const HS_INT8 *);

    void SaveEnv(void);
    void RestoreEnv(void);

    const HS_INT8 *ToString(void);
    HS_BOOL8 ToBool(void);
    HS_INT32 ToInt32(void);
    HS_DBREF ToDbref(void);
    HS_FLOAT64 ToFloat64(void);

    void Execute(HS_DBREF);
  protected:
        CHSATTR_TYPE m_type;
    HS_INT8 *m_name;
    HS_INT8 *m_env[10];
    HS_INT8 *m_envsave[10];
    HS_INT8 *m_default;
    HS_INT8 *m_buffer;
};

#endif // __HSAI_H__
