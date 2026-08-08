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

#include "base/_defs.h"
#include "base/_constants.h"
#include <wx/textdlg.h>
#include "util/mmPath.h"
#include "util/_util.h"
#include "model/PlanAssumptionModel.h"
#include "model/PlanEngine.h"
#include "PlanManagerDialog.h"
#include "PlanAssumptionDialog.h"

namespace
{
    // Tree item payload: which table a node belongs to, and its id.
    class PlanTreeItem : public wxTreeItemData
    {
    public:
        PlanTreeItem(int64 id, bool is_group) :
            m_id(id), m_is_group(is_group) {}
        int64 id() const { return m_id; }
        bool is_group() const { return m_is_group; }
    private:
        int64 m_id;
        bool  m_is_group;
    };
}

// =====================================================================
// Single plan item
// =====================================================================

wxIMPLEMENT_DYNAMIC_CLASS(PlanItemEntryDialog, wxDialog);

wxBEGIN_EVENT_TABLE(PlanItemEntryDialog, wxDialog)
    EVT_BUTTON(wxID_OK, PlanItemEntryDialog::OnOk)
    EVT_TEXT(wxID_ANY, PlanItemEntryDialog::OnUnitsChanged)
wxEND_EVENT_TABLE()

PlanItemEntryDialog::PlanItemEntryDialog()
{
}

PlanItemEntryDialog::PlanItemEntryDialog(
    wxWindow* parent,
    PlanItemData* item,
    int64 default_group_id
) :
    m_item_n(item),
    m_default_group_id(default_group_id)
{
    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY, _t("Plan Item"), wxDefaultPosition, wxSize(470, 560), style);
    mmThemeAutoColour(this);
}

bool PlanItemEntryDialog::Create(wxWindow* parent, wxWindowID id,
    const wxString& caption, const wxPoint& pos,
    const wxSize& size, long style)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style);

    CreateControls();
    fillControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    this->SetInitialSize();
    SetIcon(mmPath::getProgramIcon());
    Centre();
    return true;
}

void PlanItemEntryDialog::CreateControls()
{
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(mainSizer);

    wxFlexGridSizer* grid = new wxFlexGridSizer(0, 2, 5, 10);
    grid->AddGrowableCol(1, 1);
    mainSizer->Add(grid, 0, wxGROW | wxALL, 10);

    grid->Add(new wxStaticText(this, wxID_STATIC, _t("Name")), 0, wxALIGN_CENTER_VERTICAL);
    m_name = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(260, -1));
    grid->Add(m_name, 1, wxGROW);

    grid->Add(new wxStaticText(this, wxID_STATIC, _t("Group")), 0, wxALIGN_CENTER_VERTICAL);
    m_group = new wxChoice(this, wxID_ANY);
    m_group->Append(_t("(none)"));
    m_group_id_a.push_back(-1);
    for (const auto& g : PlanGroupModel::instance().find_data_a()) {
        if (!g.m_active) continue;
        m_group->Append(PlanGroupModel::instance().get_id_path_name(g.m_id));
        m_group_id_a.push_back(g.m_id);
    }
    grid->Add(m_group, 1, wxGROW);

    grid->Add(new wxStaticText(this, wxID_STATIC, _t("Type")), 0, wxALIGN_CENTER_VERTICAL);
    m_kind = new wxChoice(this, wxID_ANY);
    for (int i = 0; i < PlanItemKind::size; ++i)
        m_kind->Append(wxGetTranslation(PlanItemKind(i).name()));
    grid->Add(m_kind, 1, wxGROW);

    grid->Add(new wxStaticText(this, wxID_STATIC, _t("Status")), 0, wxALIGN_CENTER_VERTICAL);
    m_status = new wxChoice(this, wxID_ANY);
    for (int i = 0; i < PlanStatus::size; ++i)
        m_status->Append(wxGetTranslation(PlanStatus(i).name()));
    mmToolTip(m_status, _t(
        "Only Planned and Committed reduce free assets. A Wishlist item is "
        "tracked without being treated as an obligation."));
    grid->Add(m_status, 1, wxGROW);

    grid->Add(new wxStaticText(this, wxID_STATIC, _t("Target date")), 0, wxALIGN_CENTER_VERTICAL);
    m_date = new mmDatePicker(this, wxID_ANY);
    grid->Add(m_date->mmGetLayout(false), 1, wxGROW);

    grid->Add(new wxStaticText(this, wxID_STATIC, _t("Amount")), 0, wxALIGN_CENTER_VERTICAL);
    m_amount = new mmTextCtrl(this, wxID_ANY, "",
        wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER);
    mmToolTip(m_amount, _t("Leave empty when the value is derived from units and a price."));
    grid->Add(m_amount, 1, wxGROW);

    wxStaticLine* sep = new wxStaticLine(this, wxID_STATIC);
    mainSizer->Add(sep, 0, wxGROW | wxLEFT | wxRIGHT, 10);
    wxStaticText* unitHdr = new wxStaticText(this, wxID_STATIC,
        _t("Priced from units (for share or RSU vesting)"));
    mainSizer->Add(unitHdr, 0, wxLEFT | wxTOP, 10);

    wxFlexGridSizer* grid2 = new wxFlexGridSizer(0, 2, 5, 10);
    grid2->AddGrowableCol(1, 1);
    mainSizer->Add(grid2, 0, wxGROW | wxALL, 10);

    grid2->Add(new wxStaticText(this, wxID_STATIC, _t("Units")), 0, wxALIGN_CENTER_VERTICAL);
    m_units = new mmTextCtrl(this, wxID_ANY, "",
        wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER);
    grid2->Add(m_units, 1, wxGROW);

    grid2->Add(new wxStaticText(this, wxID_STATIC, _t("Symbol")), 0, wxALIGN_CENTER_VERTICAL);
    m_symbol = new wxTextCtrl(this, wxID_ANY);
    grid2->Add(m_symbol, 1, wxGROW);

    // Build the assumption choices once and reuse for both price and tax.
    m_assumption_id_a.push_back(-1);
    wxArrayString assumption_names;
    assumption_names.Add(_t("(none)"));
    for (const auto& a : PlanAssumptionModel::instance().find_active_a()) {
        assumption_names.Add(a.m_name);
        m_assumption_id_a.push_back(a.m_id);
    }

    grid2->Add(new wxStaticText(this, wxID_STATIC, _t("Price assumption")), 0, wxALIGN_CENTER_VERTICAL);
    m_price_assumption = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, assumption_names);
    mmToolTip(m_price_assumption, _t("A shared price used by every item that links to it."));
    grid2->Add(m_price_assumption, 1, wxGROW);

    grid2->Add(new wxStaticText(this, wxID_STATIC, _t("Tax assumption")), 0, wxALIGN_CENTER_VERTICAL);
    m_tax_assumption = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, assumption_names);
    grid2->Add(m_tax_assumption, 1, wxGROW);

    grid2->Add(new wxStaticText(this, wxID_STATIC, _t("Tax rate")), 0, wxALIGN_CENTER_VERTICAL);
    m_tax_rate = new mmTextCtrl(this, wxID_ANY, "",
        wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER);
    mmToolTip(m_tax_rate, _t("Used only when no tax assumption is linked. 32 and 0.32 both mean 32%."));
    grid2->Add(m_tax_rate, 1, wxGROW);

    m_computed = new wxStaticText(this, wxID_STATIC, "");
    mainSizer->Add(m_computed, 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);

    wxFlexGridSizer* grid3 = new wxFlexGridSizer(0, 2, 5, 10);
    grid3->AddGrowableCol(1, 1);
    mainSizer->Add(grid3, 0, wxGROW | wxLEFT | wxRIGHT | wxBOTTOM, 10);
    grid3->Add(new wxStaticText(this, wxID_STATIC, _t("Notes")), 0, wxALIGN_TOP);
    m_notes = new wxTextCtrl(this, wxID_ANY, "",
        wxDefaultPosition, wxSize(260, 50), wxTE_MULTILINE);
    grid3->Add(m_notes, 1, wxGROW);

    wxPanel* btnPanel = new wxPanel(this, wxID_ANY);
    mainSizer->Add(btnPanel, 0, wxALIGN_RIGHT, 5);
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    btnPanel->SetSizer(btnSizer);
    wxButton* btnOK = new wxButton(btnPanel, wxID_OK, _t("&OK "));
    btnSizer->Add(btnOK, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT | wxBOTTOM, 5);
    wxButton* btnCancel = new wxButton(btnPanel, wxID_CANCEL, wxGetTranslation(g_CancelLabel));
    btnSizer->Add(btnCancel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT | wxBOTTOM, 5);
}

void PlanItemEntryDialog::fillControls()
{
    auto index_of = [](const std::vector<int64>& a, int64 id) -> int {
        for (std::size_t i = 0; i < a.size(); ++i)
            if (a[i] == id) return static_cast<int>(i);
        return 0;
    };

    if (!m_item_n) {
        m_kind->SetSelection(PlanItemKind::e_expense);
        m_status->SetSelection(PlanStatus::e_planned);
        m_group->SetSelection(index_of(m_group_id_a, m_default_group_id));
        m_price_assumption->SetSelection(0);
        m_tax_assumption->SetSelection(0);
        updateComputed();
        return;
    }

    m_name->SetValue(m_item_n->m_name);
    m_group->SetSelection(index_of(m_group_id_a, m_item_n->m_group_id));
    m_kind->SetSelection(m_item_n->m_kind.id());
    m_status->SetSelection(m_item_n->m_status.id());
    if (!m_item_n->m_target_date.IsEmpty()) {
        // mmDate::dateTime() is non-const, so take a local copy.
        mmDate target(m_item_n->m_target_date);
        m_date->setValue(target.dateTime());
    }
    if (m_item_n->m_amount != 0.0)
        m_amount->SetValue(m_item_n->m_amount, 2);
    if (m_item_n->m_units > 0.0)
        m_units->SetValue(m_item_n->m_units, 4);
    m_symbol->SetValue(m_item_n->m_stock_symbol);
    if (m_item_n->m_tax_rate > 0.0)
        m_tax_rate->SetValue(m_item_n->m_tax_rate, 4);
    m_price_assumption->SetSelection(index_of(m_assumption_id_a, m_item_n->m_price_assumption_id));
    m_tax_assumption->SetSelection(index_of(m_assumption_id_a, m_item_n->m_tax_assumption_id));
    m_notes->SetValue(m_item_n->m_notes);

    updateComputed();
}

void PlanItemEntryDialog::OnUnitsChanged(wxCommandEvent& event)
{
    updateComputed();
    event.Skip();
}

void PlanItemEntryDialog::updateComputed()
{
    if (!m_computed || !m_units)
        return;

    double units = 0.0;
    if (!m_units->GetDouble(units) || units <= 0.0) {
        m_computed->SetLabel(_t("A flat amount is used; units are ignored."));
        return;
    }

    // Mirror the resolution the model performs, so the dialog shows the number
    // that will actually be stored rather than a guess of it.
    PlanItemData probe;
    probe.m_units = units;
    probe.m_stock_symbol = m_symbol->GetValue().Trim().Trim(false);

    const int pi = m_price_assumption->GetSelection();
    if (pi > 0 && static_cast<std::size_t>(pi) < m_assumption_id_a.size())
        probe.m_price_assumption_id = m_assumption_id_a[pi];

    const int ti = m_tax_assumption->GetSelection();
    if (ti > 0 && static_cast<std::size_t>(ti) < m_assumption_id_a.size())
        probe.m_tax_assumption_id = m_assumption_id_a[ti];

    double rate = 0.0;
    if (m_tax_rate->GetDouble(rate))
        probe.m_tax_rate = rate;

    PlanItemModel& pim = PlanItemModel::instance();
    const double price = pim.resolve_unit_price(probe);
    const double gross = units * price;
    const double net   = PlanItemData::apply_tax(gross, pim.resolve_tax_rate(probe));

    m_computed->SetLabel(wxString::Format(
        _t("%.4f units at %.4f  =  %.2f gross,  %.2f after tax"),
        units, price, gross, net));
    Layout();
}

void PlanItemEntryDialog::OnOk(wxCommandEvent& /*event*/)
{
    const wxString name = m_name->GetValue().Trim().Trim(false);
    if (name.IsEmpty()) {
        wxMessageBox(_t("Enter a name for the plan item."),
            _t("Plan Item"), wxOK | wxICON_WARNING, this);
        return;
    }

    double amount = 0.0;
    m_amount->GetDouble(amount);
    double units = 0.0;
    m_units->GetDouble(units);

    if (amount <= 0.0 && units <= 0.0) {
        wxMessageBox(
            _t("Enter either an amount, or a number of units with a price."),
            _t("Plan Item"), wxOK | wxICON_WARNING, this);
        return;
    }

    PlanItemData data;
    if (m_item_n)
        data = *m_item_n;

    data.m_name        = name;
    data.m_kind        = PlanItemKind(m_kind->GetSelection());
    data.m_status      = PlanStatus(m_status->GetSelection());
    data.m_target_date = mmDate(m_date->GetValue()).isoDate();
    data.m_amount      = amount;
    data.m_units       = units;
    data.m_stock_symbol = m_symbol->GetValue().Trim().Trim(false);
    data.m_notes       = m_notes->GetValue();
    data.m_active      = true;

    const int gi = m_group->GetSelection();
    data.m_group_id = (gi >= 0 && static_cast<std::size_t>(gi) < m_group_id_a.size())
        ? m_group_id_a[gi] : -1;

    const int pi = m_price_assumption->GetSelection();
    data.m_price_assumption_id =
        (pi > 0 && static_cast<std::size_t>(pi) < m_assumption_id_a.size())
        ? m_assumption_id_a[pi] : -1;

    const int ti = m_tax_assumption->GetSelection();
    data.m_tax_assumption_id =
        (ti > 0 && static_cast<std::size_t>(ti) < m_assumption_id_a.size())
        ? m_assumption_id_a[ti] : -1;

    double rate = 0.0;
    data.m_tax_rate = m_tax_rate->GetDouble(rate) ? rate : 0.0;

    PlanItemModel& pim = PlanItemModel::instance();
    if (m_item_n) pim.save_data_n(data);
    else          pim.add_data_n(data);

    EndModal(wxID_OK);
}

// =====================================================================
// Plan manager
// =====================================================================

wxIMPLEMENT_DYNAMIC_CLASS(PlanManagerDialog, wxDialog);

wxBEGIN_EVENT_TABLE(PlanManagerDialog, wxDialog)
    EVT_BUTTON(ID_ADD_GROUP, PlanManagerDialog::OnAddGroup)
    EVT_BUTTON(ID_ADD_ITEM, PlanManagerDialog::OnAddItem)
    EVT_BUTTON(wxID_EDIT, PlanManagerDialog::OnEdit)
    EVT_BUTTON(wxID_DELETE, PlanManagerDialog::OnDelete)
    EVT_BUTTON(ID_ASSUMPTIONS, PlanManagerDialog::OnAssumptions)
    EVT_TREE_ITEM_ACTIVATED(wxID_ANY, PlanManagerDialog::OnDoubleClicked)
wxEND_EVENT_TABLE()

PlanManagerDialog::PlanManagerDialog()
{
}

PlanManagerDialog::PlanManagerDialog(wxWindow* parent)
{
    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX | wxRESIZE_BORDER;
    Create(parent, wxID_ANY, _t("Long-Term Plan"), wxDefaultPosition, wxSize(640, 480), style);
    mmThemeAutoColour(this);
}

bool PlanManagerDialog::Create(wxWindow* parent, wxWindowID id,
    const wxString& caption, const wxPoint& pos,
    const wxSize& size, long style)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style);

    CreateControls();
    fillControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    this->SetInitialSize();
    SetIcon(mmPath::getProgramIcon());
    Centre();
    return true;
}

void PlanManagerDialog::CreateControls()
{
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(mainSizer);

    m_tree = new wxTreeCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(600, 300),
        wxTR_HAS_BUTTONS | wxTR_LINES_AT_ROOT | wxTR_SINGLE);
    mainSizer->Add(m_tree, 1, wxGROW | wxALL, 5);

    m_totals = new wxStaticText(this, wxID_STATIC, "");
    mainSizer->Add(m_totals, 0, wxLEFT | wxRIGHT | wxBOTTOM, 8);

    wxBoxSizer* buttonRow = new wxBoxSizer(wxHORIZONTAL);
    mainSizer->Add(buttonRow, 0, wxGROW | wxALL, 5);

    wxButton* btnGroup = new wxButton(this, ID_ADD_GROUP, _t("Add &Group"));
    buttonRow->Add(btnGroup, 0, wxALIGN_CENTER_VERTICAL);
    mmToolTip(btnGroup, _t("Add a project such as a trip or a collection"));

    wxButton* btnItem = new wxButton(this, ID_ADD_ITEM, _t("Add &Item"));
    buttonRow->Add(btnItem, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);

    wxButton* btnEdit = new wxButton(this, wxID_EDIT, _t("&Edit"));
    buttonRow->Add(btnEdit, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);

    wxButton* btnDelete = new wxButton(this, wxID_DELETE, _t("&Delete"));
    buttonRow->Add(btnDelete, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);

    wxButton* btnAssume = new wxButton(this, ID_ASSUMPTIONS, _t("&Assumptions..."));
    buttonRow->Add(btnAssume, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 15);
    mmToolTip(btnAssume, _t("Edit the estimates the plan is calculated from"));

    wxStaticLine* line = new wxStaticLine(this, wxID_STATIC);
    mainSizer->Add(line, 0, wxGROW | wxALL, 5);

    wxPanel* btnPanel = new wxPanel(this, wxID_ANY);
    mainSizer->Add(btnPanel, 0, wxALIGN_RIGHT, 5);
    wxBoxSizer* okSizer = new wxBoxSizer(wxHORIZONTAL);
    btnPanel->SetSizer(okSizer);
    wxButton* btnOK = new wxButton(btnPanel, wxID_OK, _t("&OK "));
    okSizer->Add(btnOK, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT | wxBOTTOM, 5);
}

void PlanManagerDialog::fillControls()
{
    m_tree->DeleteAllItems();

    PlanGroupModel& pgm = PlanGroupModel::instance();
    PlanItemModel& pim = PlanItemModel::instance();

    wxTreeItemId root = m_tree->AddRoot(_t("Plan"));

    for (const auto& group : pgm.find_root_a()) {
        wxTreeItemId gnode = m_tree->AppendItem(root,
            group.m_target_date.IsEmpty()
                ? group.m_name
                : wxString::Format("%s  (%s)", group.m_name, group.m_target_date),
            -1, -1, new PlanTreeItem(group.m_id, true));

        for (const auto& item : pim.find_group_a(group.m_id)) {
            const double amt = pim.net_amount_base(item);
            const wxString mark =
                (item.is_unit_based() || item.is_assumption_based()) ? " ~" : "";
            m_tree->AppendItem(gnode,
                wxString::Format("%s%s  %.2f  [%s]",
                    item.m_name, mark, amt,
                    wxGetTranslation(item.m_status.name())),
                -1, -1, new PlanTreeItem(item.m_id, false));
        }
    }

    // Items with no group still need to be reachable.
    wxTreeItemId ungrouped;
    for (const auto& item : pim.find_group_a(-1)) {
        if (!ungrouped.IsOk())
            ungrouped = m_tree->AppendItem(root, _t("(ungrouped)"), -1, -1, nullptr);
        const double amt = pim.net_amount_base(item);
        const wxString mark =
            (item.is_unit_based() || item.is_assumption_based()) ? " ~" : "";
        m_tree->AppendItem(ungrouped,
            wxString::Format("%s%s  %.2f  [%s]",
                item.m_name, mark, amt,
                wxGetTranslation(item.m_status.name())),
            -1, -1, new PlanTreeItem(item.m_id, false));
    }

    m_tree->ExpandAll();
    updateTotals();
}

void PlanManagerDialog::updateTotals()
{
    const PlanSummary s = PlanEngine::build_summary(mmDate::today());
    m_totals->SetLabel(wxString::Format(
        _t("Expected income %.2f    Committed %.2f    Wishlist %.2f    Free assets %.2f"),
        s.expected_income, s.committed_expense, s.wishlist_expense, s.free_assets()));
    Layout();
}

int64 PlanManagerDialog::selectedId(bool& is_group) const
{
    const wxTreeItemId sel = m_tree->GetSelection();
    if (!sel.IsOk())
        return -1;
    PlanTreeItem* data = static_cast<PlanTreeItem*>(m_tree->GetItemData(sel));
    if (!data)
        return -1;
    is_group = data->is_group();
    return data->id();
}

void PlanManagerDialog::OnAddGroup(wxCommandEvent& /*event*/)
{
    const wxString name = wxGetTextFromUser(
        _t("Group name (e.g. a trip, or a collection)"),
        _t("Plan Group"), "", this);
    if (name.IsEmpty())
        return;

    PlanGroupData g;
    g.m_name   = name;
    g.m_status = PlanStatus(PlanStatus::e_planned);
    g.m_active = true;

    // Nest under the selected group, so sub-projects are possible.
    bool is_group = false;
    const int64 sel = selectedId(is_group);
    if (sel > 0 && is_group)
        g.m_parent_id = sel;

    PlanGroupModel::instance().add_data_n(g);
    fillControls();
}

void PlanManagerDialog::OnAddItem(wxCommandEvent& /*event*/)
{
    bool is_group = false;
    const int64 sel = selectedId(is_group);
    const int64 group_id = (sel > 0 && is_group) ? sel : -1;

    PlanItemEntryDialog dlg(this, nullptr, group_id);
    if (dlg.ShowModal() == wxID_OK)
        fillControls();
}

void PlanManagerDialog::OnEdit(wxCommandEvent& /*event*/)
{
    bool is_group = false;
    const int64 id = selectedId(is_group);
    if (id <= 0)
        return;

    if (is_group) {
        const PlanGroupData* g_n = PlanGroupModel::instance().get_idN_data_n(id);
        if (!g_n) return;
        const wxString name = wxGetTextFromUser(
            _t("Group name"), _t("Plan Group"), g_n->m_name, this);
        if (name.IsEmpty()) return;
        PlanGroupData g = *g_n;
        g.m_name = name;
        PlanGroupModel::instance().save_data_n(g);
    }
    else {
        const PlanItemData* i_n = PlanItemModel::instance().get_idN_data_n(id);
        if (!i_n) return;
        PlanItemData item = *i_n;
        PlanItemEntryDialog dlg(this, &item);
        if (dlg.ShowModal() != wxID_OK) return;
    }

    fillControls();
}

void PlanManagerDialog::OnDelete(wxCommandEvent& /*event*/)
{
    bool is_group = false;
    const int64 id = selectedId(is_group);
    if (id <= 0)
        return;

    const wxString msg = is_group
        ? _t("Delete this group?\n\nIts sub-groups and all their items are deleted too.")
        : _t("Delete this plan item?");

    if (wxMessageBox(msg, _t("Long-Term Plan"),
            wxYES_NO | wxICON_QUESTION, this) != wxYES)
        return;

    if (is_group) PlanGroupModel::instance().purge_id(id);
    else          PlanItemModel::instance().purge_id(id);

    fillControls();
}

void PlanManagerDialog::OnAssumptions(wxCommandEvent& /*event*/)
{
    PlanAssumptionDialog dlg(this);
    dlg.ShowModal();
    // Amounts derived from assumptions may have moved.
    fillControls();
}

void PlanManagerDialog::OnDoubleClicked(wxTreeEvent& /*event*/)
{
    wxCommandEvent evt;
    OnEdit(evt);
}
