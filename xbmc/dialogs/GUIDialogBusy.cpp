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

#include "GUIDialogBusy.h"
#include "guilib/GUIWindowManager.h"

CGUIDialogBusy::CGUIDialogBusy(void)
  : CGUIDialog(WINDOW_DIALOG_BUSY, "DialogBusy.xml"), m_bLastVisible(false)
{
  m_loadOnDemand = false;
  m_bModal = true;
}

CGUIDialogBusy::~CGUIDialogBusy(void)
{
}

void CGUIDialogBusy::Show_Internal()
{
  m_bCanceled = false;
  m_active = true;
  m_bModal = true;
  m_bLastVisible = true;
  m_closing = false;
  g_windowManager.RouteToWindow(this);

  // active this window...
  CGUIMessage msg(GUI_MSG_WINDOW_INIT, 0, 0);
  OnMessage(msg);
}

void CGUIDialogBusy::DoProcess(unsigned int currentTime, CDirtyRegionList &dirtyregions)
{
  bool visible = g_windowManager.GetTopMostModalDialogID() == WINDOW_DIALOG_BUSY;
  if(!visible && m_bLastVisible)
    dirtyregions.push_back(m_renderRegion);
  m_bLastVisible = visible;
  CGUIDialog::DoProcess(currentTime, dirtyregions);
}

void CGUIDialogBusy::Render()
{
  if(!m_bLastVisible)
    return;
  CGUIDialog::Render();
}

bool CGUIDialogBusy::OnBack(int actionID)
{
  m_bCanceled = true;
  return true;
}
