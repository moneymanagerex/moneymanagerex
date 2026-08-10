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

#include <cmath>
#include "base/_defs.h"
#include "base/_constants.h"
#include "util/mmPath.h"
#include "util/_util.h"
#include "manager/CategoryManager.h"
#include "model/BudgetModel.h"
#include "model/BudgetPeriodModel.h"
#include "model/BudgetSegmentModel.h"
#include "model/CategoryModel.h"
#include "model/CurrencyModel.h"
#include "BudgetFromTrxDialog.h"

namespace
{
    // Rounding steps offered, in the order they appear in the control.
    const double STEPS[] = { 1.0, 5.0, 10.0, 50.0, 100.0 };

    enum { ROUND_NONE = 0, ROUND_NEAREST, ROUND_UP, ROUND_DOWN };
}

wxIMPLEMENT_DYNAMIC_CLASS(BudgetFromTrxDialog, wxDialog);

wxBEGIN_EVENT_TABLE(BudgetFromTrxDialog, wxDialog)
    EVT_BUTTON(wxID_OK, BudgetFromTrxDialog::OnOk)
    EVT_BUTTON(ID_CATEGORY, BudgetFromTrxDialog::OnCategory)
    EVT_CHOICE(wxID_ANY, BudgetFromTrxDialog::OnAnyChange)
wxEND_EVENT_TABLE()

BudgetFromTrxDialog::BudgetFromTrxDialog()
{
}

BudgetFromTrxDialog::BudgetFromTrxDialog(wxWindow* parent,
    double amount,
    int64 category_id,
    const wxString& iso_date,
    const wxString& description,
    int64 sched_id,
    BudgetFreq suggested_freq
) :
    m_amount(std::fabs(amount)),
    m_category_id(category_id),
    m_iso_date(iso_date),
    m_description(description),
    m_sched_id(sched_id),
    m_suggested_freq(suggested_freq)
{
    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY, _t("Add to Budget"), wxDefaultPosition, wxSize(460, 460), style);
    mmThemeAutoColour(this);
}

bool BudgetFromTrxDialog::Create(wxWindow* parent, wxWindowID id,
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

void BudgetFromTrxDialog::CreateControls()
{
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(mainSizer);

    wxStaticText* header = new wxStaticText(this, wxID_STATIC,
        wxString::Format(_t("From: %s"), m_description));
    header->Wrap(420);
    mainSizer->Add(header, 0, wxALL, 10);

    wxFlexGridSizer* grid = new wxFlexGridSizer(0, 2, 5, 10);
    grid->AddGrowableCol(1, 1);
    mainSizer->Add(grid, 0, wxGROW | wxLEFT | wxRIGHT, 10);

    grid->Add(new wxStaticText(this, wxID_STATIC, _t("Budget period")), 0, wxALIGN_CENTER_VERTICAL);
    m_period = new wxChoice(this, ID_PERIOD);
    grid->Add(m_period, 1, wxGROW);

    grid->Add(new wxStaticText(this, wxID_STATIC, _t("Segment")), 0, wxALIGN_CENTER_VERTICAL);
    m_segment = new wxChoice(this, wxID_ANY);
    mmToolTip(m_segment, _t(
        "Which part of the period this belongs to. Leave as the whole period "
        "when it is not tied to one half of the month."));
    grid->Add(m_segment, 1, wxGROW);

    grid->Add(new wxStaticText(this, wxID_STATIC, _t("Category")), 0, wxALIGN_CENTER_VERTICAL);
    m_category = new wxButton(this, ID_CATEGORY, _t("Select Category"));
    grid->Add(m_category, 1, wxGROW);

    grid->Add(new wxStaticText(this, wxID_STATIC, _t("Frequency")), 0, wxALIGN_CENTER_VERTICAL);
    m_freq = new wxChoice(this, ID_FREQ);
    for (int i = 0; i < BudgetFreq::size; ++i)
        m_freq->Append(wxGetTranslation(BudgetFreq(i).name()));
    mmToolTip(m_freq, _t("How often this amount is expected within the period"));
    grid->Add(m_freq, 1, wxGROW);

    grid->Add(new wxStaticText(this, wxID_STATIC, _t("Rounding")), 0, wxALIGN_CENTER_VERTICAL);
    wxBoxSizer* roundRow = new wxBoxSizer(wxHORIZONTAL);
    m_rounding = new wxChoice(this, ID_ROUNDING);
    m_rounding->Append(_t("None"));
    m_rounding->Append(_t("Nearest"));
    m_rounding->Append(_t("Round up"));
    m_rounding->Append(_t("Round down"));
    roundRow->Add(m_rounding, 1, wxGROW);
    m_step = new wxChoice(this, ID_STEP);
    for (const double s : STEPS)
        m_step->Append(wxString::FromDouble(s, 0));
    roundRow->Add(m_step, 0, wxLEFT, 5);
    mmToolTip(m_rounding, _t(
        "A budget is a plan, not a receipt, so a rounded figure is often more "
        "useful than the exact amount."));
    grid->Add(roundRow, 1, wxGROW);

    grid->Add(new wxStaticText(this, wxID_STATIC, _t("Amount")), 0, wxALIGN_CENTER_VERTICAL);
    m_amount_type = new wxChoice(this, ID_AMOUNT_TYPE);
    for (int i = 0; i < BudgetAmountType::size; ++i)
        m_amount_type->Append(wxGetTranslation(BudgetAmountType(i).name()));
    mmToolTip(m_amount_type, _t(
        "Fixed stores the number as it is. Estimated marks it as a guess. "
        "Automatic keeps it tied to the source, so the budget follows the "
        "scheduled transaction instead of copying it once."));
    grid->Add(m_amount_type, 1, wxGROW);

    grid->Add(new wxStaticText(this, wxID_STATIC, _t("If already budgeted")), 0, wxALIGN_CENTER_VERTICAL);
    m_existing = new wxChoice(this, wxID_ANY);
    m_existing->Append(_t("Replace the amount"));
    m_existing->Append(_t("Add to the amount"));
    m_existing->Append(_t("Leave it unchanged"));
    grid->Add(m_existing, 1, wxGROW);

    grid->Add(new wxStaticText(this, wxID_STATIC, _t("Notes")), 0, wxALIGN_TOP);
    m_notes = new mmTextCtrl(this, wxID_ANY, "",
        wxDefaultPosition, wxSize(240, 50), wxTE_MULTILINE);
    grid->Add(m_notes, 1, wxGROW);

    m_preview = new wxStaticText(this, wxID_STATIC, "");
    m_preview->Wrap(420);
    mainSizer->Add(m_preview, 0, wxALL, 10);

    wxPanel* btnPanel = new wxPanel(this, wxID_ANY);
    mainSizer->Add(btnPanel, 0, wxALIGN_RIGHT, 5);
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    btnPanel->SetSizer(btnSizer);
    wxButton* btnOK = new wxButton(btnPanel, wxID_OK, _t("&OK "));
    btnSizer->Add(btnOK, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT | wxBOTTOM, 5);
    wxButton* btnCancel = new wxButton(btnPanel, wxID_CANCEL, wxGetTranslation(g_CancelLabel));
    btnSizer->Add(btnCancel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT | wxBOTTOM, 5);
}

void BudgetFromTrxDialog::fillControls()
{
    BudgetPeriodModel& bpm = BudgetPeriodModel::instance();

    // Offer the period the transaction actually falls in first, since that is
    // almost always the one meant.
    const wxString year  = m_iso_date.Left(4);
    const wxString month = m_iso_date.Left(7);

    int select = 0;
    for (const auto& bp_d : bpm.find_data_a(
        TableClause::ORDERBY(BudgetPeriodCol::NAME_BUDGETYEARNAME)
    )) {
        m_period->Append(bp_d.m_name);
        m_period_id_a.push_back(bp_d.m_id);

        if (bp_d.m_name == month)
            select = static_cast<int>(m_period_id_a.size()) - 1;
        else if (bp_d.m_name == year && select == 0)
            select = static_cast<int>(m_period_id_a.size()) - 1;
    }
    if (!m_period_id_a.empty())
        m_period->SetSelection(select);

    updateSegments();

    if (m_category_id > 0)
        m_category->SetLabel(CategoryModel::instance().get_id_fullname(m_category_id, ":"));

    m_freq->SetSelection(m_suggested_freq.id() != BudgetFreq::e_none
        ? m_suggested_freq.id() : BudgetFreq::e_monthly);

    m_rounding->SetSelection(ROUND_NONE);
    m_step->SetSelection(0);

    // A scheduled transaction can be tracked rather than copied, which keeps
    // the budget correct when the schedule changes.
    m_amount_type->SetSelection(m_sched_id > 0
        ? BudgetAmountType::e_auto : BudgetAmountType::e_fixed);

    m_existing->SetSelection(0);
    m_notes->SetValue(m_description);

    updatePreview();
}

void BudgetFromTrxDialog::updateSegments()
{
    m_segment->Clear();
    m_segment_id_a.clear();

    m_segment->Append(_t("Whole period"));
    m_segment_id_a.push_back(-1);

    const int pi = m_period->GetSelection();
    if (pi >= 0 && static_cast<std::size_t>(pi) < m_period_id_a.size()) {
        for (const auto& seg_d :
            BudgetSegmentModel::instance().find_period_a(m_period_id_a[pi])
        ) {
            m_segment->Append(wxString::Format("%s (%d-%d)",
                seg_d.m_name, seg_d.m_start_day, seg_d.m_end_day));
            m_segment_id_a.push_back(seg_d.m_id);
        }
    }

    m_segment->SetSelection(0);
    m_segment->Enable(m_segment_id_a.size() > 1);
}

double BudgetFromTrxDialog::roundedAmount() const
{
    const int mode = m_rounding->GetSelection();
    if (mode == ROUND_NONE)
        return m_amount;

    const int si = m_step->GetSelection();
    const double step = (si >= 0 && si < static_cast<int>(WXSIZEOF(STEPS)))
        ? STEPS[si] : 1.0;
    if (step <= 0.0)
        return m_amount;

    switch (mode) {
    case ROUND_UP:   return std::ceil(m_amount / step) * step;
    case ROUND_DOWN: return std::floor(m_amount / step) * step;
    default:         return std::round(m_amount / step) * step;
    }
}

void BudgetFromTrxDialog::updatePreview()
{
    CurrencyModel& cm = CurrencyModel::instance();
    const double rounded = roundedAmount();

    wxString text = wxString::Format(_t("Actual %s will be budgeted as %s %s."),
        cm.toCurrency(m_amount),
        cm.toCurrency(rounded),
        wxGetTranslation(BudgetFreq(m_freq->GetSelection()).name()).Lower());

    if (m_amount_type->GetSelection() == BudgetAmountType::e_auto) {
        text += " " + (m_sched_id > 0
            ? _t("The amount will follow the scheduled transaction.")
            : _t("Automatic needs a source to follow; without one the amount "
                 "stays as shown."));
    }

    m_preview->SetLabel(text);
    m_preview->Wrap(420);
    Layout();
}

void BudgetFromTrxDialog::OnAnyChange(wxCommandEvent& event)
{
    if (event.GetId() == ID_PERIOD)
        updateSegments();
    updatePreview();
    event.Skip();
}

void BudgetFromTrxDialog::OnCategory(wxCommandEvent& /*event*/)
{
    CategoryManager dlg(this, true, m_category_id);
    if (dlg.ShowModal() != wxID_OK)
        return;

    m_category_id = dlg.getCategId();
    m_category->SetLabel(m_category_id > 0
        ? CategoryModel::instance().get_id_fullname(m_category_id, ":")
        : _t("Select Category"));
}

void BudgetFromTrxDialog::OnOk(wxCommandEvent& /*event*/)
{
    if (m_period_id_a.empty()) {
        wxMessageBox(_t("Create a budget period first."),
            _t("Add to Budget"), wxOK | wxICON_WARNING, this);
        return;
    }
    if (m_category_id <= 0) {
        wxMessageBox(_t("Choose a category for the budget entry."),
            _t("Add to Budget"), wxOK | wxICON_WARNING, this);
        return;
    }

    const int pi = m_period->GetSelection();
    const int64 bp_id = m_period_id_a[pi];

    const int si = m_segment->GetSelection();
    const int64 seg_id = (si >= 0 && static_cast<std::size_t>(si) < m_segment_id_a.size())
        ? m_segment_id_a[si] : -1;

    BudgetModel& bm = BudgetModel::instance();

    // An entry already exists for this category when the same thing has been
    // budgeted before; overwriting silently would lose the earlier figure.
    BudgetModel::DataA existing = bm.find_data_a(
        BudgetCol::WHERE_BUDGETYEARID(OP_EQ, bp_id),
        BudgetCol::WHERE_CATEGID(OP_EQ, m_category_id)
    );

    BudgetModel::DataA matching;
    for (const auto& b : existing) {
        if (b.m_segment_id == seg_id)
            matching.push_back(b);
    }

    const double amount = roundedAmount();

    if (!matching.empty()) {
        const int action = m_existing->GetSelection();
        if (action == 2) {
            EndModal(wxID_CANCEL);
            return;
        }

        BudgetData data = matching[0];
        data.m_amount = (action == 1) ? data.m_amount + amount : amount;
        data.m_freq   = BudgetFreq(m_freq->GetSelection());
        data.m_amount_type = BudgetAmountType(m_amount_type->GetSelection());
        if (data.m_amount_type.is_derived() && m_sched_id > 0)
            data.m_auto_source = wxString::Format("SCHED:%lld", m_sched_id.GetValue());
        if (!m_notes->GetValue().IsEmpty())
            data.m_notes = m_notes->GetValue();

        bm.save_data_n(data);
        EndModal(wxID_OK);
        return;
    }

    BudgetData data;
    data.m_period_id   = bp_id;
    data.m_category_id = m_category_id;
    data.m_segment_id  = seg_id;
    data.m_freq        = BudgetFreq(m_freq->GetSelection());
    data.m_amount      = amount;
    data.m_amount_type = BudgetAmountType(m_amount_type->GetSelection());
    data.m_notes       = m_notes->GetValue();
    data.m_active      = true;

    if (data.m_amount_type.is_derived() && m_sched_id > 0)
        data.m_auto_source = wxString::Format("SCHED:%lld", m_sched_id.GetValue());

    bm.add_data_n(data);
    EndModal(wxID_OK);
}
