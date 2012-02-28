#include "pch.h"

#include "hspace.h"
#include "hsobjects.h"
#include "hsinterface.h"
#include "hsautopilot.h"

CHSRoster cRoster;

CHSAI::CHSAI(void)
{
    m_name = NULL;
    m_type = CHSAI_NOTHING;
    m_action = NULL;
    cRoster.Register(this);
}

CHSAI::CHSAI(const char *name, CHSAI_TYPE type, CHSAIAction action)
{
    m_name = NULL;
    SetName(name);
    SetType(type);
    SetAction(action);
    cRoster.Register(this);
}

CHSAI::~CHSAI(void)
{
    if (m_name)
        delete[]m_name;
    cRoster.Unregister(this);
}

void CHSAI::SetAction(CHSAIAction action)
{
    if (!action)
        return;

    m_action = action;
}

void CHSAI::SetName(const char *name)
{
    if (m_name)
        delete[]m_name;

    if (!name || !*name)
        return;

    m_name = new char[strlen(name) + 1];
    strcpy(m_name, name);
}

void CHSAI::SetType(CHSAI_TYPE type)
{
    m_type = type;
}

void CHSAI::Perform(CHSSysAutoPilot * cAutoPilot)
{
    if (!cAutoPilot)
        return;

    if (!m_action)
        return;

    m_action(cAutoPilot, this);
}

char *CHSAI::GetName(void)
{
    if (m_name)
        return m_name;

    return "unnamed";
}

CHSAI_TYPE CHSAI::GetType(void)
{
    return m_type;
}

const HS_INT8 *CHSAI::GetTypeName(void)
{
    switch (m_type)
    {
    case CHSAI_NAVIGATION:
        return "NAVIGATION";
        break;
    case CHSAI_AWARENESS:
        return "AWARENESS";
        break;
    case CHSAI_AGGRESSION:
        return "AGGRESSION";
        break;
    case CHSAI_COWARDICE:
        return "COWARDICE";
        break;
    case CHSAI_MANUEAVER:
        return "MANUEAVER";
        break;
    case CHSAI_ORDNANCE:
        return "ORDNANCE";
        break;
    default:
        return "NOTYPE";
    }

    return "NOTYPE";
}

void CHSAI::AddAtr(const HS_INT8 * pcName, CHSATTR_TYPE type,
                   const HS_INT8 * pcDefault)
{
    const HS_INT8 *header = "HSDATA_";
    HS_INT8 *name;
    CHSAttr *cAtr;

    if (!pcName || !*pcName)
        return;

    name = new HS_INT8[strlen(header) + strlen(pcName) + 1];
    if (!name)
        return;

    sprintf(name, "%s%s", header, pcName);

    if (!*name)
        return;

    cAtr = new CHSAttr(name, type, pcDefault);
    if (!cAtr)
        return;

    m_listAtr.push_back(cAtr);
}

void CHSAI::DelAtr(const HS_INT8 * pcName)
{
    CHSAttr *cAtr;

    cAtr = GetAtr(pcName);
    if (!cAtr)
        return;

    m_listAtr.remove(cAtr);

    delete cAtr;
}

CHSAttr *CHSAI::GetAtr(const HS_INT8 * pcName)
{
    const HS_INT8 *header = "HSDATA_";
    HS_INT8 *name;
    CHSAttr *cAtr;

    if (!pcName || !*pcName)
        return NULL;

    name = new HS_INT8[strlen(header) + strlen(pcName) + 1];
    if (!name)
        return NULL;

    sprintf(name, "%s%s", header, pcName);

    if (!*name)
        return NULL;

    for (cAtr = GetFirstAtr(); cAtr; cAtr = GetNextAtr())
    {
        if (!strcasecmp(cAtr->GetName(), name))
            return cAtr;
    }

    return NULL;
}

CHSAttr *CHSAI::GetFirstAtr(void)
{
    m_iterAtr = m_listAtr.begin();
    return GetNextAtr();
}

CHSAttr *CHSAI::GetNextAtr(void)
{
    CHSAttr *cAtr;
    if (m_iterAtr == m_listAtr.end())
        return NULL;
    cAtr = *m_iterAtr;
    m_iterAtr++;
    return cAtr;
}

CHSRoster::CHSRoster(void)
{

}

CHSRoster::~CHSRoster(void)
{

}

void CHSRoster::DumpRoster(HS_DBREF player)
{
    CHSAI *cAI;
    CHSPilot *cPilot;

    if (!hsInterface.ValidObject(player))
        return;

    hsInterface.Notify(player, " HSpace AutoPilot Roster");
    hsInterface.Notify(player, "---AIs-------------------");
    for (cAI = GetFirstAI(); cAI; cAI = GetNextAI())
    {
        hsInterface.Notify(player,
                           hsInterface.HSPrintf("%-14s %10s", cAI->GetName(),
                                                cAI->GetTypeName()));
    }
    hsInterface.Notify(player, "---Pilots----------------");
    for (cPilot = GetFirstPilot(); cPilot; cPilot = GetNextPilot())
    {
        hsInterface.Notify(player,
                           hsInterface.HSPrintf("%-16s [%s%s%s%s%s%s]",
                                                cPilot->GetName(),
                                                cPilot->
                                                GetAI(CHSAI_NAVIGATION) ? "N"
                                                : "-",
                                                cPilot->
                                                GetAI(CHSAI_AWARENESS) ? "A" :
                                                "-",
                                                cPilot->
                                                GetAI(CHSAI_AGGRESSION) ? "G"
                                                : "-",
                                                cPilot->
                                                GetAI(CHSAI_COWARDICE) ? "C" :
                                                "-",
                                                cPilot->
                                                GetAI(CHSAI_MANUEAVER) ? "M" :
                                                "-",
                                                cPilot->
                                                GetAI(CHSAI_ORDNANCE) ? "O" :
                                                "-"));
    }
    hsInterface.Notify(player, "-------------------------");
}

HS_UINT32 CHSRoster::NumAI(void)
{
    return m_listAI.size();
}

HS_UINT32 CHSRoster::NumPilot(void)
{
    return m_listPilot.size();
}

CHSAI *CHSRoster::GetAI(const HS_INT8 * name)
{
    CHSAI *cAI;
    for (cAI = GetFirstAI(); cAI; cAI = GetNextAI())
    {
        if (!strcasecmp(name, cAI->GetName()))
            return cAI;
    }
    return NULL;
}

CHSPilot *CHSRoster::GetPilot(const HS_INT8 * name)
{
    CHSPilot *cPilot;
    for (cPilot = GetFirstPilot(); cPilot; cPilot = GetNextPilot())
    {
        if (!strcasecmp(name, cPilot->GetName()))
            return cPilot;
    }
    return NULL;
}

CHSAI *CHSRoster::FindAI(const HS_INT8 * name)
{
    CHSPilot *cPilot;
    HS_INT8 *buff = NULL;
    HS_INT8 *ptr = NULL;

    if (!name || !*name)
        return NULL;

    buff = new HS_INT8[strlen(name) + 1];
    strcpy(buff, name);

    ptr = strchr(buff, '.');
    if (!ptr)
    {
        return GetAI(buff);
    }
    else
    {
        *ptr++ = '\0';
        cPilot = GetPilot(buff);
        if (!cPilot)
            return NULL;

        if (!strcasecmp(ptr, "NAVIGATION"))
            return cPilot->GetAI(CHSAI_NAVIGATION);
        else if (!strcasecmp(ptr, "AWARENESS"))
            return cPilot->GetAI(CHSAI_AWARENESS);
        else if (!strcasecmp(ptr, "AGGRESSION"))
            return cPilot->GetAI(CHSAI_AGGRESSION);
        else if (!strcasecmp(ptr, "COWARDICE"))
            return cPilot->GetAI(CHSAI_COWARDICE);
        else if (!strcasecmp(ptr, "MANUEAVER"))
            return cPilot->GetAI(CHSAI_MANUEAVER);
        else if (!strcasecmp(ptr, "ORDNANCE"))
            return cPilot->GetAI(CHSAI_ORDNANCE);
        else
            return NULL;
    }
    return NULL;
}

HS_BOOL8 CHSRoster::Register(CHSAI * cAI)
{
    if (!cAI)
        return false;

    if (GetAI(cAI->GetName()))
    {
        delete cAI;
        return false;
    }

    m_listAI.push_back(cAI);
    return true;
}

HS_BOOL8 CHSRoster::Register(CHSPilot * cPilot)
{
    if (!cPilot)
        return false;

    if (GetPilot(cPilot->GetName()))
    {
        delete cPilot;
        return false;
    }

    m_listPilot.push_back(cPilot);
    return true;
}

HS_BOOL8 CHSRoster::Unregister(CHSAI * cAI)
{
    if (!cAI)
        return false;

    //m_iterAI = m_listAI.find(cAI);

    //if (!m_iterAI)
    //      return false;

    //if (!GetAI(cAI->GetName()))
    //      return false;

    m_listAI.remove(cAI);
    return true;
}

HS_BOOL8 CHSRoster::Unregister(CHSPilot * cPilot)
{
    if (!cPilot)
        return false;

    //m_iterPilot = m_listPilot.find(cPilot);

    //if (!m_iterPilot)
    //      return false;

    //if (!GetPilot(cPilot->GetName()))
    //      return false;

    m_listPilot.remove(cPilot);
    return true;
}

CHSAI *CHSRoster::GetFirstAI(void)
{
    m_iterAI = m_listAI.begin();
    return GetNextAI();
}

CHSAI *CHSRoster::GetNextAI(void)
{
    CHSAI *cAI;
    if (m_iterAI == m_listAI.end())
        return NULL;
    cAI = *m_iterAI;
    m_iterAI++;
    return cAI;
}

CHSPilot *CHSRoster::GetFirstPilot(void)
{
    m_iterPilot = m_listPilot.begin();
    return GetNextPilot();
}

CHSPilot *CHSRoster::GetNextPilot(void)
{
    CHSPilot *cPilot;
    if (m_iterPilot == m_listPilot.end())
        return NULL;
    cPilot = *m_iterPilot;
    m_iterPilot++;
    return cPilot;
}

CHSPilot::CHSPilot(void)
{
    m_name = NULL;
    m_navigation = NULL;
    m_awareness = NULL;
    m_aggression = NULL;
    m_cowardice = NULL;
    m_manueaver = NULL;
    m_ordnance = NULL;
    cRoster.Register(this);
}

CHSPilot::CHSPilot(const HS_INT8 * name)
{
    m_name = NULL;
    SetName(name);
    m_navigation = NULL;
    m_awareness = NULL;
    m_aggression = NULL;
    m_cowardice = NULL;
    m_manueaver = NULL;
    m_ordnance = NULL;
    cRoster.Register(this);
}

CHSPilot::~CHSPilot(void)
{
    if (m_name)
        delete[]m_name;
    cRoster.Unregister(this);
}

void CHSPilot::SetName(const HS_INT8 * name)
{
    if (m_name)
        delete[]m_name;

    if (!name || !*name)
        return;

    m_name = new HS_INT8[strlen(name) + 1];
    strcpy(m_name, name);
}

void CHSPilot::SetAI(CHSAI * cAI)
{
    if (!cAI)
        return;

    switch (cAI->GetType())
    {
    case CHSAI_NAVIGATION:
        m_navigation = cAI;
        break;
    case CHSAI_AWARENESS:
        m_awareness = cAI;
        break;
    case CHSAI_AGGRESSION:
        m_aggression = cAI;
        break;
    case CHSAI_COWARDICE:
        m_cowardice = cAI;
        break;
    case CHSAI_MANUEAVER:
        m_manueaver = cAI;
        break;
    case CHSAI_ORDNANCE:
        m_ordnance = cAI;
        break;
    default:
        return;
    }
}

void CHSPilot::ClearAI(CHSAI_TYPE type)
{
    switch (type)
    {
    case CHSAI_NAVIGATION:
        if (m_navigation)
            m_navigation = NULL;
        break;
    case CHSAI_AWARENESS:
        if (m_awareness)
            m_awareness = NULL;
        break;
    case CHSAI_AGGRESSION:
        if (m_aggression)
            m_aggression = NULL;
        break;
    case CHSAI_COWARDICE:
        if (m_cowardice)
            m_cowardice = NULL;
        break;
    case CHSAI_MANUEAVER:
        if (m_manueaver)
            m_manueaver = NULL;
        break;
    case CHSAI_ORDNANCE:
        if (m_ordnance)
            m_ordnance = NULL;
        break;
    default:
        return;
    }
}

CHSAI *CHSPilot::GetAI(CHSAI_TYPE type)
{
    switch (type)
    {
    case CHSAI_NAVIGATION:
        if (m_navigation)
            return m_navigation;
        break;
    case CHSAI_AWARENESS:
        if (m_awareness)
            return m_awareness;
        break;
    case CHSAI_AGGRESSION:
        if (m_aggression)
            return m_aggression;
        break;
    case CHSAI_COWARDICE:
        if (m_cowardice)
            return m_cowardice;
        break;
    case CHSAI_MANUEAVER:
        if (m_manueaver)
            return m_manueaver;
        break;
    case CHSAI_ORDNANCE:
        if (m_ordnance)
            return m_ordnance;
        break;
    default:
        return NULL;
    }

    return NULL;
}

HS_INT8 *CHSPilot::GetName(void)
{
    return m_name;
}

CHSAttr::CHSAttr(void)
{
    int idx;
    m_type = CHSATTR_NOEXEC;
    m_name = NULL;

    for (idx = 0; idx < 10; idx++)
        m_env[idx] = NULL;

    m_default = NULL;
    m_buffer = NULL;
}

CHSAttr::CHSAttr(const HS_INT8 * pcName, CHSATTR_TYPE type,
                 const HS_INT8 * pcDefault)
{
    int idx;

    m_type = type;

    m_name = NULL;
    SetName(pcName);

    for (idx = 0; idx < 10; idx++)
        m_env[idx] = NULL;

    m_default = NULL;
    SetDefault(pcDefault);
    m_buffer = NULL;
}

CHSAttr::~CHSAttr(void)
{
    int idx;

    if (m_name)
        delete[]m_name;

    for (idx = 0; idx < 10; idx++)
    {
        if (m_env[idx])
            delete[]m_env[idx];
    }

    if (m_default)
        delete[]m_default;

    if (m_buffer)
        delete[]m_buffer;
}

CHSATTR_TYPE CHSAttr::GetType(void)
{
    return m_type;
}

void CHSAttr::SetType(CHSATTR_TYPE type)
{
    m_type = type;
}

const HS_INT8 *CHSAttr::GetName(void)
{
    if (!m_name)
        return "";
    return (const HS_INT8 *) m_name;
}

void CHSAttr::SetName(const HS_INT8 * pcName)
{
    if (m_name)
        delete[]m_name;

    if (!pcName || !*pcName)
        return;

    m_name = new HS_INT8[strlen(pcName) + 1];
    strcpy(m_name, pcName);
}

const HS_INT8 *CHSAttr::GetDefault(void)
{
    if (!m_default)
        return "";
    return (const HS_INT8 *) m_default;
}

void CHSAttr::SetDefault(const HS_INT8 * pcDefault)
{
    if (m_default)
        delete[]m_default;

    if (!pcDefault || !*pcDefault)
        return;

    m_default = new HS_INT8[strlen(pcDefault) + 1];
    strcpy(m_default, pcDefault);
}

const HS_INT8 *CHSAttr::GetEnv(HS_UINT32 which)
{
    if (!(which < 10))
        return NULL;

    if (!m_env[which])
        return "";

    return m_env[which];
}

void CHSAttr::SetEnv(HS_UINT32 which, const HS_INT8 * pcEnv)
{
    if (!(which < 10))
        return;

    if (m_env[which])
        delete[]m_env[which];

    if (!pcEnv || !*pcEnv)
        return;

    m_env[which] = new HS_INT8[strlen(pcEnv) + 1];
    strcpy(m_env[which], pcEnv);
}

void CHSAttr::SaveEnv(void)
{
    int idx;

    for (idx = 0; idx < 10; idx++)
    {
        m_envsave[idx] = hsInterface.m_registers[idx];
        hsInterface.m_registers[idx] = m_env[idx];
    }
}

void CHSAttr::RestoreEnv(void)
{
    int idx;

    for (idx = 0; idx < 10; idx++)
    {
        hsInterface.m_registers[idx] = m_envsave[idx];
        m_envsave[idx] = NULL;
    }
}

const HS_INT8 *CHSAttr::GetBuffer(void)
{
    const HS_INT8 *ptr = m_buffer;

    if (!ptr)
        ptr = m_default;

    if (!ptr)
        return "";

    return ptr;
}

void CHSAttr::SetBuffer(const HS_INT8 * pcBuffer)
{
    if (m_buffer)
        delete[]m_buffer;

    if (!pcBuffer || !*pcBuffer)
        return;

    m_buffer = new HS_INT8[strlen(pcBuffer) + 1];
    strcpy(m_buffer, pcBuffer);
}

const HS_INT8 *CHSAttr::ToString(void)
{
    const HS_INT8 *ptr = GetBuffer();

    return ptr;
}

HS_BOOL8 CHSAttr::ToBool(void)
{
    const HS_INT8 *ptr = GetBuffer();

    if (!ptr || !*ptr)
        return false;

    if (!strcasecmp(ptr, "0") || !strcasecmp(ptr, "false")
        || !strcasecmp(ptr, "no") || !strcasecmp(ptr, "off"))
        return false;

    return true;
}

HS_INT32 CHSAttr::ToInt32(void)
{
    const HS_INT8 *ptr = GetBuffer();

    if (!ptr || !*ptr)
        return 0;

    return strtol(ptr, NULL, 10);
}

HS_DBREF CHSAttr::ToDbref(void)
{
    const HS_INT8 *ptr = GetBuffer();

    if (!ptr || !*ptr)
        return HSNOTHING;

    if ((strlen(ptr) < 2) && *ptr++ != '#')
        return HSNOTHING;

    if (strtol(ptr, NULL, 10) < 0)
        return HSNOTHING;

    return strtol(ptr, NULL, 10);
}

HS_FLOAT64 CHSAttr::ToFloat64(void)
{
    const HS_INT8 *ptr = GetBuffer();

    if (!ptr || !*ptr)
        return 0.0;

    return strtod(ptr, NULL);
}

void CHSAttr::Execute(HS_DBREF obj)
{
    if (!m_name)
    {
        SetBuffer(NULL);
        return;
    }

    if (!hsInterface.ValidObject(obj))
    {
        SetBuffer(NULL);
        return;
    }

    if (!hsInterface.AtrGet(obj, m_name))
    {
        SetBuffer(NULL);
        return;
    }

    switch (m_type)
    {
    case CHSATTR_FUNCTION:
        SaveEnv();
        SetBuffer(hsInterface.
                  EvalExpression(hsInterface.m_buffer, obj, obj, obj));
        RestoreEnv();
        break;
    case CHSATTR_TRIGGER:
        SaveEnv();
        if (hsInterface.EvalCommand(hsInterface.m_buffer, obj, obj))
            SetBuffer("TRUE");
        else
            SetBuffer("FALSE");
        RestoreEnv();
        break;
    default:
        SetBuffer(hsInterface.m_buffer);
        break;
    }
}
