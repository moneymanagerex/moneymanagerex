/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel, Paulo Lopes
 copyright (C) 2012 - 2021 Nikolay Akimov
 Copyright (C) 2021 - 2022 Mark Whalley (mark@ipx.co.uk)

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

#include "htmlbuilder.h"
#include "images_list.h"
#include "util.h"
#include "option.h"
#include "constants.h"
#include "model/Model_Currency.h"
#include "model/Model_Infotable.h"
#include <iomanip>
#include <ios>


namespace tags
{
    static const char END_SIMPLE[] = R"(</body>)";
    static const char HTML_SIMPLE[] = R"(<body %s %s>)";

    static const wxString END = R"(
</body>
<script>
    $(".toggle").click(function() {
        var kids = $(this).nextUntil(".toggle")
        kids.toggle(kids.first().is(":hidden"))
    })
    function expandAllToggles() {
        $(".xtoggle").show();
    }
    function collapseAllToggles() {
        $(".xtoggle").hide();
    }
    collapseAllToggles();
    var elements = document.getElementsByClassName('money');
    for (var i = 0; i < elements.length; i++) {
        elements[i].style.textAlign = 'right';
        if (elements[i].innerHTML.indexOf('-') > -1) {
            elements[i].style.color ='#ff0000';
        }
    }
</script>
</html>
)";
    static const wxString HTML = R"(<!DOCTYPE html>
<html><head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<title>%s - Report</title>
<link href = 'memory:master.css' rel = 'stylesheet'>
<script>
    window.Promise || document.write('<script src="memory:polyfill.min.js"><\/script>');
    window.Promise || document.write('<script src="memory:classlist.min.js"><\/script>');
    window.Promise || document.write('<script src="memory:resize-observer.js"><\/script>');
    window.Promise || document.write('<script src="memory:findindex.min.js"><\/script>');
    window.Promise || document.write('<script src="memory:umd.min.js"><\/script>');
</script>
<script src = 'memory:apexcharts.min.js'></script>
<script src = 'memory:sorttable.js'></script>
<script src = 'memory:jquery.min.js'></script>
<style>
    /* Sortable tables */
    table.sortable thead {cursor: default;}
    body { font-size: %s%%; }
%s
</style>
</head>
<body>
)";
    static const wxString DIV_CONTAINER = "<div class='%s'>\n";
    static const wxString DIV_ROW = "<div class='row'>\n";
    static const wxString DIV_COL8 = "<div class='col-xs-2'></div>\n<div class='col-xs-8'>\n"; //17_67%
    static const wxString DIV_COL4 = "<div class='col-xs-4'></div>\n<div class='col-xs-4'>\n"; //33_33%
    static const wxString DIV_COL3 = "<div class='col-xs-3'></div>\n<div class='col-xs-6'>\n"; //25_50%
    static const wxString DIV_COL1 = "<div class='col-xs-1'></div>\n<div class='col-xs-10'>\n"; //8%
    static const wxString DIV_END = "</div>\n";
    static const wxString TABLE_START = "<table class='table table-bordered'>\n";
    static const wxString SORTTABLE_START = "<table class='sortable table'>\n";
    static const wxString TABLE_END = "</table>\n";
    static const wxString THEAD_START = "<thead>\n";
    static const wxString THEAD_END = "</thead>\n";
    static const wxString TBODY_START = "<tbody>\n";
    static const wxString TBODY_END = "</tbody>\n";
    static const wxString TFOOT_START = "<tfoot>\n";
    static const wxString TFOOT_END = "</tfoot>\n";
    static const wxString TABLE_ROW = "<tr>\n";
    static const wxString TABLE_ROW_EXTRA = "<tr %s>\n";
    static const wxString TOTAL_TABLE_ROW = "<tr class='success'>\n";
    static const wxString TABLE_ROW_END = "</tr>\n";
    static const wxString TABLE_CELL = "<td%s>";
    static const wxString MONEY_CELL = "<td class='money'>";
    static const wxString TABLE_CELL_END = "</td>\n";
    static const wxString TABLE_CELL_LINK = R"(<a href="%s" target="_blank">%s</a>)";
    static const wxString TABLE_HEADER = "<th%s>";
    static const wxString HEADER = "<h%i>%s</h%i>";
    static const wxString TABLE_HEADER_END = "</th>\n";
    static const wxString LINK = "<a href=\"%s\">%s</a>\n";
    static const wxString HOR_LINE = "<hr size=\"%i\">\n";
    static const wxString IMAGE = "<img src=\"%s\" border=\"0\">\n";
    static const wxString BR = "<br>\n";
    static const wxString NBSP = "&nbsp;";
    static const wxString CENTER = "<center>\n";
    static const wxString CENTER_END = "</center>\n";
    static const wxString TABLE_CELL_SPAN = "<td colspan=\"%i\" >";
    static const wxString TABLE_CELL_RIGHT = "<td style='text-align: right'>";
    static const wxString SPAN = "<span %s>%s";
    static const wxString SPAN_END = "</span>\n";
}

mmHTMLBuilder::mmHTMLBuilder()
{
    today_.date = wxDateTime::Now();
    today_.todays_date = wxString::Format(_("Report Generated %s %s")
        , mmGetDateForDisplay(today_.date.FormatISODate())
        , today_.date.FormatISOTime());
}

void mmHTMLBuilder::init(bool simple, const wxString& extra_style)
{
    if (simple)
    {
        wxString bg = mmThemeMetaString(meta::COLOR_HTMLPANEL_BACK);
        wxString fg = mmThemeMetaString(meta::COLOR_HTMLPANEL_FORE);
        html_ += wxString::Format(tags::HTML_SIMPLE
                    , bg.IsEmpty() ? "" : wxString::Format("bgcolor='%s';", bg)
                    , fg.IsEmpty() ? "" : wxString::Format("text='%s';", fg));
    } else
    {
        html_ = wxString::Format(tags::HTML
            , mmex::getProgramName()
            , wxString::Format("%d", Option::instance().getHtmlFontSize())
            , extra_style);
    }
}

void mmHTMLBuilder::showUserName()
{
    //Show user name if provided
    if (Option::instance().UserName() != "")
        addHeader(2, Option::instance().UserName());
}

void mmHTMLBuilder::addReportHeader(const wxString& name, int startDay, bool futureIgnored)
{
    wxLogDebug("futureIgnored: %d",futureIgnored);
    addDivContainer("shadowTitle");
    {
        addText("<header>");
        addHeader(2, name);
        addText("</header>");

        addText("<aside>");
        {
            showUserName();
            addText("<TMPL_VAR DATE_HEADING>");
            addOffsetIndication(startDay);
            addFutureIgnoredIndication(futureIgnored);
            addReportCurrency();
            addDateNow();
        }
        addText("</aside>");

        addText("<footer>");
        addText("<TMPL_VAR FOOTER>");
        addText("</footer>");
    }
    endDiv();
}

void mmHTMLBuilder::DisplayDateHeading(const wxDateTime& startDate, const wxDateTime& endDate, bool withDateRange, bool withNoEndDate)
{
    wxString sDate;
    if (withDateRange && startDate.IsValid() && endDate.IsValid()) {
        sDate << wxString::Format(_("From %s till %s")
            , mmGetDateForDisplay(startDate.FormatISODate())
            , withNoEndDate ? _("Future") : mmGetDateForDisplay(endDate.FormatISODate()));
    }
    else if (!withDateRange) {
        sDate << _("Over Time");
    }
    else
        wxASSERT(false);

    wxString t = wxString::Format(tags::HEADER, 4, sDate, 4);
    this->html_.Replace("<TMPL_VAR DATE_HEADING>", t);
}

void mmHTMLBuilder::DisplayFooter(const wxString& footer)
{
    this->html_.Replace("<TMPL_VAR FOOTER>", footer);
}

void mmHTMLBuilder::addHeader(int level, const wxString& header)
{
    html_ += wxString::Format(tags::HEADER, level, header, level);
}

void mmHTMLBuilder::addReportCurrency()
{
    wxString base_currency_symbol;
    wxASSERT_MSG(Model_Currency::GetBaseCurrencySymbol(base_currency_symbol), "Could not find base currency symbol");

    addHeader(5, wxString::Format("%s: %s", _("Currency"), base_currency_symbol));  
}

void mmHTMLBuilder::addOffsetIndication(int startDay)
{       
    if (startDay > 1)
        addHeader(5, wxString::Format ("%s: %d"
            , _("User specified start day")
            , startDay));
}

void mmHTMLBuilder::addFutureIgnoredIndication(bool ignore)
{       
    if (ignore)
        addHeader(5, _("Future Transactions have been ignored"));
}

void mmHTMLBuilder::addDateNow()
{
    addHeader(5, today_.todays_date);
}

void mmHTMLBuilder::startTable()
{
    html_ += tags::TABLE_START;
}
void mmHTMLBuilder::startSortTable()
{
    html_ += tags::SORTTABLE_START;
}
void mmHTMLBuilder::startThead()
{
    html_ += tags::THEAD_START;
}
void mmHTMLBuilder::startTbody()
{
    html_ += tags::TBODY_START;
}
void mmHTMLBuilder::startTfoot()
{
    html_ += tags::TFOOT_START;
}

void mmHTMLBuilder::addEmptyTableRow(int cols)
{
    this->startTotalTableRow();
    html_ += wxString::Format(tags::TABLE_CELL_SPAN, cols);
    this->endTableCell();
    this->endTableRow();
}

void mmHTMLBuilder::addTotalRow(const wxString& caption
    , int cols, double value)
{
    this->startTotalTableRow();
    html_ += wxString::Format(tags::TABLE_CELL_SPAN, cols - 1);
    html_ += caption;
    this->endTableCell();
    this->addMoneyCell(value);
    this->endTableRow();
}

void mmHTMLBuilder::addTotalRow(const wxString& caption, int cols
    , const std::vector<wxString>& data)
{
    this->startTotalTableRow();
    html_ += wxString::Format(tags::TABLE_CELL_SPAN, cols - static_cast<int>(data.size()));
    html_ += caption;

    for (unsigned long idx = 0; idx < data.size(); idx++)
    {
        this->endTableCell();
        html_ += tags::MONEY_CELL;
        html_ += data[idx];
    }
    this->endTableCell();
    this->endTableRow();
}

void mmHTMLBuilder::addCurrencyTotalRow(const wxString& caption, int cols, const std::vector<double>& data)
{
    std::vector<wxString> data_str;
    for (const auto& value : data)
        data_str.push_back(Model_Currency::toCurrency(value));
    this->addTotalRow(caption, cols, data_str);
}

void mmHTMLBuilder::addMoneyTotalRow(const wxString& caption, int cols, const std::vector<double>& data)
{
    std::vector<wxString> data_str;
    int precision = Model_Currency::precision(Model_Currency::GetBaseCurrency());

    for (const auto& value : data)
        data_str.push_back(Model_Currency::toString(value, Model_Currency::GetBaseCurrency(), precision));
    this->addTotalRow(caption, cols, data_str);
}

void mmHTMLBuilder::addTableHeaderCell(const wxString& value, const wxString& css_class, int cols)
{
    html_ += wxString::Format(tags::TABLE_HEADER //TABLE_HEADER = "<th%s>";
        , wxString::Format("%s%s"
            , css_class.empty() ? "" : wxString::Format(" class='%s'", css_class)
            , cols > 1 ? wxString::Format(" colspan='%i'", cols) : "")
    );
    html_ += value;
    html_ += tags::TABLE_HEADER_END;
}

void mmHTMLBuilder::addCurrencyCell(double amount, const Model_Currency::Data* currency, int precision, bool isVoid)
{
    if (precision == -1)
        precision = Model_Currency::precision(currency);
    wxString s = Model_Currency::toCurrency(amount, currency, precision);
    if (isVoid)
        s = wxString::Format("<s>%s</s>", s);
    const wxString f = wxString::Format(" class='money' sorttable_customkey = '%f' nowrap", amount);
    html_ += wxString::Format(tags::TABLE_CELL, f);
    html_ += s;
    this->endTableCell();
}

void mmHTMLBuilder::addMoneyCell(double amount, int precision)
{
    if (precision == -1)
        precision = Model_Currency::precision(Model_Currency::GetBaseCurrency());
    const wxString s = Model_Currency::toString(amount, Model_Currency::GetBaseCurrency(), precision);
    wxString f = wxString::Format(" class='money' sorttable_customkey = '%f' nowrap", amount);
    html_ += wxString::Format(tags::TABLE_CELL, f);
    html_ += s;
    this->endTableCell();
}

void mmHTMLBuilder::addTableCellDate(const wxString& iso_date)
{
    html_ += wxString::Format(tags::TABLE_CELL
        , wxString::Format(" class='text-left' sorttable_customkey = '%s' nowrap", iso_date));
    html_ += mmGetDateForDisplay(iso_date);
    this->endTableCell();
}

void mmHTMLBuilder::addTableCell(const wxString& value, bool numeric, bool center)
{
    const wxString align = (center ? " class='text-center'" : (numeric ? " class='text-right' nowrap" : " class='text-left'"));
    html_ += wxString::Format(tags::TABLE_CELL, align);
    html_ += value;
    this->endTableCell();
}

void mmHTMLBuilder::addEmptyTableCell(const int number)
{
    for (int i = 0; i < number; i++)
        this->addTableCell("");
}

void mmHTMLBuilder::addColorMarker(const wxString& color)
{
    html_ += wxString::Format(tags::TABLE_CELL, "");
    html_ += wxString::Format("<span style='font-family: serif; %s'>%s</span>"
        , (color.empty() ? "": wxString::Format("color: %s", color))
        , (color.empty() ? L" " : L"\u2588"));
    this->endTableCell();
}

const wxString mmHTMLBuilder::getColor(int i)
{
    std::vector<wxColour> colours = mmThemeMetaColourArray(meta::COLOR_REPORT_PALETTE);
    int c = i % colours.size();
    return colours.at(c).GetAsString(wxC2S_HTML_SYNTAX);
}

const wxString mmHTMLBuilder::getRandomColor(bool positive)
{
    int red = positive ? rand() % 20 : 156 + rand() % 100;
    int green = rand() % 128;
    int blue = positive ? 156 + rand() % 100 : rand() % 20;

    const wxString& color = wxString::Format("rgba(%i, %i, %i, 0.7)", red, green, blue);
    return color;
}

void mmHTMLBuilder::addTableCellMonth(int month, int year)
{
    if (month >= 0 && month < 12) {
        wxString f = wxString::Format(" sorttable_customkey = '%i'", year * 100 + month);
        html_ += wxString::Format(tags::TABLE_CELL, f);
        if (0 != year)
            html_ += wxString::Format("%d ", year);
        html_ += wxGetTranslation(wxDateTime::GetEnglishMonthName(static_cast<wxDateTime::Month>(month)));
        this->endTableCell();
    }
    else
        this->addTableCell("");
}

void mmHTMLBuilder::addTableCellLink(const wxString& href
    , const wxString& value)
{
    addTableCell(wxString::Format(tags::TABLE_CELL_LINK, href, value));
}

void mmHTMLBuilder::addTableRow(const wxString& label, double data)
{
    this->startTableRow();
    this->addTableCell(label);
    this->addMoneyCell(data);
    this->endTableRow();
}

void mmHTMLBuilder::end(bool simple)
{
    html_ += simple ? tags::END_SIMPLE : tags::END;
}
void mmHTMLBuilder::addDivContainer(const wxString& style)
{
    html_ += wxString::Format(tags::DIV_CONTAINER, style);
}
void mmHTMLBuilder::addDivRow()
{
    html_ += tags::DIV_ROW;
}
void mmHTMLBuilder::addDivCol17_67()
{
    html_ += tags::DIV_COL8;
}
void mmHTMLBuilder::addDivCol25_50()
{
    html_ += tags::DIV_COL3;
}
void mmHTMLBuilder::addDivCol8_84()
{
    html_ += tags::DIV_COL1;
}
void mmHTMLBuilder::endDiv()
{
    html_ += tags::DIV_END;
}
void mmHTMLBuilder::endTable()
{
    html_ += tags::TABLE_END;
}
void mmHTMLBuilder::endThead()
{
    html_ += tags::THEAD_END;
};
void mmHTMLBuilder::endTbody()
{
    html_ += tags::TBODY_END;
};
void mmHTMLBuilder::endTfoot()
{
    html_ += tags::TFOOT_END;
};

void mmHTMLBuilder::startTableRow()
{
    html_ += tags::TABLE_ROW;
}
void mmHTMLBuilder::startTableRow(const wxString& classname)
{
    html_ += wxString::Format(tags::TABLE_ROW_EXTRA, wxString::Format("class='%s'", classname));
}
void mmHTMLBuilder::startTableRowColor(const wxString& color)
{
    html_ += wxString::Format(tags::TABLE_ROW_EXTRA, wxString::Format("style='background-color:%s'", color));
}

void mmHTMLBuilder::startAltTableRow()
{
    startTableRowColor(mmThemeMetaString(meta::COLOR_REPORT_ALTROW));
}

void mmHTMLBuilder::startTotalTableRow()
{
    html_ += tags::TOTAL_TABLE_ROW;
}

void mmHTMLBuilder::endTableRow()
{
    html_ += tags::TABLE_ROW_END;
}

void mmHTMLBuilder::startSpan(const wxString& val, const wxString& style)
{
    html_ += wxString::Format(tags::SPAN, style, val);
}

void mmHTMLBuilder::endSpan()
{
    html_ += tags::SPAN_END;
}

void mmHTMLBuilder::addText(const wxString& text)
{
    html_ += text;
}

void mmHTMLBuilder::addLineBreak()
{
    html_ += tags::BR;
}

void mmHTMLBuilder::addHorizontalLine(int size)
{
    html_ += wxString::Format(tags::HOR_LINE, size);
}

void mmHTMLBuilder::startTableCell(const wxString& width)
{
    html_ += wxString::Format(tags::TABLE_CELL, width);
}
void mmHTMLBuilder::endTableCell()
{
    html_ += tags::TABLE_CELL_END;
}

// Chart method (uses ApexChart.js)

void mmHTMLBuilder::addChart(const GraphData& gd)
{
    int precision = Model_Currency::precision(Model_Currency::GetBaseCurrency());
    int k = pow10(precision);
    wxString htmlChart, htmlPieData;
    wxString divid = wxString::Format("apex%i", rand()); // Generate unique identifier for each graph
 
    // Chart Type and Series type
    wxString gtype;
    int chartWidth = 95;
    wxString gSeriesType = "category";
    switch (gd.type)
    {
        case GraphData::STACKEDAREA:
            gtype = "area";
            if (gd.labels.size() < 5)
                chartWidth = 70;
            break;
        case GraphData::BAR:
            gtype = "bar";
            if (gd.labels.size() < 5)
                chartWidth = 70;
            break;
        case GraphData::LINE:
            gtype = "line";
            break;
        case GraphData::LINE_DATETIME:
            gtype = "line";
            gSeriesType = "datetime";
            break;
        case GraphData::PIE:
            gtype = "pie";
            chartWidth = 100;
            break;
        case GraphData::DONUT:
            gtype = "donut";
            chartWidth = 70;
            break;
        case GraphData::RADAR:
            gtype = "radar";
            chartWidth = 70;
            break;
        case GraphData::BARLINE:
        case GraphData::STACKEDBARLINE:
            gtype = "line";
            if (gd.labels.size() < 5)
                chartWidth = 70;
    };

    addDivContainer("shadowGraph"); 

    htmlChart += wxString::Format("chart: { animations: { enabled: false }, type: '%s', %s foreColor: '%s', toolbar: { tools: { download: false } }, width: '%i%%' }" 
                    , gtype
                    , (gd.type == GraphData::STACKEDAREA || 
                       gd.type == GraphData::STACKEDBARLINE) ? "stacked: true," : ""
                    , mmThemeMetaString(meta::COLOR_REPORT_FORECOLOR)
                    , chartWidth);
    htmlChart += wxString::Format(", title: { text: '%s'}", gd.title);

    wxString toolTipFormatter;
    if (gd.type == GraphData::PIE || gd.type == GraphData::DONUT) 
    {
        htmlChart += ", plotOptions: { pie: { customScale: 0.8 } }";

        const wxString pieFunctionToolTip = wxString::Format("function(value, opts) { return chart_%s[opts.dataPointIndex] }\n", divid);
        toolTipFormatter = wxString::Format(", y: { formatter: %s }", pieFunctionToolTip); 
    }

    htmlChart += wxString::Format(", tooltip: { theme: 'dark' %s }\n", toolTipFormatter);

    // Turn off data labels for bar charts when they get too cluttered
    if ((gd.type == GraphData::BAR || gd.type == GraphData::STACKEDAREA) && gd.labels.size() > 10) 
    {
        htmlChart += ", dataLabels: { enabled: false }";
    } else if (gd.type == GraphData::PIE || gd.type == GraphData::DONUT)
    {
        htmlChart += ", dataLabels: { enabled: true, style: { fontSize: '16px' }, dropShadow: { enabled: false } }\n";
    }

    // If colors are specified then use these in prefernce to loading our standard pallete
    std::vector<wxColour> colors;
    if (!gd.colors.empty())
        colors = gd.colors;
    else
        colors = mmThemeMetaColourArray(meta::COLOR_REPORT_PALETTE);

    htmlChart += ", colors: ";
    bool first = true;
    for (const auto& entry : colors)
    {
        htmlChart += wxString::Format("%s'%s'", first ? "[":",", entry.GetAsString(wxC2S_HTML_SYNTAX));
        first = false;
    }
    htmlChart += "]";

    wxString categories;
    first = true;
    for (const auto& entry : gd.labels) 
    {
        wxString label = entry;
        label.Replace("'","\\'"); // Need to escape the quotes!
        categories += wxString::Format("%s'%s'", first ? "":",", label);
        first = false; 
    }

    // Pie/donut charts just have a single series / data, and mixed have a single set of labels
    if (gd.type == GraphData::PIE || gd.type == GraphData::DONUT || gd.type == GraphData::BARLINE
        || gd.type == GraphData::STACKEDBARLINE)
       htmlChart += wxString::Format(",labels: [%s]", categories);
    else
        htmlChart += wxString::Format(", xaxis: { type: '%s', categories: [%s], labels: { hideOverlappingLabels: true } }\n", gSeriesType, categories);

    wxString seriesList, pieEntries;
    bool firstList = true;
    for (const auto& entry : gd.series)
    {
        wxString seriesEntries;
        first = true;
        for (const auto& item : entry.values)
        {
            long double v = item * k;
            v = round(v) / k;

            // avoid locale usage with standard printf functionality. Always want 00000.00 format
            std::ostringstream oss;
            oss.imbue(std::locale::classic());

            oss << std::fixed << std::setprecision(precision) << fabs(v);
            wxString valueAbs = oss.str();

            oss.str(std::string());
            oss << std::fixed << std::setprecision(precision) << v;
            wxString value = oss.str();

            if (gd.type == GraphData::PIE || gd.type == GraphData::DONUT)
            {
                seriesEntries += wxString::Format("%s%s", first ? "":",", valueAbs);    // pie data series must be positive
                pieEntries += wxString::Format("%s%s", first ? "":",", value);
            } else
            {
                seriesEntries += wxString::Format("%s%s", first ? "":",", value);
            }
            first = false;
        }
        if (gd.type == GraphData::PIE || gd.type == GraphData::DONUT) 
            seriesList = seriesEntries;
        else
        {
            const wxString typeString = (gd.type == GraphData::BARLINE || gd.type == GraphData::STACKEDBARLINE)
                                ? wxString::Format("type: '%s',", entry.type) : "";
            seriesList += wxString::Format("%s{ name: '%s', %s data: [%s] }", firstList ? "":",", entry.name, typeString, seriesEntries);
        }
        firstList = false;
    }
    htmlChart += wxString::Format(", series: [%s]", seriesList);

    if (gd.type == GraphData::BARLINE || gd.type == GraphData::STACKEDBARLINE)
    {
        htmlChart += ", dataLabels: { enabled: true, enabledOnSeries: [";   // Always label the lines
        int seriesNo = 0;
        first = true;
        for (const auto& item : gd.series) {
            if (item.type == "line") {
                htmlChart += wxString::Format("%s%i", first ? "":",", seriesNo);
                first = false;
            }
            seriesNo++;
        }
        htmlChart += "] }";   // Always label the lines
    }

    htmlPieData += wxString::Format("var chart_%s = [ %s ]", divid, pieEntries);

    addText(wxString::Format("<div id='%s' class='%s'></div>\n"
        "<script>\n"
        "%s; var options = { %s };\n"
        "var chart = new ApexCharts(document.querySelector('#%s'), options); chart.render();\n"
        "</script>\n", 
        divid, gtype, htmlPieData, htmlChart, divid));
    
    endDiv();
};

const wxString mmHTMLBuilder::getHTMLText() const
{
    return html_;
}

std::ostream& operator << (std::ostream& os, const wxDateTime& date)
{
    os << date.FormatISODate();
    return os;
}

