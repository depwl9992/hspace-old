#if !defined(__HSPACKETHANDLER_H__)
#define __HSPACKETHANDLER_H__

class CHSPacket;

// Responsible for handling one or more types of packets from the network.
class CHSPacketHandler
{
  public:

  protected:

  public:

    CHSPacketHandler()
    {
    }

    virtual ~CHSPacketHandler()
    {
    }

    virtual HS_BOOL8 Initialize() = 0;

  protected:
};

#endif // __HSPACKETHANDLER_H__
