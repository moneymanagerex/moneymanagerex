/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2017 James Higley

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
#include "Model_Currency.h"
#include "html_template.h"
#include "util.h"

class mmHTMLBuilder
{
public:
    mmHTMLBuilder();
    ~mmHTMLBuilder() {}

    void DisplayDateHeading(const wxDateTime& startDate, const wxDateTime& endDate, bool withDateRange = true);

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

    /** Start a table element */
    void startTable();
    void startSortTable();
    void startThead();
    void startTbody();
    void startTfoot();

    /** Add a special row that will format total values */
    void addTotalRow(const wxString& caption, int cols, double value);

    /** Add a special row that will format total values */
    void addTotalRow(const wxString& caption, int cols, const std::vector<wxString>& data);
    void addTotalRow(const wxString& caption, int cols, const std::vector<double>& data);

    /** Add a Table header cell */
    void addTableHeaderCell(const wxString& value, const bool numeric = false, const bool sortable = true, const int cols = 1, const bool center = false);

    void addCurrencyCell(double amount, const Model_Currency::Data *currency = Model_Currency::instance().GetBaseCurrency(), int precision = -1);
    void addMoneyCell(double amount, int precision = -1);
    void addTableCellMonth(int month);
    void addColorMarker(const wxString& color);
    const wxString getColor(int i);
    const wxString getRandomColor(bool positive);

    /** Add a Cell value */
    void addTableCellDate(const wxString& iso_date);
    void addTableCell(const wxString& value, const bool numeric = false, const bool center = false);
    void addEmptyTableCell(const int number = 1);

    /** Add a Cell value */
    void addTableCellLink(const wxString& href, const wxString& value);

    void end();
    void endTable();
    void endThead();
    void endTbody();
    void endTfoot();
    void addDivContainer();
    void addDivRow();
    void addDivCol17_67();
    void addDivCol25_50();
    void addDivCol6();
    void endDiv();
    void startTableRow();
    void startTableRow(const wxString& color);
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
    void startTableCell(const wxString& width = "");
    void endTableCell();

    const wxString getHTMLText() const;

    void addTableRow(const wxString& label, double data);

    void addRadarChart(std::vector<ValueTrio>& actData, std::vector<ValueTrio>& estData, const wxString& id, const int x = 300, const int y = 300);
    void addPieChart(std::vector<ValueTrio>& valueList, const wxString& id, const int x = 300, const int y = 300);
    void addLineChart(const std::vector<ValueTrio>& data, const wxString& id, const int index, const int x = 640, const int y = 256, bool pointDot = false, bool showGridLines = true, bool datasetFill = false);
    void addBarChart(const wxString & labels, const std::vector<ValueTrio>& data, const wxString& id, const int x = 192, const int y = 256);

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
