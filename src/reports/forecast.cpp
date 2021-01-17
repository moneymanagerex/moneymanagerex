/*******************************************************
 Copyright (C) 2016 Guan Lisheng (guanlisheng@gmail.com)

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

#include "forecast.h"
#include "mmex.h"
#include "mmframe.h"
#include "util.h"
#include "reports/htmlbuilder.h"
#include "model/Model_Checking.h"

class mm_html_template;

mmReportForecast::mmReportForecast(): mmPrintableBase(wxTRANSLATE("Forecast"))
{
    setReportParameters(Reports::ForecastReport);
}

mmReportForecast::~mmReportForecast()
{
}

wxString mmReportForecast::getHTMLText()
{
    // Grab the data
    std::map<wxString, std::pair<double, double> > amount_by_day;
    Model_Checking::Data_Set all_trans;
    
    if (m_date_range && m_date_range->is_with_date()) {
        all_trans = Model_Checking::instance().find(DB_Table_CHECKINGACCOUNT_V1::TRANSDATE(m_date_range->start_date().FormatISODate(), GREATER_OR_EQUAL)
            , DB_Table_CHECKINGACCOUNT_V1::TRANSDATE(m_date_range->end_date().FormatISODate(), LESS_OR_EQUAL));
    }
    else {
        all_trans = Model_Checking::instance().all();
    }

    for (const auto & trx : all_trans)
    {
        if (Model_Checking::type(trx) == Model_Checking::TRANSFER || Model_Checking::foreignTransactionAsTransfer(trx))
            continue;

        amount_by_day[trx.TRANSDATE].first += Model_Checking::withdrawal(trx, -1);
        amount_by_day[trx.TRANSDATE].second += Model_Checking::deposit(trx, -1);
    }

    

    // Build the report
    mmHTMLBuilder hb;
    hb.init();
    hb.addDivContainer("shadowTitle");
    {
        hb.showUserName();
        hb.addHeader(2, getReportTitle());
        hb.DisplayDateHeading(m_date_range->start_date(), m_date_range->end_date(), m_date_range->is_with_date());
        hb.addReportCurrency();
        hb.addDateNow();
    }
    hb.endDiv();
       
    GraphData gd;
    GraphSeries gsWithdrawl, gsDeposit;
    for (const auto & kv : amount_by_day)
    {
        wxDate d;
        wxLogDebug("kv.first  = %s", kv.first); 
        d.ParseISODate(kv.first);
        wxString label = wxString::Format("%i %s %i", d.GetDay(), wxGetTranslation(wxDateTime::GetEnglishMonthName(d.GetMonth())), d.GetYear());
        wxLogDebug("label  = %s", label); 
        gd.labels.push_back(label);
        //wxLogDebug(" Values = %d, %d", kv.second.first, kv.second.second);
        gsWithdrawl.values.push_back(kv.second.first);
        gsDeposit.values.push_back(kv.second.second);
    }
    gsDeposit.name = _("Deposit");
    gd.series.push_back(gsDeposit);
    gsWithdrawl.name = _("Withdrawal");
    gd.series.push_back(gsWithdrawl);     

    hb.addDivContainer("shadow");
    { 
        gd.type = GraphData::LINE_DATETIME;
        gd.colors = { wxColour(0, 227, 150), wxColour(255, 69, 96) };  // Green, Red
        hb.addChart(gd);
    }
    hb.endDiv();

    hb.end();

    return hb.getHTMLText();
}