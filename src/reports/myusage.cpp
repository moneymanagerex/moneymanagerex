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
#include "model/Model_Usage.h"
#include "model/Model_Report.h"

mmReportMyUsage::mmReportMyUsage()
: mmPrintableBase(_("MMEX Usage Frequency"))
{
    setReportParameters(Reports::MyUsage);
}

mmReportMyUsage::~mmReportMyUsage()
{
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

    std::map<wxString, int> usage_by_module;

    for (const auto & usage : all_usage)
    {
         Document json_doc;
         if (json_doc.Parse(usage.JSONCONTENT.utf8_str()).HasParseError()) {
             continue;
         }

         //wxLogDebug("======= mmReportMyUsage::getHTMLText =======");
         //wxLogDebug("RapidJson\n%s", JSON_PrettyFormated(json_doc));

         if (!json_doc.HasMember("usage") || !json_doc["usage"].IsArray())
             continue;
         Value u = json_doc["usage"].GetArray();

         for (Value::ConstValueIterator it = u.Begin(); it != u.End(); ++it)
         {
             if (!it->IsObject()) continue;
             const auto pobj = it->GetObject();

             if (!pobj.HasMember("module") || !pobj["module"].IsString())
                 continue;
             auto module = wxString::FromUTF8(pobj["module"].GetString());

             if (!pobj.HasMember("name") || !pobj["name"].IsString())
                 continue;
             module += " / " + wxString::FromUTF8(pobj["name"].GetString());

             if (!pobj.HasMember("seconds") || !pobj["seconds"].IsDouble())
                 continue;
             const auto s = pobj["seconds"].GetDouble();

             usage_by_module[module] += 1;
         }
    }

    if (usage_by_module.empty()) {
        usage_by_module[_("Empty value")] = 0;
    }

    std::map<int, wxString> usage_by_frequency;
    for (const auto i : usage_by_module) {
        usage_by_frequency[i.second] = i.first;
    }

    loop_t contents;
    for (auto it = usage_by_frequency.begin(); it != usage_by_frequency.end(); ++it)
    {
        row_t r;

        r(L"MODULE_NAME") = it->second;
        r(L"FREQUENCY") = wxString::Format("%d", it->first);

        contents += r;
    }

    mm_html_template report(usage_template);
    report(L"CANVAS") = getChartSelection() == 0 ? R"(<canvas id="MY_CANVAS"></canvas>)" : "";
    report(L"REPORTNAME") = getReportTitle();
    report(L"HEADER_FREQUENCY") = _("Frequency");
    report(L"HEADER_NAME") = _("Reports");
    report(L"CONTENTS") = contents;
    report(L"HTMLSCALE") = wxString::Format("%d", Option::instance().getHtmlFontSize());
    
    wxDateTime today = wxDateTime::Now();
    const wxString current_day_time = wxString::Format(_("Report Generated %s %s")
        , mmGetDateForDisplay(today.FormatISODate())
        , today.FormatISOTime());
    report(L"TODAY") = current_day_time;

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

const char* mmReportMyUsage::usage_template = R"(
<!DOCTYPE html>
<html>
<head>
  <meta http-equiv="content-type" content="text/html; charset=utf-8" />
  <title><TMPL_VAR REPORTNAME></title>
  <script src = "memory:ChartNew.js"></script>
  <script src = "memory:format.js"></script>
  <script src = "memory:sorttable.js"></script>
  <link href = "memory:master.css" rel = "stylesheet" />
  <style>
    canvas {min-height: 100px}
    body {font-size: <TMPL_VAR HTMLSCALE>%}
  </style>
</head>
<body>

<div class = "container">
<h3><TMPL_VAR REPORTNAME>

</h3>
<TMPL_VAR TODAY><hr>

<div class = "row">
<TMPL_VAR CANVAS">
<script>
function getRandomColor() {
    var letters = '0123456789ABCDEF'.split('');
    var color = '#';
    for (var i = 0; i < 6; i++ ) {
        color += letters[Math.floor(Math.random() * 16)];
    }
    return color;
};
  var d = {
  data:
    [
<TMPL_LOOP NAME=CONTENTS>
      {
        title : '<TMPL_VAR MODULE_NAME>',
        color : getRandomColor(),
        value : <TMPL_VAR FREQUENCY>
      },
</TMPL_LOOP>
    ]
  };
  var opts= {
    annotateDisplay: true,
    responsive: true,
    segmentShowStroke: false
  };

  var ctx = document.getElementById("MY_CANVAS").getContext("2d");

  window.onload = function() {
      var myBar = new Chart(ctx).Pie(d.data,opts);
  }

</script>
</div>
<table class='table sortable'>
<thead>
  <tr>
<th><TMPL_VAR HEADER_NAME></th>
<th><TMPL_VAR HEADER_FREQUENCY></th>
  </tr>
</thead>
<tbody>
<TMPL_LOOP NAME=CONTENTS>
  <tr class='success'>
    <td class="sorttable_customkey="<TMPL_VAR MODULE_NAME>"><TMPL_VAR MODULE_NAME></td>
    <td class="sorttable_customkey="<TMPL_VAR FREQUENCY>"><TMPL_VAR FREQUENCY></td>
  </tr>
</TMPL_LOOP>
</tbody>
</table>
</div></body>
</html>
)";
