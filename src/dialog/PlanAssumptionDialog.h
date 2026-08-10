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

#include <vector>
#include <wx/treectrl.h>
#include "base/_defs.h"
#include "util/mmTextCtrl.h"
#include "model/PlanAssumptionModel.h"
#include "model/PlanAssumptionGroupModel.h"

// Edits an assumption group: the set of alternative answers to one question,
// such as the price MSFT might reach. The group is typed and scoped, which is
// what stops a tax rate being offered where a share price is meant.
class PlanAssumptionGroupEntryDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(PlanAssumptionGroupEntryDialog);
    wxDECLARE_EVENT_TABLE();

public:
    PlanAssumptionGroupEntryDialog();
    PlanAssumptionGroupEntryDialog(wxWindow* parent, PlanAssumptionGroupData* group);

    bool Create(wxWindow* parent, wxWindowID id,
        const wxString& caption,
        const wxPoint& pos,
        const wxSize& size,
        long style);

private:
    void CreateControls();
    void fillControls();
    void OnOk(wxCommandEvent& event);
    void OnKindChanged(wxCommandEvent& event);
    void updateHint();

    PlanAssumptionGroupData* m_group_n = nullptr;

    wxTextCtrl*   m_name  = nullptr;
    wxChoice*     m_kind  = nullptr;
    wxTextCtrl*   m_scope = nullptr;
    wxTextCtrl*   m_unit  = nullptr;
    wxTextCtrl*   m_notes = nullptr;
    wxStaticText* m_hint  = nullptr;
};

// Edits a single assumption: one candidate value, either standing alone or as a
// member of a group.
class PlanAssumptionEntryDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(PlanAssumptionEntryDialog);
    wxDECLARE_EVENT_TABLE();

public:
    PlanAssumptionEntryDialog();
    PlanAssumptionEntryDialog(wxWindow* parent, PlanAssumptionData* assumption,
        int64 default_group_id = -1);

    bool Create(wxWindow* parent, wxWindowID id,
        const wxString& caption,
        const wxPoint& pos,
        const wxSize& size,
        long style);

private:
    void CreateControls();
    void fillControls();
    void OnOk(wxCommandEvent& event);
    void OnKindChanged(wxCommandEvent& event);
    void OnGroupChanged(wxCommandEvent& event);
    void updateHint();
    void applyGroupConstraints();

    PlanAssumptionData* m_assumption_n = nullptr;
    int64 m_default_group_id = -1;
    std::vector<int64> m_group_id_a;

    wxTextCtrl* m_name  = nullptr;
    wxChoice*   m_group = nullptr;
    wxChoice*   m_kind  = nullptr;
    mmTextCtrl* m_value = nullptr;
    wxTextCtrl* m_scope = nullptr;
    wxTextCtrl* m_unit  = nullptr;
    wxTextCtrl* m_notes = nullptr;
    wxStaticText* m_hint = nullptr;

    enum {
        ID_GROUP = wxID_HIGHEST + 560
    };
};

// Node payload: a tree row is either a group or one assumption.
class AssumptionTreeItem : public wxTreeItemData
{
public:
    AssumptionTreeItem(int64 id, bool is_group) :
        m_id(id), m_is_group(is_group) {}
    int64 id() const { return m_id; }
    bool is_group() const { return m_is_group; }

private:
    int64 m_id;
    bool m_is_group;
};

// The single place where assumptions live: groups with their candidate values,
// which member of each group is active, and any standalone assumptions.
class PlanAssumptionDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(PlanAssumptionDialog);
    wxDECLARE_EVENT_TABLE();

public:
    PlanAssumptionDialog();
    PlanAssumptionDialog(wxWindow* parent);

    bool Create(wxWindow* parent, wxWindowID id,
        const wxString& caption,
        const wxPoint& pos,
        const wxSize& size,
        long style);

private:
    void CreateControls();
    void fillControls();

    void OnAddGroup(wxCommandEvent& event);
    void OnAdd(wxCommandEvent& event);
    void OnEdit(wxCommandEvent& event);
    void OnDelete(wxCommandEvent& event);
    void OnDuplicate(wxCommandEvent& event);
    void OnSetActive(wxCommandEvent& event);
    void OnDoubleClicked(wxTreeEvent& event);

    int64 selectedId(bool& is_group) const;
    const wxString formatValue(const PlanAssumptionData& a, const wxString& unit) const;

    wxTreeCtrl*   m_tree = nullptr;
    wxStaticText* m_hint = nullptr;

    enum {
        ID_ADD_GROUP = wxID_HIGHEST + 570,
        ID_SET_ACTIVE,
        ID_DUPLICATE
    };
};
