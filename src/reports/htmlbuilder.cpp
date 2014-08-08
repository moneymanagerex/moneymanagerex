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
#include "mmOption.h"
#include "constants.h"
#include "model/Model_Currency.h"
#include "model/Model_Infotable.h"

namespace tags
{
static const char END[] =
        "</body>\n"
        "<script type=\"text/javascript\">\n"
        "var elements = document.getElementsByClassName('money');\n"
        "for (var i = 0; i < elements.length; i++) {\n"
        "    elements[i].style.textAlign = 'right'; \n"
        "   if (elements[i].innerHTML.indexOf(\"-\") > -1) {\n"
        "        elements[i].style.color ='#ff0000'; \n"
        "    }\n"
        "}\n"
        "</script>\n"
        "</html>\n";
static const char HTML[] =
    "<!DOCTYPE html PUBLIC \" -//W3C//DTD HTML 4.01//EN\">"
    "<html>\n<head>\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />"
    "<title>%s - Report</title>\n"
    "<link href = \"master.css\" rel = \"stylesheet\" />\n"
    "<script src = \"Chart.js\"></script>\n"
    "<script src = \"sorttable.js\"></script>\n"
    "    <style>\n"
    "    /* Sortable tables */\n"
    "    table.sortable thead{\n"
    "cursor: default;\n"
    "}\n"
    "</style>\n"
    "</head>\n"
    "<body>\n";
static const wxString DIV_CONTAINER = "<div class='container'>\n";
static const wxString DIV_ROW = "<div class='row'>\n";
static const wxString DIV_COL8 = "<div class='col-xs-2'></div>\n<div class='col-xs-8'>\n";
static const wxString DIV_END = "</div>";
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
static const wxString TOTAL_TABLE_ROW = "  <tr class='success'>\n";
static const wxString TABLE_ROW_END = "  </tr>\n";
static const wxString TABLE_CELL = "    <td %s>";
static const wxString MONEY_CELL = "    <td class='money'>";
static const wxString TABLE_CELL_END = "</td>\n";
static const wxString TABLE_CELL_LINK = "<a href=\"%s\">%s</a>\n";
static const wxString TABLE_HEADER = "<th %s>";
static const wxString HEADER = "<h%i>%s</h%i>";
static const wxString TABLE_HEADER_END = "</th>\n";
static const wxString HEADER_ITALIC = "<font size=\"%i\"><i>%s</i></font>\n";
static const wxString LINK = "<a href=\"%s\">%s</a>\n";
static const wxString HOR_LINE = "<hr size=\"%i\">\n";
static const wxString IMAGE = "<img src=\"%s\" border=\"0\">\n";
static const wxString BR = "<br>\n";
static const wxString NBSP = "&nbsp;";
static const wxString CENTER = "<center>\n";
static const wxString CENTER_END = "</center>\n";
static const wxString TABLE_CELL_SPAN = "    <td colspan=\"%i\" >";
static const wxString TABLE_CELL_RIGHT = "    <td style='text-align: right'>";
static const wxString COLORS [] = {
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
    , "rgba(0, 102, 102, 0.7)"};

}

mmHTMLBuilder::mmHTMLBuilder()
{
    // init font size from config
    font_size_ = mmIniOptions::instance().html_font_size_;

    today_.date = wxDateTime::Today();
    today_.date_str = today_.date.FormatDate();
    today_.todays_date = wxString::Format(_("Today's Date: %s"), today_.date_str);
}

void mmHTMLBuilder::init()
{
    html_ = wxString::Format(wxString::FromUTF8(tags::HTML)
        , mmex::getProgramName()
    );

    //Show user name if provided
    if (mmOptions::instance().userNameString_ != "")
    {
        startTable();
        startTableRow();
        startTableCell();
        addHeader(2, mmOptions::instance().userNameString_);
        endTableCell();
        endTableRow();
        endTable();
        addHorizontalLine(2);
    }
}

void mmHTMLBuilder::addHeader(int level, const wxString& header)
{
    html_+= wxString::Format(tags::HEADER, level, header, level);
}

void mmHTMLBuilder::addDateNow()
{
    addHeader(4, today_.todays_date);
    addLineBreak();
}

void mmHTMLBuilder::addImage(const wxString& src)
{
    if(!src.empty())
        html_+= wxString::Format(tags::IMAGE, src);
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
    , int cols, const wxString& value)
{
    this->startTotalTableRow();
    html_+= wxString::Format(tags::TABLE_CELL_SPAN, cols - 1);
    html_ += caption;
    this->endTableCell();
    html_+= tags::TABLE_CELL_RIGHT;
    html_ += value;
    this->endTableCell();
    this->endTableRow();
}

void mmHTMLBuilder::addTotalRow(const wxString& caption, int cols, double value)
{
    this->addTotalRow(caption, cols, Model_Currency::toCurrency(value));
}

void mmHTMLBuilder::addTotalRow(const wxString& caption, int cols
    , const std::vector<wxString>& data)
{
    this->startTotalTableRow();
    html_+= wxString::Format(tags::TABLE_CELL_SPAN, cols - (int)data.size());
    html_ += caption;

    for (unsigned long idx = 0; idx < data.size(); idx++)
    {
        this->endTableCell();
        html_+= tags::TABLE_CELL_RIGHT;
        html_ += data[idx];
    }
    this->endTableCell();
    this->endTableRow();
}

void mmHTMLBuilder::addTotalRow(const wxString& caption, int cols, const std::vector<double>& data)
{
    std::vector<wxString> data_str;
    for (const auto& value: data)
        data_str.push_back(Model_Currency::toCurrency(value));
    this->addTotalRow(caption, cols, data_str);
}

void mmHTMLBuilder::addTableHeaderCell(const wxString& value, const bool& numeric, const bool& sortable)
{
    wxString align = sortable ? "" : "class='sorttable_nosort'";
    align += numeric ? " class='text-right'" : " class='text-left'";
    html_ += wxString::Format(tags::TABLE_HEADER, align);
    html_ += (value);
    html_+= tags::TABLE_HEADER_END;
}

void mmHTMLBuilder::addCurrencyCell(double amount, const Model_Currency::Data* currency)
{
    wxString s = Model_Currency::toCurrency(amount, currency);
    wxString f = wxString::Format("class='money' sorttable_customkey = '%f'", amount);
    html_ += wxString::Format(tags::TABLE_CELL, f);
    html_ += s;
    this->endTableCell();
}

void mmHTMLBuilder::addMoneyCell(double amount)
{
    wxString s = Model_Currency::toString(amount);
    s.Replace(" ", "&nbsp;");
    wxString f = wxString::Format( "class='money' sorttable_customkey = '%f'", amount);
    html_ += wxString::Format(tags::TABLE_CELL, f);
    html_ += s;
    this->endTableCell();
}

void mmHTMLBuilder::addTableCell(const wxDateTime& date)
{
    wxString date_str = mmGetDateForDisplay(date);
    this->addTableCell(date_str);
}

void mmHTMLBuilder::addTableCell(const wxString& value)
{
    html_ += wxString::Format(tags::TABLE_CELL, "");
    html_ += value;
    this->endTableCell();
}

void mmHTMLBuilder::addColorMarker(const wxString& color)
{
    html_ += wxString::Format(tags::TABLE_CELL, wxString::Format("style='background-color:%s'", color));
    html_ += " ";
    this->endTableCell();
}

const wxString mmHTMLBuilder::getColor(int i)
{
    int c = i % (sizeof(tags::COLORS) / sizeof(wxString));
    wxString color = tags::COLORS[c];
    return color;
}

void mmHTMLBuilder::addTableCellMonth(int month)
{
    if (month >= 0 && month < 12) {
        wxString f = wxString::Format("sorttable_customkey = '%i'", (wxDateTime::Month)month);
        html_ += wxString::Format(tags::TABLE_CELL, f);
        html_ += wxGetTranslation(wxDateTime::GetMonthName((wxDateTime::Month)month));
        this->endTableCell();
    }
    else
        this->addTableCell("");
}

void mmHTMLBuilder::addTableCellLink(const wxString& href
    , const wxString& value)
{
    addTableCell(wxString::Format(tags::TABLE_CELL_LINK, href, value ));
}

void mmHTMLBuilder::DisplayDateHeading(const wxDateTime& startYear, const wxDateTime& endYear, bool withDateRange)
{

    wxString todaysDate = "";
    if (withDateRange)
    {
        todaysDate << today_.todays_date << tags::BR << tags::BR
        << wxString::Format(_("From %s till %s")
            , wxString(mmGetNiceDateSimpleString(startYear)).Prepend("<b>").Append("</b> ")
            , wxString(mmGetNiceDateSimpleString(endYear)).Prepend("<b>").Append("</b> "));
    }
    else
    {
        todaysDate << _("Over Time");
    }
    this->addHeader(4, todaysDate);
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
    html_+= tags::END;
}
void mmHTMLBuilder::addDivContainer()
{
    html_ += tags::DIV_CONTAINER;
}
void mmHTMLBuilder::addDivRow()
{
    html_ += tags::DIV_ROW;
}
void mmHTMLBuilder::addDivCol8()
{
    html_ += tags::DIV_COL8;
}
void mmHTMLBuilder::endDiv()
{
    html_ += tags::DIV_END;
}
void mmHTMLBuilder::endTable()
{
    html_+= tags::TABLE_END;
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
void mmHTMLBuilder::startTotalTableRow()
{
    html_ += tags::TOTAL_TABLE_ROW;
}

void mmHTMLBuilder::endTableRow()
{
    html_+= tags::TABLE_ROW_END;
}

void mmHTMLBuilder::addText(const wxString& text)
{
    html_+= text;
}

void mmHTMLBuilder::addLineBreak()
{
    html_+= tags::BR;
}

void mmHTMLBuilder::addHorizontalLine(int size)
{
    html_+= wxString::Format(tags::HOR_LINE, size);
}

void mmHTMLBuilder::startTableCell(const wxString& width)
{
    html_+= wxString::Format(tags::TABLE_CELL, width);
}
void mmHTMLBuilder::endTableCell()
{
    html_+= tags::TABLE_CELL_END;
}

void mmHTMLBuilder::addPieChart(std::vector<ValueTrio>& valueList, const wxString& id, const int& x, const int& y)
{
    static const wxString data_item =
        "{\n"
        "'color' : '%s',\n"
        "'label' : '%s',\n"
        "'labelAlign' : 'center',\n"
        "'labelColor' : 'black',\n"
        "'labelFontSize' : '12',\n"
        "'value' : %f,\n"
        "},\n";

    wxString data ="[";
    for (const auto& entry : valueList)
    {
        data += wxString::Format(data_item
            , entry.color
            , entry.label, entry.amount);
    }
    data += "]\n";
    static const wxString js = "<script type='text/javascript'>\n"
        "var data = %s;\n"
        "var options = {\n"
        "  animationEasing: 'easeOutQuint'\n"
        "};\n"
        "var ctx = document.getElementById('%s').getContext('2d');\n"
        "var reportChart = new Chart(ctx).Pie(data, options);\n"
        "</script>\n";
    this->addText(wxString::Format("<canvas id='%s' width ='%i' height='%i'></canvas>\n", id, x, y));
    this->addText(wxString::Format(js, data, id));
}

void mmHTMLBuilder::addBarChart(const std::vector<ValueTrio>& data, const wxString& id, const int& x, const int& y)
{
    static const wxString data_item =
        "{\n"
        "  fillColor : '%s',\n"
        "  strokeColor : '%s',\n"
        "  data : [%s],\n"
        "},\n";
    static const wxString js = "<script type='text/javascript'>\n"
        "var data = {\n"
        "  labels : [%s],\n"
        "  datasets : [%s]\n"
        "};\n"
        "var options = {\n"
        "  animationEasing: 'easeOutQuint',\n"
        "  barValueSpacing : 10,\n"
        "  scaleOverride : true,\n"
        "  scaleStartValue : 0,\n"
        "  scaleSteps : [10],\n"
        "  scaleStepWidth : [%i]\n"
        "};\n"
        "var ctx = document.getElementById('%s').getContext('2d');\n"
        "var reportChart = new Chart(ctx).Bar(data, options);\n"
        "</script>\n";

    double steps = 10.0;
    double scaleStepWidth = 1;

    wxString labels = wxEmptyString;
    wxString values = "";

    for (const auto& entry : data)
    {
        if (!entry.label.empty()) labels += wxString::Format("'%s',", entry.label);
        values += wxString::Format(data_item, entry.color, entry.color, wxString::Format("%f", entry.amount));
        scaleStepWidth = std::max(entry.amount, scaleStepWidth);
    }
    scaleStepWidth = ceil(scaleStepWidth / steps);
    // Compute chart spacing and interval (chart forced to start at zero)
    if (scaleStepWidth <= 1.0)
        scaleStepWidth = 1.0;
    else {
        double s = (pow(10, ceil(log10(scaleStepWidth)) - 1.0));
        if (s > 0) scaleStepWidth = ceil(scaleStepWidth / s)*s;
    }

    this->addText(wxString::Format("<canvas id='%s' width ='%i' height='%i'></canvas>\n", id, x, y));
    this->addText(wxString::Format(js, labels, values, (int)scaleStepWidth, id));
}

void mmHTMLBuilder::addLineChart(const std::vector<ValueTrio>& data, const wxString& id, const int& x, const int& y)
{
    static const wxString data_item =
        "{\n"
        "  'label' : '%s',\n"
        "  'strokeColor' : 'rgba(0, 121, 234, 0.7)',\n"
        "  'pointColor' : 'rgba(0, 121, 234, 0.7)',\n"
        "  'pointStrokeColor' : '#fff',\n"
        "  'data' : [%s],\n"
        "},\n";

    static const wxString js = "<script type='text/javascript'>\n"
        "var data = {\n"
        "  labels : [%s],\n"
        "  datasets : [%s]\n"
        "};\n"
        "var ctx = document.getElementById('%s').getContext('2d');\n"
        "var reportChart = new Chart(ctx).Line(data, {datasetFill: false, responsive: true});\n"
        "</script>\n";

    wxString labels = "";
    wxString values = "";

    for (const auto& entry : data)
    {
        labels += wxString::Format("'%s',", entry.label);
        values += wxString::Format("%f,", entry.amount);
    }

    wxString datasets = wxString::Format(data_item, "LineChart", values);
    this->addText(wxString::Format("<canvas id='%s' width ='%i' height='%i'></canvas>\n", id, x, y));
    this->addText(wxString::Format(js, labels, datasets, id));
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

mm_html_template::mm_html_template(const wxString& arg_template): html_template(arg_template.ToStdWstring())
{
    this->load_context();
}

void mm_html_template::load_context()
{
    (*this)(L"TODAY") = wxDateTime::Today().FormatDate();
    for (const auto &r: Model_Infotable::instance().all())
        (*this)(r.INFONAME.ToStdWstring()) = r.INFOVALUE;
    (*this)(L"INFOTABLE") = Model_Infotable::to_loop_t();

    const Model_Currency::Data* currency = Model_Currency::GetBaseCurrency();
    if (currency) currency->to_template(*this);
}
