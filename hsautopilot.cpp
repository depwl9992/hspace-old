#include "pch.h"

#include <cstdio>
#include "hspace.h"
#include "hsobjects.h"
#include "hsansi.h"
#include "hsinterface.h"
#include "hsautopilot.h"
#include <ctype.h>

CHSSysAutoPilot::CHSSysAutoPilot(void)
{
    SetType(HSS_AUTOPILOT);

    m_engaged = false;

    m_mode = CHSAP_NOTHING;

    m_navigation = NULL;
    m_awareness = NULL;
    m_aggression = NULL;
    m_cowardice = NULL;
    m_manueaver = NULL;
    m_ordnance = NULL;
}

CHSSysAutoPilot::~CHSSysAutoPilot(void)
{
    if (m_navigation)
        delete m_navigation;
    if (m_awareness)
        delete m_awareness;
    if (m_aggression)
        delete m_aggression;
    if (m_cowardice)
        delete m_cowardice;
    if (m_manueaver)
        delete m_manueaver;
    if (m_ordnance)
        delete m_ordnance;
}

HS_BOOL8 CHSSysAutoPilot::SetAttributeValue(const HS_INT8 * pcAttrName,
                                            const HS_INT8 * strValue)
{
    CHSAI *cAI;

    // Watch the name .. set the value
    if (!strcasecmp(pcAttrName, "ENGAGED"))
    {
        m_engaged = atoi(strValue) == 0 ? false : true;
        return true;
    }
    else if (!strcasecmp(pcAttrName, "MODE"))
    {
        m_mode = (CHSAP_MODE) strtoul(strValue, NULL, 10);
        return true;
    }
    else if (!strcasecmp(pcAttrName, "CONTROLLER"))
    {
        HS_DBREF dbAI = HSNOTHING;
        const HS_INT8 *ptr;

        ptr = strValue;
        if (ptr && *ptr++ == '#' && isdigit(*ptr))
            dbAI = strtol(ptr, NULL, 10);

        if (hsInterface.ValidObject(dbAI))
            m_object = dbAI;
        else
            m_object = GetOwner();

        return true;
    }
    else if (!strcasecmp(pcAttrName, "NAVIGATION")
             || !strcasecmp(pcAttrName, "AWARENESS")
             || !strcasecmp(pcAttrName, "AGGRESSION")
             || !strcasecmp(pcAttrName, "COWARDICE")
             || !strcasecmp(pcAttrName, "MANUEAVER")
             || !strcasecmp(pcAttrName, "ORDNANCE"))
    {
        cAI = cRoster.FindAI(strValue);
        if (cAI)
        {
            SetAI(cAI);
            return true;
        }
        else
            return false;
    }
    else
    {
        return CHSEngSystem::SetAttributeValue(pcAttrName, strValue);
    }
}

void CHSSysAutoPilot::SaveToFile(FILE * fp)
{
    // Save the base first
    CHSEngSystem::SaveToFile(fp);

    // Save our stuff
    fprintf(fp, "ENGAGED=%d\n", m_engaged ? 1 : 0);
    fprintf(fp, "MODE=%d\n", m_mode);
    fprintf(fp, "CONTROLLER=#%d\n", GetObj());
    if (m_navigation)
        fprintf(fp, "NAVIGATION=%s\n", m_navigation->GetName());
    if (m_awareness)
        fprintf(fp, "AWARENESS=%s\n", m_awareness->GetName());
    if (m_aggression)
        fprintf(fp, "AGGRESSION=%s\n", m_aggression->GetName());
    if (m_cowardice)
        fprintf(fp, "COWARDICE=%s\n", m_cowardice->GetName());
    if (m_manueaver)
        fprintf(fp, "MANUEAVER=%s\n", m_manueaver->GetName());
    if (m_ordnance)
        fprintf(fp, "ORDNANCE=%s\n", m_ordnance->GetName());
}

void CHSSysAutoPilot::GetAttributeList(CHSAttributeList & rlistAttrs)
{
    // Call the base class first.
    CHSEngSystem::GetAttributeList(rlistAttrs);

    // Push our own attributes.
    rlistAttrs.push_back("ENGAGED");
    rlistAttrs.push_back("MODE");
    rlistAttrs.push_back("CONTROLLER");
    rlistAttrs.push_back("NAVIGATION");
    rlistAttrs.push_back("AWARENESS");
    rlistAttrs.push_back("AGGRESSION");
    rlistAttrs.push_back("COWARDICE");
    rlistAttrs.push_back("MANUEAVER");
    rlistAttrs.push_back("ORDNANCE");
    rlistAttrs.push_back("HOSTILES");
}

HS_BOOL8
    CHSSysAutoPilot::GetAttributeValue(const HS_INT8 * pcAttrName,
                                       CHSVariant & rvarValue,
                                       HS_BOOL8 bAdjusted,
                                       HS_BOOL8 bLocalOnly)
{
    rvarValue = "";
    // Determine attribute, and return the value.
    if (!strcasecmp(pcAttrName, "ENGAGED"))
    {
        rvarValue = m_engaged;
        return true;
    }
    else if (!strcasecmp(pcAttrName, "MODE"))
    {
        rvarValue = m_mode;
        return true;
    }
    else if (!strcasecmp(pcAttrName, "CONTROLLER"))
    {
        rvarValue = hsInterface.HSPrintf("#%d", GetObj());
        return true;
    }
    else if (!strcasecmp(pcAttrName, "NAVIGATION"))
    {
        if (m_navigation)
            rvarValue = m_navigation->GetName();
        return true;
    }
    else if (!strcasecmp(pcAttrName, "AWARENESS"))
    {
        if (m_awareness)
            rvarValue = m_awareness->GetName();
        return true;
    }
    else if (!strcasecmp(pcAttrName, "AGGRESSION"))
    {
        if (m_aggression)
            rvarValue = m_aggression->GetName();
        return true;
    }
    else if (!strcasecmp(pcAttrName, "COWARDICE"))
    {
        if (m_cowardice)
            rvarValue = m_cowardice->GetName();
        return true;
    }
    else if (!strcasecmp(pcAttrName, "MANUEAVER"))
    {
        if (m_manueaver)
            rvarValue = m_manueaver->GetName();
        return true;
    }
    else if (!strcasecmp(pcAttrName, "ORDNANCE"))
    {
        if (m_ordnance)
            rvarValue = m_ordnance->GetName();
        return true;
    }
    else if (!strcasecmp(pcAttrName, "HOSTILES"))
    {
        rvarValue = ListHostiles();
        return true;
    }

    return CHSEngSystem::GetAttributeValue(pcAttrName, rvarValue, bAdjusted,
                                           bLocalOnly);
}

HS_BOOL8 CHSSysAutoPilot::IsEngaged(void)
{
    return m_engaged;
}

void CHSSysAutoPilot::SetEngaged(HS_BOOL8 bEngage)
{
    char tbuf[128];
    CHSShip *cShip;

    cShip = (CHSShip *) GetOwnerObject();
    if (!cShip || (cShip->GetType() != HST_SHIP))
        return;

    if (bEngage && !m_engaged)
    {
        m_engaged = 1;
        sprintf(tbuf,
                "%s%s-%s Autopilot engaged.",
                ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL);
        cShip->NotifyConsoles(tbuf, MSG_ENGINEERING);
    }
    else if (!bEngage && m_engaged)
    {
        m_engaged = 0;
        sprintf(tbuf,
                "%s%s-%s Autopilot disengaged.",
                ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL);
        cShip->NotifyConsoles(tbuf, MSG_ENGINEERING);
    }
}

void CHSSysAutoPilot::DoCycle(void)
{
    // Do base cycle stuff

    if (!m_engaged)
        return;

    if (GetOwnerObject() && GetOwnerObject()->GetType() == HST_SHIP)
    {
        if (IsFighting())
        {
            ControlShip(CHSAI_COWARDICE);
            if (!IsFleeing())
            {
                ControlShip(CHSAI_AWARENESS);
                ControlShip(CHSAI_MANUEAVER);
                ControlShip(CHSAI_ORDNANCE);
            }
        }
        else if (IsCruising() || IsFleeing() || IsWaiting())
        {
            if (GetFirstHostile() && !IsFleeing())
                ControlShip(CHSAI_AWARENESS);
            if (!IsFighting())
                ControlShip(CHSAI_NAVIGATION);
        }
    }
}

void CHSSysAutoPilot::SetAI(CHSAI * cAI)
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

void CHSSysAutoPilot::SetObj(HS_DBREF num)
{
    if (hsInterface.ValidObject(num))
    {
        m_object = num;
        return;
    }

    m_object = GetOwner();
}

void CHSSysAutoPilot::ControlShip(CHSAI_TYPE type)
{
    switch (type)
    {
    case CHSAI_NAVIGATION:
        if (m_navigation)
            m_navigation->Perform(this);
        break;
    case CHSAI_AWARENESS:
        if (m_awareness)
            m_awareness->Perform(this);
        break;
    case CHSAI_AGGRESSION:
        if (m_aggression)
            m_aggression->Perform(this);
        break;
    case CHSAI_COWARDICE:
        if (m_cowardice)
            m_cowardice->Perform(this);
        break;
    case CHSAI_MANUEAVER:
        if (m_manueaver)
            m_manueaver->Perform(this);
        break;
    case CHSAI_ORDNANCE:
        if (m_ordnance)
            m_ordnance->Perform(this);
        break;
    default:
        return;
    }
}

void CHSSysAutoPilot::SetMode(CHSAP_MODE mode)
{
    m_mode = mode;
}

CHSAP_MODE CHSSysAutoPilot::GetMode()
{
    return m_mode;
}

void CHSSysAutoPilot::AddHostile(CHSShip * cShip)
{
    if (!cShip)
        return;
    m_listHostiles.push_back(cShip);
}

void CHSSysAutoPilot::ClearHostiles(void)
{
    CHSShip *cShip;
    for (cShip = GetFirstHostile(); cShip; cShip = GetNextHostile())
    {
        m_listHostiles.remove(cShip);
    }
}

CHSShip *CHSSysAutoPilot::GetFirstHostile(void)
{
    m_iterHostiles = m_listHostiles.begin();
    return GetNextHostile();
}

CHSShip *CHSSysAutoPilot::GetNextHostile(void)
{
    CHSShip *cHostile;
    if (m_iterHostiles == m_listHostiles.end())
        return NULL;
    cHostile = *m_iterHostiles;
    m_iterHostiles++;
    return cHostile;
}

HS_BOOL8 CHSSysAutoPilot::IsFighting(void)
{
    return (m_mode == CHSAP_FIGHTING);
}

HS_BOOL8 CHSSysAutoPilot::IsFleeing(void)
{
    return (m_mode == CHSAP_FLEEING);
}

HS_BOOL8 CHSSysAutoPilot::IsCruising(void)
{
    return (m_mode == CHSAP_CRUISING);
}

HS_BOOL8 CHSSysAutoPilot::IsWaiting(void)
{
    return (m_mode == CHSAP_NOTHING);
}

HS_DBREF CHSSysAutoPilot::GetOwner(void)
{
    CHSShip *cShip;

    cShip = (CHSShip *) GetOwnerObject();

    if (!cShip)
        return HSNOTHING;

    return cShip->GetDbref();
}

HS_DBREF CHSSysAutoPilot::GetObj(void)
{
    if (hsInterface.ValidObject(m_object))
        return m_object;

    return GetOwner();
}

HS_INT8 *CHSSysAutoPilot::ListHostiles(void)
{
    static HS_INT8 list[1024];
    HS_INT8 *ptr;
    HS_INT32 first = 1;
    CHSShip *cShip;

    ptr = list;
    *ptr = '\0';
    for (cShip = GetFirstHostile(); cShip && strlen(list) < 1023;
         cShip = GetNextHostile())
    {
        if (!first)
            *ptr++ = ' ';
        snprintf(ptr, 1024 - strlen(list), "#%d", cShip->GetDbref());
        first = 0;
        ptr = &list[strlen(list) + 1];
    }

    return list;
}
