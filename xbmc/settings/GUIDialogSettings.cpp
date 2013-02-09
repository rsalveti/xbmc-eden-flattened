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

#include "GUIDialogSettings.h"
#include "guilib/GUISpinControlEx.h"
#include "guilib/GUIRadioButtonControl.h"
#include "guilib/GUISettingsSliderControl.h"
#include "guilib/GUIImage.h"
#include "guilib/GUIControlGroupList.h"
#include "guilib/LocalizeStrings.h"
#include "GUISettings.h"
#include "utils/log.h"
#include "dialogs/GUIDialogKeyboard.h"

#define CONTROL_GROUP_LIST          5
#define CONTROL_SETTINGS_LABEL      2
#define CONTROL_NONE_AVAILABLE      3
#define CONTROL_DEFAULT_BUTTON      7
#define CONTROL_DEFAULT_RADIOBUTTON 8
#define CONTROL_DEFAULT_SPIN        9
#define CONTROL_DEFAULT_SLIDER     10
#define CONTROL_DEFAULT_SEPARATOR  11
#define CONTROL_OKAY_BUTTON        28
#define CONTROL_CANCEL_BUTTON      29
#define CONTROL_START              30
#define CONTROL_PAGE               60

using namespace std;

CGUIDialogSettings::CGUIDialogSettings(int id, const char *xmlFile)
    : CGUIDialog(id, xmlFile)
{
  m_pOriginalSpin = NULL;
  m_pOriginalRadioButton = NULL;
  m_pOriginalSettingsButton = NULL;
  m_pOriginalSlider = NULL;
  m_pOriginalSeparator = NULL;
  m_usePopupSliders = false;
}

CGUIDialogSettings::~CGUIDialogSettings(void)
{
}

bool CGUIDialogSettings::OnMessage(CGUIMessage &message)
{
  switch (message.GetMessage())
  {
  case GUI_MSG_CLICKED:
    {
      unsigned int iControl = message.GetSenderId();
      if (iControl >= CONTROL_OKAY_BUTTON && iControl < CONTROL_PAGE)
        OnClick(iControl);
      return true;
    }
    break;
  case GUI_MSG_WINDOW_DEINIT:
    {
      CGUIDialog::OnMessage(message);
      FreeControls();
      m_settings.clear();
      return true;
    }
    break;
  }
  return CGUIDialog::OnMessage(message);
}

void CGUIDialogSettings::SetupPage()
{
  // cleanup first, if necessary
  FreeControls();
  m_pOriginalSpin = (CGUISpinControlEx*)GetControl(CONTROL_DEFAULT_SPIN);
  m_pOriginalRadioButton = (CGUIRadioButtonControl *)GetControl(CONTROL_DEFAULT_RADIOBUTTON);
  m_pOriginalSettingsButton = (CGUIButtonControl *)GetControl(CONTROL_DEFAULT_BUTTON);
  m_pOriginalSlider = (CGUISettingsSliderControl *)GetControl(CONTROL_DEFAULT_SLIDER);
  m_pOriginalSeparator = (CGUIImage *)GetControl(CONTROL_DEFAULT_SEPARATOR);
  if (m_pOriginalSpin) m_pOriginalSpin->SetVisible(false);
  if (m_pOriginalRadioButton) m_pOriginalRadioButton->SetVisible(false);
  if (m_pOriginalSettingsButton) m_pOriginalSettingsButton->SetVisible(false);
  if (m_pOriginalSlider) m_pOriginalSlider->SetVisible(false);
  if (m_pOriginalSeparator) m_pOriginalSeparator->SetVisible(false);

  // update our settings label
  SET_CONTROL_LABEL(CONTROL_SETTINGS_LABEL, g_localizeStrings.Get(13395 + GetID() - WINDOW_DIALOG_VIDEO_OSD_SETTINGS));

  CGUIControlGroupList *group = (CGUIControlGroupList *)GetControl(CONTROL_GROUP_LIST);
  if (!group)
    return;

  if (!m_settings.size())
  { // no settings available
    SET_CONTROL_VISIBLE(CONTROL_NONE_AVAILABLE);
    return;
  }
  else
  {
    SET_CONTROL_HIDDEN(CONTROL_NONE_AVAILABLE);
  }

  // create our controls
  for (unsigned int i = 0; i < m_settings.size(); i++)
  {
    SettingInfo &setting = m_settings.at(i);
    AddSetting(setting, group->GetWidth(), CONTROL_START + i);
  }
}

void CGUIDialogSettings::EnableSettings(unsigned int id, bool enabled)
{
  for (unsigned int i = 0; i < m_settings.size(); i++)
  {
    if (m_settings[i].id != id)
      continue;
    m_settings[i].enabled = enabled;
    if (enabled)
    {
      CONTROL_ENABLE(i + CONTROL_START);
    }
    else
    {
      CONTROL_DISABLE(i + CONTROL_START);
    }
    return;
  }
  CLog::Log(LOGWARNING, "%s - Invalid setting specified", __FUNCTION__);
}

void CGUIDialogSettings::UpdateSetting(unsigned int id)
{
  unsigned int settingNum = (unsigned int)-1;
  for (unsigned int i = 0; i < m_settings.size(); i++)
  {
    if (m_settings[i].id == id)
    {
      settingNum = i;
      break;
    }
  }
  if(settingNum == (unsigned int)-1)
    return;

  SettingInfo &setting = m_settings.at(settingNum);
  unsigned int controlID = settingNum + CONTROL_START;
  if (setting.type == SettingInfo::SPIN)
  {
    CGUISpinControlEx *pControl = (CGUISpinControlEx *)GetControl(controlID);
    if (pControl && setting.data) pControl->SetValue(*(int *)setting.data);
  }
  else if (setting.type == SettingInfo::CHECK)
  {
    CGUIRadioButtonControl *pControl = (CGUIRadioButtonControl *)GetControl(controlID);
    if (pControl && setting.data) pControl->SetSelected(*(bool *)setting.data);
  }
  else if (setting.type == SettingInfo::CHECK_UCHAR)
  {
    CGUIRadioButtonControl *pControl = (CGUIRadioButtonControl *)GetControl(controlID);
    if (pControl && setting.data) pControl->SetSelected(*(unsigned char*)setting.data ? true : false);
  }
  else if (setting.type == SettingInfo::SLIDER)
  {
    CGUISettingsSliderControl *pControl = (CGUISettingsSliderControl *)GetControl(controlID);
    float value = *(float *)setting.data;
    if (pControl && setting.data)
    {
      pControl->SetFloatValue(value);
      if (setting.formatFunction) pControl->SetTextValue(setting.formatFunction(value, setting.interval));
    }
  }
  else if (setting.type == SettingInfo::BUTTON)
  {
    SET_CONTROL_LABEL(controlID,setting.name);
    if (m_usePopupSliders && setting.data && setting.formatFunction)
      SET_CONTROL_LABEL2(controlID,setting.formatFunction(*(float *)setting.data, setting.interval));
  }
  else if (setting.type == SettingInfo::STRING)
  {
    SET_CONTROL_LABEL(controlID, setting.name);
    string strNewValue = string(*(CStdString *)setting.data);
    if (strNewValue.empty())
      strNewValue = "-";
    SET_CONTROL_LABEL2(controlID, strNewValue);
  }

  if (setting.enabled)
  {
    CONTROL_ENABLE(controlID);
  }
  else
  {
    CONTROL_DISABLE(controlID);
  }
}

bool CGUIDialogSettings::OnBack(int actionID)
{
  OnCancel();
  return CGUIDialog::OnBack(actionID);
}

void CGUIDialogSettings::OnClick(int iID)
{
  if (iID == CONTROL_OKAY_BUTTON)
  {
    OnOkay();
    Close();
    return;
  }
  if (iID == CONTROL_CANCEL_BUTTON)
  {
    OnCancel();
    Close();
    return;
  }
  unsigned int settingNum = iID - CONTROL_START;
  if (settingNum >= m_settings.size()) return;
  SettingInfo &setting = m_settings.at(settingNum);
  if (setting.type == SettingInfo::SPIN)
  {
    CGUISpinControlEx *pControl = (CGUISpinControlEx *)GetControl(iID);
    if (setting.data) *(int *)setting.data = pControl->GetValue();
  }
  else if (setting.type == SettingInfo::CHECK)
  {
    CGUIRadioButtonControl *pControl = (CGUIRadioButtonControl *)GetControl(iID);
    if (setting.data) *(bool *)setting.data = pControl->IsSelected();
  }
  else if (setting.type == SettingInfo::CHECK_UCHAR)
  {
    CGUIRadioButtonControl *pControl = (CGUIRadioButtonControl *)GetControl(iID);
    if (setting.data) *(unsigned char*)setting.data = pControl->IsSelected() ? 1 : 0;
  }
  else if (setting.type == SettingInfo::SLIDER)
  {
    CGUISettingsSliderControl *pControl = (CGUISettingsSliderControl *)GetControl(iID);
    if (setting.data) *(float *)setting.data = pControl->GetFloatValue();
    if (setting.formatFunction) pControl->SetTextValue(setting.formatFunction(pControl->GetFloatValue(), setting.interval));
  }
  else if (setting.type == SettingInfo::BUTTON && m_usePopupSliders && setting.data)
  { // we're using popup sliders
    CGUIDialogSlider::ShowAndGetInput(setting.name, *(float *)setting.data, setting.min, setting.interval, setting.max, this, &setting);
    if (setting.formatFunction)
      SET_CONTROL_LABEL2(iID, setting.formatFunction(*(float *)setting.data, setting.interval));
  }
  else if (setting.type == SettingInfo::STRING)
  {
    CGUIDialogKeyboard::ShowAndGetInput(*(CStdString *) setting.data, true);
    string strNewValue = string(*(CStdString *)setting.data);
    if (strNewValue.empty())
      strNewValue = "-";
    SET_CONTROL_LABEL2(iID, strNewValue);
  }
  OnSettingChanged(setting);
}

void CGUIDialogSettings::FreeControls()
{
  // just clear our group list
  CGUIControlGroupList *group = (CGUIControlGroupList *)GetControl(CONTROL_GROUP_LIST);
  if (group)
  {
    group->FreeResources();
    group->ClearAll();
  }
}

void CGUIDialogSettings::AddSetting(SettingInfo &setting, float width, int iControlID)
{
  CGUIControl *pControl = NULL;
  if (setting.type == SettingInfo::BUTTON && m_pOriginalSettingsButton)
  {
    pControl = new CGUIButtonControl(*m_pOriginalSettingsButton);
    if (!pControl) return ;
    ((CGUIButtonControl *)pControl)->SetLabel(setting.name);
    if (setting.formatFunction)
      ((CGUIButtonControl *)pControl)->SetLabel2(setting.formatFunction(*(float *)setting.data, setting.interval));
    pControl->SetWidth(width);
  }
  else if (setting.type == SettingInfo::SEPARATOR && m_pOriginalSeparator)
  {
    pControl = new CGUIImage(*m_pOriginalSeparator);
    if (!pControl) return ;
    pControl->SetWidth(width);
  }
  else if (setting.type == SettingInfo::CHECK || setting.type == SettingInfo::CHECK_UCHAR)
  {
    if (!m_pOriginalRadioButton) return;
    pControl = new CGUIRadioButtonControl(*m_pOriginalRadioButton);
    if (!pControl) return ;
    ((CGUIRadioButtonControl *)pControl)->SetLabel(setting.name);
    pControl->SetWidth(width);
    if (setting.data) ((CGUIRadioButtonControl *)pControl)->SetSelected(*(bool *)setting.data == 1);
  }
  else if (setting.type == SettingInfo::SPIN && setting.entry.size() > 0 && m_pOriginalSpin)
  {
    pControl = new CGUISpinControlEx(*m_pOriginalSpin);
    if (!pControl) return ;
    pControl->SetWidth(width);
    ((CGUISpinControlEx *)pControl)->SetText(setting.name);
    pControl->SetWidth(width);
    for (unsigned int i = 0; i < setting.entry.size(); i++)
      ((CGUISpinControlEx *)pControl)->AddLabel(setting.entry[i].second, setting.entry[i].first);
    if (setting.data) ((CGUISpinControlEx *)pControl)->SetValue(*(int *)setting.data);
  }
  else if (setting.type == SettingInfo::SLIDER)
  {
    if (!m_pOriginalSlider) return;
    pControl = new CGUISettingsSliderControl(*m_pOriginalSlider);
    if (!pControl) return ;
    pControl->SetWidth(width);
    ((CGUISettingsSliderControl *)pControl)->SetText(setting.name);
    if (setting.formatFunction)
      ((CGUISettingsSliderControl *)pControl)->SetTextValue(setting.formatFunction(*(float *)setting.data, setting.interval));
    ((CGUISettingsSliderControl *)pControl)->SetType(SPIN_CONTROL_TYPE_FLOAT);
    ((CGUISettingsSliderControl *)pControl)->SetFloatRange(setting.min, setting.max);
    ((CGUISettingsSliderControl *)pControl)->SetFloatInterval(setting.interval);
    if (setting.data) ((CGUISettingsSliderControl *)pControl)->SetFloatValue(*(float *)setting.data);
  }
  if (setting.type == SettingInfo::STRING && m_pOriginalSettingsButton)
  {
    pControl = new CGUIButtonControl(*m_pOriginalSettingsButton);
    if (!pControl) return ;
    ((CGUIButtonControl *)pControl)->SetLabel(setting.name);
    string strValue = string(*(CStdString *)setting.data);
    if (strValue.empty())
      strValue = "-";
    ((CGUIButtonControl *)pControl)->SetLabel2(strValue);
    pControl->SetWidth(width);
  }
  if (!pControl) return;

  pControl->SetID(iControlID);
  pControl->SetVisible(true);
  pControl->SetEnabled(setting.enabled);
  CGUIControlGroupList *group = (CGUIControlGroupList *)GetControl(CONTROL_GROUP_LIST);
  if (group)
  {
    pControl->AllocResources();
    group->AddControl(pControl);
  }
  else
    delete pControl;
}

void CGUIDialogSettings::AddButton(unsigned int id, int label, float *current, float min, float interval, float max, FORMATFUNCTION function)
{
  SettingInfo setting;
  setting.id = id;
  setting.name = g_localizeStrings.Get(label);
  setting.type = SettingInfo::BUTTON;
  setting.data = current;
  setting.min = min;
  setting.max = max;
  setting.interval = interval;
  setting.formatFunction = function;
  m_settings.push_back(setting);
}

void CGUIDialogSettings::AddString(unsigned int id, int label, CStdString *current)
{
  SettingInfo setting;
  setting.id = id;
  setting.name = g_localizeStrings.Get(label);
  setting.type = SettingInfo::STRING;
  setting.data = current;
  setting.enabled = true;
  m_settings.push_back(setting);
}

void CGUIDialogSettings::AddBool(unsigned int id, int label, bool *on, bool enabled)
{
  SettingInfo setting;
  setting.id = id;
  setting.name = g_localizeStrings.Get(label);
  setting.type = SettingInfo::CHECK;
  setting.data = on;
  setting.enabled = enabled;
  m_settings.push_back(setting);
}

void CGUIDialogSettings::AddSpin(unsigned int id, int label, int *current, unsigned int max, const int *entries)
{
  SettingInfo setting;
  setting.id = id;
  setting.name = g_localizeStrings.Get(label);
  setting.type = SettingInfo::SPIN;
  setting.data = current;
  for (unsigned int i = 0; i < max; i++)
    setting.entry.push_back(make_pair(i, g_localizeStrings.Get(entries[i])));
  m_settings.push_back(setting);
}

void CGUIDialogSettings::AddSpin(unsigned int id, int label, int *current, unsigned int min, unsigned int max, const char* minLabel)
{
  SettingInfo setting;
  setting.id = id;
  setting.name = g_localizeStrings.Get(label);
  setting.type = SettingInfo::SPIN;
  setting.data = current;
  for (unsigned int i = min; i <= max; i++)
  {
    CStdString format;
    if (i == min && minLabel)
      format = minLabel;
    else
      format.Format("%i", i);
    setting.entry.push_back(make_pair(i, format));
  }
  m_settings.push_back(setting);
}

void CGUIDialogSettings::AddSpin(unsigned int id, int label, int *current, vector<pair<int, CStdString> > &values)
{
  SettingInfo setting;
  setting.id = id;
  setting.name = g_localizeStrings.Get(label);
  setting.type = SettingInfo::SPIN;
  setting.data = current;
  setting.entry = values;
  m_settings.push_back(setting);
}

void CGUIDialogSettings::AddSpin(unsigned int id, int label, int *current, vector<pair<int, int> > &values)
{
  vector<pair<int, CStdString> > entries;
  for(unsigned i = 0; i < values.size(); i++)
    entries.push_back(make_pair(values[i].first, g_localizeStrings.Get(values[i].second)));
  AddSpin(id, label, current, entries);
}

void CGUIDialogSettings::AddSlider(unsigned int id, int label, float *current, float min, float interval, float max, FORMATFUNCTION function, bool allowPopup /* = true*/)
{
  if (m_usePopupSliders && allowPopup)
  {
    AddButton(id, label, current, min, interval, max, function);
    return;
  }
  SettingInfo setting;
  setting.id = id;
  setting.name = g_localizeStrings.Get(label);
  setting.type = SettingInfo::SLIDER;
  setting.min = min;
  setting.interval = interval;
  setting.max = max;
  setting.data = current;
  setting.formatFunction = function;
  m_settings.push_back(setting);
}

void CGUIDialogSettings::AddSeparator(unsigned int id)
{
  SettingInfo setting;
  setting.id = id;
  setting.type = SettingInfo::SEPARATOR;
  setting.data = NULL;
  m_settings.push_back(setting);
}

void CGUIDialogSettings::OnInitWindow()
{
  CreateSettings();
  SetInitialVisibility();
  SetupPage();
  // set the default focus control
  m_lastControlID = CONTROL_START;
  CGUIDialog::OnInitWindow();
}

void CGUIDialogSettings::OnSliderChange(void *data, CGUISliderControl *slider)
{
  if (!data || !slider)
    return;

  SettingInfo *setting = (SettingInfo *)data;
  *(float *)setting->data = slider->GetFloatValue();
  OnSettingChanged(*setting);
  if (setting->formatFunction)
    slider->SetTextValue(setting->formatFunction(slider->GetFloatValue(), setting->interval));
}
