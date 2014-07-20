/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel

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

#ifndef MM_EX_HTMLBUILDER_H_
#define MM_EX_HTMLBUILDER_H_

#include "defs.h"
#include <vector>
#include "model/Model_Currency.h"
#include "html_template.h"

class mmHTMLBuilder
{
public:
    mmHTMLBuilder();
    ~mmHTMLBuilder() {}

    void DisplayDateHeading(const wxDateTime& startYear, const wxDateTime& endYear, bool withDateRange = true);

    /** Starts a new HMTL report */
    void init();

    /** Clears the current HTML document */
    void clear()
    {
        html_.clear();
    }

    /** Create an HTML header and returns as a wxString */
    void addHeader(int level, const wxString& header);
    void addDateNow();

    /** Create an HTML Image tag */
    void addImage(const wxString& src);

    /** Start a table element */
    void startTable();
    void startSortTable();
    void startThead();
    void startTbody();
    void startTfoot();

    /** Add a special row that will format total values */
    void addTotalRow(const wxString& caption, int cols, const wxString& value);
    void addTotalRow(const wxString& caption, int cols, double value);

    /** Add a special row that will format total values */
    void addTotalRow(const wxString& caption, int cols, const std::vector<wxString>& data);
    void addTotalRow(const wxString& caption, int cols, const std::vector<double>& data);

    /** Add a Table header cell */
    void addTableHeaderCell(const wxString& value, const bool& numeric = false);

    /** Add a Table header cell link */
    void addTableHeaderCellLink(const wxString& href, const wxString& value, const bool& numeric = false);

    void addCurrencyCell(double amount, const Model_Currency::Data *currency = Model_Currency::instance().GetBaseCurrency());
    void addMoneyCell(double amount);
    void addTableCellMonth(int month);

    /** Add a Cell value */
    void addTableCell(const wxDateTime& date);
    void addTableCell(const wxString& value);

    /** Add a Cell value */
    void addTableCellLink(const wxString& href, const wxString& value);

    void end();
    void endTable();
    void endThead();
    void endTbody();
    void endTfoot();
    void addDivContainer();
    void addDivRow();
    void addDivCol8();
    void endDiv();
    void startTableRow();
    void startTotalTableRow();
    //void startTableRow(const wxString& custom_color);
    void endTableRow();

    /** append an HTML text */
    void addText(const wxString& text);

    /** Create an HTML line break */
    void addLineBreak();
    /** Create an HTML HorizontalLine */
    void addHorizontalLine(int size = 0);

    /** Starts a table cell (use only if want to nest other elements inside */
    void startTableCell(const wxString& width = "0");
    void endTableCell();

    const wxString getHTMLText() const;

    void addTableRow(const wxString& label, double data);
    void addTableRowBold(const wxString& label, double data);

private:
    wxString html_;

    int font_size_;

    struct today_
    {
        wxDateTime date;
        wxString date_str;
        wxString todays_date;
    } today_;
};

class mm_html_template: public html_template
{
public:
    explicit mm_html_template(const wxString & arg_template);

private:
    void load_context();
};

#endif
