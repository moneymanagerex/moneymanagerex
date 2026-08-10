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
#include <wx/radiobox.h>
#include <wx/checkbox.h>
#include <wx/statline.h>
#include "util/mmPath.h"
#include "util/_util.h"
#include "model/PlanAssumptionModel.h"
#include "model/PlanAssumptionGroupModel.h"
#include "model/PlanEngine.h"
#include "model/CategoryModel.h"
#include "model/CurrencyModel.h"
#include "model/AccountModel.h"
#include "manager/CategoryManager.h"
#include <algorithm>
#include <functional>
#include <wx/choicdlg.h>
#include "PlanManagerDialog.h"
#include "PlanAssumptionDialog.h"
#include "PlanAccountsDialog.h"

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
    EVT_BUTTON(ID_CATEGORY, PlanItemEntryDialog::OnCategory)
    EVT_RADIOBOX(ID_AMOUNT_MODE, PlanItemEntryDialog::OnAmountModeChanged)
    EVT_TEXT(ID_SYMBOL, PlanItemEntryDialog::OnSymbolChanged)
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
    {
        // Table order reads as random once there are more than a couple of
        // groups, so sort by the full path: that also keeps a sub-group next to
        // its parent instead of scattering the tree.
        PlanGroupModel& pgm = PlanGroupModel::instance();
        std::vector<std::pair<wxString, int64>> groups;
        for (const auto& g : pgm.find_data_a()) {
            if (!g.m_active) continue;
            groups.emplace_back(pgm.get_id_path_name(g.m_id), g.m_id);
        }
        std::sort(groups.begin(), groups.end(),
            [](const std::pair<wxString, int64>& x, const std::pair<wxString, int64>& y) {
                return x.first.CmpNoCase(y.first) < 0;
            });
        for (const auto& g : groups) {
            m_group->Append(g.first);
            m_group_id_a.push_back(g.second);
        }
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
    wxBoxSizer* dateRow = new wxBoxSizer(wxHORIZONTAL);
    m_has_date = new wxCheckBox(this, ID_HAS_DATE, _t("Has a target date"));
    m_has_date->SetValue(true);
    mmToolTip(m_has_date, _t(
        "Clear this for something that is planned but not yet scheduled. "
        "Undated items still count towards the totals."));
    dateRow->Add(m_has_date, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);
    m_date = new mmDatePicker(this, wxID_ANY);
    dateRow->Add(m_date->mmGetLayout(false), 1, wxGROW);
    grid->Add(dateRow, 1, wxGROW);

    grid->Add(new wxStaticText(this, wxID_STATIC, _t("Category")), 0, wxALIGN_CENTER_VERTICAL);
    m_category = new wxButton(this, ID_CATEGORY, _t("Select Category"));
    mmToolTip(m_category, _t(
        "The transaction category this will land in, so a planned figure can be "
        "compared against what is actually spent. Independent of the plan group."));
    grid->Add(m_category, 1, wxGROW);

    grid->Add(new wxStaticText(this, wxID_STATIC, _t("Amount is")), 0, wxALIGN_CENTER_VERTICAL);
    wxArrayString modes;
    modes.Add(_t("A fixed amount"));
    modes.Add(_t("Priced from units"));
    m_amount_mode = new wxRadioBox(this, ID_AMOUNT_MODE, "",
        wxDefaultPosition, wxDefaultSize, modes, 2, wxRA_SPECIFY_COLS);
    mmToolTip(m_amount_mode, _t(
        "A fixed amount is typed directly. Priced from units multiplies a "
        "quantity by a price, which is how share or RSU vesting is expressed."));
    grid->Add(m_amount_mode, 1, wxGROW);

    m_amount_label = new wxStaticText(this, wxID_STATIC, _t("Amount"));
    grid->Add(m_amount_label, 0, wxALIGN_CENTER_VERTICAL);
    m_amount = new mmTextCtrl(this, wxID_ANY, "",
        wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER);
    grid->Add(m_amount, 1, wxGROW);

    // The unit inputs live on their own panel so the whole block can be hidden
    // when the item is just a fixed amount.
    wxPanel* unitPanel = new wxPanel(this, wxID_ANY);
    m_unit_block = unitPanel;
    mainSizer->Add(unitPanel, 0, wxGROW);
    wxBoxSizer* unitSizer = new wxBoxSizer(wxVERTICAL);
    unitPanel->SetSizer(unitSizer);

    wxStaticLine* sep = new wxStaticLine(unitPanel, wxID_STATIC);
    unitSizer->Add(sep, 0, wxGROW | wxLEFT | wxRIGHT, 10);
    wxStaticText* unitHdr = new wxStaticText(unitPanel, wxID_STATIC,
        _t("Priced from units (for share or RSU vesting)"));
    unitSizer->Add(unitHdr, 0, wxLEFT | wxTOP, 10);

    wxFlexGridSizer* grid2 = new wxFlexGridSizer(0, 2, 5, 10);
    grid2->AddGrowableCol(1, 1);
    unitSizer->Add(grid2, 0, wxGROW | wxALL, 10);

    grid2->Add(new wxStaticText(unitPanel, wxID_STATIC, _t("Units")), 0, wxALIGN_CENTER_VERTICAL);
    m_units = new mmTextCtrl(unitPanel, wxID_ANY, "",
        wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER);
    grid2->Add(m_units, 1, wxGROW);

    grid2->Add(new wxStaticText(unitPanel, wxID_STATIC, _t("Symbol")), 0, wxALIGN_CENTER_VERTICAL);
    m_symbol = new wxTextCtrl(unitPanel, ID_SYMBOL);
    mmToolTip(m_symbol, _t(
        "The ticker this row is about. It decides which assumption groups may "
        "be used, so a price for one ticker cannot be applied to another."));
    grid2->Add(m_symbol, 1, wxGROW);

    grid2->Add(new wxStaticText(unitPanel, wxID_STATIC, _t("Price assumption")), 0, wxALIGN_CENTER_VERTICAL);
    m_price_assumption = new wxChoice(unitPanel, wxID_ANY);
    mmToolTip(m_price_assumption, _t(
        "A group of alternative prices for this ticker. The item follows "
        "whichever member the group has active, so switching that one choice "
        "moves every item using the group."));
    grid2->Add(m_price_assumption, 1, wxGROW);

    grid2->Add(new wxStaticText(unitPanel, wxID_STATIC, _t("Tax assumption")), 0, wxALIGN_CENTER_VERTICAL);
    m_tax_assumption = new wxChoice(unitPanel, wxID_ANY);
    mmToolTip(m_tax_assumption, _t(
        "A group of alternative tax rates. Only tax-rate groups are offered here."));
    grid2->Add(m_tax_assumption, 1, wxGROW);

    grid2->Add(new wxStaticText(unitPanel, wxID_STATIC, _t("Tax rate")), 0, wxALIGN_CENTER_VERTICAL);
    m_tax_rate = new mmTextCtrl(unitPanel, wxID_ANY, "",
        wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER);
    mmToolTip(m_tax_rate, _t("Used only when no tax assumption is linked. 32 and 0.32 both mean 32%."));
    grid2->Add(m_tax_rate, 1, wxGROW);

    m_computed = new wxStaticText(unitPanel, wxID_STATIC, "");
    unitSizer->Add(m_computed, 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);

    rebuildAssumptionChoices();

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

void PlanItemEntryDialog::rebuildAssumptionChoices()
{
    if (!m_price_assumption || !m_tax_assumption)
        return;

    PlanAssumptionGroupModel& pagm = PlanAssumptionGroupModel::instance();
    const wxString symbol = m_symbol ? m_symbol->GetValue().Trim().Trim(false) : wxString();

    // Remember what was chosen so a symbol edit does not silently drop a still
    // valid selection.
    const int64 prev_price = (m_price_assumption->GetSelection() > 0 &&
        static_cast<std::size_t>(m_price_assumption->GetSelection()) < m_price_group_id_a.size())
        ? m_price_group_id_a[m_price_assumption->GetSelection()] : -1;
    const int64 prev_tax = (m_tax_assumption->GetSelection() > 0 &&
        static_cast<std::size_t>(m_tax_assumption->GetSelection()) < m_tax_group_id_a.size())
        ? m_tax_group_id_a[m_tax_assumption->GetSelection()] : -1;

    auto fill = [&](wxChoice* choice, std::vector<int64>& ids,
                    PlanAssumptionKind kind, const wxString& scope, int64 keep) {
        choice->Clear();
        ids.clear();
        choice->Append(_t("(none)"));
        ids.push_back(-1);

        int select = 0;
        for (const auto& g : pagm.find_applicable_a(kind, scope)) {
            // Say which value the group is currently on, so the consequence of
            // picking it is visible before it is picked.
            const int64 active = pagm.get_active_assumption_id(g.m_id);
            wxString label = g.m_name;
            if (active > 0) {
                const PlanAssumptionData* a_n =
                    PlanAssumptionModel::instance().get_idN_data_n(active);
                if (a_n)
                    label += wxString::Format("  [%s %s]",
                        wxString::FromDouble(a_n->m_value, 4), g.unit_or_default());
            }
            choice->Append(label);
            ids.push_back(g.m_id);
            if (g.m_id == keep)
                select = static_cast<int>(ids.size()) - 1;
        }
        choice->SetSelection(select);
    };

    fill(m_price_assumption, m_price_group_id_a,
        PlanAssumptionKind(PlanAssumptionKind::e_share_price), symbol,
        (prev_price > 0) ? prev_price
                         : (m_item_n ? m_item_n->m_price_assumption_group_id : -1));

    // A tax rate is not about a ticker, so it is never scope-filtered.
    fill(m_tax_assumption, m_tax_group_id_a,
        PlanAssumptionKind(PlanAssumptionKind::e_tax_rate), wxEmptyString,
        (prev_tax > 0) ? prev_tax
                       : (m_item_n ? m_item_n->m_tax_assumption_group_id : -1));
}

void PlanItemEntryDialog::updateAmountMode()
{
    if (!m_amount_mode || !m_unit_block)
        return;

    const bool unit_based = (m_amount_mode->GetSelection() == 1);

    // Only one of the two ways of arriving at a number is ever on screen, so it
    // is unambiguous which one is in force.
    m_unit_block->Show(unit_based);
    m_amount->Enable(!unit_based);
    m_amount_label->Enable(!unit_based);

    Layout();
    GetSizer()->Fit(this);
}

void PlanItemEntryDialog::OnAmountModeChanged(wxCommandEvent& event)
{
    updateAmountMode();
    updateComputed();
    event.Skip();
}

void PlanItemEntryDialog::OnSymbolChanged(wxCommandEvent& event)
{
    // The ticker decides which price groups apply, so the list follows it.
    rebuildAssumptionChoices();
    updateComputed();
    event.Skip();
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
        m_amount_mode->SetSelection(0);
        m_price_assumption->SetSelection(0);
        m_tax_assumption->SetSelection(0);
        updateAmountMode();
        updateComputed();
        return;
    }

    m_name->SetValue(m_item_n->m_name);
    m_group->SetSelection(index_of(m_group_id_a, m_item_n->m_group_id));
    m_kind->SetSelection(m_item_n->m_kind.id());
    m_status->SetSelection(m_item_n->m_status.id());

    // A target date is optional: an item can be planned without being scheduled.
    const bool dated = !m_item_n->m_target_date.IsEmpty();
    m_has_date->SetValue(dated);
    if (dated) {
        // mmDate::dateTime() is non-const, so take a local copy.
        mmDate target(m_item_n->m_target_date);
        m_date->setValue(target.dateTime());
    }

    m_category_id = m_item_n->m_category_id;
    if (m_category_id > 0)
        m_category->SetLabel(CategoryModel::instance().get_id_fullname(m_category_id, ":"));

    // An item is priced from units when it actually has units; otherwise it is
    // a plain amount.
    m_amount_mode->SetSelection(m_item_n->is_unit_based() ? 1 : 0);

    if (m_item_n->m_amount != 0.0)
        m_amount->SetValue(m_item_n->m_amount, 2);
    if (m_item_n->m_units > 0.0)
        m_units->SetValue(m_item_n->m_units, 4);
    m_symbol->SetValue(m_item_n->m_stock_symbol);
    if (m_item_n->m_tax_rate > 0.0)
        m_tax_rate->SetValue(m_item_n->m_tax_rate, 4);

    rebuildAssumptionChoices();
    m_notes->SetValue(m_item_n->m_notes);

    updateAmountMode();
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
        m_computed->SetLabel(_t("Enter a number of units to see the value."));
        return;
    }

    // Mirror the resolution the model performs, so the dialog shows the number
    // that will actually be stored rather than a guess of it.
    PlanItemData probe;
    probe.m_units = units;
    probe.m_stock_symbol = m_symbol->GetValue().Trim().Trim(false);

    const int pi = m_price_assumption->GetSelection();
    if (pi > 0 && static_cast<std::size_t>(pi) < m_price_group_id_a.size())
        probe.m_price_assumption_group_id = m_price_group_id_a[pi];

    const int ti = m_tax_assumption->GetSelection();
    if (ti > 0 && static_cast<std::size_t>(ti) < m_tax_group_id_a.size())
        probe.m_tax_assumption_group_id = m_tax_group_id_a[ti];

    double rate = 0.0;
    if (m_tax_rate->GetDouble(rate))
        probe.m_tax_rate = rate;

    PlanItemModel& pim = PlanItemModel::instance();
    CurrencyModel& cm = CurrencyModel::instance();
    const double price = pim.resolve_unit_price(probe);
    const double gross = units * price;
    const double net   = PlanItemData::apply_tax(gross, pim.resolve_tax_rate(probe));

    if (price <= 0.0) {
        m_computed->SetLabel(_t(
            "No price is available yet: link an assumption group, or enter a "
            "symbol that matches a holding."));
        Layout();
        return;
    }

    m_computed->SetLabel(wxString::Format(
        _t("%s units at %s  =  %s gross,  %s after tax"),
        wxString::FromDouble(units, 4),
        cm.toCurrency(price),
        cm.toCurrency(gross),
        cm.toCurrency(net)));
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

    const bool unit_based = (m_amount_mode->GetSelection() == 1);

    double amount = 0.0;
    m_amount->GetDouble(amount);
    double units = 0.0;
    m_units->GetDouble(units);

    // Validate the mode the user actually chose, rather than accepting a value
    // from the half that is hidden.
    if (unit_based && units <= 0.0) {
        wxMessageBox(_t("Enter a number of units, or switch to a fixed amount."),
            _t("Plan Item"), wxOK | wxICON_WARNING, this);
        return;
    }
    if (!unit_based && amount <= 0.0) {
        wxMessageBox(_t("Enter an amount, or switch to pricing from units."),
            _t("Plan Item"), wxOK | wxICON_WARNING, this);
        return;
    }

    PlanItemData data;
    if (m_item_n)
        data = *m_item_n;

    data.m_name        = name;
    data.m_kind        = PlanItemKind(m_kind->GetSelection());
    data.m_status      = PlanStatus(m_status->GetSelection());
    // An undated item is legitimate: it is planned but not yet scheduled.
    data.m_target_date = m_has_date->GetValue()
        ? mmDate(m_date->GetValue()).isoDate() : wxString();
    data.m_category_id = m_category_id;
    data.m_notes       = m_notes->GetValue();
    data.m_active      = true;

    // Only the chosen half is stored, so a stale value from the other one can
    // never resurface later and change what the item means.
    if (unit_based) {
        data.m_amount = 0.0;
        data.m_units  = units;
        data.m_stock_symbol = m_symbol->GetValue().Trim().Trim(false);

        const int pi = m_price_assumption->GetSelection();
        data.m_price_assumption_group_id =
            (pi > 0 && static_cast<std::size_t>(pi) < m_price_group_id_a.size())
            ? m_price_group_id_a[pi] : -1;

        const int ti = m_tax_assumption->GetSelection();
        data.m_tax_assumption_group_id =
            (ti > 0 && static_cast<std::size_t>(ti) < m_tax_group_id_a.size())
            ? m_tax_group_id_a[ti] : -1;

        double rate = 0.0;
        data.m_tax_rate = m_tax_rate->GetDouble(rate) ? rate : 0.0;
    }
    else {
        data.m_amount = amount;
        data.m_units  = 0.0;
        data.m_stock_symbol.clear();
        data.m_price_assumption_group_id = -1;
        data.m_tax_assumption_group_id   = -1;
        data.m_price_assumption_id       = -1;
        data.m_tax_assumption_id         = -1;
        data.m_tax_rate = 0.0;
    }

    const int gi = m_group->GetSelection();
    data.m_group_id = (gi >= 0 && static_cast<std::size_t>(gi) < m_group_id_a.size())
        ? m_group_id_a[gi] : -1;

    PlanItemModel& pim = PlanItemModel::instance();
    if (m_item_n) pim.save_data_n(data);
    else          pim.add_data_n(data);

    EndModal(wxID_OK);
}

void PlanItemEntryDialog::OnCategory(wxCommandEvent& /*event*/)
{
    CategoryManager dlg(this, true, m_category_id);
    if (dlg.ShowModal() != wxID_OK)
        return;

    m_category_id = dlg.getCategId();
    m_category->SetLabel(m_category_id > 0
        ? CategoryModel::instance().get_id_fullname(m_category_id, ":")
        : _t("Select Category"));
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
EVT_BUTTON(ID_DUPLICATE, PlanManagerDialog::OnDuplicate)
    EVT_BUTTON(ID_ASSUMPTIONS, PlanManagerDialog::OnAssumptions)
    EVT_BUTTON(ID_ACCOUNTS, PlanManagerDialog::OnAccounts)
    EVT_BUTTON(ID_FREE_ASSETS, PlanManagerDialog::OnFreeAssets)
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

    // A single long line gets clipped as soon as the numbers grow, so the
    // summary is laid out as label/value pairs that wrap with the dialog.
    m_totals_grid = new wxFlexGridSizer(0, 4, 4, 18);
    mainSizer->Add(m_totals_grid, 0, wxGROW | wxLEFT | wxRIGHT | wxBOTTOM, 8);

    m_totals = new wxStaticText(this, wxID_STATIC, "");
    mainSizer->Add(m_totals, 0, wxGROW | wxLEFT | wxRIGHT | wxBOTTOM, 8);

    wxBoxSizer* buttonRow = new wxBoxSizer(wxHORIZONTAL);
    mainSizer->Add(buttonRow, 0, wxGROW | wxALL, 5);

    wxButton* btnGroup = new wxButton(this, ID_ADD_GROUP, _t("Add &Group"));
    buttonRow->Add(btnGroup, 0, wxALIGN_CENTER_VERTICAL);
    mmToolTip(btnGroup, _t("Add a project such as a trip or a collection"));

    wxButton* btnItem = new wxButton(this, ID_ADD_ITEM, _t("Add &Item"));
    buttonRow->Add(btnItem, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);

    wxButton* btnEdit = new wxButton(this, wxID_EDIT, _t("&Edit"));
    buttonRow->Add(btnEdit, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);

    wxButton* btnDup = new wxButton(this, ID_DUPLICATE, _t("D&uplicate"));
    buttonRow->Add(btnDup, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);
    mmToolTip(btnDup, _t(
        "Copy the selected item, or the selected group and everything in it"));

    wxButton* btnDelete = new wxButton(this, wxID_DELETE, _t("&Delete"));
    buttonRow->Add(btnDelete, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);

    wxButton* btnAssume = new wxButton(this, ID_ASSUMPTIONS, _t("&Assumptions..."));
    buttonRow->Add(btnAssume, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 15);
    mmToolTip(btnAssume, _t("Edit the estimates the plan is calculated from"));

    wxButton* btnAccounts = new wxButton(this, ID_ACCOUNTS, _tu("Acc&ounts…"));
    buttonRow->Add(btnAccounts, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);
    mmToolTip(btnAccounts, _t("Choose which accounts count towards the plan"));

    wxButton* btnFree = new wxButton(this, ID_FREE_ASSETS, _tu("&Free assets…"));
    buttonRow->Add(btnFree, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);
    mmToolTip(btnFree, _t("Choose what is added and subtracted to reach free assets"));

    wxStaticLine* line = new wxStaticLine(this, wxID_STATIC);
    mainSizer->Add(line, 0, wxGROW | wxALL, 5);

    wxPanel* btnPanel = new wxPanel(this, wxID_ANY);
    mainSizer->Add(btnPanel, 0, wxALIGN_RIGHT, 5);
    wxBoxSizer* okSizer = new wxBoxSizer(wxHORIZONTAL);
    btnPanel->SetSizer(okSizer);
    wxButton* btnOK = new wxButton(btnPanel, wxID_OK, _t("&OK "));
    okSizer->Add(btnOK, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT | wxBOTTOM, 5);
}

void PlanManagerDialog::addItemNodes(const wxTreeItemId& parent, int64 group_id)
{
    PlanItemModel& pim = PlanItemModel::instance();
    CurrencyModel& cm = CurrencyModel::instance();

    PlanItemModel::DataA items = pim.find_group_a(group_id);

    // A target date is optional, so dated rows come first in date order and
    // undated ones collect at the end rather than sorting as if they were long
    // overdue.
    std::stable_sort(items.begin(), items.end(),
        [](const PlanItemData& x, const PlanItemData& y) {
            const bool xu = x.m_target_date.IsEmpty();
            const bool yu = y.m_target_date.IsEmpty();
            if (xu != yu)
                return !xu;
            return x.m_target_date < y.m_target_date;
        });

    for (const auto& item : items) {
        const double amt = pim.net_amount_base(item);

        // A tilde marks a figure that rests on an assumption rather than a
        // known number, so a guess is never mistaken for a fact.
        const wxString mark =
            (item.is_unit_based() || item.is_assumption_based()) ? " ~" : "";

        const wxString when = item.m_target_date.IsEmpty()
            ? _t("undated")
            : mmGetDateTimeForDisplay(item.m_target_date);

        wxString category;
        if (item.m_category_id > 0)
            category = "  " + CategoryModel::instance().get_id_fullname(item.m_category_id, ":");

        m_tree->AppendItem(parent,
            wxString::Format("%s%s   %s   %s   [%s]%s",
                item.m_name, mark,
                cm.toCurrency(amt),
                when,
                wxGetTranslation(item.m_status.name()),
                category),
            -1, -1, new PlanTreeItem(item.m_id, false));
    }
}

void PlanManagerDialog::addGroupNode(const wxTreeItemId& parent, int64 group_id, int depth)
{
    // Guard against a cycle introduced by hand-edited data: without this a
    // parent loop would recurse until the stack gave out.
    if (depth > 32)
        return;

    PlanGroupModel& pgm = PlanGroupModel::instance();
    const PlanGroupData* g_n = pgm.get_idN_data_n(group_id);
    if (!g_n)
        return;

    const wxString when = g_n->m_target_date.IsEmpty()
        ? "" : wxString::Format("  (%s)", mmGetDateTimeForDisplay(g_n->m_target_date));

    wxTreeItemId node = m_tree->AppendItem(parent,
        g_n->m_name + when, -1, -1, new PlanTreeItem(g_n->m_id, true));

    addItemNodes(node, g_n->m_id);

    // Sub-groups nest to any depth: a trip may own a leg, which owns its
    // bookings.
    for (const auto& child : pgm.find_children_a(g_n->m_id))
        addGroupNode(node, child.m_id, depth + 1);
}

void PlanManagerDialog::fillControls()
{
    m_tree->DeleteAllItems();

    PlanGroupModel& pgm = PlanGroupModel::instance();
    PlanItemModel& pim = PlanItemModel::instance();

    wxTreeItemId root = m_tree->AddRoot(_t("Plan"));

    for (const auto& group : pgm.find_root_a())
        addGroupNode(root, group.m_id, 0);

    // Items with no group still need to be reachable.
    if (!pim.find_group_a(-1).empty()) {
        wxTreeItemId ungrouped =
            m_tree->AppendItem(root, _t("(ungrouped)"), -1, -1, nullptr);
        addItemNodes(ungrouped, -1);
    }

    m_tree->ExpandAll();
    updateTotals();
}

void PlanManagerDialog::updateTotals()
{
    const PlanSummary s = PlanEngine::build_summary(mmDate::today());

    // Every figure here is money: show it in the base currency rather than as a
    // bare number, and lay the four out as wrapping pairs so nothing is clipped
    // when the amounts get long.
    struct Entry { wxString label; double value; bool emphasis; };
    const Entry entries[] = {
        { _t("Expected income"),  s.expected_income,   false },
        { _t("Committed"),        s.committed_expense, false },
        { _t("Wishlist"),         s.wishlist_expense,  false },
        { _t("Free assets"),      s.free_assets(),     true  },
    };

    if (m_totals_grid->GetItemCount() == 0) {
        for (const auto& e : entries) {
            wxStaticText* label = new wxStaticText(this, wxID_STATIC, e.label + ":");
            m_totals_grid->Add(label, 0, wxALIGN_CENTER_VERTICAL);

            wxStaticText* value = new wxStaticText(this, wxID_STATIC, "",
                wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
            if (e.emphasis) {
                wxFont f = value->GetFont();
                f.SetWeight(wxFONTWEIGHT_BOLD);
                value->SetFont(f);
            }
            m_totals_grid->Add(value, 1, wxGROW | wxALIGN_CENTER_VERTICAL);
            m_total_value_a.push_back(value);
        }
    }

    for (std::size_t i = 0; i < m_total_value_a.size() && i < WXSIZEOF(entries); ++i)
        m_total_value_a[i]->SetLabel(CurrencyModel::instance().toCurrency(entries[i].value));

    // Free assets is the number the plan exists to answer, so state the formula
    // in force rather than leaving the reader to assume the default.
    m_totals->SetLabel(wxString::Format(
        _t("Free assets = %s. Configure with the Free assets button."),
        s.free_assets_formula()));
    m_totals->Wrap(GetClientSize().GetWidth() - 24);

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

void PlanManagerDialog::OnDuplicate(wxCommandEvent& /*event*/)
{
    bool is_group = false;
    const int64 id = selectedId(is_group);
    if (id <= 0)
        return;

    PlanGroupModel& pgm = PlanGroupModel::instance();
    PlanItemModel& pim = PlanItemModel::instance();

    if (!is_group) {
        const PlanItemData* src_n = pim.get_idN_data_n(id);
        if (!src_n)
            return;

        PlanItemData copy;
        copy.clone_from(*src_n);
        copy.m_name = wxString::Format(_t("%s (copy)"), src_n->m_name);
        pim.add_data_n(copy);
        fillControls();
        return;
    }

    // Duplicating a group copies the whole project -- the sub-groups and the
    // items in them -- otherwise an empty shell would be all that appeared.
    std::function<int64(int64, int64, bool)> copy_group =
        [&](int64 src_id, int64 new_parent, bool rename) -> int64 {
            const PlanGroupData* src_n = pgm.get_idN_data_n(src_id);
            if (!src_n)
                return -1;

            PlanGroupData g;
            g.clone_from(*src_n);
            g.m_parent_id = new_parent;
            if (rename)
                g.m_name = wxString::Format(_t("%s (copy)"), src_n->m_name);

            const PlanGroupData* saved_n = pgm.add_data_n(g);
            if (!saved_n)
                return -1;
            const int64 new_id = saved_n->m_id;

            for (const auto& item : pim.find_group_a(src_id)) {
                PlanItemData copy;
                copy.clone_from(item);
                copy.m_group_id = new_id;
                pim.add_data_n(copy);
            }

            for (const auto& child : pgm.find_children_a(src_id))
                copy_group(child.m_id, new_id, false);

            return new_id;
        };

    const PlanGroupData* src_n = pgm.get_idN_data_n(id);
    copy_group(id, src_n ? src_n->m_parent_id : -1, true);
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

void PlanManagerDialog::OnFreeAssets(wxCommandEvent& /*event*/)
{
    // What counts as "free" is a judgement: whether investments are really
    // spendable, or a wishlist is really a commitment, depends on how the plan
    // is being used. So it is asked rather than assumed.
    PlanFreeAssetsParts parts = PlanEngine::free_assets_parts();

    wxArrayString labels;
    labels.Add(_t("Add cash accounts"));
    labels.Add(_t("Add investment accounts"));
    labels.Add(_t("Add expected income"));
    labels.Add(_t("Subtract committed obligations"));
    labels.Add(_t("Subtract wishlist items"));

    wxMultiChoiceDialog dlg(this,
        _t("Which components make up free assets?"),
        _t("Free Assets"), labels);

    wxArrayInt selected;
    if (parts.cash)        selected.Add(0);
    if (parts.investments) selected.Add(1);
    if (parts.income)      selected.Add(2);
    if (parts.committed)   selected.Add(3);
    if (parts.wishlist)    selected.Add(4);
    dlg.SetSelections(selected);

    if (dlg.ShowModal() != wxID_OK)
        return;

    const wxArrayInt chosen = dlg.GetSelections();
    auto picked = [&chosen](int i) {
        for (std::size_t j = 0; j < chosen.GetCount(); ++j)
            if (chosen[j] == i) return true;
        return false;
    };

    parts.cash        = picked(0);
    parts.investments = picked(1);
    parts.income      = picked(2);
    parts.committed   = picked(3);
    parts.wishlist    = picked(4);

    PlanEngine::set_free_assets_parts(parts);
    updateTotals();
}

void PlanManagerDialog::OnAccounts(wxCommandEvent& /*event*/)
{
    // Not every account belongs in a long-term plan: a business account or one
    // held for someone else would otherwise inflate free assets.
    PlanAccountsDialog dlg(this);
    if (dlg.ShowModal() == wxID_OK)
        updateTotals();
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
