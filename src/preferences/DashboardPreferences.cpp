/*******************************************************
Copyright (C) 2021 Mark Whalley (mark@ipx.co.uk)
Copyright (C) 2021, 2022 Nikolay Akimov
Copyright (C) 2025 Klaus Wich

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

#include "constants.h"
#include "util/util.h"

#include "model/PreferencesModel.h"

#include "DashboardPreferences.h"

/*******************************************************/

DashboardPreferences::DashboardPreferences()
{
    m_all_date_ranges.push_back(wxSharedPtr<DateRange>(new mmCurrentMonth()));
    m_all_date_ranges.push_back(wxSharedPtr<DateRange>(new mmCurrentMonthToDate()));
    m_all_date_ranges.push_back(wxSharedPtr<DateRange>(new mmLastMonth()));
    m_all_date_ranges.push_back(wxSharedPtr<DateRange>(new mmLast30Days()));
    m_all_date_ranges.push_back(wxSharedPtr<DateRange>(new mmLast90Days()));
    m_all_date_ranges.push_back(wxSharedPtr<DateRange>(new mmLast3Months()));
    m_all_date_ranges.push_back(wxSharedPtr<DateRange>(new mmLast12Months()));
    m_all_date_ranges.push_back(wxSharedPtr<DateRange>(new mmCurrentYear()));
    m_all_date_ranges.push_back(wxSharedPtr<DateRange>(new mmCurrentYearToDate()));
    m_all_date_ranges.push_back(wxSharedPtr<DateRange>(new mmLastYear()));
    m_all_date_ranges.push_back(wxSharedPtr<DateRange>(new mmLastYearBefore()));
    m_all_date_ranges.push_back(wxSharedPtr<DateRange>(new mmCurrentFinancialYear()));
    m_all_date_ranges.push_back(wxSharedPtr<DateRange>(new mmCurrentFinancialYearToDate()));
    m_all_date_ranges.push_back(wxSharedPtr<DateRange>(new mmLastFinancialYear()));
    m_all_date_ranges.push_back(wxSharedPtr<DateRange>(new mmAllTime()));
    m_all_date_ranges.push_back(wxSharedPtr<DateRange>(new mmLast365Days()));
    // mmLastNDays must be last entry in the list
    m_all_date_ranges.push_back(wxSharedPtr<DateRange>(new mmLastNDays(InfotableModel::instance().getInt("HOMEPAGE_INCEXP_DAYS", 14))));

    int sel_id = PreferencesModel::instance().getHomePageIncExpRange();
    if (sel_id >= static_cast<int>(m_all_date_ranges.size()))
        sel_id = 0;
    m_inc_vs_exp_date_range = m_all_date_ranges[sel_id];

}

DashboardPreferences::DashboardPreferences(wxWindow *parent
    , wxWindowID id
    , const wxPoint &pos
    , const wxSize &size
    , long style, const wxString &name) : DashboardPreferences()
{
    wxPanel::Create(parent, id, pos, size, style, name);
    Create();
}

DashboardPreferences::~DashboardPreferences()
{
}

void DashboardPreferences::Create()
{
    wxBoxSizer* homePanelSizer0 = new wxBoxSizer(wxVERTICAL);
    SetSizer(homePanelSizer0);

    wxScrolledWindow* home_panel = new wxScrolledWindow(this, wxID_ANY);
    wxBoxSizer* homePanelSizer = new wxBoxSizer(wxVERTICAL);
    home_panel->SetSizer(homePanelSizer);
    homePanelSizer0->Add(home_panel, wxSizerFlags(g_flagsExpand).Proportion(0));

    // Income vs Expense
    wxStaticBox* totalsStaticBox = new wxStaticBox(home_panel, wxID_STATIC, _t("Income vs. Expenses"));
    wxStaticBoxSizer* totalsStaticBoxSizer = new wxStaticBoxSizer(totalsStaticBox, wxHORIZONTAL);
    homePanelSizer->Add(totalsStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));
    m_incExpChoice = new wxChoice(totalsStaticBox, wxID_ANY);
    // Show/hide the number text box depending on the choice selection
    m_incExpChoice->Bind(wxEVT_CHOICE, [this](wxCommandEvent& event) {
        nDays_->Show(event.GetInt() == static_cast<int>(m_all_date_ranges.size() - 1));
    });

    for (const auto & date_range : m_all_date_ranges) {
        m_incExpChoice->Append(date_range.get()->local_title());
    }
    m_incExpChoice->SetString(static_cast<int>(m_all_date_ranges.size() - 1), _t("Last N Days"));

    int sel_id = PreferencesModel::instance().getHomePageIncExpRange();
    if (sel_id < 0 || static_cast<size_t>(sel_id) >= m_all_date_ranges.size())
        sel_id = 0;
    m_incExpChoice->SetSelection(sel_id);
    totalsStaticBoxSizer->Add(m_incExpChoice, g_flagsH);

    m_inc_vs_exp_date_range = m_all_date_ranges[sel_id];
    nDays_ = new wxSpinCtrl(totalsStaticBox, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS,
        1, (wxDate::Today() - wxDateTime(1, wxDate::Month::Jan, 1900)).GetDays(),
        InfotableModel::instance().getInt("HOMEPAGE_INCEXP_DAYS", 14));
    nDays_->Bind(wxEVT_SPINCTRL, [this](wxSpinEvent& event) {
        dynamic_cast<mmLastNDays*>(m_all_date_ranges.back().get())->SetRange(nDays_->GetValue());
        event.Skip();
    });
    totalsStaticBoxSizer->Add(nDays_, g_flagsH);

    wxStaticBox* sBox = new wxStaticBox(home_panel, wxID_STATIC, _t("Miscellaneous"));
    wxStaticBoxSizer* trxSizer = new wxStaticBoxSizer(sBox, wxVERTICAL);
    homePanelSizer->Add(trxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));
    m_ignore_future_transactions_home = new wxCheckBox(
    sBox, wxID_ANY,
    _t("Ignore Future Transactions"),
    wxDefaultPosition, wxDefaultSize, wxCHK_2STATE
    );
    m_ignore_future_transactions_home->SetValue(PreferencesModel::instance().getIgnoreFutureTransactionsHomePage());
    trxSizer->Add(m_ignore_future_transactions_home, g_flagsV);

    SetBoldFontToStaticBoxHeader(totalsStaticBox);
    SetBoldFontToStaticBoxHeader(sBox);
    Fit();
    home_panel->SetMinSize(home_panel->GetBestVirtualSize());
    nDays_->Show(sel_id == static_cast<int>(m_all_date_ranges.size() - 1));
    home_panel->SetScrollRate(6, 6);
}


bool DashboardPreferences::SaveSettings()
{
    int sel_id = m_incExpChoice->GetSelection();
    PreferencesModel::instance().setHomePageIncExpRange(sel_id);
    if (sel_id == static_cast<int>(m_all_date_ranges.size() - 1))
        InfotableModel::instance().setInt("HOMEPAGE_INCEXP_DAYS", nDays_->GetValue());
    PreferencesModel::instance().setIgnoreFutureTransactionsHomePage(m_ignore_future_transactions_home->GetValue());
    return true;
}

const wxSharedPtr<DateRange> DashboardPreferences::get_inc_vs_exp_date_range() const
{
    return m_inc_vs_exp_date_range;
}

