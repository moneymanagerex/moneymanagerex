/*******************************************************
Copyright (C) 2021 Mark Whalley (mark@ipx.co.uk)
Copyright (C) 2021, 2022 Nikolay Akimov

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

#include "optionsettingshome.h"
#include "constants.h"
#include "option.h"
#include "util.h"

/*******************************************************/

OptionSettingsHome::OptionSettingsHome()
{
    m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmCurrentMonth()));
    m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmCurrentMonthToDate()));
    m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmLastMonth()));
    m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmLast30Days()));
    m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmLast90Days()));
    m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmLast3Months()));
    m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmLast12Months()));
    m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmCurrentYear()));
    m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmCurrentYearToDate()));
    m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmLastYear()));
    m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmLastYearBefore()));
    m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmCurrentFinancialYear()));
    m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmCurrentFinancialYearToDate()));
    m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmLastFinancialYear()));
    m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmAllTime()));
    m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmLast365Days()));
    m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmLastNDays(Model_Infotable::instance().getInt("HOMEPAGE_INCEXP_DAYS", 14))));

    int sel_id = Option::instance().getHomePageIncExpRange();
    if (sel_id >= static_cast<int>(m_all_date_ranges.size()))
        sel_id = 0;
    m_inc_vs_exp_date_range = m_all_date_ranges[sel_id];

}

OptionSettingsHome::OptionSettingsHome(wxWindow *parent
    , wxWindowID id
    , const wxPoint &pos
    , const wxSize &size
    , long style, const wxString &name) : OptionSettingsHome()
{
    wxPanel::Create(parent, id, pos, size, style, name);
    Create();
}

OptionSettingsHome::~OptionSettingsHome()
{
}

void OptionSettingsHome::Create()
{
    wxBoxSizer* homePanelSizer0 = new wxBoxSizer(wxVERTICAL);
    SetSizer(homePanelSizer0);

    wxScrolledWindow* home_panel = new wxScrolledWindow(this, wxID_ANY);
    wxBoxSizer* homePanelSizer = new wxBoxSizer(wxVERTICAL);
    home_panel->SetSizer(homePanelSizer);
    homePanelSizer0->Add(home_panel, wxSizerFlags(g_flagsExpand).Proportion(0));

    // Income vs Expense
    wxStaticBox* totalsStaticBox = new wxStaticBox(home_panel, wxID_STATIC, _t("Income vs. Expenses"));
    SetBoldFont(totalsStaticBox);
    wxStaticBoxSizer* totalsStaticBoxSizer = new wxStaticBoxSizer(totalsStaticBox, wxHORIZONTAL);
    homePanelSizer->Add(totalsStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));
    m_incExpChoice = new wxChoice(home_panel, wxID_ANY);
    // Show/hide the number text box depending on the choice selection
    m_incExpChoice->Bind(wxEVT_CHOICE, [this](wxCommandEvent& event) {
        nDays_->Show(event.GetInt() == 15);
    });

    for (const auto & date_range : m_all_date_ranges) {
        m_incExpChoice->Append(date_range.get()->local_title());
    }
    m_incExpChoice->SetString(15, _t("Last N Days"));

    int sel_id = Option::instance().getHomePageIncExpRange();
    if (sel_id < 0 || static_cast<size_t>(sel_id) >= m_all_date_ranges.size())
        sel_id = 0;
    m_incExpChoice->SetSelection(sel_id);
    totalsStaticBoxSizer->Add(m_incExpChoice, g_flagsH);

    m_inc_vs_exp_date_range = m_all_date_ranges[sel_id];
    nDays_ = new wxSpinCtrl(home_panel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS,
        1, (wxDate::Today() - wxDateTime(1, wxDate::Month::Jan, 1900)).GetDays(),
        Model_Infotable::instance().getInt("HOMEPAGE_INCEXP_DAYS", 14));
    nDays_->Bind(wxEVT_SPINCTRL, [this](wxSpinEvent& event) {
        dynamic_cast<mmLastNDays*>(m_all_date_ranges.back().get())->SetRange(nDays_->GetValue());
        event.Skip();
    });
    totalsStaticBoxSizer->Add(nDays_, g_flagsH);
    Fit();
    home_panel->SetMinSize(home_panel->GetBestVirtualSize());
    nDays_->Show(sel_id == 15);
    home_panel->SetScrollRate(6, 6);
}


bool OptionSettingsHome::SaveSettings()
{
    int sel_id = m_incExpChoice->GetSelection();
    Option::instance().setHomePageIncExpRange(sel_id);
    if (sel_id == 15)
        Model_Infotable::instance().setInt("HOMEPAGE_INCEXP_DAYS", nDays_->GetValue());
    return true;
}

const wxSharedPtr<mmDateRange> OptionSettingsHome::get_inc_vs_exp_date_range() const
{
    return m_inc_vs_exp_date_range;
}
