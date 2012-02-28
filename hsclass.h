// -----------------------------------------------------------------------
// $Id: hsclass.h,v 1.11 2006/06/17 03:31:48 mark Exp $
// -----------------------------------------------------------------------

#ifndef __HSCLASS_INCLUDED__
#define __HSCLASS_INCLUDED__

#include <string>
#include <map>

#include "HSSingleton.h"

// Forward declarations
class CHSSystemArray;


//! @brief The container class for storing base ship class information
//! used as defaults for ship capabilities
class CHSShipClass
{
  public:

    //! Default constructor, setup baseline parameters
  CHSShipClass(HS_UINT32 uiClassID = 0):
    m_pSystems(NULL),
        m_class_id(uiClassID),
        m_size(1),
        m_cargo_size(0),
        m_minmanned(0), m_maxhull(1), m_can_drop(false), m_spacedock(false)
    {
    }

    virtual ~CHSShipClass()
    {
    }


    //! Return the id for this class
    HS_UINT32 Id() const
    {
        return m_class_id;
    }
    //! Set the id for this class
    void Id(HS_UINT32 val)
    {
        m_class_id = val;
    }

    //! Return the default object size for this class
    HS_UINT32 Size() const
    {
        return m_size;
    }
    //! Set the object size for this class
    void Size(HS_UINT32 val)
    {
        m_size = val;
    }

    //! Return the default cargo size for this class
    HS_UINT32 CargoSize() const
    {
        return m_cargo_size;
    }
    //! Set the Cargo size for this class
    void CargoSize(HS_UINT32 val)
    {
        m_cargo_size = val;
    }

    //! Return the minimum crew count for this class
    HS_UINT32 MinCrew() const
    {
        return m_minmanned;
    }
    //! Set the minimum crew count for this class
    void MinCrew(HS_UINT32 val)
    {
        m_minmanned = val;
    }

    //! Return the hull point rating for this class
    HS_UINT32 MaxHull() const
    {
        return m_maxhull;
    }
    //! Set the hull point rating for the class
    void MaxHull(HS_UINT32 val)
    {
        m_maxhull = val;
    }

    //! Return the class name
    const HS_INT8* ClassName()const
    {
        return m_strClassName.c_str();
    }
    //! Set the class name
    void ClassName(std::string val)
    {
        m_strClassName = val;
    }

    //! Can this vessel drop to the surface of a planet?
    HS_BOOL8 CanDrop() const
    {
        return m_can_drop;
    }
    //! Set if the ability can drop to a planetary surface
    void CanDrop(HS_BOOL8 val)
    {
        m_can_drop = val;
    }

    //! Is this vessel capable of operating as a space dock
    HS_BOOL8 SpaceDock() const
    {
        return m_spacedock;
    }
    //! Set if the vessel is space dock capable
    void SpaceDock(HS_BOOL8 val)
    {
        m_spacedock = val;
    }

    //! Systems set on this class of vessels by default
    CHSSystemArray *m_pSystems;

  protected:
    //! Unique identifier for this class
    HS_UINT32 m_class_id;

    //! Scale of the vessel
    HS_UINT32 m_size;

    //! Size of the cargo bay
    HS_UINT32 m_cargo_size;

    //! Minimum crew required to operate the vessel
    HS_UINT32 m_minmanned;

    //! Maximum hull points for this class of vessel
    HS_UINT32 m_maxhull;

    //! Flag indicating if drop jets are pressent
    HS_BOOL8 m_can_drop;

    //! Can this class accomodate large vessels
    HS_BOOL8 m_spacedock;

    //! Name for this class of vessels
    std::string m_strClassName;
};

typedef std::map < HS_UINT32, CHSShipClass * >CSTLShipClassMap;

class CHSEngSystem;


// This structure is returned as a value of GetFirstClass()/GetNextClass().
typedef struct tagHSShipClassIterator
{
    CHSShipClass *pValue;       // The next class in the list.

    // Do not modify these.  They are for internal tracking.
    CSTLShipClassMap::iterator iter;

} THSShipClassIterator;

class CHSClassDBDef:public CHSSingleton < CHSClassDBDef >
{
  public:
    CHSClassDBDef();

    HS_BOOL8 LoadFromFile(const HS_INT8 * pcPath);

    HS_BOOL8 IsValidClass(HS_UINT32 uiClassID)
    {
        CSTLShipClassMap::iterator iter;

        if ((iter =
             m_mapShipClasses.find(uiClassID)) != m_mapShipClasses.end())
        {
            return true;        // Found the class
        }
        return false;
    }

    HS_BOOL8 RemoveClass(HS_UINT32);

    HS_UINT32 GetNumClasses(void)
    {
        return m_mapShipClasses.size();
    }

    HS_BOOL8 LoadClassPicture(HS_INT32, char **);

    void SaveToFile(char *);
    void PrintInfo(HS_INT32);

    CHSEngSystem *LoadSystem(FILE * pFile, CHSShipClass * pClass);

    HS_BOOL8 GetFirstClass(THSShipClassIterator & rtIter);
    HS_BOOL8 GetNextClass(THSShipClassIterator & rtPrevIter);

    CHSShipClass *GetClass(HS_UINT32);

    // Finds the next available class ID.
    // You must specify whether you want the return ID locked or not.
    HS_UINT32 GetNextClassID(HS_BOOL8 bLockID);

    // Call this to flag a given class ID as available for use.
    void UnlockClassID(HS_UINT32 uiClassID);

    // Call this to flag a given class ID as taken.
    void LockClassID(HS_UINT32 uiClassID);

    HS_BOOL8 AddClass(CHSShipClass * pClass);

  protected:

    HS_UINT32 m_uiMaxClassID;   // Internal tracking.

    CSTLShipClassMap m_mapShipClasses;

    HS_UINT8 *m_pucClassIDs;    // An array of bits, representing used class IDs.
};

typedef CHSClassDBDef CHSClassDB;
#endif
