/*******************************************************
Copyright (C) 2021 Mark Whalley (mark@ipx.co.uk)
 Copyright (C) 2021 Nikolay Akimov

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

#include "optionsettingsbase.h"
#include "reports/mmDateRange.h"
#include "wx/spinctrl.h"

class OptionSettingsHome : public OptionSettingsBase
{
//    wxDECLARE_EVENT_TABLE();

public:
    OptionSettingsHome();

    OptionSettingsHome(wxWindow *parent
        , wxWindowID id = wxID_ANY
        , const wxPoint &pos = wxDefaultPosition
        , const wxSize &size = wxDefaultSize
        , long style = wxTAB_TRAVERSAL
        , const wxString &name = wxPanelNameStr);

    ~OptionSettingsHome();

public:
    virtual bool SaveSettings();
    const wxSharedPtr<mmDateRange> get_inc_vs_exp_date_range() const;

private:
    void Create();
    wxSharedPtr<mmDateRange> m_inc_vs_exp_date_range;
    std::vector<wxSharedPtr<mmDateRange>> m_all_date_ranges;

private:
    wxChoice* m_incExpChoice = nullptr;
    wxSpinCtrl* nDays_ = nullptr;

};
