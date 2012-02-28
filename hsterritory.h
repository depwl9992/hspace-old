#ifndef __HSTERRITORY_INCLUDED__
#define __HSTERRITORY_INCLUDED__

#include <vector>

//! The core types of territories available.  
enum TERRTYPE
{
    T_RADIAL = 0,
    T_CUBIC,
};

class CHS3DObject;

//! A base territory class, from which all types of territories can be derived.
class CHSTerritory
{
    public:
        //! Default constructor, init baseline variables
        CHSTerritory();

        //! Default deconstructor, does nothing
        virtual ~CHSTerritory() { }

        //! Set the DBREF of the object representing the territory
        void SetDbref(HS_INT32 objnum);

        //! Get the DBREF of the object used to represent the territory
        HS_INT32 GetDbref();

        //! Get the universe ID containing the territory
        HS_INT32 GetUID();

        //! Get the type of the territory
        TERRTYPE GetType();

        //! Get the list of manipulatable attributes
        void GetAttributeList(CHSAttributeList & rlistAttributes);

        //! Get the value of the given attribute
        virtual HS_INT8 *GetAttributeValue(HS_INT8 *strName);

        //! Save the territory information to the given file pointer 
        virtual void SaveToFile(FILE *fp);

        //! Set the specified attribute to the given value
        virtual HS_BOOL8 SetAttributeValue(HS_INT8 *strName, HS_INT8 *strValue);

        //! Check if the given point in universe, uid, is in the territory
        virtual HS_BOOL8 PtInTerritory(HS_INT32 uid, double x, double y, 
                double z);

    protected:
        //! Universe territory is in
        HS_INT32 m_uid;      

        //! Object in the game that represents this territory
        HS_INT32 m_objnum;    

        //! Territory type - cubic or radial
        TERRTYPE m_type;
};

//! A radial territory that defines a sphere of ownership
class CHSRadialTerritory : public CHSTerritory
{
    public:
        //! Default constructor, setup default values
        CHSRadialTerritory();

        // Overriden inherited methods
        HS_BOOL8 SetAttributeValue(HS_INT8 *strName, HS_INT8 *strValue);
        HS_BOOL8 PtInTerritory(HS_INT32 uid, double x, double y, double z);
        void GetAttributeList(CHSAttributeList & rlistAttributes);
        HS_INT8 *GetAttributeValue(HS_INT8 *strName);

        //! Return the X coordinate of the territory center
        HS_FLOAT64 GetX();

        //! Return the Y coordinate of the territory center
        HS_FLOAT64 GetY();

        //! Return the Z coordinate of the territory center
        HS_FLOAT64 GetZ();

        //! Return the radius for the territory
        HS_FLOAT64 GetRadius();

        //! Write radial territory data to the given file
        void SaveToFile(FILE *fp);

    protected:
        //! Center X coordinate
        HS_FLOAT64 m_cx;         

        //! Center Y coordinate
        HS_FLOAT64 m_cy;

        //! Center Z coordinate
        HS_FLOAT64 m_cz;

        //! Radius of the universe
        HS_FLOAT64 m_radius;
};

//! A cubic territory that defines a cube of ownership
class CHSCubicTerritory:public CHSTerritory
{
    public:
        //! Default constructor
        CHSCubicTerritory();

        // Inherited methods
        HS_BOOL8 SetAttributeValue(HS_INT8 *strName, HS_INT8 *strValue);
        HS_BOOL8 PtInTerritory(HS_INT32 uid, double x, double y, double z);
        void GetAttributeList(CHSAttributeList & rlistAttributes);
        HS_INT8 *GetAttributeValue(HS_INT8 *strName);
        void SaveToFile(FILE *fp);

        //! Minimum X coordinate
        HS_FLOAT64 GetMinX();

        //! Minimum Y coordinate
        HS_FLOAT64 GetMinY();

        //! Minimum Z coordinate
        HS_FLOAT64 GetMinZ();

        //! Maximum X coordinate
        HS_FLOAT64 GetMaxX();

        //! Maximum Y coordinate
        HS_FLOAT64 GetMaxY();

        //! Maximum Z coordinate
        HS_FLOAT64 GetMaxZ();

    protected:
        // Bounding coordinates
        HS_FLOAT64 m_minx;        
        HS_FLOAT64 m_miny;
        HS_FLOAT64 m_minz;
        HS_FLOAT64 m_maxx;
        HS_FLOAT64 m_maxy;
        HS_FLOAT64 m_maxz;
};

//! The CHSTerritoryArray stores a list of all territories in the game and 
//! provides that information to whomever requests it.
class CHSTerritoryArray
{
    public:
        //! Default constructor
        CHSTerritoryArray();

        //! Deconstructor -- free all the memory
        ~CHSTerritoryArray();

        //! Allocate a new territory based on the given object and type
        CHSTerritory *NewTerritory(HS_INT32 objnum, TERRTYPE type = T_RADIAL);

        //! @brief Check if the given 3DObject is in a territory
        //! @return NULL if not in a territory or ptr to the territory
        CHSTerritory *InTerritory(CHS3DObject * cObj);

        //! Find the territory data for the specified dbref
        CHSTerritory *FindTerritory(HS_INT32 objnum);

        //! Return the current number of territories
        HS_UINT32 NumTerritories();

        //! Load territory data from the given file
        HS_BOOL8 LoadFromFile(const HS_INT8 *lpstrPath);

        //! Remove the territory associated with the given dbref
        HS_BOOL8 RemoveTerritory(HS_INT32 objnum);

        //! Save all territory data to the given file path
        void SaveToFile(const HS_INT8 *lpstrPath);

        //! Display territory information to the given player
        void PrintInfo(HS_INT32 player);
    protected:

        //! Storage array for the territories
        std::vector<CHSTerritory *> m_territories;
};

extern CHSTerritoryArray taTerritories;

#endif // __HSTERRITORY_INCLUDED__
