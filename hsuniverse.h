// -----------------------------------------------------------------------
//! $Id: hsuniverse.h,v 1.3 2006/04/04 12:41:36 mark Exp $
// -----------------------------------------------------------------------

#ifndef __HSUNIVERSE_INCLUDED__
#define __HSUNIVERSE_INCLUDED__

#include <string>
#include <list>

#include "hsobject.h"
#include "hsobjects.h"
#include "hsship.h"

class CHSCelestial;

typedef std::list < CHS3DObject * >CSTLHSObjectList;

// This structure is returned as a value of GetFirstObject()/GetNextObject().
typedef struct tagHSObjectIterator
{
    CHS3DObject *pValue;        // The next object in the list.

    // Do not modify these.  They are for internal tracking.
    std::list < CHS3DObject * >::iterator iter;
    HS_TYPE eSearchType;
    CSTLHSObjectList *pSearchList;

} THSObjectIterator;

class CHSUniverse:public CHSObject
{
  public:

  protected:

    HS_UINT32 m_uiID;           // Refers to the MUSH object for this universe
    std::string m_strName;

    // Storage arrays
    CSTLHSObjectList m_listObjects;     // All objects in this universe.
    CSTLHSObjectList m_listActiveObjects;       // Objects in this universe that are actively processing.
    CSTLHSObjectList m_listShips;       // All ships in this universe.
    CSTLHSObjectList m_listCelestials;  // All celestials in this universe.

    CSTLHSObjectList::iterator m_iterCurActiveObject;

  public:
    CHSUniverse(void);

    const HS_INT8 *GetName(void) const
    {
        return m_strName.c_str();
    }

    HS_UINT32 GetID(void) const
    {
        return m_uiID;
    }

    HS_UINT32 GetNumObjects(HS_TYPE eType = HST_NOTYPE);
    HS_UINT32 GetNumActiveObjects(HS_TYPE eType = HST_NOTYPE);

    void RemoveObject(CHS3DObject * pObject);
    void RemoveActiveObject(CHS3DObject * pObject);
    void SendMessage(const HS_INT8 * pcMessage, int, CHS3DObject *);
    void SendContactMessage(const HS_INT8 * pcMessage, int, CHS3DObject *);
    void SetID(HS_UINT32 uiID)
    {
        m_uiID = uiID;
    }
    void SetName(const HS_INT8 * pcName)
    {
        if (pcName)
        {
            m_strName = pcName;
        }
        else
        {
            m_strName = "";
        }
    }
    void SaveToFile(FILE *);

    HS_BOOL8 AddActiveObject(CHS3DObject * pObject);
    HS_BOOL8 AddObject(CHS3DObject * pObject);
    HS_BOOL8 IsEmpty(void)
    {
        return (m_listObjects.size() == 0);
    }
    HS_BOOL8 SetAttributeValue(const HS_INT8 * pcKey,
                               const HS_INT8 * pcValue);
    HS_BOOL8 GetFirstObject(THSObjectIterator & rtValue, HS_TYPE eType =
                            HST_NOTYPE);
    HS_BOOL8 GetNextObject(THSObjectIterator & rtValue, HS_TYPE eType =
                           HST_NOTYPE);
    CHS3DObject *GetFirstActiveObject();
    CHS3DObject *GetNextActiveObject();

    CHS3DObject *FindObject(HS_DBREF, HS_TYPE type = HST_NOTYPE);

  protected:
    ~CHSUniverse(void);

    // Member Functions
    void RemoveShip(CHSShip *);
    void RemoveCelestial(CHSCelestial *);

    HS_BOOL8 AddCelestial(CHSCelestial *);
    HS_BOOL8 AddShip(CHSShip *);
};


#endif // __UNIVERSE_INCLUDED__
