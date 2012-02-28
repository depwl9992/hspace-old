// -----------------------------------------------------------------------
// $Id: hscomm.h,v 1.3 2006/04/04 12:41:36 mark Exp $
// -----------------------------------------------------------------------

#ifndef __HSCOMM_INCLUDED__
#define __HSCOMM_INCLUDED__

class CHS3DObject;

//!  Storage structure for communication information
typedef struct
{
    //! Object that sent the messag
    CHS3DObject *cObj;
    //! Dbref of the source object
    HS_DBREF dbSource;
    //! Source universe id
    HS_UINT32 suid;
    //! Destination universe id
    HS_UINT32 duid;
    //! Frequency the message was sent on
    HS_FLOAT64 frq;
    //! Source coordinates
    HS_FLOAT64 sX, sY, sZ;
    //! Maximum distance this message will be allowed to propagate
    HS_FLOAT64 dMaxDist;
    //! Pointer to the data message being sent
    HS_INT8 *msg;
} HSCOMM;

//! The class that handles all communications message relay
//! in HSpace.  All messages are sent to this thing to be
//! relayed to objects in the game.
class CHSCommRelay
{
  public:
    //! Default constructor
    CHSCommRelay()
    {
    }

    //! Relay messages to other space objects as appropriate
    HS_BOOL8 RelayMessage(HSCOMM * commdata);
  private:
    //! Check if obj is on frq
    HS_BOOL8 OnFrq(int obj, HS_FLOAT64 frq);

    //! Handle passing messages to objects that are flagged as comm relays
    void RelayCommlinks(HSCOMM * commdata);
};

extern CHSCommRelay cmRelay;

#endif // __HSCOMM_INCLUDED__
