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
#include "PlanAssumptionDialog.h"

// =====================================================================
// Single assumption
// =====================================================================

wxIMPLEMENT_DYNAMIC_CLASS(PlanAssumptionEntryDialog, wxDialog);

wxBEGIN_EVENT_TABLE(PlanAssumptionEntryDialog, wxDialog)
    EVT_BUTTON(wxID_OK, PlanAssumptionEntryDialog::OnOk)
    EVT_CHOICE(wxID_ANY, PlanAssumptionEntryDialog::OnKindChanged)
wxEND_EVENT_TABLE()

PlanAssumptionEntryDialog::PlanAssumptionEntryDialog()
{
}

PlanAssumptionEntryDialog::PlanAssumptionEntryDialog(
    wxWindow* parent,
    PlanAssumptionData* assumption
) :
    m_assumption_n(assumption)
{
    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY, _t("Assumption"), wxDefaultPosition, wxSize(430, 320), style);
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
    if (!m_assumption_n) {
        m_kind->SetSelection(PlanAssumptionKind::e_share_price);
        updateHint();
        return;
    }

    m_name->SetValue(m_assumption_n->m_name);
    m_kind->SetSelection(m_assumption_n->m_kind.id());
    m_value->SetValue(m_assumption_n->m_value, 4);
    m_scope->SetValue(m_assumption_n->m_scope_key);
    m_notes->SetValue(m_assumption_n->m_notes);
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
    data.m_notes     = m_notes->GetValue();
    data.m_active    = true;

    if (m_assumption_n) pam.save_data_n(data);
    else                pam.add_data_n(data);

    EndModal(wxID_OK);
}

// =====================================================================
// Assumption list
// =====================================================================

wxIMPLEMENT_DYNAMIC_CLASS(PlanAssumptionDialog, wxDialog);

wxBEGIN_EVENT_TABLE(PlanAssumptionDialog, wxDialog)
    EVT_BUTTON(wxID_ADD, PlanAssumptionDialog::OnAdd)
    EVT_BUTTON(wxID_EDIT, PlanAssumptionDialog::OnEdit)
    EVT_BUTTON(wxID_DELETE, PlanAssumptionDialog::OnDelete)
    EVT_LISTBOX_DCLICK(wxID_ANY, PlanAssumptionDialog::OnDoubleClicked)
wxEND_EVENT_TABLE()

PlanAssumptionDialog::PlanAssumptionDialog()
{
}

PlanAssumptionDialog::PlanAssumptionDialog(wxWindow* parent)
{
    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX | wxRESIZE_BORDER;
    Create(parent, wxID_ANY, _t("Plan Assumptions"), wxDefaultPosition, wxSize(560, 360), style);
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

    wxStaticText* hint = new wxStaticText(this, wxID_STATIC, _t(
        "A long-term plan rests on estimates. Recording them here means each is "
        "stated once and every dependent figure follows from it."));
    hint->Wrap(520);
    mainSizer->Add(hint, 0, wxALL, 8);

    m_listBox = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxSize(500, 200));
    mainSizer->Add(m_listBox, 1, wxGROW | wxALL, 5);

    wxBoxSizer* buttonRow = new wxBoxSizer(wxHORIZONTAL);
    mainSizer->Add(buttonRow, 0, wxGROW | wxALL, 5);

    wxButton* btnAdd = new wxButton(this, wxID_ADD, _t("&Add"));
    buttonRow->Add(btnAdd, 0, wxALIGN_CENTER_VERTICAL);
    wxButton* btnEdit = new wxButton(this, wxID_EDIT, _t("&Edit"));
    buttonRow->Add(btnEdit, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);
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

void PlanAssumptionDialog::fillControls()
{
    m_listBox->Clear();

    PlanAssumptionModel& pam = PlanAssumptionModel::instance();

    int index = 0;
    for (const auto& a : pam.find_active_a()) {
        const wxString value = a.m_kind.is_rate()
            ? wxString::Format("%.2f %%", a.as_rate() * 100.0)
            : wxString::Format("%.4f", a.m_value);

        const std::size_t used = pam.count_dependents(a.m_id);
        const wxString scope = a.m_scope_key.IsEmpty() ? "" : (" [" + a.m_scope_key + "]");

        // Showing the dependent count makes the blast radius of a change obvious.
        const wxString label = wxString::Format("%s%s = %s   -   %s, %s",
            a.m_name, scope, value,
            wxGetTranslation(a.m_kind.name()),
            wxString::Format(wxPLURAL("used by %zu item", "used by %zu items",
                static_cast<int>(used)), used));

        m_listBox->Insert(label, index++, new mmListBoxItem(a.m_id, a.m_name));
    }
}

void PlanAssumptionDialog::OnAdd(wxCommandEvent& /*event*/)
{
    PlanAssumptionEntryDialog dlg(this, nullptr);
    if (dlg.ShowModal() == wxID_OK)
        fillControls();
}

void PlanAssumptionDialog::OnEdit(wxCommandEvent& /*event*/)
{
    const int sel = m_listBox->GetSelection();
    if (sel == wxNOT_FOUND)
        return;

    mmListBoxItem* item = static_cast<mmListBoxItem*>(m_listBox->GetClientObject(sel));
    if (!item)
        return;

    const PlanAssumptionData* a_n =
        PlanAssumptionModel::instance().get_idN_data_n(item->getIndex());
    if (!a_n)
        return;

    PlanAssumptionData data = *a_n;
    PlanAssumptionEntryDialog dlg(this, &data);
    if (dlg.ShowModal() == wxID_OK)
        fillControls();
}

void PlanAssumptionDialog::OnDelete(wxCommandEvent& /*event*/)
{
    const int sel = m_listBox->GetSelection();
    if (sel == wxNOT_FOUND)
        return;

    mmListBoxItem* item = static_cast<mmListBoxItem*>(m_listBox->GetClientObject(sel));
    if (!item)
        return;

    PlanAssumptionModel& pam = PlanAssumptionModel::instance();
    const std::size_t used = pam.count_dependents(item->getIndex());

    // Deleting freezes dependent values rather than zeroing them; say so.
    const wxString msg = used > 0
        ? wxString::Format(
            _t("Delete this assumption?\n\n"
               "%zu plan item(s) use it. They will keep the value it holds now, "
               "stored directly on each item."), used)
        : _t("Delete this assumption?");

    if (wxMessageBox(msg, _t("Plan Assumptions"),
            wxYES_NO | wxICON_QUESTION, this) != wxYES)
        return;

    pam.purge_id(item->getIndex());
    fillControls();
}

void PlanAssumptionDialog::OnDoubleClicked(wxCommandEvent& event)
{
    OnEdit(event);
}
