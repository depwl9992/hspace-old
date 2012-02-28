#include "HSPackets.h"

CHSPLogin::~CHSPLogin()
{
    if (m_bStringsAllocated)
    {
        if (m_pcPlayerName)
        {
            delete[]m_pcPlayerName;
            m_pcPlayerName = NULL;
        }
        if (m_pcPassword)
        {
            delete[]m_pcPassword;
            m_pcPassword = NULL;
        }
    }
}

//-------------------------
bool CHSPLogin::WriteToStream(CHSDataStream & rStream)
{
    rStream.WriteString(m_pcPlayerName);
    rStream.WriteString(m_pcPassword);

    return true;
}
bool CHSPLogin::ReadFromStream(CHSDataStream & rStream)
{
    rStream.ReadString(&m_pcPlayerName);
    rStream.ReadString(&m_pcPassword);

    m_bStringsAllocated = true;

    return true;
}

//-------------------------
bool CHSPLoginResponse::WriteToStream(CHSDataStream & rStream)
{
    rStream.WriteToStream(m_bLoggedIn);

    return true;
}
bool CHSPLoginResponse::ReadFromStream(CHSDataStream & rStream)
{
    rStream.ReadFromStream(m_bLoggedIn);

    return true;
}

//-------------------------
void CHSPClassList::AddClass(THSClass & rtClass)
{
    m_listClasses.push_back(rtClass);
}

CHSPClassList::THSClass * CHSPClassList::GetFirstClass()
{
    m_iterCur = m_listClasses.begin();

    return GetNextClass();
}

CHSPClassList::THSClass * CHSPClassList::GetNextClass()
{
    if (m_iterCur == m_listClasses.end())
    {
        return NULL;
    }

    THSClass *pClass = &(*m_iterCur);

    m_iterCur++;

    return pClass;
}

bool CHSPClassList::WriteToStream(CHSDataStream & rStream)
{
    // Write # of classes.
    unsigned short usNumClasses = m_listClasses.size();
    rStream.WriteToStream(usNumClasses);

    // Now write all classes.
    std::list < THSClass >::iterator iter;
    for (iter = m_listClasses.begin(); iter != m_listClasses.end(); iter++)
    {
        THSClass & rtClass = *iter;
        rStream.WriteToStream(rtClass.uiClassID);
        rStream.WriteString(rtClass.strClassName.c_str());
    }
    return true;
}
bool CHSPClassList::ReadFromStream(CHSDataStream & rStream)
{
    // Read # of classes.
    unsigned short usNumClasses = 0;

    rStream.ReadFromStream(usNumClasses);

    // Now read all classes.
    while (usNumClasses > 0)
    {
        THSClass tClass;

        rStream.ReadFromStream(tClass.uiClassID);

        char *pcName;

        rStream.ReadString(&pcName);

        tClass.strClassName = pcName;

        delete[]pcName;

        m_listClasses.push_back(tClass);

        usNumClasses--;
    }
    return true;
}

//-------------------------
bool CHSPGetClassData::WriteToStream(CHSDataStream & rStream)
{
    rStream.WriteToStream(m_uiClassID);

    return true;
}
bool CHSPGetClassData::ReadFromStream(CHSDataStream & rStream)
{
    rStream.ReadFromStream(m_uiClassID);

    return true;
}

//-------------------------
CHSPClassData::THSClassAttribute * CHSPClassData::GetFirstAttr()
{
    m_iterCur = m_listAttributes.begin();

    return GetNextAttr();
}

CHSPClassData::THSClassAttribute * CHSPClassData::GetNextAttr()
{
    if (m_iterCur == m_listAttributes.end())
    {
        return NULL;
    }

    THSClassAttribute *pAttr = &(*m_iterCur);

    m_iterCur++;

    return pAttr;
}

void CHSPClassData::AddAttribute(CHSPClassData::THSClassAttribute & rtAttr)
{
    m_listAttributes.push_back(rtAttr);
}

bool CHSPClassData::WriteToStream(CHSDataStream & rStream)
{
    rStream.WriteToStream(m_uiClassID);
    rStream.WriteToStream(m_bBadQuery);

    // Write all attributes.
    unsigned short usNumAttrs = m_listAttributes.size();

    rStream.WriteToStream(usNumAttrs);

    std::list < THSClassAttribute >::iterator iter;
    for (iter = m_listAttributes.begin(); iter != m_listAttributes.end();
         iter++)
    {
        THSClassAttribute & rtAttr = *iter;

        rStream.WriteString(rtAttr.strAttributeName.c_str());
        rtAttr.varValue.WriteToStream(rStream);
    }
    return true;
}
bool CHSPClassData::ReadFromStream(CHSDataStream & rStream)
{
    rStream.ReadFromStream(m_uiClassID);
    rStream.ReadFromStream(m_bBadQuery);

    // Read num attributes.
    unsigned short usNumAttrs;
    rStream.ReadFromStream(usNumAttrs);

    while (usNumAttrs > 0)
    {
        THSClassAttribute tAttr;
        char *pcBuffer;

        rStream.ReadString(&pcBuffer);
        tAttr.strAttributeName = pcBuffer;
        delete[]pcBuffer;
        tAttr.varValue.ReadFromStream(rStream);

        m_listAttributes.push_back(tAttr);

        usNumAttrs--;
    }

    return true;
}

//-------------------------
void CHSPSetClassData::AddAttribute(CHSPSetClassData::
                                    THSClassAttribute & rtAttr)
{
    m_listAttributes.push_back(rtAttr);
}

CHSPSetClassData::THSClassAttribute * CHSPSetClassData::GetFirstAttr()
{
    m_iterCur = m_listAttributes.begin();

    return GetNextAttr();
}
CHSPSetClassData::THSClassAttribute * CHSPSetClassData::GetNextAttr()
{
    if (m_iterCur == m_listAttributes.end())
    {
        return NULL;
    }

    THSClassAttribute *pAttr = &(*m_iterCur);

    m_iterCur++;

    return pAttr;
}


bool CHSPSetClassData::WriteToStream(CHSDataStream & rStream)
{
    rStream.WriteToStream(m_uiClassID);

    // Write all attributes.
    unsigned short usNumAttrs = m_listAttributes.size();

    rStream.WriteToStream(usNumAttrs);

    std::list < THSClassAttribute >::iterator iter;
    for (iter = m_listAttributes.begin(); iter != m_listAttributes.end();
         iter++)
    {
        THSClassAttribute & rtAttr = *iter;

        rStream.WriteString(rtAttr.strAttributeName.c_str());
        rStream.WriteString(rtAttr.strValue.c_str());
    }
    return true;
}
bool CHSPSetClassData::ReadFromStream(CHSDataStream & rStream)
{
    rStream.ReadFromStream(m_uiClassID);

    // Read num attributes.
    unsigned short usNumAttrs;
    rStream.ReadFromStream(usNumAttrs);

    while (usNumAttrs > 0)
    {
        THSClassAttribute tAttr;
        char *pcBuffer;

        rStream.ReadString(&pcBuffer);
        tAttr.strAttributeName = pcBuffer;
        delete[]pcBuffer;

        rStream.ReadString(&pcBuffer);
        tAttr.strValue = pcBuffer;
        delete[]pcBuffer;

        m_listAttributes.push_back(tAttr);

        usNumAttrs--;
    }

    return true;
}

//-------------------------
bool CHSPSetClassDataResponse::WriteToStream(CHSDataStream & rStream)
{
    rStream.WriteToStream(m_uiClassID);
    rStream.WriteToStream(m_bSuccess);
    return true;
}
bool CHSPSetClassDataResponse::ReadFromStream(CHSDataStream & rStream)
{
    rStream.ReadFromStream(m_uiClassID);
    rStream.ReadFromStream(m_bSuccess);
    return true;
}

//-------------------------
bool CHSPGetSystemList::WriteToStream(CHSDataStream & rStream)
{
    rStream.WriteToStream(m_uiClassOrObjectID);
    rStream.WriteToStream(m_bClass);
    return true;
}
bool CHSPGetSystemList::ReadFromStream(CHSDataStream & rStream)
{
    rStream.ReadFromStream(m_uiClassOrObjectID);
    rStream.ReadFromStream(m_bClass);
    return true;
}

//-------------------------
void CHSPSystemList::AddSystem(const char *pcName)
{
    std::string strName;
    strName = pcName;

    m_listSystems.push_back(strName);
}

const char *CHSPSystemList::GetFirstSystem()
{
    m_iterCur = m_listSystems.begin();

    return GetNextSystem();
}

const char *CHSPSystemList::GetNextSystem()
{
    if (m_iterCur == m_listSystems.end())
    {
        return NULL;
    }

    std::string & rstrSystem = *m_iterCur;

    m_iterCur++;

    return rstrSystem.c_str();
}

bool CHSPSystemList::WriteToStream(CHSDataStream & rStream)
{
    rStream.WriteToStream(m_bQuerySucceeded);

    // Write # of systems.
    unsigned char ucNumSystems = m_listSystems.size();
    rStream.WriteToStream(ucNumSystems);

    // Now write all systems.
    std::list < std::string >::iterator iter;
    for (iter = m_listSystems.begin(); iter != m_listSystems.end(); iter++)
    {
        std::string & rstrSystem = *iter;
        rStream.WriteString(rstrSystem.c_str());
    }
    return true;
}
bool CHSPSystemList::ReadFromStream(CHSDataStream & rStream)
{
    rStream.ReadFromStream(m_bQuerySucceeded);

    // Read # of classes.
    unsigned char ucNumClasses = 0;

    rStream.ReadFromStream(ucNumClasses);

    // Now read all classes.
    while (ucNumClasses > 0)
    {
        char *pcName;

        rStream.ReadString(&pcName);

        std::string strName;
        strName = pcName;
        m_listSystems.push_back(strName);

        delete[]pcName;

        ucNumClasses--;
    }
    return true;
}

//-------------------------
bool CHSPCreateClass::WriteToStream(CHSDataStream & rStream)
{
    rStream.WriteString(m_strClassName.c_str());
    return true;
}
bool CHSPCreateClass::ReadFromStream(CHSDataStream & rStream)
{
    char *pcName;

    rStream.ReadString(&pcName);

    m_strClassName = pcName;

    delete[]pcName;

    return true;
}

//-------------------------
bool CHSPCreateClassResponse::WriteToStream(CHSDataStream & rStream)
{
    rStream.WriteToStream(m_bCreateSucceeded);
    rStream.WriteToStream(m_uiClassID);
    return true;
}
bool CHSPCreateClassResponse::ReadFromStream(CHSDataStream & rStream)
{
    rStream.ReadFromStream(m_bCreateSucceeded);
    rStream.ReadFromStream(m_uiClassID);
    return true;
}

//-------------------------
bool CHSPGetSystemData::WriteToStream(CHSDataStream & rStream)
{
    rStream.WriteToStream(m_uiClassOrObjectID);
    rStream.WriteToStream(m_bClass);
    rStream.WriteString(m_strSystemName.c_str());
    return true;
}
bool CHSPGetSystemData::ReadFromStream(CHSDataStream & rStream)
{
    rStream.ReadFromStream(m_uiClassOrObjectID);
    rStream.ReadFromStream(m_bClass);

    char *pcName;
    rStream.ReadString(&pcName);
    m_strSystemName = pcName;

    delete[]pcName;
    return true;
}

//-------------------------
CHSPSystemData::THSSystemAttribute * CHSPSystemData::GetFirstAttr()
{
    m_iterCur = m_listAttributes.begin();

    return GetNextAttr();
}

CHSPSystemData::THSSystemAttribute * CHSPSystemData::GetNextAttr()
{
    if (m_iterCur == m_listAttributes.end())
    {
        return NULL;
    }

    THSSystemAttribute *pAttr = &(*m_iterCur);

    m_iterCur++;

    return pAttr;
}

void CHSPSystemData::AddAttribute(CHSPSystemData::THSSystemAttribute & rtAttr)
{
    m_listAttributes.push_back(rtAttr);
}

bool CHSPSystemData::WriteToStream(CHSDataStream & rStream)
{
    rStream.WriteToStream(m_uiClassOrObjectID);
    rStream.WriteToStream(m_bBadQuery);

    // Write all attributes.
    unsigned short usNumAttrs = m_listAttributes.size();

    rStream.WriteToStream(usNumAttrs);

    std::list < THSSystemAttribute >::iterator iter;
    for (iter = m_listAttributes.begin(); iter != m_listAttributes.end();
         iter++)
    {
        THSSystemAttribute & rtAttr = *iter;

        rStream.WriteString(rtAttr.strAttributeName.c_str());
        rStream.WriteToStream(rtAttr.bValueSet);
        if (rtAttr.bValueSet)
        {
            rtAttr.varValue.WriteToStream(rStream);
        }
    }
    return true;
}
bool CHSPSystemData::ReadFromStream(CHSDataStream & rStream)
{
    rStream.ReadFromStream(m_uiClassOrObjectID);
    rStream.ReadFromStream(m_bBadQuery);

    // Read num attributes.
    unsigned short usNumAttrs;
    rStream.ReadFromStream(usNumAttrs);

    while (usNumAttrs > 0)
    {
        THSSystemAttribute tAttr;
        char *pcBuffer;

        rStream.ReadString(&pcBuffer);
        tAttr.strAttributeName = pcBuffer;
        delete[]pcBuffer;
        rStream.ReadFromStream(tAttr.bValueSet);

        if (tAttr.bValueSet)
        {
            tAttr.varValue.ReadFromStream(rStream);
        }

        m_listAttributes.push_back(tAttr);

        usNumAttrs--;
    }

    return true;
}

//-------------------------
void CHSPSetSystemData::AddAttribute(CHSPSetSystemData::
                                     THSSystemAttribute & rtAttr)
{
    m_listAttributes.push_back(rtAttr);
}

CHSPSetSystemData::THSSystemAttribute * CHSPSetSystemData::GetFirstAttr()
{
    m_iterCur = m_listAttributes.begin();

    return GetNextAttr();
}
CHSPSetSystemData::THSSystemAttribute * CHSPSetSystemData::GetNextAttr()
{
    if (m_iterCur == m_listAttributes.end())
    {
        return NULL;
    }

    THSSystemAttribute *pAttr = &(*m_iterCur);

    m_iterCur++;

    return pAttr;
}


bool CHSPSetSystemData::WriteToStream(CHSDataStream & rStream)
{
    rStream.WriteToStream(m_uiClassOrObjectID);
    rStream.WriteToStream(m_bIsClass);
    rStream.WriteString(m_strSystemName.c_str());

    // Write all attributes.
    unsigned short usNumAttrs = m_listAttributes.size();

    rStream.WriteToStream(usNumAttrs);

    std::list < THSSystemAttribute >::iterator iter;
    for (iter = m_listAttributes.begin(); iter != m_listAttributes.end();
         iter++)
    {
        THSSystemAttribute & rtAttr = *iter;

        rStream.WriteString(rtAttr.strAttributeName.c_str());
        rStream.WriteString(rtAttr.strValue.c_str());
    }
    return true;
}
bool CHSPSetSystemData::ReadFromStream(CHSDataStream & rStream)
{
    rStream.ReadFromStream(m_uiClassOrObjectID);
    rStream.ReadFromStream(m_bIsClass);

    char *pcName;
    rStream.ReadString(&pcName);
    m_strSystemName = pcName;
    delete[]pcName;

    // Read num attributes.
    unsigned short usNumAttrs;
    rStream.ReadFromStream(usNumAttrs);

    while (usNumAttrs > 0)
    {
        THSSystemAttribute tAttr;
        char *pcBuffer;

        rStream.ReadString(&pcBuffer);
        tAttr.strAttributeName = pcBuffer;
        delete[]pcBuffer;

        rStream.ReadString(&pcBuffer);
        tAttr.strValue = pcBuffer;
        delete[]pcBuffer;

        m_listAttributes.push_back(tAttr);

        usNumAttrs--;
    }

    return true;
}

//-------------------------
bool CHSPSetSystemDataResponse::WriteToStream(CHSDataStream & rStream)
{
    rStream.WriteToStream(m_uiClassOrObjectID);
    rStream.WriteToStream(m_bSuccess);
    return true;
}
bool CHSPSetSystemDataResponse::ReadFromStream(CHSDataStream & rStream)
{
    rStream.ReadFromStream(m_uiClassOrObjectID);
    rStream.ReadFromStream(m_bSuccess);
    return true;
}


//-------------------------
bool CHSPDeleteClass::WriteToStream(CHSDataStream & rStream)
{
    rStream.WriteToStream(m_uiClassID);
    return true;
}
bool CHSPDeleteClass::ReadFromStream(CHSDataStream & rStream)
{
    rStream.ReadFromStream(m_uiClassID);
    return true;
}

//-------------------------
bool CHSPDeleteClassResponse::WriteToStream(CHSDataStream & rStream)
{
    rStream.WriteToStream(m_uiClassID);
    rStream.WriteToStream(m_bDeleted);
    return true;
}
bool CHSPDeleteClassResponse::ReadFromStream(CHSDataStream & rStream)
{
    rStream.ReadFromStream(m_uiClassID);
    rStream.ReadFromStream(m_bDeleted);
    return true;
}

//-------------------------
bool CHSPAddSystem::WriteToStream(CHSDataStream & rStream)
{
    rStream.WriteToStream(m_uiClassOrObjectID);
    rStream.WriteToStream(m_bIsClass);
    rStream.WriteToStream(m_uiSystemType);
    return true;
}
bool CHSPAddSystem::ReadFromStream(CHSDataStream & rStream)
{
    rStream.ReadFromStream(m_uiClassOrObjectID);
    rStream.ReadFromStream(m_bIsClass);
    rStream.ReadFromStream(m_uiSystemType);
    return true;
}

//-------------------------
bool CHSPAddSystemResponse::WriteToStream(CHSDataStream & rStream)
{
    rStream.WriteToStream(m_uiClassOrObjectID);
    rStream.WriteToStream(m_bAdded);

    if (m_bAdded)
    {
        rStream.WriteString(m_strSystemName.c_str());
    }
    return true;
}
bool CHSPAddSystemResponse::ReadFromStream(CHSDataStream & rStream)
{
    rStream.ReadFromStream(m_uiClassOrObjectID);
    rStream.ReadFromStream(m_bAdded);

    if (m_bAdded)
    {
        char *pcName;
        rStream.ReadString(&pcName);
        m_strSystemName = pcName;

        delete[]pcName;
    }
    return true;
}

//-------------------------
bool CHSPDeleteSystem::WriteToStream(CHSDataStream & rStream)
{
    rStream.WriteToStream(m_uiClassOrObjectID);
    rStream.WriteToStream(m_bIsClass);
    rStream.WriteString(m_strSystemName.c_str());
    return true;
}
bool CHSPDeleteSystem::ReadFromStream(CHSDataStream & rStream)
{
    rStream.ReadFromStream(m_uiClassOrObjectID);
    rStream.ReadFromStream(m_bIsClass);

    char *pcBuffer;
    rStream.ReadString(&pcBuffer);
    m_strSystemName = pcBuffer;
    delete[]pcBuffer;
    return true;
}

//-------------------------
bool CHSPDeleteSystemResponse::WriteToStream(CHSDataStream & rStream)
{
    rStream.WriteToStream(m_uiClassOrObjectID);
    rStream.WriteToStream(m_bDeleted);

    return true;
}
bool CHSPDeleteSystemResponse::ReadFromStream(CHSDataStream & rStream)
{
    rStream.ReadFromStream(m_uiClassOrObjectID);
    rStream.ReadFromStream(m_bDeleted);

    return true;
}


//-------------------------
void CHSPUniverseList::AddUniverse(THSUniverse & rtEntry)
{
    m_listUniverses.push_back(rtEntry);
}

CHSPUniverseList::THSUniverse * CHSPUniverseList::GetFirstUniverse()
{
    m_iterCur = m_listUniverses.begin();

    return GetNextUniverse();
}

CHSPUniverseList::THSUniverse * CHSPUniverseList::GetNextUniverse()
{
    if (m_iterCur == m_listUniverses.end())
    {
        return NULL;
    }

    THSUniverse *pEntry = &(*m_iterCur);

    m_iterCur++;

    return pEntry;
}

bool CHSPUniverseList::WriteToStream(CHSDataStream & rStream)
{
    // Write # of classes.
    unsigned short usNumEntries = m_listUniverses.size();
    rStream.WriteToStream(usNumEntries);

    // Now write all classes.
    std::list < THSUniverse >::iterator iter;
    for (iter = m_listUniverses.begin(); iter != m_listUniverses.end();
         iter++)
    {
        THSUniverse & rtEntry = *iter;
        rStream.WriteToStream(rtEntry.uiID);
        rStream.WriteString(rtEntry.strName.c_str());
        rStream.WriteToStream(rtEntry.uiNumObjects);
        rStream.WriteToStream(rtEntry.uiNumActiveObjects);
    }
    return true;
}
bool CHSPUniverseList::ReadFromStream(CHSDataStream & rStream)
{
    // Read # of classes.
    unsigned short usNumEntries = 0;

    rStream.ReadFromStream(usNumEntries);

    // Now read all classes.
    while (usNumEntries > 0)
    {
        THSUniverse tEntry;

        rStream.ReadFromStream(tEntry.uiID);

        char *pcName;

        rStream.ReadString(&pcName);
        tEntry.strName = pcName;
        delete[]pcName;

        rStream.ReadFromStream(tEntry.uiNumObjects);
        rStream.ReadFromStream(tEntry.uiNumActiveObjects);

        m_listUniverses.push_back(tEntry);

        usNumEntries--;
    }
    return true;
}


//-------------------------
bool CHSPDeleteUniverse::WriteToStream(CHSDataStream & rStream)
{
    rStream.WriteToStream(m_uiUniverseID);
    return true;
}
bool CHSPDeleteUniverse::ReadFromStream(CHSDataStream & rStream)
{
    rStream.ReadFromStream(m_uiUniverseID);
    return true;
}


//-------------------------
bool CHSPDeleteUniverseResp::WriteToStream(CHSDataStream & rStream)
{
    rStream.WriteToStream(m_bDeleted);
    return true;
}
bool CHSPDeleteUniverseResp::ReadFromStream(CHSDataStream & rStream)
{
    rStream.ReadFromStream(m_bDeleted);
    return true;
}

//-------------------------
bool CHSPGetObjectList::ReadFromStream(CHSDataStream & rStream)
{
    rStream.ReadFromStream(m_ucObjectType);
    rStream.ReadFromStream(m_uiUniverseID);
    return true;
}

bool CHSPGetObjectList::WriteToStream(CHSDataStream & rStream)
{
    rStream.WriteToStream(m_ucObjectType);
    rStream.WriteToStream(m_uiUniverseID);
    return true;
}

//-------------------------
void CHSPObjectList::AddObject(THSObject & rtObject)
{
    m_listObjects.push_back(rtObject);
}

CHSPObjectList::THSObject * CHSPObjectList::GetFirstObject()
{
    m_iterCur = m_listObjects.begin();

    return GetNextObject();
}

CHSPObjectList::THSObject * CHSPObjectList::GetNextObject()
{
    if (m_iterCur == m_listObjects.end())
    {
        return NULL;
    }

    THSObject *pObject = &(*m_iterCur);

    m_iterCur++;

    return pObject;
}

bool CHSPObjectList::WriteToStream(CHSDataStream & rStream)
{
    rStream.WriteToStream(m_bQuerySucceeded);

    // Write # of Objects.
    unsigned short usNumObjects = m_listObjects.size();
    rStream.WriteToStream(usNumObjects);

    // Now write all Objects.
    std::list < THSObject >::iterator iter;
    for (iter = m_listObjects.begin(); iter != m_listObjects.end(); iter++)
    {
        THSObject & rtObject = *iter;
        rStream.WriteToStream(rtObject.cObjectType);
        rStream.WriteToStream(rtObject.uiObjectID);
        rStream.WriteString(rtObject.strObjectName.c_str());
        rStream.WriteToStream(rtObject.fX);
        rStream.WriteToStream(rtObject.fY);
        rStream.WriteToStream(rtObject.fZ);
    }
    return true;
}
bool CHSPObjectList::ReadFromStream(CHSDataStream & rStream)
{
    rStream.ReadFromStream(m_bQuerySucceeded);

    // Read # of objects.
    unsigned short usNumObjects = 0;

    rStream.ReadFromStream(usNumObjects);

    // Now read all objects.
    while (usNumObjects > 0)
    {
        THSObject tObject;

        rStream.ReadFromStream(tObject.cObjectType);
        rStream.ReadFromStream(tObject.uiObjectID);

        char *pcName;

        rStream.ReadString(&pcName);
        tObject.strObjectName = pcName;
        delete[]pcName;

        rStream.ReadFromStream(tObject.fX);
        rStream.ReadFromStream(tObject.fY);
        rStream.ReadFromStream(tObject.fZ);

        m_listObjects.push_back(tObject);

        usNumObjects--;
    }
    return true;
}

//-------------------------
bool CHSPGetObjectData::WriteToStream(CHSDataStream & rStream)
{
    rStream.WriteToStream(m_uiObjectID);

    return true;
}
bool CHSPGetObjectData::ReadFromStream(CHSDataStream & rStream)
{
    rStream.ReadFromStream(m_uiObjectID);

    return true;
}

//-------------------------
CHSPObjectData::THSObjectAttribute * CHSPObjectData::GetFirstAttr()
{
    m_iterCur = m_listAttributes.begin();

    return GetNextAttr();
}

CHSPObjectData::THSObjectAttribute * CHSPObjectData::GetNextAttr()
{
    if (m_iterCur == m_listAttributes.end())
    {
        return NULL;
    }

    THSObjectAttribute *pAttr = &(*m_iterCur);

    m_iterCur++;

    return pAttr;
}

void CHSPObjectData::AddAttribute(CHSPObjectData::THSObjectAttribute & rtAttr)
{
    m_listAttributes.push_back(rtAttr);
}

bool CHSPObjectData::WriteToStream(CHSDataStream & rStream)
{
    rStream.WriteToStream(m_uiObjectID);
    rStream.WriteToStream(m_bBadQuery);

    // Write all attributes.
    unsigned short usNumAttrs = m_listAttributes.size();

    rStream.WriteToStream(usNumAttrs);

    std::list < THSObjectAttribute >::iterator iter;
    for (iter = m_listAttributes.begin(); iter != m_listAttributes.end();
         iter++)
    {
        THSObjectAttribute & rtAttr = *iter;

        rStream.WriteString(rtAttr.strAttributeName.c_str());
        rStream.WriteString(rtAttr.strValue.c_str());
    }
    return true;
}
bool CHSPObjectData::ReadFromStream(CHSDataStream & rStream)
{
    rStream.ReadFromStream(m_uiObjectID);
    rStream.ReadFromStream(m_bBadQuery);

    // Read num attributes.
    unsigned short usNumAttrs;
    rStream.ReadFromStream(usNumAttrs);

    while (usNumAttrs > 0)
    {
        THSObjectAttribute tAttr;
        char *pcBuffer;

        rStream.ReadString(&pcBuffer);
        tAttr.strAttributeName = pcBuffer;
        delete[]pcBuffer;

        rStream.ReadString(&pcBuffer);
        tAttr.strValue = pcBuffer;
        delete[]pcBuffer;

        m_listAttributes.push_back(tAttr);

        usNumAttrs--;
    }

    return true;
}

//-------------------------
void CHSPSetObjectData::AddAttribute(CHSPSetObjectData::
                                     THSObjectAttribute & rtAttr)
{
    m_listAttributes.push_back(rtAttr);
}

CHSPSetObjectData::THSObjectAttribute * CHSPSetObjectData::GetFirstAttr()
{
    m_iterCur = m_listAttributes.begin();

    return GetNextAttr();
}
CHSPSetObjectData::THSObjectAttribute * CHSPSetObjectData::GetNextAttr()
{
    if (m_iterCur == m_listAttributes.end())
    {
        return NULL;
    }

    THSObjectAttribute *pAttr = &(*m_iterCur);

    m_iterCur++;

    return pAttr;
}


bool CHSPSetObjectData::WriteToStream(CHSDataStream & rStream)
{
    rStream.WriteToStream(m_uiObjectID);

    // Write all attributes.
    unsigned short usNumAttrs = m_listAttributes.size();

    rStream.WriteToStream(usNumAttrs);

    std::list < THSObjectAttribute >::iterator iter;
    for (iter = m_listAttributes.begin(); iter != m_listAttributes.end();
         iter++)
    {
        THSObjectAttribute & rtAttr = *iter;

        rStream.WriteString(rtAttr.strAttributeName.c_str());
        rStream.WriteString(rtAttr.strValue.c_str());
    }
    return true;
}
bool CHSPSetObjectData::ReadFromStream(CHSDataStream & rStream)
{
    rStream.ReadFromStream(m_uiObjectID);

    // Read num attributes.
    unsigned short usNumAttrs;
    rStream.ReadFromStream(usNumAttrs);

    while (usNumAttrs > 0)
    {
        THSObjectAttribute tAttr;
        char *pcBuffer;

        rStream.ReadString(&pcBuffer);
        tAttr.strAttributeName = pcBuffer;
        delete[]pcBuffer;

        rStream.ReadString(&pcBuffer);
        tAttr.strValue = pcBuffer;
        delete[]pcBuffer;

        m_listAttributes.push_back(tAttr);

        usNumAttrs--;
    }

    return true;
}

//-------------------------
bool CHSPSetObjectDataResponse::WriteToStream(CHSDataStream & rStream)
{
    rStream.WriteToStream(m_uiObjectID);
    rStream.WriteToStream(m_bSuccess);
    return true;
}
bool CHSPSetObjectDataResponse::ReadFromStream(CHSDataStream & rStream)
{
    rStream.ReadFromStream(m_uiObjectID);
    rStream.ReadFromStream(m_bSuccess);
    return true;
}


//-------------------------
void CHSPWeaponList::AddWeapon(THSWeapon & rtWeapon)
{
    m_listWeapons.push_back(rtWeapon);
}

CHSPWeaponList::THSWeapon * CHSPWeaponList::GetFirstWeapon()
{
    m_iterCur = m_listWeapons.begin();

    return GetNextWeapon();
}

CHSPWeaponList::THSWeapon * CHSPWeaponList::GetNextWeapon()
{
    if (m_iterCur == m_listWeapons.end())
    {
        return NULL;
    }

    THSWeapon *pWeapon = &(*m_iterCur);

    m_iterCur++;

    return pWeapon;
}

bool CHSPWeaponList::WriteToStream(CHSDataStream & rStream)
{
    // Write # of weapons.
    unsigned short usNumWeapons = m_listWeapons.size();
    rStream.WriteToStream(usNumWeapons);

    // Now write all weapons.
    std::list < THSWeapon >::iterator iter;
    for (iter = m_listWeapons.begin(); iter != m_listWeapons.end(); iter++)
    {
        THSWeapon & rtWeapon = *iter;
        rStream.WriteToStream(rtWeapon.uiWeaponID);
        rStream.WriteString(rtWeapon.strName.c_str());
        rStream.WriteToStream(rtWeapon.uiType);
    }
    return true;
}
bool CHSPWeaponList::ReadFromStream(CHSDataStream & rStream)
{
    // Read # of weapons.
    unsigned short usNumWeapons = 0;

    rStream.ReadFromStream(usNumWeapons);

    // Now read all weapons.
    while (usNumWeapons > 0)
    {
        THSWeapon tWeapon;

        rStream.ReadFromStream(tWeapon.uiWeaponID);

        char *pcName;

        rStream.ReadString(&pcName);
        tWeapon.strName = pcName;
        delete[]pcName;

        rStream.ReadFromStream(tWeapon.uiType);

        m_listWeapons.push_back(tWeapon);

        usNumWeapons--;
    }
    return true;
}


//-------------------------
bool CHSPCreateWeapon::WriteToStream(CHSDataStream & rStream)
{
    rStream.WriteString(m_strWeaponName.c_str());
    rStream.WriteToStream(m_uiType);

    return true;
}
bool CHSPCreateWeapon::ReadFromStream(CHSDataStream & rStream)
{
    char *pcName;

    rStream.ReadString(&pcName);
    m_strWeaponName = pcName;
    delete[]pcName;

    rStream.ReadFromStream(m_uiType);

    return true;
}

//-------------------------
bool CHSPCreateWeaponResponse::WriteToStream(CHSDataStream & rStream)
{
    rStream.WriteToStream(m_bCreateSucceeded);
    rStream.WriteToStream(m_uiWeaponID);
    return true;
}
bool CHSPCreateWeaponResponse::ReadFromStream(CHSDataStream & rStream)
{
    rStream.ReadFromStream(m_bCreateSucceeded);
    rStream.ReadFromStream(m_uiWeaponID);
    return true;
}


//-------------------------
bool CHSPGetWeaponData::WriteToStream(CHSDataStream & rStream)
{
    rStream.WriteToStream(m_uiWeaponID);

    return true;
}
bool CHSPGetWeaponData::ReadFromStream(CHSDataStream & rStream)
{
    rStream.ReadFromStream(m_uiWeaponID);

    return true;
}

//-------------------------
CHSPWeaponData::THSWeaponAttribute * CHSPWeaponData::GetFirstAttr()
{
    m_iterCur = m_listAttributes.begin();

    return GetNextAttr();
}

CHSPWeaponData::THSWeaponAttribute * CHSPWeaponData::GetNextAttr()
{
    if (m_iterCur == m_listAttributes.end())
    {
        return NULL;
    }

    THSWeaponAttribute *pAttr = &(*m_iterCur);

    m_iterCur++;

    return pAttr;
}

void CHSPWeaponData::AddAttribute(CHSPWeaponData::THSWeaponAttribute & rtAttr)
{
    m_listAttributes.push_back(rtAttr);
}

bool CHSPWeaponData::WriteToStream(CHSDataStream & rStream)
{
    rStream.WriteToStream(m_uiWeaponID);
    rStream.WriteToStream(m_bBadQuery);

    // Write all attributes.
    unsigned short usNumAttrs = m_listAttributes.size();

    rStream.WriteToStream(usNumAttrs);

    std::list < THSWeaponAttribute >::iterator iter;
    for (iter = m_listAttributes.begin(); iter != m_listAttributes.end();
         iter++)
    {
        THSWeaponAttribute & rtAttr = *iter;

        rStream.WriteString(rtAttr.strAttributeName.c_str());
        rStream.WriteString(rtAttr.strValue.c_str());
    }
    return true;
}
bool CHSPWeaponData::ReadFromStream(CHSDataStream & rStream)
{
    rStream.ReadFromStream(m_uiWeaponID);
    rStream.ReadFromStream(m_bBadQuery);

    // Read num attributes.
    unsigned short usNumAttrs;
    rStream.ReadFromStream(usNumAttrs);

    while (usNumAttrs > 0)
    {
        THSWeaponAttribute tAttr;
        char *pcBuffer;

        rStream.ReadString(&pcBuffer);
        tAttr.strAttributeName = pcBuffer;
        delete[]pcBuffer;

        rStream.ReadString(&pcBuffer);
        tAttr.strValue = pcBuffer;
        delete[]pcBuffer;

        m_listAttributes.push_back(tAttr);

        usNumAttrs--;
    }

    return true;
}


//-------------------------
void CHSPSetWeaponData::AddAttribute(CHSPSetWeaponData::
                                     THSWeaponAttribute & rtAttr)
{
    m_listAttributes.push_back(rtAttr);
}

CHSPSetWeaponData::THSWeaponAttribute * CHSPSetWeaponData::GetFirstAttr()
{
    m_iterCur = m_listAttributes.begin();

    return GetNextAttr();
}
CHSPSetWeaponData::THSWeaponAttribute * CHSPSetWeaponData::GetNextAttr()
{
    if (m_iterCur == m_listAttributes.end())
    {
        return NULL;
    }

    THSWeaponAttribute *pAttr = &(*m_iterCur);

    m_iterCur++;

    return pAttr;
}


bool CHSPSetWeaponData::WriteToStream(CHSDataStream & rStream)
{
    rStream.WriteToStream(m_uiWeaponID);

    // Write all attributes.
    unsigned short usNumAttrs = m_listAttributes.size();

    rStream.WriteToStream(usNumAttrs);

    std::list < THSWeaponAttribute >::iterator iter;
    for (iter = m_listAttributes.begin(); iter != m_listAttributes.end();
         iter++)
    {
        THSWeaponAttribute & rtAttr = *iter;

        rStream.WriteString(rtAttr.strAttributeName.c_str());
        rStream.WriteString(rtAttr.strValue.c_str());
    }
    return true;
}
bool CHSPSetWeaponData::ReadFromStream(CHSDataStream & rStream)
{
    rStream.ReadFromStream(m_uiWeaponID);

    // Read num attributes.
    unsigned short usNumAttrs;
    rStream.ReadFromStream(usNumAttrs);

    while (usNumAttrs > 0)
    {
        THSWeaponAttribute tAttr;
        char *pcBuffer;

        rStream.ReadString(&pcBuffer);
        tAttr.strAttributeName = pcBuffer;
        delete[]pcBuffer;

        rStream.ReadString(&pcBuffer);
        tAttr.strValue = pcBuffer;
        delete[]pcBuffer;

        m_listAttributes.push_back(tAttr);

        usNumAttrs--;
    }

    return true;
}

//-------------------------
bool CHSPSetWeaponDataResponse::WriteToStream(CHSDataStream & rStream)
{
    rStream.WriteToStream(m_uiWeaponID);
    rStream.WriteToStream(m_bSuccess);
    return true;
}
bool CHSPSetWeaponDataResponse::ReadFromStream(CHSDataStream & rStream)
{
    rStream.ReadFromStream(m_uiWeaponID);
    rStream.ReadFromStream(m_bSuccess);
    return true;
}
