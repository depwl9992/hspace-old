// -----------------------------------------------------------------------
//! $Id: hspace.h,v 1.6 2007/08/19 17:41:27 grapenut_org Exp $
// -----------------------------------------------------------------------

#ifndef __HSPACE_INCLUDED__
#define __HSPACE_INCLUDED__

#include "hstypes.h"

// Message definitions, used when notifying consoles, etc.
enum HS_MESSAGE
{
    MSG_GENERAL = 0,
    MSG_SENSOR,
    MSG_ENGINEERING,
    MSG_COMBAT,
    MSG_COMBAT_DAMAGE,
    MSG_COMMUNICATION,
    NUM_MESSAGE_TYPES
};

//! The CHSpace Object is the master object for all of HSpace.
class CHSpace
{
  public:

    //! Default constructor sets up default parameters
    CHSpace();

    //! Initialize by loading the log, configuration and databases
    void InitSpace(HS_INT32 rebooting = 0);

    //! Save the databases to disk
    void DumpDatabases();

    //! Process a command request sent from the main MUSH server
    void DoCommand(char *, const char *, char *, char *, HS_DBREF);

    //! Process a function request received from the MUSH server
    char *DoFunction(char *, HS_DBREF, char **);

    //! Process a space cycle for all relevant space objects
    void DoCycle();

    //! Enable or disable the cycling of the space system
    void SetCycle(HS_BOOL8 bStart);

    //! Terminate the space system
    void Shutdown();

    //! Get the current cycle time in milliseconds
    inline double CycleTime() 
    {
        return m_cycle_ms;
    }

    //! Is the space system currently active
    inline HS_BOOL8 Active() 
    { 
        return m_cycling; 
    }

  private:

    //Attributes
    HS_BOOL8 m_cycling;
    HS_BOOL8 m_attrs_need_cleaning;
    double m_cycle_ms;
};

extern CHSpace HSpace;

#endif // __HSPACE_INCLUDED__
