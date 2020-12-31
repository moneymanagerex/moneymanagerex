/*******************************************************
 Copyright (C) 2016 Guan Lisheng (guanlisheng@gmail.com)

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

#include "forecast.h"
#include "mmex.h"
#include "mmframe.h"
#include "util.h"
#include "reports/htmlbuilder.h"
#include "model/Model_Checking.h"

class mm_html_template;

mmReportForecast::mmReportForecast(): mmPrintableBase(wxTRANSLATE("Forecast"))
{
    setReportParameters(Reports::ForecastReport);
}

mmReportForecast::~mmReportForecast()
{
}

wxString mmReportForecast::getHTMLText()
{
    // Grab the data
    std::map<wxString, std::pair<double, double> > amount_by_day;
    Model_Checking::Data_Set all_trans;
    
    if (m_date_range && m_date_range->is_with_date()) {
        all_trans = Model_Checking::instance().find(DB_Table_CHECKINGACCOUNT_V1::TRANSDATE(m_date_range->start_date().FormatISODate(), GREATER_OR_EQUAL)
            , DB_Table_CHECKINGACCOUNT_V1::TRANSDATE(m_date_range->end_date().FormatISODate(), LESS_OR_EQUAL));
    }
    else {
        all_trans = Model_Checking::instance().all();
    }

    for (const auto & trx : all_trans)
    {
        if (Model_Checking::type(trx) == Model_Checking::TRANSFER || Model_Checking::foreignTransactionAsTransfer(trx))
            continue;

        amount_by_day[trx.TRANSDATE].first += Model_Checking::withdrawal(trx, -1);
        amount_by_day[trx.TRANSDATE].second += Model_Checking::deposit(trx, -1);
    }

    

    // Build the report
    mmHTMLBuilder hb;
    hb.init();
    hb.addDivContainer();
    {
        hb.addHeader(2, getReportTitle());
        hb.DisplayDateHeading(m_date_range->start_date(), m_date_range->end_date(), m_date_range->is_with_date());
        hb.addDateNow();
        hb.addLineBreak();
        hb.addDivRow();
        {
            GraphData gd;
            GraphSeries gsWithdrawl, gsDeposit;
            for (const auto & kv : amount_by_day)
            {
                wxDate d;
                wxLogDebug("kv.first  = %s", kv.first); 
                d.ParseISODate(kv.first);
                wxString label = wxString::Format("%i %s %i", d.GetDay(), wxGetTranslation(wxDateTime::GetEnglishMonthName(d.GetMonth())), d.GetYear());
                wxLogDebug("label  = %s", label); 
                gd.labels.push_back(label);
                //wxLogDebug(" Values = %d, %d", kv.second.first, kv.second.second);
                gsWithdrawl.values.push_back(kv.second.first);
                gsDeposit.values.push_back(kv.second.second);
            }
            gsWithdrawl.name = _("Withdrawl");
            gd.series.push_back(gsWithdrawl);           
            gsDeposit.name = _("Deposit");
            gd.series.push_back(gsDeposit);

            hb.addDivContainer();
            { 
                gd.type = GraphData::LINE_DATETIME;
                hb.addChart(gd);
            }
            hb.endDiv();
        }
        hb.endDiv();
    }
    hb.endDiv();
    hb.end();

    return hb.getHTMLText();
}
  /*  // Add the chart


    //mm_html_template report(this->m_template);
    //report(L"REPORTNAME") = this->getReportTitle();
    //report(L"CONTENTS") = contents;
    wxDateTime today = wxDateTime::Now();
    const wxString current_day_time = wxString::Format(_("Report Generated %s %s")
        , mmGetDateForDisplay(today.FormatISODate())
        , today.FormatISOTime());
    report(L"TODAY") = current_day_time;
    report(L"GRAND") = wxString::Format("%ld", static_cast<long>(amount_by_day.size()));
    report(L"HTMLSCALE") = wxString::Format("%d", Option::instance().getHtmlFontSize());

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

const char * mmReportForecast::m_template = R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8" />
    <meta http - equiv = "Content-Type" content = "text/html" />
    <title><TMPL_VAR REPORTNAME></title>
    <script src = "memory:apexcharts.js"></script>
    <script src = "memory:sorttable.js"></script>
    <link href = "memory:master.css" rel = "stylesheet" />
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
                    "<TMPL_VAR DATE>"
                <TMPL_ELSE>
                    "<TMPL_VAR DATE>",
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
                            <TMPL_VAR WITHDRAWAL> 
                        <TMPL_ELSE>
                            <TMPL_VAR WITHDRAWAL>,
                        </TMPL_IF>
                    </TMPL_LOOP>
                    ],
            title : "WITHDRAWAL"
        },
        {
            fillColor : 'rgba(129, 172, 123, 0.5)',
            strokeColor : 'rgba(129, 172, 123, 1)',
            pointColor : 'rgba(129, 172, 123, 1)', 
            pointStrokeColor : "#fff",
            data : [
                    <TMPL_LOOP NAME=CONTENTS>
                        <TMPL_IF NAME=__LAST__>
                            <TMPL_VAR DEPOSIT>
                        <TMPL_ELSE>
                            <TMPL_VAR DEPOSIT>,
                        </TMPL_IF>
                    </TMPL_LOOP>
                    ],
            title : "DEPOSIT"
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

*/
