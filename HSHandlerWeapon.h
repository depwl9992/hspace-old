#ifdef _WIN32
#pragma once
#endif

#if !defined(__HSHANDLERWEAPON_H__)
#define __HSHANDLERWEAPON_H__


// Library Includes
#include "HSPacketHandler.h"

// Local Includes

// Forward Declarations

// Types

// Constants

// Prototypes

class CHSHandlerWeapon:public CHSPacketHandler
{
    // Member Variables
  public:

  protected:

    // Member Functions
  public:

    CHSHandlerWeapon()
    {
    }

       ~CHSHandlerWeapon()
    {
    }

    HS_BOOL8 Initialize();

    void HandleGetWeaponList(CHSPacket * pPacket);
    void HandleGetWeaponData(CHSPacket * pPacket);
    void HandleSetWeaponData(CHSPacket * pPacket);
    void HandleCreateWeapon(CHSPacket * pPacket);
    void HandleDeleteWeapon(CHSPacket * pPacket);

  protected:

};
#endif // __HSHANDLERWEAPON_H__
