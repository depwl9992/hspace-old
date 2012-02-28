// -----------------------------------------------------------------------
//! $Id: hsobject.h,v 1.4 2006/04/04 12:56:10 mark Exp $
// -----------------------------------------------------------------------

#ifndef __HSOBJECT_INCLUDED__
#define __HSOBJECT_INCLUDED__

#ifdef _WIN32
#pragma once
#endif

//! Base class for all space objects
class CHSObject
{
  public:

    //! Default constructor, set reference count to one
    CHSObject():m_uiRefCount(1)
    {
    }

    //! Decrement the reference count, if it is 0, delete the object
    void Release()
    {
        if (m_uiRefCount > 0)
        {
            m_uiRefCount--;

            if (0 == m_uiRefCount)
            {
                delete this;
            }
        }
    }

    //! Add a reference to the object by increasing the reference count
    void AddRef()
    {
        m_uiRefCount++;
    }

  protected:
    //! Default deconstructor, unused
    virtual ~CHSObject()
    {
    }

    //! Storage variable for the reference count to this object:w
    unsigned int m_uiRefCount;
};


#endif
