// -----------------------------------------------------------------------
// $Id: HSSingleton.h,v 1.4 2006/04/04 12:39:49 mark Exp $
// -----------------------------------------------------------------------

#if !defined(__HSSINGLETON_H__)
#define __HSSINGLETON_H__
#include <cassert>

template < typename T > class CHSSingleton
{
  public:

    CHSSingleton()
    {
        assert(NULL == m_pTheInstance);

        int offset = (int) (T *) 1 - (int) (CHSSingleton < T > *)(T *) 1;
        m_pTheInstance = (T *) ((int) this + offset);

    }

    ~CHSSingleton()
    {
        assert(m_pTheInstance);
        m_pTheInstance = NULL;
    }

    static T & GetInstance()
    {
        assert(m_pTheInstance);
        return (*m_pTheInstance);
    }

    static T *GetCHSSingletonPtr()
    {
        return (m_pTheInstance);
    }

  protected:
    static T *m_pTheInstance;

};

template < typename T > T * CHSSingleton < T >::m_pTheInstance = 0;

#endif
