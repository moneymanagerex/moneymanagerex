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

#include "mmpanelbase.h"
#include "model/Model_Budget.h"

class wxListCtrl;
class wxListEvent;
class mmBudgetingPanel;

/* Custom ListCtrl class that implements virtual LC style */
class budgetingListCtrl : public mmListCtrl
{
    DECLARE_NO_COPY_CLASS(budgetingListCtrl)
    wxDECLARE_EVENT_TABLE();

public:
    enum LIST_ID
    {
        LIST_ID_ICON = 0,
        LIST_ID_CATEGORY,
        LIST_ID_FREQUENCY,
        LIST_ID_AMOUNT,
        LIST_ID_ESTIMATED,
        LIST_ID_ACTUAL,
        LIST_ID_NOTES,
        LIST_ID_size, // number of columns
    };

private:
    static const std::vector<ListColumnInfo> LIST_INFO;

public:
    budgetingListCtrl(mmBudgetingPanel* cp, wxWindow *parent, const wxWindowID id);

public:
    /* required overrides for virtual style list control */
    virtual wxString OnGetItemText(long item, long col_nr) const;
    virtual wxListItemAttr *OnGetItemAttr(long item) const;
    virtual int OnGetItemImage(long item) const;

    void OnListItemSelected(wxListEvent& event);
    void OnListItemActivated(wxListEvent& event);

private:
    wxSharedPtr<wxListItemAttr> attr3_; // style3
    mmBudgetingPanel* cp_;
    long selectedIndex_ = -1;
};

class mmBudgetingPanel : public mmPanelBase
{
    wxDECLARE_EVENT_TABLE();

public:
    mmBudgetingPanel(int64 budgetYearID
        , wxWindow *parent
        , wxWindowID winid = wxID_ANY
        , const wxPoint& pos = wxDefaultPosition
        , const wxSize& size = wxDefaultSize
        , long style = wxTAB_TRAVERSAL | wxNO_BORDER
        , const wxString& name = "mmBudgetingPanel");
    ~mmBudgetingPanel();

    /* updates the checking panel data */
    void initVirtualListControl();

    /* Getter for Virtual List Control */
    wxString getItem(long item, int col_id);

    void DisplayBudgetingDetails(int64 budgetYearID);
    int64 GetBudgetYearID()
    {
        return budgetYearID_;
    }
    wxString GetCurrentView()
    {
        return currentView_;
    }
    int GetItemImage(long item) const;
    void OnListItemActivated(int selectedIndex);
    int64 GetTransID(long item)
    {
        return budget_[item].first;
    }

    void RefreshList();

    wxString BuildPage() const { return m_lc->BuildPage(GetPanelTitle()); }

private:
    enum EIcons
    {
        ICON_RECONCILLED,
        ICON_VOID,
        ICON_FOLLOWUP
    };

    std::vector<std::pair<int64, int64> > budget_;
    std::map<int64, std::pair<int, bool > > displayDetails_; //map categid to level of the category, whether category is visible, and whether any subtree is visible 
    std::map<int64, std::pair<double, double> > budgetTotals_;
    std::map<int64, Model_Budget::PERIOD_ID> budgetPeriod_;
    std::map<int64, double> budgetAmt_;
    std::map<int64, wxString> budgetNotes_;
    std::map<int64, std::map<int,double> > categoryStats_;
    bool monthlyBudget_;
    wxSharedPtr<budgetingListCtrl> m_lc;
    wxString currentView_;
    int64 budgetYearID_;
    wxString m_monthName;
    wxString m_budget_offset_date;
    wxStaticText* budgetReportHeading_ = nullptr;
    wxStaticText* income_estimated_ = nullptr;
    wxStaticText* income_actual_ = nullptr;
    wxStaticText* income_diff_ = nullptr;
    wxStaticText* expenses_estimated_ = nullptr;
    wxStaticText* expenses_actual_ = nullptr;
    wxStaticText* expenses_diff_ = nullptr;
    wxButton* m_bitmapTransFilter = nullptr;

    bool Create(wxWindow *parent, wxWindowID winid
        , const wxPoint& pos = wxDefaultPosition
        , const wxSize& size = wxDefaultSize
        , long style = wxTAB_TRAVERSAL | wxNO_BORDER
        , const wxString& name = "mmBudgetingPanel");

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

