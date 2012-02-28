// -----------------------------------------------------------------------
// $Id: HSHandlerLogin.h,v 1.3 2006/04/04 12:37:11 mark Exp $
// -----------------------------------------------------------------------

#if !defined(__HSHANDLERLOGIN_H__)
#define __HSHANDLERLOGIN_H__


// Library Includes
#include "HSPacketHandler.h"

//! Login handler for the HSpace Admin Server
class CHSHandlerLogin:public CHSPacketHandler
{
    // Member Functions
  public:

    //! Default constructor ... does nothing
    CHSHandlerLogin()
    {
    }

    //! Deconstructor ... does nothing
       ~CHSHandlerLogin()
    {
    }

    //! Registers the HandleLogin packet handler
    HS_BOOL8 Initialize();

    //! Process a login request
    void HandleLogin(CHSPacket * pPacket);

  protected:

};
#endif // __HSHANDLERLOGIN_H__
