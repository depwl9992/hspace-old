// -----------------------------------------------------------------------
// $Id: hscomm.cpp,v 1.4 2006/04/04 12:41:11 mark Exp $
// -----------------------------------------------------------------------

#include "pch.h"
#include <stdlib.h>
#include <cstring>

#include "hscomm.h"
#include "hsuniverse.h"
#include "hsuniversedb.h"
#include "hspace.h"
#include "hsconf.h"
#include "hsinterface.h"
#include "hsutils.h"
#include "hsdb.h"
#include "hsansi.h"
#include "hsflags.h"
#include "hsjammer.h"

//! Singular instance for the global relay object
CHSCommRelay cmRelay;

// Relays a comm message to all commlinks in the game.
void CHSCommRelay::RelayCommlinks(HSCOMM * commdata)
{
    if (!HSCONF.use_comm_objects)
    {
        return;
    }

    if (NULL == commdata)
    {
        hs_log
            ("Invalid HSCOMM pointer passed to CHSCommRelay::RelayCommlinks()");
        return;
    }


    int idx = -1;
    int uid = -1;
    int tX = 0, tY = 0, tZ = 0;
    char *s;
    char strFrq[16];
    char strDbref[16];

    // Setup some strings to pass into the COMM_HANDLER.
    sprintf(strDbref, "#%d", commdata->dbSource);
    sprintf(strFrq, "%.2f", commdata->frq);

    for (idx = 0; idx < hsInterface.MaxObjects(); idx++)
    {
        if (!hsInterface.HasFlag(idx, TYPE_THING, THING_HSPACE_COMM) ||
            hsInterface.HasFlag(idx, TYPE_THING, THING_HSPACE_CONSOLE))
        {
            continue;
        }

        // Check attributes
        if (!hsInterface.AtrGet(idx, "UID"))
        {
            continue;
        }
        uid = atoi(hsInterface.m_buffer);

        // Get coordinates
        if (!hsInterface.AtrGet(idx, "X"))
        {
            continue;
        }
        s = hsInterface.EvalExpression(hsInterface.m_buffer, idx, idx, idx);
        tX = atoi(s);

        if (!hsInterface.AtrGet(idx, "Y"))
        {
            continue;
        }
        s = hsInterface.EvalExpression(hsInterface.m_buffer, idx, idx, idx);
        tY = atoi(s);

        if (!hsInterface.AtrGet(idx, "Z"))
        {
            continue;
        }
        s = hsInterface.EvalExpression(hsInterface.m_buffer, idx, idx, idx);
        tZ = atoi(s);

        // Target within range?
        double dDistance;
        dDistance =
            Dist3D(commdata->sX, commdata->sY, commdata->sZ, tX, tY, tZ);
        if (dDistance > commdata->dMaxDist)
        {
            continue;
        }

        if (!OnFrq(idx, commdata->frq))
        {
            continue;
        }

        // Call the object's handler function.
        if (!hsInterface.AtrGet(idx, "COMM_HANDLER"))
        {
            continue;
        }

        hsInterface.ClearEnvironmentVariables();
        hsInterface.SetEnvironmentVariable(commdata->msg);
        hsInterface.SetEnvironmentVariable(strFrq);
        hsInterface.SetEnvironmentVariable(strDbref);
        hsInterface.EvalExpression(hsInterface.m_buffer, idx, idx, idx);
    }
}

// Relays a message throughout the game givin the input data.
HS_BOOL8 CHSCommRelay::RelayMessage(HSCOMM * commdata)
{

    if (NULL == commdata)
    {
        hs_log
            ("Invalid HSCOMM pointer passed to CHSCommRelay::RelayMessage()");
        return false;
    }

    // Find the destination universe for the message
    CHSUniverse *uDest;
    uDest = CHSUniverseDB::GetInstance().FindUniverse(commdata->duid);
    if (NULL == uDest)
    {
        return false;
    }


    // Find all objects in the destination universe that
    // are within range, and relay the message to them.
    double dX, dY, dZ;          // Destination object coords
    double dDist;               // Distance between coords

    THSObjectIterator tIterator;
    HS_BOOL8 bContinue;
    for (bContinue = uDest->GetFirstObject(tIterator, HST_SHIP); bContinue;
         bContinue = uDest->GetNextObject(tIterator, HST_SHIP))
    {
        CHSShip *pShip = static_cast < CHSShip * >(tIterator.pValue);

        CHSSysJammer *cJammer;

        cJammer = (CHSSysJammer *) pShip->GetEngSystem(HSS_JAMMER);

        if (!cJammer)
        {
            continue;
        }

        dX = pShip->GetX();
        dY = pShip->GetY();
        dZ = pShip->GetZ();

        // Calculate squared distance between objects.  Don't
        // bother doing sqrt, cause it's slow as a snail!
        dDist = ((commdata->sX - dX) * (commdata->sX - dX) +
                 (commdata->sY - dY) * (commdata->sY - dY) +
                 (commdata->sZ - dZ) * (commdata->sZ - dZ));

        if (dDist < cJammer->GetRange(true))
        {
            CHSShip *tShip;
            tShip = (CHSShip *) dbHSDB.FindObject(commdata->dbSource);
            if (tShip)
                tShip->NotifyConsoles(hsInterface.
                                      HSPrintf("%s%s-%s You are being jammed, source \
							bearing %d mark %d.", ANSI_HILITE,
                                               ANSI_GREEN, ANSI_NORMAL, XYAngle(dX, dY, commdata->sX, commdata->sY), ZAngle(dX, dY, dZ, commdata->sX, commdata->sY, commdata->sZ)), MSG_GENERAL);

            return false;
        }
    }

    // Relay to commlinks
    RelayCommlinks(commdata);

    double dMaxDist;            // Squared max distance to target objs
    dMaxDist = commdata->dMaxDist * commdata->dMaxDist;

    for (bContinue = uDest->GetFirstObject(tIterator); bContinue;
         bContinue = uDest->GetNextObject(tIterator))
    {
        CHS3DObject *pObject = tIterator.pValue;

        dX = pObject->GetX();
        dY = pObject->GetY();
        dZ = pObject->GetZ();

        // Calculate squared distance between objects.  Don't
        // bother doing sqrt, cause it's slow as a snail!
        dDist = ((commdata->sX - dX) * (commdata->sX - dX) +
                 (commdata->sY - dY) * (commdata->sY - dY) +
                 (commdata->sZ - dZ) * (commdata->sZ - dZ));

        if (dDist > dMaxDist)
        {
            continue;
        }

        // Give the object the message.
        pObject->HandleMessage(commdata->msg, MSG_COMMUNICATION,
                               (long *) commdata);
    }
    return true;
}

// Indicates whether the commlink object is on a given frequency.
HS_BOOL8 CHSCommRelay::OnFrq(int obj, double frq)
{

    // Check for the COMM_FRQS attr.
    if (!hsInterface.AtrGet(obj, "COMM_FRQS"))
    {
        return false;
    }

    // Separate the string into blocks of spaces 
    char *ptr;
    char *bptr;                 // Points to the first char in the frq string 
    double cfrq;
    ptr = strchr(hsInterface.m_buffer, ' ');
    bptr = hsInterface.m_buffer;

    while (ptr)
    {
        *ptr = '\0';
        cfrq = atof(bptr);

        // Do we have a match, Johnny?  
        if (cfrq == frq)
        {
            return true;
        }

        // Find the next frq in the string.  
        bptr = ptr + 1;
        ptr = strchr(bptr, ' ');
    }

    // Last frq in the string.  
    cfrq = atof(bptr);
    if (cfrq == frq)
    {
        return true;
    }

    // No match!  
    return false;
}
