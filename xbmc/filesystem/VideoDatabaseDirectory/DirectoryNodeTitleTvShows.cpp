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

#include "DirectoryNodeTitleTvShows.h"
#include "QueryParams.h"
#include "video/VideoDatabase.h"

using namespace XFILE::VIDEODATABASEDIRECTORY;

CDirectoryNodeTitleTvShows::CDirectoryNodeTitleTvShows(const CStdString& strName, CDirectoryNode* pParent)
  : CDirectoryNode(NODE_TYPE_TITLE_TVSHOWS, strName, pParent)
{

}

NODE_TYPE CDirectoryNodeTitleTvShows::GetChildType() const
{
  return NODE_TYPE_SEASONS;
}

CStdString CDirectoryNodeTitleTvShows::GetLocalizedName() const
{
  CVideoDatabase db;
  if (db.Open())
    return db.GetTvShowTitleById(GetID());
  return "";
}

bool CDirectoryNodeTitleTvShows::GetContent(CFileItemList& items) const
{
  CVideoDatabase videodatabase;
  if (!videodatabase.Open())
    return false;

  CQueryParams params;
  CollectQueryParams(params);

  bool bSuccess=videodatabase.GetTvShowsNav(BuildPath(), items, params.GetGenreId(), params.GetYear(), params.GetActorId(), params.GetDirectorId(), params.GetStudioId());

  videodatabase.Close();

  return bSuccess;
}
