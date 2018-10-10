/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2017 James Higley

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

#ifndef MM_EX_BUDGETYEARDIALOG_H_
#define MM_EX_BUDGETYEARDIALOG_H_

#include "defs.h"

class mmBudgetYearDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmBudgetYearDialog);
    wxDECLARE_EVENT_TABLE();

public:
    mmBudgetYearDialog();
    mmBudgetYearDialog(wxWindow* parent);

private:
    bool Create(wxWindow* parent, wxWindowID id,
        const wxString& caption,
        const wxPoint& pos,
        const wxSize& size,
        long style);

    void CreateControls();

    // utility functions
    void OnOk(wxCommandEvent& event);
    void OnAdd(wxCommandEvent& event);
    void OnAddMonth(wxCommandEvent& WXUNUSED(event));
    void OnDelete(wxCommandEvent& event);
    void OnBSelect(wxCommandEvent& event);
    void OnEdit(wxCommandEvent& event);
    void fillControls();
    void OnDoubleClicked(wxCommandEvent& event);

    wxListBox* m_listBox;
    enum { ID_ADD_MONTH = wxID_HIGHEST + 300 };
};

#endif

