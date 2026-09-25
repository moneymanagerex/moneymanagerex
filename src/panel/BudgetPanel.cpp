/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2012 Stefano Giorgio
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

#include "BudgetPanel.h"

#include "base/_constants.h"
#include "util/mmImage.h"
#include "util/mmDateRange.h"
#include "model/_all.h"
#include "model/PrefModel.h"
#include "dialog/BudgetEntryDialog.h"
#include "report/budget.h"

enum
{
    ID_DIALOG_BUDGETENTRY_SUMMARY_INCOME_EST = wxID_HIGHEST + 1400,
    MENU_VIEW_ALLBUDGETENTRIES,
    MENU_VIEW_PLANNEDBUDGETENTRIES,
    MENU_VIEW_NONZEROBUDGETENTRIES,
    MENU_VIEW_INCOMEBUDGETENTRIES,
    MENU_VIEW_SUMMARYBUDGETENTRIES,
    MENU_VIEW_EXPENSEBUDGETENTRIES,
    ID_PANEL_REPORTS_HEADER_PANEL,
    ID_DIALOG_BUDGETENTRY_SUMMARY_INCOME_ACT,
    ID_DIALOG_BUDGETENTRY_SUMMARY_INCOME_DIF,
    ID_DIALOG_BUDGETENTRY_SUMMARY_EXPENSES_EST,
    ID_DIALOG_BUDGETENTRY_SUMMARY_EXPENSES_ACT,
    ID_DIALOG_BUDGETENTRY_SUMMARY_EXPENSES_DIF,
    ID_BUDGET_ROLLUP,
    // Ids for the "move to" targets: the period as a whole, then one per
    // segment. Reserved as a block so the event table can claim a fixed range
    // ahead of the catch-all below, which would otherwise assert on them.
    MENU_MOVE_TO_FIRST,
    MENU_MOVE_TO_LAST = MENU_MOVE_TO_FIRST + 64,
};

const wxString BudgetPanel::VIEW_ALL      = _n("View All Budget Categories");
const wxString BudgetPanel::VIEW_NON_ZERO = _n("View Non-Zero Budget Categories");
const wxString BudgetPanel::VIEW_PLANNED  = _n("View Planned Budget Categories");
const wxString BudgetPanel::VIEW_INCOME   = _n("View Income Budget Categories");
const wxString BudgetPanel::VIEW_EXPENSE  = _n("View Expense Budget Categories");
const wxString BudgetPanel::VIEW_SUMM     = _n("View Budget Category Summary");

wxBEGIN_EVENT_TABLE(BudgetPanel, wxPanel)
    EVT_BUTTON(wxID_FILE2, BudgetPanel::onMouseLeftDown)
    EVT_CHECKBOX(ID_BUDGET_ROLLUP, BudgetPanel::onRollupChanged)
    // Must precede the catch-all: the view filter handler asserts on any id it
    // does not recognise.
    EVT_MENU_RANGE(MENU_MOVE_TO_FIRST, MENU_MOVE_TO_LAST, BudgetPanel::onMoveToSegment)
    EVT_MENU(wxID_ANY,     BudgetPanel::onViewPopupSelected)
    wxEND_EVENT_TABLE()

BudgetPanel::BudgetPanel(
    int64 bp_id,
    int64 segment_id,
    wxWindow* parent_win,
    wxWindowID win_id,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name
) :
    m_bp_id(bp_id),
    m_segment_id(segment_id),
    w_list(nullptr)
{
    create(parent_win, win_id, pos, size, style, name);
    mmThemeAutoColour(this);
}

BudgetPanel::~BudgetPanel()
{
}

bool BudgetPanel::create(
    wxWindow *parent_win,
    wxWindowID win_id,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name
) {
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create(parent_win, win_id, pos, size, style, name);

    this->windowsFreezeThaw();
    createControls();
    mmThemeAutoColour(this);
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    initVirtualListControl();
    if (!m_catId_subcatId_a.empty())
        w_list->EnsureVisible(0);

    this->windowsFreezeThaw();
    UsageModel::instance().pageview(this);
    return true;
}

void BudgetPanel::onViewPopupSelected(wxCommandEvent& event)
{
    int evt =  event.GetId();
    if (evt ==  MENU_VIEW_ALLBUDGETENTRIES)
        m_current_view = BudgetPanel::VIEW_ALL;
    else if (evt == MENU_VIEW_NONZEROBUDGETENTRIES)
        m_current_view = BudgetPanel::VIEW_NON_ZERO;
    else if (evt == MENU_VIEW_PLANNEDBUDGETENTRIES)
        m_current_view = BudgetPanel::VIEW_PLANNED;
    else if (evt == MENU_VIEW_INCOMEBUDGETENTRIES)
        m_current_view = BudgetPanel::VIEW_INCOME;
    else if (evt == MENU_VIEW_EXPENSEBUDGETENTRIES)
        m_current_view = BudgetPanel::VIEW_EXPENSE;
    else if (evt == MENU_VIEW_SUMMARYBUDGETENTRIES)
        m_current_view = BudgetPanel::VIEW_SUMM;
    else {
        wxASSERT(false);
    }

    InfoModel::instance().saveString("BUDGET_FILTER", m_current_view);

    refreshList();
}

void BudgetPanel::refreshList()
{
    initVirtualListControl();
    w_list->Refresh();
    w_list->Update();
    if (!m_catId_subcatId_a.empty())
        w_list->EnsureVisible(0);
}

void BudgetPanel::onRollupChanged(wxCommandEvent& /*event*/)
{
    // Purely a way of reading the same data; nothing is written, so this only
    // has to recompute and redraw.
    refreshList();
}

void BudgetPanel::onMouseLeftDown(wxCommandEvent& event)
{
    wxMenu menu;
    menu.Append(MENU_VIEW_ALLBUDGETENTRIES, wxGetTranslation(BudgetPanel::VIEW_ALL));
    menu.Append(MENU_VIEW_PLANNEDBUDGETENTRIES, wxGetTranslation(BudgetPanel::VIEW_PLANNED));
    menu.Append(MENU_VIEW_NONZEROBUDGETENTRIES, wxGetTranslation(BudgetPanel::VIEW_NON_ZERO));
    menu.Append(MENU_VIEW_INCOMEBUDGETENTRIES, wxGetTranslation(BudgetPanel::VIEW_INCOME));
    menu.Append(MENU_VIEW_EXPENSEBUDGETENTRIES, wxGetTranslation(BudgetPanel::VIEW_EXPENSE));
    menu.AppendSeparator();
    menu.Append(MENU_VIEW_SUMMARYBUDGETENTRIES, wxGetTranslation(BudgetPanel::VIEW_SUMM));
    PopupMenu(&menu);

    event.Skip();
}

wxString BudgetPanel::getPanelTitle() const
{
    wxString bp_name_n = BudgetPeriodModel::instance().get_id_name_n(m_bp_id);
    wxString title;
    if ((bp_name_n.length() < 5)) {
        if (PrefModel::instance().getBudgetFinancialYears()) {
            long year;
            bp_name_n.ToLong(&year);
            year++;
            title = wxString::Format(_t("Financial Year: %s - %li"), bp_name_n, year);
        }
        else {
            title = wxString::Format(_t("Year: %s"), bp_name_n);
        }
    }
    else {
        title = wxString::Format(_t("Month: %s"), bp_name_n);
        title += wxString::Format(" (%s)", m_month_name);
    }

    if (PrefModel::instance().getBudgetDaysOffset() != 0) {
        title = wxString::Format(_t("%1$s    Start Date of: %2$s"),
            title,
            mmGetDateTimeForDisplay(m_start_date)
        );
    }

    title = wxString::Format(_t("Budget Planner for %s"), title);

    // Naming the part on show is what keeps a segmented month from looking like
    // three copies of the same page. Kept to ASCII: a non-ASCII literal here is
    // read as Latin-1 by MSVC and renders as mojibake.
    if (m_segment_id > 0) {
        const BudgetSegmentData* seg_n =
            BudgetSegmentModel::instance().get_idN_data_n(m_segment_id);
        if (seg_n) {
            title += wxString::Format(_t("  -  %s (days %d-%d)"),
                seg_n->m_name, seg_n->m_start_day, seg_n->m_end_day);
        }
    }
    else if (!segment_a().empty()) {
        title += _t("  -  whole period");
    }

    return title;
}

std::vector<BudgetSegmentData> BudgetPanel::segment_a() const
{
    return BudgetSegmentModel::instance().find_period_a(m_bp_id);
}

bool BudgetPanel::isRolledUp() const
{
    return w_rollup && w_rollup->IsShown() && w_rollup->GetValue();
}

void BudgetPanel::updateBudgetHeading()
{
    w_header->SetLabel(getPanelTitle());
    w_filter_btn->SetLabel(wxGetTranslation(m_current_view));
}

void BudgetPanel::createControls()
{
    wxSizerFlags flags;
    flags.Align(wxALIGN_LEFT).Border(wxLEFT|wxTOP, 4);

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer2);

    wxPanel* itemPanel3 = new wxPanel(this, ID_PANEL_REPORTS_HEADER_PANEL
        , wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    itemBoxSizer2->Add(itemPanel3, flags);

    wxBoxSizer* itemBoxSizerVHeader = new wxBoxSizer(wxVERTICAL);
    itemPanel3->SetSizer(itemBoxSizerVHeader);

    w_header = new wxStaticText(itemPanel3, wxID_ANY, "");

    w_header->SetFont(this->GetFont().Larger().Bold());

    wxBoxSizer* budgetReportHeadingSizer = new wxBoxSizer(wxHORIZONTAL);
    budgetReportHeadingSizer->Add(w_header, 1);
    itemBoxSizerVHeader->Add(budgetReportHeadingSizer, 0, wxALL, 1);

    wxBoxSizer* itemBoxSizerHHeader2 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizerVHeader->Add(itemBoxSizerHHeader2, 0, wxALL, 1);

    w_filter_btn = new wxButton(itemPanel3, wxID_FILE2);
    w_filter_btn->SetBitmap(mmImage::bitmapBundle(mmImage::png::TRANSFILTER, mmImage::bitmapButtonSize));
    w_filter_btn->SetMinSize(wxSize(300, -1));
    itemBoxSizerHHeader2->Add(w_filter_btn, g_flagsBorder1H);

    // A year and its months are separate budgets in MMEX and always have been.
    // Rather than change that, this offers to read the year as the sum of its
    // months; nothing is written either way.
    w_rollup = new wxCheckBox(itemPanel3, ID_BUDGET_ROLLUP,
        _t("Include monthly budgets"));
    mmToolTip(w_rollup, _t(
        "Add the budgets of this year's months to the figures shown. The stored "
        "budgets are not changed."));
    w_rollup->Hide();
    itemBoxSizerHHeader2->Add(w_rollup, g_flagsBorder1H);

    wxFlexGridSizer* itemIncomeSizer = new wxFlexGridSizer(0, 7, 5, 10);
    itemBoxSizerVHeader->Add(itemIncomeSizer);

    w_income_estimated = new wxStaticText(itemPanel3,
        ID_DIALOG_BUDGETENTRY_SUMMARY_INCOME_EST, "$",
        wxDefaultPosition, wxSize(120, -1)
    );
    w_income_actual = new wxStaticText(itemPanel3,
        ID_DIALOG_BUDGETENTRY_SUMMARY_INCOME_ACT, "$",
        wxDefaultPosition, wxSize(120, -1)
    );
    w_income_diff = new wxStaticText(itemPanel3,
        ID_DIALOG_BUDGETENTRY_SUMMARY_INCOME_DIF, "$"
    );

    w_expenses_estimated = new wxStaticText(itemPanel3,
        ID_DIALOG_BUDGETENTRY_SUMMARY_EXPENSES_EST, "$",
        wxDefaultPosition, wxSize(120, -1)
    );
    w_expenses_actual = new wxStaticText(itemPanel3,
        ID_DIALOG_BUDGETENTRY_SUMMARY_EXPENSES_ACT, "$",
        wxDefaultPosition, wxSize(120, -1)
    );
    w_expenses_diff = new wxStaticText(itemPanel3,
        ID_DIALOG_BUDGETENTRY_SUMMARY_EXPENSES_DIF, "$"
    );

    itemIncomeSizer->Add(new wxStaticText(itemPanel3, wxID_STATIC, _t("Income: ")));
    itemIncomeSizer->Add(new wxStaticText(itemPanel3, wxID_STATIC, _t("Estimated: ")));
    itemIncomeSizer->Add(w_income_estimated);
    itemIncomeSizer->Add(new wxStaticText(itemPanel3, wxID_STATIC, _t("Actual: ")));
    itemIncomeSizer->Add(w_income_actual);
    itemIncomeSizer->Add(new wxStaticText(itemPanel3, wxID_STATIC, _t("Difference: ")));
    itemIncomeSizer->Add(w_income_diff);

    itemIncomeSizer->Add(new wxStaticText(itemPanel3, wxID_STATIC, _t("Expenses: ")));
    itemIncomeSizer->Add(new wxStaticText(itemPanel3, wxID_STATIC, _t("Estimated: ")));
    itemIncomeSizer->Add(w_expenses_estimated);
    itemIncomeSizer->Add(new wxStaticText(itemPanel3, wxID_STATIC, _t("Actual: ")));
    itemIncomeSizer->Add(w_expenses_actual);
    itemIncomeSizer->Add(new wxStaticText(itemPanel3, wxID_STATIC, _t("Difference: ")));
    itemIncomeSizer->Add(w_expenses_diff);
    /* ---------------------- */

    wxVector<wxBitmapBundle> images;
    images.push_back(mmImage::bitmapBundle(mmImage::png::RECONCILED));
    images.push_back(mmImage::bitmapBundle(mmImage::png::VOID_STAT));
    images.push_back(mmImage::bitmapBundle(mmImage::png::FOLLOW_UP));

    w_list = new BudgetList(this, this, wxID_ANY);
    w_list->SetSmallImages(images);
    w_list->createColumns();

    itemBoxSizer2->Add(w_list.get(), 1, wxGROW | wxALL, 1);
}

void BudgetPanel::sortList()
{
    //TODO: Sort budget panel
}

bool BudgetPanel::displayEntryAllowed(int64 cat_id, int64 subcat_id)
{
    bool is_visible = false;

    double estimated = 0;
    double actual = 0;
    if (cat_id < 0) {
        estimated = m_estimate_actual_mCatId[subcat_id].first;
        actual = m_estimate_actual_mCatId[subcat_id].second;
    }
    else {
        estimated = getEstimate(cat_id);
        actual = m_amount_mMonth_mCatId[cat_id][0];
    }

    if (m_current_view == BudgetPanel::VIEW_NON_ZERO)
        is_visible = (estimated != 0.0 || actual != 0.0);
    else if (m_current_view == BudgetPanel::VIEW_INCOME)
        is_visible = (estimated > 0.0 || actual > 0.0);
    else if (m_current_view == BudgetPanel::VIEW_PLANNED)
        is_visible = (estimated != 0.0);
    else if (m_current_view == BudgetPanel::VIEW_EXPENSE)
        is_visible = (estimated < 0.0 || actual < 0.0);
    else if (m_current_view == BudgetPanel::VIEW_SUMM)
        is_visible = (cat_id < 0);
    else
        is_visible = true;

    if (cat_id > 0) {
        const CategoryData* cat_n = CategoryModel::instance().get_idN_data_n(cat_id);
        m_level_visible_mCatId[cat_id].second = is_visible;
        for (const auto& subcat_d : CategoryModel::instance().find_data_subtree_a(*cat_n)) {
            is_visible = is_visible || displayEntryAllowed(subcat_d.m_id, -1);
        }
    }
    return is_visible;
}

void BudgetPanel::initVirtualListControl()
{
    m_catId_subcatId_a.clear();
    m_estimate_actual_mCatId.clear();
    m_freq_mCatId.clear();
    m_amount_mCatId.clear();
    m_amount_mMonth_mCatId.clear();
    m_notes_mCatId.clear();
    double estIncome = 0.0;
    double estExpenses = 0.0;
    double actIncome = 0.0;
    double actExpenses = 0.0;
    mmReportBudget budgetDetails;

    bool evaluateTransfer = false;
    if (PrefModel::instance().getBudgetIncludeTransfers()) {
        evaluateTransfer = true;
    }

    m_current_view = InfoModel::instance().getString("BUDGET_FILTER", BudgetPanel::VIEW_ALL);
    const wxString bp_name_n = BudgetPeriodModel::instance().get_id_name_n(m_bp_id);
    long year = 0;
    bp_name_n.ToLong(&year);

    int startDay = 1;
    wxDateTime::Month startMonth = wxDateTime::Jan;
    if (PrefModel::instance().getBudgetFinancialYears())
        budgetDetails.GetFinancialYearValues(startDay, startMonth);
    wxDateTime dtBegin(startDay, startMonth, year);
    wxDateTime dtEnd = dtBegin;
    dtEnd.Add(wxDateSpan::Year()).Subtract(wxDateSpan::Day());

    m_is_monthly = (bp_name_n.length() > 5);

    if (m_is_monthly) {
        budgetDetails.SetBudgetMonth(bp_name_n, dtBegin, dtEnd);
        m_month_name = wxGetTranslation(wxDateTime::GetEnglishMonthName(dtBegin.GetMonth()));
    }

    // Rolling up is offered where a period has something beneath it: a year has
    // its months, and a segmented month has its parts. In both cases it totals
    // the children without rewriting either level.
    const std::vector<BudgetSegmentData> seg_a = segment_a();
    const bool has_segments = !seg_a.empty();
    const bool can_rollup =
        (!m_is_monthly && !BudgetModel::instance().find_child_period_id_a(m_bp_id).empty())
        || (m_segment_id <= 0 && has_segments);

    if (w_rollup) {
        w_rollup->Show(can_rollup);
        if (!can_rollup)
            w_rollup->SetValue(false);
        w_rollup->SetLabel(has_segments && m_segment_id <= 0
            ? _t("Include segments")
            : _t("Include months"));
    }

    // Readjust dates by the Budget Offset Option
    PrefModel::instance().addBudgetDateOffset(dtBegin);
    m_start_date = dtBegin.FormatISODate();
    PrefModel::instance().addBudgetDateOffset(dtEnd);
    mmDateRange date_range;
    date_range.start_date(dtBegin.ResetTime()); // Start of Day
    date_range.end_date(dtEnd.ResetTime().Add(wxTimeSpan(23,59,59,999))); // End of Day

    // What this view is answerable for. A segment shows its own entries; the
    // period as a whole shows the ones filed against it rather than against a
    // part, so the month and its halves no longer read identically. Rolling up
    // widens the whole-period view to take the parts in as well.
    const bool rolled_up = isRolledUp();
    int64 segment_scope = BudgetModel::SEGMENT_ANY;
    if (m_segment_id > 0)
        segment_scope = m_segment_id;
    else if (has_segments && !rolled_up)
        segment_scope = BudgetModel::SEGMENT_NONE;

    //Get statistics
    BudgetModel::instance().getBudgetEntry(
        m_bp_id, m_freq_mCatId, m_amount_mCatId, m_notes_mCatId,
        (rolled_up && !m_is_monthly),
        segment_scope
    );
    CategoryModel::instance().getCategoryStats(
        m_amount_mMonth_mCatId,
        static_cast<wxSharedPtr<wxArrayString>>(nullptr),
        &date_range,
        PrefModel::instance().getIgnoreFutureTransactions(),
        false,
        (evaluateTransfer ? &m_amount_mCatId : nullptr)
    );

    // start with only the root categories

    for (const CategoryData& cat_d : CategoryModel::instance().find_data_a(
        CategoryCol::WHERE_PARENTID(OP_EQ, -1),
        TableClause::ORDERBY(CategoryCol::NAME_CATEGNAME)
    )) {
        m_level_visible_mCatId[cat_d.m_id].first = 0;
        double estimated = getEstimate(cat_d.m_id);
        if (estimated < 0)
            estExpenses += estimated;
        else
            estIncome += estimated;

        double actual = 0;
        if (m_current_view != BudgetPanel::VIEW_PLANNED || estimated != 0) {
            actual = m_amount_mMonth_mCatId[cat_d.m_id][0];
            if (actual < 0)
                actExpenses += actual;
            else
                actIncome += actual;
        }


        m_estimate_actual_mCatId[cat_d.m_id].first = estimated;
        m_estimate_actual_mCatId[cat_d.m_id].second = actual;

        if (displayEntryAllowed(cat_d.m_id, -1))
            m_catId_subcatId_a.emplace_back(cat_d.m_id, -1);

        std::vector<int> totals_queue;
        //now a depth-first walk of the subtree of this root cat_d
        CategoryModel::DataA subcat_a = CategoryModel::instance().find_data_subtree_a(cat_d);
        for (int i = 0; i < static_cast<int>(subcat_a.size()); i++) {
            estimated = getEstimate(subcat_a[i].m_id);
            if (estimated < 0)
                estExpenses += estimated;
            else
                estIncome += estimated;

            actual = 0;
            if (m_current_view != BudgetPanel::VIEW_PLANNED || estimated != 0) {
                actual = m_amount_mMonth_mCatId[subcat_a[i].m_id][0];
                if (actual < 0)
                    actExpenses += actual;
                else
                    actIncome += actual;
            }
            // save totals for this subcategory
            m_estimate_actual_mCatId[subcat_a[i].m_id].first = estimated;
            m_estimate_actual_mCatId[subcat_a[i].m_id].second = actual;

            // update totals of the category
            m_estimate_actual_mCatId[cat_d.m_id].first += estimated;
            m_estimate_actual_mCatId[cat_d.m_id].second += actual;

            // walk up the hierarchy and update all the parent totals as well
            int64 parent_id_n = subcat_a[i].m_parent_id_n;
            m_level_visible_mCatId[subcat_a[i].m_id].first = 1;
            for (int j = i; j > 0; --j) {
                if (subcat_a[j - 1].m_id != parent_id_n)
                    continue;
                m_level_visible_mCatId[subcat_a[i].m_id].first++;
                m_estimate_actual_mCatId[parent_id_n].first += estimated;
                m_estimate_actual_mCatId[parent_id_n].second += actual;
                parent_id_n = subcat_a[j - 1].m_parent_id_n;
                if (parent_id_n == cat_d.m_id)
                    break;
            }

            // add the subcategory row to the display list
            if (displayEntryAllowed(subcat_a[i].m_id, -1))
                m_catId_subcatId_a.emplace_back(subcat_a[i].m_id, -1);

            // check if we need to show any total rows before the next subcategory
            if (i < static_cast<int>(subcat_a.size()) - 1) { //not the last subcategory
                //if next subcategory is our child, queue the total for after the children
                if (subcat_a[i].m_id == subcat_a[i + 1].m_parent_id_n)
                    totals_queue.emplace_back(i);
                else if (subcat_a[i].m_parent_id_n != subcat_a[i + 1].m_parent_id_n) {
                    // last sibling -- we've exhausted this branch,
                    // so display all the totals we held on to
                    while (!totals_queue.empty() &&
                        subcat_a[totals_queue.back()].m_id != subcat_a[i + 1].m_parent_id_n
                    ) {
                        if (displayEntryAllowed(-1, subcat_a[totals_queue.back()].m_id)) {
                            m_catId_subcatId_a.emplace_back(-1, subcat_a[totals_queue.back()].m_id);
                            size_t transCatTotalIndex = m_catId_subcatId_a.size() - 1;
                            w_list->RefreshItem(transCatTotalIndex);
                        }
                        totals_queue.pop_back();
                    }
                }
            }
            // the very last subcategory, so show the rest of the queued totals
            else {
                while (!totals_queue.empty()) {
                    if (displayEntryAllowed(-1, subcat_a[totals_queue.back()].m_id)) {
                        m_catId_subcatId_a.emplace_back(-1, subcat_a[totals_queue.back()].m_id);
                        size_t transCatTotalIndex = m_catId_subcatId_a.size() - 1;
                        w_list->RefreshItem(transCatTotalIndex);
                    }
                    totals_queue.pop_back();
                }
            }
        }

        // show the total of the category after all subcats have been shown
        if (displayEntryAllowed(-1, cat_d.m_id)) {
            m_catId_subcatId_a.emplace_back(-1, cat_d.m_id);
            size_t transCatTotalIndex = m_catId_subcatId_a.size() - 1;
            w_list->RefreshItem(transCatTotalIndex);
        }
    }

    w_list->SetItemCount(m_catId_subcatId_a.size());

    wxString est_amount, act_amount, diff_amount;
    est_amount = CurrencyModel::instance().toCurrency(estIncome);
    act_amount = CurrencyModel::instance().toCurrency(actIncome);
    diff_amount = CurrencyModel::instance().toCurrency(actIncome - estIncome);

    w_income_estimated->SetLabelText(est_amount);
    w_income_actual->SetLabelText(act_amount);
    w_income_diff->SetLabelText(diff_amount);

    if (estExpenses < 0.0) estExpenses = -estExpenses;
    if (actExpenses < 0.0) actExpenses = -actExpenses;
    est_amount = CurrencyModel::instance().toCurrency(estExpenses);
    act_amount = CurrencyModel::instance().toCurrency(actExpenses);
    diff_amount = CurrencyModel::instance().toCurrency(estExpenses - actExpenses);

    w_expenses_estimated->SetLabelText(est_amount);
    w_expenses_actual->SetLabelText(act_amount);
    w_expenses_diff->SetLabelText(diff_amount);
    updateBudgetHeading();
}

double BudgetPanel::getEstimate(int64 cat_id) const
{
    try {
        BudgetFreq freq = m_freq_mCatId.at(cat_id);
        double amount = m_amount_mCatId.at(cat_id);
        return m_is_monthly
            ? amount * freq.times_per_month()
            : amount * freq.times_per_year();
    }
    catch (std::out_of_range const& exc) {
        wxASSERT(false);
        wxLogDebug(wxString::FromUTF8(exc.what()));
        return 0.0;
    }
}

void BudgetPanel::displayBudgetingDetails(int64 budgetYearID, int64 segment_id)
{
    this->windowsFreezeThaw();
    m_bp_id = budgetYearID;
    m_segment_id = segment_id;
    refreshList();
    this->windowsFreezeThaw();
}

wxString BudgetPanel::getItem(long item, int col_id)
{
    switch (col_id) {
    case BudgetList::LIST_ID_ICON:
        return " ";
    case BudgetList::LIST_ID_CATEGORY: {
        const CategoryData* cat_n = CategoryModel::instance().get_idN_data_n(
            m_catId_subcatId_a[item].first > 0
                ? m_catId_subcatId_a[item].first
                : m_catId_subcatId_a[item].second
        );
        if (cat_n) {
            wxString name = cat_n->m_name;
            for (int64 i = m_level_visible_mCatId[cat_n->m_id].first; i > 0; i--) {
                name.Prepend("    ");
            }
            return name;
        }
        return wxEmptyString;
    }
    case BudgetList::LIST_ID_FREQUENCY: {
        if (m_catId_subcatId_a[item].first >= 0 && m_level_visible_mCatId[m_catId_subcatId_a[item].first].second) {
            BudgetFreq freq = m_freq_mCatId[m_catId_subcatId_a[item].first];
            return wxGetTranslation(freq.name());
        }
        return wxEmptyString;
    }
    case BudgetList::LIST_ID_AMOUNT: {
        if (m_catId_subcatId_a[item].first >= 0 && m_level_visible_mCatId[m_catId_subcatId_a[item].first].second) {
            double amount = m_amount_mCatId[m_catId_subcatId_a[item].first];
            return CurrencyModel::instance().toCurrency(amount);
        }
        return wxEmptyString;
    }
    case BudgetList::LIST_ID_ESTIMATED: {
        if (m_catId_subcatId_a[item].first < 0) {
            double estimated = m_estimate_actual_mCatId[m_catId_subcatId_a[item].second].first;
            return CurrencyModel::instance().toCurrency(estimated);
        }
        else if (m_level_visible_mCatId[m_catId_subcatId_a[item].first].second) {
            double estimated = getEstimate(m_catId_subcatId_a[item].first);
            return CurrencyModel::instance().toCurrency(estimated);
        }
        return wxEmptyString;
    }
    case BudgetList::LIST_ID_ACTUAL: {
        if (m_catId_subcatId_a[item].first < 0) {
            double actual = m_estimate_actual_mCatId[m_catId_subcatId_a[item].second].second;
            return CurrencyModel::instance().toCurrency(actual);
        }
        else if (m_level_visible_mCatId[m_catId_subcatId_a[item].first].second) {
            double actual = m_amount_mMonth_mCatId[m_catId_subcatId_a[item].second >= 0
                ? m_catId_subcatId_a[item].second
                : m_catId_subcatId_a[item].first
            ][0];
            return CurrencyModel::instance().toCurrency(actual);
        }
        return wxEmptyString;
    }
    case BudgetList::LIST_ID_NOTES:
        if (m_catId_subcatId_a[item].first >= 0 && m_level_visible_mCatId[m_catId_subcatId_a[item].first].second) {
            wxString value = m_notes_mCatId[m_catId_subcatId_a[item].second >= 0
                ? m_catId_subcatId_a[item].second
                : m_catId_subcatId_a[item].first
            ];
            value.Replace("\n", " ");
            return value;
        }
        return wxEmptyString;
    default:
        return wxEmptyString;
    }
}

int BudgetPanel::getItemImage(long item) const
{
    try {

        double estimated = 0;
        double actual = 0;
        if (m_catId_subcatId_a[item].first < 0) {
            estimated = m_estimate_actual_mCatId.at(m_catId_subcatId_a[item].second).first;
            actual = m_estimate_actual_mCatId.at(m_catId_subcatId_a[item].second).second;
        }
        else {
            estimated = getEstimate(m_catId_subcatId_a[item].second >= 0
                ? m_catId_subcatId_a[item].second
                : m_catId_subcatId_a[item].first
            );
            actual = m_amount_mMonth_mCatId.at(m_catId_subcatId_a[item].second >= 0
                ? m_catId_subcatId_a[item].second
                : m_catId_subcatId_a[item].first
            ).at(0);
        }

        if (estimated == 0.0 && actual == 0.0)
            return -1;
        if (estimated == 0.0 && actual != 0.0)
            return ICON_FOLLOWUP;
        if (estimated < actual)
            return ICON_RECONCILLED;
        if (std::fabs(estimated - actual) < 0.001)
            return ICON_RECONCILLED;
        return ICON_VOID;
    }
    catch (std::out_of_range const& exc) {
        wxASSERT(false);
        wxLogDebug(wxString::FromUTF8(exc.what()));
        return 1;
    }
}

void BudgetPanel::onListItemActivated(int item)
{
    // A TOTALS entry does not contain a budget entry, therefore ignore the event.
    if (m_catId_subcatId_a[item].first == -1)
        return;

    // Rolled up, the figure on the row is a total of other periods or segments,
    // and the entry behind it belongs to this view alone. Editing would silently
    // change something other than what is being read, so say so first.
    if (isRolledUp()) {
        const int reply = wxMessageBox(
            _t("These figures include other periods, so this row is a total "
               "rather than a single entry.\n\n"
               "Editing it changes only this view's own entry, which may be a "
               "different amount. Open it anyway?"),
            _t("Budget Planner"),
            wxYES_NO | wxICON_QUESTION, this
        );
        if (reply != wxYES)
            return;
    }

    int64 subcat_id = (m_catId_subcatId_a[item].second >= 0)
            ? m_catId_subcatId_a[item].second
            : m_catId_subcatId_a[item].first;

    BudgetModel::DataA budget_a = BudgetModel::instance().find_data_a(
        BudgetCol::WHERE_BUDGETYEARID(OP_EQ, getBudgetYearID()),
        BudgetCol::WHERE_CATEGID(OP_EQ, subcat_id)
    );

    // A category can hold one entry per segment plus one for the period as a
    // whole. Taking the first row would edit whichever happened to come back
    // first, so pick the one this view is actually showing.
    BudgetData* match_n = nullptr;
    for (auto& b_d : budget_a) {
        const bool is_whole_period = (b_d.m_segment_id <= 0);
        if (m_segment_id > 0 ? (b_d.m_segment_id == m_segment_id) : is_whole_period) {
            match_n = &b_d;
            break;
        }
    }

    BudgetData budget_d = BudgetData();
    if (!match_n) {
        // CHECK: budget_d.m_category_id
        budget_d = BudgetData();
        budget_d.m_period_id   = getBudgetYearID();
        budget_d.m_category_id = m_catId_subcatId_a[item].first;
        budget_d.m_amount      = 0.0;
        // File it where the reader is looking, not against the whole period.
        budget_d.m_segment_id  = (m_segment_id > 0) ? m_segment_id : -1;
        BudgetModel::instance().add_data_n(budget_d);
    }
    else {
        budget_d = *match_n;
    }

    double estimate = getEstimate(subcat_id);
    double actual = m_amount_mMonth_mCatId[subcat_id][0];
    BudgetEntryDialog dlg(this, &budget_d,
        CurrencyModel::instance().toCurrency(estimate),
        CurrencyModel::instance().toCurrency(actual)
    );

    if (dlg.ShowModal() == wxID_OK) {
        initVirtualListControl();
        w_list->Refresh();
        w_list->Update();
        w_list->EnsureVisible(item);
    }
}

void BudgetPanel::showRowContextMenu(long item)
{
    if (item < 0 || static_cast<size_t>(item) >= m_catId_subcatId_a.size())
        return;
    // The TOTALS row stands for no entry.
    if (m_catId_subcatId_a[item].first == -1)
        return;

    const std::vector<BudgetSegmentData> seg_a = segment_a();
    if (seg_a.empty()) {
        // Nothing to move between; a menu offering one destination that is
        // already the current one would only be noise.
        return;
    }

    m_context_item = item;

    wxMenu menu;
    wxMenu* move = new wxMenu;

    move->AppendRadioItem(MENU_MOVE_TO_FIRST, _t("Whole period"))
        ->Check(m_segment_id <= 0);

    int n = 1;
    for (const auto& seg_d : seg_a) {
        if (n > 63)
            break;  // the reserved id block; a period with 63 parts is not real
        move->AppendRadioItem(
            MENU_MOVE_TO_FIRST + n,
            wxString::Format("%s (%d-%d)",
                seg_d.m_name, seg_d.m_start_day, seg_d.m_end_day)
        )->Check(seg_d.m_id == m_segment_id);
        ++n;
    }

    menu.AppendSubMenu(move, _t("Move entry to"));
    PopupMenu(&menu);
}

void BudgetPanel::onMoveToSegment(wxCommandEvent& event)
{
    if (m_context_item < 0 ||
        static_cast<size_t>(m_context_item) >= m_catId_subcatId_a.size())
        return;

    const int index = event.GetId() - MENU_MOVE_TO_FIRST;
    if (index < 0)
        return;

    int64 target = -1;  // the period as a whole
    if (index > 0) {
        const std::vector<BudgetSegmentData> seg_a = segment_a();
        if (static_cast<size_t>(index - 1) >= seg_a.size())
            return;
        target = seg_a[index - 1].m_id;
    }

    const int64 cat_id = (m_catId_subcatId_a[m_context_item].second >= 0)
        ? m_catId_subcatId_a[m_context_item].second
        : m_catId_subcatId_a[m_context_item].first;

    if (moveEntryToSegment(cat_id, target))
        refreshList();
}

bool BudgetPanel::moveEntryToSegment(int64 cat_id, int64 target_segment_id)
{
    const int64 from = (m_segment_id > 0) ? m_segment_id : -1;

    // Ask before merging rather than after, so a collision cannot quietly
    // double a figure.
    BudgetModel& model = BudgetModel::instance();
    BudgetModel::MoveOutcome outcome =
        model.move_entry_segment(m_bp_id, cat_id, from, target_segment_id, false);

    if (outcome == BudgetModel::MOVE_OCCUPIED) {
        const int reply = wxMessageBox(
            _t("That destination already has an entry for this category.\n\n"
               "Add the two amounts together into one entry?"),
            _t("Move entry"), wxYES_NO | wxICON_QUESTION, this);
        if (reply != wxYES)
            return false;
        outcome = model.move_entry_segment(
            m_bp_id, cat_id, from, target_segment_id, true);
    }

    switch (outcome) {
    case BudgetModel::MOVE_DONE:
    case BudgetModel::MOVE_MERGED:
        return true;

    case BudgetModel::MOVE_UNCHANGED:
        return false;

    case BudgetModel::MOVE_NO_SOURCE:
        wxMessageBox(
            _t("There is no budget entry for this category in the part you are "
               "looking at, so there is nothing to move."),
            _t("Move entry"), wxOK | wxICON_INFORMATION, this);
        return false;

    case BudgetModel::MOVE_FREQ_MISMATCH:
        wxMessageBox(
            _t("The destination already budgets this category at a different "
               "frequency.\n\n"
               "Combining the two would change the total, so nothing has been "
               "moved. Make the frequencies match first, or edit the "
               "destination directly."),
            _t("Move entry"), wxOK | wxICON_WARNING, this);
        return false;

    default:
        return false;
    }
}
