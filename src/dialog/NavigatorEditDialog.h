/*******************************************************
 Copyright (C) 2025, 2026 Klaus Wich

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
 ********************************************************/

#pragma once

#include "base/_defs.h"
#include "util/mmNavigatorList.h"

class NavigatorEditDialog: public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(NavigatorEditDialog);

public:
    NavigatorEditDialog();
    NavigatorEditDialog(wxWindow* parent, const mmNavigatorItem* info);
    void updateInfo(mmNavigatorItem* info);
    void OnNewText(wxCommandEvent& event);

private:
    wxTextCtrl* m_nameTextCtrl;
    wxStaticText* m_choiceLabel;
    wxTextCtrl* m_choiceTextCtrl;
    wxCheckBox* m_activeCheckBox;
    wxStaticText* m_aktivLabel;
    wxBitmapComboBox* m_cbIcon;
    wxIcon m_selectedIcon;
    wxButton* m_saveButton;

    void CreateControls();
};
