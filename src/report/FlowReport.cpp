/*******************************************************
Copyright (C) 2012 Guan Lisheng (guanlisheng@gmail.com)
Copyright (C) 2017 James Higley
Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)

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

#include "FlowReport.h"

#include "base/_defs.h"
#include "util/_util.h"
#include "model/AccountModel.h"
#include "model/SchedModel.h"
#include "model/CurrencyHistoryModel.h"
#include "htmlbuilder.h"
#include "app/mmFrame.h"

// -- CashFlow base class

FlowReport::FlowReport(const wxString& name) :
    ReportBase(name),
    m_today(wxDateTime::Now().ResetTime())
{
    m_only_active = true;
}

FlowReport::~FlowReport()
{
}

double FlowReport::trueAmount(const TrxData& trx_d)
{
    double amount = 0.0;
    bool isAccountFound = std::find(m_account_id.begin(), m_account_id.end(),
        trx_d.m_account_id
    ) != m_account_id.end();
    bool isToAccountFound = std::find(m_account_id.begin(), m_account_id.end(),
        trx_d.m_to_account_id_n
    ) != m_account_id.end();
    if (!(isAccountFound && isToAccountFound)) {
        const double convRate = CurrencyHistoryModel::instance().get_id_date_rate(
            AccountModel::instance().get_idN_data_n(trx_d.m_account_id)->m_currency_id,
            trx_d.m_date()
        );
        switch (trx_d.m_type.id()) {
        case TrxType::e_withdrawal:
            amount = -trx_d.m_amount * convRate;
            break;
        case TrxType::e_deposit:
            amount = +trx_d.m_amount * convRate;
            break;
        case TrxType::e_transfer:
            if (isAccountFound)
                amount = -trx_d.m_amount * convRate;
            else {
                const double toConvRate = CurrencyHistoryModel::instance().get_id_date_rate(
                    AccountModel::instance().get_idN_data_n(trx_d.m_to_account_id_n)->m_currency_id,
                    trx_d.m_date()
                );
                amount = +trx_d.m_to_amount * toConvRate;
            }
        }
    }
    return amount;
}

void FlowReport::getTransactions()
{
    m_balance = 0.0;
    m_account_id.clear();
    m_forecastVector.clear();

    wxDateTime endOfToday = mmDateRange::getDayEnd(m_today);
    wxString todayString = endOfToday.FormatISOCombined();
    wxDateTime endDate = mmDateRange::getDayEnd(
        m_today.Add(wxDateSpan::Months(getForwardMonths()))
    );

    // Get initial Balance as of today
    for (const auto& account_d : AccountModel::instance().find_data_a(
        AccountCol::WHERE_ACCOUNTTYPE(OP_NE, mmNavigatorList::instance().getInvestmentAccountStr()),
        AccountModel::WHERE_STATUS(OP_NE, AccountStatus(AccountStatus::e_closed))
    )) {
        if (m_account_a &&
            std::find(m_account_a->begin(), m_account_a->end(), account_d.m_name) ==
                m_account_a->end()
        ) {
            continue;
        }

        double convRate = CurrencyHistoryModel::instance().get_id_date_rate(
            account_d.m_currency_id
        );
        m_balance += account_d.m_open_balance * convRate;

        m_account_id.push_back(account_d.m_id);

        for (const auto& trx_d : AccountModel::instance().find_id_trx_aBySN(account_d.m_id)) {
            wxString strDate = trx_d.m_isoDateTime();
            // Do not include asset or stock transfers in income expense calculations.
            if (TrxModel::is_foreignAsTransfer(trx_d) || (strDate > todayString))
                continue;
            m_balance += trx_d.account_flow(account_d.m_id) * convRate;
        }
    }

    // Process all transations posted after today
    for (TrxData& trx_d : TrxModel::instance().find_data_a(
        TrxModel::WHERE_DATE(OP_GT, mmDate(endOfToday)),
        TrxModel::WHERE_DATE(OP_LT, mmDate(endDate)),
        TrxModel::WHERE_IS_VOID(false),
        TrxModel::WHERE_IS_DELETED(false)
    )) {
        bool isAccountFound = std::find(m_account_id.begin(), m_account_id.end(),
            trx_d.m_account_id
        ) != m_account_id.end();
        bool isToAccountFound = std::find(m_account_id.begin(), m_account_id.end(),
            trx_d.m_to_account_id_n
        ) != m_account_id.end();
        if (!isAccountFound && !isToAccountFound)
            continue;
        const auto& tp_a = TrxModel::instance().find_id_tp_a(trx_d.m_id);
        if (tp_a.empty()) {
            trx_d.m_amount = trueAmount(trx_d);
            m_forecastVector.push_back(trx_d);
        }
        else {
            for (const auto& tp_d : tp_a) {
                trx_d.m_category_id_n = tp_d.m_category_id;
                trx_d.m_amount        = tp_d.m_amount;
                trx_d.m_amount        = trueAmount(trx_d);
                m_forecastVector.push_back(trx_d);
            }
        }
    }

    // Gather the recurring transaction list
    for (const auto& sched_d : SchedModel::instance().find_data_a(
        SchedModel::WHERE_IS_VOID(false)
    )) {
        // CHECK: use m_date() instead of m_due_date
        mmDate next_date = sched_d.m_due_date;
        if (next_date > mmDate(endDate))
            continue;

        bool isAccountFound = std::find(m_account_id.begin(), m_account_id.end(),
            sched_d.m_account_id
        ) != m_account_id.end();
        bool isToAccountFound = std::find(m_account_id.begin(), m_account_id.end(),
            sched_d.m_to_account_id_n
        ) != m_account_id.end();
        if (!isAccountFound && !isToAccountFound)
            continue;

        Repeat repeat = sched_d.m_repeat;

        // Process all possible recurring transactions for this BD
        while (1) {
            if (next_date > mmDate(endDate))
                break;

            TrxData trx_d;
            trx_d.m_datetime        = mmDateTime(next_date);
            trx_d.m_type            = sched_d.m_type;
            trx_d.m_account_id      = sched_d.m_account_id;
            trx_d.m_to_account_id_n = sched_d.m_to_account_id_n;
            trx_d.m_payee_id_n      = sched_d.m_payee_id_n;
            trx_d.m_amount          = sched_d.m_amount;
            trx_d.m_to_amount       = sched_d.m_to_amount;

            const SchedSplitModel::DataA qp_a = SchedModel::instance().find_id_qp_a(
                sched_d.m_id
            );
            if (!qp_a.empty()) {
                for (const auto& qp_d : qp_a) {
                    trx_d.m_category_id_n = qp_d.m_category_id;
                    trx_d.m_amount        = qp_d.m_amount;
                    trx_d.m_amount        = trueAmount(trx_d);
                    m_forecastVector.push_back(trx_d);
                }
            }
            else {
                trx_d.m_category_id_n = sched_d.m_category_id_n;
                trx_d.m_amount        = trueAmount(trx_d);
                m_forecastVector.push_back(trx_d);
            }

            if (repeat.m_num == 1)
                break;

            next_date = repeat.next_date(next_date);
            repeat.next_repeat();
        }
    }

    // Sort by transaction date
    sort(
        m_forecastVector.begin(), m_forecastVector.end(),
        [](TrxData const& a, TrxData const& b) {
            return a.m_datetime < b.m_datetime;
        }
    );
}

wxString FlowReport::getHTMLText_DayOrMonth(bool monthly)
{
    // Grab the data
    getTransactions();

    std::map<wxString, double> dateMap;

    // Initialise data
    wxDateTime dt = m_today;
    wxDateTime et = dt;
    et.Add(wxDateSpan::Months(getForwardMonths()));
    if (monthly)
        dt.SetDay(1);

    while(dt.IsEarlierThan(et)) {
        dateMap[dt.FormatISODate()] = 0;
        if (monthly)
            dt.Add(wxDateSpan::Month());
        else
            dt.Add(wxDateSpan::Day());
    }

    // squash the data by month or day
    for (TrxData& trx_d : m_forecastVector) {
        dt = trx_d.m_datetime.dateTime();
        wxString date = dt.FormatISODate();
        if (monthly) {
            date = dt.SetDay(1).FormatISODate();
        }
        dateMap[date] += trx_d.m_amount;
    }

    // Build the report
    mmHTMLBuilder hb;
    hb.init();
    const wxString& headingStr = wxString::Format(
        wxPLURAL("%1$s (%2$i month)", "%1$s (%2$i months)", getForwardMonths()),
        getTitle(), getForwardMonths()
    );
    hb.addReportHeader(headingStr, 1, false);
    hb.displayFooter(getAccountNames());

    GraphData gd;
    GraphSeries gs;

    double runningBalance = m_balance;
    for (const auto& entry : dateMap) {
        runningBalance += entry.second;
        gs.values.push_back(runningBalance);
        gd.labels.push_back(entry.first);
    }
    gd.series.push_back(gs);

    if (getChartSelection() == 0 && !gd.series.empty())
    {
        gd.type = GraphData::LINE_DATETIME;
        hb.addChart(gd);
    }

    hb.addDivContainer("shadow");
    {
        hb.startTable();
        {
            hb.startThead();
            {
                hb.startTableRow();
                hb.addTableHeaderCell(_t("Date"));
                hb.addTableHeaderCell(_t("Total"), "text-right");
                hb.addTableHeaderCell(_t("Difference"), "text-right");
                hb.addTableHeaderCell(_t("Cumulative Difference"), "text-right");
                hb.endTableRow();
            }
            hb.endThead();

            runningBalance = m_balance;
            hb.startTbody();
            {
                int lastRowDate = -1;
                int rowDate;
                bool rowType = false;
                double lastBalance = runningBalance;
                for (const auto& entry : dateMap)
                {
                    dt.ParseISODate(entry.first);
                    if (monthly)
                        rowDate = dt.GetYear();
                    else
                        rowDate = dt.GetMonth();
                    if (rowDate != lastRowDate)
                    {
                        lastRowDate = rowDate;
                        rowType = !rowType;
                    }
                    if (rowType)
                        hb.startTableRow();
                    else
                        hb.startAltTableRow();

                    hb.addTableCell(entry.first);
                    runningBalance += entry.second;
                    hb.addMoneyCell(runningBalance);
                    hb.addMoneyCell(runningBalance - lastBalance);
                    hb.addMoneyCell(runningBalance - m_balance);
                    lastBalance = runningBalance;
                    hb.endTableRow();
                }
            }
            hb.endTbody();
        }
        hb.endTable();
    }
    hb.endDiv();

    hb.end();

    wxLogDebug("======= FlowReport:getHTMLText =======");
    wxLogDebug("%s", hb.getHTMLText());

    return hb.getHTMLText();
}

//--------- Cash Flow - Daily

mmReportCashFlowDaily::mmReportCashFlowDaily()
    : FlowReport(_n("Cash Flow - Daily"))
{
    this->setForwardMonths(12);
    setReportParameters(REPORT_ID::DailyCashFlow);
}

wxString mmReportCashFlowDaily::getHTMLText()
{
    return getHTMLText_DayOrMonth(false);
}

//--------- Cash Flow - Monthly

mmReportCashFlowMonthly::mmReportCashFlowMonthly()
    : FlowReport(_n("Cash Flow - Monthly"))
{
    this->setForwardMonths(120);
    setReportParameters(REPORT_ID::MonthlyCashFlow);
}

wxString mmReportCashFlowMonthly::getHTMLText()
{
    return getHTMLText_DayOrMonth(true);
}

//--------- Cash Flow - Transactions

mmReportCashFlowTransactions::mmReportCashFlowTransactions()
    : FlowReport(_n("Cash Flow - Transactions"))
{
    setReportParameters(REPORT_ID::TransactionsCashFlow);
}

wxString mmReportCashFlowTransactions::getHTMLText()
{
    // Grab the data
    getTransactions();

    // Build the report
    mmHTMLBuilder hb;
    hb.init();
    const wxString& headingStr = wxString::Format(
        wxPLURAL("%1$s (%2$i month)", "%1$s (%2$i months)" , getForwardMonths()),
        getTitle(), getForwardMonths()
    );
    hb.addReportHeader(headingStr, 1, false);
    hb.displayFooter(getAccountNames());

    // Display graph
    GraphData gd;
    GraphSeries gs;

    double runningBalance = m_balance;
    for (const auto& trx_d : m_forecastVector) {
        runningBalance += trx_d.m_amount;
        gs.values.push_back(runningBalance);
        gd.labels.push_back(trx_d.m_isoDateTime());
    }
    gd.series.push_back(gs);

    if (getChartSelection() == 0 && !gd.series.empty()) {
        gd.type = GraphData::LINE_DATETIME;
        hb.addChart(gd);
    }

    // Now display the  transaction detail

    hb.addDivContainer("shadow");
    hb.startTable();
    hb.startThead();
    hb.startTableRow();
    hb.addTableHeaderCell(_t("Date"));
    hb.addTableHeaderCell(_t("Account"));
    hb.addTableHeaderCell(_t("Payee"));
    hb.addTableHeaderCell(_t("Category"));
    hb.addTableHeaderCell(_t("Amount"), "text-right");
    hb.addTableHeaderCell(_t("Balance"), "text-right");
    hb.addTableHeaderCell(_t("Cumulative Difference"), "text-right");
    hb.endTableRow();
    hb.endThead();
    hb.startTbody();

    int lastRowDate = -1;
    bool rowType = false;
    runningBalance = m_balance;
    for (TrxData& trx_d : m_forecastVector) {
        int rowDate = trx_d.m_datetime.dateTime(false).GetMonth();
        if (rowDate != lastRowDate) {
            lastRowDate = rowDate;
            rowType = !rowType;
        }
        if (rowType)
            hb.startTableRow();
        else
            hb.startAltTableRow();
        hb.addTableCellDate(trx_d.m_isoDateTime());
        hb.addTableCell(AccountModel::instance().get_id_name(trx_d.m_account_id));
        hb.addTableCell((trx_d.m_to_account_id_n == -1)
            ? PayeeModel::instance().get_id_name(trx_d.m_payee_id_n)
            : "> " + AccountModel::instance().get_id_name(trx_d.m_to_account_id_n)
        );
        hb.addTableCell(CategoryModel::instance().get_id_fullname(trx_d.m_category_id_n));
        double amount = trx_d.m_amount;
        hb.addMoneyCell(amount);
        runningBalance += amount;
        hb.addMoneyCell(runningBalance);
        hb.addMoneyCell(runningBalance - m_balance);
        hb.endTableRow();
    }

    hb.endTbody();
    hb.endTable();
    hb.endDiv();
    hb.end();

    wxLogDebug("======= mmReportCashFlowTransactions:getHTMLText =======");
    wxLogDebug("%s", hb.getHTMLText());
    return hb.getHTMLText();
}
