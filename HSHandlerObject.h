// -----------------------------------------------------------------------
// $Id: HSHandlerObject.h,v 1.4 2006/04/04 12:56:10 mark Exp $
// -----------------------------------------------------------------------

#if !defined(__HSHANDLEROBJECT_H__)
#define __HSHANDLEROBJECT_H__

#include "HSPacketHandler.h"

//! Admin server handler class for processing object data via list,
//! set and get.
class CHSHandlerObject:public CHSPacketHandler
{
  public:
    //! Default constructor
    CHSHandlerObject()
    {
    }

    //! Default deconstructor
    virtual ~CHSHandlerObject()
    {
    }

    //! Registers packet handlers
    HS_BOOL8 Initialize();

    //! Process requests for Object List
    void HandleGetObjectList(CHSPacket * pPacket);

    //! Process request to get object data
    void HandleGetObjectData(CHSPacket * pPacket);

    //! Process request to set object data
    void HandleSetObjectData(CHSPacket * pPacket);

  protected:

};
#endif // __HSHANDLERCLASS_H__
