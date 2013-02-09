#pragma once
/*
 *      Copyright (C) 2005-2009 Team XBMC
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

#include "guilib/GUIDialog.h"
#include "video/Teletext.h"

class CBaseTexture;

class CGUIDialogTeletext : public CGUIDialog
{
public:
  CGUIDialogTeletext(void);
  virtual ~CGUIDialogTeletext(void);
  virtual bool OnMessage(CGUIMessage& message);
  virtual bool OnAction(const CAction& action);
  virtual bool OnBack(int actionID);
  virtual void Render();
  virtual void OnInitWindow();
  virtual void OnDeinitWindow(int nextWindowID);

protected:
  bool                m_bClose;           /* Close sendet, needed for fade out */
  CBaseTexture*       m_pTxtTexture;      /* Texture info class to render to screen */
  CRect               m_vertCoords;       /* Coordinates of teletext field on screen */
  CTeletextDecoder    m_TextDecoder;      /* Decoding class for teletext code */
};
