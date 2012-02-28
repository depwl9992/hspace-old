#ifdef _WIN32
#pragma once
#pragma warning(disable:4786)
#endif

#if !defined(__HSPACKETS_H__)
#define __HSPACKETS_H__


// Library Includes
#include <list>
#include <string>

#include "HSVariant.h"
#include "HSDataStream.h"
#include "HSPacketTypes.h"

// The base class for all packets.  This class cannot be used alone.
class CHSPacket
{
    // Member Variables
  public:

  protected:

    EHSPacketType m_ePacketType;

    unsigned int m_uiPacketAddress;     // Identifies the socket the packet will go to/is coming from.


    // Member Functions
  public:

        CHSPacket(): m_uiPacketAddress(0)
    {
        m_ePacketType = PT_INVALID;
    }

    virtual ~ CHSPacket()
    {
    }

    EHSPacketType GetPacketType() const
    {
        return m_ePacketType;
    }

    // Only useful if you are the server.
    unsigned int GetPacketAddress() const
    {
        return m_uiPacketAddress;
    }

    // You only need to call this if you are the server, not the client.
    // If you are a client, just call SendPacket in the HSNetwork, and
    // the packet will be sent to the server.
    void SetPacketAddress(unsigned int uiAddress)
    {
        m_uiPacketAddress = uiAddress;
    }

    // All packets derived from this base class must implement these two
    // functions to write and read from a CHSDataStream.
    virtual bool WriteToStream(CHSDataStream & /* rStream */ ) = 0;
    virtual bool ReadFromStream(CHSDataStream & /* rStream */ ) = 0;

  protected:
};

// If you receive this packet, a connection was closed, either to the server or to a client.
class CHSPConnectionClosed:public CHSPacket
{
    // Member Variables
  public:

  protected:

    // Member Functions
  public:

    CHSPConnectionClosed()
    {
        m_ePacketType = PT_CONNECTION_CLOSED;
    }

       ~CHSPConnectionClosed()
    {
    }

    bool WriteToStream(CHSDataStream & /* rStream */ )
    {
        return true;
    }
    bool ReadFromStream(CHSDataStream & /* rStream */ )
    {
        return true;
    }
};

// You should not directly send this packet to the server.  It is used internally
// during connection to the server.
class CHSPLogin:public CHSPacket
{
    // Member Variables
  public:

    char *m_pcPlayerName;
    char *m_pcPassword;

  protected:

        bool m_bStringsAllocated;

    // Member Functions
  public:

        CHSPLogin():
        m_pcPlayerName(NULL), m_pcPassword(NULL), m_bStringsAllocated(false)
    {
        m_ePacketType = PT_LOGIN;
    }

       ~CHSPLogin();

    bool WriteToStream(CHSDataStream & rStream);
    bool ReadFromStream(CHSDataStream & rStream);
};

// This packet is handled internally by the CHSNetwork.
class CHSPLoginResponse:public CHSPacket
{
    // Member Variables
  public:

    bool m_bLoggedIn;

  protected:

    // Member Functions
  public:

    CHSPLoginResponse(): m_bLoggedIn(false)
    {
        m_ePacketType = PT_LOGIN_RESPONSE;
    }

    bool WriteToStream(CHSDataStream & rStream);
    bool ReadFromStream(CHSDataStream & rStream);
};

// Send this packet to the server when you want to retrieve a list of classes.
// You will receive a CHSPClassList packet in response.
class CHSPGetClassList:public CHSPacket
{
    // Member Variables
  public:

  protected:

    // Member Functions
  public:

    CHSPGetClassList()
    {
        m_ePacketType = PT_GET_CLASS_LIST;
    }

    bool WriteToStream(CHSDataStream & /* rStream */ )
    {
        return true;
    }
    bool ReadFromStream(CHSDataStream & /* rStream */ )
    {
        return true;
    }
};

// This is sent to the client from the server in response to a CHSPGetClassList.
// This packet will contain a list of class IDs and class names.
class CHSPClassList:public CHSPacket
{
    // Member Variables
  public:

    struct THSClass
    {
        unsigned int uiClassID;
            std::string strClassName;
    };

  protected:

        std::list < THSClass > m_listClasses;

        std::list < THSClass >::iterator m_iterCur;

    // Member Functions
  public:

        CHSPClassList()
    {
        m_ePacketType = PT_CLASS_LIST;
    }
    // The server uses this to add a new class.
    void AddClass(THSClass & rtClass);

    // Call this to get the first class in the list.
    THSClass *GetFirstClass();

    // Call this to get all classes beyond the first.
    THSClass *GetNextClass();

    bool WriteToStream(CHSDataStream & rStream);
    bool ReadFromStream(CHSDataStream & rStream);

};

// Send this to the server when you want to retrieve a list of attributes and values for
// a specific class ID.  You will receive a CHSPClassData packet in response.
class CHSPGetClassData:public CHSPacket
{
    // Member Variables
  public:

    unsigned int m_uiClassID;   // The class ID for which to retrieve the data.

  protected:

    // Member Functions
  public:

        CHSPGetClassData(): m_uiClassID(0)
    {
        m_ePacketType = PT_GET_CLASS_DATA;
    }

    bool WriteToStream(CHSDataStream & rStream);
    bool ReadFromStream(CHSDataStream & rStream);
};

// Sent to the client in response to a CHSPGetClassData packet.  This packet will
// contain a list of class attribute IDs and values.
class CHSPClassData:public CHSPacket
{
    // Member Variables
  public:

    struct THSClassAttribute
    {
        std::string strAttributeName;
        CHSVariant varValue;    // A variant object containing the atribute value.
    };

    unsigned int m_uiClassID;   // The class ID to which the data pertains.
    bool m_bBadQuery;           // True if the query failed.

  protected:

        std::list < THSClassAttribute > m_listAttributes;       // Use GetFirstAttribute()/GetNextAttribute().
        std::list < THSClassAttribute >::iterator m_iterCur;

    // Member Functions
  public:

        CHSPClassData(): m_uiClassID(0), m_bBadQuery(false)
    {
        m_ePacketType = PT_CLASS_DATA;
    }

    // Use the GetFirstAttr/GetNextAttr to get attributes in the packet.
    THSClassAttribute *GetFirstAttr();
    THSClassAttribute *GetNextAttr();

    // The server uses this to add attributes to the packet.
    void AddAttribute(THSClassAttribute & rtAttr);

    bool WriteToStream(CHSDataStream & rStream);
    bool ReadFromStream(CHSDataStream & rStream);
};

// Send this packet to the server when you want to modify one or more attributes of a
// class.  You should call AddAttribute() for each attribute you want to modify.
// A list of attributes is available in HSAttributes.h.
// Whether the request succeeds or fails, you will receive a CHSPSetClassDataResponse.
class CHSPSetClassData:public CHSPacket
{
    // Member Variables
  public:

    struct THSClassAttribute
    {
        std::string strAttributeName;
        std::string strValue;
    };

    unsigned int m_uiClassID;   // The class ID to which the data pertains.  You MUST set this!

  protected:

        std::list < THSClassAttribute > m_listAttributes;       // Use GetFirstAttribute()/GetNextAttribute().
        std::list < THSClassAttribute >::iterator m_iterCur;

    // Member Functions
  public:

        CHSPSetClassData(): m_uiClassID(0)
    {
        m_ePacketType = PT_SET_CLASS_DATA;
    }

    // Call this function for each attribute that you want to modify.
    void AddAttribute(THSClassAttribute & rtAttr);

    // These functions will be used by the server to retrieve attributes.
    THSClassAttribute *GetFirstAttr();
    THSClassAttribute *GetNextAttr();

    bool WriteToStream(CHSDataStream & rStream);
    bool ReadFromStream(CHSDataStream & rStream);
};

class CHSPSetClassDataResponse:public CHSPacket
{
    // Member Variables
  public:

    unsigned int m_uiClassID;   // The class ID that was being modified.
    bool m_bSuccess;            // True if the modifications passed.  Otherwise, false for any failures.

  protected:

    // Member Functions
  public:

        CHSPSetClassDataResponse(): m_uiClassID(0)
    {
        m_ePacketType = PT_SET_CLASS_DATA_RESPONSE;
    }

    bool WriteToStream(CHSDataStream & rStream);
    bool ReadFromStream(CHSDataStream & rStream);
};

// Send this packet to the server to retrieve a list of engineering systems
// on a given ship class or HSpace object.  You will receive a CHSPSystemList
// in response.
class CHSPGetSystemList:public CHSPacket
{
    // Member Variables
  public:

    unsigned int m_uiClassOrObjectID;   // Object ID or Class ID from which to retrieve the list.

    bool m_bClass;              // Set this to true if this query is for a class.  False if this,
    // query is for an HSpace object.

  protected:

    // Member Functions
  public:

        CHSPGetSystemList(): m_uiClassOrObjectID(0), m_bClass(true)
    {
        m_ePacketType = PT_GET_SYSTEM_LIST;
    }

    bool WriteToStream(CHSDataStream & rStream);
    bool ReadFromStream(CHSDataStream & rStream);
};

// This is sent to the client from the server in response to a CHSPGetSystemList.
// This packet will contain a list of system names.
class CHSPSystemList:public CHSPacket
{
    // Member Variables
  public:

    bool m_bQuerySucceeded;     // True if this packet contains valid data.  False if the query failed.
  protected:

    std::list < std::string > m_listSystems;

    std::list < std::string >::iterator m_iterCur;

    // Member Functions
  public:

    CHSPSystemList(): m_bQuerySucceeded(true)
    {
        m_ePacketType = PT_SYSTEM_LIST;
    }

    // Call this to get the first system in the list.
    const char *GetFirstSystem();

    // Call this to get all classes beyond the first.
    const char *GetNextSystem();

    // The server uses this to add a new system to the list.
    void AddSystem(const char *pcName);

    bool WriteToStream(CHSDataStream & rStream);
    bool ReadFromStream(CHSDataStream & rStream);

};

// Send this packet to the server with a new class name to have the
// class created.  You will receive a CHSPCreateClassResponse packet in response.
class CHSPCreateClass:public CHSPacket
{
    // Member Variables
  public:

    std::string m_strClassName; // Set this to the name of the class you want to create.

  protected:

    // Member Functions
  public:

    CHSPCreateClass()
    {
        m_ePacketType = PT_CREATE_CLASS;
    }

    bool WriteToStream(CHSDataStream & rStream);
    bool ReadFromStream(CHSDataStream & rStream);
};

// The server sends this packet to the client in response to a 
// CHSPCreateClass packet.
class CHSPCreateClassResponse:public CHSPacket
{
    // Member Variables
  public:

    bool m_bCreateSucceeded;    // True if the class was created.  False if any failure.

    unsigned int m_uiClassID;   // The ID of the class, if it was created.

  protected:

    // Member Functions
  public:

        CHSPCreateClassResponse(): m_bCreateSucceeded(true), m_uiClassID(0)
    {
        m_ePacketType = PT_CREATE_CLASS_RESPONSE;
    }

    bool WriteToStream(CHSDataStream & rStream);
    bool ReadFromStream(CHSDataStream & rStream);
};

// Send this packet to the server when you want to retrieve a list of attributes
// for an engineering system, either on an object or ship class.
// You will get, in response, a CHSPSystemData packet.
class CHSPGetSystemData:public CHSPacket
{
    // Member Variables
  public:

    unsigned int m_uiClassOrObjectID;   // Set this to the ID of the ship class or object.
        std::string m_strSystemName;    // Set this to the name of the system for which to retrieve attributes.
    bool m_bClass;              // Set this to true if this query is for a class.  False if this,
    // query is for an HSpace object.

  protected:

    // Member Functions
  public:

        CHSPGetSystemData(): m_uiClassOrObjectID(0), m_bClass(true)
    {
        m_ePacketType = PT_GET_SYSTEM_DATA;
    }

    bool WriteToStream(CHSDataStream & rStream);
    bool ReadFromStream(CHSDataStream & rStream);
};

// Sent to the client in response to a CHSPGetSystemData packet.  This packet will
// contain a list of system attribute IDs and values.  Refer to the HSSYSATTR_* IDs
// in HSAttributes.h
class CHSPSystemData:public CHSPacket
{
    // Member Variables
  public:

    struct THSSystemAttribute
    {
        std::string strAttributeName;
        bool bValueSet;         // True if the varValue contains a valid value, or
        // false if the attribute is not set locally on the system.
        CHSVariant varValue;    // A variant object containing the atribute value.
    };

    unsigned int m_uiClassOrObjectID;   // The class/object ID to which the data pertains.
    bool m_bBadQuery;           // True if the query failed.

  protected:

        std::list < THSSystemAttribute > m_listAttributes;      // Use GetFirstAttribute()/GetNextAttribute().
        std::list < THSSystemAttribute >::iterator m_iterCur;

    // Member Functions
  public:

        CHSPSystemData(): m_uiClassOrObjectID(0), m_bBadQuery(false)
    {
        m_ePacketType = PT_SYSTEM_DATA;
    }

    // Use the GetFirstAttr/GetNextAttr to get attributes in the packet.
    THSSystemAttribute *GetFirstAttr();
    THSSystemAttribute *GetNextAttr();

    // The server uses this to add attributes to the packet.
    void AddAttribute(THSSystemAttribute & rtAttr);

    bool WriteToStream(CHSDataStream & rStream);
    bool ReadFromStream(CHSDataStream & rStream);
};

// Send this packet to the server when you want to modify one or more attributes of an
// engineering system.  You should call AddAttribute() for each attribute you want to modify.
// A list of attributes is available in HSAttributes.h.
// Whether the request succeeds or fails, you will receive a CHSPSetSystemDataResponse.
class CHSPSetSystemData:public CHSPacket
{
    // Member Variables
  public:

    struct THSSystemAttribute
    {
        std::string strAttributeName;
        std::string strValue;   // A variant object containing the atribute value.
    };

    unsigned int m_uiClassOrObjectID;   // The class/object ID to which the data pertains.  You MUST set this!
    bool m_bIsClass;            // Set this to true if the m_uiClassOrObjectID refers to a class ID.
        std::string m_strSystemName;    // Set this to the name of the system that is being modified.

  protected:

        std::list < THSSystemAttribute > m_listAttributes;      // Use GetFirstAttribute()/GetNextAttribute().
        std::list < THSSystemAttribute >::iterator m_iterCur;

    // Member Functions
  public:

        CHSPSetSystemData(): m_uiClassOrObjectID(0), m_bIsClass(true)
    {
        m_ePacketType = PT_SET_SYSTEM_DATA;
    }

    // Call this function for each attribute that you want to modify.
    void AddAttribute(THSSystemAttribute & rtAttr);

    // These functions will be used by the server to retrieve attributes.
    THSSystemAttribute *GetFirstAttr();
    THSSystemAttribute *GetNextAttr();

    bool WriteToStream(CHSDataStream & rStream);
    bool ReadFromStream(CHSDataStream & rStream);
};

class CHSPSetSystemDataResponse:public CHSPacket
{
    // Member Variables
  public:

    unsigned int m_uiClassOrObjectID;   // The class/object ID that was being modified.
    bool m_bSuccess;            // True if the modifications passed.  Otherwise, false for any failures.

  protected:

    // Member Functions
  public:

        CHSPSetSystemDataResponse(): m_uiClassOrObjectID(0), m_bSuccess(true)
    {
        m_ePacketType = PT_SET_SYSTEM_DATA_RESPONSE;
    }

    bool WriteToStream(CHSDataStream & rStream);
    bool ReadFromStream(CHSDataStream & rStream);
};

// Send this packet to the server when you want to delete a ship class.
// You will get a CHSPDeleteClassResponse packet in response.
class CHSPDeleteClass:public CHSPacket
{
    // Member Variables
  public:

    unsigned int m_uiClassID;   // The class ID to delete.

  protected:

    // Member Functions
  public:

        CHSPDeleteClass(): m_uiClassID(0)
    {
        m_ePacketType = PT_DELETE_CLASS;
    }

    bool WriteToStream(CHSDataStream & rStream);
    bool ReadFromStream(CHSDataStream & rStream);
};

// You get this packet in response to a CHSPDeleteClass packet.
class CHSPDeleteClassResponse:public CHSPacket
{
    // Member Variables
  public:

    unsigned int m_uiClassID;   // The class ID to delete.
    bool m_bDeleted;            // True if the class was deleted.  False if the class could not be deleted.

  protected:

    // Member Functions
  public:

        CHSPDeleteClassResponse(): m_uiClassID(0), m_bDeleted(false)
    {
        m_ePacketType = PT_DELETE_CLASS_RESPONSE;
    }

    bool WriteToStream(CHSDataStream & rStream);
    bool ReadFromStream(CHSDataStream & rStream);
};

// Send this packet to the server when you want to add an engineering system
// to an HSpace object or ship class.
class CHSPAddSystem:public CHSPacket
{
    // Member Variables
  public:

    unsigned int m_uiClassOrObjectID;   // The class/object ID to which the system should be added.
    bool m_bIsClass;            // Set this to true if m_uiClassOrObjectID is a class ID.
    unsigned int m_uiSystemType;        // The type of engineering system to add.  These are HSEngTypes.h.

  protected:

    // Member Functions
  public:

        CHSPAddSystem():
        m_uiClassOrObjectID(0), m_bIsClass(true), m_uiSystemType(0)
    {
        m_ePacketType = PT_ADD_SYSTEM;
    }

    bool WriteToStream(CHSDataStream & rStream);
    bool ReadFromStream(CHSDataStream & rStream);
};

// The server sends this packet in response to a CHSPAddSystem packet.
class CHSPAddSystemResponse:public CHSPacket
{
    // Member Variables
  public:

    unsigned int m_uiClassOrObjectID;   // The class/object ID to which the system was being added.
    bool m_bAdded;              // True if the system was added.  Otherwise, false.
        std::string m_strSystemName;    // The name of the system that was added, if it was added.

  protected:

    // Member Functions
  public:

        CHSPAddSystemResponse(): m_uiClassOrObjectID(0), m_bAdded(false)
    {
        m_ePacketType = PT_ADD_SYSTEM_RESPONSE;
    }

    bool WriteToStream(CHSDataStream & rStream);
    bool ReadFromStream(CHSDataStream & rStream);
};


// Send this packet to the server when you want to delete an engineering system
// from an HSpace object or ship class.
class CHSPDeleteSystem:public CHSPacket
{
    // Member Variables
  public:

    unsigned int m_uiClassOrObjectID;   // The class/object ID to which the system should be deleted.
    bool m_bIsClass;            // Set this to true if m_uiClassOrObjectID is a class ID.
        std::string m_strSystemName;    // The name of the engineering system to delete.

  protected:

    // Member Functions
  public:

        CHSPDeleteSystem(): m_uiClassOrObjectID(0), m_bIsClass(true)
    {
        m_ePacketType = PT_DELETE_SYSTEM;
    }

    bool WriteToStream(CHSDataStream & rStream);
    bool ReadFromStream(CHSDataStream & rStream);
};

// The server sends this packet in response to a CHSPDeleteSystem packet.
class CHSPDeleteSystemResponse:public CHSPacket
{
    // Member Variables
  public:

    unsigned int m_uiClassOrObjectID;   // The class/object ID to which the system was being deleted.
    bool m_bDeleted;            // True if the system was deleted.  Otherwise, false.

  protected:

    // Member Functions
  public:

        CHSPDeleteSystemResponse(): m_uiClassOrObjectID(0), m_bDeleted(false)
    {
        m_ePacketType = PT_DELETE_SYSTEM_RESPONSE;
    }

    bool WriteToStream(CHSDataStream & rStream);
    bool ReadFromStream(CHSDataStream & rStream);
};

// Send this packet to get a list of universes from the server.
class CHSPGetUniverseList:public CHSPacket
{
    // Member Variables
  public:

  protected:

    // Member Functions
  public:

    CHSPGetUniverseList()
    {
        m_ePacketType = PT_GET_UNIVERSE_LIST;
    }

    bool WriteToStream(CHSDataStream & /* rStream */ )
    {
        return true;
    }
    bool ReadFromStream(CHSDataStream & /* rStream */ )
    {
        return true;
    }
};

// This is sent to the client from the server in response to a CHSPGetUniverseList.
// This packet will contain a list of universe IDs and names.
class CHSPUniverseList:public CHSPacket
{
    // Member Variables
  public:

    struct THSUniverse
    {
        unsigned int uiID;
            std::string strName;
        unsigned int uiNumObjects;      // Includes active objects
        unsigned int uiNumActiveObjects;

            THSUniverse(): uiID(0), uiNumObjects(0), uiNumActiveObjects(0)
        {
        }
    };

  protected:

    std::list < THSUniverse > m_listUniverses;

    std::list < THSUniverse >::iterator m_iterCur;

    // Member Functions
  public:

    CHSPUniverseList()
    {
        m_ePacketType = PT_UNIVERSE_LIST;
    }
    // The server uses this to add a new universe.
    void AddUniverse(THSUniverse & rtClass);

    // Call this to get the first universe in the list.
    THSUniverse *GetFirstUniverse();

    // Call this to get all universes beyond the first.
    THSUniverse *GetNextUniverse();

    bool WriteToStream(CHSDataStream & rStream);
    bool ReadFromStream(CHSDataStream & rStream);

};


// Send this packet to delete an existing universe.
class CHSPDeleteUniverse:public CHSPacket
{
    // Member Variables
  public:

    unsigned int m_uiUniverseID;

  protected:

    // Member Functions
  public:

        CHSPDeleteUniverse(): m_uiUniverseID(0)
    {
        m_ePacketType = PT_DELETE_UNIVERSE;
    }

    bool WriteToStream(CHSDataStream & rStream);
    bool ReadFromStream(CHSDataStream & rStream);
};


// A response to a DeleteUniverse packet.
class CHSPDeleteUniverseResp:public CHSPacket
{
    // Member Variables
  public:

    bool m_bDeleted;            // True if the universe was deleted.

  protected:

    // Member Functions
  public:

    CHSPDeleteUniverseResp(): m_bDeleted(false)
    {
        m_ePacketType = PT_DELETE_UNIVERSE_RESPONSE;
    }

    bool WriteToStream(CHSDataStream & rStream);
    bool ReadFromStream(CHSDataStream & rStream);
};

// Send this packet to the server when you want to retrieve a list of objects.
// You will receive a CHSPObjectList packet in response.
class CHSPGetObjectList:public CHSPacket
{
    // Member Variables
  public:

    unsigned char m_ucObjectType;       // 0 for all objects (default), or specify the type of object.
    unsigned int m_uiUniverseID;        // 0 for all universes, or the universe ID you want.

  protected:

    // Member Functions
  public:

        CHSPGetObjectList(): m_ucObjectType(0), m_uiUniverseID(0)
    {
        m_ePacketType = PT_GET_OBJECT_LIST;
    }

    bool WriteToStream(CHSDataStream & rStream);
    bool ReadFromStream(CHSDataStream & rStream);
};

// This is sent to the client from the server in response to a CHSPGetObjectList.
// This packet will contain a list of object IDs, their types, names, and positions.
class CHSPObjectList:public CHSPacket
{
    // Member Variables
  public:

    bool m_bQuerySucceeded;

    struct THSObject
    {
        char cObjectType;
        unsigned int uiObjectID;
            std::string strObjectName;

        float fX;
        float fY;
        float fZ;
    };

  protected:

        std::list < THSObject > m_listObjects;

        std::list < THSObject >::iterator m_iterCur;

    // Member Functions
  public:

        CHSPObjectList(): m_bQuerySucceeded(true)
    {
        m_ePacketType = PT_OBJECT_LIST;
    }
    // The server uses this to add a new object to the packet.
    void AddObject(THSObject & rtObject);

    // Call this to get the first object in the list.
    THSObject *GetFirstObject();

    // Call this to get all objectes beyond the first.
    THSObject *GetNextObject();

    bool WriteToStream(CHSDataStream & rStream);
    bool ReadFromStream(CHSDataStream & rStream);

};


// Send this to the server when you want to retrieve a list of attributes and values for
// a specific object ID.  You will receive a CHSPObjectData packet in response.
class CHSPGetObjectData:public CHSPacket
{
    // Member Variables
  public:

    unsigned int m_uiObjectID;  // The object ID for which to retrieve the data.

  protected:

    // Member Functions
  public:

        CHSPGetObjectData(): m_uiObjectID(0)
    {
        m_ePacketType = PT_GET_OBJECT_DATA;
    }

    bool WriteToStream(CHSDataStream & rStream);
    bool ReadFromStream(CHSDataStream & rStream);
};

// Sent to the client in response to a CHSPGetObjectData packet.  This packet will
// contain a list of object attribute IDs and values.
class CHSPObjectData:public CHSPacket
{
    // Member Variables
  public:

    struct THSObjectAttribute
    {
        std::string strAttributeName;
        std::string strValue;
    };

    unsigned int m_uiObjectID;  // The object ID to which the data pertains.
    bool m_bBadQuery;           // True if the query failed.

  protected:

        std::list < THSObjectAttribute > m_listAttributes;      // Use GetFirstAttribute()/GetNextAttribute().
        std::list < THSObjectAttribute >::iterator m_iterCur;

    // Member Functions
  public:

        CHSPObjectData(): m_uiObjectID(0), m_bBadQuery(false)
    {
        m_ePacketType = PT_OBJECT_DATA;
    }

    // Use the GetFirstAttr/GetNextAttr to get attributes in the packet.
    THSObjectAttribute *GetFirstAttr();
    THSObjectAttribute *GetNextAttr();

    // The server uses this to add attributes to the packet.
    void AddAttribute(THSObjectAttribute & rtAttr);

    bool WriteToStream(CHSDataStream & rStream);
    bool ReadFromStream(CHSDataStream & rStream);
};


// Send this packet to the server when you want to modify one or more attributes of a
// object.  You should call AddAttribute() for each attribute you want to modify.
// A list of attributes is available in HSAttributes.h.
// Whether the request succeeds or fails, you will receive a CHSPSetObjectDataResponse.
class CHSPSetObjectData:public CHSPacket
{
    // Member Variables
  public:

    struct THSObjectAttribute
    {
        std::string strAttributeName;
        std::string strValue;
    };

    unsigned int m_uiObjectID;  // The object ID to which the data pertains.  You MUST set this!

  protected:

        std::list < THSObjectAttribute > m_listAttributes;      // Use GetFirstAttribute()/GetNextAttribute().
        std::list < THSObjectAttribute >::iterator m_iterCur;

    // Member Functions
  public:

        CHSPSetObjectData(): m_uiObjectID(0)
    {
        m_ePacketType = PT_SET_OBJECT_DATA;
    }

    // Call this function for each attribute that you want to modify.
    void AddAttribute(THSObjectAttribute & rtAttr);

    // These functions will be used by the server to retrieve attributes.
    THSObjectAttribute *GetFirstAttr();
    THSObjectAttribute *GetNextAttr();

    bool WriteToStream(CHSDataStream & rStream);
    bool ReadFromStream(CHSDataStream & rStream);
};

class CHSPSetObjectDataResponse:public CHSPacket
{
    // Member Variables
  public:

    unsigned int m_uiObjectID;  // The object ID that was being modified.
    bool m_bSuccess;            // True if the modifications passed.  Otherwise, false for any failures.

  protected:

    // Member Functions
  public:

        CHSPSetObjectDataResponse(): m_uiObjectID(0)
    {
        m_ePacketType = PT_SET_OBJECT_DATA_RESPONSE;
    }

    bool WriteToStream(CHSDataStream & rStream);
    bool ReadFromStream(CHSDataStream & rStream);
};

// Send this packet to the server when you want to retrieve a list of weapons.
// You will receive a CHSPWeaponList packet in response.
class CHSPGetWeaponList:public CHSPacket
{
    // Member Variables
  public:

  protected:

    // Member Functions
  public:

    CHSPGetWeaponList()
    {
        m_ePacketType = PT_GET_WEAPON_LIST;
    }

    bool WriteToStream(CHSDataStream & /* rStream */ )
    {
        return true;
    }
    bool ReadFromStream(CHSDataStream & /* rStream */ )
    {
        return true;
    }
};

// This is sent to the client from the server in response to a CHSPGetWeaponList.
// This packet will contain a list of weapon IDs, types and names.
class CHSPWeaponList:public CHSPacket
{
    // Member Variables
  public:

    struct THSWeapon
    {
        unsigned int uiWeaponID;
            std::string strName;
        unsigned int uiType;
    };

  protected:

        std::list < THSWeapon > m_listWeapons;

        std::list < THSWeapon >::iterator m_iterCur;

    // Member Functions
  public:

        CHSPWeaponList()
    {
        m_ePacketType = PT_WEAPON_LIST;
    }
    // The server uses this to add a new weapon.
    void AddWeapon(THSWeapon & rtWeapon);

    // Call this to get the first weapon in the list.
    THSWeapon *GetFirstWeapon();

    // Call this to get all weapons beyond the first.
    THSWeapon *GetNextWeapon();

    bool WriteToStream(CHSDataStream & rStream);
    bool ReadFromStream(CHSDataStream & rStream);

};


// Send this packet to the server with a new weapon name and type to have the
// weapon created.  You will receive a CHSPCreateWeaponResponse packet in response.
class CHSPCreateWeapon:public CHSPacket
{
    // Member Variables
  public:

    std::string m_strWeaponName;        // Set this to the name of the weapon you want to create.
    unsigned int m_uiType;      // The type of weapon to create.

  protected:

    // Member Functions
  public:

        CHSPCreateWeapon()
    {
        m_ePacketType = PT_CREATE_WEAPON;
    }

    bool WriteToStream(CHSDataStream & rStream);
    bool ReadFromStream(CHSDataStream & rStream);
};

// The server sends this packet to the client in response to a 
// CHSPCreateWeapon packet.
class CHSPCreateWeaponResponse:public CHSPacket
{
    // Member Variables
  public:

    bool m_bCreateSucceeded;    // True if the weapon was created.  False if any failure.

    unsigned int m_uiWeaponID;  // The ID of the weapon, if it was created.

  protected:

    // Member Functions
  public:

        CHSPCreateWeaponResponse(): m_bCreateSucceeded(true), m_uiWeaponID(0)
    {
        m_ePacketType = PT_CREATE_WEAPON_RESPONSE;
    }

    bool WriteToStream(CHSDataStream & rStream);
    bool ReadFromStream(CHSDataStream & rStream);
};


// Send this to the server when you want to retrieve a list of attributes and values for
// a specific weapon ID.  You will receive a CHSPWeaponData packet in response.
class CHSPGetWeaponData:public CHSPacket
{
    // Member Variables
  public:

    unsigned int m_uiWeaponID;  // The weapon ID for which to retrieve the data.

  protected:

    // Member Functions
  public:

        CHSPGetWeaponData(): m_uiWeaponID(0)
    {
        m_ePacketType = PT_GET_WEAPON_DATA;
    }

    bool WriteToStream(CHSDataStream & rStream);
    bool ReadFromStream(CHSDataStream & rStream);
};

// Sent to the client in response to a CHSPGetWeaponData packet.  This packet will
// contain a list of weapon attribute names and values.
class CHSPWeaponData:public CHSPacket
{
    // Member Variables
  public:

    struct THSWeaponAttribute
    {
        std::string strAttributeName;
        std::string strValue;
    };

    unsigned int m_uiWeaponID;  // The weapon ID to which the data pertains.
    bool m_bBadQuery;           // True if the query failed.

  protected:

        std::list < THSWeaponAttribute > m_listAttributes;      // Use GetFirstAttribute()/GetNextAttribute().
        std::list < THSWeaponAttribute >::iterator m_iterCur;

    // Member Functions
  public:

        CHSPWeaponData(): m_uiWeaponID(0), m_bBadQuery(false)
    {
        m_ePacketType = PT_WEAPON_DATA;
    }

    // Use the GetFirstAttr/GetNextAttr to get attributes in the packet.
    THSWeaponAttribute *GetFirstAttr();
    THSWeaponAttribute *GetNextAttr();

    // The server uses this to add attributes to the packet.
    void AddAttribute(THSWeaponAttribute & rtAttr);

    bool WriteToStream(CHSDataStream & rStream);
    bool ReadFromStream(CHSDataStream & rStream);
};

// Send this packet to the server when you want to modify one or more attributes of a
// weapon.  You should call AddAttribute() for each attribute you want to modify.
// A list of attributes is available in HSAttributes.h.
// Whether the request succeeds or fails, you will receive a CHSPSetWeaponDataResponse.
class CHSPSetWeaponData:public CHSPacket
{
    // Member Variables
  public:

    struct THSWeaponAttribute
    {
        std::string strAttributeName;
        std::string strValue;
    };

    unsigned int m_uiWeaponID;  // The Weapon ID to which the data pertains.  You MUST set this!

  protected:

        std::list < THSWeaponAttribute > m_listAttributes;      // Use GetFirstAttribute()/GetNextAttribute().
        std::list < THSWeaponAttribute >::iterator m_iterCur;

    // Member Functions
  public:

        CHSPSetWeaponData(): m_uiWeaponID(0)
    {
        m_ePacketType = PT_SET_WEAPON_DATA;
    }

    // Call this function for each attribute that you want to modify.
    void AddAttribute(THSWeaponAttribute & rtAttr);

    // These functions will be used by the server to retrieve attributes.
    THSWeaponAttribute *GetFirstAttr();
    THSWeaponAttribute *GetNextAttr();

    bool WriteToStream(CHSDataStream & rStream);
    bool ReadFromStream(CHSDataStream & rStream);
};

class CHSPSetWeaponDataResponse:public CHSPacket
{
    // Member Variables
  public:

    unsigned int m_uiWeaponID;  // The Weapon ID that was being modified.
    bool m_bSuccess;            // True if the modifications passed.  Otherwise, false for any failures.

  protected:

    // Member Functions
  public:

        CHSPSetWeaponDataResponse(): m_uiWeaponID(0)
    {
        m_ePacketType = PT_SET_WEAPON_DATA_RESPONSE;
    }

    bool WriteToStream(CHSDataStream & rStream);
    bool ReadFromStream(CHSDataStream & rStream);
};

#endif // __HSPACKETS_H__
