/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel, Paulo Lopes

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
static const char HTML[] =
    "<html>\n<head>\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />"
    "<title>%s - Report</title>\n"
    "</head>\n"
    "<body bgcolor=\"%s\" "
    "text=\"%s\" "
    "link=\"%s\" "
    "vlink=\"%s\" "
    "alink=\"%s\">\n";
static const wxString TABLE_START = "\n<table cellspacing=\"1\" bgcolor=\"%s\" width=\"%s\" valign=\"%s\" border=\"%s\">\n";
static const wxString TABLE_END = "</table>\n";
static const wxString TABLE_ROW = "<tr bgcolor=\"%s\" >";
static const wxString TABLE_ROW_END = "</tr>";
static const wxString TABLE_CELL = "<td width=\"%s\" >";
static const wxString TABLE_CELL_RIGHT = "<td nowrap align=\"right\">";
static const wxString TABLE_CELL_SPAN = "<td colspan=\"%i\" >";
static const wxString TABLE_CELL_END = "</td>\n";
static const wxString TABLE_CELL_LINK = "<a href=\"%s\">%s</a>\n";
static const wxString TABLE_HEADER = "<th align=\"%s\" valign=\"center\" bgcolor=\"%s\" colspan=\"%i\">";
static const wxString TABLE_HEADER_END = "</th>\n";
static const wxString HEADER = "<font size=\"%i\"><b>%s</b></font><br>\n";
static const wxString HEADER_ITALIC = "<font size=\"%i\"><i>%s</i></font>\n";
static const wxString PARAGRAPH = "<p><font size=\"%d\">%s</font></p>\n";
static const wxString LINK = "<a href=\"%s\">%s</a>\n";
static const wxString BI = "<b><i>%s</i></b>";
static const wxString BOLD = "<b>%s</b>";
static const wxString ITALIC = "<i>%s</i>";
static const wxString FONT = "<font size=\"%i\" color=\"%s\">";
static const wxString FONT_END = "</font>";
static const wxString HOR_LINE = "<hr size=\"%i\">\n";
static const wxString IMAGE = "<img src=\"%s\" border=\"0\">";
static const wxString END = "\n</body>\n</html>\n";
static const wxString BR = "<br>\n";
static const wxString NBSP = "&nbsp;";
static const wxString CENTER = "<center>";
static const wxString CENTER_END = "</center>";
}

mmHTMLBuilder::mmHTMLBuilder()
{
    // init colors from config
    color_.color1 = mmColors::listAlternativeColor0.GetAsString(wxC2S_HTML_SYNTAX);
    color_.color0 = mmColors::listBackColor.GetAsString(wxC2S_HTML_SYNTAX);
    color_.bgcolor = mmColors::listBackColor.GetAsString(wxC2S_HTML_SYNTAX);
    color_.text = mmColors::listBorderColor.GetAsString(wxC2S_HTML_SYNTAX);
    color_.link = "#0000cc";
    color_.vlink = "#551a8b";
    color_.alink = "#ff0000";
    color_.table_header = "#4f81bd";
    // init font size from config
    font_size_ = mmIniOptions::instance().html_font_size_;

    today_.date = wxDateTime::Today();
    today_.date_str = today_.date.FormatDate();
    today_.todays_date = wxString::Format(_("Today's Date: %s"), today_.date_str);
}

void mmHTMLBuilder::init()
{
    color_.bgswitch = true;
    html_ = wxString::Format(wxString::FromUTF8(tags::HTML)
        , mmex::getProgramName()
        , color_.bgcolor
        , color_.text
        , color_.link
        , color_.vlink
        , color_.alink
    );

    //Show user name if provided
    if (mmOptions::instance().userNameString_ != "")
    {
        startTable("100%");
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
    int header_font_size = level + font_size_;
    if (header_font_size > 7) header_font_size = 7;
    html_+= wxString::Format(tags::HEADER, header_font_size, header);
}

void mmHTMLBuilder::addHeaderItalic(int level, const wxString& header)
{
    int header_font_size = level + font_size_;
    if (header_font_size > 7) header_font_size = 7;
    html_+= wxString::Format(tags::HEADER_ITALIC, header_font_size, header);
}

void mmHTMLBuilder::addDateNow()
{
    addHeaderItalic(1, today_.todays_date);
    addLineBreak();
}

void mmHTMLBuilder::addImage(const wxString& src)
{
    if(!src.empty())
        html_+= wxString::Format(tags::IMAGE, src);
}

void mmHTMLBuilder::startTable(const wxString& width
    , const wxString& valign, const wxString& border)
{
    html_+= wxString::Format(tags::TABLE_START
        , color_.bgcolor
        , width, valign, border);
    color_.bgswitch = true;
}

void mmHTMLBuilder::addRowSeparator(int cols)
{
    color_.bgswitch = true;

    if (cols > 0)
    {
        startTableRow();
        html_+= wxString::Format(tags::TABLE_CELL_SPAN, cols);
        addText("<hr>");
        endTableCell();
        endTableRow();
        color_.bgswitch = true;
    }
}

void mmHTMLBuilder::addTotalRow(const wxString& caption
    , int cols, const wxString& value, const bool color)
{
    this->startTableRow(color_.bgcolor);
    html_+= wxString::Format(tags::TABLE_CELL_SPAN, cols - 1);
    this->font_settings(font_size_);
    this->bold_italic(tags::NBSP + tags::NBSP + caption);
    this->font_end();
    this->endTableCell();
    html_+= wxString::Format(tags::TABLE_CELL_RIGHT);
    double amount = 0;
    if (Model_Currency::fromString(value, amount, 0))
        this->font_settings(font_size_, (amount < 0 && color) ? "RED": "");
    else
        this->font_settings(font_size_);
    this->bold_italic(value);
    this->font_end();
    this->endTableCell();
    this->endTableRow();
}

void mmHTMLBuilder::addTotalRow(const wxString& caption, int cols, double value, const bool color)
{
    this->addTotalRow(caption, cols, Model_Currency::toCurrency(value), color);
}

void mmHTMLBuilder::addTotalRow(const wxString& caption, int cols
    , const std::vector<wxString>& data, const bool color)
{
    html_+= wxString::Format(tags::TABLE_ROW, color_.bgcolor);
    html_+= wxString::Format(tags::TABLE_CELL_SPAN, cols - (int)data.size());
    this->font_settings(font_size_);
    this->bold_italic(tags::NBSP + tags::NBSP + caption);
    this->font_end();

    for (unsigned long idx = 0; idx < data.size(); idx++)
    {
        this->endTableCell();
        html_+= wxString::Format(tags::TABLE_CELL_RIGHT);
        double amount = 0;
        if (Model_Currency::fromString(data[idx], amount, 0))
            this->font_settings(font_size_, (amount < 0 && color) ? "RED": "");
        else
            this->font_settings(font_size_);
        this->bold_italic(data[idx]);
        this->font_end();
    }
    this->endTableCell();
    this->endTableRow();
}

void mmHTMLBuilder::addTotalRow(const wxString& caption, int cols, const std::vector<double>& data, const bool color)
{
    std::vector<wxString> data_str;
    for (const auto& value: data)
    {
        data_str.push_back(Model_Currency::toCurrency(value));
    }
    this->addTotalRow(caption, cols, data_str, color);
}

void mmHTMLBuilder::addTableHeaderRow(const wxString& value, int cols)
{
    html_+= wxString::Format(tags::TABLE_ROW, color_.bgcolor);
    html_+= wxString::Format(tags::TABLE_HEADER, "left", color_.table_header, cols);
    this->font_settings(font_size_, "white");
    this->bold(tags::NBSP + value);
    this->font_end();
    html_+= tags::TABLE_HEADER_END;
    this->endTableRow();
    color_.bgswitch = true;
}

void mmHTMLBuilder::addTableHeaderCell(const wxString& value, const bool& numeric)
{
    wxString align = numeric ? "right" : "left";
    html_+= wxString::Format(tags::TABLE_HEADER, align, color_.table_header, 0);
    this->font_settings(font_size_, "white");
    this->bold(value);
    this->font_end();
    html_+= tags::TABLE_HEADER_END;
    color_.bgswitch = false;
}

void mmHTMLBuilder::addCurrencyCell(double amount, const Model_Currency::Data* currency)
{
    wxString balance = Model_Currency::toCurrency(amount, currency);
    this->addTableCell(balance, true, true, true, (amount < 0) ? "RED" : "");
}
void mmHTMLBuilder::addMoneyCell(double amount, bool color)
{
    wxString balance = "";
    const Model_Currency::Data* currency = Model_Currency::GetBaseCurrency();
    if (currency)
        balance = Model_Currency::toString(amount, currency);
    else
        balance = Model_Currency::toString(amount);
    this->addTableCell(balance, true, true, true, (amount < 0 && color) ? "RED": "");
}

void mmHTMLBuilder::addMoneyCell(double amount, const wxString& color)
{
    wxString balance = Model_Currency::toCurrency(amount);
    this->addTableCell(balance, true, true, true, color);
}

void mmHTMLBuilder::addTableCell(const wxDateTime& date)
{
    wxString date_str = mmGetDateForDisplay(date);
    this->addTableCell(date_str, false);
}

void mmHTMLBuilder::addTableCell(const wxString& value
    , const bool& numeric, const bool& italic, const bool& bold, const wxString& fontColor)
{
    html_<< wxString::Format(tags::TABLE_CELL , (numeric ? "0%\" align=\"right" : "0%"));

    this->font_settings(font_size_, fontColor);

    if (!bold && !italic)    html_+= value;
    else if (bold && italic) this->bold_italic(value);
    else if (bold)           this->bold(value);
    else if (italic)         this->italic(value);

    this->font_end();
    this->endTableCell();
}

void mmHTMLBuilder::addTableCellMonth(int month)
{
    if (month >= 0 && month < 12)
        this->addTableCell(wxGetTranslation(wxDateTime::GetMonthName((wxDateTime::Month)month)));
    else
        this->addTableCell("");
}

void mmHTMLBuilder::addTableCellLink(const wxString& href
    , const wxString& value, const bool& numeric
    , const bool& italic, const bool& bold, const wxString& fontColor)
{
    addTableCell(wxString::Format(tags::TABLE_CELL_LINK, href, value )
        , numeric, italic, bold, fontColor);
}

void mmHTMLBuilder::addTableHeaderCellLink(const wxString& href, const wxString& value, const bool& numeric)
{
    addTableHeaderCell(wxString::Format(tags::LINK, href, value ), numeric);
}

void mmHTMLBuilder::addTableHeaderRowLink(const wxString& href
    , const wxString& value, int cols)
{
    addTableHeaderRow(wxString::Format(tags::LINK, href, value ), cols);
}

void mmHTMLBuilder::DisplayDateHeading(const wxDateTime& startYear, const wxDateTime& endYear, bool withDateRange)
{

    wxString todaysDate = "";
    if (withDateRange)
    {
        todaysDate << today_.todays_date << tags::BR << tags::BR
        << wxString::Format(_("From %s till %s")
            , mmGetNiceDateSimpleString(startYear).Prepend("<b>").Append("</b> ")
            , mmGetNiceDateSimpleString(endYear).Prepend("<b>").Append("</b> "));
    }
    else
    {
        todaysDate << _("Over Time");
    }
    this->addHeaderItalic(1, todaysDate);
    this->addLineBreak();
    this->addLineBreak();
}

wxString mmHTMLBuilder::getHTMLinTableWraper(bool indent)
{
    wxString html = this->getHTMLText();
    html_.clear();
    if (!html.IsEmpty())
    {
        if (indent)
        {
            startTable("100%", "", "0");
            startTableRow();
            addTableCell("");
            endTableRow();
            endTable();
        }
        html_ += wxString::Format(tags::TABLE_START, "#DDDDDD", "100%" ,"", "0");
        startTableRow();
        startTableCell();
        addText(html);
        endTableCell();
        endTableRow();
        endTable();
    }

    return this->getHTMLText();
}

void mmHTMLBuilder::addTableRow(const wxString& label, double data)
{
    this->startTableRow();
    this->addTableCell(label, false, true);
    this->addMoneyCell(data);
    this->endTableRow();
}
void mmHTMLBuilder::addTableRowBold(const wxString& label, double data)
{
    this->startTableRow();
    this->addTableCell(label, false, true, true);
    this->addMoneyCell(data);
    this->endTableRow();
}

void mmHTMLBuilder::end()
{
    html_+= tags::END;
};
void mmHTMLBuilder::endTable()
{
    html_+= tags::TABLE_END;
};
void mmHTMLBuilder::startTableRow()
{
    html_ += wxString::Format(tags::TABLE_ROW, (color_.bgswitch ? color_.color0 : color_.color1));
}
void mmHTMLBuilder::startTableRow(const wxString& custom_color)
{
    html_ += wxString::Format(tags::TABLE_ROW, custom_color);
}
void mmHTMLBuilder::endTableRow()
{
    html_+= tags::TABLE_ROW_END;
    color_.bgswitch = !color_.bgswitch;
}

void mmHTMLBuilder::addParaText(const wxString& text)
{
    html_+= wxString::Format(tags::PARAGRAPH, font_size_, text);
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

void mmHTMLBuilder::bold_italic(const wxString& value)
{
    html_+= wxString::Format(tags::BI, value);
}
void mmHTMLBuilder::bold(const wxString& value)
{
    html_+= wxString::Format(tags::BOLD, value);
}
void mmHTMLBuilder::italic(const wxString& value)
{
    html_+= wxString::Format(tags::ITALIC, value);
}
void mmHTMLBuilder::font_settings(int size, const wxString& color)
{
    html_+= wxString::Format(tags::FONT, size, color);
}
void mmHTMLBuilder::font_end()
{
    html_+= tags::FONT_END;
}
int mmHTMLBuilder::font_size()
{
    return font_size_;
}
/** Centers the content from this point on */
void mmHTMLBuilder::startCenter()
{
    html_+= tags::CENTER;
}
void mmHTMLBuilder::endCenter()
{
    html_+= tags::CENTER_END;
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

mm_html_template::mm_html_template(const wxString& arg_file_name): html_template(arg_file_name.ToStdString())
{
    this->load_context();
}

void mm_html_template::load_context()
{
    (*this)("TODAY") = wxDateTime::Today();
    for (const auto &r: Model_Infotable::instance().all())
        (*this)(r.INFONAME.ToStdString()) = r.INFOVALUE;
    (*this)("INFOTABLE") = Model_Infotable::to_loop_t();
}
