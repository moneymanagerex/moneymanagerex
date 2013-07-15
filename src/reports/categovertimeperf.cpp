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

//----------------------------------------------------------------------------
#include "categovertimeperf.h"
#include "htmlbuilder.h"
#include "util.h"

enum TYPE {INCOME = 0, EXPENCES, TOTAL, MAX};
static const wxString type_names[] = {_("Incomes"), _("Expences"), _("Total")};

//----------------------------------------------------------------------------

mmReportCategoryOverTimePerformance::mmReportCategoryOverTimePerformance(mmCoreDB *core
, mmDateRange* date_range) :
    mmPrintableBase(core)
    , date_range_(date_range)
    , title_(_("Category Income/Expenses: %s"))
{
    wxASSERT(core_);
}
//----------------------------------------------------------------------------
mmReportCategoryOverTimePerformance::~mmReportCategoryOverTimePerformance()
{
    if (date_range_) delete date_range_;
}

wxString mmReportCategoryOverTimePerformance::getHTMLText()
{
    const int MONTHS_IN_PERIOD = 12; // including current month

    mmHTMLBuilder hb;
    hb.init();

    hb.addHeader(2, wxString::Format(title_, date_range_->title()));
    hb.addDateNow();
    hb.addLineBreak();

    hb.startCenter();
    hb.startTable();

    //Add header
    hb.startTableRow();
    hb.addTableHeaderCell(_("Category"));
    wxDateTime start_date = date_range_->start_date();
    for (int i = 0; i < MONTHS_IN_PERIOD; i++)
    {
        wxDateTime d = wxDateTime(start_date).Add(wxDateSpan::Months(i));
        hb.addTableHeaderCell(wxGetTranslation(wxDateTime::GetEnglishMonthName(d.GetMonth(), wxDateTime::Name_Abbr))
            + wxString::Format("<br>%i", d.GetYear()));
    }
    hb.addTableHeaderCell(_("Overall"));
    hb.endTableRow();

    //Get statistic
    std::map<int, std::map<int, std::map<int, double> > > categoryStats;
    core_->bTransactionList_.getCategoryStats(categoryStats
        , date_range_
        , mmIniOptions::instance().ignoreFutureTransactions_);
    core_->currencyList_.LoadBaseCurrencySettings();

    //Init totals
    //Type(Withdrawal/Income/Summ), month, value
    std::map<int, std::map<int, double> > totals;
    double overall = 0;

    //Begin of table
    for (const auto& category: core_->categoryList_.entries_)
    {
        int categID = category->categID_;

        hb.startTableRow();
        hb.addTableCell(core_->categoryList_.GetFullCategoryString(categID, -1));
        overall = 0;
        for (const auto &i : categoryStats[categID][-1])
        {
            double value = i.second;
            hb.addMoneyCell(value);
            overall += value;
            totals[value<0][i.first] += value;
            totals[value>=0][i.first] += 0;
            totals[TOTAL][i.first] += value;
        }
        hb.addMoneyCell(overall);
        hb.endTableRow();

        for (const auto& sub_category: category->children_)
        {
            int subcategID = sub_category->categID_;
            hb.startTableRow();
            hb.addTableCell(core_->categoryList_.GetFullCategoryString(categID, subcategID));
            overall = 0;
            for (const auto &i : categoryStats[categID][subcategID])
            {
                double value = i.second;
                hb.addMoneyCell(value);
                overall += value;
                totals[value<0][i.first] += value;
                totals[value>=0][i.first] += 0;
                totals[TOTAL][i.first] += value;
            }
            hb.addMoneyCell(overall);
            hb.endTableRow();
        }
    }
    hb.addRowSeparator(MONTHS_IN_PERIOD+2);
    //Totals
    for (const auto& print_totals : totals)
    {
        hb.startTableRow();
        hb.addTableCell(type_names[print_totals.first]);
        overall = 0;
        for (const auto& range : totals[print_totals.first])
        {
            double amount = range.second;
            overall += amount;
            hb.addMoneyCell(amount);
        }
        hb.addMoneyCell(overall);
        hb.endTableRow();
    }
    hb.addRowSeparator(MONTHS_IN_PERIOD+2);
    hb.endTable();
    hb.endCenter();
    hb.end();
    return hb.getHTMLText();
}
//----------------------------------------------------------------------------
