/*************************************************************************
 Copyright (C) 2006 Madhan Kanagavel
 copyright (C) 2011, 2012 Nikolay & Stefano Giorgio.
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
    void SaveNewSystemSettings();
    mmGUIApp* m_app;

private:
    /// Dialog Creation - Used by constructor
    bool Create(wxWindow* parent, wxWindowID id,
        const wxString& caption,
        const wxPoint& pos,
        const wxSize& size,
        long style);

    void CreateControls();
    void OnOk(wxCommandEvent& /*event*/);
    void OnApply(wxCommandEvent& /*event*/);

private:
    wxImageList* m_imageList;
    wxListbook* m_notebook;
    std::vector<OptionSettingsBase*> m_panel_list;
};

#endif
