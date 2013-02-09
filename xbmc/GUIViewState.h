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

#include "utils/LabelFormatter.h"
#include "SortFileItem.h"
#include "guilib/GUIBaseContainer.h"
#include "MediaSource.h"

class CViewState; // forward
class CFileItemList;

class CGUIViewState
{
public:
  virtual ~CGUIViewState();
  static CGUIViewState* GetViewState(int windowId, const CFileItemList& items);

  void SetViewAsControl(int viewAsControl);
  void SaveViewAsControl(int viewAsControl);
  int GetViewAsControl() const;

  SORT_METHOD SetNextSortMethod(int direction = 1);
  void SetCurrentSortMethod(int method);
  SORT_METHOD GetSortMethod() const;
  int GetSortMethodLabel() const;
  void GetSortMethodLabelMasks(LABEL_MASKS& masks) const;
  void GetSortMethods(std::vector< std::pair<int,int> > &sortMethods) const;

  SORT_ORDER SetNextSortOrder();
  SORT_ORDER GetSortOrder() const { return m_sortOrder; };
  SORT_ORDER GetDisplaySortOrder() const;
  virtual bool HideExtensions();
  virtual bool HideParentDirItems();
  virtual bool DisableAddSourceButtons();
  virtual int GetPlaylist();
  const CStdString& GetPlaylistDirectory();
  void SetPlaylistDirectory(const CStdString& strDirectory);
  bool IsCurrentPlaylistDirectory(const CStdString& strDirectory);
  virtual bool AutoPlayNextItem();
  virtual CStdString GetLockType();
  virtual CStdString GetExtensions();
  virtual VECSOURCES& GetSources();

protected:
  CGUIViewState(const CFileItemList& items);  // no direct object creation, use GetViewState()
  virtual void SaveViewState()=0;
  virtual void SaveViewToDb(const CStdString &path, int windowID, CViewState *viewState = NULL);
  void LoadViewState(const CStdString &path, int windowID);
  
  /*! \brief Add the addons source for the given content type, if the user has suitable addons
   \param content the type of addon content desired
   \param label the name of the addons source
   \param thumb the skin image to use as the icon
   */
  void AddAddonsSource(const CStdString &content, const CStdString &label, const CStdString& thumb);
  void AddLiveTVSources();

  void AddSortMethod(SORT_METHOD sortMethod, int buttonLabel, LABEL_MASKS labelmasks);
  void SetSortMethod(SORT_METHOD sortMethod);
  void SetSortOrder(SORT_ORDER sortOrder);
  const CFileItemList& m_items;

  static VECSOURCES m_sources;

  int m_currentViewAsControl;
  int m_playlist;

  std::vector<SORT_METHOD_DETAILS> m_sortMethods;
  int m_currentSortMethod;

  SORT_ORDER m_sortOrder;

  static CStdString m_strPlaylistDirectory;
};

class CGUIViewStateGeneral : public CGUIViewState
{
public:
  CGUIViewStateGeneral(const CFileItemList& items);

protected:
  virtual void SaveViewState() {};
};

class CGUIViewStateFromItems : public CGUIViewState
{
public:
  CGUIViewStateFromItems(const CFileItemList& items);

protected:
  virtual void SaveViewState();
};
