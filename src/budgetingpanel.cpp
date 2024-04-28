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

#include "budgetingpanel.h"
#include "budgetentrydialog.h"
#include "images_list.h"
#include "option.h"
#include "mmex.h"
#include "mmframe.h"
#include "reports/budget.h"
#include "reports/mmDateRange.h"
#include "model/allmodel.h"

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
    MENU_HEADER_HIDE,
    MENU_HEADER_SORT,
    MENU_HEADER_RESET,
};

static const wxString VIEW_ALL = wxTRANSLATE("View All Budget Categories");
static const wxString VIEW_NON_ZERO = wxTRANSLATE("View Non-Zero Budget Categories");
static const wxString VIEW_PLANNED = wxTRANSLATE("View Planned Budget Categories");
static const wxString VIEW_INCOME = wxTRANSLATE("View Income Budget Categories");
static const wxString VIEW_EXPENSE = wxTRANSLATE("View Expense Budget Categories");
static const wxString VIEW_SUMM = wxTRANSLATE("View Budget Category Summary");

/*******************************************************/
wxBEGIN_EVENT_TABLE(mmBudgetingPanel, wxPanel)
EVT_BUTTON(wxID_FILE2, mmBudgetingPanel::OnMouseLeftDown)
EVT_MENU(wxID_ANY, mmBudgetingPanel::OnViewPopupSelected)
wxEND_EVENT_TABLE()
/*******************************************************/
wxBEGIN_EVENT_TABLE(budgetingListCtrl, mmListCtrl)
EVT_LIST_ITEM_SELECTED(wxID_ANY, budgetingListCtrl::OnListItemSelected)
EVT_LIST_ITEM_ACTIVATED(wxID_ANY, budgetingListCtrl::OnListItemActivated)
wxEND_EVENT_TABLE()
/*******************************************************/
mmBudgetingPanel::mmBudgetingPanel(int budgetYearID
    , wxWindow *parent, mmGUIFrame *frame
    , wxWindowID winid
    , const wxPoint& pos, const wxSize& size
    , long style, const wxString& name)
    : m_frame(frame)
    , listCtrlBudget_(nullptr)
    , budgetYearID_(budgetYearID)
{
    Create(parent, winid, pos, size, style, name);
}

bool mmBudgetingPanel::Create(wxWindow *parent
    , wxWindowID winid, const wxPoint& pos
    , const wxSize& size, long style, const wxString& name)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create(parent, winid, pos, size, style, name);

    this->windowsFreezeThaw();
    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    initVirtualListControl();
    if (!budget_.empty())
        listCtrlBudget_->EnsureVisible(0);

    this->windowsFreezeThaw();
    Model_Usage::instance().pageview(this);
    return true;
}

mmBudgetingPanel::~mmBudgetingPanel()
{

}

void mmBudgetingPanel::OnViewPopupSelected(wxCommandEvent& event)
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
    else
        wxASSERT(false);

    Model_Infotable::instance().Set("BUDGET_FILTER", currentView_);

    RefreshList();
}

void mmBudgetingPanel::RefreshList()
{
    initVirtualListControl();
    listCtrlBudget_->Refresh();
    listCtrlBudget_->Update();
    if (!budget_.empty())
        listCtrlBudget_->EnsureVisible(0);
}

void mmBudgetingPanel::OnMouseLeftDown(wxCommandEvent& event)
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

wxString mmBudgetingPanel::GetPanelTitle() const
{
    wxString yearStr = Model_Budgetyear::instance().Get(budgetYearID_);
    if ((yearStr.length() < 5))
    {
        if (Option::instance().BudgetFinancialYears())
        {
            long year;
            yearStr.ToLong(&year);
            year++;
            yearStr = wxString::Format(_("Financial Year: %s - %li"), yearStr, year);
        }
        else
        {
            yearStr = wxString::Format(_("Year: %s"), yearStr);
        }
    }
    else
    {
        yearStr = wxString::Format(_("Month: %s"), yearStr);
        yearStr += wxString::Format(" (%s)", m_monthName);
    }

    if (Option::instance().getBudgetDaysOffset() != 0)
    {
        yearStr = wxString::Format(_("%1$s    Start Date of: %2$s"), yearStr, mmGetDateForDisplay(m_budget_offset_date));
    }

    return wxString::Format(_("Budget Planner for %s"), yearStr);
}

void mmBudgetingPanel::UpdateBudgetHeading()
{
    budgetReportHeading_->SetLabel(GetPanelTitle());
    m_bitmapTransFilter->SetLabel(wxGetTranslation(currentView_));
}

void mmBudgetingPanel::CreateControls()
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

    itemIncomeSizer->Add(new wxStaticText(itemPanel3, wxID_STATIC, _("Income: ")));
    itemIncomeSizer->Add(new wxStaticText(itemPanel3, wxID_STATIC, _("Estimated: ")));
    itemIncomeSizer->Add(income_estimated_);
    itemIncomeSizer->Add(new wxStaticText(itemPanel3, wxID_STATIC, _("Actual: ")));
    itemIncomeSizer->Add(income_actual_);
    itemIncomeSizer->Add(new wxStaticText(itemPanel3, wxID_STATIC, _("Difference: ")));
    itemIncomeSizer->Add(income_diff_);

    itemIncomeSizer->Add(new wxStaticText(itemPanel3, wxID_STATIC, _("Expenses: ")));
    itemIncomeSizer->Add(new wxStaticText(itemPanel3, wxID_STATIC, _("Estimated: ")));
    itemIncomeSizer->Add(expenses_estimated_);
    itemIncomeSizer->Add(new wxStaticText(itemPanel3, wxID_STATIC, _("Actual: ")));
    itemIncomeSizer->Add(expenses_actual_);
    itemIncomeSizer->Add(new wxStaticText(itemPanel3, wxID_STATIC, _("Difference: ")));
    itemIncomeSizer->Add(expenses_diff_);
    /* ---------------------- */

    wxVector<wxBitmapBundle> images;
    images.push_back(mmBitmapBundle(png::RECONCILED));
    images.push_back(mmBitmapBundle(png::VOID_STAT));
    images.push_back(mmBitmapBundle(png::FOLLOW_UP));

    listCtrlBudget_ = new budgetingListCtrl(this, this, wxID_ANY);

    listCtrlBudget_->SetSmallImages(images);
    listCtrlBudget_->InsertColumn(COL_ICON, (" "));
    listCtrlBudget_->InsertColumn(COL_CATEGORY, listCtrlBudget_->m_columns[COL_CATEGORY].HEADER);
    listCtrlBudget_->InsertColumn(COL_FREQUENCY, listCtrlBudget_->m_columns[COL_FREQUENCY].HEADER);
    listCtrlBudget_->InsertColumn(COL_AMOUNT, listCtrlBudget_->m_columns[COL_AMOUNT].HEADER, wxLIST_FORMAT_RIGHT);
    listCtrlBudget_->InsertColumn(COL_ESTIMATED, listCtrlBudget_->m_columns[COL_ESTIMATED].HEADER, wxLIST_FORMAT_RIGHT);
    listCtrlBudget_->InsertColumn(COL_ACTUAL, listCtrlBudget_->m_columns[COL_ACTUAL].HEADER, wxLIST_FORMAT_RIGHT);
    listCtrlBudget_->InsertColumn(COL_NOTES, listCtrlBudget_->m_columns[COL_NOTES].HEADER, wxLIST_FORMAT_LEFT);

    /* Get data from inidb */
    for (int i = 0; i < listCtrlBudget_->GetColumnCount(); ++i)
    {
        int col_width = Model_Setting::instance().GetIntSetting(wxString::Format(listCtrlBudget_->m_col_width, i)
            , listCtrlBudget_->m_columns[i].WIDTH);
        listCtrlBudget_->SetColumnWidth(i, col_width);
    }
    itemBoxSizer2->Add(listCtrlBudget_.get(), 1, wxGROW | wxALL, 1);
}

budgetingListCtrl::budgetingListCtrl(mmBudgetingPanel* cp, wxWindow *parent, const wxWindowID id)
    : mmListCtrl(parent, id)
    , attr3_(new wxListItemAttr(wxNullColour, mmThemeMetaColour(meta::COLOR_LISTTOTAL), wxNullFont))
    , cp_(cp)
{
    mmThemeMetaColour(this, meta::COLOR_LISTPANEL);

    m_columns.push_back(PANEL_COLUMN(_("Icon"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT, false));
    m_columns.push_back(PANEL_COLUMN(_("Category"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_RIGHT, true));
    m_columns.push_back(PANEL_COLUMN(_("Frequency"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_RIGHT, true));
    m_columns.push_back(PANEL_COLUMN(_("Amount"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_RIGHT, true));
    m_columns.push_back(PANEL_COLUMN(_("Estimated"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_RIGHT, true));
    m_columns.push_back(PANEL_COLUMN(_("Actual"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_RIGHT, true));
    m_columns.push_back(PANEL_COLUMN(_("Notes"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT, true));

    m_col_width = "BUDGET_COL%d_WIDTH";
}

void mmBudgetingPanel::sortTable()
{
    //TODO: Sort budget panel
}

bool mmBudgetingPanel::DisplayEntryAllowed(int categoryID, int subcategoryID)
{
    bool result = false;

    double actual = 0;
    double estimated = 0;
    if (categoryID < 0)
    {
        actual = budgetTotals_[subcategoryID].second;
        estimated = budgetTotals_[subcategoryID].first;
    }
    else
    {
        actual = categoryStats_[categoryID][0];
        estimated = getEstimate(categoryID);
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
        result = (categoryID < 0);
    else
        result = true;

    if (categoryID > 0) {
        displayDetails_[categoryID].second = result;
        for (const auto& subcat : Model_Category::sub_tree(Model_Category::instance().get(categoryID))) {
            result = result || DisplayEntryAllowed(subcat.CATEGID, -1);
        }
    }
    return result;
}

void mmBudgetingPanel::initVirtualListControl()
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
    if (Option::instance().BudgetIncludeTransfers())
    {
        evaluateTransfer = true;
    }

    currentView_ = Model_Infotable::instance().GetStringInfo("BUDGET_FILTER", VIEW_ALL);
    const wxString budgetYearStr = Model_Budgetyear::instance().Get(budgetYearID_);
    long year = 0;
    budgetYearStr.ToLong(&year);

    int startDay = 1;
    wxDate::Month startMonth = wxDateTime::Jan;
    if (Option::instance().BudgetFinancialYears())
        budgetDetails.GetFinancialYearValues(startDay, startMonth);
    wxDateTime dtBegin(startDay, startMonth, year);
    wxDateTime dtEnd = dtBegin;
    dtEnd.Add(wxDateSpan::Year()).Subtract(wxDateSpan::Day());

    monthlyBudget_ = (budgetYearStr.length() > 5);

    if (monthlyBudget_)
    {
        budgetDetails.SetBudgetMonth(budgetYearStr, dtBegin, dtEnd);
        m_monthName = wxGetTranslation(wxDateTime::GetEnglishMonthName(dtBegin.GetMonth()));
    }

    // Readjust dates by the Budget Offset Option
    Option::instance().setBudgetDateOffset(dtBegin);
    m_budget_offset_date = dtBegin.FormatISODate();
    Option::instance().setBudgetDateOffset(dtEnd);
    mmSpecifiedRange date_range(dtBegin, dtEnd);

    //Get statistics
    Model_Budget::instance().getBudgetEntry(budgetYearID_, budgetPeriod_, budgetAmt_, budgetNotes_);
    Model_Category::instance().getCategoryStats(categoryStats_
        , static_cast<wxSharedPtr<wxArrayString>>(nullptr)
        , &date_range, Option::instance().getIgnoreFutureTransactions()
        , false, (evaluateTransfer ? &budgetAmt_ : 0));

    //start with only the root categories
    Model_Category::Data_Set categories = Model_Category::instance().find(Model_Category::PARENTID(-1));
    std::stable_sort(categories.begin(), categories.end(), SorterByCATEGNAME());
    for (const auto& category : categories)
    {
        displayDetails_[category.CATEGID].first = 0;
        double estimated = getEstimate(category.CATEGID);
        if (estimated < 0)
            estExpenses += estimated;
        else
            estIncome += estimated;

        double actual = 0;
        if (currentView_ != VIEW_PLANNED || estimated != 0)
        {
            actual = categoryStats_[category.CATEGID][0];
            if (actual < 0)
                actExpenses += actual;
            else
                actIncome += actual;
        }
        

        budgetTotals_[category.CATEGID].first = estimated;
        budgetTotals_[category.CATEGID].second = actual;

        if (DisplayEntryAllowed(category.CATEGID, -1))
            budget_.push_back(std::make_pair(category.CATEGID, -1));

        std::vector<int> totals_queue;
        //now a depth-first walk of the subtree of this root category
        Model_Category::Data_Set subcats = Model_Category::sub_tree(category);
        for (int i = 0; i < static_cast<int>(subcats.size()); i++)
        {
            estimated = getEstimate(subcats[i].CATEGID);
            if (estimated < 0)
                estExpenses += estimated;
            else
                estIncome += estimated;

            actual = 0;
            if (currentView_ != VIEW_PLANNED || estimated != 0)
            {
                actual = categoryStats_[subcats[i].CATEGID][0];
                if (actual < 0)
                    actExpenses += actual;
                else
                    actIncome += actual;
            }
            //save totals for this subcategory
            budgetTotals_[subcats[i].CATEGID].first = estimated;
            budgetTotals_[subcats[i].CATEGID].second = actual;

            //update totals of the category
            budgetTotals_[category.CATEGID].first += estimated;
            budgetTotals_[category.CATEGID].second += actual;

            //walk up the hierarchy and update all the parent totals as well
            int nextParent = subcats[i].PARENTID;
            displayDetails_[subcats[i].CATEGID].first = 1;
            for (int j = i; j > 0; j--) {
                if (subcats[j - 1].CATEGID == nextParent) {
                    displayDetails_[subcats[i].CATEGID].first++;
                    budgetTotals_[subcats[j - 1].CATEGID].first += estimated;
                    budgetTotals_[subcats[j - 1].CATEGID].second += actual;
                    nextParent = subcats[j - 1].PARENTID;
                    if (nextParent == category.CATEGID)
                        break;
                }
            }

            // add the subcategory row to the display list
            if (DisplayEntryAllowed(subcats[i].CATEGID, -1))
                budget_.push_back(std::make_pair(subcats[i].CATEGID, -1));

            // check if we need to show any total rows before the next subcategory
            if (i < static_cast<int>(subcats.size()) - 1) { //not the last subcategory
                if (subcats[i].CATEGID == subcats[i + 1].PARENTID) totals_queue.push_back(i); //if next subcategory is our child, queue the total for after the children
                else if (subcats[i].PARENTID != subcats[i + 1].PARENTID) { // last sibling -- we've exhausted this branch, so display all the totals we held on to
                    while (!totals_queue.empty() && subcats[totals_queue.back()].CATEGID != subcats[i + 1].PARENTID) {
                        if (DisplayEntryAllowed(-1, subcats[totals_queue.back()].CATEGID))
                        {
                            budget_.push_back(std::make_pair(-1, subcats[totals_queue.back()].CATEGID));
                            size_t transCatTotalIndex = budget_.size() - 1;
                            listCtrlBudget_->RefreshItem(transCatTotalIndex);
                        }
                        totals_queue.pop_back();
                    }
                }
            }
            // the very last subcategory, so show the rest of the queued totals
            else {
                while (!totals_queue.empty()) {
                    if (DisplayEntryAllowed(-1, subcats[totals_queue.back()].CATEGID))
                    {
                        budget_.push_back(std::make_pair(-1, subcats[totals_queue.back()].CATEGID));
                        size_t transCatTotalIndex = budget_.size() - 1;
                        listCtrlBudget_->RefreshItem(transCatTotalIndex);
                    }
                    totals_queue.pop_back();
                }
            }
        }

        // show the total of the category after all subcats have been shown
        if (DisplayEntryAllowed(-1, category.CATEGID))
        {
            budget_.push_back(std::make_pair(-1, category.CATEGID));
            size_t transCatTotalIndex = budget_.size() - 1;
            listCtrlBudget_->RefreshItem(transCatTotalIndex);
        }
    }

    listCtrlBudget_->SetItemCount(budget_.size());

    wxString est_amount, act_amount, diff_amount;
    est_amount = Model_Currency::toCurrency(estIncome);
    act_amount = Model_Currency::toCurrency(actIncome);
    diff_amount = Model_Currency::toCurrency(actIncome - estIncome);

    income_estimated_->SetLabelText(est_amount);
    income_actual_->SetLabelText(act_amount);
    income_diff_->SetLabelText(diff_amount);

    if (estExpenses < 0.0) estExpenses = -estExpenses;
    if (actExpenses < 0.0) actExpenses = -actExpenses;
    est_amount = Model_Currency::toCurrency(estExpenses);
    act_amount = Model_Currency::toCurrency(actExpenses);
    diff_amount = Model_Currency::toCurrency(estExpenses - actExpenses);

    expenses_estimated_->SetLabelText(est_amount);
    expenses_actual_->SetLabelText(act_amount);
    expenses_diff_->SetLabelText(diff_amount);
    UpdateBudgetHeading();
}

double mmBudgetingPanel::getEstimate(int category) const
{
    try
    {
        Model_Budget::PERIOD_ENUM period = budgetPeriod_.at(category);
        double amt = budgetAmt_.at(category);
        return Model_Budget::getEstimate(monthlyBudget_, period, amt);
    }
    catch (std::out_of_range const& exc)
    {
        wxASSERT(false);
        wxLogDebug(wxString::FromUTF8(exc.what()));
        return 0.0;
    }
}

void mmBudgetingPanel::DisplayBudgetingDetails(int budgetYearID)
{
    this->windowsFreezeThaw();
    budgetYearID_ = budgetYearID;
    RefreshList();
    this->windowsFreezeThaw();
}

void budgetingListCtrl::OnListItemSelected(wxListEvent& event)
{
    selectedIndex_ = event.GetIndex();
}

wxString mmBudgetingPanel::getItem(long item, long column)
{
    switch (column)
    {
    case COL_ICON:
        return " ";
    case COL_CATEGORY:
    {
        Model_Category::Data* category = Model_Category::instance().get(budget_[item].first > 0
            ? budget_[item].first : budget_[item].second);
        if (category) {
            wxString name = category->CATEGNAME;
            for (int i = displayDetails_[category->CATEGID].first; i > 0; i--) {
                name.Prepend("    ");
            }
            return name;
        }
        return wxEmptyString;
    }
    case COL_FREQUENCY:
    {
        if (budget_[item].first >= 0 && displayDetails_[budget_[item].first].second)
            return Model_Budget::all_period()[budgetPeriod_[budget_[item].first]];
        return wxEmptyString;
    }
    case COL_AMOUNT:
    {
        if (budget_[item].first >= 0 && displayDetails_[budget_[item].first].second)
        {
            double amt = budgetAmt_[budget_[item].first];
            return Model_Currency::toCurrency(amt);
        }
        return wxEmptyString;
    }
    case COL_ESTIMATED:
    {
        if (budget_[item].first < 0)
        {
            double estimated = budgetTotals_[budget_[item].second].first;
            return Model_Currency::toCurrency(estimated);
        }
        else if (displayDetails_[budget_[item].first].second)
        {
            double estimated = getEstimate(budget_[item].first);
            return Model_Currency::toCurrency(estimated);
        }
        return wxEmptyString;
    }
    case COL_ACTUAL:
    {
        if (budget_[item].first < 0)
        {
            double actual = budgetTotals_[budget_[item].second].second;
            return Model_Currency::toCurrency(actual);
        }
        else if (displayDetails_[budget_[item].first].second)
        {
            double actual = categoryStats_[budget_[item].second >= 0 ? budget_[item].second
                : budget_[item].first][0];
            return Model_Currency::toCurrency(actual);
        }
        return wxEmptyString;
    }
    case COL_NOTES:
        if (budget_[item].first >= 0 && displayDetails_[budget_[item].first].second)
        {
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

int budgetingListCtrl::OnGetItemImage(long item) const
{
    return cp_->GetItemImage(item);
}

int mmBudgetingPanel::GetItemImage(long item) const
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

wxString budgetingListCtrl::OnGetItemText(long item, long column) const
{
    return cp_->getItem(item, column);
}

wxListItemAttr* budgetingListCtrl::OnGetItemAttr(long item) const
{
    if ((cp_->GetTransID(item) < 0) &&
        (cp_->GetCurrentView() != VIEW_SUMM))
    {
        return attr3_.get();
    }

    /* Returns the alternating background pattern */
    return (item % 2) ? attr2_.get() : attr1_.get();
}

void budgetingListCtrl::OnListItemActivated(wxListEvent& event)
{
    selectedIndex_ = event.GetIndex();
    cp_->OnListItemActivated(selectedIndex_);
}

void mmBudgetingPanel::OnListItemActivated(int selectedIndex)
{
    /***************************************************************************
     A TOTALS entry does not contain a budget entry, therefore ignore the event.
     ***************************************************************************/
    Model_Budget::Data_Set budget = Model_Budget::instance().find(Model_Budget::BUDGETYEARID(GetBudgetYearID())
        , Model_Budget::CATEGID(budget_[selectedIndex].second > 0 ? budget_[selectedIndex].second : budget_[selectedIndex].first));

    if (budget_[selectedIndex].first == -1)
        return;

    Model_Budget::Data* entry = 0;
    if (budget.empty())
    {
        entry = Model_Budget::instance().create();
        entry->BUDGETYEARID = GetBudgetYearID();
        entry->CATEGID = budget_[selectedIndex].first;
        entry->PERIOD = "";
        entry->AMOUNT = 0.0;
        entry->ACTIVE = 1;
        Model_Budget::instance().save(entry);
    }
    else
        entry = &budget[0];

    double estimated = getEstimate(budget_[selectedIndex].second >= 0 ? budget_[selectedIndex].second
        : budget_[selectedIndex].first);
    double actual = categoryStats_[budget_[selectedIndex].second >= 0 ? budget_[selectedIndex].second
        : budget_[selectedIndex].first][0];

    mmBudgetEntryDialog dlg(this, entry, Model_Currency::toCurrency(estimated), Model_Currency::toCurrency(actual));
    if (dlg.ShowModal() == wxID_OK)
    {
        initVirtualListControl();
        listCtrlBudget_->Refresh();
        listCtrlBudget_->Update();
        listCtrlBudget_->EnsureVisible(selectedIndex);
    }
}
