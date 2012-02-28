// -----------------------------------------------------------------------
//! $Id: hsobjects.h,v 1.16 2006/04/04 12:41:36 mark Exp $
// -----------------------------------------------------------------------

#ifndef __HSOBJECTS_INCLUDED__
#define __HSOBJECTS_INCLUDED__

#include <map>
#include <list>
#include <set>

#include "hsobject.h"
#include "hslandingloc.h"
#include "hshatch.h"
#include "hsconsole.h"
#include "hstypes.h"
#include "hsweapon.h"
#include "hsfuel.h"

class CHSUniverse;
class CHSConsole;
class CHSLandingLoc;

//! A class that represents a vector.  
class CHSVector
{
  public:
    //! Default constructor
    CHSVector();
    //! Construct using specified coefficients
    CHSVector(HS_FLOAT64 i, HS_FLOAT64 j, HS_FLOAT64 k);

    //! Get the i coefficient
    HS_FLOAT64 i();

    //! Get the j coefficient
    HS_FLOAT64 j();

    //! Get the k coefficient
    HS_FLOAT64 k();

    //! Overloaded set operation
    void operator=(CHSVector & vec);

    //! Overload equality operator
    HS_BOOL8 operator==(CHSVector & vec);

    //! Overload the addition operator
        CHSVector & operator+(CHSVector & vec);

    //! Overload the += operator
    void operator+=(CHSVector & vec);

    //! Compute the dot product between two vectors
    HS_FLOAT64 DotProduct(CHSVector & vec);

  protected:
        HS_FLOAT64 ci, cj, ck;  // The coefficients for the vector
};

//! @brief Defined object types within the HSpace system
//! Possible object types.  If you're adding any new types of objects
//! to space, you have to add a listing here.
enum HS_TYPE
{
    HST_NOTYPE = 0,
    HST_SHIP,
    HST_MISSILE,
    HST_PLANET,
    HST_WORMHOLE,
    HST_BLACKHOLE,
    HST_NEBULA,
    HST_ASTEROID
};

//! Define a typedef for landing location lists
typedef std::list < CHSLandingLoc * >CSTLLandingLocList;

//! Base HSpace object class for space objects
class CHS3DObject:public CHSObject
{
  public:
    //! Default constructor
    CHS3DObject();

    //! Get X coordinate as a floating point value
    HS_FLOAT64 GetX();
    //! Get Y coordinate as a floating point value
    HS_FLOAT64 GetY();
    //! Get Z coordinate as a floating point value
    HS_FLOAT64 GetZ();
    //! Get the universie ID this object is in
    HS_UINT32 GetUID();
    //! Return the DBREF of the object as referenced in the MU*
    HS_DBREF GetDbref();
    //! Get the name of the object
    HS_INT8 *GetName();
    //! Set the X coordinate of the object
    void SetX(HS_FLOAT64 x);
    //! Set the Y coordinate of the object
    void SetY(HS_FLOAT64 y);
    //! Set the Z coordinate of the object
    void SetZ(HS_FLOAT64 z);
    //! Set the universe id where this object current exists
    void SetUID(HS_UINT32 uid);
    //! Set the name for this object
    void SetName(const HS_INT8 * objnmae);
    //! Set the server based DBREF of the game object
    void SetDbref(HS_DBREF obj_dbref);
    //! @brief Check if the object is visible
    //! @return true if visible, false if not
    HS_BOOL8 IsVisible();

    //! Return a pointer to the CHSUniverse of this object
    CHSUniverse *GetUniverse();

    //! Get the motion vector for the object
    CHSVector & GetMotionVector();

    //! Equality comparison by reference
    HS_BOOL8 operator==(CHS3DObject &);
    //! Equality comparison by pointer
    HS_BOOL8 operator==(CHS3DObject *);

    //! Return speed as a measure of units / second
    virtual HS_INT32 GetSpeed();

    //! @brief Move the object towards certain coordinates at a certain speed.
    //! @param x - goal x coordinate
    //! @param y - goal y coordinate
    //! @param z - goal z coordinate
    //! @param speed - speed of movement toward goal point
    virtual void MoveTowards(HS_FLOAT64 x, HS_FLOAT64 y, HS_FLOAT64 z,
                             HS_FLOAT32 speed);


    //! @brief Move the object at units/second along the specified headings
    //! @param xyheading - xyheading for the trajectory
    //! @param zheading - zheading for the trajectory
    //! @param speed - goal speed along trajectory
    virtual void MoveInDirection(HS_INT32 xyheading, HS_INT32 zheading,
                                 HS_FLOAT32 speed);

    //! @brief Handle a message of the specified type
    //! @param lpstrMsg - message to be sent
    //! @param msgType - type of message being transmitted
    //! @param data - user specified data
    virtual void HandleMessage(const HS_INT8 * lpstrMsg, HS_INT32 msgType,
                               long *data = NULL);

    //! Process a single cycle for the objec/t
    virtual void DoCycle();

    //! Remove object based attributes; does nothing at the parent level
    virtual void ClearObjectAttrs();

    //! @brief Write object data to the specified file pointer. 
    //! @param fp - file pointer to write to
    //! All necessary state data must be stored in the file to reload 
    //! on a game restart.  Each derived class should override this method
    //! to store data appropriate to that particular class type.
    virtual void WriteToFile(FILE * fp);


    //! @brief Handling incoming target lock or unlock
    //! @param cLocker - space object performing the (un)lock
    //! @param bLocking - true if locking, false if unlocking
    //! Parent implementation does nothing
    virtual void HandleLock(CHS3DObject * cLocker, HS_BOOL8 bLocking);


    //! @brief Apply incoming damage from another object
    //! @param cSource - attacking object
    //! @param cWeapon - attacking weapon data
    //! @param strength - amount of damage being inflicted
    //! @param cConsole - console that fired the shot or caused the damage
    //! @param iSysType - preferred system to damage
    virtual void HandleDamage(CHS3DObject * cSource,
                              CHSWeaponData * cWeapon, HS_INT32 strength,
                              CHSConsole * cConsole, HS_INT32 iSysType);

    //! Remove the object from space for whatever reason
    virtual void ExplodeMe();

    //! @brief Called by the scanning object to generate a scan report
    //! @param cScanner - object requesting the scan
    //! @param player - game object being given the scan report
    //! @param id - is object identified on scanning object's sensors
    virtual void GiveScanReport(CHS3DObject * cScanner, HS_DBREF player,
                                HS_BOOL8 id);

    //! Cleanup handler when an object is finished loading from the database
    virtual void EndLoadObject();

    //! Get the ANSI color code string for this object
    virtual HS_INT8 *GetObjectColor();

    //! Get the single character identifier for this object
    virtual HS_INT8 GetObjectCharacter();

    //! @brief Lookup an attribute on the object
    //! @param strName - attribute to be queried
    //! @return attribute value as a char* array
    virtual HS_INT8 *GetAttributeValue(HS_INT8 * strName);

    //! @brief Get the list of attributes available for this object
    //! @param rlistAttributes - list object to store attributes in
    //! Overload this in each derived object type to provide a list of attributes
    //! that can be queried through GetAttributeValue() or SetAttributeValue.
    virtual void GetAttributeList(CHSAttributeList & rlistAttributes);

    //! @brief Attempt to set the specified attribute to the given value
    //! @param strName - attribute being set
    //! @param strValue - value to be stored
    //! @return true on success, false if attribute not found or value invalid
    virtual HS_BOOL8 SetAttributeValue(HS_INT8 * strName, HS_INT8 * strValue);

    //! Check if object is active or not
    virtual HS_BOOL8 IsActive();

    //! @brief Load object data from a file
    //! @param fp - file pointer to use for loading
    //! @return true on success, false on failure
    virtual HS_BOOL8 LoadFromFile(FILE * fp);

    //! @brief Get a list of engineering system types on the object
    //! @param iBuff - array of integers to store the type ids
    //! @return Number of Engineering system types found
    virtual HS_UINT32 GetEngSystemTypes(HS_INT32 * iBuff);

    //! Return the size of the object
    virtual HS_UINT32 GetSize();

    //! Overrideable method to return an array of engineering systems.
    //! @return NULL if not present, else array
    virtual CHSSystemArray *GetEngSystemArray();

    //! Find a console on the object by dbref
    virtual CHSConsole *FindConsole(HS_DBREF dbref);

    //! Overrideable class to get an engineering system on the object
    //! @return NULL if not present, else valid pointer
    virtual CHSEngSystem *GetEngSystem(HS_INT32);

    //! @brief Adds a landing location or bay to the object
    //! @param room - game room to be added as a landing loc
    //! @return true on success, false on failure
    HS_BOOL8 AddLandingLoc(HS_DBREF room);

    //! @brief Delete the specified location as a landing loc on this object
    //! @param dbLocation - location to be deleted
    //! @return true on success, false on failure
    HS_BOOL8 DelLandingLoc(HS_DBREF dbLocation);

    //! @brief Return a landing location pointer by slot (list order)
    //! @param slot - slot number of the landing loc to be queried
    //! @return NULL on failure, CHSLandingLoc* otherwise
    CHSLandingLoc *GetLandingLoc(HS_INT32 slot);

    //! @brief Find a landing location identified by dbref
    //! @param objnum - dbref of location to be found
    //! @return NULL on failure, CHSLandingLoc* otherwise
    CHSLandingLoc *FindLandingLoc(HS_DBREF objnum);

    //! @brief Find a landing location identified by name
    //! @param pcName - name of the landing location to be found
    //! @return NULL on failure, CHSLandingLoc* otherwise
    CHSLandingLoc *FindLandingLocByName(const HS_INT8 * pcName);

    //! Return the number of landing locations on object that are visible
    HS_UINT32 GetNumVisibleLandingLocs();

    //! Return the HS_TYPE of this object
    HS_TYPE GetType();

    //! Set the next object in the linked list
    void SetNext(CHS3DObject *);
    //! Set the previous object in the linked list
    void SetPrev(CHS3DObject *);
    //! Get the next object in the linked list
    CHS3DObject *GetNext();
    //! Get the previous object in the linked list
    CHS3DObject *GetPrev();

    //! Return the typename as a string for overriding default typenames
    const HS_INT8 *GetTypeName();

    //! Set the typename as a string
    void SetTypeName(HS_INT8 * newname);

  protected:
    //! Default deconstructor: cleans up allocated memory and unsets flags
    ~CHS3DObject();

    //! @brief Key based data loading
    //! @param key - key identifier to process
    //! @param strValue - value loaded from the file
    //! @param fp - file pointer used for processing
    //! @return true if found & loaded, false on failure
    //! You MUST implement this function in your derived
    //! object if you have custom information that's loaded
    //! from the object db.  Otherwise, you can just initialize
    //! your variables in the constructor and let the CHS3DObject
    //! handle any keyed information in the database.
    virtual HS_BOOL8 HandleKey(HS_INT32 key, HS_INT8 * strValue, FILE * fp);

    //! Unique object identifier
    HS_DBREF m_objnum;
    //! x coordinate
    HS_FLOAT64 m_x;
    //! y coordinate
    HS_FLOAT64 m_y;
    //! z coordinate
    HS_FLOAT64 m_z;
    //! current universe identifier
    HS_UINT32 m_uid;
    //! boolean flag to mark visiblity
    HS_BOOL8 m_visible;
    //! size of the object
    HS_INT32 m_size;
    //! current speed of the object
    HS_INT32 m_current_speed;
    //! object name
    HS_INT8 *m_name;
    //! Is object being loaded?
    HS_BOOL8 m_bLoading;

    //! Object type 
    enum HS_TYPE m_type;

    //! Current motion vector
    CHSVector m_motion_vector;

    //! Landing locations for the object.
    CSTLLandingLocList m_listLandingLocs;

    //! Overridable typename attribute that is used for reporting
    std::string mTypeName;

    //! previous object in the linked list
    CHS3DObject *m_prev;
    //! next object in the linked list
    CHS3DObject *m_next;
};


#endif // __HSOBJECTS_INCLUDED__
