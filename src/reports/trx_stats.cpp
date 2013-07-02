
#include "trx_stats.h"
#include "htmlbuilder.h"
#include "../util.h"

mmReportTransactionStats::mmReportTransactionStats(mmCoreDB* core, int year)
: mmPrintableBase(core)
, year_(year)
{}

wxString mmReportTransactionStats::getHTMLText()
{
    const int yearsHist = 10; //How many years should show the report

    mmHTMLBuilder hb;
    hb.init();
    hb.addHeader(2, wxString::Format(_("Transaction Statistics for %i - %i")
        , year_ - yearsHist + 1, year_ )
    );
    hb.addDateNow();
    hb.addHorizontalLine();
    hb.startCenter();
    hb.startTable("75%");

    std::map<int, int> grand_total;
    std::map<wxDateTime::Month, std::map<int, int> > totals;
    core_->bTransactionList_.getTransactionStats(totals, year_ - yearsHist +1);

    //Header
    // Month 2014 2013 2012 2011 .....
    hb.startTableRow();
    hb.addTableHeaderCell(_("Month"));
    for (const auto &y : totals.begin()->second)
    {
        hb.addTableHeaderCell(wxString::Format("%i", y.first));
    }
    hb.endTableRow();
    //Table

    for (const auto & month_stat : totals)
    {
        wxDateTime::Month month = month_stat.first;
        //
        hb.startTableRow();
        hb.addTableCellMonth(month);
        // Totals for month
        for (const auto &y : month_stat.second )
        {
            int year = y.first;
            hb.addTableCell(wxString::Format("%i", y.second), true);
            grand_total[year] += y.second; //calculation totals for year
        }
        hb.endTableRow();
    }
    hb.addRowSeparator(yearsHist + 1);
    hb.startTableRow();
    hb.addTableCell(_("Total"), true, true);

    //Grand Totals
    for (const auto &grand_totals : grand_total)
    {
        hb.addTableCell(wxString::Format("%i", grand_totals.second), true, true);
    }
    //------------
    hb.endTable();
    hb.endCenter();
    hb.end();

    return hb.getHTMLText();
}
