// -----------------------------------------------------------------------
// $Id: hsjammer.h,v 1.4 2006/04/04 12:41:36 mark Exp $
// -----------------------------------------------------------------------
#if !defined(__HSJAMMER_H__)
#define __HSJAMMER_H__

#include "hseng.h"


//! Communication Jammers, interfere with communications within range
class CHSSysJammer:public CHSEngSystem
{
  public:

    //! Set defaults, system type
    CHSSysJammer();

    //! Clear allocated memory, if needed
    ~CHSSysJammer();


    //! Set the given attribute to the specified val
    HS_BOOL8 SetAttributeValue(const HS_INT8 * pcAttrName,
                               const HS_INT8 * pcValue);

    //! Save jammer system info to the given file
    void SaveToFile(FILE * fp);

    //! Return a list of manipulatable attribs
    void GetAttributeList(CHSAttributeList & rlistAttrs);

    //! Get the current attribute value for the specified attrib
    HS_BOOL8 GetAttributeValue(const HS_INT8 * pcAttrName,
                               CHSVariant & rvarReturnVal,
                               HS_BOOL8 bAdjusted,
                               HS_BOOL8 bLocalOnly = false);

    //! Get current range, adjusting for power if desired
    double GetRange(HS_BOOL8 bAdjusted);

    //! Set the maximum jamming range
    void SetRange(double dRange)
    {
        if (!m_puiRange)
        {
            m_puiRange = new HS_UINT32;
        }

           *m_puiRange = (HS_UINT32) dRange;
    }

    //! REturn a pointer to the raw range attribute
    HS_UINT32 *GetRawRange()
    {
        return m_puiRange;
    }

  protected:

    //! Override Range of the sensor jammer.
    HS_UINT32 * m_puiRange;
};

#endif // __HSJAMMER_H__
