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


#pragma once

#ifdef __GNUC__
// under gcc, inline will only take place if optimizations are applied (-O). this will force inline even whith optimizations.
#define XBMC_FORCE_INLINE __attribute__((always_inline))
#else
#define XBMC_FORCE_INLINE
#endif

// include as less is possible to prevent dependencies
#include "system.h"
#include "DVDDemuxers/DVDDemux.h"
#include "DVDMessageTracker.h"
#include "DVDResource.h"

#include <assert.h>

class CDVDMsg : public IDVDResourceCounted<CDVDMsg>
{
public:
  enum Message
  {
    NONE = 1000,


    // messages used in the whole system

    GENERAL_RESYNC,                 //
    GENERAL_FLUSH,                  // flush all buffers
    GENERAL_RESET,                  // reset codecs for new data
    GENERAL_STREAMCHANGE,           //
    GENERAL_SYNCHRONIZE,            //
    GENERAL_DELAY,                  //
    GENERAL_GUI_ACTION,             // gui action of some sort
    GENERAL_EOF,                    // eof of stream


    // player core related messages (cdvdplayer.cpp)

    PLAYER_SET_AUDIOSTREAM,         //
    PLAYER_SET_SUBTITLESTREAM,      //
    PLAYER_SET_SUBTITLESTREAM_VISIBLE, //
    PLAYER_SET_STATE,               // restore the dvdplayer to a certain state
    PLAYER_SET_RECORD,              // set record state
    PLAYER_SEEK,                    //
    PLAYER_SEEK_CHAPTER,            //
    PLAYER_SETSPEED,                // set the playback speed

    PLAYER_CHANNEL_NEXT,            // switches to next playback channel
    PLAYER_CHANNEL_PREV,            // switches to previous playback channel
    PLAYER_CHANNEL_SELECT,          // switches to given playback channel
    PLAYER_STARTED,                 // sent whenever a sub player has finished it's first frame after open

    // demuxer related messages

    DEMUXER_PACKET,                 // data packet
    DEMUXER_RESET,                  // reset the demuxer


    // video related messages

    VIDEO_NOSKIP,                   // next pictures is not to be skipped by the video renderer
    VIDEO_SET_ASPECT,               // set aspectratio of video

    // audio related messages

    AUDIO_SILENCE,

    // subtitle related messages
    SUBTITLE_CLUTCHANGE
  };

  CDVDMsg(Message msg)
  {
    m_message = msg;

#ifdef DVDDEBUG_MESSAGE_TRACKER
    g_dvdMessageTracker.Register(this);
#endif
  }

  virtual ~CDVDMsg()
  {
#ifdef DVDDEBUG_MESSAGE_TRACKER
    g_dvdMessageTracker.UnRegister(this);
#endif
  }

  /**
   * checks for message type
   */
  inline bool IsType(Message msg) XBMC_FORCE_INLINE
  {
    return (m_message == msg);
  }

  inline Message GetMessageType() XBMC_FORCE_INLINE
  {
    return m_message;
  }

  long GetNrOfReferences()
  {
    return m_refs;
  }

private:
  Message m_message;
};

////////////////////////////////////////////////////////////////////////////////
//////
////// GENERAL_ Messages
//////
////////////////////////////////////////////////////////////////////////////////

class CDVDMsgGeneralResync : public CDVDMsg
{
public:
  CDVDMsgGeneralResync(double timestamp, bool clock) : CDVDMsg(GENERAL_RESYNC)  { m_timestamp = timestamp; m_clock = clock; }
  double m_timestamp;
  bool m_clock;
};

#define SYNCSOURCE_AUDIO  0x00000001
#define SYNCSOURCE_VIDEO  0x00000002
#define SYNCSOURCE_SUB    0x00000004
#define SYNCSOURCE_ALL    (SYNCSOURCE_AUDIO | SYNCSOURCE_VIDEO | SYNCSOURCE_SUB)

class CDVDMsgGeneralSynchronize : public CDVDMsg
{
public:
  CDVDMsgGeneralSynchronize(DWORD timeout, DWORD sources);

  // waits until all threads waiting, released the object
  // if abort is set somehow
  void Wait(volatile bool *abort, DWORD source);
private:
  DWORD m_sources;
  long m_objects;
  unsigned int m_timeout;
};

template <typename T>
class CDVDMsgType : public CDVDMsg
{
public:
  CDVDMsgType(Message type, T value)
    : CDVDMsg(type)
    , m_value(value)
  {}
  operator T() { return m_value; }
  T m_value;
};

typedef CDVDMsgType<bool>   CDVDMsgBool;
typedef CDVDMsgType<int>    CDVDMsgInt;
typedef CDVDMsgType<double> CDVDMsgDouble;

////////////////////////////////////////////////////////////////////////////////
//////
////// PLAYER_ Messages
//////
////////////////////////////////////////////////////////////////////////////////

class CDVDMsgPlayerSetAudioStream : public CDVDMsg
{
public:
  CDVDMsgPlayerSetAudioStream(int streamId) : CDVDMsg(PLAYER_SET_AUDIOSTREAM) { m_streamId = streamId; }
  int GetStreamId()                     { return m_streamId; }
private:
  int m_streamId;
};

class CDVDMsgPlayerSetSubtitleStream : public CDVDMsg
{
public:
  CDVDMsgPlayerSetSubtitleStream(int streamId) : CDVDMsg(PLAYER_SET_SUBTITLESTREAM) { m_streamId = streamId; }
  int GetStreamId()                     { return m_streamId; }
private:
  int m_streamId;
};

class CDVDMsgPlayerSetState : public CDVDMsg
{
public:
  CDVDMsgPlayerSetState(std::string& state) : CDVDMsg(PLAYER_SET_STATE) { m_state = state; }
  std::string GetState()                { return m_state; }
private:
  std::string m_state;
};

class CDVDMsgPlayerSeek : public CDVDMsg
{
public:
  CDVDMsgPlayerSeek(int time, bool backward, bool flush = true, bool accurate = true, bool restore = true, bool trickplay = false)
    : CDVDMsg(PLAYER_SEEK)
    , m_time(time)
    , m_backward(backward)
    , m_flush(flush)
    , m_accurate(accurate)
    , m_restore(restore)
    , m_trickplay(trickplay)
  {}
  int  GetTime()              { return m_time; }
  bool GetBackward()          { return m_backward; }
  bool GetFlush()             { return m_flush; }
  bool GetAccurate()          { return m_accurate; }
  bool GetRestore()           { return m_restore; }
  bool GetTrickPlay()         { return m_trickplay; }
private:
  int  m_time;
  bool m_backward;
  bool m_flush;
  bool m_accurate;
  bool m_restore; // whether to restore any EDL cut time
  bool m_trickplay;
};

class CDVDMsgPlayerSeekChapter : public CDVDMsg
{
  public:
    CDVDMsgPlayerSeekChapter(int iChapter)
      : CDVDMsg(PLAYER_SEEK_CHAPTER)
      , m_iChapter(iChapter)
    {}

    int GetChapter() const { return m_iChapter; }

  private:

    int m_iChapter;
};

////////////////////////////////////////////////////////////////////////////////
//////
////// DEMUXER_ Messages
//////
////////////////////////////////////////////////////////////////////////////////

class CDVDMsgDemuxerPacket : public CDVDMsg
{
public:
  CDVDMsgDemuxerPacket(DemuxPacket* packet, bool drop = false);
  virtual ~CDVDMsgDemuxerPacket();
  DemuxPacket* GetPacket()      { return m_packet; }
  unsigned int GetPacketSize()  { if(m_packet) return m_packet->iSize; else return 0; }
  bool         GetPacketDrop()  { return m_drop; }
  DemuxPacket* m_packet;
  bool         m_drop;
};

class CDVDMsgDemuxerReset : public CDVDMsg
{
public:
  CDVDMsgDemuxerReset() : CDVDMsg(DEMUXER_RESET)  {}
};



////////////////////////////////////////////////////////////////////////////////
//////
////// VIDEO_ Messages
//////
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//////
////// SUBTITLE_ Messages
//////
////////////////////////////////////////////////////////////////////////////////

class CDVDMsgSubtitleClutChange : public CDVDMsg
{
public:
  CDVDMsgSubtitleClutChange(BYTE* data) : CDVDMsg(SUBTITLE_CLUTCHANGE) { memcpy(m_data, data, 16*4); }
  BYTE m_data[16][4];
private:
};
