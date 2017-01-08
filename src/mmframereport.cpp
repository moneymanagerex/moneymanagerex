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
#include "reports/allreport.h"
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
    mmGeneralGroupReport(const wxString& groupname): mmPrintableBase(_("General Group Report"))
        , m_group_name(groupname)
    {
        m_sub_reports = Model_Report::instance().find(Model_Report::GROUPNAME(groupname));
    }

    wxString getHTMLText()
    {
        loop_t contents;
        for (const auto & report : m_sub_reports)
            contents += report.to_row_t();

        mm_html_template report(group_report_template);
        report(L"REPORTNAME") = this->m_title + " For " + this->m_group_name;
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
    wxTreeItemId myusage = m_nav_tree_ctrl->AppendItem(reports
        , _("My Usage"), img::PIECHART_PNG, img::PIECHART_PNG);
    m_nav_tree_ctrl->SetItemData(myusage, new mmTreeItemData(new mmReportMyUsage()));

    //////////////////////////////////////////////////////////////////
    wxTreeItemId reportsSummary = m_nav_tree_ctrl->AppendItem(reports
        , _("Monthly Summary of Accounts"), img::PIECHART_PNG, img::PIECHART_PNG);
    m_nav_tree_ctrl->SetItemData(reportsSummary, new mmTreeItemData("Monthly Summary of Accounts"
        , new mmReportSummaryByDate(this, 0)));

    wxTreeItemId categsGoes = m_nav_tree_ctrl->AppendItem(reports
        , _("Where the Money Goes"), img::PIECHART_PNG, img::PIECHART_PNG);
    m_nav_tree_ctrl->SetItemData(categsGoes, new mmTreeItemData("Where the Money Goes"
        , new mmReportCategoryExpensesGoes()));

    wxTreeItemId categsComes = m_nav_tree_ctrl->AppendItem(reports
        , _("Where the Money Comes From"), img::PIECHART_PNG, img::PIECHART_PNG);
    m_nav_tree_ctrl->SetItemData(categsComes
        , new mmTreeItemData("Where the Money Comes From"
        , new mmReportCategoryExpensesComes()));
    
    wxTreeItemId categs = m_nav_tree_ctrl->AppendItem(reports
        , _("Categories"), img::PIECHART_PNG, img::PIECHART_PNG);
    m_nav_tree_ctrl->SetItemData(categs
        , new mmTreeItemData("Categories"
        , new mmReportCategoryExpensesCategories()));

    wxTreeItemId categsOverTime = m_nav_tree_ctrl->AppendItem(categs
        , _("Over Time"), img::PIECHART_PNG, img::PIECHART_PNG);
    m_nav_tree_ctrl->SetItemData(categsOverTime
        , new mmTreeItemData("Categories - Over Time"
        , new mmReportCategoryOverTimePerformance()));

    wxTreeItemId payeesOverTime = m_nav_tree_ctrl->AppendItem(reports
        , _("Payees"), img::PIECHART_PNG, img::PIECHART_PNG);
    m_nav_tree_ctrl->SetItemData(payeesOverTime
        , new mmTreeItemData("Payee Report"
        , new mmReportPayeeExpenses()));

    wxTreeItemId incexpOverTime = m_nav_tree_ctrl->AppendItem(reports, _("Income vs Expenses")
        , img::PIECHART_PNG, img::PIECHART_PNG);
    m_nav_tree_ctrl->SetItemData(incexpOverTime, new mmTreeItemData("Income vs Expenses"
        , new mmReportIncomeExpenses()));

    wxTreeItemId incexpMonthly = m_nav_tree_ctrl->AppendItem(incexpOverTime
        , _("Monthly"), img::PIECHART_PNG, img::PIECHART_PNG);
    m_nav_tree_ctrl->SetItemData(incexpMonthly
        , new mmTreeItemData("Income vs Expenses - Monthly"
        , new mmReportIncomeExpensesMonthly()));

    ///////////////////////////////////////////////////////////////////

    wxTreeItemId incexpOverTimeSpecificAccounts = m_nav_tree_ctrl->AppendItem(reports
        , _("Income vs Expenses - Specific Accounts"), img::PIECHART_PNG, img::PIECHART_PNG);
    m_nav_tree_ctrl->SetItemData(incexpOverTimeSpecificAccounts, new mmTreeItemData("Income vs Expenses - Specific Accounts"
        , new mmReportIncomeExpensesSpecificAccounts()));

    wxTreeItemId incexpOverTimeLastYearSpecificAccounts = m_nav_tree_ctrl->AppendItem(incexpOverTimeSpecificAccounts
        , _("Monthly"), img::PIECHART_PNG, img::PIECHART_PNG);
    m_nav_tree_ctrl->SetItemData(incexpOverTimeLastYearSpecificAccounts
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
            budgetPerformance = m_nav_tree_ctrl->AppendItem(reports
                , _("Budget Performance"), img::PIECHART_PNG, img::PIECHART_PNG);
            m_nav_tree_ctrl->SetItemData(budgetPerformance, new mmTreeItemData("Budget Performance"));

            budgetSetupPerformance = m_nav_tree_ctrl->AppendItem(reports
                , _("Budget Category Summary"), img::PIECHART_PNG, img::PIECHART_PNG);
            m_nav_tree_ctrl->SetItemData(budgetSetupPerformance, new mmTreeItemData("Budget Setup Performance"));
        }

        int id = e.BUDGETYEARID;
        const wxString& name = e.BUDGETYEARNAME;

        wxTreeItemId bYear = m_nav_tree_ctrl->AppendItem(budgeting, name, img::CALENDAR_PNG, img::CALENDAR_PNG);
        m_nav_tree_ctrl->SetItemData(bYear, new mmTreeItemData(id, true));

        // Only add YEARS for Budget Performance
        if (name.length() < 5)
        {
            wxTreeItemId bYearData = m_nav_tree_ctrl->AppendItem(budgetPerformance
                , name, img::PIECHART_PNG, img::PIECHART_PNG);
            m_nav_tree_ctrl->SetItemData(bYearData, new mmTreeItemData(id, true));
        }
        wxTreeItemId bYearSetupData = m_nav_tree_ctrl->AppendItem(budgetSetupPerformance
            , name, img::PIECHART_PNG, img::PIECHART_PNG);
        m_nav_tree_ctrl->SetItemData(bYearSetupData, new mmTreeItemData(id, true));
        ++i;
    }

    ///////////////////////////////////////////////////////////////////
    wxTreeItemId cashFlow = m_nav_tree_ctrl->AppendItem(reports
        , _("Cash Flow"), img::PIECHART_PNG, img::PIECHART_PNG);
    m_nav_tree_ctrl->SetItemData(cashFlow, new mmTreeItemData("Cash Flow", new mmReportCashFlowAllAccounts()));

    wxTreeItemId cashflowWithBankAccounts = m_nav_tree_ctrl->AppendItem(cashFlow
        , _("Cash Flow - With Bank Accounts"), img::PIECHART_PNG, img::PIECHART_PNG);
    m_nav_tree_ctrl->SetItemData(cashflowWithBankAccounts, new mmTreeItemData("Cash Flow - With Bank Accounts"
        , new mmReportCashFlowBankAccounts()));

    wxTreeItemId cashflowWithTermAccounts = m_nav_tree_ctrl->AppendItem(cashFlow
        , _("Cash Flow - With Term Accounts"), img::PIECHART_PNG, img::PIECHART_PNG);
    m_nav_tree_ctrl->SetItemData(cashflowWithTermAccounts, new mmTreeItemData("Cash Flow - With Term Accounts"
        , new mmReportCashFlowTermAccounts()));

    wxTreeItemId cashflowSpecificAccounts = m_nav_tree_ctrl->AppendItem(cashFlow
        , _("Cash Flow - Specific Accounts"), img::PIECHART_PNG, img::PIECHART_PNG);
    m_nav_tree_ctrl->SetItemData(cashflowSpecificAccounts, new mmTreeItemData("Cash Flow - Specific Accounts"
        , new mmReportCashFlowSpecificAccounts()));

    wxTreeItemId cashflowSpecificAccountsDaily = m_nav_tree_ctrl->AppendItem(cashFlow
        , _("Daily Cash Flow - Specific Accounts"), img::PIECHART_PNG, img::PIECHART_PNG);
    m_nav_tree_ctrl->SetItemData(cashflowSpecificAccountsDaily, new mmTreeItemData("Daily Cash Flow - Specific Accounts"
        , new mmReportDailyCashFlowSpecificAccounts()));

    ///////////////////////////////////////////////////////////////////

    wxTreeItemId transactionList = m_nav_tree_ctrl->AppendItem(reports
        , _("Transaction Report"), img::FILTER_PNG, img::FILTER_PNG);
    m_nav_tree_ctrl->SetItemData(transactionList, new mmTreeItemData("Transaction Report"));

    //////////////////////////////////////////////////////////////////

    wxTreeItemId stocksReport = m_nav_tree_ctrl->AppendItem(reports
        , _("Stocks Report"), img::PIECHART_PNG, img::PIECHART_PNG);
    m_nav_tree_ctrl->SetItemData(stocksReport, new mmTreeItemData("Stocks Report",
        new mmReportChartStocks()));

    wxTreeItemId stocksReportSummary = m_nav_tree_ctrl->AppendItem(stocksReport
        , _("Summary"), img::PIECHART_PNG, img::PIECHART_PNG);
    m_nav_tree_ctrl->SetItemData(stocksReportSummary, new mmTreeItemData("Summary of Stocks"
        , new mmReportSummaryStocks()));

    //////////////////////////////////////////////////////////////////

    wxTreeItemId forecastReport = m_nav_tree_ctrl->AppendItem(reports
        , _("Forecast Report"), img::PIECHART_PNG, img::PIECHART_PNG);
    m_nav_tree_ctrl->SetItemData(forecastReport, new mmTreeItemData("Forecast Report"
        , new mmReportForecast()));

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
            group = m_nav_tree_ctrl->AppendItem(reports
                , wxGetTranslation(record.GROUPNAME), img::CUSTOMSQL_GRP_PNG, img::CUSTOMSQL_GRP_PNG);
            m_nav_tree_ctrl->SetItemData(group, new mmTreeItemData(record.GROUPNAME
                , new mmGeneralGroupReport(record.GROUPNAME)));
            group_name = record.GROUPNAME;
        }
        Model_Report::Data* r = Model_Report::instance().get(record.REPORTID);
        wxTreeItemId item = m_nav_tree_ctrl->AppendItem(no_group ? reports : group
            , wxGetTranslation(record.REPORTNAME), img::CUSTOMSQL_PNG, img::CUSTOMSQL_PNG);
        m_nav_tree_ctrl->SetItemData(item, new mmTreeItemData(r->REPORTNAME, new mmGeneralReport(r)));
    }
}

