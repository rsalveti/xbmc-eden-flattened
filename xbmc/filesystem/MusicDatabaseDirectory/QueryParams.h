#pragma once
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

#include "DirectoryNode.h"

namespace XFILE
{
  namespace MUSICDATABASEDIRECTORY
  {
    class CQueryParams
    {
    public:
      CQueryParams();
      long GetArtistId() { return m_idArtist; }
      long GetAlbumId() { return m_idAlbum; }
      long GetGenreId() { return m_idGenre; }
      long GetSongId() { return m_idSong; }
      long GetYear() { return m_year; }

    protected:
      void SetQueryParam(NODE_TYPE NodeType, const CStdString& strNodeName);

      friend class CDirectoryNode;
    private:
      long m_idArtist;
      long m_idAlbum;
      long m_idGenre;
      long m_idSong;
      long m_year;
    };
  }
}


