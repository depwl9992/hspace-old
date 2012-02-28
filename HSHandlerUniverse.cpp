// -----------------------------------------------------------------------
// $Id: HSHandlerUniverse.cpp,v 1.3 2006/04/04 12:37:11 mark Exp $
// -----------------------------------------------------------------------

#include "pch.h"

#include "hsutils.h"
#include "HSPackets.h"
#include "hsserver.h"
#include "hsuniversedb.h"
#include "hsuniverse.h"
#include "HSNetwork.h"

#include "HSHandlerUniverse.h"


HS_BOOL8 CHSHandlerUniverse::Initialize()
{
    // Register handlers.
    REGISTER_PACKET_HANDLER(PT_GET_UNIVERSE_LIST,
                            &CHSHandlerUniverse::HandleGetUniverseList, true);
    REGISTER_PACKET_HANDLER(PT_DELETE_UNIVERSE,
                            &CHSHandlerUniverse::HandleDeleteUniverse, false);

    return true;
}


void CHSHandlerUniverse::HandleGetUniverseList(CHSPacket * pPacket)
{
    hs_log("ADMIN SERVER: Handle packet GetUniverseList");
    CHSPGetUniverseList *cmdGetList =
        static_cast < CHSPGetUniverseList * >(pPacket);

    // Create a class list packet as a response.
    CHSPUniverseList cmdList;
    cmdList.SetPacketAddress(cmdGetList->GetPacketAddress());

    THSUniverseIterator tIter;
    HS_BOOL8 bContinue;

    for (bContinue = CHSUniverseDB::GetInstance().GetFirstUniverse(tIter);
         bContinue;
         bContinue = CHSUniverseDB::GetInstance().GetNextUniverse(tIter))
    {
        if (tIter.pValue != NULL)
        {
            CHSUniverse *pUniverse = (CHSUniverse *) tIter.pValue;

            CHSPUniverseList::THSUniverse tEntry;

            tEntry.uiID = pUniverse->GetID();
            tEntry.strName = pUniverse->GetName();
            tEntry.uiNumObjects = pUniverse->GetNumObjects();
            tEntry.uiNumActiveObjects = pUniverse->GetNumActiveObjects();

            cmdList.AddUniverse(tEntry);
        }
    }

    HSNetwork.SendPacket(cmdList);
}


void CHSHandlerUniverse::HandleDeleteUniverse(CHSPacket * pPacket)
{
    hs_log("ADMIN SERVER: Handle packet DeleteUniverse");
    CHSPDeleteUniverse *cmdDelete =
        static_cast < CHSPDeleteUniverse * >(pPacket);

    // Create a class list packet as a response.
    CHSPDeleteUniverseResp cmdResponse;
    cmdResponse.SetPacketAddress(cmdDelete->GetPacketAddress());

    cmdResponse.m_bDeleted =
        CHSUniverseDB::GetInstance().DeleteUniverse(cmdDelete->
                                                    m_uiUniverseID);

    HSNetwork.SendPacket(cmdResponse);
}
