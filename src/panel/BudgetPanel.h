/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2013 - 2022 Nikolay Akimov
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

#pragma once

#include "model/BudgetModel.h"
#include "_PanelBase.h"
#include "BudgetList.h"

class wxListCtrl;
class wxListEvent;

class BudgetPanel : public PanelBase
{
    wxDECLARE_EVENT_TABLE();

public:
    enum EIcons
    {
        ICON_RECONCILLED,
        ICON_VOID,
        ICON_FOLLOWUP
    };

public:
    static const wxString VIEW_ALL;
    static const wxString VIEW_NON_ZERO;
    static const wxString VIEW_PLANNED;
    static const wxString VIEW_INCOME;
    static const wxString VIEW_EXPENSE;
    static const wxString VIEW_SUMM;

private:
    std::vector<std::pair<int64, int64>> budget_;
    //map categid to level of the category, whether category is visible,
    // and whether any subtree is visible 
    std::map<int64, std::pair<int, bool>> displayDetails_;
    std::map<int64, std::pair<double, double> > budgetTotals_;
    std::map<int64, BudgetFreq> budgetPeriod_;
    std::map<int64, double> budgetAmt_;
    std::map<int64, wxString> budgetNotes_;
    std::map<int64, std::map<int,double> > categoryStats_;
    bool monthlyBudget_;
    wxString currentView_;
    int64 budgetYearID_;
    wxString m_monthName;
    wxString m_budget_offset_date;

    wxSharedPtr<BudgetList> m_lc;
    wxStaticText* budgetReportHeading_ = nullptr;
    wxStaticText* income_estimated_ = nullptr;
    wxStaticText* income_actual_ = nullptr;
    wxStaticText* income_diff_ = nullptr;
    wxStaticText* expenses_estimated_ = nullptr;
    wxStaticText* expenses_actual_ = nullptr;
    wxStaticText* expenses_diff_ = nullptr;
    wxButton* m_bitmapTransFilter = nullptr;

public:
    BudgetPanel(
        int64 budgetYearID,
        wxWindow *parent,
        wxWindowID winid = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxNO_BORDER,
        const wxString& name = "BudgetPanel"
    );
    ~BudgetPanel();

    /* updates the checking panel data */
    void initVirtualListControl();

    /* Getter for Virtual List Control */
    wxString getItem(long item, int col_id);

    void DisplayBudgetingDetails(int64 budgetYearID);
    int64 GetBudgetYearID() { return budgetYearID_; }
    wxString GetCurrentView() { return currentView_; }
    int GetItemImage(long item) const;
    void OnListItemActivated(int selectedIndex);
    int64 GetTransID(long item) { return budget_[item].first; }

    void RefreshList();

    wxString BuildPage() const { return m_lc->BuildPage(GetPanelTitle()); }

private:
    bool Create(
        wxWindow *parent,
        wxWindowID winid,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxNO_BORDER,
        const wxString& name = "BudgetPanel"
    );

    void CreateControls();
    void sortList();
    bool DisplayEntryAllowed(int64 categoryID, int64 subcategoryID);
    void UpdateBudgetHeading();
    double getEstimate(int64 category) const;
    wxString GetPanelTitle() const;

    /* Event handlers for Buttons */
    void OnViewPopupSelected(wxCommandEvent& event);
    void OnMouseLeftDown(wxCommandEvent& event);
};
