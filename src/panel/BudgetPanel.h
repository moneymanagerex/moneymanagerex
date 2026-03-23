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
    friend class BudgetList;

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
    int64 m_bp_id;
    bool m_is_monthly;
    wxString m_month_name;
    wxString m_start_date;
    wxString m_current_view;
    std::vector<std::pair<int64, int64>> m_catId_subcatId_a;

    std::map<int64, std::pair<int, bool>>      m_level_visible_mCatId;
    std::map<int64, std::pair<double, double>> m_estimate_actual_mCatId;
    std::map<int64, BudgetFreq>                m_freq_mCatId;
    std::map<int64, double>                    m_amount_mCatId;
    std::map<int64, wxString>                  m_notes_mCatId;
    std::map<int64, std::map<int, double>>     m_amount_mMonth_mCatId;

    wxSharedPtr<BudgetList> w_list;
    wxStaticText* w_header             = nullptr;
    wxStaticText* w_income_estimated   = nullptr;
    wxStaticText* w_income_actual      = nullptr;
    wxStaticText* w_income_diff        = nullptr;
    wxStaticText* w_expenses_estimated = nullptr;
    wxStaticText* w_expenses_actual    = nullptr;
    wxStaticText* w_expenses_diff      = nullptr;
    wxButton*     w_filter_btn         = nullptr;

public:
    BudgetPanel(
        int64 bp_id,
        wxWindow* parent_win,
        wxWindowID win_id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxNO_BORDER,
        const wxString& name = "BudgetPanel"
    );
    ~BudgetPanel();

public:
    // override PanelBase
    virtual auto buildPage() const -> wxString override {
        return w_list->buildPage(getPanelTitle());
    }
    virtual void sortList() override;

    void refreshList();
    void displayBudgetingDetails(int64 budgetYearID);

private:
    bool create(
        wxWindow* parent_win,
        wxWindowID win_id,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxNO_BORDER,
        const wxString& name = "BudgetPanel"
    );
    void createControls();
    bool displayEntryAllowed(int64 cat_id, int64 subcat_id);
    void updateBudgetHeading();
    auto getEstimate(int64 category) const -> double;
    auto getPanelTitle() const -> wxString;
    void initVirtualListControl();
    auto getItem(long item, int col_id) -> wxString;
    auto getBudgetYearID() -> int64 { return m_bp_id; }
    auto getCurrentView() -> wxString { return m_current_view; }
    int  getItemImage(long item) const;
    void onListItemActivated(int selectedIndex);
    auto getCatId(long item) -> int64 { return m_catId_subcatId_a[item].first; }

    // Event handlers
    void onViewPopupSelected(wxCommandEvent& event);
    void onMouseLeftDown(wxCommandEvent& event);
};
