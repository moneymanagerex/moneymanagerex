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

#ifndef MM_EX_REPORTSPANEL_H_
#define MM_EX_REPORTSPANEL_H_

#include "mmpanelbase.h"
#include "mmSimpleDialogs.h"
#include "mmcheckingpanel.h"
#include "reports/reportbase.h"
#include <wx/spinctrl.h>
#include <wx/timectrl.h>

class mmGUIFrame;
class mmDateRange;
class mmReportsPanel : public mmPanelBase
{
    wxDECLARE_EVENT_TABLE();

public:
    mmReportsPanel(mmPrintableBase* rb,
        bool cleanupReport,
        wxWindow *parent,
        mmGUIFrame *frame,
        wxWindowID winid = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxNO_BORDER,
        const wxString& name = "mmReportsPanel");
    ~mmReportsPanel();

    bool Create(wxWindow *parent, wxWindowID winid,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxNO_BORDER,
        const wxString& name = "mmReportsPanel");

    void CreateControls();
    void loadFilterSettings();
    void saveFilterSettings();
    void sortList() {}

    bool saveReportText(bool initial = true);
    mmPrintableBase* getPrintableBase();
    void PrintPage();

    mmGUIFrame *m_frame = nullptr;

    enum RepPanel
    {
        ID_CHOICE_DATE_RANGE = wxID_HIGHEST + 555,
        ID_CHOICE_ACCOUNTS,
        ID_CHOICE_START_DATE,
        ID_CHOICE_END_DATE,
        ID_CHOICE_TIME,
        ID_CHOICE_YEAR,
        ID_CHOICE_BUDGET,
        ID_CHOICE_CHART,
        ID_CHOICE_FORWARD_MONTHS,
        ID_FILTER_PERIOD,
        ID_FILTER_DATE_MIN,
        ID_FILTER_DATE_MAX = ID_FILTER_DATE_MIN + 99,
        ID_EDIT_DATE_RANGES,
    };

private:
    void OnNewWindow(wxWebViewEvent& evt);
    std::vector<wxSharedPtr<mmDateRange>> m_all_date_ranges;
    std::vector<DateRange2::Spec> m_date_range_a = {};
    wxChoice* m_date_ranges = nullptr;
    mmDatePickerCtrl *m_start_date = nullptr, *m_end_date = nullptr;
    wxTimePickerCtrl *m_time = nullptr;
    wxWebView * browser_ = nullptr;
    mmPrintableBase* rb_ = nullptr;
    wxChoice* m_accounts = nullptr;
    wxChoice* m_chart = nullptr;
    wxSpinCtrl *m_forwardMonths = nullptr;

    wxButton* m_bitmapDataPeriodFilterBtn = nullptr;
    DateRange2 m_current_date_range = DateRange2();
    mmCheckingPanel::FILTER_ID m_filter_id;

private:
    void OnDateRangeChanged(wxCommandEvent& event);
    void OnYearChanged(wxCommandEvent& event);
    void OnBudgetChanged(wxCommandEvent & event);
    void OnStartEndDateChanged(wxDateEvent& event);
    void OnAccountChanged(wxCommandEvent& event);
    void OnChartChanged(wxCommandEvent& event);
    void OnForwardMonthsChangedSpin(wxSpinEvent& event);
    void OnForwardMonthsChangedText(wxCommandEvent& event);
    void OnShiftPressed(wxCommandEvent& event);
    void OnPeriodSelectPopup(wxCommandEvent& event);
    void onFilterDateMenu(wxCommandEvent& event);
    void onEditDateRanges(wxCommandEvent& event);

    void updateFilter();

    bool cleanup_;
    bool cleanupmem_ = false;
    int m_shift = 0;

    // New filtering
    bool m_use_dedicated_filter;
    int m_date_range_m = -1;
    wxString htmlreport_;

};

inline mmPrintableBase* mmReportsPanel::getPrintableBase() { return rb_; }
#endif
