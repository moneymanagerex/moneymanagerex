/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2012 Stefano Giorgio
 Copyright (C) 2013, 2015 Nikolay Akimov
 Copyright (C) 2014, 2017 James Higley
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
#include "reports/reportbase.h"
#include "Model_Budgetyear.h"
#include "Model_Report.h"

const char *group_report_template = R"(
<!DOCTYPE html>
<html>
<head>
    <meta http-equiv="content-type" content="text/html; charset=utf-8" />
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

        return out;
    }
private:
    wxString m_group_name;
    Model_Report::Data_Set m_sub_reports;
};

void mmGUIFrame::updateReportNavigation(wxTreeItemId& reports, bool budget)
{
    ///////////////////////////////////////////////////////////////////

    wxTreeItemId transactionList = m_nav_tree_ctrl->AppendItem(reports
        , _("Transaction Report"), img::FILTER_PNG, img::FILTER_PNG);
    m_nav_tree_ctrl->SetItemData(transactionList, new mmTreeItemData("Transaction Report"));

    //////////////////////////////////////////////////////////////////

    wxTreeItemId reportGroup;
    wxString reportGroupName;
    for (int r = 0; r < Option::instance().ReportCount(); r++)
    {
        const wxString& groupName = Option::instance().ReportGroup(r);
        bool no_group = groupName.IsEmpty();
        if (reportGroupName != groupName && !no_group)
        {
            bool bAdd = false;
            for (int s = 0; (s < Option::instance().ReportCount()) && !bAdd; s++)
            {
                if (Option::instance().ReportGroup(s) == groupName)
                {
                    bool a = !Option::instance().HideReport(s);
                    if (a && Option::instance().BudgetReport(s))
                        a = budget;
                    if (a)
                        bAdd = true;
                }
            }
            if (bAdd)
            {
                reportGroup = m_nav_tree_ctrl->AppendItem(reports
                    , wxGetTranslation(groupName), img::PIECHART_PNG, img::PIECHART_PNG);
                m_nav_tree_ctrl->SetItemData(reportGroup, new mmTreeItemData(groupName));
                reportGroupName = groupName;
            }
        }
        bool bShow = !Option::instance().HideReport(r);
        if (bShow && Option::instance().BudgetReport(r))
            bShow = budget;
        
        if (bShow)
        {
            const auto& reportName = Option::instance().ReportName(r);
            wxTreeItemId item = m_nav_tree_ctrl->AppendItem(no_group ? reports : reportGroup
                , wxGetTranslation(reportName), img::PIECHART_PNG, img::PIECHART_PNG);
            m_nav_tree_ctrl->SetItemData(item
                , new mmTreeItemData(reportName, Option::instance().ReportFunction(r)));
        }
    }

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
