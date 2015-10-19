/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2012 Stefano Giorgio
 Copyright (C) 2013, 2015 Nikolay
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

#include "images_list.h"
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
#include "reports/summary.h"
#include "reports/summarystocks.h"
#include "reports/myusage.h"
#include "model/Model_Budgetyear.h"
#include "model/Model_Report.h"

const char *group_report_template = R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8" />
    <meta http - equiv = "Content-Type" content = "text/html" />
    <title><TMPL_VAR REPORTNAME></title>
    <script src = "ChartNew.js"></script>
    <script src = "sorttable.js"></script>
    <link href = "master.css" rel = "stylesheet" />
</head>
<body>

<div class = "container">
<h3><TMPL_VAR REPORTNAME></h3>
<TMPL_VAR TODAY><hr>
<div class = "row">
<div class = "col-xs-2"></div>
<div class = "col-xs-8">

<table class = "table">
    <thead>
        <tr>
            <th>REPORTID</th>
            <th>REPORTNAME</th>
        </tr>
    </thead>
    <tbody>
        <TMPL_LOOP NAME=CONTENTS>
            <tr>
            <td><TMPL_VAR REPORTID></td>
            <td><TMPL_VAR REPORTNAME></td>
            </tr>
        </TMPL_LOOP>
    </tbody>
</table>
</div></div></div></body>
</html>
)";

class mmGeneralGroupReport : public mmPrintableBase
{
public:
    mmGeneralGroupReport(const wxString& groupname): mmPrintableBase("mmGeneralGroupReport"
        , _("mmGeneralGroupReport")), m_group_name(groupname)
    {
        m_sub_reports = Model_Report::instance().find(Model_Report::GROUPNAME(groupname));
    }

    wxString getHTMLText()
    {
        loop_t contents;
        for (const auto & report : m_sub_reports)
            contents += report.to_row_t();

        mm_html_template report(group_report_template);
        report(L"REPORTNAME") = this->local_title() + " For " + this->m_group_name;
        report(L"CONTENTS") = contents;

        wxString out = wxEmptyString;
        try 
        {
            out = report.Process();
        }
        catch (const syntax_ex& e)
        {
            out = e.what();
        }
        catch (...)
        {
            // TODO
        }

        Model_Report::outputReportFile(out);
        return out;
    }
private:
    wxString m_group_name;
    Model_Report::Data_Set m_sub_reports;
};

void mmGUIFrame::updateReportNavigation(wxTreeItemId& reports, wxTreeItemId& budgeting)
{
    //////////////////////////////////////////////////////////////////
    wxTreeItemId myusage = navTreeCtrl_->AppendItem(reports
        , _("My Usage"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(myusage, new mmTreeItemData(new mmReportMyUsage()));

    //////////////////////////////////////////////////////////////////
    wxTreeItemId reportsSummary = navTreeCtrl_->AppendItem(reports, _("Monthly Summary of Accounts")
        , img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(reportsSummary, new mmTreeItemData("Monthly Summary of Accounts"
        , new mmReportSummaryByDate(this, 0)));

    int day = Model_Infotable::instance().GetIntInfo("FINANCIAL_YEAR_START_DAY", 1);
    int month = Model_Infotable::instance().GetIntInfo("FINANCIAL_YEAR_START_MONTH", 7);
    bool ignoreFuture = mmIniOptions::instance().ignoreFutureTransactions_;
    wxTreeItemId categsOverTime = navTreeCtrl_->AppendItem(reports
        , _("Where the Money Goes"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(categsOverTime, new mmTreeItemData("Where the Money Goes"
        , new mmReportCategoryExpensesGoes()));

    this->updateReportCategoryExpensesGoesNavigation(categsOverTime);

    wxTreeItemId posCategs = navTreeCtrl_->AppendItem(reports
        , _("Where the Money Comes From"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(posCategs
        , new mmTreeItemData("Where the Money Comes From"
        , new mmReportCategoryExpensesComes()));
    
    this->updateReportCategoryExpensesComesNavigation(posCategs);

    wxTreeItemId categs = navTreeCtrl_->AppendItem(reports
        , _("Categories"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(categs
        , new mmTreeItemData("Categories - Over Time"
        , new mmReportCategoryOverTimePerformance()));

    this->updateReportCategoryNavigation(categs);

    wxTreeItemId payeesOverTime = navTreeCtrl_->AppendItem(reports
        , _("Payees"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(payeesOverTime
        , new mmTreeItemData("Payee Report"
        , new mmReportPayeeExpenses()));

    this->updateReportPayeeNavigation(payeesOverTime);

    wxTreeItemId incexpOverTime = navTreeCtrl_->AppendItem(reports, _("Income vs Expenses")
        , img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(incexpOverTime, new mmTreeItemData("Income vs Expenses"
        , new mmReportIncomeExpensesAllTime()));

    wxTreeItemId incexpOverTimeCalMonth = navTreeCtrl_->AppendItem(incexpOverTime
        , _("Last Calendar Month"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(incexpOverTimeCalMonth
        , new mmTreeItemData("Income vs Expenses - Last Calendar Month"
        , new mmReportIncomeExpensesLastMonth()));

    if (ignoreFuture)
    {
        wxTreeItemId incexpOverTimeCurrentMonth = navTreeCtrl_->AppendItem(incexpOverTime
            , _("Current Month to Date"), img::PIECHART_PNG, img::PIECHART_PNG);
        navTreeCtrl_->SetItemData(incexpOverTimeCurrentMonth
            , new mmTreeItemData("Income vs Expenses - Current Month"
            , new mmReportIncomeExpensesCurrentMonthToDate()));
    }
    else
    {
        wxTreeItemId incexpOverTimeCurrentMonth = navTreeCtrl_->AppendItem(incexpOverTime
            , _("Current Month"), img::PIECHART_PNG, img::PIECHART_PNG);
        navTreeCtrl_->SetItemData(incexpOverTimeCurrentMonth
            , new mmTreeItemData("Income vs Expenses - Current Month"
            , new mmReportIncomeExpensesCurrentMonth()));
    }

    wxTreeItemId incexpOverTimeLast30 = navTreeCtrl_->AppendItem(incexpOverTime
        , _("Last 30 Days"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(incexpOverTimeLast30
        , new mmTreeItemData("Income vs Expenses - Last 30 Days"
        , new mmReportIncomeExpensesLast30Days()));

    wxTreeItemId incexpOverTimeLastYear = navTreeCtrl_->AppendItem(incexpOverTime
        , _("Last Year"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(incexpOverTimeLastYear
        , new mmTreeItemData("Income vs Expenses - Last Year"
        , new mmReportIncomeExpensesLastYear()));

    if (ignoreFuture)
    {
        wxTreeItemId incexpOverTimeCurrentYear = navTreeCtrl_->AppendItem(incexpOverTime
            , _("Current Year to Date"), img::PIECHART_PNG, img::PIECHART_PNG);
        navTreeCtrl_->SetItemData(incexpOverTimeCurrentYear
            , new mmTreeItemData("Income vs Expenses - Current Year"
            , new mmReportIncomeExpensesCurrentYearToDate()));
    }
    else
    {
        wxTreeItemId incexpOverTimeCurrentYear = navTreeCtrl_->AppendItem(incexpOverTime
            , _("Current Year"), img::PIECHART_PNG, img::PIECHART_PNG);
        navTreeCtrl_->SetItemData(incexpOverTimeCurrentYear
            , new mmTreeItemData("Income vs Expenses - Current Year"
            , new mmReportIncomeExpensesCurrentYear()));
    }

    if (financialYearIsDifferent())
    {
        wxTreeItemId incexpOverTimeLastFinancialYear = navTreeCtrl_->AppendItem(incexpOverTime
            , _("Last Financial Year"), img::PIECHART_PNG, img::PIECHART_PNG);
        navTreeCtrl_->SetItemData(incexpOverTimeLastFinancialYear
            , new mmTreeItemData("Income vs Expenses - Last Financial Year"
            , new mmReportIncomeExpensesLastFinancialYear(day, month)));

        if (ignoreFuture)
        {
            wxTreeItemId incexpOverTimeCurrentFinancialYear = navTreeCtrl_->AppendItem(incexpOverTime
                , _("Current Financial Year to Date"), img::PIECHART_PNG, img::PIECHART_PNG);
            navTreeCtrl_->SetItemData(incexpOverTimeCurrentFinancialYear
                , new mmTreeItemData("Income vs Expenses - Current Financial Year to Date"
                , new mmReportIncomeExpensesCurrentFinancialYearToDate(day, month)));
        }
        else
        {
            wxTreeItemId incexpOverTimeCurrentFinancialYear = navTreeCtrl_->AppendItem(incexpOverTime
                , _("Current Financial Year"), img::PIECHART_PNG, img::PIECHART_PNG);
            navTreeCtrl_->SetItemData(incexpOverTimeCurrentFinancialYear
                , new mmTreeItemData("Income vs Expenses - Current Financial Year"
                , new mmReportIncomeExpensesCurrentFinancialYear(day, month)));
        }
    }

    ///////////////////////////////////////////////////////////////////

    wxTreeItemId incexpOverTimeSpecificAccounts = navTreeCtrl_->AppendItem(reports
        , _("Income vs Expenses - Specific Accounts"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(incexpOverTimeSpecificAccounts, new mmTreeItemData("Income vs Expenses - Specific Accounts"
        , new mmReportIncomeExpensesAllTimeSpecificAccounts()));

    wxTreeItemId incexpOverTimeCalMonthSpecificAccounts = navTreeCtrl_->AppendItem(incexpOverTimeSpecificAccounts
        , _("Last Calendar Month"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(incexpOverTimeCalMonthSpecificAccounts
        , new mmTreeItemData("Income vs Expenses - Last Calendar Month - Specific Accounts"
        , new mmReportIncomeExpensesLastMonthSpecificAccounts()));

    if (ignoreFuture)
    {
        wxTreeItemId incexpOverTimeCurrentMonthSpecificAccounts = navTreeCtrl_->AppendItem(incexpOverTimeSpecificAccounts
            , _("Current Month to Date"), img::PIECHART_PNG, img::PIECHART_PNG);
        navTreeCtrl_->SetItemData(incexpOverTimeCurrentMonthSpecificAccounts
            , new mmTreeItemData("Income vs Expenses - Current Month - Specific Accounts"
            , new mmReportIncomeExpensesCurrentMonthToDateSpecificAccounts()));
    }
    else
    {
        wxTreeItemId incexpOverTimeCurrentMonthSpecificAccounts = navTreeCtrl_->AppendItem(incexpOverTimeSpecificAccounts
            , _("Current Month"), img::PIECHART_PNG, img::PIECHART_PNG);
        navTreeCtrl_->SetItemData(incexpOverTimeCurrentMonthSpecificAccounts
            , new mmTreeItemData("Income vs Expenses - Current Month - Specific Accounts"
            , new mmReportIncomeExpensesCurrentMonthSpecificAccounts()));
    }

    wxTreeItemId incexpOverTimeLast30SpecificAccounts = navTreeCtrl_->AppendItem(incexpOverTimeSpecificAccounts
        , _("Last 30 Days"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(incexpOverTimeLast30SpecificAccounts
        , new mmTreeItemData("Income vs Expenses - Last 30 Days - Specific Accounts"
        , new mmReportIncomeExpensesLast30DaysSpecificAccounts()));

    wxTreeItemId incexpOverTimeLastYearSpecificAccounts = navTreeCtrl_->AppendItem(incexpOverTimeSpecificAccounts
        , _("Last Year"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(incexpOverTimeLastYearSpecificAccounts
        , new mmTreeItemData("Income vs Expenses - Last Year - Specific Accounts"
        , new mmReportIncomeExpensesLastYearSpecificAccounts()));

    if (ignoreFuture)
    {
        wxTreeItemId incexpOverTimeCurrentYearSpecificAccounts = navTreeCtrl_->AppendItem(incexpOverTimeSpecificAccounts
            , _("Current Year to Date"), img::PIECHART_PNG, img::PIECHART_PNG);
        navTreeCtrl_->SetItemData(incexpOverTimeCurrentYearSpecificAccounts
            , new mmTreeItemData("Income vs Expenses - Current Year - Specific Accounts"
            , new mmReportIncomeExpensesCurrentYearToDateSpecificAccounts()));
    }
    else
    {
        wxTreeItemId incexpOverTimeCurrentYearSpecificAccounts = navTreeCtrl_->AppendItem(incexpOverTimeSpecificAccounts
            , _("Current Year"), img::PIECHART_PNG, img::PIECHART_PNG);
        navTreeCtrl_->SetItemData(incexpOverTimeCurrentYearSpecificAccounts
            , new mmTreeItemData("Income vs Expenses - Current Year - Specific Accounts"
            , new mmReportIncomeExpensesCurrentYearSpecificAccounts()));
    }

    if (financialYearIsDifferent())
    {
        wxTreeItemId incexpOverTimeLastFinancialYearSpecificAccounts = navTreeCtrl_->AppendItem(incexpOverTimeSpecificAccounts
            , _("Last Financial Year"), img::PIECHART_PNG, img::PIECHART_PNG);
        navTreeCtrl_->SetItemData(incexpOverTimeLastFinancialYearSpecificAccounts
            , new mmTreeItemData("Income vs Expenses - Last Financial Year - Specific Accounts"
            , new mmReportIncomeExpensesLastFinancialYearSpecificAccounts(day, month)));

        if (ignoreFuture)
        {
            wxTreeItemId incexpOverTimeCurrentFinancialYearSpecificAccounts = navTreeCtrl_->AppendItem(incexpOverTimeSpecificAccounts
                , _("Current Financial Year to Date"), img::PIECHART_PNG, img::PIECHART_PNG);
            navTreeCtrl_->SetItemData(incexpOverTimeCurrentFinancialYearSpecificAccounts
                , new mmTreeItemData("Income vs Expenses - Current Financial Year - Specific Accounts"
                , new mmReportIncomeExpensesCurrentFinancialYearToDateSpecificAccounts(day, month)));
        }
        else
        {
            wxTreeItemId incexpOverTimeCurrentFinancialYearSpecificAccounts = navTreeCtrl_->AppendItem(incexpOverTimeSpecificAccounts
                , _("Current Financial Year"), img::PIECHART_PNG, img::PIECHART_PNG);
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
            budgetPerformance = navTreeCtrl_->AppendItem(reports
                , _("Budget Performance"), img::PIECHART_PNG, img::PIECHART_PNG);
            navTreeCtrl_->SetItemData(budgetPerformance, new mmTreeItemData("Budget Performance"));

            budgetSetupPerformance = navTreeCtrl_->AppendItem(reports
                , _("Budget Category Summary"), img::PIECHART_PNG, img::PIECHART_PNG);
            navTreeCtrl_->SetItemData(budgetSetupPerformance, new mmTreeItemData("Budget Setup Performance"));
        }

        int id = e.BUDGETYEARID;
        const wxString& name = e.BUDGETYEARNAME;

        wxTreeItemId bYear = navTreeCtrl_->AppendItem(budgeting, name, img::CALENDAR_PNG, img::CALENDAR_PNG);
        navTreeCtrl_->SetItemData(bYear, new mmTreeItemData(id, true));

        // Only add YEARS for Budget Performance
        if (name.length() < 5)
        {
            wxTreeItemId bYearData = navTreeCtrl_->AppendItem(budgetPerformance
                , name, img::PIECHART_PNG, img::PIECHART_PNG);
            navTreeCtrl_->SetItemData(bYearData, new mmTreeItemData(id, true));
        }
        wxTreeItemId bYearSetupData = navTreeCtrl_->AppendItem(budgetSetupPerformance
            , name, img::PIECHART_PNG, img::PIECHART_PNG);
        navTreeCtrl_->SetItemData(bYearSetupData, new mmTreeItemData(id, true));
        ++i;
    }

    ///////////////////////////////////////////////////////////////////
    wxTreeItemId cashFlow = navTreeCtrl_->AppendItem(reports
        , _("Cash Flow"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(cashFlow, new mmTreeItemData("Cash Flow", new mmReportCashFlowAllAccounts()));

    wxTreeItemId cashflowWithBankAccounts = navTreeCtrl_->AppendItem(cashFlow
        , _("Cash Flow - With Bank Accounts"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(cashflowWithBankAccounts, new mmTreeItemData("Cash Flow - With Bank Accounts"
        , new mmReportCashFlowBankAccounts()));

    wxTreeItemId cashflowWithTermAccounts = navTreeCtrl_->AppendItem(cashFlow
        , _("Cash Flow - With Term Accounts"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(cashflowWithTermAccounts, new mmTreeItemData("Cash Flow - With Term Accounts"
        , new mmReportCashFlowTermAccounts()));

    wxTreeItemId cashflowSpecificAccounts = navTreeCtrl_->AppendItem(cashFlow
        , _("Cash Flow - Specific Accounts"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(cashflowSpecificAccounts, new mmTreeItemData("Cash Flow - Specific Accounts"
        , new mmReportCashFlowSpecificAccounts()));

    wxTreeItemId cashflowSpecificAccountsDaily = navTreeCtrl_->AppendItem(cashFlow
        , _("Daily Cash Flow - Specific Accounts"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(cashflowSpecificAccountsDaily, new mmTreeItemData("Daily Cash Flow - Specific Accounts"
        , new mmReportDailyCashFlowSpecificAccounts()));

    ///////////////////////////////////////////////////////////////////

    wxTreeItemId transactionList = navTreeCtrl_->AppendItem(reports
        , _("Transaction Report"), img::FILTER_PNG, img::FILTER_PNG);
    navTreeCtrl_->SetItemData(transactionList, new mmTreeItemData("Transaction Report"));

    //////////////////////////////////////////////////////////////////

    wxTreeItemId stocksReportSummary = navTreeCtrl_->AppendItem(reports
        , _("Stocks Report"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(stocksReportSummary, new mmTreeItemData("Summary of Stocks"
        , new mmReportSummaryStocks()));

    wxTreeItemId stocksReportLast30 = navTreeCtrl_->AppendItem(stocksReportSummary
        , _("Last 30 Days"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(stocksReportLast30, new mmTreeItemData("Stocks Report - 30 Days",
        new mmReportChartStocks(new mmLast30Days())));

    wxTreeItemId stocksReport = navTreeCtrl_->AppendItem(stocksReportSummary
        , _("Last 365 Days"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(stocksReport, new mmTreeItemData("Stocks Report",
        new mmReportChartStocks(new mmLast365Days())));

    wxTreeItemId stocksReportLastYear = navTreeCtrl_->AppendItem(stocksReportSummary
        , _("Last Year"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(stocksReportLastYear, new mmTreeItemData("Stocks Report - Last Year",
        new mmReportChartStocks(new mmLastYear)));

    wxTreeItemId stocksReportCurrentYear = navTreeCtrl_->AppendItem(stocksReportSummary
        , _("Current Year"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(stocksReportCurrentYear, new mmTreeItemData("Stocks Report - Current Year",
        new mmReportChartStocks(new mmCurrentYear)));

    wxTreeItemId stocksReportAllTime = navTreeCtrl_->AppendItem(stocksReportSummary
        , _("All Time"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(stocksReportAllTime, new mmTreeItemData("Stocks Report - All Time",
        new mmReportChartStocks()));

    //////////////////////////////////////////////////////////////////

    /*GRM Reports*/
    auto records = Model_Report::instance().all();
    //Sort by group name and report name
    std::sort(records.begin(), records.end(), SorterByREPORTNAME());
    std::stable_sort(records.begin(), records.end(), SorterByGROUPNAME());

    wxTreeItemId group;
    wxString group_name;
    for (const auto& record : records)
    {
        bool no_group = record.GROUPNAME.empty();
        if (group_name != record.GROUPNAME && !no_group)
        {
            group = navTreeCtrl_->AppendItem(reports
                , wxGetTranslation(record.GROUPNAME), img::CUSTOMSQL_GRP_PNG, img::CUSTOMSQL_GRP_PNG);
            navTreeCtrl_->SetItemData(group, new mmTreeItemData(record.GROUPNAME
                , new mmGeneralGroupReport(record.GROUPNAME)));
            group_name = record.GROUPNAME;
        }
        Model_Report::Data* r = Model_Report::instance().get(record.REPORTID);
        wxTreeItemId item = navTreeCtrl_->AppendItem(no_group ? reports : group
            , wxGetTranslation(record.REPORTNAME), img::CUSTOMSQL_PNG, img::CUSTOMSQL_PNG);
        navTreeCtrl_->SetItemData(item, new mmTreeItemData(r->REPORTNAME, new mmGeneralReport(r)));
    }
}

void mmGUIFrame::updateReportCategoryExpensesGoesNavigation(wxTreeItemId& categsOverTime)
{
    bool ignoreFuture = mmIniOptions::instance().ignoreFutureTransactions_;

    wxTreeItemId categsOverTimeCalMonth = navTreeCtrl_->AppendItem(categsOverTime
        , _("Last Calendar Month"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(categsOverTimeCalMonth
        , new mmTreeItemData("Where the Money Goes - Last Calendar Month"
        , new mmReportCategoryExpensesGoesLastMonth()));

    if (ignoreFuture)
    {
        wxTreeItemId categsOverTimeCurrentMonth = navTreeCtrl_->AppendItem(categsOverTime
            , _("Current Month to Date"), img::PIECHART_PNG, img::PIECHART_PNG);
        navTreeCtrl_->SetItemData(categsOverTimeCurrentMonth
            , new mmTreeItemData("Where the Money Goes - Current Month"
            , new mmReportCategoryExpensesGoesCurrentMonthToDate()));
    }
    else
    {
        wxTreeItemId categsOverTimeCurrentMonth = navTreeCtrl_->AppendItem(categsOverTime
            , _("Current Month"), img::PIECHART_PNG, img::PIECHART_PNG);
        navTreeCtrl_->SetItemData(categsOverTimeCurrentMonth
            , new mmTreeItemData("Where the Money Goes - Current Month"
            , new mmReportCategoryExpensesGoesCurrentMonth()));
    }

    wxTreeItemId categsOverTimeLast30 = navTreeCtrl_->AppendItem(categsOverTime
        , _("Last 30 Days"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(categsOverTimeLast30
        , new mmTreeItemData("Where the Money Goes - Last 30 Days"
        , new mmReportCategoryExpensesGoesLast30Days()));

    wxTreeItemId categsOverTimeLastYear = navTreeCtrl_->AppendItem(categsOverTime
        , _("Last Year"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(categsOverTimeLastYear
        , new mmTreeItemData("Where the Money Goes - Last Year"
        , new mmReportCategoryExpensesGoesLastYear()));

    if (ignoreFuture)
    {
        wxTreeItemId categsOverTimeCurrentYear = navTreeCtrl_->AppendItem(categsOverTime
            , _("Current Year to Date"), img::PIECHART_PNG, img::PIECHART_PNG);
        navTreeCtrl_->SetItemData(categsOverTimeCurrentYear
            , new mmTreeItemData("Where the Money Goes - Current Year"
            , new mmReportCategoryExpensesGoesCurrentYearToDate()));
    }
    else
    {
        wxTreeItemId categsOverTimeCurrentYear = navTreeCtrl_->AppendItem(categsOverTime
            , _("Current Year"), img::PIECHART_PNG, img::PIECHART_PNG);
        navTreeCtrl_->SetItemData(categsOverTimeCurrentYear
            , new mmTreeItemData("Where the Money Goes - Current Year"
            , new mmReportCategoryExpensesGoesCurrentYear()));
    }

    int day = Model_Infotable::instance().GetIntInfo("FINANCIAL_YEAR_START_DAY", 1);
    int month = Model_Infotable::instance().GetIntInfo("FINANCIAL_YEAR_START_MONTH", 7);
    if (financialYearIsDifferent())
    {
        wxTreeItemId categsOverTimeLastFinancialYear = navTreeCtrl_->AppendItem(categsOverTime
            , _("Last Financial Year"), img::PIECHART_PNG, img::PIECHART_PNG);
        navTreeCtrl_->SetItemData(categsOverTimeLastFinancialYear
            , new mmTreeItemData("Where the Money Goes - Last Financial Year"
            , new mmReportCategoryExpensesGoesLastFinancialYear(day, month)));

        if (ignoreFuture)
        {
            wxTreeItemId categsOverTimeCurrentFinancialYear = navTreeCtrl_->AppendItem(categsOverTime
                , _("Current Financial Year to Date"), img::PIECHART_PNG, img::PIECHART_PNG);
            navTreeCtrl_->SetItemData(categsOverTimeCurrentFinancialYear
                , new mmTreeItemData("Where the Money Goes - Current Financial Year"
                , new mmReportCategoryExpensesGoesCurrentFinancialYearToDate(day, month)));
        }
        else
        {
            wxTreeItemId categsOverTimeCurrentFinancialYear = navTreeCtrl_->AppendItem(categsOverTime
                , _("Current Financial Year"), img::PIECHART_PNG, img::PIECHART_PNG);
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
        , _("Last Calendar Month"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(posCategsCalMonth
        , new mmTreeItemData("Where the Money Comes From - Last Calendar Month"
        , new mmReportCategoryExpensesComesLastMonth()));

    if (ignoreFuture)
    {
        wxTreeItemId posCategsCurrentMonth = navTreeCtrl_->AppendItem(posCategs
            , _("Current Month to Date"), img::PIECHART_PNG, img::PIECHART_PNG);
        navTreeCtrl_->SetItemData(posCategsCurrentMonth
            , new mmTreeItemData("Where the Money Comes From - Current Month"
            , new mmReportCategoryExpensesComesCurrentMonthToDate()));
    }
    else
    {
        wxTreeItemId posCategsCurrentMonth = navTreeCtrl_->AppendItem(posCategs
            , _("Current Month"), img::PIECHART_PNG, img::PIECHART_PNG);
        navTreeCtrl_->SetItemData(posCategsCurrentMonth
            , new mmTreeItemData("Where the Money Comes From - Current Month"
            , new mmReportCategoryExpensesComesCurrentMonth()));
    }

    wxTreeItemId posCategsTimeLast30 = navTreeCtrl_->AppendItem(posCategs
        , _("Last 30 Days"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(posCategsTimeLast30
        , new mmTreeItemData("Where the Money Comes From - Last 30 Days"
        , new mmReportCategoryExpensesComesLast30Days()));

    wxTreeItemId posCategsTimeLastYear = navTreeCtrl_->AppendItem(posCategs
        , _("Last Year"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(posCategsTimeLastYear
        , new mmTreeItemData("Where the Money Comes From - Last Year"
        , new mmReportCategoryExpensesComesLastYear()));

    if (ignoreFuture)
    {
        wxTreeItemId posCategsTimeCurrentYear = navTreeCtrl_->AppendItem(posCategs
            , _("Current Year to Date"), img::PIECHART_PNG, img::PIECHART_PNG);
        navTreeCtrl_->SetItemData(posCategsTimeCurrentYear
            , new mmTreeItemData("Where the Money Comes From - Current Year"
            , new mmReportCategoryExpensesComesCurrentYearToDate()));
    }
    else
    {
        wxTreeItemId posCategsTimeCurrentYear = navTreeCtrl_->AppendItem(posCategs
            , _("Current Year"), img::PIECHART_PNG, img::PIECHART_PNG);
        navTreeCtrl_->SetItemData(posCategsTimeCurrentYear
            , new mmTreeItemData("Where the Money Comes From - Current Year"
            , new mmReportCategoryExpensesComesCurrentYear()));
    }

    int day = Model_Infotable::instance().GetIntInfo("FINANCIAL_YEAR_START_DAY", 1);
    int month = Model_Infotable::instance().GetIntInfo("FINANCIAL_YEAR_START_MONTH", 7);
    if (financialYearIsDifferent())
    {
        wxTreeItemId posCategsTimeLastFinancialYear = navTreeCtrl_->AppendItem(posCategs
            , _("Last Financial Year"), img::PIECHART_PNG, img::PIECHART_PNG);
        navTreeCtrl_->SetItemData(posCategsTimeLastFinancialYear
            , new mmTreeItemData("Where the Money Comes From - Last Financial Year"
            , new mmReportCategoryExpensesComesLastFinancialYear(day, month)));

        if (ignoreFuture)
        {
            wxTreeItemId posCategsTimeCurrentFinancialYear = navTreeCtrl_->AppendItem(posCategs
                , _("Current Financial Year to Date"), img::PIECHART_PNG, img::PIECHART_PNG);
            navTreeCtrl_->SetItemData(posCategsTimeCurrentFinancialYear
                , new mmTreeItemData("Where the Money Comes From - Current Financial Year"
                , new mmReportCategoryExpensesComesCurrentFinancialYearToDate(day, month)));
        }
        else
        {
            wxTreeItemId posCategsTimeCurrentFinancialYear = navTreeCtrl_->AppendItem(posCategs
                , _("Current Financial Year"), img::PIECHART_PNG, img::PIECHART_PNG);
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
        , _("Last Calendar Month"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(categsCalMonth
        , new mmTreeItemData("Categories - Last Calendar Month"
        , new mmReportCategoryExpensesCategoriesLastMonth()));

    if (ignoreFuture)
    {
        wxTreeItemId categsCurrentMonth = navTreeCtrl_->AppendItem(categs
            , _("Current Month to Date"), img::PIECHART_PNG, img::PIECHART_PNG);
        navTreeCtrl_->SetItemData(categsCurrentMonth
            , new mmTreeItemData("Categories - Current Month"
            , new mmReportCategoryExpensesCategoriesCurrentMonthToDate()));
    }
    else
    {
        wxTreeItemId categsCurrentMonth = navTreeCtrl_->AppendItem(categs
            , _("Current Month"), img::PIECHART_PNG, img::PIECHART_PNG);
        navTreeCtrl_->SetItemData(categsCurrentMonth
            , new mmTreeItemData("Categories - Current Month"
            , new mmReportCategoryExpensesCategoriesCurrentMonth()));
    }

    wxTreeItemId categsTimeLast30 = navTreeCtrl_->AppendItem(categs
        , _("Last 30 Days"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(categsTimeLast30
        , new mmTreeItemData("Categories - Last 30 Days"
        , new mmReportCategoryExpensesCategoriesLast30Days()));

    wxTreeItemId categsTimeLastYear = navTreeCtrl_->AppendItem(categs
        , _("Last Year"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(categsTimeLastYear
        , new mmTreeItemData("Categories - Last Year"
        , new mmReportCategoryExpensesCategoriesLastYear()));

    if (ignoreFuture)
    {
        wxTreeItemId categsTimeCurrentYear = navTreeCtrl_->AppendItem(categs
            , _("Current Year to Date"), img::PIECHART_PNG, img::PIECHART_PNG);
        navTreeCtrl_->SetItemData(categsTimeCurrentYear
            , new mmTreeItemData("Categories - Current Year"
            , new mmReportCategoryExpensesCategoriesCurrentYearToDate()));
    }
    else
    {
        wxTreeItemId categsTimeCurrentYear = navTreeCtrl_->AppendItem(categs
            , _("Current Year"), img::PIECHART_PNG, img::PIECHART_PNG);
        navTreeCtrl_->SetItemData(categsTimeCurrentYear
            , new mmTreeItemData("Categories - Current Year"
            , new mmReportCategoryExpensesCategoriesCurrentYear()));
    }

    if (financialYearIsDifferent())
    {
        wxTreeItemId categsTimeLastFinancialYear = navTreeCtrl_->AppendItem(categs
            , _("Last Financial Year"), img::PIECHART_PNG, img::PIECHART_PNG);
        navTreeCtrl_->SetItemData(categsTimeLastFinancialYear
            , new mmTreeItemData("Categories - Last Financial Year"
            , new mmReportCategoryExpensesCategoriesLastFinancialYear(day, month)));

        if (ignoreFuture)
        {
            wxTreeItemId categsTimeCurrentFinancialYear = navTreeCtrl_->AppendItem(categs
                , _("Current Financial Year to Date"), img::PIECHART_PNG, img::PIECHART_PNG);
            navTreeCtrl_->SetItemData(categsTimeCurrentFinancialYear
                , new mmTreeItemData("Categories - Current Financial Year"
                , new mmReportCategoryExpensesCategoriesCurrentFinancialYearToDate(day, month)));
        }
        else
        {
            wxTreeItemId categsTimeCurrentFinancialYear = navTreeCtrl_->AppendItem(categs
                , _("Current Financial Year"), img::PIECHART_PNG, img::PIECHART_PNG);
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
        , _("Last Calendar Month"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(payeesOverTimeCalMonth
        , new mmTreeItemData("Payees - Last Calendar Month"
        , new mmReportPayeeExpensesLastMonth()));

    if (ignoreFuture)
    {
        wxTreeItemId payeesOverTimeCurrentMonth = navTreeCtrl_->AppendItem(payeesOverTime
            , _("Current Month to Date"), img::PIECHART_PNG, img::PIECHART_PNG);
        navTreeCtrl_->SetItemData(payeesOverTimeCurrentMonth
            , new mmTreeItemData("Payees - Current Month"
            , new mmReportPayeeExpensesCurrentMonthToDate()));
    }
    else
    {
        wxTreeItemId payeesOverTimeCurrentMonth = navTreeCtrl_->AppendItem(payeesOverTime
            , _("Current Month"), img::PIECHART_PNG, img::PIECHART_PNG);
        navTreeCtrl_->SetItemData(payeesOverTimeCurrentMonth
            , new mmTreeItemData("Payees - Current Month"
            , new mmReportPayeeExpensesCurrentMonth()));
    }

    wxTreeItemId payeesOverTimeLast30 = navTreeCtrl_->AppendItem(payeesOverTime
        , _("Last 30 Days"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(payeesOverTimeLast30
        , new mmTreeItemData("Payees - Last 30 Days"
        , new mmReportPayeeExpensesLast30Days()));

    wxTreeItemId payeesOverTimeLastYear = navTreeCtrl_->AppendItem(payeesOverTime
        , _("Last Year"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(payeesOverTimeLastYear
        , new mmTreeItemData("Payees - Last Year"
        , new mmReportPayeeExpensesLastYear()));

    if (ignoreFuture)
    {
        wxTreeItemId payeesOverTimeCurrentYear = navTreeCtrl_->AppendItem(payeesOverTime
            , _("Current Year to Date"), img::PIECHART_PNG, img::PIECHART_PNG);
        navTreeCtrl_->SetItemData(payeesOverTimeCurrentYear
            , new mmTreeItemData("Payees - Current Year"
            , new mmReportPayeeExpensesCurrentYearToDate()));
    }
    else
    {
        wxTreeItemId payeesOverTimeCurrentYear = navTreeCtrl_->AppendItem(payeesOverTime
            , _("Current Year"), img::PIECHART_PNG, img::PIECHART_PNG);
        navTreeCtrl_->SetItemData(payeesOverTimeCurrentYear
            , new mmTreeItemData("Payees - Current Year"
            , new mmReportPayeeExpensesCurrentYear()));
    }

    if (financialYearIsDifferent())
    {
        wxTreeItemId payeesOverTimeLastFinancialYear = navTreeCtrl_->AppendItem(payeesOverTime
            , _("Last Financial Year"), img::PIECHART_PNG, img::PIECHART_PNG);
        navTreeCtrl_->SetItemData(payeesOverTimeLastFinancialYear
            , new mmTreeItemData("Payees - Last Financial Year"
            , new mmReportPayeeExpensesLastFinancialYear(day, month)));

        if (ignoreFuture)
        {
            wxTreeItemId payeesOverTimeCurrentFinancialYear = navTreeCtrl_->AppendItem(payeesOverTime
                , _("Current Financial Year to Date"), img::PIECHART_PNG, img::PIECHART_PNG);
            navTreeCtrl_->SetItemData(payeesOverTimeCurrentFinancialYear
                , new mmTreeItemData("Payees - Current Financial Year"
                , new mmReportPayeeExpensesCurrentFinancialYearToDate(day, month)));
        }
        else
        {
            wxTreeItemId payeesOverTimeCurrentFinancialYear = navTreeCtrl_->AppendItem(payeesOverTime
                , _("Current Financial Year"), img::PIECHART_PNG, img::PIECHART_PNG);
            navTreeCtrl_->SetItemData(payeesOverTimeCurrentFinancialYear
                , new mmTreeItemData("Payees - Current Financial Year"
                , new mmReportPayeeExpensesCurrentFinancialYear(day, month)));
        }
    }
}

