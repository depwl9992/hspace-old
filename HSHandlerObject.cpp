// -----------------------------------------------------------------------
// $Id: HSHandlerObject.cpp,v 1.4 2006/04/04 12:37:11 mark Exp $
// -----------------------------------------------------------------------

#include "pch.h"
#include "hsutils.h"
#include "HSPackets.h"
#include "hsserver.h"
#include "HSNetwork.h"
#include "hsobjects.h"
#include "hsuniversedb.h"
#include "hsuniverse.h"
#include "hsdb.h"

#include "HSHandlerObject.h"


HS_BOOL8 CHSHandlerObject::Initialize()
{
    // Register handlers.
    REGISTER_PACKET_HANDLER(PT_GET_OBJECT_LIST,
                            &CHSHandlerObject::HandleGetObjectList, true);
    REGISTER_PACKET_HANDLER(PT_GET_OBJECT_DATA,
                            &CHSHandlerObject::HandleGetObjectData, true);
    REGISTER_PACKET_HANDLER(PT_SET_OBJECT_DATA,
                            &CHSHandlerObject::HandleSetObjectData, true);

    return true;
}

void CHSHandlerObject::HandleGetObjectList(CHSPacket * pPacket)
{
    hs_log("ADMIN SERVER: Handle packet GetObjectList");
    CHSPGetObjectList *cmdGetList =
        static_cast < CHSPGetObjectList * >(pPacket);

    // Create an object list packet as a response.
    CHSPObjectList cmdList;
    cmdList.SetPacketAddress(cmdGetList->GetPacketAddress());

    // Get objects from a specific universe?
    if (cmdGetList->m_uiUniverseID != 0)
    {
        CHSUniverse *pUniverse =
            CHSUniverseDB::GetInstance().FindUniverse(cmdGetList->
                                                      m_uiUniverseID);

        if (pUniverse)
        {
            cmdList.m_bQuerySucceeded = true;

            THSObjectIterator tIter;
            HS_BOOL8 bContinue;
            for (bContinue =
                 pUniverse->GetFirstObject(tIter,
                                           (HS_TYPE) cmdGetList->
                                           m_ucObjectType); bContinue;
                 bContinue =
                 pUniverse->GetNextObject(tIter,
                                          (HS_TYPE) cmdGetList->
                                          m_ucObjectType))
            {
                if (tIter.pValue != NULL)
                {
                    CHSPObjectList::THSObject tObject;

                    tObject.uiObjectID = tIter.pValue->GetDbref();
                    tObject.strObjectName = tIter.pValue->GetName();
                    tObject.fX = tIter.pValue->GetX();
                    tObject.fY = tIter.pValue->GetY();
                    tObject.fZ = tIter.pValue->GetZ();

                    cmdList.AddObject(tObject);
                }
            }
        }
        else
        {
            cmdList.m_bQuerySucceeded = false;
        }
    }
    else
    {
        // All universes.
        cmdList.m_bQuerySucceeded = true;

        THSUniverseIterator tUnivIter;
        HS_BOOL8 bUnivContinue;

        for (bUnivContinue =
             CHSUniverseDB::GetInstance().GetFirstUniverse(tUnivIter);
             bUnivContinue;
             bUnivContinue =
             CHSUniverseDB::GetInstance().GetNextUniverse(tUnivIter))
        {
            CHSUniverse *pUniverse = tUnivIter.pValue;

            if (!pUniverse)
            {
                continue;
            }

            THSObjectIterator tIter;
            HS_BOOL8 bContinue;

            for (bContinue = pUniverse->GetFirstObject(tIter,
                                                       (HS_TYPE) cmdGetList->
                                                       m_ucObjectType);
                 bContinue;
                 bContinue =
                 pUniverse->GetNextObject(tIter,
                                          (HS_TYPE) cmdGetList->
                                          m_ucObjectType))
            {
                if (tIter.pValue != NULL)
                {
                    CHSPObjectList::THSObject tObject;

                    tObject.cObjectType = (char) tIter.pValue->GetType();
                    tObject.uiObjectID = tIter.pValue->GetDbref();
                    tObject.strObjectName = tIter.pValue->GetName();
                    tObject.fX = tIter.pValue->GetX();
                    tObject.fY = tIter.pValue->GetY();
                    tObject.fZ = tIter.pValue->GetZ();

                    cmdList.AddObject(tObject);
                }
            }
        }
    }

    HSNetwork.SendPacket(cmdList);
}


void CHSHandlerObject::HandleGetObjectData(CHSPacket * pPacket)
{
    hs_log("ADMIN SERVER: Handle packet GetObjectData");

    CHSPGetObjectData *cmdGetData =
        static_cast < CHSPGetObjectData * >(pPacket);

    // Create a response.
    CHSPObjectData cmdData;
    cmdData.SetPacketAddress(cmdGetData->GetPacketAddress());
    cmdData.m_uiObjectID = cmdGetData->m_uiObjectID;

    // Find the object in question.
    CHS3DObject *pObject = dbHSDB.FindObject(cmdGetData->m_uiObjectID);
    if (!pObject)
    {
        cmdData.m_bBadQuery = true;
    }
    else
    {
        // Got the object.  Add all of the attributes.
        CHSAttributeList listAttributes;
        pObject->GetAttributeList(listAttributes);

        while (!listAttributes.empty())
        {
            std::string & rstrAttribute = listAttributes.front();

            CHSPObjectData::THSObjectAttribute tAttribute;

            tAttribute.strAttributeName = rstrAttribute;
            tAttribute.strValue = pObject->GetAttributeValue((HS_INT8 *)
                                                             rstrAttribute.
                                                             c_str());

            cmdData.AddAttribute(tAttribute);

            listAttributes.pop_front();
        }
    }

    HSNetwork.SendPacket(cmdData);
}


void CHSHandlerObject::HandleSetObjectData(CHSPacket * pPacket)
{
    hs_log("ADMIN SERVER: Handle packet SetObjectData");
    CHSPSetObjectData *cmdSetData =
        static_cast < CHSPSetObjectData * >(pPacket);
    CHSPSetObjectDataResponse cmdResponse;

    cmdResponse.SetPacketAddress(cmdSetData->GetPacketAddress());
    cmdResponse.m_uiObjectID = cmdSetData->m_uiObjectID;

    // Find the object in question.
    CHS3DObject *pObject = dbHSDB.FindObject(cmdSetData->m_uiObjectID);
    if (!pObject)
    {
        cmdResponse.m_bSuccess = false;
    }
    else
    {
        cmdResponse.m_bSuccess = true;

        // Run through the attributes contained in the packet.
        CHSPSetObjectData::THSObjectAttribute * pAttribute;

        for (pAttribute = cmdSetData->GetFirstAttr(); pAttribute;
             pAttribute = cmdSetData->GetNextAttr())
        {
            if (!pObject->SetAttributeValue((HS_INT8 *)
                                            pAttribute->strAttributeName.
                                            c_str(),
                                            (HS_INT8 *) pAttribute->strValue.
                                            c_str()))
            {
                // Record failure, but continue.
                cmdResponse.m_bSuccess = false;
            }
        }
    }

    HSNetwork.SendPacket(cmdResponse);
}
