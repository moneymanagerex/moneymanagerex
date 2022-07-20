/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)

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
#include "reports/reportbase.h"
#include <wx/spinctrl.h>

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
    void sortTable() {}

    bool saveReportText(bool initial = true);
    mmPrintableBase* getPrintableBase();
    void PrintPage();

    mmGUIFrame *m_frame;

    enum RepPanel
    {
        ID_CHOICE_DATE_RANGE = wxID_HIGHEST + 555,
        ID_CHOICE_ACCOUNTS,
        ID_CHOICE_START_DATE,
        ID_CHOICE_END_DATE,
        ID_CHOICE_YEAR,
        ID_CHOICE_BUDGET,
        ID_CHOICE_CHART,
        ID_CHOICE_FORWARD_MONTHS
    };

private:
    void OnNewWindow(wxWebViewEvent& evt);
    std::vector<wxSharedPtr<mmDateRange>> m_all_date_ranges;
    wxChoice* m_date_ranges;
    mmDatePickerCtrl *m_start_date, *m_end_date;
    wxWebView * browser_;
    mmPrintableBase* rb_;
    wxChoice* m_accounts;
    wxChoice* m_chart;
    wxSpinCtrl *m_forwardMonths;

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

    bool cleanup_;
    bool cleanupmem_;
    int m_shift;
    wxString htmlreport_;

};

inline mmPrintableBase* mmReportsPanel::getPrintableBase() { return rb_; }
#endif

