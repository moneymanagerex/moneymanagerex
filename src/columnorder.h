/*
Copyright (c) 2024 Jens Benecke

This Program is free software; you can redistribute it and/or modify
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
 */

#ifndef COLUMNORDER_H
#define COLUMNORDER_H

#include "wx/dialog.h"
#include <wx/listbox.h>
#include <wx/button.h>
#include <wx/listbase.h>

class mmColumnsDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmColumnsDialog);
    wxDECLARE_EVENT_TABLE();

public:
    // mmColumnsDialog(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Column Order"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE);
    mmColumnsDialog();
    ~mmColumnsDialog();
    mmColumnsDialog(wxWindow* parent);

    // called from mmcheckingpanel.cpp
    static wxArrayString updateColumnsOrder(wxArrayString defaultColumns);

private:
    // Declare your dialog controls here
    wxListBox* m_listBox;
    wxButton* m_upButton; // Fixed: Correctly declare the button for moving items up
    wxButton* m_downButton; // Fixed: Correctly declare the button for moving items down
    wxButton* m_OkButton; // Added: Correctly declare the OK button
    wxButton* m_CancelButton; // Added: Correctly declare the cancel button

    wxArrayString columnList_;

    void SetColumnsOrder();
    wxArrayString GetColumnsOrder();

    bool Create(wxWindow* parent);
    void CreateControls();
    void OnCancel(wxCommandEvent& event);
    void OnOk(wxCommandEvent& event);
    void OnUp(wxCommandEvent& event);
    void OnDown(wxCommandEvent& event);
    void Move(int direction);
};

#endif // COLUMNORDER_H