// -----------------------------------------------------------------------
// $Id: HSFileDatabase.h,v 1.4 2006/04/04 12:37:11 mark Exp $
// -----------------------------------------------------------------------

#if !defined(__HSFILEDATABASE_H__)
#define __HSFILEDATABASE_H__


// Library Includes
#include <list>
#include <string>

// Local Includes
#include "hsobject.h"

//! Section indicator token used in file database processing
const HS_INT8 FILE_DATABASE_SECTION_TOKEN = '!';

//! Handle file based database reads and writes
class CHSFileDatabase:public CHSObject
{
    // Member Variables
  public:

    //! Return codes for database operations
    enum EHSReturnVal
    {
        DB_OK = 0,
        DB_FILE_NOT_OPEN,
        DB_FILE_NOT_FOUND,
        DB_CREATE_FILE_FAILED,
        DB_INVALID_OPERATION,
        DB_INVALID_PARAMETER,
        DB_END_OF_FILE,
        DB_INVALID_ATTRIBUTE_NAME,
        DB_SECTION_READ_ERROR
    };

    //! Mode used for file access -- READ or WRITE
    enum EHSFileOpenMode
    {
        OPENMODE_READ = 0,
        OPENMODE_WRITE
    };

    //! storage definition for a tagged database entry
    typedef struct tagHSDBEntry
    {
        std::string strAttributeName;
        std::string strValue;
    } THSDBEntry;

    //! Default constructor
        CHSFileDatabase();
    //! Default deconstrcuctor
       ~CHSFileDatabase();

    //! Call this to open a database file for writing or reading.
    EHSReturnVal OpenFile(const HS_INT8 * pcFilePath, EHSFileOpenMode eMode);

    //! Call this to close the current database file.
    void CloseFile();

    //! Call this to begin a new section of attributes in the database.
    EHSReturnVal StartSection();

    //! Call this to commit all current section attributes to file.
    EHSReturnVal EndSection();

    //! Call this to negate all section attributes that would have 
    //! been committed to file.
    void CancelSection()
    {
        m_listSectionAttributes.clear();
    }

    //! Call this to load the first, and all subsequent, sections from 
    //! the database file.  After a section is loaded, you can use 
    //! GetFirstAttribute() and GetNextAttribute() to retrieve individual 
    //! attributes.
    EHSReturnVal LoadNextSection();

    //! Call these functions to get the first and subsequent attributes 
    //! from a loaded database section.  You must call LoadNextSection() 
    //! before using these.
    HS_BOOL8 GetFirstAttribute(THSDBEntry & rtEntry);
    HS_BOOL8 GetNextAttribute(THSDBEntry & rtEntry);

    EHSReturnVal AddSectionAttribute(const HS_INT8 * pcAttributeValue)
    {
        THSDBEntry tEntry;

        if (!pcAttributeValue)
        {
            return DB_INVALID_PARAMETER;
        }

        if (*pcAttributeValue == FILE_DATABASE_SECTION_TOKEN)
        {
            return DB_INVALID_ATTRIBUTE_NAME;
        }

        // Just use attribute name as the parameter to store this value, and
        // we'll just write out the name with no "=<value>"
        tEntry.strAttributeName = pcAttributeValue;

        m_listSectionAttributes.push_back(tEntry);

        return DB_OK;
    }

    EHSReturnVal AddSectionAttribute(const HS_INT8 * pcAttributeName,
                                     const HS_INT8 * pcAttributeValue)
    {
        THSDBEntry tEntry;

        if (!pcAttributeName || !pcAttributeValue)
        {
            return DB_INVALID_PARAMETER;
        }

        if (*pcAttributeName == FILE_DATABASE_SECTION_TOKEN)
        {
            return DB_INVALID_ATTRIBUTE_NAME;
        }


        tEntry.strAttributeName = pcAttributeName;
        tEntry.strValue = pcAttributeValue;

        m_listSectionAttributes.push_back(tEntry);

        return DB_OK;
    }

  protected:

    FILE * m_pFilePtr;
    EHSFileOpenMode m_eCurrentMode;
    std::list < THSDBEntry > m_listSectionAttributes;
    std::list < THSDBEntry >::iterator m_iterCurAttribute;

};
#endif // __HSFILEDATABASE_H__
