// -----------------------------------------------------------------------
// $Id: hscloaking.h,v 1.6 2006/04/04 12:56:10 mark Exp $
// -----------------------------------------------------------------------

#if !defined(__HSCLOAKING_H__)
#define __HSCLOAKING_H__


// Library Includes
#include "hseng.h"


//! Cloaking Device, Generic system with some enhancements.
class CHSSysCloak:public CHSEngSystem
{
  public:
    //! Default constructor -- init values, set sys type
    CHSSysCloak();

    //! Default deconstructor
    virtual ~CHSSysCloak();

    //! Set the specified system attribute to the given value
    HS_BOOL8 SetAttributeValue(const HS_INT8 * pcAttrName,
                               const HS_INT8 * pcValue);

    //! Save cloaking system info to the specified file
    void SaveToFile(FILE * fp);

    //! Return the list of modifiable attribs
    void GetAttributeList(CHSAttributeList & rlistAttrs);

    //! Get the current value of the given attribute
    HS_BOOL8 GetAttributeValue(const HS_INT8 * pcAttrName,
                               CHSVariant & rvarReturnVal,
                               HS_BOOL8 bAdjusted,
                               HS_BOOL8 bLocalOnly = false);


    //! Return the current efficiency of the cloaking sys
    HS_FLOAT32 GetEfficiency(HS_BOOL8 bAdjusted);

    //! Is the system engaged?
    int GetEngaged();

    //! Toggle if the system is engaged or not
    void SetEngaged(HS_BOOL8 bEngage);

    //! Check to see if the system is currently being engaged
    HS_BOOL8 IsEngaging();

    //! Singular cycle processes the engaging count, nothing else
    void DoCycle();

    //! Set the current efficiency of the cloaking sys
    void SetEfficiency(float fValue)
    {
        if (!m_efficiency)
        {
            m_efficiency = new float;
        }
           *m_efficiency = fValue;
    }

    //! Return pointer to the raw efficiency member
    float *GetRawEfficiency()
    {
        return m_efficiency;
    }

  protected:

    //! Efficiency of the cloaking device (0 - 100)
    float *m_efficiency;

    //! Engaging count handler
    float m_engaging;

    //! Engaged toggle
    int m_engaged;
};

#endif // __HSCLOAKING_H__
