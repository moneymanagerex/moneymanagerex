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
#include "mmCurrencyFormatter.h"
#include "mmex.h"
#include "reports/budget.h"
#include "mmex_settings.h"
#include "model/Model_Infotable.h"

/*******************************************************/
BEGIN_EVENT_TABLE(mmBudgetingPanel, wxPanel)
    EVT_LEFT_DOWN( mmBudgetingPanel::OnMouseLeftDown )
    EVT_MENU(wxID_ANY, mmBudgetingPanel::OnViewPopupSelected)
END_EVENT_TABLE()
/*******************************************************/
BEGIN_EVENT_TABLE(budgetingListCtrl, wxListCtrl)
    EVT_LIST_ITEM_SELECTED(ID_PANEL_CHECKING_LISTCTRL_ACCT, budgetingListCtrl::OnListItemSelected)
    EVT_LIST_ITEM_ACTIVATED(ID_PANEL_CHECKING_LISTCTRL_ACCT, budgetingListCtrl::OnListItemActivated)
    EVT_LIST_COL_END_DRAG(ID_PANEL_CHECKING_LISTCTRL_ACCT, budgetingListCtrl::OnItemResize)
END_EVENT_TABLE()
/*******************************************************/
mmBudgetingPanel::mmBudgetingPanel(
    mmCoreDB* core, int budgetYearID,
    wxWindow *parent, wxWindowID winid,
    const wxPoint& pos, const wxSize& size,
    long style,const wxString& name)
: mmPanelBase(core)
, m_imageList()
, listCtrlBudget_()
, budgetYearID_(budgetYearID)
{
    Create(parent, winid, pos, size, style, name);
}

bool mmBudgetingPanel::Create( wxWindow *parent,
            wxWindowID winid, const wxPoint& pos,
            const wxSize& size,long style, const wxString& name  )
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create(parent, winid, pos, size, style, name);

    this->windowsFreezeThaw();
    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    initVirtualListControl();

    this->windowsFreezeThaw();
    return TRUE;
}

mmBudgetingPanel::~mmBudgetingPanel()
{
    if (m_imageList) delete m_imageList;
    wxGetApp().m_frame->SetBudgetingPageInactive();
}

void mmBudgetingPanel::save_column_width(int width)
{
    int col_x = listCtrlBudget_->GetColumnWidth(width);
    core_->iniSettings_->SetSetting(wxString::Format("BUDGET_COL%d_WIDTH", width), col_x);
}

void mmBudgetingPanel::OnViewPopupSelected(wxCommandEvent& event)
{
    int evt =  event.GetId();
    if (evt ==  MENU_VIEW_ALLBUDGETENTRIES)
        currentView_ = wxTRANSLATE("View All Budget Categories");
    else if (evt == MENU_VIEW_NONZEROBUDGETENTRIES)
        currentView_ = wxTRANSLATE("View Non-Zero Budget Categories");
    else if (evt == MENU_VIEW_INCOMEBUDGETENTRIES)
        currentView_ = wxTRANSLATE("View Income Budget Categories");
    else if (evt == MENU_VIEW_EXPENSEBUDGETENTRIES)
        currentView_ = wxTRANSLATE("View Expense Budget Categories");
    else if (evt == MENU_VIEW_SUMMARYBUDGETENTRIES)
        currentView_ = wxTRANSLATE("View Budget Category Summary");
    else
        wxASSERT(false);

    Model_Infotable::instance().Set("BUDGET_FILTER", currentView_);

    listCtrlBudget_->DeleteAllItems();
    initVirtualListControl();
    if (trans_.size()>0)
        listCtrlBudget_->RefreshItems(0, (static_cast<long>(trans_.size()-1)));
}

void mmBudgetingPanel::OnMouseLeftDown( wxMouseEvent& event )
{
    // depending on the clicked control's window id.
    switch( event.GetId() )
    {
        case ID_PANEL_BUDGETENTRY_STATIC_BITMAP_VIEW :
        {
            wxMenu menu;
            menu.Append(MENU_VIEW_ALLBUDGETENTRIES, _("View All Budget Categories"));
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

void mmBudgetingPanel::UpdateBudgetHeading()
{
    wxString yearStr = mmDBWrapper::getBudgetYearForID(core_->db_.get(), budgetYearID_);
    if ((yearStr.length() < 5))
    {
        if (wxGetApp().m_frame->budgetFinancialYears() )
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
    budgetReportHeading_->SetLabel(wxString::Format(_("Budget Setup for %s"), yearStr));

    wxStaticText* header = (wxStaticText*)FindWindow(ID_PANEL_CHECKING_STATIC_PANELVIEW);
    header->SetLabel(wxGetTranslation(currentView_));
}

void mmBudgetingPanel::CreateControls()
{
    wxSizerFlags flags;
    flags.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).Border(wxLEFT|wxTOP, 4);

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer2);

    wxPanel* itemPanel3 = new wxPanel( this, ID_PANEL_REPORTS_HEADER_PANEL,
        wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    itemBoxSizer2->Add(itemPanel3, flags);

    wxBoxSizer* itemBoxSizerVHeader = new wxBoxSizer(wxVERTICAL);
    itemPanel3->SetSizer(itemBoxSizerVHeader);

    budgetReportHeading_ = new wxStaticText(itemPanel3, wxID_ANY, "");

    int font_size = this->GetFont().GetPointSize();
    budgetReportHeading_->SetFont(wxFont(font_size+2, wxSWISS, wxNORMAL, wxBOLD, FALSE, ""));

    wxBoxSizer* budgetReportHeadingSizer = new wxBoxSizer(wxHORIZONTAL);
    budgetReportHeadingSizer->Add(budgetReportHeading_, 1);
    itemBoxSizerVHeader->Add(budgetReportHeadingSizer, 0, wxALL, 1);

    wxBoxSizer* itemBoxSizerHHeader2 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizerVHeader->Add(itemBoxSizerHHeader2, 0, wxALL, 1);

    wxBitmap itemStaticBitmap3Bitmap(wxBitmap(wxImage(rightarrow_xpm).Scale(16,16)));
    wxStaticBitmap* itemStaticBitmap3 = new wxStaticBitmap( itemPanel3,
        ID_PANEL_BUDGETENTRY_STATIC_BITMAP_VIEW,
        itemStaticBitmap3Bitmap, wxDefaultPosition, wxSize(16, 16), 0 );
    itemStaticBitmap3->Connect(ID_PANEL_BUDGETENTRY_STATIC_BITMAP_VIEW, wxEVT_LEFT_DOWN,
        wxMouseEventHandler(mmBudgetingPanel::OnMouseLeftDown), NULL, this);
    itemBoxSizerHHeader2->Add(itemStaticBitmap3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxStaticText* itemStaticText18 = new wxStaticText( itemPanel3,
        ID_PANEL_CHECKING_STATIC_PANELVIEW , "");
    itemBoxSizerHHeader2->Add(itemStaticText18, 0, wxALL, 1);

    wxFlexGridSizer* itemIncomeSizer = new wxFlexGridSizer(0,7,5,10);
    itemBoxSizerVHeader->Add(itemIncomeSizer);

    income_estimated_ = new wxStaticText( itemPanel3,
        ID_DIALOG_BUDGETENTRY_SUMMARY_INCOME_EST, "$", wxDefaultPosition, wxSize(120, -1));
    income_actual_ = new wxStaticText( itemPanel3,
        ID_DIALOG_BUDGETENTRY_SUMMARY_INCOME_ACT, "$", wxDefaultPosition, wxSize(120, -1));
    income_diff_ = new wxStaticText( itemPanel3,
        ID_DIALOG_BUDGETENTRY_SUMMARY_INCOME_DIF, "$");

    expences_estimated_ = new wxStaticText( itemPanel3,
        ID_DIALOG_BUDGETENTRY_SUMMARY_EXPENSES_EST, "$", wxDefaultPosition, wxSize(120, -1));
    expences_actual_ = new wxStaticText( itemPanel3,
        ID_DIALOG_BUDGETENTRY_SUMMARY_EXPENSES_ACT, "$", wxDefaultPosition, wxSize(120, -1));
    expences_diff_ = new wxStaticText( itemPanel3,
        ID_DIALOG_BUDGETENTRY_SUMMARY_EXPENSES_DIF, "$");

    itemIncomeSizer->Add(new wxStaticText( itemPanel3, wxID_STATIC, _("Income: ")));
    itemIncomeSizer->Add(new wxStaticText( itemPanel3, wxID_STATIC, _("Estimated: ")));
    itemIncomeSizer->Add(income_estimated_);
    itemIncomeSizer->Add(new wxStaticText( itemPanel3, wxID_STATIC, _("Actual: ")));
    itemIncomeSizer->Add(income_actual_);
    itemIncomeSizer->Add(new wxStaticText( itemPanel3, wxID_STATIC, _("Difference: ")));
    itemIncomeSizer->Add(income_diff_);

    itemIncomeSizer->Add(new wxStaticText( itemPanel3, wxID_STATIC, _("Expenses: ")));
    itemIncomeSizer->Add(new wxStaticText( itemPanel3, wxID_STATIC, _("Estimated: ")));
    itemIncomeSizer->Add(expences_estimated_);
    itemIncomeSizer->Add(new wxStaticText( itemPanel3, wxID_STATIC, _("Actual: ")));
    itemIncomeSizer->Add(expences_actual_);
    itemIncomeSizer->Add(new wxStaticText( itemPanel3, wxID_STATIC, _("Difference: ")));
    itemIncomeSizer->Add(expences_diff_);
    /* ---------------------- */

    wxSize imageSize(16, 16);
    m_imageList = new wxImageList( imageSize.GetWidth(), imageSize.GetHeight() );
    m_imageList->Add(wxBitmap(reconciled_xpm));
    m_imageList->Add(wxBitmap(void_xpm));
    m_imageList->Add(wxBitmap(flag_xpm));
    m_imageList->Add(wxBitmap(empty_xpm));

    listCtrlBudget_ = new budgetingListCtrl( this, this,
        ID_PANEL_CHECKING_LISTCTRL_ACCT, wxDefaultPosition, wxDefaultSize,
        wxLC_REPORT | wxLC_HRULES | wxLC_VRULES | wxLC_VIRTUAL | wxLC_SINGLE_SEL  );

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
        int col = core_->iniSettings_->GetIntSetting(wxString::Format("BUDGET_COL%d_WIDTH", i), 80);
        listCtrlBudget_->SetColumnWidth(i, col);
    }
    itemBoxSizer2->Add(listCtrlBudget_, 1, wxGROW | wxALL, 1);
}

bool mmBudgetingPanel::DisplayEntryAllowed(mmBudgetEntryHolder& budgetEntry)
{
    bool result = false;

    if (currentView_ == "View Non-Zero Budget Categories")
        result =((budgetEntry.estimated_ != 0.0) || (budgetEntry.actual_ != 0.0));
    else if (currentView_ == "View Income Budget Categories")
        result = ((budgetEntry.estimated_ > 0.0) || (budgetEntry.actual_ > 0.0));
    else if (currentView_ == "View Expense Budget Categories")
        result = ((budgetEntry.estimated_ < 0.0) || (budgetEntry.actual_ < 0.0));
    else if (currentView_ == "View Budget Category Summary")
        result = ((budgetEntry.id_ < 0.0));
    else
        result = true;

    return result;
}

void mmBudgetingPanel::initVirtualListControl()
{
    trans_.clear();
    double estIncome = 0.0;
    double estExpenses = 0.0;
    double actIncome = 0.0;
    double actExpenses = 0.0;
    mmReportBudget budgetDetails;

    bool evaluateTransfer = false;
    if (wxGetApp().m_frame->budgetTransferTotal())
    {
        evaluateTransfer = true;
    }

    currentView_ = Model_Infotable::instance().GetStringInfo("BUDGET_FILTER", "View All Budget Categories");
    wxString budgetYearStr = mmDBWrapper::getBudgetYearForID(core_->db_.get(), budgetYearID_);
    long year = 0;
    budgetYearStr.ToLong(&year);
    wxDateTime dtBegin(1, wxDateTime::Jan, year);
    wxDateTime dtEnd(31, wxDateTime::Dec, year);

    bool monthlyBudget = (budgetYearStr.length() > 5);

    if (monthlyBudget)
    {
        budgetDetails.SetBudgetMonth(budgetYearStr, dtBegin, dtEnd);
    }
    else
    {
        int day, month;
        budgetDetails.AdjustYearValues(day, month, dtBegin);
        budgetDetails.AdjustDateForEndFinancialYear(dtEnd);
    }

    core_->currencyList_.LoadBaseCurrencySettings();

    for (const auto& category: core_->categoryList_.entries_)
    {
        mmBudgetEntryHolder th;
        budgetDetails.initBudgetEntryFields(th, budgetYearID_);
        th.categID_ = category->categID_;
        th.catStr_ = category->categName_;

        mmDBWrapper::getBudgetEntry(core_->db_.get(),
            budgetYearID_, th.categID_, th.subcategID_, th.period_, th.amt_);
        budgetDetails.setBudgetEstimate(th, monthlyBudget);
        if (th.estimated_ < 0)
            estExpenses += th.estimated_;
        else
            estIncome += th.estimated_;

        bool transferAsDeposit = true;
        if (th.amt_ < 0)
        {
            transferAsDeposit = false;
        }
        th.actual_ = core_->bTransactionList_.getAmountForCategory(
            th.categID_, th.subcategID_, false,
            dtBegin, dtEnd, evaluateTransfer, transferAsDeposit,
            mmIniOptions::instance().ignoreFutureTransactions_
        );
        if (th.actual_ < 0)
            actExpenses += th.actual_;
        else
            actIncome += th.actual_;

        th.amtString_ = CurrencyFormatter::float2String(th.amt_);
        th.estimatedStr_ = CurrencyFormatter::float2String(th.estimated_);
        th.actualStr_ = CurrencyFormatter::float2String(th.actual_);

        /***************************************************************************
         Create a TOTALS entry for the category.
         ***************************************************************************/
        mmBudgetEntryHolder catTotals;
        catTotals.id_ = -1;
        catTotals.categID_ = -1;
        catTotals.catStr_  = th.catStr_;
        catTotals.subcategID_ = -1;
        catTotals.subCatStr_ = wxEmptyString;
        catTotals.period_    = wxEmptyString;
        catTotals.amt_       = th.amt_;
        catTotals.estimated_ = th.estimated_;
        catTotals.actual_    = th.actual_;
        catTotals.amtString_ = CurrencyFormatter::float2String(catTotals.amt_);
        catTotals.estimatedStr_ = CurrencyFormatter::float2String(catTotals.estimated_);
        catTotals.actualStr_ = CurrencyFormatter::float2String(catTotals.actual_);

        if (DisplayEntryAllowed(th)) {
            trans_.push_back(th);
        }

        for (const auto& sub_category : category->children_)
        {
            mmBudgetEntryHolder thsub;
            budgetDetails.initBudgetEntryFields(thsub, budgetYearID_);
            thsub.categID_ = th.categID_;
            thsub.catStr_ = th.catStr_;
            thsub.subcategID_ = sub_category->categID_;
            thsub.subCatStr_   = sub_category->categName_;

            mmDBWrapper::getBudgetEntry(core_->db_.get(), budgetYearID_,
                thsub.categID_, thsub.subcategID_, thsub.period_, thsub.amt_);
            budgetDetails.setBudgetEstimate(thsub, monthlyBudget);
            if (thsub.estimated_ < 0)
                estExpenses += thsub.estimated_;
            else
                estIncome += thsub.estimated_;

            transferAsDeposit = true;
            if (thsub.amt_ < 0)
            {
                transferAsDeposit = false;
            }
            thsub.actual_ = core_->bTransactionList_.getAmountForCategory(thsub.categID_, thsub.subcategID_, false,
                dtBegin, dtEnd, evaluateTransfer, transferAsDeposit, mmIniOptions::instance().ignoreFutureTransactions_
            );
            if (thsub.actual_ < 0)
                actExpenses += thsub.actual_;
            else
                actIncome += thsub.actual_;

            thsub.amtString_ = CurrencyFormatter::float2String(thsub.amt_);
            thsub.estimatedStr_ = CurrencyFormatter::float2String(thsub.estimated_);
            thsub.actualStr_ = CurrencyFormatter::float2String(thsub.actual_);

            /***************************************************************************
             Update the TOTALS entry for the subcategory.
            ***************************************************************************/
            catTotals.estimated_    += thsub.estimated_;
            catTotals.actual_       += thsub.actual_;
            catTotals.amtString_ = wxEmptyString;
            catTotals.estimatedStr_ = CurrencyFormatter::float2String(catTotals.estimated_);
            catTotals.actualStr_ = CurrencyFormatter::float2String(catTotals.actual_);

            if (DisplayEntryAllowed(thsub)) {
                trans_.push_back(thsub);
            }
        }

        if (wxGetApp().m_frame->budgetSetupWithSummary() && DisplayEntryAllowed(catTotals))
        {
            trans_.push_back(catTotals);
            int transCatTotalIndex = (int)trans_.size()-1;
            listCtrlBudget_->RefreshItem(transCatTotalIndex);
        }
    }

    listCtrlBudget_->SetItemCount((int)trans_.size());

    wxString est_amount, act_amount, diff_amount;
    est_amount = CurrencyFormatter::float2Money(estIncome);
    act_amount = CurrencyFormatter::float2Money(actIncome);
    diff_amount = CurrencyFormatter::float2Money(estIncome - actIncome);

    income_estimated_->SetLabel(est_amount);
    income_actual_->SetLabel(act_amount);
    income_diff_->SetLabel(diff_amount);

    if (estExpenses < 0.0) estExpenses = -estExpenses;
    if (actExpenses < 0.0) actExpenses = -actExpenses;
    est_amount = CurrencyFormatter::float2Money(estExpenses);
    act_amount = CurrencyFormatter::float2Money(actExpenses);
    diff_amount = CurrencyFormatter::float2Money(estExpenses -actExpenses);

    expences_estimated_->SetLabel(est_amount);
    expences_actual_->SetLabel(act_amount);
    expences_diff_->SetLabel(diff_amount);
    UpdateBudgetHeading();
}

void mmBudgetingPanel::DisplayBudgetingDetails(int budgetYearID)
{
    this->windowsFreezeThaw();
    budgetYearID_ = budgetYearID;
    initVirtualListControl();
    UpdateBudgetHeading();
    listCtrlBudget_->RefreshItem(0);
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
    if (column == 0) return trans_[item].catStr_;
    if (column == 1) return trans_[item].subCatStr_;
    if (column == 2) return wxGetTranslation(trans_[item].period_);
    if (column == 3) return trans_[item].amtString_;
    if (column == 4) return trans_[item].estimatedStr_;
    if (column == 5) return trans_[item].actualStr_;

    return "";
}

int budgetingListCtrl::OnGetItemImage(long item) const
{
    return cp_->GetItemImage(item);
}
int mmBudgetingPanel::GetItemImage(long item) const
{
    if ((trans_[item].estimated_ == 0.0) && (trans_[item].actual_ == 0.0)) return 3;
    if ((trans_[item].estimated_ == 0.0) && (trans_[item].actual_ != 0.0)) return 2;
    if (trans_[item].estimated_ < trans_[item].actual_) return 0;
    if (fabs(trans_[item].estimated_ - trans_[item].actual_)  < 0.001) return 0;
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
    return item % 2 ? (wxListItemAttr *)&attr2_ : (wxListItemAttr *)&attr1_;
}

void budgetingListCtrl::OnListItemActivated(wxListEvent& event)
{
    selectedIndex_ = event.GetIndex();
    cp_->OnListItemActivated(selectedIndex_);
    RefreshItem(selectedIndex_);
}

void mmBudgetingPanel::OnListItemActivated(int selectedIndex)
{
    /***************************************************************************
     A TOTALS entry does not contain a budget entry, therefore ignore the event.
     ***************************************************************************/
    if (trans_[selectedIndex].id_ < 0) return;
    mmBudgetEntryDialog dlg(
        core_, GetBudgetYearID(),
        trans_[selectedIndex].categID_,
        trans_[selectedIndex].subcategID_,
        trans_[selectedIndex].estimatedStr_,
        trans_[selectedIndex].actualStr_, this);
    if ( dlg.ShowModal() == wxID_OK )
    {
        initVirtualListControl();
    }
}
