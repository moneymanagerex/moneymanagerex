/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel, Paulo Lopes
 copyright (C) 2012 Nikolay

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
#include "util.h"
#include "option.h"
#include "constants.h"
#include "model/Model_Currency.h"
#include "model/Model_Infotable.h"

namespace tags
{
    static const wxString END = R"(
</body>
<script>
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
    static const char HTML[] = R"(<!DOCTYPE html>
<html><head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
<title>%s - Report</title>
<link href = 'memory:master.css' rel = 'stylesheet' />
<script src = 'memory:ChartNew.js'></script>
<script src = 'memory:sorttable.js'></script>
<style>
    /* Sortable tables */
    table.sortable thead {cursor: default;}
    body { font-size: %s%%; }
</style>
</head>
<body>
)";
    static const wxString DIV_CONTAINER = "<div class='container'>\n";
    static const wxString DIV_ROW = "<div class='row'>\n";
    static const wxString DIV_COL8 = "<div class='col-xs-2'></div>\n<div class='col-xs-8'>\n"; //17_67%
    static const wxString DIV_COL4 = "<div class='col-xs-4'></div>\n<div class='col-xs-4'>\n"; //33_33%
    static const wxString DIV_COL3 = "<div class='col-xs-3'></div>\n<div class='col-xs-6'>\n"; //25_50%
    static const wxString DIV_COL1 = "<div class='col-xs-1'></div>\n"; //8%
    static const wxString DIV_END = "</div>\n";
    static const wxString TABLE_START = "<table class='table'>\n";
    static const wxString SORTTABLE_START = "<table class='sortable table'>\n";
    static const wxString TABLE_END = "</table>\n";
    static const wxString THEAD_START = "<thead>\n";
    static const wxString THEAD_END = "</thead>\n";
    static const wxString TBODY_START = "<tbody>\n";
    static const wxString TBODY_END = "</tbody>\n";
    static const wxString TFOOT_START = "<tfoot>\n";
    static const wxString TFOOT_END = "</tfoot>\n";
    static const wxString TABLE_ROW = "  <tr>\n";
    static const wxString TABLE_ROW_BG = "  <tr %s>\n";
    static const wxString TOTAL_TABLE_ROW = "  <tr class='success'>\n";
    static const wxString TABLE_ROW_END = "  </tr>\n";
    static const wxString TABLE_CELL = "    <td%s>";
    static const wxString MONEY_CELL = "    <td class='money'>";
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
    static const wxString TABLE_CELL_SPAN = "    <td colspan=\"%i\" >";
    static const wxString TABLE_CELL_RIGHT = "    <td style='text-align: right'>";
    static const wxString COLORS[] = {
        "rgba(0, 121, 234, 0.7)"
        , "rgba(238, 42, 0, 0.7)"
        , "rgba(247, 151, 49, 0.7)"
        , "rgba(189, 127, 174, 0.7)"
        , "rgba(255, 243, 171, 0.7)"
        , "rgba(102, 204, 204, 0.7)"
        , "rgba(0, 204, 204, 0.7)"
        , "rgba(100, 145, 170, 0.7)"
        , "rgba(232, 193, 69, 0.7)"
        , "rgba(51, 153, 153, 0.7)"
        , "rgba(210, 154, 247, 0.7)"
        , "rgba(143, 234, 123, 0.7)"
        , "rgba(255, 255, 59, 0.7)"
        , "rgba(122, 179, 62, 0.7)"
        , "rgba(66, 68, 63, 0.7)"
        , "rgba(0, 102, 102, 0.7)" };
}

mmHTMLBuilder::mmHTMLBuilder()
{
    today_.date = wxDateTime::Now();
    today_.todays_date = wxString::Format(_("Report Generated %s %s")
        , mmGetDateForDisplay(today_.date.FormatISODate())
        , today_.date.FormatISOTime());
}

void mmHTMLBuilder::init()
{
    html_ = wxString::Format(wxString::FromUTF8(tags::HTML)
        , mmex::getProgramName()
        , wxString::Format("%d", Option::instance().getHtmlFontSize())
    );

    //Show user name if provided
    if (Option::instance().UserName() != "")
    {
        startTable();
        startTableRow();
        startTableCell();
        addHeader(2, Option::instance().UserName());
        endTableCell();
        endTableRow();
        endTable();
        addHorizontalLine(2);
    }
}

void mmHTMLBuilder::addHeader(int level, const wxString& header)
{
    html_ += wxString::Format(tags::HEADER, level, header, level);
}

void mmHTMLBuilder::addDateNow()
{
    addHeader(4, today_.todays_date);
    addLineBreak();
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
        html_ += tags::TABLE_CELL_RIGHT;
        html_ += data[idx];
    }
    this->endTableCell();
    this->endTableRow();
}

void mmHTMLBuilder::addTotalRow(const wxString& caption, int cols, const std::vector<double>& data)
{
    std::vector<wxString> data_str;
    for (const auto& value : data)
        data_str.push_back(Model_Currency::toCurrency(value));
    this->addTotalRow(caption, cols, data_str);
}

void mmHTMLBuilder::addTableHeaderCell(const wxString& value, const bool numeric, const bool sortable, const int cols, const bool center)
{
    const wxString sort = (sortable ? "" : " class='sorttable_nosort'");
    const wxString align = (center ? " class='text-center'" : (numeric ? " class='text-right'" : " class='text-left'"));
    const wxString cspan = (cols > 1 ? wxString::Format(" colspan='%i'", cols) : "");

    html_ += wxString::Format(tags::TABLE_HEADER, sort + align + cspan);
    html_ += value;
    html_ += tags::TABLE_HEADER_END;
}

void mmHTMLBuilder::addCurrencyCell(double amount, const Model_Currency::Data* currency, int precision)
{
    if (precision == -1)
        precision = Model_Currency::precision(currency);
    const wxString s = Model_Currency::toCurrency(amount, currency, precision);
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
    html_ += wxString::Format("<span style='font-family: serif; color: %s'>%s</span>", color, L"\u2588");
    this->endTableCell();
}

const wxString mmHTMLBuilder::getColor(int i)
{
    int c = i % (sizeof(tags::COLORS) / sizeof(wxString));
    wxString color = tags::COLORS[c];
    return color;
}

const wxString mmHTMLBuilder::getRandomColor(bool positive)
{
    int red = positive ? rand() % 20 : 156 + rand() % 100;
    int green = rand() % 128;
    int blue = positive ? 156 + rand() % 100 : rand() % 20;

    const wxString& color = wxString::Format("rgba(%i, %i, %i, 0.7)", red, green, blue);
    return color;
}

void mmHTMLBuilder::addTableCellMonth(int month)
{
    if (month >= 0 && month < 12) {
        wxString f = wxString::Format(" sorttable_customkey = '%i'", static_cast<wxDateTime::Month>(month));
        html_ += wxString::Format(tags::TABLE_CELL, f);
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

void mmHTMLBuilder::DisplayDateHeading(const wxDateTime& startDate, const wxDateTime& endDate, bool withDateRange)
{
    wxString sDate;
    if (withDateRange)
    {
        sDate << wxString::Format(_("From %s till %s")
            , mmGetDateForDisplay(startDate.FormatISODate())
            , mmGetDateForDisplay(endDate.FormatISODate()));
    }
    else
    {
        sDate << _("Over Time");
    }
    this->addHeader(3, sDate);
}

void mmHTMLBuilder::addTableRow(const wxString& label, double data)
{
    this->startTableRow();
    this->addTableCell(label);
    this->addMoneyCell(data);
    this->endTableRow();
}

void mmHTMLBuilder::end()
{
    html_ += tags::END;
}
void mmHTMLBuilder::addDivContainer()
{
    html_ += tags::DIV_CONTAINER;
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
void mmHTMLBuilder::addDivCol6()
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
void mmHTMLBuilder::startTableRow(const wxString& color)
{
    html_ += wxString::Format(tags::TABLE_ROW_BG, wxString::Format("style='background-color:%s'", color));
}

void mmHTMLBuilder::startTotalTableRow()
{
    html_ += tags::TOTAL_TABLE_ROW;
}

void mmHTMLBuilder::endTableRow()
{
    html_ += tags::TABLE_ROW_END;
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

void mmHTMLBuilder::addRadarChart(std::vector<ValueTrio>& actData, std::vector<ValueTrio>& estData, const wxString& id, int x, int y)
{
    static const wxString html_parts = R"(
<canvas id='%s' width ='%i' height='%i'></canvas>
<script type='text/javascript'>
  var data = {
    labels : [%s],
    datasets : [%s]
  };
  var ctx = document.getElementById('%s').getContext('2d');
  var options = %s;
  var reportChart = new Chart(ctx).Radar(data, options);
</script>
)";

    static const wxString data_item = R"(
{
  'label' : '%s',
  'fillColor' : '%s',
  'strokeColor' : '%s',
  'pointColor' : '%s',
  'pointStrokeColor' : '#fff',
  'data' : [%s],
},)";

    static const wxString opt = R"(
{
  datasetFill: false,
  inGraphDataShow : false,
  annotateDisplay : true,
  responsive: true,
  pointDot : false,
  showGridLines: true
}
)";

    wxString labels = "";
    wxString actValues = "";
    wxString estValues = "";

    for (const auto& entry : actData)
    {
        labels += wxString::Format("'%s',", entry.label);
        actValues += wxString::FromCDouble(fabs(entry.amount), 2) + ",";
    }
    for (const auto& entry : estData)
    {
        estValues += wxString::FromCDouble(fabs(entry.amount), 2) + ",";
    }

    const auto ac = getColor(8);
    const auto ec = getColor(6);
    wxString datasets = wxString::Format(data_item, _("Actual"), ac, ac, ac, actValues);
    datasets += wxString::Format(data_item, _("Estimated"), ec, ec, ec, estValues);
    addText(wxString::Format(html_parts, id, x, y, labels, datasets, id, opt));
}

void mmHTMLBuilder::addPieChart(std::vector<ValueTrio>& valueList, const wxString& id, const int x, const int y)
{
    static const wxString html_parts = R"(
<canvas id='%s' width ='%i' height='%i' style='min-width: %dpx; min-height: %dpx'></canvas>
<script>
  var d = %s;
  var ctx = document.getElementById('%s').getContext('2d');
  var reportChart = new Chart(ctx).Pie(d.data, d.options);
</script>
)";

    int precision = Model_Currency::precision(Model_Currency::GetBaseCurrency());
    int round = pow(10, precision);

    Document jsonDoc;
    jsonDoc.SetObject();
    Value data_array(kArrayType);
    Document::AllocatorType& allocator = jsonDoc.GetAllocator();

    for (const auto& entry : valueList)
    {
        // Replace problem causing character
        auto label = entry.label;
        label.Replace("'", "`");

        Value objValue;
        objValue.SetObject();

        Value title, color;
        title.SetString(label.utf8_str(), allocator);
        objValue.AddMember("title", title, allocator);

        color.SetString(entry.color.utf8_str(), allocator);
        objValue.AddMember("color", color, allocator);

        double v = (floor(fabs(entry.amount) * round) / round);
        objValue.AddMember("value", v, allocator);

        data_array.PushBack(objValue, allocator);
    }

    jsonDoc.AddMember("data", data_array, allocator);

    Value optionsValue;
    optionsValue.SetObject();
    optionsValue.AddMember("annotateDisplay", true, allocator);
    optionsValue.AddMember("segmentShowStroke", false, allocator);
    optionsValue.AddMember("responsive", true, allocator);
    jsonDoc.AddMember("options", optionsValue, allocator);


    StringBuffer strbuf;
    Writer<StringBuffer> writer(strbuf);
    jsonDoc.Accept(writer);

    const wxString data = strbuf.GetString();

    addText(wxString::Format(html_parts, id, x, y, x / 2, y / 2, data, id));
}

void mmHTMLBuilder::addBarChart(const wxArrayString& labels
    , const std::vector<BarGraphData>& data, const wxString& id
    , int x, int y)
{
    static const wxString html_parts = R"(
<canvas id='%s' width ='%i' height='%i' style='min-width: %dpx; min-height: %dpx'></canvas>
<script>
  var d = %s;
  var ctx = document.getElementById('%s').getContext('2d');
  var reportChart = new Chart(ctx).Bar(d.data, d.options);
</script>
)";

    int precision = Model_Currency::precision(Model_Currency::GetBaseCurrency());

    Document jsonDoc;
    jsonDoc.SetObject();
    Document::AllocatorType& allocator = jsonDoc.GetAllocator();

    Value dataObjValue;
    dataObjValue.SetObject();

    Value labelsArray(kArrayType);
    for (const auto& entry : labels)
    {
        Value label_str;
        label_str.SetString(entry.utf8_str(), allocator);
        labelsArray.PushBack(label_str, allocator);
    }
    dataObjValue.AddMember("labels", labelsArray, allocator);

    double max_value = 0;
    int round = pow(10, precision);
    Value datasets_array(kArrayType);
    for (const auto& entry : data)
    {
        Value objValue;
        objValue.SetObject();

        Value title, color;
        title.SetString(entry.title.utf8_str(), allocator);
        objValue.AddMember("title", title, allocator);

        color.SetString(entry.fillColor.utf8_str(), allocator);
        objValue.AddMember("fillColor", color, allocator);

        color.SetString(entry.strokeColor.utf8_str(), allocator);
        objValue.AddMember("strokeColor", color, allocator);

        Value data_array(kArrayType);
        for (const auto& item : entry.data)
        {
            double v = (floor(fabs(item) * round) / round);
            data_array.PushBack(v, allocator);
            max_value = std::max(v, max_value);
        }
        objValue.AddMember("data", data_array, allocator);

        datasets_array.PushBack(objValue, allocator);
    }
    dataObjValue.AddMember("datasets", datasets_array, allocator);

    jsonDoc.AddMember("data", dataObjValue, allocator);

    double vertical_steps = 10.0;
    // Compute chart spacing and interval (chart forced to start at zero)
    double scaleStepWidth = ceil(max_value / vertical_steps);
    // Compute chart spacing and interval (chart forced to start at zero)
    if (scaleStepWidth < 1.0)
        scaleStepWidth = 1.0;
    else {
        double s = (pow(10, ceil(log10(scaleStepWidth)) - 1.0));
        if (s > 0) {
            scaleStepWidth = ceil(scaleStepWidth / s) * s;
        }
    }

    Value optionsValue;
    optionsValue.SetObject();

    optionsValue.AddMember("responsive", true, allocator);
    optionsValue.AddMember("scaleOverride", true, allocator);
    optionsValue.AddMember("annotateDisplay", true, allocator);
    optionsValue.AddMember("scaleStartValue", 0, allocator);
    optionsValue.AddMember("scaleSteps", 0, allocator);
    Value scale(kArrayType);
    scale.PushBack(static_cast<int>(scaleStepWidth), allocator);
    optionsValue.AddMember("scaleStepWidth", scale, allocator);
    Value steps(kArrayType);
    steps.PushBack(vertical_steps, allocator);
    optionsValue.AddMember("scaleSteps", steps, allocator);
    jsonDoc.AddMember("options", optionsValue, allocator);

    StringBuffer strbuf;
    Writer<StringBuffer> writer(strbuf);
    jsonDoc.Accept(writer);

    const wxString d = strbuf.GetString();
    addText(wxString::Format(html_parts, id, x, y, x, y, d, id));
}

void mmHTMLBuilder::addLineChart(const std::vector<LineGraphData>& data, const wxString& id, int colorNum, int x, int y, bool pointDot, bool showGridLines, bool datasetFill)
{
    static const wxString html_parts = R"(
<canvas id='%s' width ='%i' height='%i'></canvas>
<script type='text/javascript'>
var d = %s
var ctx = document.getElementById('%s').getContext('2d');
var reportChart = new Chart(ctx).Line(d.data, d.options);
</script>
)";

    int precision = Model_Currency::precision(Model_Currency::GetBaseCurrency());
    int round = pow(10, precision);

    Document jsonDoc;
    jsonDoc.SetObject();
    Document::AllocatorType& allocator = jsonDoc.GetAllocator();

    Value dObjValue;
    dObjValue.SetObject();

    Value labelsArray(kArrayType);
    Value valuesArray(kArrayType);
    Value xPosArray(kArrayType);

    for (const auto& entry : data)
    {
        Value label_str, xPos;
        label_str.SetString(entry.label.utf8_str(), allocator);
        labelsArray.PushBack(label_str, allocator);

        xPos.SetString(entry.xPos.utf8_str(), allocator);
        xPosArray.PushBack(xPos, allocator);

        double v = (floor((entry.amount) * round) / round);
        valuesArray.PushBack(v, allocator);
    }

    Value datasetsValue;
    datasetsValue.SetObject();
    datasetsValue.AddMember("data", valuesArray, allocator);
    datasetsValue.AddMember("xPos", xPosArray, allocator);

    const auto c = getColor(colorNum);
    Value fillColor, strokeColor, pointColor, pointStrokeColor;
    fillColor.SetString(c.utf8_str(), allocator);
    strokeColor.SetString(c.utf8_str(), allocator);
    pointColor.SetString(c.utf8_str(), allocator);
    fillColor.SetString(c.utf8_str(), allocator);
    datasetsValue.AddMember("fillColor", fillColor, allocator);
    datasetsValue.AddMember("strokeColor", strokeColor, allocator);
    datasetsValue.AddMember("pointColor", pointColor, allocator);
    pointStrokeColor.SetString("#fff", allocator);
    datasetsValue.AddMember("pointStrokeColor", pointStrokeColor, allocator);

    Value datasetsArray(kArrayType);
    datasetsArray.PushBack(datasetsValue, allocator);

    dObjValue.AddMember("labels", labelsArray, allocator);
    dObjValue.AddMember("datasets", datasetsArray, allocator);

    //Options
    Value optionsValue;
    optionsValue.SetObject();
    optionsValue.AddMember("responsive", true, allocator);
    optionsValue.AddMember("datasetFill", datasetFill, allocator);
    optionsValue.AddMember("inGraphDataShow", false, allocator);
    optionsValue.AddMember("annotateDisplay", true, allocator);
    optionsValue.AddMember("responsive", true, allocator);
    optionsValue.AddMember("pointDot", pointDot, allocator);
    optionsValue.AddMember("showGridLines", showGridLines, allocator);

    //
    jsonDoc.AddMember("options", optionsValue, allocator);
    jsonDoc.AddMember("data", dObjValue, allocator);

    StringBuffer strbuf;
    Writer<StringBuffer> writer(strbuf);
    jsonDoc.Accept(writer);

    const wxString d = strbuf.GetString();

    const auto text = (wxString::Format(html_parts, id, x, y, d, id));
    addText(text);
}

const wxString mmHTMLBuilder::getHTMLText() const
{
    return html_;
}

std::ostream& operator << (std::ostream& os, const wxDateTime& date)
{
    os << date.FormatISODate();
    return os;
}

