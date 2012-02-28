// -----------------------------------------------------------------------
// $Id: hsdamagecontrol.h,v 1.8 2006/04/26 23:53:15 mark Exp $
// -----------------------------------------------------------------------

#if !defined(__HSDAMAGECONTROL_H__)
#define __HSDAMAGECONTROL_H__

// Library Includes
#include "hseng.h"
#include "hscopyright.h"
#include <vector>

//! Damage control engineering system
class CHSDamCon:public CHSEngSystem
{
  public:
    //! Default constructors, set crew information to NULL
    CHSDamCon();

    //! Default deconstructor, does nothing
    virtual ~CHSDamCon();

    //! Set the specified attribute to the given value
    HS_BOOL8 SetAttributeValue(const HS_INT8 * pcAttrName,
                               const HS_INT8 * pcValue);

    //! Save damage control system data to the file pointer
    void SaveToFile(FILE * fp);

    //! Process a single cycle for the damage control
    void DoCycle();

    //! @brief Assign the specified crew to the given system
    //! @param player - enacting player
    //! @param iCrew - crew id to assign
    //! @param pSystem - engineering system to be repaired
    void AssignCrew(HS_DBREF player, int iCrew, CHSEngSystem * pSystem);

    //! @brief Assign the specified crew to the given system
    //! @param player - enacting player
    //! @param iCrew - crew id to assign
    void UnassignCrew(HS_DBREF player, int iCrew);

    //! Get the current list of manipulatable attributes on this system
    void GetAttributeList(CHSAttributeList & rlistAttrs);

    //! Get the current value of the specified attribute
    HS_BOOL8 GetAttributeValue(const HS_INT8 * pcAttrName,
                               CHSVariant & rvarReturnVal,
                               HS_BOOL8 bAdjusted,
                               HS_BOOL8 bLocalOnly = false);

    //! Return the number of current damage control crews
    HS_UINT32 GetNumCrews();

    //! Return the number of cycles remaining for the given repair crew
    int GetCyclesLeft(int crew);

    //! Return the system the specified crew is repairing
    CHSEngSystem *GetWorkingCrew(int crew);

    //! Set the parent system for this damange crew system
    void SetParentSystem(CHSEngSystem * pParent);


    //! Set the number of available damage crews
    void SetNumCrews(int iValue)
    {
        if (!m_puiNumCrews)
        {
            m_puiNumCrews = new HS_UINT32;
        }

           *m_puiNumCrews = iValue;
    }

    //! Return the pointer to the raw number of crews 
    HS_UINT32 *GetRawNumCrews()
    {
        return m_puiNumCrews;
    }

  protected:

    //! The damage crew storage structure
    struct THSDamageCrew
    {
        //! engineering system being repaired
        CHSEngSystem *pAssignedTo;
        //! Number of seconds remaining to repair the system
        int iSecondsLeft;

            THSDamageCrew(): pAssignedTo(NULL), iSecondsLeft(0)
        {
        }
    };

    //! Pointer to member for the number of crews
    HS_UINT32 *m_puiNumCrews;

    //! Damage crew vector
    std::vector < THSDamageCrew > m_vecCrews;
};

#endif // __HSDAMAGECONTROL_H__
