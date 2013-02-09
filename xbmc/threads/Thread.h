/*
 *      Copyright (C) 2005-2008 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

// Thread.h: interface for the CThread class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_THREAD_H__ACFB7357_B961_4AC1_9FB2_779526219817__INCLUDED_) && !defined(AFX_THREAD_H__67621B15_8724_4B5D_9343_7667075C89F2__INCLUDED_)
#define AFX_THREAD_H__ACFB7357_B961_4AC1_9FB2_779526219817__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
#include "system.h" // for HANDLE
#ifdef _LINUX
#include "PlatformInclude.h"
#endif
#include "Event.h"

class IRunnable
{
public:
  virtual void Run()=0;
  virtual ~IRunnable() {}
};

#ifdef CTHREAD
#undef CTHREAD
#endif

// minimum as mandated by XTL
#define THREAD_MINSTACKSIZE 0x10000

class CThread
{
public:
  CThread(const char* ThreadName = NULL);
  CThread(IRunnable* pRunnable, const char* ThreadName = NULL);
  virtual ~CThread();
  void Create(bool bAutoDelete = false, unsigned stacksize = 0);
  bool WaitForThreadExit(unsigned int milliseconds);
  void Sleep(unsigned int milliseconds);
  bool SetPriority(const int iPriority);
  void SetPrioritySched_RR(void);
  int GetMinPriority(void);
  int GetMaxPriority(void);
  int GetNormalPriority(void);
  HANDLE ThreadHandle();
  operator HANDLE();
  operator HANDLE() const;
  bool IsAutoDelete() const;
  virtual void StopThread(bool bWait = true);
  float GetRelativeUsage();  // returns the relative cpu usage of this thread since last call
  bool IsCurrentThread() const;

  static bool IsCurrentThread(const ThreadIdentifier tid);
  static ThreadIdentifier GetCurrentThreadId();
protected:
  virtual void OnStartup(){};
  virtual void OnExit(){};
  virtual void OnException(){} // signal termination handler
  virtual void Process();

  volatile bool m_bStop;
  HANDLE m_ThreadHandle;

  enum WaitResponse { WAIT_INTERRUPTED = -1, WAIT_SIGNALED = 0, WAIT_TIMEDOUT = 1 };

  /**
   * This call will wait on a CEvent in an interruptible way such that if
   *  stop is called on the thread the wait will return with a respone
   *  indicating what happened.
   */
  inline WaitResponse AbortableWait(CEvent& event, int timeoutMillis)
  {
    XbmcThreads::CEventGroup group(&event, &m_StopEvent, NULL);
    CEvent* result = group.wait(timeoutMillis);
    return  result == &event ? WAIT_SIGNALED : 
      (result == NULL ? WAIT_TIMEDOUT : WAIT_INTERRUPTED);
  }

  inline WaitResponse AbortableWait(CEvent& event)
  {
    XbmcThreads::CEventGroup group(&event, &m_StopEvent, NULL);
    CEvent* result = group.wait();
    return  result == &event ? WAIT_SIGNALED : 
      (result == NULL ? WAIT_TIMEDOUT : WAIT_INTERRUPTED);
  }

private:
  /*! \brief set the threadname for the debugger/callstack, implementation dependent.
   */
  void SetDebugCallStackName( const char *threadName );
  std::string GetTypeName(void);

private:
  ThreadIdentifier ThreadId() const;
  bool m_bAutoDelete;
  CEvent m_StopEvent;
  unsigned m_ThreadId; // This value is unreliable on platforms using pthreads
                       // Use m_ThreadHandle->m_hThread instead
  IRunnable* m_pRunnable;

  unsigned __int64 m_iLastUsage;
  unsigned __int64 m_iLastTime;
  float m_fLastUsage;

  std::string m_ThreadName;

#ifdef _LINUX
  static void term_handler (int signum);
#endif

#ifndef _WIN32
  static int staticThread(void* data);
#else
  static DWORD WINAPI staticThread(LPVOID* data);
#endif

private:
};

#endif // !defined(AFX_THREAD_H__ACFB7357_B961_4AC1_9FB2_779526219817__INCLUDED_)
