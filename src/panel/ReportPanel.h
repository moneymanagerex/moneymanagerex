/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)
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

#pragma once

#include "base/defs.h"
#include <wx/spinctrl.h>
#include <wx/timectrl.h>

#include "util/_simple.h"
#include "_PanelBase.h"
#include "JournalPanel.h"
#include "report/_ReportBase.h"

class mmGUIFrame;
class mmDateRange;
class ReportPanel : public PanelBase
{
    wxDECLARE_EVENT_TABLE();

public:
    enum RepPanel
    {
        ID_UNUSED = wxID_HIGHEST + 555,
        ID_ACCOUNT_CHOICE,
        ID_SINGLE_DATE_PICKER,
        ID_START_DATE_PICKER,
        ID_END_DATE_PICKER,
        ID_TIME_PICKER,
        ID_YEAR_CHOICE,
        ID_BUDGET_CHOICE,
        ID_CHART_CHOICE,
        ID_FORWARD_MONTHS,
        ID_DATE_RANGE_BUTTON,
        ID_DATE_RANGE_MIN,
        ID_DATE_RANGE_MAX = ID_DATE_RANGE_MIN + 99,
        ID_DATE_RANGE_EDIT,
    };

private:
    ReportBase* m_rb = nullptr;
    std::vector<mmDateRange2::Range> m_date_range_a = {};
    int m_date_range_m = -1;
    mmDateRange2 m_date_range = mmDateRange2();
    JournalPanel::FILTER_ID m_filter_id;
    bool m_cleanup;
    int m_shift = 0;
    bool m_use_account_specific_filter;
    wxString u_html_report;
    bool u_cleanup_mem = false;

private:
    mmGUIFrame*       w_frame              = nullptr;
    wxWebView*        w_browser            = nullptr;
    wxButton*         w_date_range_button  = nullptr;
    mmDatePickerCtrl* w_single_date_picker = nullptr;
    mmDatePickerCtrl* w_start_date_picker  = nullptr;
    mmDatePickerCtrl* w_end_date_picker    = nullptr;
    wxTimePickerCtrl* w_time_picker        = nullptr;
    wxChoice*         w_year_choice        = nullptr;
    wxSpinCtrl*       w_forward_months     = nullptr;
    wxChoice*         w_account_choice     = nullptr;
    wxChoice*         w_chart_choice       = nullptr;

public:
    ReportPanel(
        ReportBase* rb,
        bool cleanup,
        wxWindow *parent,
        mmGUIFrame *frame,
        wxWindowID winid = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxNO_BORDER,
        const wxString& name = "ReportPanel"
    );
    ~ReportPanel();

public:
    bool Create(
        wxWindow *parent, wxWindowID winid,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxNO_BORDER,
        const wxString& name = "ReportPanel"
    );
    void CreateControls();
    void loadFilterSettings();
    void saveFilterSettings();
    void sortList() {}
    bool saveReportText(bool initial = true);
    ReportBase* getReportBase();
    void PrintPage();

public:
    static void loadDateRanges(std::vector<mmDateRange2::Range>* date_range_a, int* date_range_m, bool all_ranges = false);

private:
    void onNewWindow(wxWebViewEvent& evt);
    void onYearChanged(wxCommandEvent& event);
    void onBudgetChanged(wxCommandEvent & event);
    void onStartEndDateChanged(wxDateEvent& event);
    void onSingleDateChanged(wxDateEvent& event);
    void onAccountChanged(wxCommandEvent& event);
    void onChartChanged(wxCommandEvent& event);
    void onForwardMonthsChangedSpin(wxSpinEvent& event);
    void onForwardMonthsChangedText(wxCommandEvent& event);
    void onShiftPressed(wxCommandEvent& event);
    void onDateRangePopup(wxCommandEvent& event);
    void onDateRangeSelect(wxCommandEvent& event);
    void onDateRangeEdit(wxCommandEvent& event);

    void updateFilter();
};

inline ReportBase* ReportPanel::getReportBase() { return m_rb; }

