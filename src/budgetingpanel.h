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
class mmGUIFrame;

/* Custom ListCtrl class that implements virtual LC style */
class budgetingListCtrl : public mmListCtrl
{
    DECLARE_NO_COPY_CLASS(budgetingListCtrl)
    wxDECLARE_EVENT_TABLE();

public:
    budgetingListCtrl(mmBudgetingPanel* cp, wxWindow *parent, const wxWindowID id);

public:
    /* required overrides for virtual style list control */
    virtual wxString OnGetItemText(long item, long column) const;
    virtual wxListItemAttr *OnGetItemAttr(long item) const;
    virtual int OnGetItemImage(long item) const;

    void OnListItemSelected(wxListEvent& event);
    void OnListItemActivated(wxListEvent& event);

private:
    wxSharedPtr<wxListItemAttr> attr3_; // style3
    mmBudgetingPanel* cp_;
    long selectedIndex_;
};

class mmBudgetingPanel : public mmPanelBase
{
    wxDECLARE_EVENT_TABLE();

public:
    mmBudgetingPanel(int budgetYearID
        , wxWindow *parent, mmGUIFrame *frame
        , wxWindowID winid = wxID_ANY
        , const wxPoint& pos = wxDefaultPosition
        , const wxSize& size = wxDefaultSize
        , long style = wxTAB_TRAVERSAL | wxNO_BORDER
        , const wxString& name = "mmBudgetingPanel");
    ~mmBudgetingPanel();

    /* updates the checking panel data */
    void initVirtualListControl();
    int col_max() { return COL_MAX; }

    /* Getter for Virtual List Control */
    wxString getItem(long item, long column);

    void DisplayBudgetingDetails(int budgetYearID);
    int GetBudgetYearID()
    {
        return budgetYearID_;
    }
    wxString GetCurrentView()
    {
        return currentView_;
    }
    int GetItemImage(long item) const;
    void OnListItemActivated(int selectedIndex);
    int GetTransID(long item)
    {
        return budget_[item].first;
    }

    void RefreshList();

    wxString BuildPage() const { return listCtrlBudget_->BuildPage(GetPanelTitle()); }

private:
    enum EIcons //m_imageList
    {
        ICON_RECONCILLED,
        ICON_VOID,
        ICON_FOLLOWUP
    };

    mmGUIFrame* m_frame;
    std::vector<std::pair<int, int> > budget_;
    std::map<int, std::pair<double, double> > budgetTotals_;
    std::map<int, std::map<int, Model_Budget::PERIOD_ENUM> > budgetPeriod_;
    std::map<int, std::map<int, double> > budgetAmt_;
    std::map<int, std::map<int, wxString> > budgetNotes_;
    std::map<int, std::map<int, std::map<int, double> > > categoryStats_;
    bool monthlyBudget_;
    wxSharedPtr<budgetingListCtrl> listCtrlBudget_;
    wxString currentView_;
    int budgetYearID_;
    wxString m_monthName;
    wxString m_budget_offset_date;
    wxSharedPtr<wxImageList> m_imageList;
    wxStaticText* budgetReportHeading_;
    wxStaticText* income_estimated_;
    wxStaticText* income_actual_;
    wxStaticText* income_diff_;
    wxStaticText* expenses_estimated_;
    wxStaticText* expenses_actual_;
    wxStaticText* expenses_diff_;
    wxButton* m_bitmapTransFilter;

    bool Create(wxWindow *parent, wxWindowID winid
        , const wxPoint& pos = wxDefaultPosition
        , const wxSize& size = wxDefaultSize
        , long style = wxTAB_TRAVERSAL | wxNO_BORDER
        , const wxString& name = "mmBudgetingPanel");

    void CreateControls();
    void sortTable();
    bool DisplayEntryAllowed(int categoryID, int subcategoryID);
    void UpdateBudgetHeading();
    double getEstimate(int category, int subcategory) const;
    wxString GetPanelTitle() const;

    /* Event handlers for Buttons */
    void OnViewPopupSelected(wxCommandEvent& event);
    void OnMouseLeftDown(wxCommandEvent& event);

    enum EColumn
    {
        COL_ICON = 0,
        COL_CATEGORY,
        COL_SUBCATEGORY,
        COL_FREQUENCY,
        COL_AMOUNT,
        COL_ESTIMATED,
        COL_ACTUAL,
        COL_NOTES,
        COL_MAX, // number of columns
    };
};

