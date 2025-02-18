/*************************************************************************
 Copyright (C) 2006 Madhan Kanagavel
 copyright (C) 2011, 2012 Nikolay Akimov
 copyright (C) 2011, 2012 Stefano Giorgio.
 copyright (C) 2014 Guan Lisheng (guanlisheng@gmail.com)

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *************************************************************************/

#ifndef MM_EX_OPTIONSDIALOG_H_
#define MM_EX_OPTIONSDIALOG_H_

#include "defs.h"
#include <vector>

class mmGUIApp;
class OptionSettingsBase;

class mmOptionsDialog: public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmOptionsDialog);
    wxDECLARE_EVENT_TABLE();

public:
    mmOptionsDialog( );
    mmOptionsDialog(wxWindow* parent, mmGUIApp* app);
    ~mmOptionsDialog( );

public:
    /// Saves the updated System Options to the appropriate databases.
    bool SaveNewSystemSettings();
    mmGUIApp* m_app = nullptr;

private:
    /// Dialog Creation - Used by constructor
    bool Create(wxWindow* parent, wxWindowID id = wxID_ANY,
        const wxString& caption = _t("Settings"),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX);

    void CreateControls();
    void OnOk(wxCommandEvent& /*event*/);
    void OnApply(wxCommandEvent& /*event*/);
    void OnPageChange(wxBookCtrlEvent& event);

private:
    wxListbook* m_listbook = nullptr;
    std::vector<OptionSettingsBase*> m_panel_list;
};

#endif
