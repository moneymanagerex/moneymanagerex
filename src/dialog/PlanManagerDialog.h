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
#include "base/_defs.h"
#include "util/mmTextCtrl.h"
#include "util/mmDatePicker.h"
#include "model/PlanGroupModel.h"
#include "model/PlanItemModel.h"

// Edits a single plan item: a future expense or income. An item may be a flat
// amount, or a quantity priced from an assumption (which is how share vesting
// is expressed).
class PlanItemEntryDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(PlanItemEntryDialog);
    wxDECLARE_EVENT_TABLE();

public:
    PlanItemEntryDialog();
    PlanItemEntryDialog(wxWindow* parent, PlanItemData* item, int64 default_group_id = -1);

    bool Create(wxWindow* parent, wxWindowID id,
        const wxString& caption,
        const wxPoint& pos,
        const wxSize& size,
        long style);

private:
    void CreateControls();
    void fillControls();
    void OnOk(wxCommandEvent& event);
    void OnUnitsChanged(wxCommandEvent& event);
    void OnAmountModeChanged(wxCommandEvent& event);
    void OnSymbolChanged(wxCommandEvent& event);
    void OnCategory(wxCommandEvent& event);
    void updateComputed();
    void updateAmountMode();
    void rebuildAssumptionChoices();

    PlanItemData* m_item_n = nullptr;
    int64 m_default_group_id = -1;

    std::vector<int64> m_group_id_a;
    std::vector<int64> m_price_group_id_a;
    std::vector<int64> m_tax_group_id_a;
    int64 m_category_id = -1;

    wxTextCtrl*   m_name   = nullptr;
    wxChoice*     m_group  = nullptr;
    wxChoice*     m_kind   = nullptr;
    wxChoice*     m_status = nullptr;
    mmDatePicker* m_date   = nullptr;
    wxCheckBox*   m_has_date = nullptr;
    wxButton*     m_category = nullptr;
    wxRadioBox*   m_amount_mode = nullptr;
    mmTextCtrl*   m_amount = nullptr;
    wxStaticText* m_amount_label = nullptr;
    mmTextCtrl*   m_units  = nullptr;
    wxTextCtrl*   m_symbol = nullptr;
    wxChoice*     m_price_assumption = nullptr;
    wxChoice*     m_tax_assumption   = nullptr;
    mmTextCtrl*   m_tax_rate = nullptr;
    wxTextCtrl*   m_notes  = nullptr;
    wxStaticText* m_computed = nullptr;
    wxWindow*     m_unit_block = nullptr;

    enum {
        ID_AMOUNT_MODE = wxID_HIGHEST + 540,
        ID_SYMBOL,
        ID_CATEGORY,
        ID_HAS_DATE
    };
};

// Manages the long-term plan: groups (a trip, a collection) and their items.
class PlanManagerDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(PlanManagerDialog);
    wxDECLARE_EVENT_TABLE();

public:
    PlanManagerDialog();
    PlanManagerDialog(wxWindow* parent);

    bool Create(wxWindow* parent, wxWindowID id,
        const wxString& caption,
        const wxPoint& pos,
        const wxSize& size,
        long style);

private:
    void CreateControls();
    void fillControls();
    void updateTotals();

    void OnAddGroup(wxCommandEvent& event);
    void OnAddItem(wxCommandEvent& event);
    void OnEdit(wxCommandEvent& event);
    void OnDelete(wxCommandEvent& event);
    void OnDuplicate(wxCommandEvent& event);
    void OnAssumptions(wxCommandEvent& event);
    void OnAccounts(wxCommandEvent& event);
    void OnDoubleClicked(wxTreeEvent& event);

    // Selected node, or -1. is_group tells the caller which table it is in.
    int64 selectedId(bool& is_group) const;

    // Groups nest arbitrarily, so the tree is built by walking down from each
    // root rather than one level at a time.
    void addGroupNode(const wxTreeItemId& parent, int64 group_id, int depth);
    void addItemNodes(const wxTreeItemId& parent, int64 group_id);

    wxTreeCtrl*   m_tree = nullptr;
    wxStaticText* m_totals = nullptr;
    wxFlexGridSizer* m_totals_grid = nullptr;
    std::vector<wxStaticText*> m_total_value_a;

    enum {
        ID_ADD_GROUP = wxID_HIGHEST + 520,
        ID_ADD_ITEM,
        ID_ASSUMPTIONS,
        ID_DUPLICATE,
        ID_ACCOUNTS
    };
};
