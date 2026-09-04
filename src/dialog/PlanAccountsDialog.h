/*******************************************************
 Copyright (C) 2026

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

#include <map>
#include <vector>
#include <wx/treelist.h>
#include "base/_defs.h"

// Chooses which accounts count towards the long-term plan.
//
// A flat list is hard to read once there are sub-accounts: the share accounts
// of an investment portfolio look like unrelated top-level entries. This
// mirrors the navigation rail instead - grouped by account type, with share
// accounts nested under the portfolio they belong to - so the structure is the
// one the user already knows.
class PlanAccountsDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(PlanAccountsDialog);
    wxDECLARE_EVENT_TABLE();

public:
    PlanAccountsDialog();
    PlanAccountsDialog(wxWindow* parent);

    bool Create(wxWindow* parent, wxWindowID id,
        const wxString& caption,
        const wxPoint& pos,
        const wxSize& size,
        long style);

private:
    void CreateControls();
    void fillControls();
    void OnOk(wxCommandEvent& event);
    void OnAll(wxCommandEvent& event);
    void OnNone(wxCommandEvent& event);
    void OnItemChecked(wxTreeListEvent& event);

    // Walk the tree collecting the accounts that are checked.
    void collectChecked(const wxTreeListItem& parent, std::vector<int64>& out) const;
    void setAllChecked(bool checked);
    void updateSummary();

    wxTreeListCtrl* m_tree = nullptr;
    wxStaticText*   m_summary = nullptr;

    // Every account the plan could count, so unchecked ones can be stored as
    // exclusions without having to re-derive the full set later.
    std::vector<int64> m_all_account_id_a;

    enum {
        ID_ALL = wxID_HIGHEST + 620,
        ID_NONE
    };
};
