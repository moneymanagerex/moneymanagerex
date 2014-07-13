/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2012 Stefano Giorgio
 Copyright (C) 2013 Nikolay
 Copyright (C) 2014 James Higley
 Copyright (C) 2014 Guan Lisheng (guanlisheng@gmail.com)

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

#include "mmframe.h"
#include "reports/budgetcategorysummary.h"
#include "reports/budgetingperf.h"
#include "reports/cashflow.h"
#include "reports/categexp.h"
#include "reports/categovertimeperf.h"
#include "reports/incexpenses.h"
#include "reports/htmlbuilder.h"
#include "reports/payee.h"
#include "reports/transactions.h"
#include "model/Model_Budgetyear.h"
#include "model/Model_Report.h"

void mmGUIFrame::updateReportNavigation(wxTreeItemId& reports, wxTreeItemId& budgeting)
{
    int day = Model_Infotable::instance().GetIntInfo("FINANCIAL_YEAR_START_DAY", 1);
    int month = Model_Infotable::instance().GetIntInfo("FINANCIAL_YEAR_START_MONTH", 7);
    bool ignoreFuture = mmIniOptions::instance().ignoreFutureTransactions_;
    wxTreeItemId categsOverTime = navTreeCtrl_->AppendItem(reports
        , _("Where the Money Goes"), 4, 4);
    navTreeCtrl_->SetItemData(categsOverTime, new mmTreeItemData("Where the Money Goes"
        , new mmReportCategoryExpensesGoes()));

    this->updateReportCategoryExpensesGoesNavigation(categsOverTime);

    wxTreeItemId posCategs = navTreeCtrl_->AppendItem(reports
        , _("Where the Money Comes From"), 4, 4);
    navTreeCtrl_->SetItemData(posCategs
        , new mmTreeItemData("Where the Money Comes From"
        , new mmReportCategoryExpensesComes()));
    
    this->updateReportCategoryExpensesComesNavigation(posCategs);

    wxTreeItemId categs = navTreeCtrl_->AppendItem(reports, _("Categories"), 4, 4);
    navTreeCtrl_->SetItemData(categs
        , new mmTreeItemData("Categories - Over Time"
        , new mmReportCategoryOverTimePerformance()));

    this->updateReportCategoryNavigation(categs);

    wxTreeItemId payeesOverTime = navTreeCtrl_->AppendItem(reports, _("Payees"), 4, 4);
    navTreeCtrl_->SetItemData(payeesOverTime
        , new mmTreeItemData("Payee Report"
        , new mmReportPayeeExpenses()));

    this->updateReportPayeeNavigation(payeesOverTime);

    wxTreeItemId incexpOverTime = navTreeCtrl_->AppendItem(reports, _("Income vs Expenses"), 4, 4);
    navTreeCtrl_->SetItemData(incexpOverTime, new mmTreeItemData("Income vs Expenses"
        , new mmReportIncomeExpensesAllTime()));

    wxTreeItemId incexpOverTimeCalMonth = navTreeCtrl_->AppendItem(incexpOverTime, _("Last Calendar Month"), 4, 4);
    navTreeCtrl_->SetItemData(incexpOverTimeCalMonth
        , new mmTreeItemData("Income vs Expenses - Last Calendar Month"
        , new mmReportIncomeExpensesLastMonth()));

    if (ignoreFuture)
    {
        wxTreeItemId incexpOverTimeCurrentMonth = navTreeCtrl_->AppendItem(incexpOverTime
            , _("Current Month to Date"), 4, 4);
        navTreeCtrl_->SetItemData(incexpOverTimeCurrentMonth
            , new mmTreeItemData("Income vs Expenses - Current Month"
            , new mmReportIncomeExpensesCurrentMonthToDate()));
    }
    else
    {
        wxTreeItemId incexpOverTimeCurrentMonth = navTreeCtrl_->AppendItem(incexpOverTime
            , _("Current Month"), 4, 4);
        navTreeCtrl_->SetItemData(incexpOverTimeCurrentMonth
            , new mmTreeItemData("Income vs Expenses - Current Month"
            , new mmReportIncomeExpensesCurrentMonth()));
    }

    wxTreeItemId incexpOverTimeLast30 = navTreeCtrl_->AppendItem(incexpOverTime, _("Last 30 Days"), 4, 4);
    navTreeCtrl_->SetItemData(incexpOverTimeLast30
        , new mmTreeItemData("Income vs Expenses - Last 30 Days"
        , new mmReportIncomeExpensesLast30Days()));

    wxTreeItemId incexpOverTimeLastYear = navTreeCtrl_->AppendItem(incexpOverTime, _("Last Year"), 4, 4);
    navTreeCtrl_->SetItemData(incexpOverTimeLastYear
        , new mmTreeItemData("Income vs Expenses - Last Year"
        , new mmReportIncomeExpensesLastYear()));

    if (ignoreFuture)
    {
        wxTreeItemId incexpOverTimeCurrentYear = navTreeCtrl_->AppendItem(incexpOverTime, _("Current Year to Date"), 4, 4);
        navTreeCtrl_->SetItemData(incexpOverTimeCurrentYear
            , new mmTreeItemData("Income vs Expenses - Current Year"
            , new mmReportIncomeExpensesCurrentYearToDate()));
    }
    else
    {
        wxTreeItemId incexpOverTimeCurrentYear = navTreeCtrl_->AppendItem(incexpOverTime, _("Current Year"), 4, 4);
        navTreeCtrl_->SetItemData(incexpOverTimeCurrentYear
            , new mmTreeItemData("Income vs Expenses - Current Year"
            , new mmReportIncomeExpensesCurrentYear()));
    }

    if (financialYearIsDifferent())
    {
        wxTreeItemId incexpOverTimeLastFinancialYear = navTreeCtrl_->AppendItem(incexpOverTime
            , _("Last Financial Year"), 4, 4);
        navTreeCtrl_->SetItemData(incexpOverTimeLastFinancialYear
            , new mmTreeItemData("Income vs Expenses - Last Financial Year"
            , new mmReportIncomeExpensesLastFinancialYear(day, month)));

        if (ignoreFuture)
        {
            wxTreeItemId incexpOverTimeCurrentFinancialYear = navTreeCtrl_->AppendItem(incexpOverTime
                , _("Current Financial Year to Date"), 4, 4);
            navTreeCtrl_->SetItemData(incexpOverTimeCurrentFinancialYear
                , new mmTreeItemData("Income vs Expenses - Current Financial Year to Date"
                , new mmReportIncomeExpensesCurrentFinancialYearToDate(day, month)));
        }
        else
        {
            wxTreeItemId incexpOverTimeCurrentFinancialYear = navTreeCtrl_->AppendItem(incexpOverTime
                , _("Current Financial Year"), 4, 4);
            navTreeCtrl_->SetItemData(incexpOverTimeCurrentFinancialYear
                , new mmTreeItemData("Income vs Expenses - Current Financial Year"
                , new mmReportIncomeExpensesCurrentFinancialYear(day, month)));
        }
    }

    ///////////////////////////////////////////////////////////////////

    wxTreeItemId incexpOverTimeSpecificAccounts = navTreeCtrl_->AppendItem(reports, _("Income vs Expenses - Specific Accounts"), 4, 4);
    navTreeCtrl_->SetItemData(incexpOverTimeSpecificAccounts, new mmTreeItemData("Income vs Expenses - Specific Accounts"
        , new mmReportIncomeExpensesAllTimeSpecificAccounts()));

    wxTreeItemId incexpOverTimeCalMonthSpecificAccounts = navTreeCtrl_->AppendItem(incexpOverTimeSpecificAccounts, _("Last Calendar Month"), 4, 4);
    navTreeCtrl_->SetItemData(incexpOverTimeCalMonthSpecificAccounts
        , new mmTreeItemData("Income vs Expenses - Last Calendar Month - Specific Accounts"
        , new mmReportIncomeExpensesLastMonthSpecificAccounts()));

    if (ignoreFuture)
    {
        wxTreeItemId incexpOverTimeCurrentMonthSpecificAccounts = navTreeCtrl_->AppendItem(incexpOverTimeSpecificAccounts
            , _("Current Month to Date"), 4, 4);
        navTreeCtrl_->SetItemData(incexpOverTimeCurrentMonthSpecificAccounts
            , new mmTreeItemData("Income vs Expenses - Current Month - Specific Accounts"
            , new mmReportIncomeExpensesCurrentMonthToDateSpecificAccounts()));
    }
    else
    {
        wxTreeItemId incexpOverTimeCurrentMonthSpecificAccounts = navTreeCtrl_->AppendItem(incexpOverTimeSpecificAccounts
            , _("Current Month"), 4, 4);
        navTreeCtrl_->SetItemData(incexpOverTimeCurrentMonthSpecificAccounts
            , new mmTreeItemData("Income vs Expenses - Current Month - Specific Accounts"
            , new mmReportIncomeExpensesCurrentMonthSpecificAccounts()));
    }

    wxTreeItemId incexpOverTimeLast30SpecificAccounts = navTreeCtrl_->AppendItem(incexpOverTimeSpecificAccounts, _("Last 30 Days"), 4, 4);
    navTreeCtrl_->SetItemData(incexpOverTimeLast30SpecificAccounts
        , new mmTreeItemData("Income vs Expenses - Last 30 Days - Specific Accounts"
        , new mmReportIncomeExpensesLast30DaysSpecificAccounts()));

    wxTreeItemId incexpOverTimeLastYearSpecificAccounts = navTreeCtrl_->AppendItem(incexpOverTimeSpecificAccounts, _("Last Year"), 4, 4);
    navTreeCtrl_->SetItemData(incexpOverTimeLastYearSpecificAccounts
        , new mmTreeItemData("Income vs Expenses - Last Year - Specific Accounts"
        , new mmReportIncomeExpensesLastYearSpecificAccounts()));

    if (ignoreFuture)
    {
        wxTreeItemId incexpOverTimeCurrentYearSpecificAccounts = navTreeCtrl_->AppendItem(incexpOverTimeSpecificAccounts, _("Current Year to Date"), 4, 4);
        navTreeCtrl_->SetItemData(incexpOverTimeCurrentYearSpecificAccounts
            , new mmTreeItemData("Income vs Expenses - Current Year - Specific Accounts"
            , new mmReportIncomeExpensesCurrentYearToDateSpecificAccounts()));
    }
    else
    {
        wxTreeItemId incexpOverTimeCurrentYearSpecificAccounts = navTreeCtrl_->AppendItem(incexpOverTimeSpecificAccounts, _("Current Year"), 4, 4);
        navTreeCtrl_->SetItemData(incexpOverTimeCurrentYearSpecificAccounts
            , new mmTreeItemData("Income vs Expenses - Current Year - Specific Accounts"
            , new mmReportIncomeExpensesCurrentYearSpecificAccounts()));
    }

    if (financialYearIsDifferent())
    {
        wxTreeItemId incexpOverTimeLastFinancialYearSpecificAccounts = navTreeCtrl_->AppendItem(incexpOverTimeSpecificAccounts
            , _("Last Financial Year"), 4, 4);
        navTreeCtrl_->SetItemData(incexpOverTimeLastFinancialYearSpecificAccounts
            , new mmTreeItemData("Income vs Expenses - Last Financial Year - Specific Accounts"
            , new mmReportIncomeExpensesLastFinancialYearSpecificAccounts(day, month)));

        if (ignoreFuture)
        {
            wxTreeItemId incexpOverTimeCurrentFinancialYearSpecificAccounts = navTreeCtrl_->AppendItem(incexpOverTimeSpecificAccounts
                , _("Current Financial Year to Date"), 4, 4);
            navTreeCtrl_->SetItemData(incexpOverTimeCurrentFinancialYearSpecificAccounts
                , new mmTreeItemData("Income vs Expenses - Current Financial Year - Specific Accounts"
                , new mmReportIncomeExpensesCurrentFinancialYearToDateSpecificAccounts(day, month)));
        }
        else
        {
            wxTreeItemId incexpOverTimeCurrentFinancialYearSpecificAccounts = navTreeCtrl_->AppendItem(incexpOverTimeSpecificAccounts
                , _("Current Financial Year"), 4, 4);
            navTreeCtrl_->SetItemData(incexpOverTimeCurrentFinancialYearSpecificAccounts
                , new mmTreeItemData("Income vs Expenses - Current Financial Year - Specific Accounts"
                , new mmReportIncomeExpensesCurrentFinancialYearSpecificAccounts(day, month)));
        }
    }

    //////////////////////////////////////////////////////////////////
    wxTreeItemId budgetPerformance;
    wxTreeItemId budgetSetupPerformance;

    size_t i = 0;
    for (const auto& e : Model_Budgetyear::instance().all(Model_Budgetyear::COL_BUDGETYEARNAME))
    {
        if (!i)
        { // first loop only
            budgetPerformance = navTreeCtrl_->AppendItem(reports, _("Budget Performance"), 4, 4);
            navTreeCtrl_->SetItemData(budgetPerformance, new mmTreeItemData("Budget Performance"));

            budgetSetupPerformance = navTreeCtrl_->AppendItem(reports, _("Budget Category Summary"), 4, 4);
            navTreeCtrl_->SetItemData(budgetSetupPerformance, new mmTreeItemData("Budget Setup Performance"));
        }

        int id = e.BUDGETYEARID;
        const wxString& name = e.BUDGETYEARNAME;

        wxTreeItemId bYear = navTreeCtrl_->AppendItem(budgeting, name, 3, 3);
        navTreeCtrl_->SetItemData(bYear, new mmTreeItemData(id, true));

        // Only add YEARS for Budget Performance
        if (name.length() < 5)
        {
            wxTreeItemId bYearData = navTreeCtrl_->AppendItem(budgetPerformance, name, 4, 4);
            navTreeCtrl_->SetItemData(bYearData, new mmTreeItemData(id, true));
        }
        wxTreeItemId bYearSetupData = navTreeCtrl_->AppendItem(budgetSetupPerformance, name, 4, 4);
        navTreeCtrl_->SetItemData(bYearSetupData, new mmTreeItemData(id, true));
        ++i;
    }

    //TODO: Set up as a permanent user option
    if (expandedBudgetingNavTree_)
        navTreeCtrl_->Expand(budgeting);

    ///////////////////////////////////////////////////////////////////
    wxTreeItemId cashFlow = navTreeCtrl_->AppendItem(reports, _("Cash Flow"), 4, 4);
    navTreeCtrl_->SetItemData(cashFlow, new mmTreeItemData("Cash Flow", new mmReportCashFlowAllAccounts()));

    wxTreeItemId cashflowWithBankAccounts = navTreeCtrl_->AppendItem(cashFlow, _("Cash Flow - With Bank Accounts"), 4, 4);
    navTreeCtrl_->SetItemData(cashflowWithBankAccounts, new mmTreeItemData("Cash Flow - With Bank Accounts", new mmReportCashFlowBankAccounts()));

    wxTreeItemId cashflowWithTermAccounts = navTreeCtrl_->AppendItem(cashFlow, _("Cash Flow - With Term Accounts"), 4, 4);
    navTreeCtrl_->SetItemData(cashflowWithTermAccounts, new mmTreeItemData("Cash Flow - With Term Accounts", new mmReportCashFlowTermAccounts()));

    wxTreeItemId cashflowSpecificAccounts = navTreeCtrl_->AppendItem(cashFlow, _("Cash Flow - Specific Accounts"), 4, 4);
    navTreeCtrl_->SetItemData(cashflowSpecificAccounts, new mmTreeItemData("Cash Flow - Specific Accounts", new mmReportCashFlowSpecificAccounts()));

    wxTreeItemId cashflowSpecificAccountsDaily = navTreeCtrl_->AppendItem(cashFlow, _("Daily Cash Flow - Specific Accounts"), 4, 4);
    navTreeCtrl_->SetItemData(cashflowSpecificAccountsDaily, new mmTreeItemData("Daily Cash Flow - Specific Accounts", new mmReportDailyCashFlowSpecificAccounts()));

    ///////////////////////////////////////////////////////////////////

    wxTreeItemId transactionList = navTreeCtrl_->AppendItem(reports, _("Transaction Report"), 6, 6);
    navTreeCtrl_->SetItemData(transactionList, new mmTreeItemData("Transaction Report"));

    ///////////////////////////////////////////////////////////////////

    const auto &records = Model_Report::instance().all(Model_Report::COL_GROUPNAME, Model_Report::COL_REPORTNAME);
    wxTreeItemId group;
    wxString group_name;
    for (const auto& record : records)
    {
        bool no_group = record.GROUPNAME.empty();
        if (group_name != record.GROUPNAME && !no_group)
        {
            group = navTreeCtrl_->AppendItem(reports, wxGetTranslation(record.GROUPNAME), 8, 8);
            navTreeCtrl_->SetItemData(group, new mmTreeItemData(record.GROUPNAME, 0));
            group_name = record.GROUPNAME;
        }
        Model_Report::Data* r = Model_Report::instance().get(record.REPORTID);
        wxTreeItemId item = navTreeCtrl_->AppendItem(no_group ? reports : group
            , wxGetTranslation(record.REPORTNAME), 8, 8);
        navTreeCtrl_->SetItemData(item, new mmTreeItemData(wxString::Format("id:%i/%s", r->REPORTID, r->REPORTNAME)
            , new mmGeneralReport(r)));
    }
}

void mmGUIFrame::updateReportCategoryExpensesGoesNavigation(wxTreeItemId& categsOverTime)
{
    bool ignoreFuture = mmIniOptions::instance().ignoreFutureTransactions_;

    wxTreeItemId categsOverTimeCalMonth = navTreeCtrl_->AppendItem(categsOverTime
        , _("Last Calendar Month"), 4, 4);
    navTreeCtrl_->SetItemData(categsOverTimeCalMonth
        , new mmTreeItemData("Where the Money Goes - Last Calendar Month"
        , new mmReportCategoryExpensesGoesLastMonth()));

    if (ignoreFuture)
    {
        wxTreeItemId categsOverTimeCurrentMonth = navTreeCtrl_->AppendItem(categsOverTime
            , _("Current Month to Date"), 4, 4);
        navTreeCtrl_->SetItemData(categsOverTimeCurrentMonth
            , new mmTreeItemData("Where the Money Goes - Current Month"
            , new mmReportCategoryExpensesGoesCurrentMonthToDate()));
    }
    else
    {
        wxTreeItemId categsOverTimeCurrentMonth = navTreeCtrl_->AppendItem(categsOverTime
            , _("Current Month"), 4, 4);
        navTreeCtrl_->SetItemData(categsOverTimeCurrentMonth
            , new mmTreeItemData("Where the Money Goes - Current Month"
            , new mmReportCategoryExpensesGoesCurrentMonth()));
    }

    wxTreeItemId categsOverTimeLast30 = navTreeCtrl_->AppendItem(categsOverTime
        , _("Last 30 Days"), 4, 4);
    navTreeCtrl_->SetItemData(categsOverTimeLast30
        , new mmTreeItemData("Where the Money Goes - Last 30 Days"
        , new mmReportCategoryExpensesGoesLast30Days()));

    wxTreeItemId categsOverTimeLastYear = navTreeCtrl_->AppendItem(categsOverTime, _("Last Year"), 4, 4);
    navTreeCtrl_->SetItemData(categsOverTimeLastYear
        , new mmTreeItemData("Where the Money Goes - Last Year"
        , new mmReportCategoryExpensesGoesLastYear()));

    if (ignoreFuture)
    {
        wxTreeItemId categsOverTimeCurrentYear = navTreeCtrl_->AppendItem(categsOverTime
            , _("Current Year to Date"), 4, 4);
        navTreeCtrl_->SetItemData(categsOverTimeCurrentYear
            , new mmTreeItemData("Where the Money Goes - Current Year"
            , new mmReportCategoryExpensesGoesCurrentYearToDate()));
    }
    else
    {
        wxTreeItemId categsOverTimeCurrentYear = navTreeCtrl_->AppendItem(categsOverTime
            , _("Current Year"), 4, 4);
        navTreeCtrl_->SetItemData(categsOverTimeCurrentYear
            , new mmTreeItemData("Where the Money Goes - Current Year"
            , new mmReportCategoryExpensesGoesCurrentYear()));
    }

    int day = Model_Infotable::instance().GetIntInfo("FINANCIAL_YEAR_START_DAY", 1);
    int month = Model_Infotable::instance().GetIntInfo("FINANCIAL_YEAR_START_MONTH", 7);
    if (financialYearIsDifferent())
    {
        wxTreeItemId categsOverTimeLastFinancialYear = navTreeCtrl_->AppendItem(categsOverTime
            , _("Last Financial Year"), 4, 4);
        navTreeCtrl_->SetItemData(categsOverTimeLastFinancialYear
            , new mmTreeItemData("Where the Money Goes - Last Financial Year"
            , new mmReportCategoryExpensesGoesLastFinancialYear(day, month)));

        if (ignoreFuture)
        {
            wxTreeItemId categsOverTimeCurrentFinancialYear = navTreeCtrl_->AppendItem(categsOverTime
                , _("Current Financial Year to Date"), 4, 4);
            navTreeCtrl_->SetItemData(categsOverTimeCurrentFinancialYear
                , new mmTreeItemData("Where the Money Goes - Current Financial Year"
                , new mmReportCategoryExpensesGoesCurrentFinancialYearToDate(day, month)));
        }
        else
        {
            wxTreeItemId categsOverTimeCurrentFinancialYear = navTreeCtrl_->AppendItem(categsOverTime
                , _("Current Financial Year"), 4, 4);
            navTreeCtrl_->SetItemData(categsOverTimeCurrentFinancialYear
                , new mmTreeItemData("Where the Money Goes - Current Financial Year"
                , new mmReportCategoryExpensesGoesCurrentFinancialYear(day, month)));
        }
    }
}

void mmGUIFrame::updateReportCategoryExpensesComesNavigation(wxTreeItemId& posCategs)
{
    bool ignoreFuture = mmIniOptions::instance().ignoreFutureTransactions_;

    wxTreeItemId posCategsCalMonth = navTreeCtrl_->AppendItem(posCategs
        , _("Last Calendar Month"), 4, 4);
    navTreeCtrl_->SetItemData(posCategsCalMonth
        , new mmTreeItemData("Where the Money Comes From - Last Calendar Month"
        , new mmReportCategoryExpensesComesLastMonth()));

    if (ignoreFuture)
    {
        wxTreeItemId posCategsCurrentMonth = navTreeCtrl_->AppendItem(posCategs
            , _("Current Month to Date"), 4, 4);
        navTreeCtrl_->SetItemData(posCategsCurrentMonth
            , new mmTreeItemData("Where the Money Comes From - Current Month"
            , new mmReportCategoryExpensesComesCurrentMonthToDate()));
    }
    else
    {
        wxTreeItemId posCategsCurrentMonth = navTreeCtrl_->AppendItem(posCategs
            , _("Current Month"), 4, 4);
        navTreeCtrl_->SetItemData(posCategsCurrentMonth
            , new mmTreeItemData("Where the Money Comes From - Current Month"
            , new mmReportCategoryExpensesComesCurrentMonth()));
    }

    wxTreeItemId posCategsTimeLast30 = navTreeCtrl_->AppendItem(posCategs
        , _("Last 30 Days"), 4, 4);
    navTreeCtrl_->SetItemData(posCategsTimeLast30
        , new mmTreeItemData("Where the Money Comes From - Last 30 Days"
        , new mmReportCategoryExpensesComesLast30Days()));

    wxTreeItemId posCategsTimeLastYear = navTreeCtrl_->AppendItem(posCategs
        , _("Last Year"), 4, 4);
    navTreeCtrl_->SetItemData(posCategsTimeLastYear
        , new mmTreeItemData("Where the Money Comes From - Last Year"
        , new mmReportCategoryExpensesComesLastYear()));

    if (ignoreFuture)
    {
        wxTreeItemId posCategsTimeCurrentYear = navTreeCtrl_->AppendItem(posCategs
            , _("Current Year to Date"), 4, 4);
        navTreeCtrl_->SetItemData(posCategsTimeCurrentYear
            , new mmTreeItemData("Where the Money Comes From - Current Year"
            , new mmReportCategoryExpensesComesCurrentYearToDate()));
    }
    else
    {
        wxTreeItemId posCategsTimeCurrentYear = navTreeCtrl_->AppendItem(posCategs
            , _("Current Year"), 4, 4);
        navTreeCtrl_->SetItemData(posCategsTimeCurrentYear
            , new mmTreeItemData("Where the Money Comes From - Current Year"
            , new mmReportCategoryExpensesComesCurrentYear()));
    }

    int day = Model_Infotable::instance().GetIntInfo("FINANCIAL_YEAR_START_DAY", 1);
    int month = Model_Infotable::instance().GetIntInfo("FINANCIAL_YEAR_START_MONTH", 7);
    if (financialYearIsDifferent())
    {
        wxTreeItemId posCategsTimeLastFinancialYear = navTreeCtrl_->AppendItem(posCategs
            , _("Last Financial Year"), 4, 4);
        navTreeCtrl_->SetItemData(posCategsTimeLastFinancialYear
            , new mmTreeItemData("Where the Money Comes From - Last Financial Year"
            , new mmReportCategoryExpensesComesLastFinancialYear(day, month)));

        if (ignoreFuture)
        {
            wxTreeItemId posCategsTimeCurrentFinancialYear = navTreeCtrl_->AppendItem(posCategs
                , _("Current Financial Year to Date"), 4, 4);
            navTreeCtrl_->SetItemData(posCategsTimeCurrentFinancialYear
                , new mmTreeItemData("Where the Money Comes From - Current Financial Year"
                , new mmReportCategoryExpensesComesCurrentFinancialYearToDate(day, month)));
        }
        else
        {
            wxTreeItemId posCategsTimeCurrentFinancialYear = navTreeCtrl_->AppendItem(posCategs
                , _("Current Financial Year"), 4, 4);
            navTreeCtrl_->SetItemData(posCategsTimeCurrentFinancialYear
                , new mmTreeItemData("Where the Money Comes From - Current Financial Year"
                , new mmReportCategoryExpensesComesCurrentFinancialYear(day, month)));
        }
    }
}

void mmGUIFrame::updateReportCategoryNavigation(wxTreeItemId& categs)
{
    int day = Model_Infotable::instance().GetIntInfo("FINANCIAL_YEAR_START_DAY", 1);
    int month = Model_Infotable::instance().GetIntInfo("FINANCIAL_YEAR_START_MONTH", 7);
    bool ignoreFuture = mmIniOptions::instance().ignoreFutureTransactions_;

    wxTreeItemId categsCalMonth = navTreeCtrl_->AppendItem(categs
        , _("Last Calendar Month"), 4, 4);
    navTreeCtrl_->SetItemData(categsCalMonth
        , new mmTreeItemData("Categories - Last Calendar Month"
        , new mmReportCategoryExpensesCategoriesLastMonth()));

    if (ignoreFuture)
    {
        wxTreeItemId categsCurrentMonth = navTreeCtrl_->AppendItem(categs
            , _("Current Month to Date"), 4, 4);
        navTreeCtrl_->SetItemData(categsCurrentMonth
            , new mmTreeItemData("Categories - Current Month"
            , new mmReportCategoryExpensesCategoriesCurrentMonthToDate()));
    }
    else
    {
        wxTreeItemId categsCurrentMonth = navTreeCtrl_->AppendItem(categs
            , _("Current Month"), 4, 4);
        navTreeCtrl_->SetItemData(categsCurrentMonth
            , new mmTreeItemData("Categories - Current Month"
            , new mmReportCategoryExpensesCategoriesCurrentMonth()));
    }

    wxTreeItemId categsTimeLast30 = navTreeCtrl_->AppendItem(categs, _("Last 30 Days"), 4, 4);
    navTreeCtrl_->SetItemData(categsTimeLast30
        , new mmTreeItemData("Categories - Last 30 Days"
        , new mmReportCategoryExpensesCategoriesLast30Days()));

    wxTreeItemId categsTimeLastYear = navTreeCtrl_->AppendItem(categs, _("Last Year"), 4, 4);
    navTreeCtrl_->SetItemData(categsTimeLastYear
        , new mmTreeItemData("Categories - Last Year"
        , new mmReportCategoryExpensesCategoriesLastYear()));

    if (ignoreFuture)
    {
        wxTreeItemId categsTimeCurrentYear = navTreeCtrl_->AppendItem(categs
            , _("Current Year to Date"), 4, 4);
        navTreeCtrl_->SetItemData(categsTimeCurrentYear
            , new mmTreeItemData("Categories - Current Year"
            , new mmReportCategoryExpensesCategoriesCurrentYearToDate()));
    }
    else
    {
        wxTreeItemId categsTimeCurrentYear = navTreeCtrl_->AppendItem(categs
            , _("Current Year"), 4, 4);
        navTreeCtrl_->SetItemData(categsTimeCurrentYear
            , new mmTreeItemData("Categories - Current Year"
            , new mmReportCategoryExpensesCategoriesCurrentYear()));
    }

    if (financialYearIsDifferent())
    {
        wxTreeItemId categsTimeLastFinancialYear = navTreeCtrl_->AppendItem(categs
            , _("Last Financial Year"), 4, 4);
        navTreeCtrl_->SetItemData(categsTimeLastFinancialYear
            , new mmTreeItemData("Categories - Last Financial Year"
            , new mmReportCategoryExpensesCategoriesLastFinancialYear(day, month)));

        if (ignoreFuture)
        {
            wxTreeItemId categsTimeCurrentFinancialYear = navTreeCtrl_->AppendItem(categs
                , _("Current Financial Year to Date"), 4, 4);
            navTreeCtrl_->SetItemData(categsTimeCurrentFinancialYear
                , new mmTreeItemData("Categories - Current Financial Year"
                , new mmReportCategoryExpensesCategoriesCurrentFinancialYearToDate(day, month)));
        }
        else
        {
            wxTreeItemId categsTimeCurrentFinancialYear = navTreeCtrl_->AppendItem(categs
                , _("Current Financial Year"), 4, 4);
            navTreeCtrl_->SetItemData(categsTimeCurrentFinancialYear
                , new mmTreeItemData("Categories - Current Financial Year"
                , new mmReportCategoryExpensesCategoriesCurrentFinancialYear(day, month)));
        }
    }
}

void mmGUIFrame::updateReportPayeeNavigation(wxTreeItemId& payeesOverTime)
{
    int day = Model_Infotable::instance().GetIntInfo("FINANCIAL_YEAR_START_DAY", 1);
    int month = Model_Infotable::instance().GetIntInfo("FINANCIAL_YEAR_START_MONTH", 7);
    bool ignoreFuture = mmIniOptions::instance().ignoreFutureTransactions_;

    wxTreeItemId payeesOverTimeCalMonth = navTreeCtrl_->AppendItem(payeesOverTime
        , _("Last Calendar Month"), 4, 4);
    navTreeCtrl_->SetItemData(payeesOverTimeCalMonth
        , new mmTreeItemData("Payees - Last Calendar Month"
        , new mmReportPayeeExpensesLastMonth()));

    if (ignoreFuture)
    {
        wxTreeItemId payeesOverTimeCurrentMonth = navTreeCtrl_->AppendItem(payeesOverTime
            , _("Current Month to Date"), 4, 4);
        navTreeCtrl_->SetItemData(payeesOverTimeCurrentMonth
            , new mmTreeItemData("Payees - Current Month"
            , new mmReportPayeeExpensesCurrentMonthToDate()));
    }
    else
    {
        wxTreeItemId payeesOverTimeCurrentMonth = navTreeCtrl_->AppendItem(payeesOverTime
            , _("Current Month"), 4, 4);
        navTreeCtrl_->SetItemData(payeesOverTimeCurrentMonth
            , new mmTreeItemData("Payees - Current Month"
            , new mmReportPayeeExpensesCurrentMonth()));
    }

    wxTreeItemId payeesOverTimeLast30 = navTreeCtrl_->AppendItem(payeesOverTime
        , _("Last 30 Days"), 4, 4);
    navTreeCtrl_->SetItemData(payeesOverTimeLast30
        , new mmTreeItemData("Payees - Last 30 Days"
        , new mmReportPayeeExpensesLast30Days()));

    wxTreeItemId payeesOverTimeLastYear = navTreeCtrl_->AppendItem(payeesOverTime
        , _("Last Year"), 4, 4);
    navTreeCtrl_->SetItemData(payeesOverTimeLastYear
        , new mmTreeItemData("Payees - Last Year"
        , new mmReportPayeeExpensesLastYear()));

    if (ignoreFuture)
    {
        wxTreeItemId payeesOverTimeCurrentYear = navTreeCtrl_->AppendItem(payeesOverTime
            , _("Current Year to Date"), 4, 4);
        navTreeCtrl_->SetItemData(payeesOverTimeCurrentYear
            , new mmTreeItemData("Payees - Current Year"
            , new mmReportPayeeExpensesCurrentYearToDate()));
    }
    else
    {
        wxTreeItemId payeesOverTimeCurrentYear = navTreeCtrl_->AppendItem(payeesOverTime
            , _("Current Year"), 4, 4);
        navTreeCtrl_->SetItemData(payeesOverTimeCurrentYear
            , new mmTreeItemData("Payees - Current Year"
            , new mmReportPayeeExpensesCurrentYear()));
    }

    if (financialYearIsDifferent())
    {
        wxTreeItemId payeesOverTimeLastFinancialYear = navTreeCtrl_->AppendItem(payeesOverTime
            , _("Last Financial Year"), 4, 4);
        navTreeCtrl_->SetItemData(payeesOverTimeLastFinancialYear
            , new mmTreeItemData("Payees - Last Financial Year"
            , new mmReportPayeeExpensesLastFinancialYear(day, month)));

        if (ignoreFuture)
        {
            wxTreeItemId payeesOverTimeCurrentFinancialYear = navTreeCtrl_->AppendItem(payeesOverTime
                , _("Current Financial Year to Date"), 4, 4);
            navTreeCtrl_->SetItemData(payeesOverTimeCurrentFinancialYear
                , new mmTreeItemData("Payees - Current Financial Year"
                , new mmReportPayeeExpensesCurrentFinancialYearToDate(day, month)));
        }
        else
        {
            wxTreeItemId payeesOverTimeCurrentFinancialYear = navTreeCtrl_->AppendItem(payeesOverTime
                , _("Current Financial Year"), 4, 4);
            navTreeCtrl_->SetItemData(payeesOverTimeCurrentFinancialYear
                , new mmTreeItemData("Payees - Current Financial Year"
                , new mmReportPayeeExpensesCurrentFinancialYear(day, month)));
        }
    }
}

