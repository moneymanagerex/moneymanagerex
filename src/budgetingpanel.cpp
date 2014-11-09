/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2012 Stefano Giorgio

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
#include "mmOption.h"
#include "mmex.h"
#include "mmframe.h"
#include "reports/budget.h"
#include "reports/mmDateRange.h"
#include "model/Model_Setting.h"
#include "model/Model_Infotable.h"
#include "model/Model_Budgetyear.h"
#include "model/Model_Category.h"

#include "../resources/rightarrow.xpm"
#include "../resources/void.xpm"
#include "../resources/flag.xpm"
#include "../resources/empty.xpm"
#include "../resources/reconciled.xpm"


enum
{
    ID_DIALOG_BUDGETENTRY_SUMMARY_INCOME_EST = wxID_HIGHEST + 1400,
    MENU_VIEW_ALLBUDGETENTRIES,
    MENU_VIEW_PLANNEDBUDGETENTRIES,
    MENU_VIEW_NONZEROBUDGETENTRIES,
    MENU_VIEW_INCOMEBUDGETENTRIES,
    MENU_VIEW_SUMMARYBUDGETENTRIES,
    MENU_VIEW_EXPENSEBUDGETENTRIES,
    ID_PANEL_BUDGETENTRY_STATIC_BITMAP_VIEW,
    ID_PANEL_CHECKING_STATIC_PANELVIEW,
    ID_PANEL_REPORTS_HEADER_PANEL,
    ID_DIALOG_BUDGETENTRY_SUMMARY_INCOME_ACT,
    ID_DIALOG_BUDGETENTRY_SUMMARY_INCOME_DIF,
    ID_DIALOG_BUDGETENTRY_SUMMARY_EXPENSES_EST,
    ID_DIALOG_BUDGETENTRY_SUMMARY_EXPENSES_ACT,
    ID_DIALOG_BUDGETENTRY_SUMMARY_EXPENSES_DIF,
};

/*******************************************************/
wxBEGIN_EVENT_TABLE(mmBudgetingPanel, wxPanel)
    EVT_LEFT_DOWN(mmBudgetingPanel::OnMouseLeftDown)
    EVT_MENU(wxID_ANY, mmBudgetingPanel::OnViewPopupSelected)
wxEND_EVENT_TABLE()
/*******************************************************/
wxBEGIN_EVENT_TABLE(budgetingListCtrl, wxListCtrl)
    EVT_LIST_ITEM_SELECTED(wxID_ANY, budgetingListCtrl::OnListItemSelected)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY, budgetingListCtrl::OnListItemActivated)
    EVT_LIST_COL_END_DRAG(wxID_ANY, budgetingListCtrl::OnItemResize)
wxEND_EVENT_TABLE()
/*******************************************************/
mmBudgetingPanel::mmBudgetingPanel(int budgetYearID,
    wxWindow *parent, mmGUIFrame* frame, wxWindowID winid,
    const wxPoint& pos, const wxSize& size,
    long style,const wxString& name)
    : m_imageList(nullptr)
    , listCtrlBudget_(nullptr)
    , budgetYearID_(budgetYearID)
    , m_frame(frame)
    , budgetReportHeading_(nullptr)
    , income_estimated_(nullptr)
    , income_actual_(nullptr)
    , income_diff_(nullptr)
    , expenses_estimated_(nullptr)
    , expenses_actual_(nullptr)
    , expenses_diff_(nullptr)
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
    return TRUE;
}

mmBudgetingPanel::~mmBudgetingPanel()
{
    if (m_imageList) delete m_imageList;
    m_frame->SetBudgetingPageInactive();
}

void mmBudgetingPanel::save_column_width(int width)
{
    int col_x = listCtrlBudget_->GetColumnWidth(width);
    Model_Setting::instance().Set(wxString::Format("BUDGET_COL%d_WIDTH", width), col_x);
}

void mmBudgetingPanel::OnViewPopupSelected(wxCommandEvent& event)
{
    int evt =  event.GetId();
    if (evt ==  MENU_VIEW_ALLBUDGETENTRIES)
        currentView_ = wxTRANSLATE("View All Budget Categories");
    else if (evt == MENU_VIEW_NONZEROBUDGETENTRIES)
        currentView_ = wxTRANSLATE("View Non-Zero Budget Categories");
    else if (evt == MENU_VIEW_PLANNEDBUDGETENTRIES)
        currentView_ = wxTRANSLATE("View Planned Budget Categories");
    else if (evt == MENU_VIEW_INCOMEBUDGETENTRIES)
        currentView_ = wxTRANSLATE("View Income Budget Categories");
    else if (evt == MENU_VIEW_EXPENSEBUDGETENTRIES)
        currentView_ = wxTRANSLATE("View Expense Budget Categories");
    else if (evt == MENU_VIEW_SUMMARYBUDGETENTRIES)
        currentView_ = wxTRANSLATE("View Budget Category Summary");
    else
        wxASSERT(false);

    Model_Infotable::instance().Set("BUDGET_FILTER", currentView_);

    RefreshList();
}

void mmBudgetingPanel::RefreshList()
{
    initVirtualListControl();
    listCtrlBudget_->Refresh();
    if (!budget_.empty())
        listCtrlBudget_->EnsureVisible(0);
}

void mmBudgetingPanel::OnMouseLeftDown(wxMouseEvent& event)
{
    // depending on the clicked control's window id.
    switch (event.GetId())
    {
        case ID_PANEL_BUDGETENTRY_STATIC_BITMAP_VIEW :
        {
            wxMenu menu;
            menu.Append(MENU_VIEW_ALLBUDGETENTRIES, _("View All Budget Categories"));
            menu.Append(MENU_VIEW_PLANNEDBUDGETENTRIES, _("View Planned Budget Categories"));
            menu.Append(MENU_VIEW_NONZEROBUDGETENTRIES, _("View Non-Zero Budget Categories"));
            menu.Append(MENU_VIEW_INCOMEBUDGETENTRIES, _("View Income Budget Categories"));
            menu.Append(MENU_VIEW_EXPENSEBUDGETENTRIES, _("View Expense Budget Categories"));
            menu.AppendSeparator();
            menu.Append(MENU_VIEW_SUMMARYBUDGETENTRIES, _("View Budget Category Summary"));
            PopupMenu(&menu, event.GetPosition());
            break;
        }
    }
    event.Skip();
}

wxString mmBudgetingPanel::GetPanelTitle() const
{
    wxString yearStr = Model_Budgetyear::instance().Get(budgetYearID_);
    if ((yearStr.length() < 5))
    {
        if (mmIniOptions::instance().budgetFinancialYears_)
        {
            long year;
            yearStr.ToLong(&year);
            year++;
            yearStr = wxString::Format(_("Financial Year: %s - %i"), yearStr, year);
        }
        else
        {
            yearStr = wxString::Format(_("Year: %s"), yearStr);
        }
    }
    else
    {
        yearStr = wxString::Format(_("Month: %s"), yearStr);
    }
    return wxString::Format(_("Budget Setup for %s"), yearStr);
}

void mmBudgetingPanel::UpdateBudgetHeading()
{
    budgetReportHeading_->SetLabel(GetPanelTitle());

    wxStaticText* header = (wxStaticText*)FindWindow(ID_PANEL_CHECKING_STATIC_PANELVIEW);
    header->SetLabel(wxGetTranslation(currentView_));
}

void mmBudgetingPanel::CreateControls()
{
    wxSizerFlags flags;
    flags.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).Border(wxLEFT|wxTOP, 4);

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

    wxBitmap itemStaticBitmap3Bitmap(wxBitmap(wxImage(rightarrow_xpm).Scale(16,16)));
    wxStaticBitmap* itemStaticBitmap3 = new wxStaticBitmap(itemPanel3
        , ID_PANEL_BUDGETENTRY_STATIC_BITMAP_VIEW
        , itemStaticBitmap3Bitmap, wxDefaultPosition, wxSize(16, 16), 0);
    itemStaticBitmap3->Connect(ID_PANEL_BUDGETENTRY_STATIC_BITMAP_VIEW, wxEVT_LEFT_DOWN
        , wxMouseEventHandler(mmBudgetingPanel::OnMouseLeftDown), nullptr, this);
    itemBoxSizerHHeader2->Add(itemStaticBitmap3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxStaticText* itemStaticText18 = new wxStaticText(itemPanel3
        , ID_PANEL_CHECKING_STATIC_PANELVIEW, "");
    itemBoxSizerHHeader2->Add(itemStaticText18, 0, wxALL, 1);

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

    wxSize imageSize(16, 16);
    m_imageList = new wxImageList(imageSize.GetWidth(), imageSize.GetHeight());
    m_imageList->Add(wxBitmap(reconciled_xpm));
    m_imageList->Add(wxBitmap(void_xpm));
    m_imageList->Add(wxBitmap(flag_xpm));
    m_imageList->Add(wxBitmap(empty_xpm));

    listCtrlBudget_ = new budgetingListCtrl(this, this, wxID_ANY);

    listCtrlBudget_->SetImageList(m_imageList, wxIMAGE_LIST_SMALL);
    listCtrlBudget_->InsertColumn(0, _("Category"));
    listCtrlBudget_->InsertColumn(1, _("Sub Category"));
    listCtrlBudget_->InsertColumn(2, _("Frequency"));
    listCtrlBudget_->InsertColumn(3, _("Amount"), wxLIST_FORMAT_RIGHT);
    listCtrlBudget_->InsertColumn(4, _("Estimated"), wxLIST_FORMAT_RIGHT);
    listCtrlBudget_->InsertColumn(5, _("Actual"), wxLIST_FORMAT_RIGHT);

    /* Get data from inidb */
    for (int i = 0; i < listCtrlBudget_->GetColumnCount(); ++i)
    {
        int col = Model_Setting::instance().GetIntSetting(wxString::Format("BUDGET_COL%d_WIDTH", i), 80);
        listCtrlBudget_->SetColumnWidth(i, col);
    }
    itemBoxSizer2->Add(listCtrlBudget_, 1, wxGROW | wxALL, 1);
}

void mmBudgetingPanel::sortTable()
{
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
        actual = categoryStats_[categoryID][subcategoryID][0];
        estimated = getEstimate(categoryID, subcategoryID);
    }

    if (currentView_ == "View Non-Zero Budget Categories")
        result = ((estimated != 0.0) || (actual != 0.0));
    else if (currentView_ == "View Income Budget Categories")
        result = ((estimated > 0.0) || (actual > 0.0));
    else if (currentView_ == "View Planned Budget Categories")
        result = (estimated != 0.0);
    else if (currentView_ == "View Expense Budget Categories")
        result = ((estimated < 0.0) || (actual < 0.0));
    else if (currentView_ == "View Budget Category Summary")
        result = ((categoryID < 0.0));
    else
        result = true;

    return result;
}

void mmBudgetingPanel::initVirtualListControl()
{
    budget_.clear();
    budgetTotals_.clear();
    budgetPeriod_.clear();
    budgetAmt_.clear();
    categoryStats_.clear();
    double estIncome = 0.0;
    double estExpenses = 0.0;
    double actIncome = 0.0;
    double actExpenses = 0.0;
    mmReportBudget budgetDetails;

    bool evaluateTransfer = false;
    if (mmIniOptions::instance().budgetIncludeTransfers_)
    {
        evaluateTransfer = true;
    }

    currentView_ = Model_Infotable::instance().GetStringInfo("BUDGET_FILTER", "View All Budget Categories");
    wxString budgetYearStr = Model_Budgetyear::instance().Get(budgetYearID_);
    long year = 0;
    budgetYearStr.ToLong(&year);
    wxDateTime dtBegin(1, wxDateTime::Jan, year);
    wxDateTime dtEnd(31, wxDateTime::Dec, year);

    monthlyBudget_ = (budgetYearStr.length() > 5);

    if (monthlyBudget_)
    {
        budgetDetails.SetBudgetMonth(budgetYearStr, dtBegin, dtEnd);
    }
    else
    {
        int day, month;
        budgetDetails.AdjustYearValues(day, month, dtBegin);
        budgetDetails.AdjustDateForEndFinancialYear(dtEnd);
    }
    mmSpecifiedRange date_range(dtBegin, dtEnd);
    //Get statistics
    Model_Budget::instance().getBudgetEntry(budgetYearID_, budgetPeriod_, budgetAmt_);
    Model_Category::instance().getCategoryStats(categoryStats_
        , &date_range, mmIniOptions::instance().ignoreFutureTransactions_
        , false, true, (evaluateTransfer ? &budgetAmt_ : 0));

    const Model_Subcategory::Data_Set& allSubcategories = Model_Subcategory::instance().all(Model_Subcategory::COL_SUBCATEGNAME);
    for (const auto& category : Model_Category::instance().all(Model_Category::COL_CATEGNAME))
    {
        double estimated = getEstimate(category.CATEGID, -1);
        if (estimated < 0)
            estExpenses += estimated;
        else
            estIncome += estimated;

        double actual = 0;
        if (currentView_ != "View Planned Budget Categories" || estimated != 0)
        {
            actual = categoryStats_[category.CATEGID][-1][0];
            if (actual < 0)
                actExpenses += actual;
            else
                actIncome += actual;
        }

        /***************************************************************************
         Create a TOTALS entry for the category.
         ***************************************************************************/
        double catTotalsEstimated = estimated;
        double catTotalsActual = actual;

        if (DisplayEntryAllowed(category.CATEGID, -1)) {
            std::pair <int, int> category_pair;
            category_pair.first = category.CATEGID;
            category_pair.second = -1;
            budget_.push_back(category_pair);
        }

        for (const auto& subcategory : allSubcategories)
        {
            if (subcategory.CATEGID != category.CATEGID) continue;

            estimated = getEstimate(category.CATEGID, subcategory.SUBCATEGID);
            if (estimated < 0)
                estExpenses += estimated;
            else
                estIncome += estimated;

            actual = 0;
            if (currentView_ != "View Planned Budget Categories" || estimated != 0)
            {
                actual = categoryStats_[category.CATEGID][subcategory.SUBCATEGID][0];
                if (actual < 0)
                    actExpenses += actual;
                else
                    actIncome += actual;
            }
    
            /***************************************************************************
             Update the TOTALS entry for the subcategory.
            ***************************************************************************/
            catTotalsEstimated += estimated;
            catTotalsActual += actual;

            if (DisplayEntryAllowed(category.CATEGID, subcategory.SUBCATEGID)) {
                std::pair <int, int> category_pair;
                category_pair.first = category.CATEGID;
                category_pair.second = subcategory.SUBCATEGID;
                budget_.push_back(category_pair);
            }
        }

        budgetTotals_[category.CATEGID].first = catTotalsEstimated;
        budgetTotals_[category.CATEGID].second = catTotalsActual;

        if (mmIniOptions::instance().budgetSetupWithoutSummaries_ && DisplayEntryAllowed(-1, category.CATEGID))
        {
            std::pair <int, int> category_pair;
            category_pair.first = -1;
            category_pair.second = category.CATEGID;
            budget_.push_back(category_pair);
            int transCatTotalIndex = (int)budget_.size() - 1;
            listCtrlBudget_->RefreshItem(transCatTotalIndex);
        }
    }

    listCtrlBudget_->SetItemCount((int)budget_.size());

    wxString est_amount, act_amount, diff_amount;
    est_amount = Model_Currency::toCurrency(estIncome);
    act_amount = Model_Currency::toCurrency(actIncome);
    diff_amount = Model_Currency::toCurrency(estIncome - actIncome);

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

double mmBudgetingPanel::getEstimate(int category, int subcategory) const
{
    Model_Budget::PERIOD_ENUM period = budgetPeriod_.at(category).at(subcategory);
    double amt = budgetAmt_.at(category).at(subcategory);
    return (monthlyBudget_ ? Model_Budget::getMonthlyEstimate(period, amt) : Model_Budget::getYearlyEstimate(period, amt));
}

void mmBudgetingPanel::DisplayBudgetingDetails(int budgetYearID)
{
    this->windowsFreezeThaw();
    budgetYearID_ = budgetYearID;
    RefreshList();
    this->windowsFreezeThaw();
}

/*******************************************************/
void budgetingListCtrl::OnItemResize(wxListEvent& event)
{
    cp_->save_column_width(event.GetColumn());
}

void budgetingListCtrl::OnListItemSelected(wxListEvent& event)
{
    selectedIndex_ = event.GetIndex();
}

wxString mmBudgetingPanel::getItem(long item, long column)
{
    wxString text = "";
    if (column == 0)
    {
        if (budget_[item].first < 0)
        {
            Model_Category::Data* category = Model_Category::instance().get(budget_[item].second);
            if(category) text = category->CATEGNAME;
        }
        else
        {
            Model_Category::Data* category = Model_Category::instance().get(budget_[item].first);
            if (category) text = category->CATEGNAME;
        }
    }
    if (column == 1)
    {
        if (budget_[item].first >= 0)
        {
            Model_Subcategory::Data* subcategory = Model_Subcategory::instance().get(budget_[item].second);
            if (subcategory) text = subcategory->SUBCATEGNAME;
        }
    }
    if (column == 2)
    {
        if (budget_[item].first >= 0)
            text = Model_Budget::all_period()[budgetPeriod_[budget_[item].first][budget_[item].second]];
    }
    if (column == 3)
    {
        if (budget_[item].first >= 0)
        {
            double amt = budgetAmt_[budget_[item].first][budget_[item].second];
            text = Model_Currency::toCurrency(amt);
        }
    }
    if (column == 4)
    {
        if (budget_[item].first < 0)
        {
            double estimated = budgetTotals_[budget_[item].second].first;
            text = Model_Currency::toCurrency(estimated);
        }
        else
        {
            double estimated = getEstimate(budget_[item].first, budget_[item].second);
            text = Model_Currency::toCurrency(estimated);
        }
    }
    if (column == 5)
    {
        if (budget_[item].first < 0)
        {
            double actual = budgetTotals_[budget_[item].second].second;
            text = Model_Currency::toCurrency(actual);
        }
        else
        {
            double actual = categoryStats_[budget_[item].first][budget_[item].second][0];
            text = Model_Currency::toCurrency(actual);
        }
    }
    return text;
}

int budgetingListCtrl::OnGetItemImage(long item) const
{
    return cp_->GetItemImage(item);
}
int mmBudgetingPanel::GetItemImage(long item) const
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
        estimated = getEstimate(budget_[item].first, budget_[item].second);
        actual = categoryStats_.at(budget_[item].first).at(budget_[item].second).at(0);
    }

    if ((estimated == 0.0) && (actual == 0.0)) return 3;
    if ((estimated == 0.0) && (actual != 0.0)) return 2;
    if (estimated < actual) return 0;
    if (fabs(estimated - actual)  < 0.001) return 0;
    return 1;
}

wxString budgetingListCtrl::OnGetItemText(long item, long column) const
{
    return cp_->getItem(item, column);
}

wxListItemAttr* budgetingListCtrl::OnGetItemAttr(long item) const
{
    if ((cp_->GetTransID(item) < 0) &&
        (cp_->GetCurrentView() != "View Budget Category Summary") )
    {
        return (wxListItemAttr *)&attr3_;
    }

    /* Returns the alternating background pattern */
    return (item % 2) ? attr2_ : attr1_;
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
    if (budget_[selectedIndex].first < 0) return;

    Model_Budget::Data_Set budget = Model_Budget::instance().find(Model_Budget::BUDGETYEARID(GetBudgetYearID())
        , Model_Budget::CATEGID(budget_[selectedIndex].first), Model_Budget::SUBCATEGID(budget_[selectedIndex].second));
    Model_Budget::Data* entry = 0;
    if (budget.empty())
    {
        entry = Model_Budget::instance().create();
        entry->BUDGETYEARID = GetBudgetYearID();
        entry->CATEGID = budget_[selectedIndex].first;
        entry->SUBCATEGID = budget_[selectedIndex].second;
        entry->PERIOD = "";
        entry->AMOUNT = 0.0;
        Model_Budget::instance().save(entry);
    }
    else
        entry = &budget[0];

    double estimated = getEstimate(budget_[selectedIndex].first, budget_[selectedIndex].second);
    double actual = categoryStats_[budget_[selectedIndex].first][budget_[selectedIndex].second][0];

    mmBudgetEntryDialog dlg(this, entry, Model_Currency::toCurrency(estimated), Model_Currency::toCurrency(actual));
    if (dlg.ShowModal() == wxID_OK)
    {
        initVirtualListControl();
        listCtrlBudget_->Refresh();
        listCtrlBudget_->EnsureVisible(selectedIndex);
    }
}
