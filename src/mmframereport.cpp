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
            return e.what();
        }
        catch (...)
        {
            return _("Caught exception");
        }

        Model_Report::outputReportFile(out);
        return "";
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
    wxTreeItemId reportsSummary = navTreeCtrl_->AppendItem(reports
        , _("Monthly Summary of Accounts"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(reportsSummary, new mmTreeItemData("Monthly Summary of Accounts"
        , new mmReportSummaryByDate(this, 0)));

    wxTreeItemId categsGoes = navTreeCtrl_->AppendItem(reports
        , _("Where the Money Goes"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(categsGoes, new mmTreeItemData("Where the Money Goes"
        , new mmReportCategoryExpensesGoes()));

    wxTreeItemId categsComes = navTreeCtrl_->AppendItem(reports
        , _("Where the Money Comes From"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(categsComes
        , new mmTreeItemData("Where the Money Comes From"
        , new mmReportCategoryExpensesComes()));
    
    wxTreeItemId categs = navTreeCtrl_->AppendItem(reports
        , _("Categories"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(categs
        , new mmTreeItemData("Categories"
        , new mmReportCategoryExpensesCategories()));

    wxTreeItemId categsOverTime = navTreeCtrl_->AppendItem(categs
        , _("Over Time"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(categsOverTime
        , new mmTreeItemData("Categories - Over Time"
        , new mmReportCategoryOverTimePerformance()));

    wxTreeItemId payeesOverTime = navTreeCtrl_->AppendItem(reports
        , _("Payees"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(payeesOverTime
        , new mmTreeItemData("Payee Report"
        , new mmReportPayeeExpenses()));

    wxTreeItemId incexpOverTime = navTreeCtrl_->AppendItem(reports, _("Income vs Expenses")
        , img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(incexpOverTime, new mmTreeItemData("Income vs Expenses"
        , new mmReportIncomeExpenses()));

    wxTreeItemId incexpMonthly = navTreeCtrl_->AppendItem(incexpOverTime
        , _("Monthly"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(incexpMonthly
        , new mmTreeItemData("Income vs Expenses - Monthly"
        , new mmReportIncomeExpensesMonthly()));

    ///////////////////////////////////////////////////////////////////

    wxTreeItemId incexpOverTimeSpecificAccounts = navTreeCtrl_->AppendItem(reports
        , _("Income vs Expenses - Specific Accounts"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(incexpOverTimeSpecificAccounts, new mmTreeItemData("Income vs Expenses - Specific Accounts"
        , new mmReportIncomeExpensesSpecificAccounts()));

    wxTreeItemId incexpOverTimeLastYearSpecificAccounts = navTreeCtrl_->AppendItem(incexpOverTimeSpecificAccounts
        , _("Monthly"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemData(incexpOverTimeLastYearSpecificAccounts
        , new mmTreeItemData("Income vs Expenses - Monthly - Specific Accounts"
        , new mmReportIncomeExpensesMonthlySpecificAccounts()));

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

