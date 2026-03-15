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

#include "mmex.h"
#include "base/images_list.h"
#include "util/_util.h"
#include "htmlbuilder.h"

#include "mmframe.h"
#include "model/TrxModel.h"
#include "ForecastReport.h"

class mm_html_template;

ForecastReport::ForecastReport(): ReportBase(_n("Forecast"))
{
    setReportParameters(REPORT_ID::ForecastReport);
}

ForecastReport::~ForecastReport()
{
}

wxString ForecastReport::getHTMLText()
{
    // Grab the data
    std::map<wxString, std::pair<double, double>> amount_by_day;
    TrxModel::DataA trx_a;
    
    if (m_date_range && m_date_range->is_with_date()) {
        trx_a = TrxModel::instance().find(
            TrxModel::DATE(OP_GE, mmDate(m_date_range->start_date())),
            TrxModel::DATE(OP_LE, mmDate(m_date_range->end_date()))
        );
    }
    else {
        trx_a = TrxModel::instance().find_all();
    }

    for (const auto& trx_d : trx_a) {
        if (trx_d.is_transfer() || TrxModel::is_foreignAsTransfer(trx_d))
            continue;
        const double convRate = CurrencyHistoryModel::instance().get_id_date_rate(
            AccountModel::instance().get_id_data_n(trx_d.m_account_id)->m_currency_id,
            trx_d.m_date()
        );
        // FIXME: use only the date part
        amount_by_day[trx_d.m_date_time.isoDateTime()].first +=
            trx_d.account_outflow(trx_d.m_account_id) * convRate;
        amount_by_day[trx_d.m_date_time.isoDateTime()].second +=
            trx_d.account_inflow(trx_d.m_account_id) * convRate;
    }

    // Build the report
    mmHTMLBuilder hb;
    hb.init();
    hb.addReportHeader(getTitle(), m_date_range->startDay(), m_date_range->isFutureIgnored());
    hb.displayDateHeading(m_date_range);

    GraphData gd;
    GraphSeries gsWithdrawal, gsDeposit;
    for (const auto & kv : amount_by_day)
    {
        gd.labels.push_back(kv.first);
        //wxLogDebug(" Values = %d, %d", kv.second.first, kv.second.second);
        gsWithdrawal.values.push_back(kv.second.first);
        gsDeposit.values.push_back(kv.second.second);
    }
    gsDeposit.name = _t("Deposit");
    gd.series.push_back(gsDeposit);
    gsWithdrawal.name = _t("Withdrawal");
    gd.series.push_back(gsWithdrawal);

    gd.type = GraphData::LINE_DATETIME;
    gd.colors = { mmThemeMetaColour(meta::COLOR_REPORT_CREDIT)
                    , mmThemeMetaColour(meta::COLOR_REPORT_DEBIT) };
    hb.addChart(gd);

    hb.end();

    return hb.getHTMLText();
}
