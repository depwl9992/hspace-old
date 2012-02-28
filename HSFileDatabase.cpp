// -----------------------------------------------------------------------
// $Id: HSFileDatabase.cpp,v 1.4 2006/04/04 12:37:11 mark Exp $
// -----------------------------------------------------------------------

#include "pch.h"

#include <cstdio>

#include "HSFileDatabase.h"

CHSFileDatabase::CHSFileDatabase():
m_pFilePtr(NULL), m_eCurrentMode(OPENMODE_READ)
{
}

CHSFileDatabase::~CHSFileDatabase()
{
    if (m_pFilePtr)
    {
        fclose(m_pFilePtr);
        m_pFilePtr = NULL;
    }
}

CHSFileDatabase::EHSReturnVal
    CHSFileDatabase::OpenFile(const HS_INT8 * pcFilePath,
                              EHSFileOpenMode eMode)
{
    if (eMode == OPENMODE_READ)
    {
        m_pFilePtr = fopen(pcFilePath, "r");
        if (NULL == m_pFilePtr)
        {
            return DB_FILE_NOT_FOUND;
        }
    }
    else
    {
        m_pFilePtr = fopen(pcFilePath, "w");
        if (NULL == m_pFilePtr)
        {
            // Couldn't create it.
            return DB_CREATE_FILE_FAILED;
        }
    }

    m_eCurrentMode = eMode;

    return DB_OK;
}

void CHSFileDatabase::CloseFile()
{
    if (m_pFilePtr)
    {
        fclose(m_pFilePtr);
        m_pFilePtr = NULL;
    }
}

CHSFileDatabase::EHSReturnVal CHSFileDatabase::StartSection()
{
    if (!m_pFilePtr)
    {
        return DB_FILE_NOT_OPEN;
    }

    // We should be open in write mode.
    if (m_eCurrentMode != OPENMODE_WRITE)
    {
        return DB_INVALID_OPERATION;
    }

    // Write a section token.
    fwrite(&FILE_DATABASE_SECTION_TOKEN, 1, 1, m_pFilePtr);
    fwrite("\n", 1, 1, m_pFilePtr);

    return DB_OK;
}

CHSFileDatabase::EHSReturnVal CHSFileDatabase::EndSection()
{
    if (!m_pFilePtr)
    {
        return DB_FILE_NOT_OPEN;
    }

    // We should be open in write mode.
    if (m_eCurrentMode != OPENMODE_WRITE)
    {
        return DB_INVALID_OPERATION;
    }

    // Do we have any attributes to write?
    while (!m_listSectionAttributes.empty())
    {
        THSDBEntry & rtEntry = m_listSectionAttributes.front();

        // Write the attribute name.
        fwrite(rtEntry.strAttributeName.c_str(), 1,
               rtEntry.strAttributeName.length(), m_pFilePtr);

        // Is there an attribute value to write?
        if (rtEntry.strValue.length() > 0)
        {
            fwrite("=", 1, 1, m_pFilePtr);
            fwrite(rtEntry.strValue.c_str(), 1,
                   rtEntry.strValue.length(), m_pFilePtr);
        }

        // Next line please.
        fwrite("\n", 1, 1, m_pFilePtr);

        m_listSectionAttributes.pop_front();
    }

    return DB_OK;
}

CHSFileDatabase::EHSReturnVal CHSFileDatabase::LoadNextSection()
{
    if (!m_pFilePtr)
    {
        return DB_FILE_NOT_OPEN;
    }

    // We should be open in write mode.
    if (m_eCurrentMode != OPENMODE_READ)
    {
        return DB_INVALID_OPERATION;
    }

    m_listSectionAttributes.clear();

    // Load lines from the database until we hit the section token.
    HS_INT8 cDBCharacter;
    HS_UINT32 uiLineOffset = 0;

    while (fread(&cDBCharacter, 1, 1, m_pFilePtr) == 1)
    {
        if (cDBCharacter == EOF)
        {
            return DB_END_OF_FILE;
        }

        if (cDBCharacter == '\r' || cDBCharacter == '\n')
        {
            // We hit a new line character.
            uiLineOffset = 0;
            continue;
        }
        else if (cDBCharacter == FILE_DATABASE_SECTION_TOKEN)
        {
            // Are we at the beginning of a newline?
            if (uiLineOffset == 0)
            {
                // This is the next section.  Skip to the next line.
                while (fread(&cDBCharacter, 1, 1, m_pFilePtr) == 1)
                {
                    if (cDBCharacter == EOF)
                    {
                        // This is technically a corrupt database.
                        return DB_END_OF_FILE;
                    }

                    if (cDBCharacter == '\r' || cDBCharacter == '\n')
                    {
                        // Peek the next character to see if it's a printable.
                        while (fread(&cDBCharacter, 1, 1, m_pFilePtr) == 1)
                        {
                            if (cDBCharacter != '\r' && cDBCharacter != '\n')
                            {
                                // Rewind one character.
                                fseek(m_pFilePtr, -1, SEEK_CUR);
                                break;
                            }
                        }
                        break;
                    }
                }
                if (feof(m_pFilePtr))
                {
                    // This is technically a corrupt database.
                    return DB_END_OF_FILE;
                }

                break;
            }
        }
        uiLineOffset++;
    }

    if (feof(m_pFilePtr))
    {
        return DB_END_OF_FILE;
    }

    // We are now at the first attribute in the section.
    // Read all attributes until the next section token.
    HS_BOOL8 bLoadAttrName = true;      // Load attribute name first.
    THSDBEntry tCurrentEntry;
    uiLineOffset = 0;
    while (fread(&cDBCharacter, 1, 1, m_pFilePtr) == 1)
    {
        if (cDBCharacter == EOF)
        {
            break;
        }

        if ((cDBCharacter == FILE_DATABASE_SECTION_TOKEN) &&
            (uiLineOffset == 0))
        {
            // This is the next section.
            // Rewind one character.
            fseek(m_pFilePtr, -1, SEEK_CUR);
            break;
        }

        if (cDBCharacter == '\n' || cDBCharacter == '\r')
        {
            // Add the current database entry to the list.
            m_listSectionAttributes.push_back(tCurrentEntry);

            // Clear the entry.
            tCurrentEntry.strAttributeName = "";
            tCurrentEntry.strValue = "";

            uiLineOffset = 0;
            bLoadAttrName = true;
            continue;
        }

        if (cDBCharacter == '=')
        {
            // Are we loading the attribute name?
            if (bLoadAttrName)
            {
                bLoadAttrName = false;
                continue;
            }
            else
            {
                // Corruption.
                return DB_SECTION_READ_ERROR;
            }
        }

        // Are we loading the attribute name or value?
        if (bLoadAttrName)
        {
            tCurrentEntry.strAttributeName.append(&cDBCharacter, 1);
        }
        else
        {
            tCurrentEntry.strValue.append(&cDBCharacter, 1);
        }

        uiLineOffset++;
    }

    return DB_OK;
}

HS_BOOL8 CHSFileDatabase::GetFirstAttribute(THSDBEntry & rtEntry)
{
    m_iterCurAttribute = m_listSectionAttributes.begin();

    return GetNextAttribute(rtEntry);
}

HS_BOOL8 CHSFileDatabase::GetNextAttribute(THSDBEntry & rtEntry)
{
    if (m_iterCurAttribute == m_listSectionAttributes.end())
    {
        return false;
    }

    THSDBEntry & rtListEntry = *m_iterCurAttribute;

    rtEntry.strAttributeName = rtListEntry.strAttributeName;
    rtEntry.strValue = rtListEntry.strValue;

    m_iterCurAttribute++;

    return true;
}
