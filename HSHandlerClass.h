// -----------------------------------------------------------------------
// $Id: HSHandlerClass.h,v 1.3 2006/04/04 12:37:11 mark Exp $
// -----------------------------------------------------------------------

#if !defined(__HSHANDLERCLASS_H__)
#define __HSHANDLERCLASS_H__


// Library Includes
#include "HSPacketHandler.h"

//! Admin port handler for processing ship class information
class CHSHandlerClass:public CHSPacketHandler
{
  public:

    CHSHandlerClass()
    {
    }
       ~CHSHandlerClass()
    {
    }

    HS_BOOL8 Initialize();

    void HandleGetClassList(CHSPacket * pPacket);
    void HandleGetClassData(CHSPacket * pPacket);
    void HandleSetClassData(CHSPacket * pPacket);
    void HandleCreateClass(CHSPacket * pPacket);
    void HandleDeleteClass(CHSPacket * pPacket);

  protected:

};
#endif // __HSHANDLERCLASS_H__
