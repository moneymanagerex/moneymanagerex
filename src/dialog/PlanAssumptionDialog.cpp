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
#include "base/mmListBoxItem.h"
#include "util/mmPath.h"
#include "util/_util.h"
#include "model/CurrencyModel.h"
#include "PlanAssumptionDialog.h"
#include <wx/statline.h>

// =====================================================================
// Single assumption
// =====================================================================

wxIMPLEMENT_DYNAMIC_CLASS(PlanAssumptionEntryDialog, wxDialog);

wxBEGIN_EVENT_TABLE(PlanAssumptionEntryDialog, wxDialog)
    EVT_BUTTON(wxID_OK, PlanAssumptionEntryDialog::OnOk)
    EVT_CHOICE(ID_GROUP, PlanAssumptionEntryDialog::OnGroupChanged)
    EVT_CHOICE(wxID_ANY, PlanAssumptionEntryDialog::OnKindChanged)
wxEND_EVENT_TABLE()

PlanAssumptionEntryDialog::PlanAssumptionEntryDialog()
{
}

PlanAssumptionEntryDialog::PlanAssumptionEntryDialog(
    wxWindow* parent,
    PlanAssumptionData* assumption,
    int64 default_group_id
) :
    m_assumption_n(assumption),
    m_default_group_id(default_group_id)
{
    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY, _t("Assumption"), wxDefaultPosition, wxSize(430, 380), style);
    mmThemeAutoColour(this);
}

bool PlanAssumptionEntryDialog::Create(wxWindow* parent, wxWindowID id,
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

void PlanAssumptionEntryDialog::CreateControls()
{
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(mainSizer);

    wxFlexGridSizer* grid = new wxFlexGridSizer(0, 2, 5, 10);
    grid->AddGrowableCol(1, 1);
    mainSizer->Add(grid, 0, wxGROW | wxALL, 10);

    grid->Add(new wxStaticText(this, wxID_STATIC, _t("Name")),
        0, wxALIGN_CENTER_VERTICAL);
    m_name = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(240, -1));
    grid->Add(m_name, 1, wxGROW);

    grid->Add(new wxStaticText(this, wxID_STATIC, _t("Group")),
        0, wxALIGN_CENTER_VERTICAL);
    m_group = new wxChoice(this, ID_GROUP);
    m_group->Append(_t("(standalone)"));
    m_group_id_a.push_back(-1);
    for (const auto& g : PlanAssumptionGroupModel::instance().find_active_a()) {
        const wxString scope = g.m_scope_key.IsEmpty() ? "" : (" [" + g.m_scope_key + "]");
        m_group->Append(g.m_name + scope);
        m_group_id_a.push_back(g.m_id);
    }
    mmToolTip(m_group, _t(
        "Belonging to a group makes this one of several candidate values. The "
        "group decides what the value is about and which member is in force."));
    grid->Add(m_group, 1, wxGROW);

    grid->Add(new wxStaticText(this, wxID_STATIC, _t("Type")),
        0, wxALIGN_CENTER_VERTICAL);
    m_kind = new wxChoice(this, wxID_ANY);
    for (int i = 0; i < PlanAssumptionKind::size; ++i)
        m_kind->Append(wxGetTranslation(PlanAssumptionKind(i).name()));
    grid->Add(m_kind, 1, wxGROW);

    grid->Add(new wxStaticText(this, wxID_STATIC, _t("Value")),
        0, wxALIGN_CENTER_VERTICAL);
    m_value = new mmTextCtrl(this, wxID_ANY, "",
        wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER);
    grid->Add(m_value, 1, wxGROW);

    grid->Add(new wxStaticText(this, wxID_STATIC, _t("Applies to")),
        0, wxALIGN_CENTER_VERTICAL);
    m_scope = new wxTextCtrl(this, wxID_ANY);
    mmToolTip(m_scope, _t("For a share price, the stock symbol (e.g. MSFT). Optional."));
    grid->Add(m_scope, 1, wxGROW);

    grid->Add(new wxStaticText(this, wxID_STATIC, _t("Unit")),
        0, wxALIGN_CENTER_VERTICAL);
    m_unit = new wxTextCtrl(this, wxID_ANY);
    mmToolTip(m_unit, _t(
        "How to read the value: a currency symbol, %, or free text. Taken from "
        "the group when this belongs to one."));
    grid->Add(m_unit, 1, wxGROW);

    grid->Add(new wxStaticText(this, wxID_STATIC, _t("Notes")),
        0, wxALIGN_TOP);
    m_notes = new wxTextCtrl(this, wxID_ANY, "",
        wxDefaultPosition, wxSize(240, 60), wxTE_MULTILINE);
    grid->Add(m_notes, 1, wxGROW);

    m_hint = new wxStaticText(this, wxID_STATIC, "");
    m_hint->Wrap(390);
    mainSizer->Add(m_hint, 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);

    wxPanel* btnPanel = new wxPanel(this, wxID_ANY);
    mainSizer->Add(btnPanel, 0, wxALIGN_RIGHT, 5);
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    btnPanel->SetSizer(btnSizer);

    wxButton* btnOK = new wxButton(btnPanel, wxID_OK, _t("&OK "));
    btnSizer->Add(btnOK, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT | wxBOTTOM, 5);
    wxButton* btnCancel = new wxButton(btnPanel, wxID_CANCEL, wxGetTranslation(g_CancelLabel));
    btnSizer->Add(btnCancel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT | wxBOTTOM, 5);
}

void PlanAssumptionEntryDialog::applyGroupConstraints()
{
    const int gi = m_group->GetSelection();
    const int64 group_id = (gi >= 0 && static_cast<std::size_t>(gi) < m_group_id_a.size())
        ? m_group_id_a[gi] : -1;

    if (group_id <= 0) {
        m_kind->Enable(true);
        m_scope->Enable(true);
        m_unit->Enable(true);
        return;
    }

    // A member cannot disagree with its group about what the value is for, so
    // those fields are taken from the group and shown read-only rather than
    // silently overridden on save.
    const PlanAssumptionGroupData* g_n =
        PlanAssumptionGroupModel::instance().get_idN_data_n(group_id);
    if (!g_n)
        return;

    m_kind->SetSelection(g_n->m_kind.id());
    m_scope->SetValue(g_n->m_scope_key);
    m_unit->SetValue(g_n->unit_or_default());

    m_kind->Enable(false);
    m_scope->Enable(false);
    m_unit->Enable(false);
}

void PlanAssumptionEntryDialog::OnGroupChanged(wxCommandEvent& /*event*/)
{
    applyGroupConstraints();
    updateHint();
}

void PlanAssumptionEntryDialog::updateHint()
{
    const PlanAssumptionKind kind(m_kind->GetSelection());
    m_hint->SetLabel(kind.is_rate()
        ? _t("A rate may be entered as 32 or as 0.32; both mean 32%. "
             "To express one percent, enter 0.01.")
        : _t("Every plan item linked to this assumption is recalculated when "
             "the value changes."));
    m_hint->Wrap(390);
    Layout();
}

void PlanAssumptionEntryDialog::fillControls()
{
    auto index_of = [](const std::vector<int64>& a, int64 id) -> int {
        for (std::size_t i = 0; i < a.size(); ++i)
            if (a[i] == id) return static_cast<int>(i);
        return 0;
    };

    if (!m_assumption_n) {
        m_kind->SetSelection(PlanAssumptionKind::e_share_price);
        m_group->SetSelection(index_of(m_group_id_a, m_default_group_id));
        applyGroupConstraints();
        updateHint();
        return;
    }

    m_name->SetValue(m_assumption_n->m_name);
    m_group->SetSelection(index_of(m_group_id_a, m_assumption_n->m_group_id));
    m_kind->SetSelection(m_assumption_n->m_kind.id());
    m_value->SetValue(m_assumption_n->m_value, 4);
    m_scope->SetValue(m_assumption_n->m_scope_key);
    m_unit->SetValue(m_assumption_n->m_unit);
    m_notes->SetValue(m_assumption_n->m_notes);
    applyGroupConstraints();
    updateHint();
}

void PlanAssumptionEntryDialog::OnKindChanged(wxCommandEvent& /*event*/)
{
    updateHint();
}

void PlanAssumptionEntryDialog::OnOk(wxCommandEvent& /*event*/)
{
    const wxString name = m_name->GetValue().Trim().Trim(false);
    if (name.IsEmpty()) {
        wxMessageBox(_t("Enter a name for the assumption."),
            _t("Assumption"), wxOK | wxICON_WARNING, this);
        return;
    }

    double value = 0.0;
    if (!m_value->GetDouble(value)) {
        wxMessageBox(_t("Enter a numeric value."),
            _t("Assumption"), wxOK | wxICON_WARNING, this);
        return;
    }
    if (value <= 0.0) {
        wxMessageBox(_t("The value must be greater than zero."),
            _t("Assumption"), wxOK | wxICON_WARNING, this);
        return;
    }

    PlanAssumptionModel& pam = PlanAssumptionModel::instance();

    // Names are the handle used elsewhere, so they must stay unique.
    const PlanAssumptionData* clash_n = pam.get_name_data_n(name);
    if (clash_n && (!m_assumption_n || clash_n->m_id != m_assumption_n->m_id)) {
        wxMessageBox(_t("An assumption with this name already exists."),
            _t("Assumption"), wxOK | wxICON_WARNING, this);
        return;
    }

    PlanAssumptionData data;
    if (m_assumption_n)
        data = *m_assumption_n;

    data.m_name      = name;
    data.m_kind      = PlanAssumptionKind(m_kind->GetSelection());
    data.m_value     = value;
    data.m_scope_key = m_scope->GetValue().Trim().Trim(false);
    data.m_unit      = m_unit->GetValue().Trim().Trim(false);
    data.m_notes     = m_notes->GetValue();
    data.m_active    = true;

    const int gi = m_group->GetSelection();
    const int64 group_id = (gi >= 0 && static_cast<std::size_t>(gi) < m_group_id_a.size())
        ? m_group_id_a[gi] : -1;
    data.m_group_id = group_id;

    // The group is the authority on what its members are about, so take those
    // fields from it rather than trusting disabled controls.
    if (group_id > 0) {
        const PlanAssumptionGroupData* g_n =
            PlanAssumptionGroupModel::instance().get_idN_data_n(group_id);
        if (g_n) {
            data.m_kind      = g_n->m_kind;
            data.m_scope_key = g_n->m_scope_key;
            data.m_unit      = g_n->m_unit;
        }
    }

    const PlanAssumptionData* saved_n = nullptr;
    if (m_assumption_n) saved_n = pam.save_data_n(data);
    else                saved_n = pam.add_data_n(data);

    // A group with exactly one value has an obvious active member; setting it
    // here saves the user a step they would always have to take.
    if (saved_n && group_id > 0) {
        PlanAssumptionGroupModel& pagm = PlanAssumptionGroupModel::instance();
        if (pagm.get_active_assumption_id(group_id) <= 0)
            pagm.set_active_assumption(group_id, saved_n->m_id);
    }

    EndModal(wxID_OK);
}

// =====================================================================
// Assumption groups and their members
// =====================================================================

wxIMPLEMENT_DYNAMIC_CLASS(PlanAssumptionGroupEntryDialog, wxDialog);

wxBEGIN_EVENT_TABLE(PlanAssumptionGroupEntryDialog, wxDialog)
    EVT_BUTTON(wxID_OK, PlanAssumptionGroupEntryDialog::OnOk)
    EVT_CHOICE(wxID_ANY, PlanAssumptionGroupEntryDialog::OnKindChanged)
wxEND_EVENT_TABLE()

PlanAssumptionGroupEntryDialog::PlanAssumptionGroupEntryDialog()
{
}

PlanAssumptionGroupEntryDialog::PlanAssumptionGroupEntryDialog(
    wxWindow* parent, PlanAssumptionGroupData* group
) :
    m_group_n(group)
{
    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY,
        group ? _t("Edit Assumption Group") : _t("New Assumption Group"),
        wxDefaultPosition, wxDefaultSize, style);
    mmThemeAutoColour(this);
}

bool PlanAssumptionGroupEntryDialog::Create(wxWindow* parent, wxWindowID id,
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

void PlanAssumptionGroupEntryDialog::CreateControls()
{
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(mainSizer);

    wxFlexGridSizer* grid = new wxFlexGridSizer(0, 2, 5, 10);
    grid->AddGrowableCol(1, 1);
    mainSizer->Add(grid, 0, wxGROW | wxALL, 10);

    grid->Add(new wxStaticText(this, wxID_STATIC, _t("Name")), 0, wxALIGN_CENTER_VERTICAL);
    m_name = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(260, -1));
    mmToolTip(m_name, _t("What the group is about, e.g. 'MSFT share price'"));
    grid->Add(m_name, 1, wxGROW);

    grid->Add(new wxStaticText(this, wxID_STATIC, _t("Type")), 0, wxALIGN_CENTER_VERTICAL);
    m_kind = new wxChoice(this, wxID_ANY);
    for (int i = 0; i < PlanAssumptionKind::size; ++i)
        m_kind->Append(wxGetTranslation(PlanAssumptionKind(i).name()));
    mmToolTip(m_kind, _t(
        "What question the group answers. Only items asking the same question "
        "may use it."));
    grid->Add(m_kind, 1, wxGROW);

    grid->Add(new wxStaticText(this, wxID_STATIC, _t("Applies to")), 0, wxALIGN_CENTER_VERTICAL);
    m_scope = new wxTextCtrl(this, wxID_ANY);
    mmToolTip(m_scope, _t(
        "For a share price, the stock symbol (e.g. MSFT). Leave empty to make "
        "the group generic and usable anywhere."));
    grid->Add(m_scope, 1, wxGROW);

    grid->Add(new wxStaticText(this, wxID_STATIC, _t("Unit")), 0, wxALIGN_CENTER_VERTICAL);
    m_unit = new wxTextCtrl(this, wxID_ANY);
    mmToolTip(m_unit, _t(
        "How a value in this group should be read: a currency symbol, %, or "
        "free text such as 'shares'. Members share the unit."));
    grid->Add(m_unit, 1, wxGROW);

    grid->Add(new wxStaticText(this, wxID_STATIC, _t("Notes")), 0, wxALIGN_TOP);
    m_notes = new wxTextCtrl(this, wxID_ANY, "",
        wxDefaultPosition, wxSize(260, 60), wxTE_MULTILINE);
    grid->Add(m_notes, 1, wxGROW);

    m_hint = new wxStaticText(this, wxID_STATIC, "");
    m_hint->Wrap(400);
    mainSizer->Add(m_hint, 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);

    wxPanel* btnPanel = new wxPanel(this, wxID_ANY);
    mainSizer->Add(btnPanel, 0, wxALIGN_RIGHT, 5);
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    btnPanel->SetSizer(btnSizer);
    wxButton* btnOK = new wxButton(btnPanel, wxID_OK, _t("&OK "));
    btnSizer->Add(btnOK, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT | wxBOTTOM, 5);
    wxButton* btnCancel = new wxButton(btnPanel, wxID_CANCEL, wxGetTranslation(g_CancelLabel));
    btnSizer->Add(btnCancel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT | wxBOTTOM, 5);
}

void PlanAssumptionGroupEntryDialog::updateHint()
{
    const PlanAssumptionKind kind(m_kind->GetSelection());

    if (m_unit->GetValue().IsEmpty())
        m_unit->SetHint(kind.is_rate() ? "%" : _t("currency"));

    m_hint->SetLabel(kind.is_rate()
        ? _t("Members hold alternative rates. A rate may be entered as 32 or "
             "as 0.32; both mean 32%.")
        : _t("Members hold alternative values for the same thing. Items follow "
             "whichever member is active, so switching it moves the whole plan."));
    m_hint->Wrap(400);
    Layout();
}

void PlanAssumptionGroupEntryDialog::fillControls()
{
    if (!m_group_n) {
        m_kind->SetSelection(PlanAssumptionKind::e_share_price);
        updateHint();
        return;
    }

    m_name->SetValue(m_group_n->m_name);
    m_kind->SetSelection(m_group_n->m_kind.id());
    m_scope->SetValue(m_group_n->m_scope_key);
    m_unit->SetValue(m_group_n->m_unit);
    m_notes->SetValue(m_group_n->m_notes);
    updateHint();
}

void PlanAssumptionGroupEntryDialog::OnKindChanged(wxCommandEvent& /*event*/)
{
    updateHint();
}

void PlanAssumptionGroupEntryDialog::OnOk(wxCommandEvent& /*event*/)
{
    const wxString name = m_name->GetValue().Trim().Trim(false);
    if (name.IsEmpty()) {
        wxMessageBox(_t("Enter a name for the group."),
            _t("Assumption Group"), wxOK | wxICON_WARNING, this);
        return;
    }

    PlanAssumptionGroupModel& pagm = PlanAssumptionGroupModel::instance();

    const PlanAssumptionGroupData* clash_n = pagm.get_name_data_n(name);
    if (clash_n && (!m_group_n || clash_n->m_id != m_group_n->m_id)) {
        wxMessageBox(_t("A group with this name already exists."),
            _t("Assumption Group"), wxOK | wxICON_WARNING, this);
        return;
    }

    PlanAssumptionGroupData data;
    if (m_group_n)
        data = *m_group_n;

    data.m_name      = name;
    data.m_kind      = PlanAssumptionKind(m_kind->GetSelection());
    data.m_scope_key = m_scope->GetValue().Trim().Trim(false);
    data.m_unit      = m_unit->GetValue().Trim().Trim(false);
    data.m_notes     = m_notes->GetValue();
    data.m_active    = true;

    if (m_group_n) pagm.save_data_n(data);
    else           pagm.add_data_n(data);

    EndModal(wxID_OK);
}

// =====================================================================
// Assumption manager
// =====================================================================

wxIMPLEMENT_DYNAMIC_CLASS(PlanAssumptionDialog, wxDialog);

wxBEGIN_EVENT_TABLE(PlanAssumptionDialog, wxDialog)
    EVT_BUTTON(ID_ADD_GROUP, PlanAssumptionDialog::OnAddGroup)
    EVT_BUTTON(wxID_ADD, PlanAssumptionDialog::OnAdd)
    EVT_BUTTON(wxID_EDIT, PlanAssumptionDialog::OnEdit)
    EVT_BUTTON(ID_DUPLICATE, PlanAssumptionDialog::OnDuplicate)
    EVT_BUTTON(ID_SET_ACTIVE, PlanAssumptionDialog::OnSetActive)
    EVT_BUTTON(wxID_DELETE, PlanAssumptionDialog::OnDelete)
    EVT_TREE_ITEM_ACTIVATED(wxID_ANY, PlanAssumptionDialog::OnDoubleClicked)
wxEND_EVENT_TABLE()

PlanAssumptionDialog::PlanAssumptionDialog()
{
}

PlanAssumptionDialog::PlanAssumptionDialog(wxWindow* parent)
{
    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX | wxRESIZE_BORDER;
    Create(parent, wxID_ANY, _t("Plan Assumptions"), wxDefaultPosition, wxSize(620, 440), style);
    mmThemeAutoColour(this);
}

bool PlanAssumptionDialog::Create(wxWindow* parent, wxWindowID id,
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

void PlanAssumptionDialog::CreateControls()
{
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(mainSizer);

    m_hint = new wxStaticText(this, wxID_STATIC, _t(
        "A plan rests on estimates. A group holds the alternatives for one "
        "question - a conservative, a base and an optimistic price, say - and "
        "one member is active. Items follow the group, so switching the active "
        "member here moves every figure that depends on it."));
    m_hint->Wrap(580);
    mainSizer->Add(m_hint, 0, wxALL, 8);

    m_tree = new wxTreeCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(580, 240),
        wxTR_HAS_BUTTONS | wxTR_HIDE_ROOT | wxTR_SINGLE | wxTR_FULL_ROW_HIGHLIGHT);
    mainSizer->Add(m_tree, 1, wxGROW | wxALL, 5);

    wxBoxSizer* buttonRow = new wxBoxSizer(wxHORIZONTAL);
    mainSizer->Add(buttonRow, 0, wxGROW | wxALL, 5);

    wxButton* btnAddGroup = new wxButton(this, ID_ADD_GROUP, _t("Add &Group"));
    buttonRow->Add(btnAddGroup, 0, wxALIGN_CENTER_VERTICAL);
    mmToolTip(btnAddGroup, _t("Add a set of alternative values for one question"));

    wxButton* btnAdd = new wxButton(this, wxID_ADD, _t("&Add Value"));
    buttonRow->Add(btnAdd, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);
    mmToolTip(btnAdd, _t("Add a candidate value to the selected group"));

    wxButton* btnActive = new wxButton(this, ID_SET_ACTIVE, _t("Set A&ctive"));
    buttonRow->Add(btnActive, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);
    mmToolTip(btnActive, _t(
        "Make the selected value the one its group uses. Every item following "
        "that group changes at once."));

    wxButton* btnEdit = new wxButton(this, wxID_EDIT, _t("&Edit"));
    buttonRow->Add(btnEdit, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);

    wxButton* btnDup = new wxButton(this, ID_DUPLICATE, _t("D&uplicate"));
    buttonRow->Add(btnDup, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);

    wxButton* btnDelete = new wxButton(this, wxID_DELETE, _t("&Delete"));
    buttonRow->Add(btnDelete, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);

    wxStaticLine* line = new wxStaticLine(this, wxID_STATIC,
        wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
    mainSizer->Add(line, 0, wxGROW | wxALL, 5);

    wxPanel* btnPanel = new wxPanel(this, wxID_ANY);
    mainSizer->Add(btnPanel, 0, wxALIGN_RIGHT, 5);
    wxBoxSizer* okSizer = new wxBoxSizer(wxHORIZONTAL);
    btnPanel->SetSizer(okSizer);
    wxButton* btnOK = new wxButton(btnPanel, wxID_OK, _t("&OK "));
    okSizer->Add(btnOK, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT | wxBOTTOM, 5);
}

const wxString PlanAssumptionDialog::formatValue(
    const PlanAssumptionData& a, const wxString& unit
) const {
    // A bare number does not say whether it is a price or a percentage, so the
    // unit is always shown: from the group when it has one, otherwise inferred
    // from the kind, otherwise the base currency for money-like values.
    const wxString u = unit.IsEmpty() ? a.unit_or_default() : unit;

    if (a.m_kind.is_rate())
        return wxString::Format("%s %s",
            wxString::FromDouble(a.as_rate() * 100.0, 2),
            u.IsEmpty() ? "%" : u);

    if (u.IsEmpty())
        return CurrencyModel::instance().toCurrency(a.m_value);

    return wxString::Format("%s %s", wxString::FromDouble(a.m_value, 4), u);
}

void PlanAssumptionDialog::fillControls()
{
    m_tree->DeleteAllItems();

    PlanAssumptionModel& pam = PlanAssumptionModel::instance();
    PlanAssumptionGroupModel& pagm = PlanAssumptionGroupModel::instance();

    wxTreeItemId root = m_tree->AddRoot(_t("Assumptions"));

    for (const auto& g : pagm.find_active_a()) {
        const int used = pagm.count_dependents(g.m_id);
        const wxString scope = g.m_scope_key.IsEmpty() ? "" : (" [" + g.m_scope_key + "]");

        wxTreeItemId gnode = m_tree->AppendItem(root,
            wxString::Format("%s%s   -   %s, %s",
                g.m_name, scope,
                wxGetTranslation(g.m_kind.name()),
                wxString::Format(wxPLURAL("used by %d item", "used by %d items", used), used)),
            -1, -1, new AssumptionTreeItem(g.m_id, true));

        const int64 active_id = pagm.get_active_assumption_id(g.m_id);

        for (const auto& a : pam.find_group_a(g.m_id)) {
            const bool is_active = (a.m_id == active_id);
            m_tree->AppendItem(gnode,
                wxString::Format("%s%s = %s",
                    is_active ? wxString::FromUTF8("\xE2\x97\x8F ") : wxString("   "),
                    a.m_name, formatValue(a, g.m_unit)),
                -1, -1, new AssumptionTreeItem(a.m_id, false));
        }

        if (pam.find_group_a(g.m_id).empty())
            m_tree->AppendItem(gnode, _t("(no values yet)"), -1, -1, nullptr);
    }

    // Assumptions that belong to no group are still usable as a pinned value.
    wxTreeItemId standalone;
    for (const auto& a : pam.find_active_a()) {
        if (a.is_grouped())
            continue;
        if (!standalone.IsOk())
            standalone = m_tree->AppendItem(root, _t("(standalone)"), -1, -1, nullptr);

        const std::size_t used = pam.count_dependents(a.m_id);
        const wxString scope = a.m_scope_key.IsEmpty() ? "" : (" [" + a.m_scope_key + "]");
        m_tree->AppendItem(standalone,
            wxString::Format("%s%s = %s   -   %s, %s",
                a.m_name, scope, formatValue(a, wxEmptyString),
                wxGetTranslation(a.m_kind.name()),
                wxString::Format(wxPLURAL("used by %zu item", "used by %zu items",
                    static_cast<int>(used)), used)),
            -1, -1, new AssumptionTreeItem(a.m_id, false));
    }

    m_tree->ExpandAll();
}

int64 PlanAssumptionDialog::selectedId(bool& is_group) const
{
    const wxTreeItemId sel = m_tree->GetSelection();
    if (!sel.IsOk())
        return -1;
    AssumptionTreeItem* data = static_cast<AssumptionTreeItem*>(m_tree->GetItemData(sel));
    if (!data)
        return -1;
    is_group = data->is_group();
    return data->id();
}

void PlanAssumptionDialog::OnAddGroup(wxCommandEvent& /*event*/)
{
    PlanAssumptionGroupEntryDialog dlg(this, nullptr);
    if (dlg.ShowModal() == wxID_OK)
        fillControls();
}

void PlanAssumptionDialog::OnAdd(wxCommandEvent& /*event*/)
{
    // Adding while a group (or one of its values) is selected puts the new
    // value straight into that group, which is nearly always what is meant.
    bool is_group = false;
    const int64 sel = selectedId(is_group);

    int64 group_id = -1;
    if (sel > 0) {
        if (is_group)
            group_id = sel;
        else {
            const PlanAssumptionData* a_n =
                PlanAssumptionModel::instance().get_idN_data_n(sel);
            if (a_n)
                group_id = a_n->m_group_id;
        }
    }

    PlanAssumptionEntryDialog dlg(this, nullptr, group_id);
    if (dlg.ShowModal() == wxID_OK)
        fillControls();
}

void PlanAssumptionDialog::OnEdit(wxCommandEvent& /*event*/)
{
    bool is_group = false;
    const int64 id = selectedId(is_group);
    if (id <= 0)
        return;

    if (is_group) {
        const PlanAssumptionGroupData* g_n =
            PlanAssumptionGroupModel::instance().get_idN_data_n(id);
        if (!g_n)
            return;
        PlanAssumptionGroupData data = *g_n;
        PlanAssumptionGroupEntryDialog dlg(this, &data);
        if (dlg.ShowModal() == wxID_OK)
            fillControls();
        return;
    }

    const PlanAssumptionData* a_n = PlanAssumptionModel::instance().get_idN_data_n(id);
    if (!a_n)
        return;
    PlanAssumptionData data = *a_n;
    PlanAssumptionEntryDialog dlg(this, &data);
    if (dlg.ShowModal() == wxID_OK)
        fillControls();
}

void PlanAssumptionDialog::OnDuplicate(wxCommandEvent& /*event*/)
{
    bool is_group = false;
    const int64 id = selectedId(is_group);
    if (id <= 0)
        return;

    PlanAssumptionModel& pam = PlanAssumptionModel::instance();
    PlanAssumptionGroupModel& pagm = PlanAssumptionGroupModel::instance();

    if (!is_group) {
        const PlanAssumptionData* a_n = pam.get_idN_data_n(id);
        if (!a_n)
            return;

        PlanAssumptionData copy;
        copy.clone_from(*a_n);
        copy.m_name = pam.make_unique_name(
            wxString::Format(_t("%s (copy)"), a_n->m_name));
        pam.add_data_n(copy);
        fillControls();
        return;
    }

    // Duplicating a group copies its candidate values too: an empty copy of a
    // scenario would be of no use.
    const PlanAssumptionGroupData* g_n = pagm.get_idN_data_n(id);
    if (!g_n)
        return;

    PlanAssumptionGroupData gcopy;
    gcopy.clone_from(*g_n);
    gcopy.m_active_assumption_id = -1;
    for (int n = 2; n < 1000; ++n) {
        const wxString candidate = wxString::Format(_t("%s (copy)"), g_n->m_name)
            + ((n == 2) ? wxString() : wxString::Format(" %d", n));
        if (!pagm.get_name_data_n(candidate)) {
            gcopy.m_name = candidate;
            break;
        }
    }

    const PlanAssumptionGroupData* saved_n = pagm.add_data_n(gcopy);
    if (!saved_n)
        return;

    const int64 old_active = pagm.get_active_assumption_id(id);
    int64 new_active = -1;

    for (const auto& a : pam.find_group_a(id)) {
        PlanAssumptionData acopy;
        acopy.clone_from(a);
        acopy.m_group_id = saved_n->m_id;
        acopy.m_name = pam.make_unique_name(
            wxString::Format(_t("%s (copy)"), a.m_name));
        const PlanAssumptionData* saved_a = pam.add_data_n(acopy);
        if (saved_a && a.m_id == old_active)
            new_active = saved_a->m_id;
    }

    if (new_active > 0)
        pagm.set_active_assumption(saved_n->m_id, new_active);

    fillControls();
}

void PlanAssumptionDialog::OnSetActive(wxCommandEvent& /*event*/)
{
    bool is_group = false;
    const int64 id = selectedId(is_group);
    if (id <= 0 || is_group) {
        wxMessageBox(_t("Select a value inside a group to make it active."),
            _t("Plan Assumptions"), wxOK | wxICON_INFORMATION, this);
        return;
    }

    const PlanAssumptionData* a_n = PlanAssumptionModel::instance().get_idN_data_n(id);
    if (!a_n || !a_n->is_grouped()) {
        wxMessageBox(_t("This value does not belong to a group, so there is "
                        "nothing to switch."),
            _t("Plan Assumptions"), wxOK | wxICON_INFORMATION, this);
        return;
    }

    PlanAssumptionGroupModel& pagm = PlanAssumptionGroupModel::instance();
    const int used = pagm.count_dependents(a_n->m_group_id);

    // Switching is the whole point of a group, but it moves every dependent
    // figure, so say how many before doing it.
    if (used > 0) {
        const wxString msg = wxString::Format(
            wxPLURAL("Make this the active value?\n\n%d plan item follows this group and will be recalculated.",
                     "Make this the active value?\n\n%d plan items follow this group and will be recalculated.",
                     used), used);
        if (wxMessageBox(msg, _t("Plan Assumptions"),
                wxYES_NO | wxICON_QUESTION, this) != wxYES)
            return;
    }

    pagm.set_active_assumption(a_n->m_group_id, a_n->m_id);
    fillControls();
}

void PlanAssumptionDialog::OnDelete(wxCommandEvent& /*event*/)
{
    bool is_group = false;
    const int64 id = selectedId(is_group);
    if (id <= 0)
        return;

    PlanAssumptionModel& pam = PlanAssumptionModel::instance();
    PlanAssumptionGroupModel& pagm = PlanAssumptionGroupModel::instance();

    if (is_group) {
        const int used = pagm.count_dependents(id);
        const wxString msg = used > 0
            ? wxString::Format(
                wxPLURAL("Delete this group?\n\nIts values become standalone assumptions. %d plan item follows the group and will keep the value in force now.",
                         "Delete this group?\n\nIts values become standalone assumptions. %d plan items follow the group and will keep the value in force now.",
                         used), used)
            : _t("Delete this group?\n\nIts values become standalone assumptions.");

        if (wxMessageBox(msg, _t("Plan Assumptions"),
                wxYES_NO | wxICON_QUESTION, this) != wxYES)
            return;

        pagm.purge_id(id);
        fillControls();
        return;
    }

    const std::size_t used = pam.count_dependents(id);
    const wxString msg = used > 0
        ? wxString::Format(
            _t("Delete this value?\n\n"
               "%zu plan item(s) pin it. They will keep the value it holds now, "
               "stored directly on each item."), used)
        : _t("Delete this value?");

    if (wxMessageBox(msg, _t("Plan Assumptions"),
            wxYES_NO | wxICON_QUESTION, this) != wxYES)
        return;

    pam.purge_id(id);
    fillControls();
}

void PlanAssumptionDialog::OnDoubleClicked(wxTreeEvent& /*event*/)
{
    wxCommandEvent evt;
    OnEdit(evt);
}
