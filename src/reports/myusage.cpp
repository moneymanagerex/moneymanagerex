/*******************************************************
Copyright (C) 2014 Guan Lisheng (guanlisheng@gmail.com)
Copyright (C) 2021 Mark Whalley (mark@ipx.co.uk)

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
#include "reports/mmDateRange.h"
#include "reports/htmlbuilder.h"
#include "model/Model_Usage.h"

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

             auto module = wxGetTranslation(wxString::FromUTF8(pobj["module"].GetString()));

             if (!pobj.HasMember("name") || !pobj["name"].IsString())
                 continue;

             const wxString rep_name = wxString::FromUTF8(pobj["name"].GetString());
             wxRegEx pattern(R"(^([a-zA-Z0-9_ \/]+)( - ([a-zA-Z0-9_ ]+))?$)");
             if (pattern.Matches(rep_name))
             {
                 const wxString rep_name_1 = pattern.GetMatch(rep_name, 1);
                 const wxString rep_name_2 = pattern.GetMatch(rep_name, 3);
                 module += " / " + wxGetTranslation(rep_name_1) + (rep_name_2.empty() ? "" : " - " + wxGetTranslation(rep_name_2));
             }
             else
             {
                 module += " / " + rep_name;
             }

             if (!pobj.HasMember("seconds") || !pobj["seconds"].IsDouble())
                 continue;

             usage_by_module[module] += 1;
         }
    }

    if (usage_by_module.empty()) {
        usage_by_module[_("Empty value")] = 0;
    }

    std::map<int, wxString> usage_by_frequency;
    std::vector<std::pair<wxString, int>> usage_vector;
    for (const auto &i : usage_by_module) {
        usage_by_frequency[i.second] = i.first;
        usage_vector.push_back(i);
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
    hb.addReportHeader(getReportTitle(), m_date_range->startDay(), m_date_range->isFutureIgnored());
    hb.DisplayDateHeading(m_date_range->start_date(), m_date_range->end_date(), m_date_range->is_with_date());

    if (getChartSelection() == 0)
    {
        GraphData gd;
        GraphSeries data_usage;

        wxArrayString labels;

        std::stable_sort(usage_vector.begin(), usage_vector.end(), [](const std::pair<wxString, int>& left, const std::pair<wxString, int>& right) {
            return left.second > right.second;});

        for (const auto &stats : usage_vector)
        {
            data_usage.values.push_back(stats.second);
            gd.labels.push_back(stats.first);
        }

        data_usage.name = _("Reports");
        gd.series.push_back(data_usage);

        if (!gd.series.empty())
        {
            gd.type = GraphData::PIE;
            hb.addChart(gd);
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
                hb.addTableHeaderCell(_("Frequency"), "text-right");
                hb.endTableRow();
                hb.endThead();
            }
            hb.endThead();
            hb.startTbody();
            {
                for (const auto &stats : usage_by_module)
                {
                    wxString frequency = wxString::Format("%d", stats.second);
                    hb.startTableRow();
                    hb.addTableCell(stats.first);
                    hb.addTableCell(frequency, true);
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
