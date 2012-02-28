#include "pch.h"
#include "hsutils.h"
#include "HSPackets.h"
#include "hsserver.h"
#include "HSNetwork.h"
#include "hsweapon.h"

#include "HSHandlerWeapon.h"


HS_BOOL8 CHSHandlerWeapon::Initialize()
{
    // Register handlers.
    REGISTER_PACKET_HANDLER(PT_GET_WEAPON_LIST,
                            &CHSHandlerWeapon::HandleGetWeaponList, true);
    REGISTER_PACKET_HANDLER(PT_GET_WEAPON_DATA,
                            &CHSHandlerWeapon::HandleGetWeaponData, true);
    REGISTER_PACKET_HANDLER(PT_SET_WEAPON_DATA,
                            &CHSHandlerWeapon::HandleSetWeaponData, true);
    REGISTER_PACKET_HANDLER(PT_CREATE_WEAPON,
                            &CHSHandlerWeapon::HandleCreateWeapon, true);

    return true;
}

void CHSHandlerWeapon::HandleGetWeaponList(CHSPacket * pPacket)
{
    hs_log("ADMIN SERVER: Handle packet GetWeaponList");
    CHSPGetWeaponList *cmdGetList =
        static_cast < CHSPGetWeaponList * >(pPacket);

    // Create a class list packet as a response.
    CHSPWeaponList cmdList;
    cmdList.SetPacketAddress(cmdGetList->GetPacketAddress());

    CHSWeaponData *pData;
    for (pData = waWeapons.GetFirstWeapon(); pData;
         pData = waWeapons.GetNextWeapon())
    {
        CHSPWeaponList::THSWeapon tWeapon;

        tWeapon.uiWeaponID = pData->TypeID();
        tWeapon.strName = pData->Name();
        tWeapon.uiType = (unsigned int) (pData->WeaponClass());

        cmdList.AddWeapon(tWeapon);
    }

    HSNetwork.SendPacket(cmdList);
}

void CHSHandlerWeapon::HandleGetWeaponData(CHSPacket * pPacket)
{
    hs_log("ADMIN SERVER: Handle packet GetWeaponData");
    CHSPGetWeaponData *cmdGetData =
        static_cast < CHSPGetWeaponData * >(pPacket);

    // Create a response.
    CHSPWeaponData cmdData;
    cmdData.SetPacketAddress(cmdGetData->GetPacketAddress());
    cmdData.m_uiWeaponID = cmdGetData->m_uiWeaponID;

    // Find the weapon in question.
    CHSWeaponData *pData = waWeapons.GetWeapon(cmdGetData->m_uiWeaponID);
    if (!pData)
    {
        cmdData.m_bBadQuery = true;
    }
    else
    {
        // Got the weapon.  Add all of the attributes.
        CHSAttributeList listAttributes;

        pData->GetAttributeList(listAttributes);

        while (!listAttributes.empty())
        {
            CHSPWeaponData::THSWeaponAttribute tAttribute;

            tAttribute.strAttributeName = listAttributes.front();
            tAttribute.strValue =
                pData->GetAttributeValue(listAttributes.front().c_str());

            cmdData.AddAttribute(tAttribute);

            listAttributes.pop_front();
        }

    }

    HSNetwork.SendPacket(cmdData);
}

void CHSHandlerWeapon::HandleSetWeaponData(CHSPacket * pPacket)
{
    CHSPSetWeaponData *cmdSetData =
        static_cast < CHSPSetWeaponData * >(pPacket);
    CHSPSetWeaponDataResponse cmdResponse;

    cmdResponse.SetPacketAddress(cmdSetData->GetPacketAddress());
    cmdResponse.m_uiWeaponID = cmdSetData->m_uiWeaponID;

    // Find the weapon in question.
    CHSWeaponData *pData = waWeapons.GetWeapon(cmdSetData->m_uiWeaponID);
    if (!pData)
    {
        cmdResponse.m_bSuccess = false;
    }
    else
    {
        cmdResponse.m_bSuccess = true;

        // Run through the attributes contained in the packet.
        CHSPSetWeaponData::THSWeaponAttribute * pAttribute;

        for (pAttribute = cmdSetData->GetFirstAttr(); pAttribute;
             pAttribute = cmdSetData->GetNextAttr())
        {
            if (!pData->
                SetAttributeValue(pAttribute->strAttributeName.c_str(),
                                  pAttribute->strValue.c_str()))
            {
                cmdResponse.m_bSuccess = false;
            }
        }
    }

    HSNetwork.SendPacket(cmdResponse);
}


void CHSHandlerWeapon::HandleCreateWeapon(CHSPacket * pPacket)
{
    CHSPCreateWeapon *cmdCreate = static_cast < CHSPCreateWeapon * >(pPacket);
    CHSPCreateWeaponResponse cmdResponse;

    cmdResponse.SetPacketAddress(cmdCreate->GetPacketAddress());

    // Do we have a valid weapon name?
    if (cmdCreate->m_strWeaponName.length() > 0)
    {
        // Try to create the new weapon data.
        CHSWeaponData *pData =
            CHSWeaponData::CreateFromClass((EHSWeaponClass) cmdCreate->
                                           m_uiType);
        if (pData)
        {
            pData->Name(cmdCreate->m_strWeaponName.c_str());
            waWeapons.AddWeapon(pData, true);
            cmdResponse.m_bCreateSucceeded = true;
            cmdResponse.m_uiWeaponID = pData->TypeID();
        }
        else
        {
            cmdResponse.m_bCreateSucceeded = false;
        }
    }
    else
    {
        cmdResponse.m_bCreateSucceeded = false;
    }

    HSNetwork.SendPacket(cmdResponse);
}
