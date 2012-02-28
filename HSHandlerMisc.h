// -----------------------------------------------------------------------
// $Id: HSHandlerMisc.h,v 1.3 2006/04/04 12:37:11 mark Exp $
// -----------------------------------------------------------------------

#if !defined(__HSHANDLERMISC_H__)
#define __HSHANDLERMISC_H__

#include "HSPacketHandler.h"

//! Implement the Admin Port the ability to list systems, get system
//! data, or set system data.   Systems may also be added or deleted
//! by this class through  the admin port.
class CHSHandlerMisc:public CHSPacketHandler
{
  public:

    //! Bare constructor
    CHSHandlerMisc()
    {
    }

    //! Bare deconstructor
       ~CHSHandlerMisc()
    {
    }

    //! Register packet handler methods
    HS_BOOL8 Initialize();

    //! Process a system list request
    void HandleGetSystemList(CHSPacket * pPacket);

    //! Retrieve system information for a specific system
    void HandleGetSystemData(CHSPacket * pPacket);

    //! Set system data 
    void HandleSetSystemData(CHSPacket * pPacket);

    //! Add a system
    void HandleAddSystem(CHSPacket * pPacket);

    //! Process a system deletion request.
    void HandleDeleteSystem(CHSPacket * pPacket);

  protected:

};
#endif // __HSHANDLERMISC_H__
