// -----------------------------------------------------------------------
// $Id: HSHandlerMisc.cpp,v 1.4 2006/04/04 12:37:11 mark Exp $
// -----------------------------------------------------------------------

#include "pch.h"

#include "hsutils.h"
#include "HSPackets.h"
#include "hsserver.h"
#include "hsclass.h"
#include "hseng.h"
#include "HSNetwork.h"
#include "hsuniverse.h"
#include "hsuniversedb.h"
#include "HSHandlerMisc.h"


HS_BOOL8 CHSHandlerMisc::Initialize()
{
    // Register handlers.
    REGISTER_PACKET_HANDLER(PT_GET_SYSTEM_LIST,
                            &CHSHandlerMisc::HandleGetSystemList, true);
    REGISTER_PACKET_HANDLER(PT_GET_SYSTEM_DATA,
                            &CHSHandlerMisc::HandleGetSystemData, true);
    REGISTER_PACKET_HANDLER(PT_SET_SYSTEM_DATA,
                            &CHSHandlerMisc::HandleSetSystemData, true);
    REGISTER_PACKET_HANDLER(PT_ADD_SYSTEM,
                            &CHSHandlerMisc::HandleAddSystem, true);
    REGISTER_PACKET_HANDLER(PT_DELETE_SYSTEM,
                            &CHSHandlerMisc::HandleDeleteSystem, true);
    return true;
}

void CHSHandlerMisc::HandleGetSystemList(CHSPacket * pPacket)
{
    CHSPGetSystemList *cmdGetList =
        static_cast < CHSPGetSystemList * >(pPacket);

    // Create a system list packet as a response.
    CHSPSystemList cmdList;
    cmdList.SetPacketAddress(cmdGetList->GetPacketAddress());

    CHSSystemArray *pSysArray = NULL;

    // Is this is a shipclass query, find that class.
    if (cmdGetList->m_bClass)
    {
        CHSShipClass *pClass =
            CHSClassDB::GetInstance().GetClass(cmdGetList->
                                               m_uiClassOrObjectID);

        if (!pClass)
        {
            cmdList.m_bQuerySucceeded = false;
        }
        else
        {
            pSysArray = pClass->m_pSystems;
        }
    }
    else
    {
        // Find the hspace object in question.
        THSUniverseIterator tIter;
        HS_BOOL8 bContinue;
        CHS3DObject *pObject = NULL;
        for (bContinue = CHSUniverseDB::GetInstance().GetFirstUniverse(tIter);
             bContinue;
             bContinue = CHSUniverseDB::GetInstance().GetNextUniverse(tIter))
        {
            // See if the object is in this universe.
            CHSUniverse *pUniverse = tIter.pValue;

            pObject = pUniverse->FindObject(cmdGetList->m_uiClassOrObjectID);

            if (pObject)
            {
                break;
            }
        }

        // Did we find the object?
        if (!pObject)
        {
            cmdList.m_bQuerySucceeded = false;
        }
        else
        {
            pSysArray = pObject->GetEngSystemArray();
        }
    }

    // Do we have a system array?  If so, get all of the systems in it.
    if (pSysArray)
    {
        CHSEngSystem *pSystem;

        for (pSystem = pSysArray->GetHead(); pSystem;
             pSystem = pSystem->GetNext())
        {
            cmdList.AddSystem(pSystem->GetName());
        }
    }

    HSNetwork.SendPacket(cmdList);
}

void CHSHandlerMisc::HandleGetSystemData(CHSPacket * pPacket)
{
    CHSPGetSystemData *cmdGetData =
        static_cast < CHSPGetSystemData * >(pPacket);

    // Create a system data packet as a response.
    CHSPSystemData cmdData;
    cmdData.SetPacketAddress(cmdGetData->GetPacketAddress());
    cmdData.m_uiClassOrObjectID = cmdGetData->m_uiClassOrObjectID;

    // Find the class or object ID.
    CHSSystemArray *pSysArray = NULL;
    if (cmdGetData->m_bClass)
    {
        CHSShipClass *pClass =
            CHSClassDB::GetInstance().GetClass(cmdGetData->
                                               m_uiClassOrObjectID);
        if (pClass)
        {
            // Get the system array from the class.
            pSysArray = pClass->m_pSystems;
        }
    }
    else
    {
        // Find the hspace object in question.
        THSUniverseIterator tIter;
        HS_BOOL8 bContinue;
        CHS3DObject *pObject = NULL;
        for (bContinue = CHSUniverseDB::GetInstance().GetFirstUniverse(tIter);
             bContinue;
             bContinue = CHSUniverseDB::GetInstance().GetNextUniverse(tIter))
        {
            // See if the object is in this universe.
            CHSUniverse *pUniverse = tIter.pValue;

            pObject = pUniverse->FindObject(cmdGetData->m_uiClassOrObjectID);

            if (pObject)
            {
                break;
            }
        }

        // Did we find the object?
        if (pObject)
        {
            pSysArray = pObject->GetEngSystemArray();
        }
    }

    // Did we find the systems array?
    if (!pSysArray)
    {
        cmdData.m_bBadQuery = true;
    }
    else
    {
        // Find the specified system in the array.
        CHSEngSystem *pSystem =
            pSysArray->GetSystemByName(cmdGetData->m_strSystemName.c_str());

        if (pSystem)
        {
            // Found the system.  Dump the attributes.
            cmdData.m_bBadQuery = false;

            CHSAttributeList listAttrs;
            pSystem->GetAttributeList(listAttrs);

            while (!listAttrs.empty())
            {
                std::string & rstrAttribute = listAttrs.front();

                CHSPSystemData::THSSystemAttribute tAttribute;

                tAttribute.strAttributeName = rstrAttribute;

                // Query the attribute locally on the system.
                tAttribute.bValueSet =
                    pSystem->GetAttributeValue(rstrAttribute.c_str(),
                                               tAttribute.varValue, true,
                                               true);

                cmdData.AddAttribute(tAttribute);
                listAttrs.pop_front();
            }
        }
        else
        {
            // System not found.
            cmdData.m_bBadQuery = true;
        }
    }

    HSNetwork.SendPacket(cmdData);
}


void CHSHandlerMisc::HandleSetSystemData(CHSPacket * pPacket)
{
    CHSPSetSystemData *cmdSetData =
        static_cast < CHSPSetSystemData * >(pPacket);

    // Create a system data response packet.
    CHSPSetSystemDataResponse cmdResponse;
    cmdResponse.SetPacketAddress(cmdSetData->GetPacketAddress());
    cmdResponse.m_uiClassOrObjectID = cmdSetData->m_uiClassOrObjectID;

    // Find the class or object ID.
    CHSSystemArray *pSysArray = NULL;
    if (cmdSetData->m_bIsClass)
    {
        CHSShipClass *pClass =
            CHSClassDB::GetInstance().GetClass(cmdSetData->
                                               m_uiClassOrObjectID);

        if (pClass)
        {
            // Get the system array from the class.
            pSysArray = pClass->m_pSystems;
        }
    }
    else
    {
        // Find the hspace object in question.
        THSUniverseIterator tIter;
        HS_BOOL8 bContinue;
        CHS3DObject *pObject = NULL;
        for (bContinue = CHSUniverseDB::GetInstance().GetFirstUniverse(tIter);
             bContinue;
             bContinue = CHSUniverseDB::GetInstance().GetNextUniverse(tIter))
        {
            // See if the object is in this universe.
            CHSUniverse *pUniverse = tIter.pValue;

            pObject = pUniverse->FindObject(cmdSetData->m_uiClassOrObjectID);

            if (pObject)
            {
                break;
            }
        }

        // Did we find the object?
        if (pObject)
        {
            pSysArray = pObject->GetEngSystemArray();
        }
    }

    // Did we find the systems array?
    if (!pSysArray)
    {
        cmdResponse.m_bSuccess = false;
    }
    else
    {
        // Find the specified system in the array.
        CHSEngSystem *pSystem =
            pSysArray->GetSystemByName(cmdSetData->m_strSystemName.c_str());

        if (pSystem)
        {
            // Initially assume all modifications succeed.
            cmdResponse.m_bSuccess = true;

            // Found the system.  Run through the list of attribute in the 
            // packet, and modify them.
            CHSPSetSystemData::THSSystemAttribute * pAttrModify;

            for (pAttrModify = cmdSetData->GetFirstAttr(); pAttrModify;
                 pAttrModify = cmdSetData->GetNextAttr())
            {

                // Try to set this attribute on the system.
                if (!pSystem->
                    SetAttributeValue(pAttrModify->strAttributeName.c_str(),
                                      pAttrModify->strValue.c_str()))
                {
                    cmdResponse.m_bSuccess = false;
                }
            }
        }
        else
        {
            // System not found.
            cmdResponse.m_bSuccess = false;
        }
    }

    HSNetwork.SendPacket(cmdResponse);
}

void CHSHandlerMisc::HandleAddSystem(CHSPacket * pPacket)
{
    CHSPAddSystem *pAddSystem = static_cast < CHSPAddSystem * >(pPacket);
    CHSPAddSystemResponse cmdResponse;

    cmdResponse.m_uiClassOrObjectID = pAddSystem->m_uiClassOrObjectID;
    cmdResponse.SetPacketAddress(pAddSystem->GetPacketAddress());

    // Find the class or object ID.
    CHSSystemArray *pSysArray = NULL;
    if (pAddSystem->m_bIsClass)
    {
        CHSShipClass *pClass =
            CHSClassDB::GetInstance().GetClass(pAddSystem->
                                               m_uiClassOrObjectID);
        if (pClass)
        {
            // Get the system array from the class.
            if (!pClass->m_pSystems)
            {
                pClass->m_pSystems = new CHSSystemArray;
            }

            pSysArray = pClass->m_pSystems;
        }
    }
    else
    {
        // Find the hspace object in question.
        THSUniverseIterator tIter;
        HS_BOOL8 bContinue;
        CHS3DObject *pObject = NULL;
        for (bContinue = CHSUniverseDB::GetInstance().GetFirstUniverse(tIter);
             bContinue;
             bContinue = CHSUniverseDB::GetInstance().GetNextUniverse(tIter))
        {
            // See if the object is in this universe.
            CHSUniverse *pUniverse = tIter.pValue;

            pObject = pUniverse->FindObject(pAddSystem->m_uiClassOrObjectID);

            if (pObject)
            {
                break;
            }
        }

        // Did we find the object?
        if (pObject)
        {
            pSysArray = pObject->GetEngSystemArray();
        }
    }

    // Did we find the systems array?
    if (!pSysArray)
    {
        cmdResponse.m_bAdded = false;
    }
    else
    {
        HSS_TYPE eNewType = (HSS_TYPE) pAddSystem->m_uiSystemType;

        // Try to find the system already on the class
        HS_BOOL8 bOkToAdd = true;
        if (eNewType != HSS_FICTIONAL)
        {
            CHSEngSystem *pSystem;

            pSystem = pSysArray->GetSystem(eNewType);
            if (pSystem)
            {
                bOkToAdd = false;
                cmdResponse.m_bAdded = false;
            }
        }

        if (bOkToAdd)
        {
            // Add the system
            CHSEngSystem *pSystem = CHSEngSystem::CreateFromType(eNewType);
            if (!pSystem)
            {
                cmdResponse.m_bAdded = false;
            }
            else
            {
                pSysArray->AddSystem(pSystem);
                cmdResponse.m_bAdded = true;
                cmdResponse.m_strSystemName = pSystem->GetName();
            }
        }
    }

    HSNetwork.SendPacket(cmdResponse);
}


void CHSHandlerMisc::HandleDeleteSystem(CHSPacket * pPacket)
{
    CHSPDeleteSystem *pDeleteSystem =
        static_cast < CHSPDeleteSystem * >(pPacket);
    CHSPDeleteSystemResponse cmdResponse;

    cmdResponse.m_uiClassOrObjectID = pDeleteSystem->m_uiClassOrObjectID;
    cmdResponse.SetPacketAddress(pDeleteSystem->GetPacketAddress());

    // Find the class or object ID.
    CHSSystemArray *pSysArray = NULL;
    if (pDeleteSystem->m_bIsClass)
    {
        CHSShipClass *pClass =
            CHSClassDB::GetInstance().GetClass(pDeleteSystem->
                                               m_uiClassOrObjectID);

        if (pClass)
        {
            // Get the system array from the class.
            if (pClass->m_pSystems)
            {
                pSysArray = pClass->m_pSystems;
            }
        }
    }
    else
    {
        // Find the hspace object in question.
        THSUniverseIterator tIter;
        HS_BOOL8 bContinue;
        CHS3DObject *pObject = NULL;
        for (bContinue = CHSUniverseDB::GetInstance().GetFirstUniverse(tIter);
             bContinue;
             bContinue = CHSUniverseDB::GetInstance().GetNextUniverse(tIter))
        {
            // See if the object is in this universe.
            CHSUniverse *pUniverse = tIter.pValue;

            pObject =
                pUniverse->FindObject(pDeleteSystem->m_uiClassOrObjectID);

            if (pObject)
            {
                break;
            }
        }

        // Did we find the object?
        if (pObject)
        {
            pSysArray = pObject->GetEngSystemArray();
        }
    }

    // Did we find the systems array?
    if (!pSysArray)
    {
        // They don't care if the system was truly deleted.
        cmdResponse.m_bDeleted = true;
    }
    else
    {
        // Try to find the system in the array.
        CHSEngSystem *pSystem = NULL;
        pSystem =
            pSysArray->GetSystemByName(pDeleteSystem->m_strSystemName.
                                       c_str());

        if (!pSystem)
        {
            cmdResponse.m_bDeleted = false;
        }
        else
        {
            cmdResponse.m_bDeleted = pSysArray->DelSystem(pSystem);
        }
    }

    HSNetwork.SendPacket(cmdResponse);
}
