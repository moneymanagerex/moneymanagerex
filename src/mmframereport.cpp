/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2012 Stefano Giorgio
 Copyright (C) 2013 - 2022 Nikolay Akimov
 Copyright (C) 2014 James Higley
 Copyright (C) 2014 Guan Lisheng (guanlisheng@gmail.com)
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
    <script>
        window.Promise || document.write('<script src="memory:polyfill.min.js"><\/script>');
        window.Promise || document.write('<script src="memory:classlist.min.js"><\/script>');
        window.Promise || document.write('<script src="memory:resize-observer.js"><\/script>');
        window.Promise || document.write('<script src="memory:findindex.min.js"><\/script>');
        window.Promise || document.write('<script src="memory:umd.min.js"><\/script>');
    </script>
    <script src = "memory:apexcharts.min.js"></script>
    <script src = "memory:sorttable.js"></script>
    <link href = "memory:master.css" rel = "stylesheet" />
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
    mmGeneralGroupReport(const wxString& groupname) : mmPrintableBase(wxTRANSLATE("General Group Report"))
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
        report(L"REPORTNAME") = this->getReportTitle() + " For " + this->m_group_name;
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

        return out;
    }
private:
    wxString m_group_name;
    Model_Report::Data_Set m_sub_reports;
};

void mmGUIFrame::DoUpdateReportNavigation(wxTreeItemId& parent_item)
{
    wxArrayString hidden_reports = Model_Infotable::instance().GetArrayStringSetting("HIDDEN_REPORTS");

    if (hidden_reports.Index("Cash Flow") == wxNOT_FOUND)
    {
        wxTreeItemId cashFlow = m_nav_tree_ctrl->AppendItem(parent_item, _("Cash Flow"), img::PIECHART_PNG, img::PIECHART_PNG);
        m_nav_tree_ctrl->SetItemData(cashFlow, new mmTreeItemData(mmTreeItemData::MENU_REPORT, "Cash Flow"));

        wxTreeItemId cashflowWithBankAccounts = m_nav_tree_ctrl->AppendItem(cashFlow, _("Daily"), img::PIECHART_PNG, img::PIECHART_PNG);
        m_nav_tree_ctrl->SetItemData(cashflowWithBankAccounts, new mmTreeItemData("Cash Flow - Daily", new mmReportCashFlowDaily()));

        wxTreeItemId cashflowWithTermAccounts = m_nav_tree_ctrl->AppendItem(cashFlow, _("Monthly"), img::PIECHART_PNG, img::PIECHART_PNG);
        m_nav_tree_ctrl->SetItemData(cashflowWithTermAccounts, new mmTreeItemData("Cash Flow - Monthly", new mmReportCashFlowMonthly()));

        wxTreeItemId cashflowWithTransactions = m_nav_tree_ctrl->AppendItem(cashFlow, _("Transactions"), img::PIECHART_PNG, img::PIECHART_PNG);
        m_nav_tree_ctrl->SetItemData(cashflowWithTransactions, new mmTreeItemData("Cash Flow - Transactions", new mmReportCashFlowTransactions()));
    }

    ///////////////////////////////////////////////////////////////////

    if (hidden_reports.Index("Categories") == wxNOT_FOUND)
    {
        wxTreeItemId categs = m_nav_tree_ctrl->AppendItem(parent_item, _("Categories"), img::PIECHART_PNG, img::PIECHART_PNG);
        m_nav_tree_ctrl->SetItemData(categs, new mmTreeItemData(mmTreeItemData::MENU_REPORT, "Categories"));

        wxTreeItemId categsMonthly = m_nav_tree_ctrl->AppendItem(categs, _("Monthly"), img::PIECHART_PNG, img::PIECHART_PNG);
        m_nav_tree_ctrl->SetItemData(categsMonthly, new mmTreeItemData("Categories Monthly", new mmReportCategoryOverTimePerformance()));

        wxTreeItemId categsSummary = m_nav_tree_ctrl->AppendItem(categs, _("Summary"), img::PIECHART_PNG, img::PIECHART_PNG);
        m_nav_tree_ctrl->SetItemData(categsSummary, new mmTreeItemData("Categories Summary", new  mmReportCategoryExpensesCategories()));

        wxTreeItemId categsGoes = m_nav_tree_ctrl->AppendItem(categs, _("Where the Money Goes"), img::PIECHART_PNG, img::PIECHART_PNG);
        m_nav_tree_ctrl->SetItemData(categsGoes, new mmTreeItemData("Where the Money Goes", new mmReportCategoryExpensesGoes()));

        wxTreeItemId categsComes = m_nav_tree_ctrl->AppendItem(categs, _("Where the Money Comes From"), img::PIECHART_PNG, img::PIECHART_PNG);
        m_nav_tree_ctrl->SetItemData(categsComes, new mmTreeItemData("Where the Money Comes From", new mmReportCategoryExpensesComes()));
    }

    //////////////////////////////////////////////////////////////////

    if (hidden_reports.Index("Forecast Report") == wxNOT_FOUND)
    {
        wxTreeItemId forecastReport = m_nav_tree_ctrl->AppendItem(parent_item, _("Forecast Report"), img::PIECHART_PNG, img::PIECHART_PNG);
        m_nav_tree_ctrl->SetItemData(forecastReport, new mmTreeItemData("Forecast Report", new mmReportForecast()));
    }

    ///////////////////////////////////////////////////////////////////

    if (hidden_reports.Index("Income vs Expenses") == wxNOT_FOUND)
    {
        wxTreeItemId incexpOverTime = m_nav_tree_ctrl->AppendItem(parent_item, _("Income vs Expenses"), img::PIECHART_PNG, img::PIECHART_PNG);
        m_nav_tree_ctrl->SetItemData(incexpOverTime, new mmTreeItemData("Income vs Expenses", new mmReportIncomeExpenses()));

        wxTreeItemId incexpMonthly = m_nav_tree_ctrl->AppendItem(incexpOverTime, _("Monthly"), img::PIECHART_PNG, img::PIECHART_PNG);
        m_nav_tree_ctrl->SetItemData(incexpMonthly, new mmTreeItemData("Income vs Expenses - Monthly", new mmReportIncomeExpensesMonthly()));
    }

    ///////////////////////////////////////////////////////////////////

    if (hidden_reports.Index("My Usage") == wxNOT_FOUND)
    {
        wxTreeItemId myusage = m_nav_tree_ctrl->AppendItem(parent_item, _("My Usage"), img::PIECHART_PNG, img::PIECHART_PNG);
        m_nav_tree_ctrl->SetItemData(myusage, new mmTreeItemData("My Usage", new mmReportMyUsage()));
    }

    //////////////////////////////////////////////////////////////////////

    if (hidden_reports.Index("Payees") == wxNOT_FOUND)
    {
        wxTreeItemId payeesOverTime = m_nav_tree_ctrl->AppendItem(parent_item, _("Payees"), img::PIECHART_PNG, img::PIECHART_PNG);
        m_nav_tree_ctrl->SetItemData(payeesOverTime, new mmTreeItemData("Payee Report", new mmReportPayeeExpenses()));
    }

    //////////////////////////////////////////////////////////////////

    if (hidden_reports.Index("Summary of Accounts") == wxNOT_FOUND)
    {
        wxTreeItemId reportsSummary = m_nav_tree_ctrl->AppendItem(parent_item, _("Summary of Accounts"), img::PIECHART_PNG, img::PIECHART_PNG);
        m_nav_tree_ctrl->SetItemData(reportsSummary, new mmTreeItemData(mmTreeItemData::MENU_REPORT, "Summary of Accounts"));

        wxTreeItemId accMonthly = m_nav_tree_ctrl->AppendItem(reportsSummary, _("Monthly"), img::PIECHART_PNG, img::PIECHART_PNG);
        m_nav_tree_ctrl->SetItemData(accMonthly, new mmTreeItemData("Monthly Summary of Accounts", new mmReportSummaryByDateMontly()));

        wxTreeItemId accYearly = m_nav_tree_ctrl->AppendItem(reportsSummary, _("Yearly"), img::PIECHART_PNG, img::PIECHART_PNG);
        m_nav_tree_ctrl->SetItemData(accYearly, new mmTreeItemData("Yearly Summary of Accounts", new mmReportSummaryByDateYearly()));
    }

    //////////////////////////////////////////////////////////////////

    size_t i = Model_Budgetyear::instance().all().size();
    if (i > 0)
    {
        if (hidden_reports.Index("Budget Performance") == wxNOT_FOUND)
        {
            wxTreeItemId budgetPerformance = m_nav_tree_ctrl->AppendItem(parent_item, _("Budget Performance"), img::PIECHART_PNG, img::PIECHART_PNG);
            m_nav_tree_ctrl->SetItemData(budgetPerformance, new mmTreeItemData("Budget Performance", new mmReportBudgetingPerformance()));
        }

        if (hidden_reports.Index("Budget Category Summary") == wxNOT_FOUND)
        {
            wxTreeItemId budgetSetupPerformance = m_nav_tree_ctrl->AppendItem(parent_item, _("Budget Category Summary"), img::PIECHART_PNG, img::PIECHART_PNG);
            m_nav_tree_ctrl->SetItemData(budgetSetupPerformance, new mmTreeItemData("Budget Category Summary", new mmReportBudgetCategorySummary()));
        }
    }

    ///////////////////////////////////////////////////////////////////

    Model_Account::Data_Set investments_account = Model_Account::instance().find(Model_Account::ACCOUNTTYPE(Model_Account::all_type()[Model_Account::INVESTMENT], EQUAL));
    if (!investments_account.empty())
    {
        if (hidden_reports.Index("Stocks Report") == wxNOT_FOUND)
        {
            wxTreeItemId stocksReport = m_nav_tree_ctrl->AppendItem(parent_item, _("Stocks Report"), img::PIECHART_PNG, img::PIECHART_PNG);
            m_nav_tree_ctrl->SetItemData(stocksReport, new mmTreeItemData("Stocks Report", new mmReportChartStocks()));

            wxTreeItemId stocksReportSummary = m_nav_tree_ctrl->AppendItem(stocksReport, _("Summary"), img::PIECHART_PNG, img::PIECHART_PNG);
            m_nav_tree_ctrl->SetItemData(stocksReportSummary, new mmTreeItemData("Summary of Stocks", new mmReportSummaryStocks()));
        }
    }

}

void mmGUIFrame::DoUpdateGRMNavigation(wxTreeItemId& parent_item)
{
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
            group = m_nav_tree_ctrl->AppendItem(parent_item, wxGetTranslation(record.GROUPNAME), img::CUSTOMSQL_GRP_PNG, img::CUSTOMSQL_GRP_PNG);
            m_nav_tree_ctrl->SetItemData(group, new mmTreeItemData(new mmGeneralGroupReport(record.GROUPNAME), record.GROUPNAME));
            group_name = record.GROUPNAME;
        }
        Model_Report::Data* r = Model_Report::instance().get(record.REPORTID);
        wxTreeItemId item = m_nav_tree_ctrl->AppendItem(no_group ? parent_item : group, wxGetTranslation(record.REPORTNAME), img::CUSTOMSQL_PNG, img::CUSTOMSQL_PNG);
        m_nav_tree_ctrl->SetItemData(item, new mmTreeItemData(new mmGeneralReport(r), r->REPORTNAME));
    }

}

void mmGUIFrame::DoUpdateFilterNavigation(wxTreeItemId& parent_item)
{

    wxArrayString filter_settings = Model_Infotable::instance().GetArrayStringSetting("TRANSACTIONS_FILTER", true);
    for (const auto& data : filter_settings)
    {
        Document j_doc;
        if (j_doc.Parse(data.utf8_str()).HasParseError()) {
            j_doc.Parse("{}");
        }

        Value& j_label = GetValueByPointerWithDefault(j_doc, "/LABEL", "");
        const wxString& s_label = j_label.IsString() ? wxString::FromUTF8(j_label.GetString()) : "";

        wxTreeItemId item = m_nav_tree_ctrl->AppendItem(parent_item, s_label, img::FILTER_PNG, img::FILTER_PNG);
        m_nav_tree_ctrl->SetItemData(item, new mmTreeItemData(mmTreeItemData::FILTER_REPORT, data));
    }

}

void mmGUIFrame::mmDoHideReportsDialog()
{
    wxString rep[] =
    {
        "Cash Flow",
        "Categories",
        "Forecast Report",
        "Income vs Expenses",
        "My Usage",
        "Payees",
        "Summary of Accounts",
        "Budget Performance",
        "Budget Category Summary",
        "Stocks Report",
    };

    wxArrayString stored_items = Model_Infotable::instance().GetArrayStringSetting("HIDDEN_REPORTS");
    wxArrayInt hidden_reports;
    wxArrayString reports_name;
    wxArrayString reports_name_i10n;

    for (const auto& r : rep) {
        reports_name_i10n.Add(wxGetTranslation(r));
        reports_name.Add(r);
        if (stored_items.Index(r) != wxNOT_FOUND) {
            hidden_reports.Add(reports_name.Index(r));
        }
    }

    mmMultiChoiceDialog reports(this, _("Hide"), _("Reports"), reports_name_i10n);
    reports.SetSelections(hidden_reports);

    if (reports.ShowModal() == wxID_OK)
    {
        Model_Infotable::instance().Set("HIDDEN_REPORTS", "[]");
        const auto sel = reports.GetSelections();
        for (const auto& i : sel)
        {
            const auto& report_name = reports_name[i];
            Model_Infotable::instance().Prepend("HIDDEN_REPORTS", report_name, -1);
        }
    }
    DoRecreateNavTreeControl();
}
