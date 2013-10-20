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

#ifndef _MM_EX_HTMLBUILDER_H_
#define _MM_EX_HTMLBUILDER_H_

#include "defs.h"
#include <vector>
#include "model/Model_Currency.h"

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

class mmHTMLBuilder
{
public:
    mmHTMLBuilder();
    ~mmHTMLBuilder() {}

    void DisplayDateHeading(const wxDateTime& startYear, const wxDateTime& endYear, bool withDateRange = true);

    /** Starts a new HMTL report */
    void init();

    /** Clears the current HTML document */
    virtual void clear()
    {
        html_.clear();
    }

    /** Create an HTML header and returns as a wxString */
    void addHeader(int level, const wxString& header);
    void addHeaderItalic(int level, const wxString& header);
    void addDateNow();

    /** Create an HTML Image tag */
    void addImage(const wxString& src);

    /** Start a table element */
    void startTable(const wxString& width = "0"
                                            , const wxString& valign = "left", const wxString& border = "0");

    /** Add a special row that is a separator, cols is the number of columns the row has to spread along */
    void addRowSeparator(int cols);

    /** Add a special row that will format total values */
    void addTotalRow(const wxString& caption, int cols, const wxString& value, const bool color = true);
    void addTotalRow(const wxString& caption, int cols, double value, const bool color = true);

    /** Add a special row that will format total values */
    void addTotalRow(const wxString& caption, int cols, const std::vector<wxString>& data, const bool color = true);
    void addTotalRow(const wxString& caption, int cols, const std::vector<double>& data, const bool color = true);

    /** Add a Table header cell */
    void addTableHeaderCell(const wxString& value, const bool& numeric = false);

    /** Add a Table header cell link */
    void addTableHeaderCellLink(const wxString& href, const wxString& value, const bool& numeric = false);

    /** Add a Table header row */
    void addTableHeaderRow(const wxString& value, int cols = 0);

    /** Add a Table header row with link */
    void addTableHeaderRowLink(const wxString& href, const wxString& value, int cols = 0);

    void addCurrencyCell(double amount, const Model_Currency::Data *currency = Model_Currency::instance().GetBaseCurrency());
    void addMoneyCell(double amount, bool color = true);
    void addMoneyCell(double amount, const wxString& color);
    void addTableCellMonth(int month);

    /** Add a Cell value */
    void addTableCell(const wxDateTime& date);
    void addTableCell(const wxString& value
                      , const bool& numeric = false
                              , const bool& italic = false
                                      , const bool& bold = false
                                              , const wxString& fontColor = "");

    /** Add a Cell value */
    void addTableCellLink(const wxString& href, const wxString& value
                          , const bool& numeric = false
                                  , const bool& italic = false
                                          , const bool& bold = false
                                                  , const wxString& fontColor = "");

    virtual void end()
    {
        html_+= tags::END;
    };
    virtual void endTable()
    {
        html_+= tags::TABLE_END;
    };
    virtual void startTableRow()
    {
        html_ += wxString::Format(tags::TABLE_ROW, (color_.bgswitch ? color_.color0 : color_.color1));
    }
    virtual void startTableRow(const wxString& custom_color)
    {
        html_ += wxString::Format(tags::TABLE_ROW, custom_color);
    }
    virtual void endTableRow()
    {
        html_+= tags::TABLE_ROW_END;
        color_.bgswitch = !color_.bgswitch;
    }

    /** Create an HTML paragrapth */
    virtual void addParaText(const wxString& text)
    {
        html_+= wxString::Format(tags::PARAGRAPH, font_size_, text);
    }
    virtual void addText(const wxString& text)
    {
        html_+= text;
    }

    /** Create an HTML line break */
    virtual void addLineBreak()
    {
        html_+= tags::BR;
    }

    /** Create an HTML HorizontalLine */
    virtual void addHorizontalLine(int size = 0)
    {
        html_+= wxString::Format(tags::HOR_LINE, size);
    }

    /** Starts a table cell (use only if want to nest other elements inside */
    virtual void startTableCell(const wxString& width = "0")
    {
        html_+= wxString::Format(tags::TABLE_CELL, width);
    }
    virtual void endTableCell()
    {
        html_+= tags::TABLE_CELL_END;
    }

    virtual void bold_italic(const wxString& value)
    {
        html_+= wxString::Format(tags::BI, value);
    }
    virtual void bold(const wxString& value)
    {
        html_+= wxString::Format(tags::BOLD, value);
    }
    virtual void italic(const wxString& value)
    {
        html_+= wxString::Format(tags::ITALIC, value);
    }
    virtual void font_settings(int size, const wxString& color = "")
    {
        html_+= wxString::Format(tags::FONT, size, color);
    }
    virtual void font_end()
    {
        html_+= tags::FONT_END;
    }
    virtual int font_size()
    {
        return font_size_;
    }
    /** Centers the content from this point on */
    virtual void startCenter()
    {
        html_+= tags::CENTER;
    }
    virtual void endCenter()
    {
        html_+= tags::CENTER_END;
    }
    virtual const wxString getHTMLText() const
    {
        return html_;
    }
    wxString getHTMLinTableWraper(bool indent = false);

    void addTableRow(const wxString& label, double data);
    void addTableRowBold(const wxString& label, double data);

private:
    wxString html_;

    int font_size_;
    struct color_
    {
        wxString color0;
        wxString color1;
        wxString bgcolor;
        wxString text;
        wxString table_header;
        wxString link;
        wxString vlink;
        wxString alink;
        bool bgswitch;
    } color_;
    struct today_
    {
        wxDateTime date;
        wxString date_str;
        wxString todays_date;
    } today_;
};

#endif
