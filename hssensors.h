// -----------------------------------------------------------------------
// $Id: hssensors.h,v 1.7 2006/04/04 12:56:10 mark Exp $
// -----------------------------------------------------------------------

#if !defined(__HSSENSORS_H__)
#define __HSSENSORS_H__

// Library Includes
#include "hseng.h"

//! Sensor contact status enumeration
enum CONTACT_STATUS
{
    UPDATED = 0,                // It was detected, now identified
    DETECTED,                   // Only detected
    IDENTIFIED,                 // Instantly detected AND identified
};

//! Base contact struct for storing related data
struct SENSOR_CONTACT
{
    HS_UINT32 m_id;             // Contact number
    CHS3DObject *m_obj;
    HS_BOOL8 bVerified;         // Used each cycle to indicate object still on sensors
    CONTACT_STATUS status;
};

//! Maximum number of contacts to be tracked 
#define HS_MAX_CONTACTS		64
//! Maximum number of new contacts
#define HS_MAX_NEW_CONTACTS 64

//! @brief A system specifically for sensors, derived from
//! the base CHSEngSystem class.  This class handles the
//! list of contacts currently on sensors.
class CHSSysSensors:public CHSEngSystem
{
  public:
    //! Default constructor for the sensor system
    CHSSysSensors();

    //! Default deconstructor does nothing
    virtual ~CHSSysSensors()
    {
    }

    //! Set an attribute value to pcValue
    HS_BOOL8 SetAttributeValue(const HS_INT8 * pcAttrName,
                               const HS_INT8 * pcValue);

    //! Get the current list of modifiable attributes
    void GetAttributeList(CHSAttributeList & rlistAttrs);

    //! Get the current attribute value
    HS_BOOL8 GetAttributeValue(const HS_INT8 * pcAttrName,
                               CHSVariant & rvarReturnVal,
                               HS_BOOL8 bAdjusted,
                               HS_BOOL8 bLocalOnly = false);

    //! Get the current sensor rating
    HS_UINT32 GetSensorRating();

    //! Return the current number of contacts
    HS_UINT32 NumContacts();

    //! Return the current number of contacts based on the specified type
    HS_UINT32 NumContacts(int iType);

    //! Save the sensory system information to the specified file
    void SaveToFile(FILE * fp);

    //! Process the sensor swee cycle; gain, lose, identify contacts
    void DoCycle();

    //! Clear current and lost contact list structures
    void ClearContacts();

    //! Called externally to force object to be lost from sensors
    void LoseObject(CHS3DObject *);

    void SetSensorRating(int iValue)
    {
        if (!m_puiSensorRating)
        {
            m_puiSensorRating = new HS_UINT32;
        }

        if (NULL != m_puiSensorRating)
        {
            *m_puiSensorRating = iValue;
        }
    }


    //! @brief Set index to the first new contact slot.  Use GetNextNewContact()
    //! to retrieve the data
    SENSOR_CONTACT *GetFirstNewContact();

    //! Fetch the current contact index data
    SENSOR_CONTACT *GetNextNewContact();

    //! @brief Set index to the first lost contact slot. Use GetNextLostContact()
    //! to retrieve the data
    SENSOR_CONTACT *GetFirstLostContact();

    //! Return the sensor data for the lost contact at the current index position
    SENSOR_CONTACT *GetNextLostContact();

    //! Get Contact information at the specified slot
    SENSOR_CONTACT *GetContact(int slot);       // Retrieves by slot

    //! Retrieve contact information by object
    SENSOR_CONTACT *GetContact(CHS3DObject * cObj);     // Retrieves by object

    //! Get contact information by specific contact id
    SENSOR_CONTACT *GetContactByID(int id);     // Retrives by ID

    //! Return the raw pointer to the sensor rating
    HS_UINT32 *GetRawSensorRating()
    {
        return m_puiSensorRating;
    }

  protected:
    //! Add a new contact based on the 3DObject & stat
    void NewContact(CHS3DObject * cObj, CONTACT_STATUS stat);

    //! Update contact information
    void UpdateContact(SENSOR_CONTACT *);

    //! Process loss of a specific sensor contact
    void LoseContact(SENSOR_CONTACT *);

    //! Return a 4 digit random id for a new contact; id is unique among contacts
    HS_UINT32 RandomSensorID();

    //! Overridable variables at the ship level
    HS_UINT32 *m_puiSensorRating;

    //! Current contact list
    SENSOR_CONTACT *m_contact_list[HS_MAX_CONTACTS];    // Contact list

    //! Stores new contacts
    SENSOR_CONTACT *m_new_contacts[HS_MAX_NEW_CONTACTS];

    //! Lost contact list
    SENSOR_CONTACT *m_lost_contacts[HS_MAX_NEW_CONTACTS];

    //! Index to the current sensor contact slot being queried
    HS_UINT32 uContactSlot;

    //! Current number of known contacts
    HS_UINT32 m_nContacts;
};


#endif // __HSSENSORS_H__
