/*************************************************************************
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
 *************************************************************************/

/*************************************************************************
 Renamed after extensive modifications to original file reportbudgetsetup.cpp
**************************************************************************/
#include "budgetcategorysummary.h"
#include "../budgetingpanel.h"
#include "htmlbuilder.h"
#include "../mmex.h"

mmReportBudgetCategorySummary::mmReportBudgetCategorySummary(mmCoreDB* core, int budgetYearID)
: mmReportBudget(core)
, budgetYearID_(budgetYearID)
{}

wxString mmReportBudgetCategorySummary::actualAmountColour(const mmBudgetEntryHolder& budEntry, bool total)
{
    wxString actAmtColStr = "black";
    if (total) {
        actAmtColStr = "blue";
    }

    if (budEntry.amt_ == 0) {
        actAmtColStr = "blue";
    } else {
        if (budEntry.actual_ < budEntry.estimated_) {
            actAmtColStr = "red";
        }
    }

    return actAmtColStr;
}

// Displays Row: Category, Sub Category, Period, Amount, Estimated, Actual
void mmReportBudgetCategorySummary::displayReportLine(mmHTMLBuilder& hb, const mmBudgetEntryHolder& budEntry)
{
    hb.startTableRow();
    hb.addTableCell(budEntry.catStr_, false, true);
    hb.addTableCell(budEntry.subCatStr_, false, true);
	hb.addMoneyCell(budEntry.amt_);
    hb.addTableCell(budEntry.period_, false, true);
	hb.addMoneyCell(budEntry.estimated_);
	hb.addMoneyCell(budEntry.actual_, this->actualAmountColour(budEntry));
    hb.endTableRow();
}

wxString mmReportBudgetCategorySummary::getHTMLText()
{
    int startDay = 1;
    int startMonth = wxDateTime::Jan;
    int endDay   = 31;
    int endMonth = wxDateTime::Dec;

    long startYear;
    wxString startYearStr = mmDBWrapper::getBudgetYearForID(core_->db_.get(), budgetYearID_);
    startYearStr.ToLong(&startYear);

    wxString headingStr = AdjustYearValues(startDay, startMonth, startYear, startYearStr);
    wxDateTime yearBegin(startDay, (wxDateTime::Month)startMonth, startYear);
    wxDateTime yearEnd(endDay, (wxDateTime::Month)endMonth, startYear);

    bool monthlyBudget = (startYearStr.length() > 5);
    if (monthlyBudget) {
        SetBudgetMonth(startYearStr, yearBegin, yearEnd);
    } else {
        AdjustDateForEndFinancialYear(yearEnd);
    }

    bool evaluateTransfer = false;
    if (wxGetApp().m_frame->budgetTransferTotal())
    {
        evaluateTransfer = true;
    }
    mmHTMLBuilder hb;
    hb.init();
    wxString headerStartupMsg;
    if (wxGetApp().m_frame->budgetCategoryTotal())
        headerStartupMsg = _("Budget Categories for ");
    else
        headerStartupMsg = _("Budget Category Summary for ");
    hb.addHeader(2, headerStartupMsg + headingStr + "<br>" + _("( Estimated Vs Actual )"));
    hb.DisplayDateHeading(yearBegin, yearEnd);

    double estIncome = 0.0;
    double estExpenses = 0.0;
    double actIncome = 0.0;
    double actExpenses = 0.0;

    hb.startCenter();

    hb.startTable("65%");
    hb.startTableRow();
    hb.addTableHeaderCell(_("Category"));
    hb.addTableHeaderCell(_("Sub Category"));
    hb.addTableHeaderCell(_("Amount"), true);
    hb.addTableHeaderCell(_("Frequency"));
    hb.addTableHeaderCell(_("Estimated"), true);
    hb.addTableHeaderCell(_("Actual"), true);
    hb.endTableRow();

    core_->currencyList_.LoadBaseCurrencySettings();

    wxSQLite3Statement st = core_->db_.get()->PrepareStatement(SELECT_SUBCATEGS_FROM_SUBCATEGORY_V1);

    wxSQLite3ResultSet q1 = core_->db_.get()->ExecuteQuery(SELECT_ALL_FROM_CATEGORY_V1);
    while (q1.NextRow())
    {
        mmBudgetEntryHolder th;
        initBudgetEntryFields(th, budgetYearID_);
        th.categID_ = q1.GetInt("CATEGID");
        th.catStr_ = q1.GetString("CATEGNAME");
        mmDBWrapper::getBudgetEntry(core_->db_.get(), budgetYearID_, th.categID_, th.subcategID_, th.period_, th.amt_);

        setBudgetEstimate(th,monthlyBudget);
        if (th.estimated_ < 0) {
            estExpenses += th.estimated_;
        } else {
            estIncome += th.estimated_;
        }

        bool transferAsDeposit = true;
        if (th.amt_ < 0)
        {
            transferAsDeposit = false;
        }
        th.actual_ = core_->bTransactionList_.getAmountForCategory(th.categID_, th.subcategID_, false,
            yearBegin, yearEnd, evaluateTransfer, transferAsDeposit, mmIniOptions::instance().ignoreFutureTransactions_
        );

        if (th.actual_ < 0) {
            actExpenses += th.actual_;
        } else {
            actIncome += th.actual_;
        }

        if (wxGetApp().m_frame->budgetCategoryTotal())
        {
            th.subCatStr_ = wxEmptyString;
            displayReportLine(hb, th);
        }
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
        /***************************************************************************/

        //START: SUBCATEGORY ROW
        st.Bind(1, th.categID_);
        wxSQLite3ResultSet q2 = st.ExecuteQuery();

        while(q2.NextRow())
        {
            mmBudgetEntryHolder thsub;
            initBudgetEntryFields(thsub, budgetYearID_);
            thsub.categID_ = th.categID_;
            thsub.catStr_ = th.catStr_;
            thsub.subcategID_ = q2.GetInt("SUBCATEGID");
            thsub.subCatStr_   = q2.GetString("SUBCATEGNAME");
            mmDBWrapper::getBudgetEntry(core_->db_.get(), budgetYearID_, thsub.categID_, thsub.subcategID_, thsub.period_, thsub.amt_);

            setBudgetEstimate(thsub,monthlyBudget);
            if (thsub.estimated_ < 0) {
                estExpenses += thsub.estimated_;
            } else {
                estIncome += thsub.estimated_;
            }

            transferAsDeposit = true;
            if (thsub.amt_ < 0)
            {
                transferAsDeposit = false;
            }
            thsub.actual_ = core_->bTransactionList_.getAmountForCategory(thsub.categID_, thsub.subcategID_, false,
                yearBegin, yearEnd, evaluateTransfer, transferAsDeposit, mmIniOptions::instance().ignoreFutureTransactions_
            );
            if (thsub.actual_ < 0) {
                actExpenses += thsub.actual_;
            } else {
                actIncome += thsub.actual_;
            }

            if (wxGetApp().m_frame->budgetCategoryTotal())
            {
                displayReportLine(hb, thsub);
            }

            /***************************************************************************
             Update the TOTALS entry for the subcategory.
             ***************************************************************************/
            catTotals.amt_       += thsub.amt_;
            catTotals.estimated_ += thsub.estimated_;
            catTotals.actual_    += thsub.actual_;
        }
        //END: SUBCATEGORY ROW
        st.Reset();

        /***************************************************************************
            Display a TOTALS entry for the category.
        ****************************************************************************/
        if (wxGetApp().m_frame->budgetCategoryTotal()) {
            hb.addRowSeparator(6);
        }
        // Category, Sub Category, Period, Amount, Estimated, Actual
        hb.startTableRow();
        hb.addTableCell(catTotals.catStr_, false, true, true, "blue");
        hb.addTableCell("", false, true, true, "blue");
        hb.addTableCell(wxEmptyString, true, false,true, "blue");
        hb.addTableCell(catTotals.period_, false, true, true, "blue");
		hb.addMoneyCell(catTotals.estimated_, "blue");
		hb.addMoneyCell(catTotals.actual_, this->actualAmountColour(catTotals, true));
        hb.endTableRow();
        hb.addRowSeparator(6);
        /***************************************************************************/
    }
    q1.Finalize();
    st.Finalize();

    hb.endTable();
    hb.endCenter();

    double difIncome = estIncome - actIncome;
    double difExpense = estExpenses - actExpenses;

    //Summary of Estimated Vs Actual totals
    hb.addLineBreak();
    hb.startCenter();
    hb.startTable("50%");
//    hb.addRowSeparator(3);
    hb.startTableRow();
	hb.addMoneyCell(estIncome); 
	hb.addMoneyCell(actIncome);
	hb.addMoneyCell(difIncome);
    hb.endTableRow();

    hb.startTableRow();
	hb.addMoneyCell(estExpenses);
	hb.addMoneyCell(actExpenses);
	hb.addMoneyCell(difExpense);
    hb.endTableRow();
    hb.addRowSeparator(3);
    hb.endTable();
    hb.endCenter();

    hb.end();
    return hb.getHTMLText();
}
