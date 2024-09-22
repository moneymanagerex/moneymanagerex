/*******************************************************
 Copyright (C) 2016 Guan Lisheng (guanlisheng@gmail.com)
 Copyright (C) 2021 Mark Whalley (mark@ipx.co.uk)

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

#include "images_list.h"
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
        all_trans = Model_Checking::instance().find(Model_Checking::TRANSDATE(m_date_range->start_date(), GREATER_OR_EQUAL),
                                                    Model_Checking::TRANSDATE(m_date_range->end_date().FormatISOCombined(), LESS_OR_EQUAL));
    }
    else {
        all_trans = Model_Checking::instance().all();
    }

    for (const auto & trx : all_trans)
    {
        if (Model_Checking::type_id(trx) == Model_Checking::TYPE_ID_TRANSFER || Model_Checking::foreignTransactionAsTransfer(trx))
            continue;
        const double convRate = Model_CurrencyHistory::getDayRate(Model_Account::instance().get(trx.ACCOUNTID)->CURRENCYID, trx.TRANSDATE);
        amount_by_day[trx.TRANSDATE].first += Model_Checking::account_outflow(trx, trx.ACCOUNTID) * convRate;
        amount_by_day[trx.TRANSDATE].second += Model_Checking::account_inflow(trx, trx.ACCOUNTID) * convRate;
    }

    

    // Build the report
    mmHTMLBuilder hb;
    hb.init();
    hb.addReportHeader(getReportTitle(), m_date_range->startDay(), m_date_range->isFutureIgnored());
    hb.DisplayDateHeading(m_date_range->start_date(), m_date_range->end_date(), m_date_range->is_with_date());

    GraphData gd;
    GraphSeries gsWithdrawal, gsDeposit;
    for (const auto & kv : amount_by_day)
    {
        gd.labels.push_back(kv.first);
        //wxLogDebug(" Values = %d, %d", kv.second.first, kv.second.second);
        gsWithdrawal.values.push_back(kv.second.first);
        gsDeposit.values.push_back(kv.second.second);
    }
    gsDeposit.name = _("Deposit");
    gd.series.push_back(gsDeposit);
    gsWithdrawal.name = _("Withdrawal");
    gd.series.push_back(gsWithdrawal);

    gd.type = GraphData::LINE_DATETIME;
    gd.colors = { mmThemeMetaColour(meta::COLOR_REPORT_CREDIT)
                    , mmThemeMetaColour(meta::COLOR_REPORT_DEBIT) };
    hb.addChart(gd);

    hb.end();

    return hb.getHTMLText();
}
