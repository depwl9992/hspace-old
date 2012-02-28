#include <stdio.h>

#ifdef WIN32
#include <windows.h>
#include <conio.h>
#else
#include <iostream>
#include <unistd.h>
#endif



#include "HSNetwork.h"
#include "HSPackets.h"
#include "HSAttributes.h"
#include "HSEngTypes.h"

// Prototype functions
void GetClassList();
void GetClassData1();
void GetSystemsClass1();
void ModifyClass1();
void CreateNewClass();
void GetSystemDataClass1();
void ModifySystemDataClass1();
void DeleteClass1();
void AddSystemClass1();


#define ADMIN_LOGIN			"One"
#define ADMIN_PASSWORD		"one"
#define ADMIN_SERVER_ADDR	"localhost"
#define ADMIN_SERVER_PORT	4202



// The main application loop.  It all starts here.
int main()
{
	// Connect to the server, using login name and password.

	if (HSNetwork.Connect(ADMIN_SERVER_ADDR, ADMIN_SERVER_PORT, ADMIN_LOGIN, ADMIN_PASSWORD) != 0)
	{
		// We're logged in.

		// Uncomment any of these functions to see them in action.
		GetClassList();

//		GetClassData1();

//		GetSystemsClass1();

//		ModifyClass1();

//		CreateNewClass();

//		GetSystemDataClass1();

//		ModifySystemDataClass1();

//		DeleteClass1();

//		AddSystemClass1();

		// A dummy loop so the application doesn't exit.  Hit any key to exit.
		printf("Press any key to exit...");
#ifdef WIN32
		while(!_kbhit())
		{
			SleepEx(100, true);
		}
#else
		while(!getc(stdin))
		{
			usleep(10000);
		}
#endif
	}
	else
	{
		printf("Failed to login to server.\n");
		
		// A dummy loop so the application doesn't exit.  Hit any key to exit.
		printf("Press any key to exit...");
	}
#ifdef WIN32
		while(!_kbhit())
		{
			SleepEx(100, true);
		}
#else
		while(!getc(stdin))
		{
			usleep(10000);
		}
#endif

	return 0;
}

// An example of how to get a class list from the server.
void GetClassList()
{
	CHSPGetClassList	cmdGetClasses;

	HSNetwork.SendPacket(cmdGetClasses);

	if (HSNetwork.WaitForPacket(PT_CLASS_LIST))
	{
		// We've received a list of classes.
		CHSPClassList* cmdClassList = static_cast<CHSPClassList*>(HSNetwork.GetPendingPacket());

		CHSPClassList::THSClass*	pClassInfo;
		
		printf("Class List:\n");

		for (pClassInfo = cmdClassList->GetFirstClass(); pClassInfo; pClassInfo = cmdClassList->GetNextClass())
		{
			printf("[%2d] %s\n", pClassInfo->uiClassID, pClassInfo->strClassName.c_str());
		}

		// Free the packet.
		delete cmdClassList;
	}
}

// An example of how to get class information for class 1.
void GetClassData1()
{
	CHSPGetClassData	cmdGetData;

	cmdGetData.m_uiClassID = 1;

	HSNetwork.SendPacket(cmdGetData);

	if (HSNetwork.WaitForPacket(PT_CLASS_DATA))
	{
		// We've received a response packet for the query.
		CHSPClassData* cmdClassData = static_cast<CHSPClassData*>(HSNetwork.GetPendingPacket());

		CHSPClassData::THSClassAttribute*	pClassAttr;
		
		printf("Class Attributes:\n");

		for (pClassAttr = cmdClassData->GetFirstAttr(); pClassAttr; pClassAttr = cmdClassData->GetNextAttr())
		{
			printf("[%s] ", pClassAttr->strAttributeName.c_str());

			// Determine the type of attributes being managed.
			switch(pClassAttr->varValue.GetType())
			{
			case CHSVariant::VT_STRING:
				printf("%s", pClassAttr->varValue.GetString());
				break;

			// These should really be using the various Get*() methods in variant, but
			// we're safe for this example to use just GetUInt().
			case CHSVariant::VT_UINT32:
			case CHSVariant::VT_UINT8:
			case CHSVariant::VT_UINT16:
			case CHSVariant::VT_INT16:
			case CHSVariant::VT_INT8:
			case CHSVariant::VT_INT32:
				printf("%d", pClassAttr->varValue.GetUInt());
				break;
			case CHSVariant::VT_BOOL:
				printf("%s", pClassAttr->varValue.GetBool() ? "true" : "false");
				break;
			}

			printf("\n");
		}

		// Free the packet.
		delete cmdClassData;
	}
}

// An example of how to retrieve a list of engineering systems from a class or object.
void GetSystemsClass1()
{
	CHSPGetSystemList	cmdGetList;

	cmdGetList.m_uiClassOrObjectID = 1;
	cmdGetList.m_bClass = true;

	HSNetwork.SendPacket(cmdGetList);

	// Wait for the response.
	if (HSNetwork.WaitForPacket(PT_SYSTEM_LIST))
	{
		// We got a list of systems back from the server.
		CHSPSystemList*		pSysList = static_cast<CHSPSystemList*>(HSNetwork.GetPendingPacket());

		if (pSysList->m_bQuerySucceeded)
		{
			// Print out the list of engineering systems.
			printf("Engineering systems for class 1:\n");
			const char*	pcSystemName;

			for (pcSystemName = pSysList->GetFirstSystem(); pcSystemName; pcSystemName = pSysList->GetNextSystem())
			{
				printf("%s\n", pcSystemName);
			}
		}
		else
		{
			printf("Failed to retrieve a list of engineering systems for class 1.\n");
		}

		// Free the packet.
		delete pSysList;
	}
}


// An example of how to modify class information for class 1.
void ModifyClass1()
{
	CHSPSetClassData	cmdSetData;

	cmdSetData.m_uiClassID = 1;

	CHSPSetClassData::THSClassAttribute		tAttr1;
	CHSPSetClassData::THSClassAttribute		tAttr2;

	tAttr1.strAttributeName = "MAXHULL";
	tAttr1.strValue = "200";
	cmdSetData.AddAttribute(tAttr1);

	tAttr2.strAttributeName = "CAN DROP";
	tAttr2.strValue = "1";
	cmdSetData.AddAttribute(tAttr2);

	HSNetwork.SendPacket(cmdSetData);

	if (HSNetwork.WaitForPacket(PT_SET_CLASS_DATA_RESPONSE))
	{
		// We've received a response.
		CHSPSetClassDataResponse* cmdResponse = static_cast<CHSPSetClassDataResponse*>(HSNetwork.GetPendingPacket());

		if (cmdResponse->m_bSuccess)
		{
			printf("Class data information changed!\n");
		}
		else
		{
			printf("Failed to set class data information.\n");
		}

		// Free the packet.
		delete cmdResponse;
	}
}

// An example of how to create a new class on the server.
void CreateNewClass()
{
	CHSPCreateClass	cmdCreate;

	cmdCreate.m_strClassName = "My ShipClass";

	HSNetwork.SendPacket(cmdCreate);

	// Wait for a create response.
	if (HSNetwork.WaitForPacket(PT_CREATE_CLASS_RESPONSE))
	{
		CHSPCreateClassResponse*	pResponse = static_cast<CHSPCreateClassResponse*>(HSNetwork.GetPendingPacket());

		if (pResponse->m_bCreateSucceeded)
		{
			printf("Created class \"My ShipClass\" with server assigned ID %d!\n", pResponse->m_uiClassID);
		}
		else
		{
			printf("Failed to create ship class.\n");
		}

		// Free the packet.
		delete pResponse;
	}
}

void GetSystemDataClass1()
{
	CHSPGetSystemData	cmdGetData;

	cmdGetData.m_uiClassOrObjectID = 1;
	cmdGetData.m_bClass = true;
	cmdGetData.m_strSystemName = "Engines";

	HSNetwork.SendPacket(cmdGetData);

	// Wait for a system data response.
	if (HSNetwork.WaitForPacket(PT_SYSTEM_DATA))
	{
		// We got the packet back.
		CHSPSystemData*		pData = static_cast<CHSPSystemData*>(HSNetwork.GetPendingPacket());

		// Did the query work?
		if (pData->m_bBadQuery)
		{
			printf("Failed to query system data for class 1, Engines.\n");
		}
		else
		{
			printf("System data for class 1:\n");

			// Run through the attributes, printing their information.
			CHSPSystemData::THSSystemAttribute*		pAttribute;
			for (pAttribute = pData->GetFirstAttr(); pAttribute; pAttribute = pData->GetNextAttr())
			{
				printf("ATTRIBUTE: %s, Value = ", 
						pAttribute->strAttributeName.c_str());

				if (!pAttribute->bValueSet)
				{
					printf("Not Set Locally\n");
				}
				else
				{
					// Determine the type of attributes being managed.
					switch(pAttribute->varValue.GetType())
					{
					case CHSVariant::VT_STRING:
						printf("%s", pAttribute->varValue.GetString());
						break;

					// These should really be using the various Get*() methods in variant, but
					// we're safe for this example to use just GetUInt().
					case CHSVariant::VT_UINT32:
					case CHSVariant::VT_UINT8:
					case CHSVariant::VT_UINT16:
					case CHSVariant::VT_INT16:
					case CHSVariant::VT_INT8:
					case CHSVariant::VT_INT32:
						printf("%d", pAttribute->varValue.GetUInt());
						break;

					case CHSVariant::VT_FLOAT:
					case CHSVariant::VT_DOUBLE:
						printf("%.2f", pAttribute->varValue.GetFloat());
						break;
						
					case CHSVariant::VT_BOOL:
						printf("%s", pAttribute->varValue.GetBool() ? "true" : "false");
						break;
					}

					printf("\n");
				}
			}
		}

		// Free the packet.
		delete pData;
	}
	else
	{
		printf("Never received CHSPSystemData packet in response to query.\n");
	}
}

void ModifySystemDataClass1()
{
	// Create a system data modify packet.
	CHSPSetSystemData	cmdSetData;

	// Set all required attributes in the packet.
	cmdSetData.m_uiClassOrObjectID = 1;
	cmdSetData.m_bIsClass = true;
	cmdSetData.m_strSystemName = "Engines";

	// Add some attributes to the packet.
	CHSPSetSystemData::THSSystemAttribute	tAttr;

	tAttr.strAttributeName = "MAX_VELOCITY";
	tAttr.strValue = "2000";
	cmdSetData.AddAttribute(tAttr);

	tAttr.strAttributeName = "ACCELERATION";
	tAttr.strValue = "50";
	cmdSetData.AddAttribute(tAttr);

	// Send the command to the server.
	HSNetwork.SendPacket(cmdSetData);

	// Wait for a response.
	if (HSNetwork.WaitForPacket(PT_SET_SYSTEM_DATA_RESPONSE))
	{
		CHSPSetSystemDataResponse*		pResponse = static_cast<CHSPSetSystemDataResponse*>(HSNetwork.GetPendingPacket());

		// Did it work?
		if (pResponse->m_bSuccess)
		{
			printf("Modify class 1 - succeeded!\n");
		}
		else
		{
			printf("Failed to modify class 1.\n");
		}

		// Free the packet.
		delete pResponse;
	}
	else
	{
		printf("Failed to receive a PT_SET_SYSTEM_DATA_RESPONSE packet.\n");
	}
}

void DeleteClass1()
{
	// Create the delete packet.
	CHSPDeleteClass		cmdDelete;

	cmdDelete.m_uiClassID	= 1;
	
	// Send the command to the server.
	HSNetwork.SendPacket(cmdDelete);

	// Wait for the response.
	if (HSNetwork.WaitForPacket(PT_DELETE_CLASS_RESPONSE))
	{
		CHSPDeleteClassResponse*	pResponse = static_cast<CHSPDeleteClassResponse*>(HSNetwork.GetPendingPacket());

		// Did it get deleted?
		if (pResponse->m_bDeleted)
		{
			printf("Deleted ship class 1.\n");
		}
		else
		{
			printf("Failed to delete ship class 1.  Maybe it's in use?\n");
		}

		// Free the packet.
		delete pResponse;
	}
	else
	{
		printf("Failed to receive a PT_DELETE_CLASS_RESPONSE packet.\n");
	}
}

void AddSystemClass1()
{
	// Create the packet.
	CHSPAddSystem	cmdAddSystem;

	cmdAddSystem.m_uiClassOrObjectID = 1;
	cmdAddSystem.m_bIsClass = true;
	cmdAddSystem.m_uiSystemType = HSS_CLOAK;

	// Send the command to the server.
	HSNetwork.SendPacket(cmdAddSystem);

	// Wait for the response.
	if (HSNetwork.WaitForPacket(PT_ADD_SYSTEM_RESPONSE))
	{
		CHSPAddSystemResponse*	pResponse = static_cast<CHSPAddSystemResponse*>(HSNetwork.GetPendingPacket());

		// Did it work?
		if (pResponse->m_bAdded)
		{
			printf("Added cloaking to class 1.\n");
		}
		else
		{
			printf("Failed to add cloaking to class 1.  Already added?\n");
		}

		// Free the packet.
		delete pResponse;
	}
	else
	{
		printf("Failed to receive a PT_ADD_SYSTEM_RESPONSE packet.\n");
	}
}
