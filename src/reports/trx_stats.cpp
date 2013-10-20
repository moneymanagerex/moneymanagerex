
#include "trx_stats.h"
#include "htmlbuilder.h"
#include "util.h"
#include "model/Model_Checking.h"

mmReportTransactionStats::mmReportTransactionStats(int year)
: year_(year)
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
    getTransactionStats(totals, year_ - yearsHist +1);

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

void mmReportTransactionStats::getTransactionStats(std::map<wxDateTime::Month, std::map<int, int> > &stats, int start_year) const
{
    //Initialization
    int end_year = wxDateTime::Now().GetYear();
    for (wxDateTime::Month m = wxDateTime::Jan; m != wxDateTime::Inv_Month; m = wxDateTime::Month(m + 1))
    {
        std::map<int, int> month_stat;
        for (int y = start_year; y <= end_year; y++)
        {
            month_stat[y] = 0;
        }
        stats[m] = month_stat;
    }

    //Calculations
    Model_Checking::Data_Set transactions = Model_Checking::instance().all();
    for (const auto &trx : transactions)
    {
        if (Model_Checking::status(trx) == Model_Checking::VOID_)
            continue; // skip

        wxDateTime trx_date = Model_Checking::TRANSDATE(trx);
        if (trx_date.GetYear() < start_year)
            continue;
        if (trx_date.GetYear() > end_year)
            continue; //skip future dated transactions for next years

        stats[trx_date.GetMonth()][trx_date.GetYear()] += 1;
    }
}
