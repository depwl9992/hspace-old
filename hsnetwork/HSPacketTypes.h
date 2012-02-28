#ifdef _WIN32
#pragma once
#endif

#if !defined(__HSPACKETTYPES_H__)
#define __HSPACKETTYPES_H__

// These are all of the packet types available.  These are just
// type type identifiers.  The actual packets are in HSPackets.h
enum EHSPacketType
{
    PT_INVALID = 0,             // Do not use.
    PT_CONNECTION_CLOSED,       // A connection has closed.  If you're a client, the connection to the server is closed.
    PT_LOGIN,                   // A remote client wants to login to the server.
    PT_LOGIN_RESPONSE,          // A response from the server to a request to login.
    PT_GET_CLASS_LIST,          // A remote client wants a list of ship classes
    PT_CLASS_LIST,              // A list of classes was sent to a remote client
    PT_GET_CLASS_DATA,          // A remote client wants a list of attribute/values for a given class.
    PT_CLASS_DATA,              // Data on a given class was sent to a remote client
    PT_SET_CLASS_DATA,          // A remote client wants to modify a class on the server
    PT_SET_CLASS_DATA_RESPONSE, // The server is responding to a PT_SET_CLASS_DATA packet
    PT_GET_SYSTEM_LIST,         // A remote client wants to retrieve a list of systems
    PT_SYSTEM_LIST,             // A list of systems is being sent to a client.
    PT_CREATE_CLASS,            // A remote client wants to create a new class on the server.
    PT_CREATE_CLASS_RESPONSE,   // A response from the server to a PT_CREATE_CLASS packet
    PT_GET_SYSTEM_DATA,         // A remote client wants a list of attributes on an engineering system.
    PT_SYSTEM_DATA,             // The server is sending a list of attributes for an engineering system.
    PT_SET_SYSTEM_DATA,         // A remote client wants to modify an engineering system on the server.
    PT_SET_SYSTEM_DATA_RESPONSE,        // The server is responding to a PT_SET_SYSTEM_DATA packet
    PT_DELETE_CLASS,            // A remote client wants to delete a class on the server.
    PT_DELETE_CLASS_RESPONSE,   // The server is responding to a PT_DELETE_CLASS packet.
    PT_ADD_SYSTEM,              // A remote client wants to add an engineering system to an object or class.
    PT_ADD_SYSTEM_RESPONSE,     // The server is responding to a PT_ADD_SYSTEM packet.
    PT_DELETE_SYSTEM,           // A remote client wants to delete an engineering system from an object or class.
    PT_DELETE_SYSTEM_RESPONSE,  // The server is responding to a PT_DELETE_SYSTEM packet.
    PT_GET_UNIVERSE_LIST,       // A remote client wants a list of universes.
    PT_UNIVERSE_LIST,           // The server is sending a universe list.
    PT_DELETE_UNIVERSE,         // A remote client wants to delete a universe.
    PT_DELETE_UNIVERSE_RESPONSE,        // A response to a PT_DELETE_UNIVERSE packet.
    PT_GET_OBJECT_LIST,         // A remote client wants a list of objects.
    PT_OBJECT_LIST,             // The server is sending an object list
    PT_GET_OBJECT_DATA,         // A remote client wants a list of attribute/values for a given object.
    PT_OBJECT_DATA,             // Data on a given object was sent to a remote client
    PT_SET_OBJECT_DATA,         // A remote client wants to change one or more attributes of an object.
    PT_SET_OBJECT_DATA_RESPONSE,        // The server is responding to a PT_SET_OBJECT_DATA packet
    PT_GET_WEAPON_LIST,         // A remote client wants a list of weapons
    PT_WEAPON_LIST,             // The server is sending a weapon list.
    PT_CREATE_WEAPON,           // A remote client wants to create a new weapon on the server.
    PT_CREATE_WEAPON_RESPONSE,  // A response from the server to a PT_CREATE_WEAPON packet
    PT_GET_WEAPON_DATA,         // A remote client wants a list of attribute/values for a given weapon.
    PT_WEAPON_DATA,             // Data on a given weapon was sent to a remote client
    PT_SET_WEAPON_DATA,         // A remote client wants to change one or more attributes of a weapon.
    PT_SET_WEAPON_DATA_RESPONSE,        // The server is responding to a PT_SET_WEAPON_DATA packet

    PT_MAX_PACKET_VALUE
};


#endif // __HSPACKETTYPES_H__
