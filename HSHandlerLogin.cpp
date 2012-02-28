// -----------------------------------------------------------------------
// $Id: HSHandlerLogin.cpp,v 1.8 2006/04/04 12:37:11 mark Exp $
// -----------------------------------------------------------------------

#include "pch.h"
#include "hsutils.h"
#include "hsflags.h"            // for validating player has HSPACE_ADMIN flag
#include "HSPackets.h"
#include "hsserver.h"
#include "hsinterface.h"
#include "HSNetwork.h"
#include "hsflags.h"

#include "HSHandlerLogin.h"


HS_BOOL8 CHSHandlerLogin::Initialize()
{
    // Register handlers.
    REGISTER_PACKET_HANDLER(PT_LOGIN, &CHSHandlerLogin::HandleLogin, false);
    return true;
}


void CHSHandlerLogin::HandleLogin(CHSPacket * pPacket)
{
    hs_log("HandlePacket: Login");
    char errstr[256];

    // This is a login packet.
    CHSPLogin *pLogin = static_cast < CHSPLogin * >(pPacket);
    CHSPLoginResponse cmdResponse;
    cmdResponse.SetPacketAddress(pLogin->GetPacketAddress());

    HS_DBREF dbPlayer;

    // Check for a valid player name
    if ((dbPlayer = hsInterface.LookupPlayer(pLogin->m_pcPlayerName))
        != HSNOTHING)
    {
        // Validate the login password for the player
        if (hsInterface.
            ValidatePlayerPassword(dbPlayer, pLogin->m_pcPassword))
        {
            // Does this player have wizard permissions 
            // or for PennMUSH the HSPACE_ADMIN flag?
#if defined(TM3) || defined(MUX)
            if (hsInterface.IsWizard(dbPlayer))
#else
            if (hsInterface.IsWizard(dbPlayer) ||
                hsInterface.HasFlag(dbPlayer,
                                    TYPE_PLAYER, PLAYER_HSPACE_ADMIN))
#endif
            {
                // Looks good!
                CHSServer::GetInstance().AddValidatedPlayer(pPacket->
                                                            GetPacketAddress
                                                            ());
                cmdResponse.m_bLoggedIn = true;

                HSNetwork.SendPacket(cmdResponse);
                return;
            }
            else
            {
                sprintf_s(errstr, "HandleLogin %s does not have a Wizard flag!",
                        pLogin->m_pcPassword);
                hs_log(errstr);
            }
        }
        else
        {
            sprintf_s(errstr,
                    "HandleLogin Login Failed! Could not validate password for %s.",
                    pLogin->m_pcPlayerName);
            hs_log(errstr);
        }
    }
    else
    {
        sprintf_s(errstr, "HandleLogin No such player '%s'.",
                pLogin->m_pcPlayerName);
        hs_log(errstr);
    }
}
