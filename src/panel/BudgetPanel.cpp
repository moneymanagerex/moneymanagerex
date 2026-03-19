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

static const wxString VIEW_ALL      = _n("View All Budget Categories");
static const wxString VIEW_NON_ZERO = _n("View Non-Zero Budget Categories");
static const wxString VIEW_PLANNED  = _n("View Planned Budget Categories");
static const wxString VIEW_INCOME   = _n("View Income Budget Categories");
static const wxString VIEW_EXPENSE  = _n("View Expense Budget Categories");
static const wxString VIEW_SUMM     = _n("View Budget Category Summary");

wxBEGIN_EVENT_TABLE(BudgetPanel, wxPanel)
    EVT_BUTTON(wxID_FILE2, BudgetPanel::OnMouseLeftDown)
    EVT_MENU(wxID_ANY,     BudgetPanel::OnViewPopupSelected)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(BudgetList, ListBase)
    EVT_LIST_ITEM_SELECTED(wxID_ANY,  BudgetList::OnListItemSelected)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY, BudgetList::OnListItemActivated)
    EVT_MOTION(BudgetList::OnMouseMove)
wxEND_EVENT_TABLE()

const std::vector<ListColumnInfo> BudgetList::LIST_INFO = {
    { LIST_ID_ICON,      true, _n("Icon"),      _WH, _FL, false },
    { LIST_ID_CATEGORY,  true, _n("Category"),  _WH, _FL, false },
    { LIST_ID_FREQUENCY, true, _n("Frequency"), _WH, _FL, false },
    { LIST_ID_AMOUNT,    true, _n("Amount"),    _WH, _FR, false },
    { LIST_ID_ESTIMATED, true, _n("Estimated"), _WH, _FR, false },
    { LIST_ID_ACTUAL,    true, _n("Actual"),    _WH, _FR, false },
    { LIST_ID_NOTES,     true, _n("Notes"),     _WH, _FL, false },
};

BudgetPanel::BudgetPanel(int64 budgetYearID
    , wxWindow *parent
    , wxWindowID winid
    , const wxPoint& pos, const wxSize& size
    , long style, const wxString& name)
    : m_lc(nullptr)
    , budgetYearID_(budgetYearID)
{
    Create(parent, winid, pos, size, style, name);
    mmThemeAutoColour(this);
}

bool BudgetPanel::Create(wxWindow *parent
    , wxWindowID winid, const wxPoint& pos
    , const wxSize& size, long style, const wxString& name)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create(parent, winid, pos, size, style, name);

    this->windowsFreezeThaw();
    CreateControls();
    mmThemeAutoColour(this);
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    initVirtualListControl();
    if (!budget_.empty())
        m_lc->EnsureVisible(0);

    this->windowsFreezeThaw();
    UsageModel::instance().pageview(this);
    return true;
}

BudgetPanel::~BudgetPanel()
{

}

void BudgetPanel::OnViewPopupSelected(wxCommandEvent& event)
{
    int evt =  event.GetId();
    if (evt ==  MENU_VIEW_ALLBUDGETENTRIES)
        currentView_ = VIEW_ALL;
    else if (evt == MENU_VIEW_NONZEROBUDGETENTRIES)
        currentView_ = VIEW_NON_ZERO;
    else if (evt == MENU_VIEW_PLANNEDBUDGETENTRIES)
        currentView_ = VIEW_PLANNED;
    else if (evt == MENU_VIEW_INCOMEBUDGETENTRIES)
        currentView_ = VIEW_INCOME;
    else if (evt == MENU_VIEW_EXPENSEBUDGETENTRIES)
        currentView_ = VIEW_EXPENSE;
    else if (evt == MENU_VIEW_SUMMARYBUDGETENTRIES)
        currentView_ = VIEW_SUMM;
    else {
        wxASSERT(false);
    }

    InfoModel::instance().setString("BUDGET_FILTER", currentView_);

    RefreshList();
}

void BudgetPanel::RefreshList()
{
    initVirtualListControl();
    m_lc->Refresh();
    m_lc->Update();
    if (!budget_.empty())
        m_lc->EnsureVisible(0);
}

void BudgetPanel::OnMouseLeftDown(wxCommandEvent& event)
{
    wxMenu menu;
    menu.Append(MENU_VIEW_ALLBUDGETENTRIES, wxGetTranslation(VIEW_ALL));
    menu.Append(MENU_VIEW_PLANNEDBUDGETENTRIES, wxGetTranslation(VIEW_PLANNED));
    menu.Append(MENU_VIEW_NONZEROBUDGETENTRIES, wxGetTranslation(VIEW_NON_ZERO));
    menu.Append(MENU_VIEW_INCOMEBUDGETENTRIES, wxGetTranslation(VIEW_INCOME));
    menu.Append(MENU_VIEW_EXPENSEBUDGETENTRIES, wxGetTranslation(VIEW_EXPENSE));
    menu.AppendSeparator();
    menu.Append(MENU_VIEW_SUMMARYBUDGETENTRIES, wxGetTranslation(VIEW_SUMM));
    PopupMenu(&menu);

    event.Skip();
}

wxString BudgetPanel::GetPanelTitle() const
{
    wxString bp_name_n = BudgetPeriodModel::instance().get_id_name_n(budgetYearID_);
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
        title += wxString::Format(" (%s)", m_monthName);
    }

    if (PrefModel::instance().getBudgetDaysOffset() != 0) {
        title = wxString::Format(_t("%1$s    Start Date of: %2$s"),
            title,
            mmGetDateTimeForDisplay(m_budget_offset_date)
        );
    }

    title = wxString::Format(_t("Budget Planner for %s"), title);

    return title;
}

void BudgetPanel::UpdateBudgetHeading()
{
    budgetReportHeading_->SetLabel(GetPanelTitle());
    m_bitmapTransFilter->SetLabel(wxGetTranslation(currentView_));
}

void BudgetPanel::CreateControls()
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

    budgetReportHeading_ = new wxStaticText(itemPanel3, wxID_ANY, "");

    budgetReportHeading_->SetFont(this->GetFont().Larger().Bold());

    wxBoxSizer* budgetReportHeadingSizer = new wxBoxSizer(wxHORIZONTAL);
    budgetReportHeadingSizer->Add(budgetReportHeading_, 1);
    itemBoxSizerVHeader->Add(budgetReportHeadingSizer, 0, wxALL, 1);

    wxBoxSizer* itemBoxSizerHHeader2 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizerVHeader->Add(itemBoxSizerHHeader2, 0, wxALL, 1);

    m_bitmapTransFilter = new wxButton(itemPanel3, wxID_FILE2);
    m_bitmapTransFilter->SetBitmap(mmBitmapBundle(png::TRANSFILTER, mmBitmapButtonSize));
    m_bitmapTransFilter->SetMinSize(wxSize(300, -1));
    itemBoxSizerHHeader2->Add(m_bitmapTransFilter, g_flagsBorder1H);

    wxFlexGridSizer* itemIncomeSizer = new wxFlexGridSizer(0, 7, 5, 10);
    itemBoxSizerVHeader->Add(itemIncomeSizer);

    income_estimated_ = new wxStaticText(itemPanel3
        , ID_DIALOG_BUDGETENTRY_SUMMARY_INCOME_EST, "$", wxDefaultPosition, wxSize(120, -1));
    income_actual_ = new wxStaticText(itemPanel3
        , ID_DIALOG_BUDGETENTRY_SUMMARY_INCOME_ACT, "$", wxDefaultPosition, wxSize(120, -1));
    income_diff_ = new wxStaticText(itemPanel3
        , ID_DIALOG_BUDGETENTRY_SUMMARY_INCOME_DIF, "$");

    expenses_estimated_ = new wxStaticText(itemPanel3
        , ID_DIALOG_BUDGETENTRY_SUMMARY_EXPENSES_EST, "$", wxDefaultPosition, wxSize(120, -1));
    expenses_actual_ = new wxStaticText(itemPanel3
        , ID_DIALOG_BUDGETENTRY_SUMMARY_EXPENSES_ACT, "$", wxDefaultPosition, wxSize(120, -1));
    expenses_diff_ = new wxStaticText(itemPanel3
        , ID_DIALOG_BUDGETENTRY_SUMMARY_EXPENSES_DIF, "$");

    itemIncomeSizer->Add(new wxStaticText(itemPanel3, wxID_STATIC, _t("Income: ")));
    itemIncomeSizer->Add(new wxStaticText(itemPanel3, wxID_STATIC, _t("Estimated: ")));
    itemIncomeSizer->Add(income_estimated_);
    itemIncomeSizer->Add(new wxStaticText(itemPanel3, wxID_STATIC, _t("Actual: ")));
    itemIncomeSizer->Add(income_actual_);
    itemIncomeSizer->Add(new wxStaticText(itemPanel3, wxID_STATIC, _t("Difference: ")));
    itemIncomeSizer->Add(income_diff_);

    itemIncomeSizer->Add(new wxStaticText(itemPanel3, wxID_STATIC, _t("Expenses: ")));
    itemIncomeSizer->Add(new wxStaticText(itemPanel3, wxID_STATIC, _t("Estimated: ")));
    itemIncomeSizer->Add(expenses_estimated_);
    itemIncomeSizer->Add(new wxStaticText(itemPanel3, wxID_STATIC, _t("Actual: ")));
    itemIncomeSizer->Add(expenses_actual_);
    itemIncomeSizer->Add(new wxStaticText(itemPanel3, wxID_STATIC, _t("Difference: ")));
    itemIncomeSizer->Add(expenses_diff_);
    /* ---------------------- */

    wxVector<wxBitmapBundle> images;
    images.push_back(mmBitmapBundle(png::RECONCILED));
    images.push_back(mmBitmapBundle(png::VOID_STAT));
    images.push_back(mmBitmapBundle(png::FOLLOW_UP));

    m_lc = new BudgetList(this, this, wxID_ANY);
    m_lc->SetSmallImages(images);
    m_lc->createColumns();

    itemBoxSizer2->Add(m_lc.get(), 1, wxGROW | wxALL, 1);
}

BudgetList::BudgetList(
    BudgetPanel* cp, wxWindow *parent, const wxWindowID id
) :
    ListBase(parent, id),
    attr3_(new wxListItemAttr(
        wxNullColour, mmThemeMetaColour(meta::COLOR_LISTTOTAL), wxNullFont
    )),
    cp_(cp)
{
    mmThemeMetaColour(this, meta::COLOR_LISTPANEL);

    m_setting_name = "BUDGET";
    m_col_info_id = LIST_INFO;
    o_col_width_prefix = "BUDGET_COL";
}

void BudgetPanel::sortList()
{
    //TODO: Sort budget panel
}

bool BudgetPanel::DisplayEntryAllowed(int64 cat_id, int64 subcategoryID)
{
    bool result = false;

    double actual = 0;
    double estimated = 0;
    if (cat_id < 0) {
        actual = budgetTotals_[subcategoryID].second;
        estimated = budgetTotals_[subcategoryID].first;
    }
    else {
        actual = categoryStats_[cat_id][0];
        estimated = getEstimate(cat_id);
    }

    if (currentView_ == VIEW_NON_ZERO)
        result = ((estimated != 0.0) || (actual != 0.0));
    else if (currentView_ == VIEW_INCOME)
        result = ((estimated > 0.0) || (actual > 0.0));
    else if (currentView_ == VIEW_PLANNED)
        result = (estimated != 0.0);
    else if (currentView_ == VIEW_EXPENSE)
        result = ((estimated < 0.0) || (actual < 0.0));
    else if (currentView_ == VIEW_SUMM)
        result = (cat_id < 0);
    else
        result = true;

    if (cat_id > 0) {
        const CategoryData* cat_n = CategoryModel::instance().get_id_data_n(cat_id);
        displayDetails_[cat_id].second = result;
        for (const auto& subcat_d : CategoryModel::instance().find_data_subtree_a(*cat_n)) {
            result = result || DisplayEntryAllowed(subcat_d.m_id, -1);
        }
    }
    return result;
}

void BudgetPanel::initVirtualListControl()
{
    budget_.clear();
    budgetTotals_.clear();
    budgetPeriod_.clear();
    budgetAmt_.clear();
    categoryStats_.clear();
    budgetNotes_.clear();
    double estIncome = 0.0;
    double estExpenses = 0.0;
    double actIncome = 0.0;
    double actExpenses = 0.0;
    mmReportBudget budgetDetails;

    bool evaluateTransfer = false;
    if (PrefModel::instance().getBudgetIncludeTransfers()) {
        evaluateTransfer = true;
    }

    currentView_ = InfoModel::instance().getString("BUDGET_FILTER", VIEW_ALL);
    const wxString bp_name_n = BudgetPeriodModel::instance().get_id_name_n(budgetYearID_);
    long year = 0;
    bp_name_n.ToLong(&year);

    int startDay = 1;
    wxDate::Month startMonth = wxDateTime::Jan;
    if (PrefModel::instance().getBudgetFinancialYears())
        budgetDetails.GetFinancialYearValues(startDay, startMonth);
    wxDateTime dtBegin(startDay, startMonth, year);
    wxDateTime dtEnd = dtBegin;
    dtEnd.Add(wxDateSpan::Year()).Subtract(wxDateSpan::Day());

    monthlyBudget_ = (bp_name_n.length() > 5);

    if (monthlyBudget_) {
        budgetDetails.SetBudgetMonth(bp_name_n, dtBegin, dtEnd);
        m_monthName = wxGetTranslation(wxDateTime::GetEnglishMonthName(dtBegin.GetMonth()));
    }

    // Readjust dates by the Budget Offset Option
    PrefModel::instance().addBudgetDateOffset(dtBegin);
    m_budget_offset_date = dtBegin.FormatISODate();   
    PrefModel::instance().addBudgetDateOffset(dtEnd);
    mmDateRange date_range;
    date_range.start_date(dtBegin.ResetTime()); // Start of Day
    date_range.end_date(dtEnd.ResetTime().Add(wxTimeSpan(23,59,59,999))); // End of Day

    //Get statistics
    BudgetModel::instance().getBudgetEntry(budgetYearID_, budgetPeriod_, budgetAmt_, budgetNotes_);
    CategoryModel::instance().getCategoryStats(
        categoryStats_,
        static_cast<wxSharedPtr<wxArrayString>>(nullptr),
        &date_range,
        PrefModel::instance().getIgnoreFutureTransactions(),
        false,
        (evaluateTransfer ? &budgetAmt_ : nullptr)
    );

    //start with only the root categories
    CategoryModel::DataA cat_a = CategoryModel::instance().find(
        CategoryCol::PARENTID(-1)
    );
    std::stable_sort(cat_a.begin(), cat_a.end(), CategoryData::SorterByCATEGNAME());
    for (const auto& cat_d : cat_a) {
        displayDetails_[cat_d.m_id].first = 0;
        double estimated = getEstimate(cat_d.m_id);
        if (estimated < 0)
            estExpenses += estimated;
        else
            estIncome += estimated;

        double actual = 0;
        if (currentView_ != VIEW_PLANNED || estimated != 0)
        {
            actual = categoryStats_[cat_d.m_id][0];
            if (actual < 0)
                actExpenses += actual;
            else
                actIncome += actual;
        }


        budgetTotals_[cat_d.m_id].first = estimated;
        budgetTotals_[cat_d.m_id].second = actual;

        if (DisplayEntryAllowed(cat_d.m_id, -1))
            budget_.emplace_back(cat_d.m_id, -1);

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
            if (currentView_ != VIEW_PLANNED || estimated != 0) {
                actual = categoryStats_[subcat_a[i].m_id][0];
                if (actual < 0)
                    actExpenses += actual;
                else
                    actIncome += actual;
            }
            //save totals for this subcategory
            budgetTotals_[subcat_a[i].m_id].first = estimated;
            budgetTotals_[subcat_a[i].m_id].second = actual;

            //update totals of the category
            budgetTotals_[cat_d.m_id].first += estimated;
            budgetTotals_[cat_d.m_id].second += actual;

            //walk up the hierarchy and update all the parent totals as well
            int64 nextParent = subcat_a[i].m_parent_id_n;
            displayDetails_[subcat_a[i].m_id].first = 1;
            for (int j = i; j > 0; j--) {
                if (subcat_a[j - 1].m_id == nextParent) {
                    displayDetails_[subcat_a[i].m_id].first++;
                    budgetTotals_[subcat_a[j - 1].m_id].first += estimated;
                    budgetTotals_[subcat_a[j - 1].m_id].second += actual;
                    nextParent = subcat_a[j - 1].m_parent_id_n;
                    if (nextParent == cat_d.m_id)
                        break;
                }
            }

            // add the subcategory row to the display list
            if (DisplayEntryAllowed(subcat_a[i].m_id, -1))
                budget_.emplace_back(subcat_a[i].m_id, -1);

            // check if we need to show any total rows before the next subcategory
            if (i < static_cast<int>(subcat_a.size()) - 1) { //not the last subcategory
                //if next subcategory is our child, queue the total for after the children
                if (subcat_a[i].m_id == subcat_a[i + 1].m_parent_id_n) totals_queue.emplace_back(i);
                else if (subcat_a[i].m_parent_id_n != subcat_a[i + 1].m_parent_id_n) {
                    // last sibling -- we've exhausted this branch, so display all the totals we held on to
                    while (!totals_queue.empty() && subcat_a[totals_queue.back()].m_id != subcat_a[i + 1].m_parent_id_n) {
                        if (DisplayEntryAllowed(-1, subcat_a[totals_queue.back()].m_id)) {
                            budget_.emplace_back(-1, subcat_a[totals_queue.back()].m_id);
                            size_t transCatTotalIndex = budget_.size() - 1;
                            m_lc->RefreshItem(transCatTotalIndex);
                        }
                        totals_queue.pop_back();
                    }
                }
            }
            // the very last subcategory, so show the rest of the queued totals
            else {
                while (!totals_queue.empty()) {
                    if (DisplayEntryAllowed(-1, subcat_a[totals_queue.back()].m_id)) {
                        budget_.emplace_back(-1, subcat_a[totals_queue.back()].m_id);
                        size_t transCatTotalIndex = budget_.size() - 1;
                        m_lc->RefreshItem(transCatTotalIndex);
                    }
                    totals_queue.pop_back();
                }
            }
        }

        // show the total of the category after all subcats have been shown
        if (DisplayEntryAllowed(-1, cat_d.m_id)) {
            budget_.emplace_back(-1, cat_d.m_id);
            size_t transCatTotalIndex = budget_.size() - 1;
            m_lc->RefreshItem(transCatTotalIndex);
        }
    }

    m_lc->SetItemCount(budget_.size());

    wxString est_amount, act_amount, diff_amount;
    est_amount = CurrencyModel::instance().toCurrency(estIncome);
    act_amount = CurrencyModel::instance().toCurrency(actIncome);
    diff_amount = CurrencyModel::instance().toCurrency(actIncome - estIncome);

    income_estimated_->SetLabelText(est_amount);
    income_actual_->SetLabelText(act_amount);
    income_diff_->SetLabelText(diff_amount);

    if (estExpenses < 0.0) estExpenses = -estExpenses;
    if (actExpenses < 0.0) actExpenses = -actExpenses;
    est_amount = CurrencyModel::instance().toCurrency(estExpenses);
    act_amount = CurrencyModel::instance().toCurrency(actExpenses);
    diff_amount = CurrencyModel::instance().toCurrency(estExpenses - actExpenses);

    expenses_estimated_->SetLabelText(est_amount);
    expenses_actual_->SetLabelText(act_amount);
    expenses_diff_->SetLabelText(diff_amount);
    UpdateBudgetHeading();
}

double BudgetPanel::getEstimate(int64 category_id) const
{
    try {
        BudgetFreq freq = budgetPeriod_.at(category_id);
        double amt = budgetAmt_.at(category_id);
        return monthlyBudget_
            ? amt * freq.times_per_month()
            : amt * freq.times_per_year();
    }
    catch (std::out_of_range const& exc) {
        wxASSERT(false);
        wxLogDebug(wxString::FromUTF8(exc.what()));
        return 0.0;
    }
}

void BudgetPanel::DisplayBudgetingDetails(int64 budgetYearID)
{
    this->windowsFreezeThaw();
    budgetYearID_ = budgetYearID;
    RefreshList();
    this->windowsFreezeThaw();
}

void BudgetList::OnListItemSelected(wxListEvent& event)
{
    selectedIndex_ = event.GetIndex();
}

wxString BudgetPanel::getItem(long item, int col_id)
{
    switch (col_id) {
    case BudgetList::LIST_ID_ICON:
        return " ";
    case BudgetList::LIST_ID_CATEGORY: {
        const CategoryData* category_n = CategoryModel::instance().get_id_data_n(
            budget_[item].first > 0 ? budget_[item].first : budget_[item].second
        );
        if (category_n) {
            wxString name = category_n->m_name;
            for (int64 i = displayDetails_[category_n->m_id].first; i > 0; i--) {
                name.Prepend("    ");
            }
            return name;
        }
        return wxEmptyString;
    }
    case BudgetList::LIST_ID_FREQUENCY: {
        if (budget_[item].first >= 0 && displayDetails_[budget_[item].first].second) {
            BudgetFreq freq = budgetPeriod_[budget_[item].first];
            return wxGetTranslation(freq.name());
        }
        return wxEmptyString;
    }
    case BudgetList::LIST_ID_AMOUNT: {
        if (budget_[item].first >= 0 && displayDetails_[budget_[item].first].second) {
            double amt = budgetAmt_[budget_[item].first];
            return CurrencyModel::instance().toCurrency(amt);
        }
        return wxEmptyString;
    }
    case BudgetList::LIST_ID_ESTIMATED: {
        if (budget_[item].first < 0) {
            double estimated = budgetTotals_[budget_[item].second].first;
            return CurrencyModel::instance().toCurrency(estimated);
        }
        else if (displayDetails_[budget_[item].first].second) {
            double estimated = getEstimate(budget_[item].first);
            return CurrencyModel::instance().toCurrency(estimated);
        }
        return wxEmptyString;
    }
    case BudgetList::LIST_ID_ACTUAL: {
        if (budget_[item].first < 0) {
            double actual = budgetTotals_[budget_[item].second].second;
            return CurrencyModel::instance().toCurrency(actual);
        }
        else if (displayDetails_[budget_[item].first].second) {
            double actual = categoryStats_[budget_[item].second >= 0 ? budget_[item].second
                : budget_[item].first][0];
            return CurrencyModel::instance().toCurrency(actual);
        }
        return wxEmptyString;
    }
    case BudgetList::LIST_ID_NOTES:
        if (budget_[item].first >= 0 && displayDetails_[budget_[item].first].second) {
            wxString value = budgetNotes_[budget_[item].second >= 0 ? budget_[item].second
                : budget_[item].first];
            value.Replace("\n", " ");
            return value;
        }
        return wxEmptyString;
    default:
        return wxEmptyString;
    }
}

int BudgetList::OnGetItemImage(long item) const
{
    return cp_->GetItemImage(item);
}

int BudgetPanel::GetItemImage(long item) const
{
    try
    {

        double estimated = 0;
        double actual = 0;
        if (budget_[item].first < 0)
        {
            estimated = budgetTotals_.at(budget_[item].second).first;
            actual = budgetTotals_.at(budget_[item].second).second;
        }
        else
        {
            estimated = getEstimate(budget_[item].second >= 0 ? budget_[item].second
            : budget_[item].first);
            actual = categoryStats_.at(budget_[item].second >= 0 ? budget_[item].second
                : budget_[item].first).at(0);
        }

        if ((estimated == 0.0) && (actual == 0.0)) return -1;
        if ((estimated == 0.0) && (actual != 0.0)) return ICON_FOLLOWUP;
        if (estimated < actual) return ICON_RECONCILLED;
        if (std::fabs(estimated - actual) < 0.001) return ICON_RECONCILLED;
        return ICON_VOID;
    }
    catch (std::out_of_range const& exc)
    {
        wxASSERT(false);
        wxLogDebug(wxString::FromUTF8(exc.what()));
        return 1;
    }
}

wxString BudgetList::OnGetItemText(long item, long col_nr) const
{
    return cp_->getItem(item, getColId_Nr(static_cast<int>(col_nr)));
}

wxListItemAttr* BudgetList::OnGetItemAttr(long item) const
{
    if ((cp_->GetTransID(item) < 0) &&
        (cp_->GetCurrentView() != VIEW_SUMM))
    {
        return attr3_.get();
    }

    /* Returns the alternating background pattern */
    return (item % 2) ? attr2_.get() : attr1_.get();
}

void BudgetList::OnListItemActivated(wxListEvent& event)
{
    selectedIndex_ = event.GetIndex();
    cp_->OnListItemActivated(selectedIndex_);
}

void BudgetPanel::OnListItemActivated(int selectedIndex)
{
    /***************************************************************************
     A TOTALS entry does not contain a budget entry, therefore ignore the event.
     ***************************************************************************/
    BudgetModel::DataA budget_a = BudgetModel::instance().find(
        BudgetCol::BUDGETYEARID(GetBudgetYearID()),
        BudgetCol::CATEGID(budget_[selectedIndex].second > 0
            ? budget_[selectedIndex].second
            : budget_[selectedIndex].first
        )
    );

    if (budget_[selectedIndex].first == -1)
        return;

    BudgetData budget_d = BudgetData();
    if (budget_a.empty()) {
        budget_d = BudgetData();
        budget_d.m_period_id   = GetBudgetYearID();
        budget_d.m_category_id = budget_[selectedIndex].first;
        budget_d.m_amount      = 0.0;
        BudgetModel::instance().add_data_n(budget_d);
    }
    else
        budget_d = budget_a[0];

    double estimated = getEstimate(budget_[selectedIndex].second >= 0
        ? budget_[selectedIndex].second
        : budget_[selectedIndex].first
    );
    double actual = categoryStats_[budget_[selectedIndex].second >= 0
        ? budget_[selectedIndex].second
        : budget_[selectedIndex].first
    ][0];

    BudgetEntryDialog dlg(this, &budget_d,
        CurrencyModel::instance().toCurrency(estimated),
        CurrencyModel::instance().toCurrency(actual)
    );
    if (dlg.ShowModal() == wxID_OK) {
        initVirtualListControl();
        m_lc->Refresh();
        m_lc->Update();
        m_lc->EnsureVisible(selectedIndex);
    }
}
/* ===================== Tooltip logic ===================== */

void BudgetList::OnMouseMove(wxMouseEvent& event)
{
    long row = -1;
    long column = -1;
    int flags = 0;

    row = HitTest(event.GetPosition(), flags, &column);

    if (LIST_ID_ICON == column && row >= 0) {
        wxString tooltip;
        int icon = cp_->GetItemImage(row);

        if (-1 == icon)
            tooltip = _("No budget defined");
        else if (BudgetPanel::ICON_VOID == icon)
            tooltip = _("Critical: budget exceeded! Stop spending");
        else if (BudgetPanel::ICON_RECONCILLED == icon)
            tooltip = _("Within budget limits");
        else if (BudgetPanel::ICON_FOLLOWUP == icon)
            tooltip = _("Alert: budget close to or over limit");

        mmToolTip(this, tooltip);
    }
    else {
        UnsetToolTip();
    }

    event.Skip();
}

