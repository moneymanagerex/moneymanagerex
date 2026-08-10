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
#include "data/_DataEnum.h"
#include "data/_Repeat.h"

// Turns a real transaction - one that happened, or one that is scheduled - into
// a budget entry, so a figure that is already known does not have to be typed
// into the budget by hand.
//
// The amount comes from the transaction. What the dialog asks for is everything
// the transaction cannot say by itself: which period it belongs in, how often it
// is expected to recur, whether to round it off, and whether the budget should
// keep tracking the source or freeze the value as typed.
class BudgetFromTrxDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(BudgetFromTrxDialog);
    wxDECLARE_EVENT_TABLE();

public:
    BudgetFromTrxDialog();

    // sched_id > 0 marks the source as a scheduled transaction, which allows
    // the budget entry to keep following it rather than copying a number once.
    BudgetFromTrxDialog(wxWindow* parent,
        double amount,
        int64 category_id,
        const wxString& iso_date,
        const wxString& description,
        int64 sched_id = -1,
        BudgetFreq suggested_freq = BudgetFreq());

    bool Create(wxWindow* parent, wxWindowID id,
        const wxString& caption,
        const wxPoint& pos,
        const wxSize& size,
        long style);

private:
    void CreateControls();
    void fillControls();
    void OnOk(wxCommandEvent& event);
    void OnCategory(wxCommandEvent& event);
    void OnAnyChange(wxCommandEvent& event);
    void updatePreview();
    void updateSegments();

    // The amount after rounding, which is what will actually be stored.
    double roundedAmount() const;

    double   m_amount = 0.0;
    int64    m_category_id = -1;
    wxString m_iso_date;
    wxString m_description;
    int64    m_sched_id = -1;
    BudgetFreq m_suggested_freq;

    std::vector<int64> m_period_id_a;
    std::vector<int64> m_segment_id_a;

    wxChoice*   m_period  = nullptr;
    wxChoice*   m_segment = nullptr;
    wxButton*   m_category = nullptr;
    wxChoice*   m_freq    = nullptr;
    wxChoice*   m_rounding = nullptr;
    wxChoice*   m_step    = nullptr;
    wxChoice*   m_amount_type = nullptr;
    wxChoice*   m_existing = nullptr;
    mmTextCtrl* m_notes   = nullptr;
    wxStaticText* m_preview = nullptr;

    enum {
        ID_CATEGORY = wxID_HIGHEST + 600,
        ID_PERIOD,
        ID_ROUNDING,
        ID_STEP,
        ID_FREQ,
        ID_AMOUNT_TYPE
    };
};

// A schedule already says how often it recurs, so the budget frequency can be
// suggested rather than asked for. Anything without a clean equivalent (an
// x-day cycle, a one-off) is left unset for the user to decide.
inline BudgetFreq budget_freq_from_repeat(const Repeat& repeat)
{
    switch (repeat.m_freq.id()) {
    case RepeatFreq::e_1_day:                   return BudgetFreq(BudgetFreq::e_daily);
    case RepeatFreq::e_1_week:                  return BudgetFreq(BudgetFreq::e_weekly);
    case RepeatFreq::e_2_weeks:                 return BudgetFreq(BudgetFreq::e_biweekly);
    case RepeatFreq::e_1_month:                 return BudgetFreq(BudgetFreq::e_monthly);
    case RepeatFreq::e_month_last_day:          return BudgetFreq(BudgetFreq::e_monthly);
    case RepeatFreq::e_month_last_business_day: return BudgetFreq(BudgetFreq::e_monthly);
    case RepeatFreq::e_2_months:                return BudgetFreq(BudgetFreq::e_bimonthly);
    case RepeatFreq::e_3_months:                return BudgetFreq(BudgetFreq::e_quarterly);
    case RepeatFreq::e_6_months:                return BudgetFreq(BudgetFreq::e_halfyearly);
    case RepeatFreq::e_1_year:                  return BudgetFreq(BudgetFreq::e_yearly);
    default:                                    return BudgetFreq();
    }
}
