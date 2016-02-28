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
#include "cajun/json/elements.h"
#include "cajun/json/reader.h"

const char *usage_template = R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8" />
    <meta http - equiv = "Content-Type" content = "text/html" />
    <title><TMPL_VAR REPORTNAME></title>
    <script src = "ChartNew.js"></script>
    <script src = "sorttable.js"></script>
    <link href = "master.css" rel = "stylesheet" />
    <style>
        canvas {max-height: 400px; min-height: 100px;}
        body {font-size: <TMPL_VAR HTMLSCALE>%;};
    </style>
</head>
<body>

<div class = "container">
<h3><TMPL_VAR REPORTNAME>

<select id="chart-type" onchange='onChartChange(this)'>
    <option value="line" selected>Line Chart</option>
    <option value="bar">Bar Chart</option>
</select>
</h3>
<TMPL_VAR TODAY><hr>

<div class = "row">
<div class = "col-xs-1"></div>
<div class = "col-xs-10">

<canvas id="mycanvas" height="200" width="600"></canvas>
<script>
    var data = {
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
        var myBar = new Chart(document.getElementById("mycanvas").getContext("2d")).Line(data,opts);
    }

    function onChartChange(select){
        var value = select.value;
        if (value == "line") {
           new Chart(document.getElementById("mycanvas").getContext("2d")).Line(data,opts); 
        }
        else if (value == "bar") {
           new Chart(document.getElementById("mycanvas").getContext("2d")).Bar(data,opts);
        }
    }

</script>
</div></div></div></body>
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
    std::map<wxString, std::pair<int, wxString> > usage_by_day;

    for (const auto & usage : all_usage)
    {
        usage_by_day[usage.USAGEDATE].first += 1;

        std::wstringstream ss ;
        ss << usage.JSONCONTENT.ToStdWstring();
        json::Object o;
        json::Reader::Read(o, ss);
        if (o.Find(L"usage") == o.End()) continue;
        const json::Array& u = o[L"usage"];

        for (json::Array::const_iterator it = u.Begin(); it != u.End(); ++it)
        {
            const json::Object& pobj = *it;

            if (pobj.Find(L"module") == pobj.End()) continue;
            if (pobj.Find(L"start") == pobj.End()) continue;
            if (pobj.Find(L"end") == pobj.End()) continue;

            wxString module = (wxString)((json::String)pobj[L"module"]).Value();
            if (pobj.Find(L"name") != pobj.End())
                module += (wxString)((json::String)pobj[L"name"]).Value();
        
            wxDateTime start, end;
            start.ParseISOCombined((wxString)((json::String)pobj[L"start"]).Value());
            end.ParseISOCombined((wxString)((json::String)pobj[L"end"]).Value());

            long delta = end.Subtract(start).GetSeconds().ToLong();
            if (delta < 1)
                continue;

            usage_by_day[usage.USAGEDATE].second += module + ";";
        }
    }

    loop_t contents;
    for (auto it = usage_by_day.begin(); it != usage_by_day.end(); ++ it)
    {
        row_t r;
        r(L"USAGEDATE") = it->first;
        r(L"FREQUENCY") = wxString::Format("%d", it->second.first);
        r(L"SLOW") = it->second.second;

        contents += r;
    }

    mm_html_template report(usage_template);
    report(L"REPORTNAME") = this->local_title();
    report(L"CONTENTS") = contents;
    report(L"GRAND") = wxString::Format("%ld", (long)all_usage.size());
    report(L"HTMLSCALE") = wxString::Format("%d", mmIniOptions::instance().html_font_size_);

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
