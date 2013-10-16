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
#include "mmCurrencyFormatter.h"
#include "mmOption.h"
#include "constants.h"
#include "model/Model_Currency.h"

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
    color_.table_header = "#d5d6de";
    // init font size from config
    font_size_ = mmIniOptions::instance().html_font_size_;

    today_.date = wxDateTime::Now().GetDateOnly();
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
        if (mmIniOptions::instance().enableCustomLogo_)
        {
            startTableCell();
            addImage(mmIniOptions::instance().logoName_);
            endTableCell();
        }
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
        startTable("100%", "top", "1");
        endTable();
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
    if(CurrencyFormatter::formatCurrencyToDouble(value, amount))
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
    this->addTotalRow(caption, cols, CurrencyFormatter::float2Money(value), color);
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
        if(CurrencyFormatter::formatCurrencyToDouble(data[idx], amount))
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
        data_str.push_back(CurrencyFormatter::float2Money(value));
    }
    this->addTotalRow(caption, cols, data_str, color);
}

void mmHTMLBuilder::addTableHeaderRow(const wxString& value, int cols)
{
    html_+= wxString::Format(tags::TABLE_ROW, color_.bgcolor);
    html_+= wxString::Format(tags::TABLE_HEADER, "left", color_.table_header, cols);
    this->font_settings(font_size_);
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
    this->font_settings(font_size_);
    this->bold(value);
    this->font_end();
    html_+= tags::TABLE_HEADER_END;
    color_.bgswitch = false;
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
    wxString balance = CurrencyFormatter::float2Money(amount);
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
        startTable("100%", "", "1");
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
