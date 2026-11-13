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
#include <wx/numdlg.h>
#include <wx/textdlg.h>
#include "util/mmPath.h"
#include "util/_util.h"
#include "model/BudgetPeriodModel.h"
#include "BudgetSegmentDialog.h"

wxIMPLEMENT_DYNAMIC_CLASS(BudgetSegmentDialog, wxDialog);

wxBEGIN_EVENT_TABLE(BudgetSegmentDialog, wxDialog)
    EVT_BUTTON(wxID_ADD, BudgetSegmentDialog::OnAdd)
    EVT_BUTTON(wxID_EDIT, BudgetSegmentDialog::OnEdit)
    EVT_BUTTON(wxID_DELETE, BudgetSegmentDialog::OnDelete)
    EVT_BUTTON(ID_SPLIT_HALVES, BudgetSegmentDialog::OnSplitHalves)
    EVT_LISTBOX_DCLICK(wxID_ANY, BudgetSegmentDialog::OnDoubleClicked)
wxEND_EVENT_TABLE()

BudgetSegmentDialog::BudgetSegmentDialog()
{
}

BudgetSegmentDialog::BudgetSegmentDialog(wxWindow* parent, int64 budget_period_id) :
    m_bp_id(budget_period_id)
{
    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX | wxRESIZE_BORDER;
    Create(parent, wxID_ANY, _t("Budget Segments"), wxDefaultPosition, wxSize(520, 340), style);
    mmThemeAutoColour(this);
}

bool BudgetSegmentDialog::Create(wxWindow* parent, wxWindowID id,
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

void BudgetSegmentDialog::CreateControls()
{
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(mainSizer);

    const wxString bp_name = BudgetPeriodModel::instance().get_id_name_n(m_bp_id);
    wxStaticText* header = new wxStaticText(this, wxID_STATIC,
        wxString::Format(_t("Segments of budget period: %s"),
            bp_name.IsEmpty() ? "-" : bp_name));
    mainSizer->Add(header, 0, wxALL, 5);

    wxStaticText* hint = new wxStaticText(this, wxID_STATIC, _t(
        "A segment covers part of the month. Day 31 always means the last day "
        "of the month, so a 16-31 segment stays correct in February."));
    hint->Wrap(480);
    mainSizer->Add(hint, 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);

    m_listBox = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxSize(400, 180));
    mainSizer->Add(m_listBox, 1, wxGROW | wxALL, 5);

    wxBoxSizer* buttonRow = new wxBoxSizer(wxHORIZONTAL);
    mainSizer->Add(buttonRow, 0, wxGROW | wxALL, 5);

    wxButton* btnAdd = new wxButton(this, wxID_ADD, _t("&Add"));
    buttonRow->Add(btnAdd, 0, wxALIGN_CENTER_VERTICAL);
    mmToolTip(btnAdd, _t("Add a new segment"));

    wxButton* btnEdit = new wxButton(this, wxID_EDIT, _t("&Edit"));
    buttonRow->Add(btnEdit, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);
    mmToolTip(btnEdit, _t("Edit the selected segment"));

    wxButton* btnDelete = new wxButton(this, wxID_DELETE, _t("&Delete"));
    buttonRow->Add(btnDelete, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);
    mmToolTip(btnDelete, _t("Delete the selected segment"));

    wxButton* btnHalves = new wxButton(this, ID_SPLIT_HALVES, _t("Split into &halves"));
    buttonRow->Add(btnHalves, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 15);
    mmToolTip(btnHalves, _t("Create two paycheck-aligned segments: day 1-15 and day 16 to end of month"));

    wxStaticLine* line = new wxStaticLine(this, wxID_STATIC,
        wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
    mainSizer->Add(line, 0, wxGROW | wxALL, 5);

    wxPanel* btnPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    mainSizer->Add(btnPanel, 0, wxALIGN_RIGHT, 5);

    wxBoxSizer* okSizer = new wxBoxSizer(wxHORIZONTAL);
    btnPanel->SetSizer(okSizer);

    wxButton* btnOK = new wxButton(btnPanel, wxID_OK, _t("&OK "));
    okSizer->Add(btnOK, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT | wxBOTTOM, 5);

    wxButton* btnCancel = new wxButton(btnPanel, wxID_CANCEL, wxGetTranslation(g_CancelLabel));
    okSizer->Add(btnCancel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT | wxBOTTOM, 5);
}

void BudgetSegmentDialog::fillControls()
{
    m_listBox->Clear();

    int index = 0;
    for (const auto& seg : BudgetSegmentModel::instance().find_period_a(m_bp_id)) {
        const wxString end = (seg.m_end_day >= 31)
            ? _t("end of month")
            : wxString::Format("%d", seg.m_end_day);
        const wxString label = wxString::Format("%s  (%d - %s)",
            seg.m_name, seg.m_start_day, end);
        m_listBox->Insert(label, index++, new mmListBoxItem(seg.m_id, seg.m_name));
    }
}

bool BudgetSegmentDialog::editSegment(BudgetSegmentData& seg)
{
    const wxString name = wxGetTextFromUser(
        _t("Segment name"), _t("Budget Segment"), seg.m_name, this);
    if (name.IsEmpty())
        return false;

    const long start = wxGetNumberFromUser(
        _t("First day of the segment"), _t("Day"), _t("Budget Segment"),
        seg.m_start_day, 1, 31, this);
    if (start < 1)
        return false;

    const long end = wxGetNumberFromUser(
        _t("Last day of the segment (31 = end of month)"), _t("Day"),
        _t("Budget Segment"), seg.m_end_day, 1, 31, this);
    if (end < 1)
        return false;

    if (end < start) {
        wxMessageBox(
            _t("The last day cannot fall before the first day."),
            _t("Budget Segment"), wxOK | wxICON_WARNING, this);
        return false;
    }

    seg.m_name      = name;
    seg.m_start_day = static_cast<int>(start);
    seg.m_end_day   = static_cast<int>(end);
    return true;
}

void BudgetSegmentDialog::OnAdd(wxCommandEvent& /*event*/)
{
    BudgetSegmentData seg;
    seg.m_period_id = m_bp_id;
    seg.m_start_day = 1;
    seg.m_end_day   = 15;
    seg.m_active    = true;
    seg.m_sort_order = static_cast<int>(m_listBox->GetCount());

    if (!editSegment(seg))
        return;

    BudgetSegmentModel::instance().add_data_n(seg);
    fillControls();
}

void BudgetSegmentDialog::OnEdit(wxCommandEvent& /*event*/)
{
    const int sel = m_listBox->GetSelection();
    if (sel == wxNOT_FOUND)
        return;

    mmListBoxItem* item = static_cast<mmListBoxItem*>(m_listBox->GetClientObject(sel));
    if (!item)
        return;

    const BudgetSegmentData* seg_n =
        BudgetSegmentModel::instance().get_idN_data_n(item->getIndex());
    if (!seg_n)
        return;

    BudgetSegmentData seg = *seg_n;
    if (!editSegment(seg))
        return;

    BudgetSegmentModel::instance().save_data_n(seg);
    fillControls();
}

void BudgetSegmentDialog::OnDelete(wxCommandEvent& /*event*/)
{
    const int sel = m_listBox->GetSelection();
    if (sel == wxNOT_FOUND)
        return;

    mmListBoxItem* item = static_cast<mmListBoxItem*>(m_listBox->GetClientObject(sel));
    if (!item)
        return;

    // Entries bound to this segment are not deleted; they revert to applying
    // to the whole period. Say so, so the choice is informed.
    if (wxMessageBox(
            _t("Delete this segment?\n\n"
               "Budget entries assigned to it will apply to the whole period instead."),
            _t("Budget Segment"), wxYES_NO | wxICON_QUESTION, this) != wxYES)
        return;

    BudgetSegmentModel::instance().purge_id(item->getIndex());
    fillControls();
}

void BudgetSegmentDialog::OnSplitHalves(wxCommandEvent& /*event*/)
{
    if (!BudgetSegmentModel::instance().create_default_halves(m_bp_id)) {
        wxMessageBox(
            _t("This budget period already has segments."),
            _t("Budget Segment"), wxOK | wxICON_INFORMATION, this);
        return;
    }
    fillControls();
}

void BudgetSegmentDialog::OnDoubleClicked(wxCommandEvent& event)
{
    OnEdit(event);
}
