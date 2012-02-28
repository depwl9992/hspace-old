// -----------------------------------------------------------------------
// $Id: HSHandlerUniverse.h,v 1.4 2006/04/04 12:56:10 mark Exp $
// -----------------------------------------------------------------------

#ifdef _WIN32
#pragma once
#endif

#if !defined(__HSHANDLERUNIVERSE_H__)
#define __HSHANDLERUNIVERSE_H__

#include "HSPacketHandler.h"

class CHSHandlerUniverse:public CHSPacketHandler
{
  public:

    CHSHandlerUniverse()
    {
    }

    virtual ~CHSHandlerUniverse()
    {
    }

    HS_BOOL8 Initialize();

    void HandleGetUniverseList(CHSPacket * pPacket);
    void HandleDeleteUniverse(CHSPacket * pPacket);

};
#endif // __HSHANDLERUNIVERSE_H__
