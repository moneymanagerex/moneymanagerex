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
#include "option.h"
#include "reports/mmDateRange.h"
#include "reports/htmlbuilder.h"
#include "Model_Usage.h"
#include "Model_Report.h"

const char *usage_template = R"(
<!DOCTYPE html>
<html>
<head>
    <meta http-equiv="content-type" content="text/html; charset=utf-8" />
    <title><TMPL_VAR REPORTNAME></title>
    <script src = "ChartNew.js"></script>
    <script src = "format.js"></script>
    <script src = "sorttable.js"></script>
    <link href = "master.css" rel = "stylesheet" />
    <style>
        canvas {min-height: 100px}
        body {font-size: <TMPL_VAR HTMLSCALE>%}
    </style>
</head>
<body>

<div class = "container">
<h3><TMPL_VAR REPORTNAME>

<!--
<select id="chart-type" onchange='onChartChange(this)'>
    <option value="line" selected><TMPL_VAR _LINECHART></option>
    <option value="bar"><TMPL_VAR _BARCHART></option>
</select>
-->
</h3>
<TMPL_VAR TODAY><hr>

<div class = "row">
<canvas id="mycanvas"></canvas>
<script>
    var data = {
    labels: [ <TMPL_VAR STARTDATE>, <TMPL_VAR ENDDATE> ],
    xBegin: <TMPL_VAR STARTDATE>,
    xEnd: <TMPL_VAR ENDDATE>,
    datasets: [
        {
            // fillColor : 'rgba(129, 172, 123, 0.5)',
            strokeColor : 'rgba(129, 172, 123, 1)',
            // pointColor : 'rgba(129, 172, 123, 1)', 
            // pointStrokeColor : "#fff",
            data : [ <TMPL_LOOP NAME=CONTENTS><TMPL_VAR FREQUENCY><TMPL_UNLESS NAME=__LAST__>,</TMPL_UNLESS></TMPL_LOOP> ],
            xPos: [ <TMPL_LOOP NAME=CONTENTS><TMPL_VAR USAGEDATE><TMPL_UNLESS NAME=__LAST__>,</TMPL_UNLESS></TMPL_LOOP> ],
            title : "<TMPL_VAR _FREQUENCY>"
        }
        ]
    }
    var opts= {
        annotateDisplay: true,
        responsive: true,
        yAxisMinimumInterval: 1,
        extrapolateMissingData: false,
        datasetFill: false,
        pointDot : false,
        linkType: 1,
        fmtV2: "date",
        fmtXLabel: "date"
    };

    var ctx = document.getElementById("mycanvas").getContext("2d");

    window.onload = function() {
        var myBar = new Chart(ctx).Line(data,opts);
    }
/*
    function onChartChange(select){
        var value = select.value;
        if (value == "line") {
           new Chart(ctx).Line(data,opts); 
        }
        else if (value == "bar") {
           new Chart(ctx).Bar(data,opts);
        }
    }
*/
</script>
</div></div></body>
</html>
)";

mmReportMyUsage::mmReportMyUsage()
: mmPrintableBase(_("MMEX Usage Frequency"))
{
}

mmReportMyUsage::~mmReportMyUsage()
{
}

int mmReportMyUsage::report_parameters()
{
    return RepParams::DATE_RANGE;
}

wxString mmReportMyUsage::getHTMLText()
{
    Model_Usage::Data_Set all_usage;
    wxDateTime _start_date, _end_date;

    if (m_date_range && m_date_range->is_with_date()) {
        all_usage = Model_Usage::instance().find(Model_Usage::USAGEDATE(m_date_range->start_date().FormatISODate(), GREATER_OR_EQUAL)
            , Model_Usage::USAGEDATE(m_date_range->end_date().FormatISODate(), LESS_OR_EQUAL));
        _start_date=m_date_range->start_date();
        _end_date=m_date_range->end_date();
    }
    else {
        all_usage = Model_Usage::instance().all();
        wxASSERT(_start_date.ParseISODate(all_usage.front().USAGEDATE));
        wxASSERT(_end_date.ParseISODate(all_usage.back().USAGEDATE));
    }
    std::map<wxString, std::pair<int, wxString> > usage_by_day;

    for (const auto & usage : all_usage)
    {
        usage_by_day[usage.USAGEDATE].first += 1;

        Document json_doc;
        if (json_doc.Parse(usage.JSONCONTENT.c_str()).HasParseError()) {
            continue;
        }

        // wxLogDebug("======= mmReportMyUsage::getHTMLText =======");
        // wxLogDebug("RapidJson\n%s", JSON_PrettyFormated(json_doc));

        if (!json_doc.HasMember("usage") || !json_doc["usage"].IsArray())
            continue;
        Value u = json_doc["usage"].GetArray();

        for (Value::ConstValueIterator it = u.Begin(); it != u.End(); ++it)
        {
            if (!it->IsObject()) continue;
            const auto pobj = it->GetObject();

            if (!pobj.HasMember("module") || !pobj["module"].IsString())
                continue;
            auto module = wxString::FromUTF8(pobj["symbol"].GetString());

            if (!pobj.HasMember("module") || !pobj["module"].IsString())
                continue;
            module += wxString::FromUTF8(pobj["name"].GetString());

            if (!pobj.HasMember("start") || !pobj["start"].IsString())
                continue;
            const auto s = wxString::FromUTF8(pobj["start"].GetString());
            
            if (!pobj.HasMember("end") || !pobj["end"].IsString())
                continue;
            const auto e = wxString::FromUTF8(pobj["end"].GetString());

            wxDateTime start, end;
            start.ParseISOCombined(s);
            end.ParseISOCombined(e);

            long delta = end.Subtract(start).GetSeconds().ToLong();
            if (delta < 1)
                continue;

            // usage_by_day[usage.USAGEDATE].second += module + ";";
        }
    }

    if (usage_by_day.empty()) {
        usage_by_day[wxDateTime::Today().FormatISODate()] = std::make_pair(0, "");
    }

    loop_t contents;
    wxDateTime day;
    for (auto it = usage_by_day.begin(); it != usage_by_day.end(); ++ it)
    {
        row_t r;
        wxASSERT(day.ParseISODate(it->first));
        r(L"USAGEDATE") = wxString::Format("new Date(%d,%d,%d,0,0,0)",
                             day.GetYear(), day.GetMonth(), day.GetDay());
        r(L"FREQUENCY") = wxString::Format("%d", it->second.first);
        // r(L"SLOW") = it->second.second;

        contents += r;
    }

    mm_html_template report(usage_template);
    report(L"REPORTNAME") = this->title();
    // report(L"_LINECHART") = _("Line Chart");
    // report(L"_BARCHART") = _("Bar Chart");
    report(L"_FREQUENCY") = _("Frequency");
    report(L"STARTDATE") = wxString::Format("new Date(%d,%d,%d,0,0,0)",
                            _start_date.GetYear(),
                            _start_date.GetMonth(),
                            _start_date.GetDay());
    report(L"ENDDATE") = wxString::Format("new Date(%d,%d,%d,0,0,0)",
                            _end_date.GetYear(),
                            _end_date.GetMonth(),
                            _end_date.GetDay());
    report(L"CONTENTS") = contents;
    report(L"GRAND") = wxString::Format("%ld", (long)all_usage.size());
    report(L"HTMLSCALE") = wxString::Format("%d", Option::instance().HtmlFontSize());

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
