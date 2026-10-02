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
#include "model/BudgetSegmentModel.h"
#include "_PanelBase.h"
#include "BudgetList.h"

class wxListCtrl;
class wxListEvent;

class BudgetPanel : public PanelBase
{
    friend class BudgetList;

public:
    static const wxString VIEW_ALL;
    static const wxString VIEW_NON_ZERO;
    static const wxString VIEW_PLANNED;
    static const wxString VIEW_INCOME;
    static const wxString VIEW_EXPENSE;
    static const wxString VIEW_SUMM;

public:
    enum
    {
        ICON_RECONCILLED,
        ICON_VOID,
        ICON_FOLLOWUP
    };

private:
    wxDECLARE_EVENT_TABLE();

private:
    int64 m_bp_id;
    // Which part of the period is on show. -1 means the period as a whole; a
    // segment id narrows the page to that part. A segmented month and each of
    // its parts are separate views of separate entries, so this decides both
    // what is listed and where a new entry is filed.
    int64 m_segment_id = -1;
    // The row a context menu was opened on, so the chosen action knows its
    // subject after the menu has closed.
    long m_context_item = -1;
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
    // Shown only for a year period: rolls its months into the figures without
    // altering either the year's own entries or the months'. A segmented month
    // reuses it to fold its segments in, which is the same idea one level down.
    wxCheckBox*   w_rollup             = nullptr;

public:
    BudgetPanel(
        int64 bp_id,
        int64 segment_id,
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
    void displayBudgetingDetails(int64 budgetYearID, int64 segment_id = -1);

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

    // True when the figures on show total more than this view's own entries,
    // so an edit here would not be editing what the reader is looking at.
    bool isRolledUp() const;
    // The segments of this period, empty when it is not split.
    auto segment_a() const -> std::vector<BudgetSegmentData>;
    // Move a category's entry in this view to another segment, or to the period
    // as a whole. Returns false when nothing was moved.
    bool moveEntryToSegment(int64 cat_id, int64 target_segment_id);

public:
    // Offer the move on a right-click; the list owns the mouse event.
    void showRowContextMenu(long item);

private:
    // Event handlers
    void onViewPopupSelected(wxCommandEvent& event);
    void onMouseLeftDown(wxCommandEvent& event);
    void onRollupChanged(wxCommandEvent& event);
    void onMoveToSegment(wxCommandEvent& event);
};
