/*
 *      Copyright (C) 2010 Team XBMC
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

#ifndef IOSAUDIORINGBUFFER_H_
#define IOSAUDIORINGBUFFER_H_

#define RING_BUFFER_OK 0;
#define RING_BUFFER_EMPTY 1;
#define RING_BUFFER_FULL 2;
#define RING_BUFFER_NOTAVAILABLE 3;

//#define RING_BUFFER_DEBUG

#include "utils/log.h"  //CLog
#include <string.h>     //memset, memcpy

/**
 * This buffer can be used by one read and one write thread at any one time
 * without the risk of data corruption.
 * If you intend to call the Reset() method, please use Locks.
 * All other operations are thread-safe.
 */
class IOSAudioRingBuffer {

public:
	IOSAudioRingBuffer() :
    m_iReadPos(0),
    m_iWritePos(0),
    m_iRead(0),
    m_iWritten(0),
    m_iSize(0),
    m_Buffer(NULL)
  {
  }

	IOSAudioRingBuffer(unsigned int size) :
    m_iReadPos(0),
    m_iWritePos(0),
    m_iRead(0),
    m_iWritten(0),
    m_iSize(0),
    m_Buffer(NULL)
  {
    Create(size);
  }

	~IOSAudioRingBuffer()
  {
#ifdef RING_BUFFER_DEBUG
    CLog::Log(LOGDEBUG, "IOSAudioRingBuffer::~IOSAudioRingBuffer: Deleting buffer.");
#endif
    delete[] m_Buffer;
  }

  /**
   * Allocates space for buffer, and sets it's contents to 0.
   *
   * @return true on success, false otherwise
   */
  bool Create(int size)
  {
    m_Buffer = new unsigned char[ size ];
    if ( m_Buffer )
    {
      m_iSize = size;
      memset(m_Buffer, 0, m_iSize);
      return true;
    }
    return false;
  }

  /**
   * Fills the buffer with zeros and resets the pointers.
   * This method is not thread-safe, so before using this method
   * please acquire a Lock()
   */
  void Reset() {
#ifdef RING_BUFFER_DEBUG
    CLog::Log(LOGDEBUG, "IOSAudioRingBuffer::Reset: Buffer reset.");
#endif
    m_iWritten = 0;
    m_iRead = 0;
    m_iReadPos = 0;
    m_iWritePos = 0;
  }

  /**
   * Writes data to buffer.
   * Attempt to write more bytes than available results in RING_BUFFER_FULL.
   *
   * @return RING_BUFFER_OK on success, otherwise an error code
   */
	int Write(unsigned char *src, unsigned int size)
  {
    unsigned int space = GetWriteSize();

    //do we have enough space for all the data?
    if (size > space) {
#ifdef RING_BUFFER_DEBUG
    CLog::Log(LOGDEBUG, "IOSAudioRingBuffer: Not enough space, ignoring data. Requested: %u Available: %u",size, space);
#endif
      return RING_BUFFER_FULL;
    }

    //no wrapping?
    if ( m_iSize > size + m_iWritePos )
    {
#ifdef RING_BUFFER_DEBUG
      CLog::Log(LOGDEBUG, "IOSAudioRingBuffer: Written to: %u size: %u space before: %u\n", m_iWritePos, size, space);
#endif
      memcpy(&(m_Buffer[m_iWritePos]), src, size);
      m_iWritePos+=size;
    }
    //need to wrap
    else
    {
      unsigned int first = m_iSize - m_iWritePos;
      unsigned int second = size - first;
#ifdef RING_BUFFER_DEBUG
      CLog::Log(LOGDEBUG, "IOSAudioRingBuffer: Written to (split) first: %u second: %u size: %u space before: %u\n", first, second, size, space);
#endif
      memcpy(&(m_Buffer[m_iWritePos]), src, first);
      memcpy(&(m_Buffer[0]), &src[first], second);
      m_iWritePos = second;
    }

    //we can increase the write count now
    m_iWritten+=size;
    return RING_BUFFER_OK;
  }

  /**
   * Reads data from buffer.
   * Attempt to read more bytes than available results in RING_BUFFER_NOTAVAILABLE.
   * Reading from empty buffer returns RING_BUFFER_EMPTY
   *
   * @return RING_BUFFER_OK on success, otherwise an error code
   */
	int Read(unsigned char *dest, unsigned int size)
  {
    unsigned int space = GetReadSize();

    //want to read more than we have written?
    if( space <= 0 )
    {
#ifdef RING_BUFFER_DEBUG
      CLog::Log(LOGDEBUG, "IOSAudioRingBuffer: Can't read from empty buffer.");
#endif
      return RING_BUFFER_EMPTY;
    }

    //want to read more than we have available
    if( size > space )
    {
#ifdef RING_BUFFER_DEBUG
      CLog::Log(LOGDEBUG, "IOSAudioRingBuffer: Can't read %u bytes when we only have %u.", size, space);
#endif
      return RING_BUFFER_NOTAVAILABLE;
    }

    //no wrapping?
    if ( size + m_iReadPos < m_iSize )
    {
#ifdef RING_BUFFER_DEBUG
      CLog::Log(LOGDEBUG, "IOSAudioRingBuffer: Reading from: %u size: %u space before: %u\n", m_iWritePos, size, space);
#endif
      memcpy(dest, &(m_Buffer[m_iReadPos]), size);
      m_iReadPos+=size;
    }
    //need to wrap
    else
    {
      unsigned int first = m_iSize - m_iReadPos;
      unsigned int second = size - first;
#ifdef RING_BUFFER_DEBUG
      CLog::Log(LOGDEBUG, "IOSAudioRingBuffer: Reading from (split) first: %u second: %u size: %u space before: %u\n", first, second, size, space);
#endif
      memcpy(dest, &(m_Buffer[m_iReadPos]), first);
      memcpy(&dest[first], &(m_Buffer[0]), second);
      m_iReadPos = second;
    }
    //we can increase the read count now
    m_iRead+=size;

    return RING_BUFFER_OK;
  }

  /**
   * Dumps the buffer.
   */
	void Dump()
  {
    unsigned char* bufferContents = new unsigned char[m_iSize + 1];
    for (unsigned int i=0; i<m_iSize; i++) {
      if (i >= m_iReadPos && i<m_iWritePos)
        bufferContents[i] = m_Buffer[i];
      else
        bufferContents[i] = '_';
    }
    bufferContents[m_iSize] = '\0';
    CLog::Log(LOGDEBUG, "IOSAudioRingBuffer::Dump()\n%s",bufferContents);
    delete[] bufferContents;
  }

  /**
   * Returns available space for writing to buffer.
   * Attempt to write more bytes than available results in RING_BUFFER_FULL.
   */
	unsigned int GetWriteSize()
  {
    return m_iSize - ( m_iWritten - m_iRead );
  }

  /**
   * Returns available space for reading from buffer.
   * Attempt to read more bytes than available results in RING_BUFFER_EMPTY.
   */
	unsigned int GetReadSize()
  {
    return m_iWritten - m_iRead;
  }

  /**
   * Returns the buffer size.
   */
	unsigned int GetMaxSize()
  {
    return m_iSize;
  }

private:
	unsigned int m_iReadPos;
	unsigned int m_iWritePos;
	unsigned int m_iRead;
	unsigned int m_iWritten;
	unsigned int m_iSize;
	unsigned char *m_Buffer;
};
#endif //#define IOSAUDIORINGBUFFER_H_
