// -----------------------------------------------------------------------
// $Id: HSHandlerClass.cpp,v 1.8 2006/06/17 03:31:48 mark Exp $
// -----------------------------------------------------------------------

#include "pch.h"
#include "hsutils.h"
#include "HSPackets.h"
#include "hsserver.h"
#include "hsclass.h"
#include "HSNetwork.h"
#include "hsuniverse.h"
#include "hsuniversedb.h"
#include "hsobjects.h"

#include "HSHandlerClass.h"


HS_BOOL8 CHSHandlerClass::Initialize()
{
    // Register handlers.
    REGISTER_PACKET_HANDLER(PT_GET_CLASS_LIST,
                            &CHSHandlerClass::HandleGetClassList, true);
    REGISTER_PACKET_HANDLER(PT_GET_CLASS_DATA,
                            &CHSHandlerClass::HandleGetClassData, true);
    REGISTER_PACKET_HANDLER(PT_SET_CLASS_DATA,
                            &CHSHandlerClass::HandleSetClassData, true);
    REGISTER_PACKET_HANDLER(PT_CREATE_CLASS,
                            &CHSHandlerClass::HandleCreateClass, true);
    REGISTER_PACKET_HANDLER(PT_DELETE_CLASS,
                            &CHSHandlerClass::HandleDeleteClass, true);

    return true;
}

void CHSHandlerClass::HandleGetClassList(CHSPacket * pPacket)
{
    hs_log("ADMIN SERVER: Handle packet GetClassList");
    CHSPGetClassList *cmdGetList =
        static_cast < CHSPGetClassList * >(pPacket);

    // Create a class list packet as a response.
    CHSPClassList cmdList;
    cmdList.SetPacketAddress(cmdGetList->GetPacketAddress());

    THSShipClassIterator tIter;
    HS_BOOL8 bContinue;

    for (bContinue = CHSClassDB::GetInstance().GetFirstClass(tIter);
         bContinue; bContinue = CHSClassDB::GetInstance().GetNextClass(tIter))
    {
        if (tIter.pValue != NULL)
        {
            CHSShipClass *pClass = (CHSShipClass *) tIter.pValue;

            CHSPClassList::THSClass tClass;

            tClass.uiClassID = pClass->Id();
            tClass.strClassName = pClass->ClassName();

            cmdList.AddClass(tClass);
        }
    }

    HSNetwork.SendPacket(cmdList);
}

void CHSHandlerClass::HandleGetClassData(CHSPacket * pPacket)
{
    hs_log("ADMIN SERVER: Handle packet GetClassData");
    CHSPGetClassData *cmdGetData =
        static_cast < CHSPGetClassData * >(pPacket);

    // Create a response.
    CHSPClassData cmdData;
    cmdData.SetPacketAddress(cmdGetData->GetPacketAddress());
    cmdData.m_uiClassID = cmdGetData->m_uiClassID;

    // Find the class in question.
    CHSShipClass *pClass =
        CHSClassDB::GetInstance().GetClass(cmdGetData->m_uiClassID);
    if (!pClass)
    {
        cmdData.m_bBadQuery = true;
    }
    else
    {
        // Got the class.  Add all of the attributes.
        CHSPClassData::THSClassAttribute tAttribute;

        // Class ID.
        tAttribute.strAttributeName = "ID";
        tAttribute.varValue = pClass->Id();
        cmdData.AddAttribute(tAttribute);

        // Class name.
        tAttribute.strAttributeName = "NAME";
        tAttribute.varValue = pClass->ClassName();
        cmdData.AddAttribute(tAttribute);

        // Class size.
        tAttribute.strAttributeName = "SIZE";
        tAttribute.varValue = pClass->Size();
        cmdData.AddAttribute(tAttribute);

        // Cargo size.
        tAttribute.strAttributeName = "CARGO SIZE";
        tAttribute.varValue = pClass->CargoSize();
        cmdData.AddAttribute(tAttribute);

        // Min manned.
        tAttribute.strAttributeName = "MINMANNED";
        tAttribute.varValue = pClass->MinCrew();
        cmdData.AddAttribute(tAttribute);

        // Maxhull.
        tAttribute.strAttributeName = "MAXHULL";
        tAttribute.varValue = pClass->MaxHull();
        cmdData.AddAttribute(tAttribute);

        // Can drop.
        tAttribute.strAttributeName = "CAN DROP";
        tAttribute.varValue = pClass->CanDrop();
        cmdData.AddAttribute(tAttribute);

        // Space dock.
        tAttribute.strAttributeName = "SPACE DOCK";
        tAttribute.varValue = pClass->SpaceDock();
        cmdData.AddAttribute(tAttribute);
    }

    HSNetwork.SendPacket(cmdData);
}

void CHSHandlerClass::HandleSetClassData(CHSPacket * pPacket)
{
    CHSPSetClassData *cmdSetData =
        static_cast < CHSPSetClassData * >(pPacket);
    CHSPSetClassDataResponse cmdResponse;

    cmdResponse.SetPacketAddress(cmdSetData->GetPacketAddress());
    cmdResponse.m_uiClassID = cmdSetData->m_uiClassID;

    // Find the class in question.
    CHSShipClass *pClass =
        CHSClassDB::GetInstance().GetClass(cmdSetData->m_uiClassID);
    if (!pClass)
    {
        cmdResponse.m_bSuccess = false;
    }
    else
    {
        cmdResponse.m_bSuccess = true;

        // Run through the attributes contained in the packet.
        CHSPSetClassData::THSClassAttribute * pAttribute;

        for (pAttribute = cmdSetData->GetFirstAttr(); pAttribute;
             pAttribute = cmdSetData->GetNextAttr())
        {
            if (pAttribute->strAttributeName == "NAME")
            {
                pClass->ClassName(pAttribute->strValue);
            }
            else if (pAttribute->strAttributeName == "SIZE")
            {
                HS_UINT32 uiValue = atoi(pAttribute->strValue.c_str());
                if (uiValue < 1)
                {
                    cmdResponse.m_bSuccess = false;
                }

                pClass->Size(uiValue);
            }
            else if (pAttribute->strAttributeName == "MAXHULL")
            {
                HS_UINT32 uiValue = atoi(pAttribute->strValue.c_str());
                pClass->MaxHull(uiValue);
            }
            else if (pAttribute->strAttributeName == "CAN DROP")
            {
                HS_UINT32 uiValue = atoi(pAttribute->strValue.c_str());
                pClass->CanDrop(uiValue == 0 ? false : true);
            }
            else if (pAttribute->strAttributeName == "SPACE DOCK")
            {
                HS_UINT32 uiValue = atoi(pAttribute->strValue.c_str());
                pClass->SpaceDock(uiValue == 0 ? false : true);
            }
            else if (pAttribute->strAttributeName == "CARGO SIZE")
            {
                HS_UINT32 uiValue = atoi(pAttribute->strValue.c_str());
                pClass->CargoSize(uiValue);
            }
            else if (pAttribute->strAttributeName == "MINMANNED")
            {
                HS_UINT32 uiValue = atoi(pAttribute->strValue.c_str());
                pClass->MinCrew(uiValue);
            }
            else
            {
                cmdResponse.m_bSuccess = false;
            }
        }
    }

    HSNetwork.SendPacket(cmdResponse);
}


void CHSHandlerClass::HandleCreateClass(CHSPacket * pPacket)
{
    CHSPCreateClass *cmdCreate = static_cast < CHSPCreateClass * >(pPacket);
    CHSPCreateClassResponse cmdResponse;

    cmdResponse.SetPacketAddress(cmdCreate->GetPacketAddress());

    // Is the class name valid?

    // Must specify a name
    if (cmdCreate->m_strClassName.length() > 0)
    {

        // Good name.  Create the class.
        CHSShipClass *pNewClass =
            new CHSShipClass(CHSClassDB::GetInstance().GetNextClassID(true));
        if (!pNewClass)
        {
            cmdResponse.m_bCreateSucceeded = false;
        }
        else
        {
            // Set size to default of 1
            pNewClass->Size(1);

            // Set the name
            pNewClass->ClassName(cmdCreate->m_strClassName);

            if (CHSClassDB::GetInstance().AddClass(pNewClass))
            {
                cmdResponse.m_bCreateSucceeded = true;
                cmdResponse.m_uiClassID = pNewClass->Id();
            }
            else
            {
                cmdResponse.m_bCreateSucceeded = false;
            }
        }
    }
    else
    {
        cmdResponse.m_bCreateSucceeded = false;
    }

    HSNetwork.SendPacket(cmdResponse);
}

void CHSHandlerClass::HandleDeleteClass(CHSPacket * pPacket)
{
    CHSPDeleteClass *pDelete = static_cast < CHSPDeleteClass * >(pPacket);
    CHSPDeleteClassResponse cmdResponse;

    cmdResponse.SetPacketAddress(pDelete->GetPacketAddress());
    cmdResponse.m_bDeleted = true;

    // Check to see if any ships remain with that class.
    // If so, don't allow it to be deleted.
    THSUniverseIterator tIter;
    HS_BOOL8 bIter;
    for (bIter = CHSUniverseDB::GetInstance().GetFirstUniverse(tIter); bIter;
         bIter = CHSUniverseDB::GetInstance().GetNextUniverse(tIter))
    {
        CHSUniverse *pUniverse = tIter.pValue;

        // Search ships in the universe
        THSObjectIterator tIterator;
        HS_BOOL8 bContinue;
        for (bContinue = pUniverse->GetFirstObject(tIterator, HST_SHIP);
             bContinue;
             bContinue = pUniverse->GetNextObject(tIterator, HST_SHIP))
        {
            if (tIterator.pValue)
            {
                CHSShip *pShip = static_cast < CHSShip * >(tIterator.pValue);
                if (pShip->ClassNum() == pDelete->m_uiClassID)
                {
                    // This class is still in use.
                    cmdResponse.m_bDeleted = false;
                    break;
                }
            }
        }

        if (bContinue)
        {
            // Inner loop exited prematurely.
            break;
        }
    }

    // Can we delete this class?
    if (cmdResponse.m_bDeleted)
    {
        cmdResponse.m_bDeleted =
            CHSClassDB::GetInstance().RemoveClass(pDelete->m_uiClassID);
    }

    HSNetwork.SendPacket(cmdResponse);
}
