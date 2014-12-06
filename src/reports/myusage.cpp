/*******************************************************
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

#include "myusage.h"
#include "model/Model_Usage.h"
#include "reports/mmDateRange.h"
#include "reports/htmlbuilder.h"
#include "model/Model_Report.h"

const char *usage_template = R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8" />
    <meta http - equiv = "Content-Type" content = "text/html" />
    <title><TMPL_VAR REPORTNAME></title>
    <script src = "Chart.js"></script>
    <script src = "sorttable.js"></script>
    <link href = "master.css" rel = "stylesheet" />
    <style>
        canvas {max-height: 400px; min-height: 100px;}
    </style>
</head>
<body>

<div class = "container">
<h3><TMPL_VAR REPORTNAME></h3>
<TMPL_VAR TODAY><hr>

<div class = "row">
<div class = "col-xs-1"></div>
<div class = "col-xs-10">

<canvas id="mycanvas" height="200" width="600"></canvas>
<script>
    var LineData = {
    labels: [
            <TMPL_LOOP NAME=CONTENTS>
                <TMPL_IF NAME=__LAST__>
                    "<TMPL_VAR USAGEDATE>"
                <TMPL_ELSE>
                    "<TMPL_VAR USAGEDATE>",
                </TMPL_IF>
            </TMPL_LOOP>
            ],
    datasets: [
        {
            fillColor : 'rgba(129, 172, 123, 0.5)',
            strokeColor : 'rgba(129, 172, 123, 1)',
            pointColor : 'rgba(129, 172, 123, 1)', 
            pointStrokeColor : "#fff",
            data : [
                    <TMPL_LOOP NAME=CONTENTS>
                        <TMPL_IF NAME=__LAST__>
                            <TMPL_VAR FREQUENCY>
                        <TMPL_ELSE>
                            <TMPL_VAR FREQUENCY>,
                        </TMPL_IF>
                    </TMPL_LOOP>
                    ],
            title : "FREQUENCY"
        }
        ]
    }
    var opts= { annotateDisplay : true, responsive : true };

    window.onload = function() {
        var myBar = new Chart(document.getElementById("mycanvas").getContext("2d")).Line(LineData,opts);
    }
</script>
<div class = "col-xs-4"></div>
<div class = "col-xs-4">
<table class = "table">
    <thead>
        <tr>
            <th>USAGEDATE</th>
            <th>FREQUENCY</th>
        </tr>
    </thead>
    <tbody>
        <TMPL_LOOP NAME=CONTENTS>
            <tr>
            <td><TMPL_VAR "USAGEDATE"></td>
            <td><TMPL_VAR "FREQUENCY"></td>
            </tr>
        </TMPL_LOOP>
        <tr>
            <td>Grand Total</td>
            <td><TMPL_VAR "GRAND"></td>
        </tr>
    </tbody>
</table>
</div></div></div></div></body>
</html>
)";

mmReportMyUsage::mmReportMyUsage(mmDateRange* date_range)
: mmPrintableBase("My Usage", _("My Usage"))
{
    this->m_date_range = date_range;
}

mmReportMyUsage::~mmReportMyUsage()
{}

bool mmReportMyUsage::has_date_range()
{
    return true;
}

wxString mmReportMyUsage::getHTMLText()
{
    Model_Usage::Data_Set all_usage;

    if (m_date_range && m_date_range->is_with_date())
        all_usage = Model_Usage::instance().find(Model_Usage::USAGEDATE(m_date_range->start_date().FormatISODate(), GREATER_OR_EQUAL)
                    , Model_Usage::USAGEDATE(m_date_range->end_date().FormatISODate(), LESS_OR_EQUAL));
    else
        all_usage = Model_Usage::instance().all();
    std::map<wxString, int> usage_by_day;

    for (const auto & usage : all_usage)
    {
        usage_by_day[usage.USAGEDATE] += 1;
    }

    loop_t contents;
    for (auto it = usage_by_day.begin(); it != usage_by_day.end(); ++ it)
    {
        row_t r;
        r(L"USAGEDATE") = it->first;
        r(L"FREQUENCY") = wxString::Format("%d", it->second);

        contents += r;
    }

    mm_html_template report(usage_template);
    report(L"REPORTNAME") = this->local_title();
    report(L"CONTENTS") = contents;
    report(L"GRAND") = wxString::Format("%ld", (long)all_usage.size());

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
