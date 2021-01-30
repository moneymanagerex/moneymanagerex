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
: mmPrintableBase(wxTRANSLATE("MMEX Usage Frequency"))
{
    setReportParameters(Reports::MyUsage);
}

mmReportMyUsage::~mmReportMyUsage()
{
}

wxString mmReportMyUsage::getHTMLText()
{
    // Grab the data
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
         if (json_doc.Parse(usage.JSONCONTENT.utf8_str()).HasParseError())
             continue;

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

   // Build the report
    mmHTMLBuilder hb;
    hb.init();
    hb.addReportHeader(getReportTitle());
    hb.DisplayDateHeading(m_date_range->start_date(), m_date_range->end_date(), m_date_range->is_with_date());

    if (getChartSelection() == 0)
    {
        GraphData gd;
        GraphSeries data_usage;

        wxArrayString labels;
        for (const auto &stats : usage_by_module)
        {
            data_usage.values.push_back(stats.second);
            gd.labels.push_back(stats.first);
        }

        data_usage.name = _("Reports");
        gd.series.push_back(data_usage);

        if (!gd.series.empty())
        {
            hb.addDivContainer("shadow");
            {
                gd.type = GraphData::PIE;
                hb.addChart(gd);
            }
            hb.endDiv();
        }
    }
    
    hb.addDivContainer("shadow"); 
    { 
        hb.startTable();
        {
            hb.startThead();
            {
                hb.startTableRow();
                hb.addTableHeaderCell(_("Reports"));
                hb.addTableHeaderCell(_("Frequency"), true);
                hb.endTableRow();
                hb.endThead();
            }
            hb.endThead();
            hb.startTbody();
            {
                for (const auto &stats : usage_by_module)
                {
                    wxString frequency = wxString::Format(wxT("%d"), stats.second);
                    hb.startTableRow();
                    hb.addTableCell(stats.first);    
                    hb.addTableCell(frequency,true);
                    hb.endTableRow();
                }
            }
            hb.endTbody();
        }
        hb.endTable();
    }
    hb.endDiv(); 

    hb.end();

    wxLogDebug("======= mmReportUsage:getHTMLText =======");
    wxLogDebug("%s", hb.getHTMLText());

    return hb.getHTMLText();
}