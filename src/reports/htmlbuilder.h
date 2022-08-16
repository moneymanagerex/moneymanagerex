/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2012 - 2021 Nikolay Akimov
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

#ifndef MM_EX_HTMLBUILDER_H_
#define MM_EX_HTMLBUILDER_H_

#include "defs.h"
#include <vector>
#include "model/Model_Currency.h"
#include "html_template.h"
#include "util.h"

struct GraphSeries
{
    wxString name;
    wxString type;
    std::vector<double> values;
};

struct GraphData
{
    wxString title;
    enum { BAR = 0, LINE, LINE_DATETIME, PIE, DONUT, RADAR, BARLINE, STACKEDBARLINE, STACKEDAREA } type;
    std::vector<wxString> labels;
    std::vector<GraphSeries> series;
    std::vector<wxColour> colors;
};

class mmHTMLBuilder
{
public:
    mmHTMLBuilder();
    ~mmHTMLBuilder() {}

    void DisplayDateHeading(const wxDateTime& startDate, const wxDateTime& endDate, bool withDateRange = true, bool withNoEndDate = false);
    void DisplayFooter(const wxString& footer);
    /** Starts a new HMTL report */
    void init(bool simple = false, const wxString& extra_style = "");

    /** Clears the current HTML document */
    void clear()
    {
        html_.clear();
    }

    /** Add an HTML header */
    void addReportHeader(const wxString& name, int startDay = 1, bool futureIgnored = false);
    void addHeader(int level, const wxString& header);
    void showUserName();
    void addReportCurrency();
    void addDateNow();
    void addOffsetIndication(int startDay);
    void addFutureIgnoredIndication(bool ignore);

    /** Start a table element */
    void startTable();
    void startSortTable();
    void startThead();
    void startTbody();
    void startTfoot();

    /** Add a special row that will format total values */
    void addTotalRow(const wxString& caption, int cols, double value);

    /** Add a special rows that will format total values */
    void addTotalRow(const wxString& caption, int cols, const std::vector<wxString>& data);
    void addCurrencyTotalRow(const wxString& caption, int cols, const std::vector<double>& data);
    void addMoneyTotalRow(const wxString& caption, int cols, const std::vector<double>& data);

    /** Add a Table header cell */
    //void addTableHeaderCell(const wxString& value, bool numeric = false, bool sortable = true, int cols = 1, bool center = false);
    void addTableHeaderCell(const wxString& value, const wxString& css_class = "", int cols = 1);

    void addCurrencyCell(double amount, const Model_Currency::Data *currency = Model_Currency::instance().GetBaseCurrency(), int precision = -1, bool isVoid = false);
    void addMoneyCell(double amount, int precision = -1);
    void addTableCellMonth(int month, int year = 0);
    void addColorMarker(const wxString& color);
    const wxString getColor(int i);
    const wxString getRandomColor(bool positive);

    /** Add a Cell value */
    void addTableCellDate(const wxString& iso_date);
    void addTableCell(const wxString& value, bool numeric = false, bool center = false);
    void addEmptyTableCell(const int number = 1);

    /** Add a Cell value */
    void addTableCellLink(const wxString& href, const wxString& value);

    void end(bool simple=false);
    void endTable();
    void endThead();
    void endTbody();
    void endTfoot();
    void addDivContainer(const wxString& style = "container");
    void addDivRow();
    void addDivCol17_67();
    void addDivCol25_50();
    void addDivCol8_84();
    void endDiv();
    void addEmptyTableRow(int cols);
    void startTableRow();
    void startTableRow(const wxString& classname);
    void startTableRowColor(const wxString& color);
    void startAltTableRow();
    void startTotalTableRow();
    //void startTableRow(const wxString& custom_color);
    void endTableRow();
    void startSpan(const wxString& val, const wxString& style);
    void endSpan();

    /** append an HTML text */
    void addText(const wxString& text);

    /** Create an HTML line break */
    void addLineBreak();
    /** Create an HTML HorizontalLine */
    void addHorizontalLine(int size = 0);

    /** Starts a table cell (use only if want to nest other elements inside */
    void startTableCell(const wxString& width = "");
    void endTableCell();

    const wxString getHTMLText() const;

    void addTableRow(const wxString& label, double data);
    void addTableRowBold(const wxString& label, double data);

    void addChart(const GraphData& data);

private:
    wxString html_;
    struct today_
    {
        wxDateTime date;
        wxString date_str;
        wxString todays_date;
    } today_;
};


#endif
