/*
* XBMC Media Center
* Copyright (c) 2002 Frodo
* Portions Copyright (c) by the authors of ffmpeg and xvid
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "threads/SystemClock.h"
#include "Thread.h"
#ifndef _LINUX
#include <process.h>
#include "utils/win32exception.h"
#ifndef _MT
#pragma message( "Please compile using multithreaded run-time libraries" )
#endif
typedef unsigned (WINAPI *PBEGINTHREADEX_THREADFUNC)(LPVOID lpThreadParameter);
#else
#include "PlatformInclude.h"
#include "XHandle.h"
#include <signal.h>
typedef int (*PBEGINTHREADEX_THREADFUNC)(LPVOID lpThreadParameter);
#endif

#if defined(__GNUC__) && !defined(__clang__)
#include <cxxabi.h>
using namespace __cxxabiv1;
#endif

#include "utils/log.h"
#include "utils/TimeUtils.h"
#include "threads/ThreadLocal.h"

static XbmcThreads::ThreadLocal<CThread> currentThread;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CThread::CThread(const char* ThreadName) : m_StopEvent(true,true)
{
  m_bStop = false;

  m_bAutoDelete = false;
  m_ThreadHandle = NULL;
  m_ThreadId = 0;
  m_iLastTime = 0;
  m_iLastUsage = 0;
  m_fLastUsage = 0.0f;

  m_pRunnable=NULL;

  if (ThreadName)
    m_ThreadName = ThreadName;
}

CThread::CThread(IRunnable* pRunnable, const char* ThreadName) : m_StopEvent(true,true)
{
  m_bStop = false;

  m_bAutoDelete = false;
  m_ThreadHandle = NULL;
  m_ThreadId = 0;
  m_iLastTime = 0;
  m_iLastUsage = 0;
  m_fLastUsage = 0.0f;

  m_pRunnable=pRunnable;

  if (ThreadName)
    m_ThreadName = ThreadName;
}

CThread::~CThread()
{
  if (m_ThreadHandle != NULL)
  {
    CloseHandle(m_ThreadHandle);
  }
  m_ThreadHandle = NULL;

}

#ifndef _WIN32
void CThread::term_handler (int signum)
{
  CLog::Log(LOGERROR,"thread 0x%lx (%lu) got signal %d. calling OnException and terminating thread abnormally.", (long unsigned int)pthread_self(), (long unsigned int)pthread_self(), signum);

  CThread* curThread = currentThread.get();
  if (curThread)
  {
    curThread->m_bStop = TRUE;
    curThread->m_StopEvent.Set();

    curThread->OnException();
    if( curThread->IsAutoDelete() )
      delete curThread;
  }

  pthread_exit(NULL);
}

int CThread::staticThread(void* data)
#else
DWORD WINAPI CThread::staticThread(LPVOID* data)
#endif
{
  CThread* pThread = (CThread*)(data);
  if (!pThread) {
    CLog::Log(LOGERROR,"%s, sanity failed. thread is NULL.",__FUNCTION__);
    return 1;
  }

  if (pThread->m_ThreadName.empty())
    pThread->m_ThreadName = pThread->GetTypeName();
  pThread->SetDebugCallStackName(pThread->m_ThreadName.c_str());

  CLog::Log(LOGDEBUG,"Thread %s start, auto delete: %d", pThread->m_ThreadName.c_str(), pThread->IsAutoDelete());

  currentThread.set(pThread);
#ifndef _LINUX
  /* install win32 exception translator */
  win32_exception::install_handler();
#else
  struct sigaction action;
  action.sa_handler = term_handler;
  sigemptyset (&action.sa_mask);
  action.sa_flags = 0;
  //sigaction (SIGABRT, &action, NULL);
  //sigaction (SIGSEGV, &action, NULL);
#endif


  try
  {
    pThread->OnStartup();
  }
#ifndef _LINUX
  catch (const win32_exception &e)
  {
    e.writelog(__FUNCTION__);
    if( pThread->IsAutoDelete() )
    {
      delete pThread;
      _endthreadex(123);
      return 0;
    }
  }
#endif
  catch(...)
  {
    CLog::Log(LOGERROR, "%s - thread %s, Unhandled exception caught in thread startup, aborting. auto delete: %d", __FUNCTION__, pThread->m_ThreadName.c_str(), pThread->IsAutoDelete());
    if( pThread->IsAutoDelete() )
    {
      delete pThread;
#ifndef _LINUX
      _endthreadex(123);
#endif
      return 0;
    }
  }

  try
  {
    pThread->Process();
  }
#ifndef _LINUX
  catch (const access_violation &e)
  {
    e.writelog(__FUNCTION__);
  }
  catch (const win32_exception &e)
  {
    e.writelog(__FUNCTION__);
  }
#endif
  catch(...)
  {
    CLog::Log(LOGERROR, "%s - thread %s, Unhandled exception caught in thread process, attemping cleanup in OnExit", __FUNCTION__, pThread->m_ThreadName.c_str());
  }

  try
  {
    pThread->OnExit();
  }
#ifndef _LINUX
  catch (const access_violation &e)
  {
    e.writelog(__FUNCTION__);
  }
  catch (const win32_exception &e)
  {
    e.writelog(__FUNCTION__);
  }
#endif
  catch(...)
  {
    CLog::Log(LOGERROR, "%s - thread %s, Unhandled exception caught in thread exit", __FUNCTION__, pThread->m_ThreadName.c_str());
  }

  if ( pThread->IsAutoDelete() )
  {
    CLog::Log(LOGDEBUG,"Thread %s %"PRIu64" terminating (autodelete)", pThread->m_ThreadName.c_str(), (uint64_t)CThread::GetCurrentThreadId());
    delete pThread;
    pThread = NULL;
  }
  else
    CLog::Log(LOGDEBUG,"Thread %s %"PRIu64" terminating", pThread->m_ThreadName.c_str(), (uint64_t)CThread::GetCurrentThreadId());

// DXMERGE - this looks like it might have used to have been useful for something...
//  g_graphicsContext.DeleteThreadContext();

#ifndef _LINUX
  _endthreadex(123);
#endif
  return 0;
}

void CThread::Create(bool bAutoDelete, unsigned stacksize)
{
  if (m_ThreadHandle != NULL)
  {
    throw 1; //ERROR should not b possible!!!
  }
  m_iLastTime = XbmcThreads::SystemClockMillis() * 10000;
  m_iLastUsage = 0;
  m_fLastUsage = 0.0f;
  m_bAutoDelete = bAutoDelete;
  m_bStop = false;
  m_StopEvent.Reset();

  m_ThreadHandle = (HANDLE)_beginthreadex(NULL, stacksize, (PBEGINTHREADEX_THREADFUNC)staticThread, (void*)this, 0, &m_ThreadId);

#ifdef _LINUX
  if (m_ThreadHandle && m_ThreadHandle->m_threadValid && m_bAutoDelete)
    // FIXME: WinAPI can truncate 64bit pthread ids
    pthread_detach(m_ThreadHandle->m_hThread);
#endif
}

bool CThread::IsAutoDelete() const
{
  return m_bAutoDelete;
}

void CThread::StopThread(bool bWait /*= true*/)
{
  m_bStop = true;
  m_StopEvent.Set();
  if (m_ThreadHandle && bWait)
  {
    WaitForThreadExit(INFINITE);
    CloseHandle(m_ThreadHandle);
    m_ThreadHandle = NULL;
  }
}

ThreadIdentifier CThread::ThreadId() const
{
#ifdef _LINUX
  if (m_ThreadHandle && m_ThreadHandle->m_threadValid)
    return m_ThreadHandle->m_hThread;
  else
    return 0;
#else
  return m_ThreadId;
#endif
}


CThread::operator HANDLE()
{
  return m_ThreadHandle;
}

CThread::operator HANDLE() const
{
  return m_ThreadHandle;
}

bool CThread::SetPriority(const int iPriority)
// Set thread priority
// Return true for success
{
  bool rtn = false;

  if (m_ThreadHandle)
  {
    rtn = SetThreadPriority( m_ThreadHandle, iPriority ) == TRUE;
  }

  return(rtn);
}

void CThread::SetPrioritySched_RR(void)
{
#ifdef __APPLE__
  // Changing to SCHED_RR is safe under OSX, you don't need elevated privileges and the
  // OSX scheduler will monitor SCHED_RR threads and drop to SCHED_OTHER if it detects
  // the thread running away. OSX automatically does this with the CoreAudio audio
  // device handler thread.
  int32_t result;
  thread_extended_policy_data_t theFixedPolicy;

  // make thread fixed, set to 'true' for a non-fixed thread
  theFixedPolicy.timeshare = false;
  result = thread_policy_set(pthread_mach_thread_np(ThreadId()), THREAD_EXTENDED_POLICY,
    (thread_policy_t)&theFixedPolicy, THREAD_EXTENDED_POLICY_COUNT);

  int policy;
  struct sched_param param;
  result = pthread_getschedparam(ThreadId(), &policy, &param );
  // change from default SCHED_OTHER to SCHED_RR
  policy = SCHED_RR;
  result = pthread_setschedparam(ThreadId(), policy, &param );
#endif
}

int CThread::GetMinPriority(void)
{
#if 0
//#if defined(__APPLE__)
  struct sched_param sched;
  int rtn, policy;

  rtn = pthread_getschedparam(ThreadId(), &policy, &sched);
  int min = sched_get_priority_min(policy);

  return(min);
#else
  return(THREAD_PRIORITY_IDLE);
#endif
}

int CThread::GetMaxPriority(void)
{
#if 0
//#if defined(__APPLE__)
  struct sched_param sched;
  int rtn, policy;

  rtn = pthread_getschedparam(ThreadId(), &policy, &sched);
  int max = sched_get_priority_max(policy);

  return(max);
#else
  return(THREAD_PRIORITY_HIGHEST);
#endif
}

int CThread::GetNormalPriority(void)
{
#if 0
//#if defined(__APPLE__)
  struct sched_param sched;
  int rtn, policy;

  rtn = pthread_getschedparam(ThreadId(), &policy, &sched);
  int min = sched_get_priority_min(policy);
  int max = sched_get_priority_max(policy);

  return( min + ((max-min) / 2)  );
#else
  return(THREAD_PRIORITY_NORMAL);
#endif
}


void CThread::SetDebugCallStackName( const char *name )
{
#ifdef _WIN32
  const unsigned int MS_VC_EXCEPTION = 0x406d1388;
  struct THREADNAME_INFO
  {
    DWORD dwType;     // must be 0x1000
    LPCSTR szName;    // pointer to name (in same addr space)
    DWORD dwThreadID; // thread ID (-1 caller thread)
    DWORD dwFlags;    // reserved for future use, most be zero
  } info;

  info.dwType = 0x1000;
  info.szName = name;
  info.dwThreadID = m_ThreadId;
  info.dwFlags = 0;

  try
  {
    RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR *)&info);
  }
  catch(...)
  {
  }
#endif
}

// Get the thread name using the implementation dependant typeid() class
// and attempt to clean it.
std::string CThread::GetTypeName(void)
{
  std::string name = typeid(*this).name();
 
#if defined(_MSC_VER)
  // Visual Studio 2010 returns the name as "class CThread" etc
  if (name.substr(0, 6) == "class ")
    name = name.substr(6, name.length() - 6);
#elif defined(__GNUC__) && !defined(__clang__)
  // gcc provides __cxa_demangle to demangle the name
  char* demangled = NULL;
  int   status;

  demangled = __cxa_demangle(name.c_str(), NULL, 0, &status);
  if (status == 0)
    name = demangled;
  else
    CLog::Log(LOGDEBUG,"%s, __cxa_demangle(%s) failed with status %d", __FUNCTION__, name.c_str(), status);

  if (demangled)
    free(demangled);
#endif

  return name;
}

bool CThread::WaitForThreadExit(unsigned int milliseconds)
// Waits for thread to exit, timeout in given number of msec.
// Returns true when thread ended
{
  if (!m_ThreadHandle) return true;

#ifndef _LINUX
  // boost priority of thread we are waiting on to same as caller
  int callee = GetThreadPriority(m_ThreadHandle);
  int caller = GetThreadPriority(GetCurrentThread());
  if(caller > callee)
    SetThreadPriority(m_ThreadHandle, caller);

  if (::WaitForSingleObject(m_ThreadHandle, milliseconds) != WAIT_TIMEOUT)
    return true;

  // restore thread priority if thread hasn't exited
  if(caller > callee)
    SetThreadPriority(m_ThreadHandle, callee);
#else
  if (!(m_ThreadHandle->m_threadValid) || pthread_join(m_ThreadHandle->m_hThread, NULL) == 0)
  {
    m_ThreadHandle->m_threadValid = false;
    return true;
  }
#endif

  return false;
}

HANDLE CThread::ThreadHandle()
{
  return m_ThreadHandle;
}

void CThread::Process()
{
  if(m_pRunnable)
    m_pRunnable->Run();
}

float CThread::GetRelativeUsage()
{
  unsigned __int64 iTime = XbmcThreads::SystemClockMillis();
  iTime *= 10000; // convert into 100ns tics

  // only update every 1 second
  if( iTime < m_iLastTime + 1000*10000 ) return m_fLastUsage;

  FILETIME CreationTime, ExitTime, UserTime, KernelTime;
  if( GetThreadTimes( m_ThreadHandle, &CreationTime, &ExitTime, &KernelTime, &UserTime ) )
  {
    unsigned __int64 iUsage = 0;
    iUsage += (((unsigned __int64)UserTime.dwHighDateTime) << 32) + ((unsigned __int64)UserTime.dwLowDateTime);
    iUsage += (((unsigned __int64)KernelTime.dwHighDateTime) << 32) + ((unsigned __int64)KernelTime.dwLowDateTime);

    if(m_iLastUsage > 0 && m_iLastTime > 0)
      m_fLastUsage = (float)( iUsage - m_iLastUsage ) / (float)( iTime - m_iLastTime );

    m_iLastUsage = iUsage;
    m_iLastTime = iTime;

    return m_fLastUsage;
  }
  return 0.0f;
}

bool CThread::IsCurrentThread() const
{
  return IsCurrentThread(ThreadId());
}


ThreadIdentifier CThread::GetCurrentThreadId()
{
#ifdef _LINUX
  return pthread_self();
#else
  return ::GetCurrentThreadId();
#endif
}

bool CThread::IsCurrentThread(const ThreadIdentifier tid)
{
#ifdef _LINUX
  return pthread_equal(pthread_self(), tid);
#else
  return (::GetCurrentThreadId() == tid);
#endif
}

void CThread::Sleep(unsigned int milliseconds)
{
  if(milliseconds > 10 && IsCurrentThread())
    m_StopEvent.WaitMSec(milliseconds);
  else
    ::Sleep(milliseconds);
}


