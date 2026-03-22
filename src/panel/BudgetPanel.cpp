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

#include "base/constants.h"
#include "base/images_list.h"
#include "mmex.h"
#include "util/mmDateRange.h"

#include "model/_all.h"
#include "model/PrefModel.h"

#include "BudgetPanel.h"
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
};

const wxString BudgetPanel::VIEW_ALL      = _n("View All Budget Categories");
const wxString BudgetPanel::VIEW_NON_ZERO = _n("View Non-Zero Budget Categories");
const wxString BudgetPanel::VIEW_PLANNED  = _n("View Planned Budget Categories");
const wxString BudgetPanel::VIEW_INCOME   = _n("View Income Budget Categories");
const wxString BudgetPanel::VIEW_EXPENSE  = _n("View Expense Budget Categories");
const wxString BudgetPanel::VIEW_SUMM     = _n("View Budget Category Summary");

wxBEGIN_EVENT_TABLE(BudgetPanel, wxPanel)
    EVT_BUTTON(wxID_FILE2, BudgetPanel::onMouseLeftDown)
    EVT_MENU(wxID_ANY,     BudgetPanel::onViewPopupSelected)
wxEND_EVENT_TABLE()

BudgetPanel::BudgetPanel(
    int64 bp_id,
    wxWindow* parent_win,
    wxWindowID win_id,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name
) :
    w_list(nullptr),
    m_bp_id(bp_id)
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

    InfoModel::instance().setString("BUDGET_FILTER", m_current_view);

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

    return title;
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
    w_filter_btn->SetBitmap(mmBitmapBundle(png::TRANSFILTER, mmBitmapButtonSize));
    w_filter_btn->SetMinSize(wxSize(300, -1));
    itemBoxSizerHHeader2->Add(w_filter_btn, g_flagsBorder1H);

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
    images.push_back(mmBitmapBundle(png::RECONCILED));
    images.push_back(mmBitmapBundle(png::VOID_STAT));
    images.push_back(mmBitmapBundle(png::FOLLOW_UP));

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
        const CategoryData* cat_n = CategoryModel::instance().get_id_data_n(cat_id);
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

    // Readjust dates by the Budget Offset Option
    PrefModel::instance().addBudgetDateOffset(dtBegin);
    m_start_date = dtBegin.FormatISODate();   
    PrefModel::instance().addBudgetDateOffset(dtEnd);
    mmDateRange date_range;
    date_range.start_date(dtBegin.ResetTime()); // Start of Day
    date_range.end_date(dtEnd.ResetTime().Add(wxTimeSpan(23,59,59,999))); // End of Day

    //Get statistics
    BudgetModel::instance().getBudgetEntry(
        m_bp_id, m_freq_mCatId, m_amount_mCatId, m_notes_mCatId
    );
    CategoryModel::instance().getCategoryStats(
        m_amount_mMonth_mCatId,
        static_cast<wxSharedPtr<wxArrayString>>(nullptr),
        &date_range,
        PrefModel::instance().getIgnoreFutureTransactions(),
        false,
        (evaluateTransfer ? &m_amount_mCatId : nullptr)
    );

    //start with only the root categories
    CategoryModel::DataA cat_a = CategoryModel::instance().find(
        CategoryCol::PARENTID(-1)
    );
    std::stable_sort(cat_a.begin(), cat_a.end(), CategoryData::SorterByCATEGNAME());
    for (const auto& cat_d : cat_a) {
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

void BudgetPanel::displayBudgetingDetails(int64 budgetYearID)
{
    this->windowsFreezeThaw();
    m_bp_id = budgetYearID;
    refreshList();
    this->windowsFreezeThaw();
}

wxString BudgetPanel::getItem(long item, int col_id)
{
    switch (col_id) {
    case BudgetList::LIST_ID_ICON:
        return " ";
    case BudgetList::LIST_ID_CATEGORY: {
        const CategoryData* category_n = CategoryModel::instance().get_id_data_n(
            m_catId_subcatId_a[item].first > 0
                ? m_catId_subcatId_a[item].first
                : m_catId_subcatId_a[item].second
        );
        if (category_n) {
            wxString name = category_n->m_name;
            for (int64 i = m_level_visible_mCatId[category_n->m_id].first; i > 0; i--) {
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

    int64 subcat_id = (m_catId_subcatId_a[item].second >= 0)
            ? m_catId_subcatId_a[item].second
            : m_catId_subcatId_a[item].first;

    BudgetModel::DataA budget_a = BudgetModel::instance().find(
        BudgetCol::BUDGETYEARID(getBudgetYearID()),
        BudgetCol::CATEGID(subcat_id)
    );

    BudgetData budget_d = BudgetData();
    if (budget_a.empty()) {
        // CHECK: budget_d.m_category_id
        budget_d = BudgetData();
        budget_d.m_period_id   = getBudgetYearID();
        budget_d.m_category_id = m_catId_subcatId_a[item].first;
        budget_d.m_amount      = 0.0;
        BudgetModel::instance().add_data_n(budget_d);
    }
    else
        budget_d = budget_a[0];

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
