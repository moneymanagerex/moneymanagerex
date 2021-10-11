/*******************************************************
Copyright (C) 2021 Mark Whalley (mark@ipx.co.uk)

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
/*******************************************************/


OptionSettingsHome::OptionSettingsHome()
{
    m_all_date_ranges.push_back(new mmCurrentMonth());
    m_all_date_ranges.push_back(new mmCurrentMonthToDate());
    m_all_date_ranges.push_back(new mmLastMonth());
    m_all_date_ranges.push_back(new mmLast30Days());
    m_all_date_ranges.push_back(new mmLast90Days());
    m_all_date_ranges.push_back(new mmLast3Months());
    m_all_date_ranges.push_back(new mmLast12Months());
    m_all_date_ranges.push_back(new mmCurrentYear());
    m_all_date_ranges.push_back(new mmCurrentYearToDate());
    m_all_date_ranges.push_back(new mmLastYear());
    m_all_date_ranges.push_back(new mmCurrentFinancialYear());
    m_all_date_ranges.push_back(new mmCurrentFinancialYearToDate());
    m_all_date_ranges.push_back(new mmLastFinancialYear());
    m_all_date_ranges.push_back(new mmAllTime());
    m_all_date_ranges.push_back(new mmLast365Days());
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
    wxBoxSizer* homePanelSizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(homePanelSizer);

    // Income vs Expense
    wxStaticBox* totalsStaticBox = new wxStaticBox(this, wxID_STATIC, _("Income vs Expenses"));
    SetBoldFont(totalsStaticBox);
    wxStaticBoxSizer* totalsStaticBoxSizer = new wxStaticBoxSizer(totalsStaticBox, wxVERTICAL);
    homePanelSizer->Add(totalsStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));
    m_incExpChoice = new wxChoice(this, wxID_ANY);
    for (const auto & date_range : m_all_date_ranges) {
        m_incExpChoice->Append(date_range->local_title());
    }
    int sel_id = Option::instance().getHomePageIncExpRange();
    if (sel_id < 0 || static_cast<size_t>(sel_id) >= m_all_date_ranges.size())
        sel_id = 0;
    m_incExpChoice->SetSelection(sel_id);
    totalsStaticBoxSizer->Add(m_incExpChoice, g_flagsV);
}


bool OptionSettingsHome::SaveSettings()
{
    int test = m_incExpChoice->GetSelection();
    Option::instance().setHomePageIncExpRange(m_incExpChoice->GetSelection());

    return true;
}
