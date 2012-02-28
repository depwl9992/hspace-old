// -----------------------------------------------------------------------
//! $Id: hsautopilot.h,v 1.6 2006/04/04 12:56:10 mark Exp $
// -----------------------------------------------------------------------

#if !defined(__HSAUTOPILOT_H__)
#define __HSAUTOPILOT_H__

// Library Includes

// Local Includes
#include "hsobjects.h"
#include "hsship.h"
#include "hsai.h"

// Forward Declarations

// Types
enum CHSAP_MODE
{
    CHSAP_NOTHING = 0,
    CHSAP_CRUISING,
    CHSAP_FLEEING,
    CHSAP_FIGHTING
};

// Constants

// Prototypes

// AutoPilot, Generic system with the ability to control a ship.
class CHSSysAutoPilot:public CHSEngSystem
{
  public:
    CHSSysAutoPilot(void);
        virtual ~CHSSysAutoPilot(void);

    HS_BOOL8 SetAttributeValue(const HS_INT8 * pcAttrName,
                               const HS_INT8 * pcValue);

    void SaveToFile(FILE * fp);

    void GetAttributeList(CHSAttributeList & rlistAttrs);

    HS_BOOL8 GetAttributeValue(const HS_INT8 * pcAttrName,
                               CHSVariant & rvarReturnVal,
                               HS_BOOL8 bAdjusted,
                               HS_BOOL8 bLocalOnly = false);

    void DoCycle(void);
    void SetEngaged(HS_BOOL8);
    void ControlShip(CHSAI_TYPE);
    void SetAI(CHSAI *);
    void SetObj(HS_DBREF);
    void SetMode(CHSAP_MODE);
    CHSAP_MODE GetMode(void);
    void AddHostile(CHSShip *);
    void ClearHostiles(void);
    CHSShip *GetFirstHostile(void);
    CHSShip *GetNextHostile(void);
    HS_BOOL8 IsEngaged(void);
    HS_BOOL8 IsFighting(void);
    HS_BOOL8 IsFleeing(void);
    HS_BOOL8 IsCruising(void);
    HS_BOOL8 IsWaiting(void);
    HS_DBREF GetObj(void);
    HS_DBREF GetOwner(void);
    HS_INT8 *ListHostiles(void);

  protected:

    typedef std::list < CHSShip * >CSTLHostiles;

    // Status of the autopilot
    HS_BOOL8 m_engaged;
    HS_DBREF m_object;

    CHSAP_MODE m_mode;

    CSTLHostiles m_listHostiles;
        CSTLHostiles::iterator m_iterHostiles;

    // Navigation behaviour
    CHSAI *m_navigation;

    // Targetting behaviour
    CHSAI *m_awareness;

    // Aggression behaviour
    CHSAI *m_aggression;

    // Cowardice behaviour
    CHSAI *m_cowardice;

    // Manueaver behaviour
    CHSAI *m_manueaver;

    // Ordnance behaviour
    CHSAI *m_ordnance;
};

#endif // __HSAUTOPILOT_H__
