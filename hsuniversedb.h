// -----------------------------------------------------------------------
// $Id: hsuniversedb.h,v 1.4 2006/04/04 12:41:36 mark Exp $
// -----------------------------------------------------------------------
#if !defined(__HSUNIVERSEDB_H__)
#define __HSUNIVERSEDB_H__

#include <map>
#include "HSSingleton.h"

// Forward declarations
class CHSUniverse;

//! Typedef the universe map for readability
typedef std::map < HS_UINT32, CHSUniverse * >CSTLUniverseMap;

//! This structure is returned as a value of GetFirstObject()/GetNextObject().
typedef struct tagHSUniverseIterator
{
    CHSUniverse *pValue;        // The next object in the list.

    // Do not modify these.  They are for internal tracking.
    CSTLUniverseMap::iterator iter;

} THSUniverseIterator;

class CHSUniverseDBDef:public CHSSingleton < CHSUniverseDBDef >
{
  public:
    //! Initialize the iterator
    CHSUniverseDBDef();
    //! Free any universes that were loaded
    ~CHSUniverseDBDef();

    //! Lookup a universe by id
    CHSUniverse *FindUniverse(HS_UINT32 uiID);

    //! @brief Get the first universe in the list
    //! @return true on success, false if the list is empty
    HS_BOOL8 GetFirstUniverse(THSUniverseIterator & rtIter);

    //! @brief Get the next universe in the list
    //! @return false if no next universe otherwise true
    HS_BOOL8 GetNextUniverse(THSUniverseIterator & rtPrevIter);

    //! Return the current number of known universes
    HS_UINT32 GetNumUniverses()
    {
        return m_mapUniverses.size();
    }

    //! Save the universe information to the specified file
    void SaveToFile(const HS_INT8 * pcFilePath);

    //! @brief Load universe information from the specified file
    //! @return true on success, false on failure
    HS_BOOL8 LoadFromFile(const HS_INT8 * pcFilePath);

    //! Display a list of universe information to the specified player
    void PrintInfo(int player);


    //! @brief Add a universe to the current list.
    //! @return true on success, false if error or universe exists
    HS_BOOL8 AddUniverse(CHSUniverse *);

    //! Delete the universe with the specified id value
    HS_BOOL8 DeleteUniverse(HS_UINT32 uid);

  protected:
    //! Current data map
    CSTLUniverseMap m_mapUniverses;

    //! Iterator pointing to the current universe being evaluated
    CSTLUniverseMap::iterator m_iterCurUniverse;

};

typedef CHSUniverseDBDef CHSUniverseDB;

#endif
