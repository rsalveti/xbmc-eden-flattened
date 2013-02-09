#pragma once
/*
 *      Copyright (C) 2005-2010 Team XBMC
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

class CVariant;
namespace ANNOUNCEMENT
{
  enum EAnnouncementFlag
  {
    Player = 0x1,
    GUI = 0x2,
    System = 0x4,
    VideoLibrary = 0x8,
    AudioLibrary = 0x10,
    Other = 0x20
  };

  #define ANNOUNCE_ALL (Player | GUI | System | VideoLibrary | AudioLibrary | Other)

  class IAnnouncer
  {
  public:
    IAnnouncer() { };
    virtual ~IAnnouncer() { };
    virtual void Announce(EAnnouncementFlag flag, const char *sender, const char *message, const CVariant &data) = 0;
  };
}
