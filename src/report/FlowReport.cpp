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

#include "base/defs.h"
#include "mmex.h"
#include "util/_util.h"
#include "htmlbuilder.h"

#include "model/AccountModel.h"
#include "model/ScheduledModel.h"
#include "model/CurrencyHistoryModel.h"
#include "mmframe.h"
#include "FlowReport.h"

// --------- CashFlow base class

FlowReport::FlowReport(const wxString& name)
    : ReportBase(name), m_today(wxDateTime::Now().ResetTime())
{
    m_only_active = true;
}

FlowReport::~FlowReport()
{
}

double FlowReport::trueAmount(const TransactionModel::Data& trx)
{
    double amount = 0.0;
    bool isAccountFound = std::find(m_account_id.begin(), m_account_id.end(), trx.ACCOUNTID) != m_account_id.end();
    bool isToAccountFound = std::find(m_account_id.begin(), m_account_id.end(), trx.TOACCOUNTID) != m_account_id.end();
    if (!(isAccountFound && isToAccountFound))
    {
        const double convRate = CurrencyHistoryModel::getDayRate(AccountModel::instance().cache_id(trx.ACCOUNTID)->CURRENCYID, trx.TRANSDATE);
        switch (TransactionModel::type_id(trx.TRANSCODE)) {
        case TransactionModel::TYPE_ID_WITHDRAWAL:
            amount = -trx.TRANSAMOUNT * convRate;
            break;
        case TransactionModel::TYPE_ID_DEPOSIT:
            amount = +trx.TRANSAMOUNT * convRate;
            break;
        case TransactionModel::TYPE_ID_TRANSFER:
            if (isAccountFound)
                amount = -trx.TRANSAMOUNT * convRate;
            else
            {
                const double toConvRate = CurrencyHistoryModel::getDayRate(AccountModel::instance().cache_id(trx.TOACCOUNTID)->CURRENCYID, trx.TRANSDATE);
                amount = +trx.TOTRANSAMOUNT * toConvRate;
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
    wxDateTime endDate = mmDateRange::getDayEnd(m_today.Add(wxDateSpan::Months(getForwardMonths())));

    // Get initial Balance as of today
    for (const auto& account : AccountModel::instance().find(
        AccountModel::ACCOUNTTYPE(OP_NE, NavigatorTypes::instance().getInvestmentAccountStr()),
        AccountModel::STATUS(OP_NE, AccountModel::STATUS_ID_CLOSED)
    )) {
        if (m_account_a &&
            std::find(m_account_a->begin(), m_account_a->end(), account.ACCOUNTNAME) ==
                m_account_a->end()
        ) {
            continue;
        }

        double convRate = CurrencyHistoryModel::getDayRate(account.CURRENCYID, todayString);
        m_balance += account.INITIALBAL * convRate;

        m_account_id.push_back(account.ACCOUNTID);

        for (const auto& tran : AccountModel::transactionsByDateTimeId(account)) {
            wxString strDate = TransactionModel::getTransDateTime(tran).FormatISOCombined();
            // Do not include asset or stock transfers in income expense calculations.
            if (TransactionModel::foreignTransactionAsTransfer(tran) || (strDate > todayString))
                continue;
            m_balance += TransactionModel::account_flow(tran, account.ACCOUNTID) * convRate;
        }
    }

    // Now gather all transations posted after today
    TransactionModel::Data_Set transactions = TransactionModel::instance().find(
        TransactionModel::TRANSDATE(OP_GT, endOfToday),
        TransactionModel::TRANSDATE(OP_LT, endDate),
        TransactionModel::STATUS(OP_NE, TransactionModel::STATUS_ID_VOID)
    );
    for (auto& trx : transactions) {
        if (!trx.DELETEDTIME.IsEmpty()) continue;
        bool isAccountFound = std::find(m_account_id.begin(), m_account_id.end(), trx.ACCOUNTID) != m_account_id.end();
        bool isToAccountFound = std::find(m_account_id.begin(), m_account_id.end(), trx.TOACCOUNTID) != m_account_id.end();
        if (!isAccountFound && !isToAccountFound)
            continue; // skip account
        if (TransactionModel::is_split(trx)) {
            TransactionModel::Data *transaction = TransactionModel::instance().cache_id(trx.TRANSID);
            for (const auto& split_item : TransactionModel::split(transaction)) {
                trx.CATEGID = split_item.CATEGID;
                trx.TRANSAMOUNT = split_item.SPLITTRANSAMOUNT;
                trx.TRANSAMOUNT = trueAmount(trx);
                m_forecastVector.push_back(trx);
            }
        }
        else {
            trx.TRANSAMOUNT = trueAmount(trx);
            m_forecastVector.push_back(trx);
        }
    }

    // Now we gather the recurring transaction list
    for (const auto& entry : ScheduledModel::instance().find(
        ScheduledModel::STATUS(OP_NE, TransactionModel::STATUS_ID_VOID)
    )) {
        wxDateTime nextOccurDate = ScheduledModel::NEXTOCCURRENCEDATE(entry);
        if (nextOccurDate > endDate) continue;

        // demultiplex entry.REPEATS
        int repeats = entry.REPEATS.GetValue() % BD_REPEATS_MULTIPLEX_BASE;
        int numRepeats = entry.NUMOCCURRENCES.GetValue();

        // ignore old inactive entries
        if (repeats >= ScheduledModel::REPEAT_IN_X_DAYS && repeats <= ScheduledModel::REPEAT_EVERY_X_MONTHS && numRepeats == -1)
            continue;

        // ignore invalid entries
        if (repeats != ScheduledModel::REPEAT_ONCE && (numRepeats == 0 || numRepeats < -1))
            continue;

        bool isAccountFound = std::find(m_account_id.begin(), m_account_id.end(), entry.ACCOUNTID) != m_account_id.end();
        bool isToAccountFound = std::find(m_account_id.begin(), m_account_id.end(), entry.TOACCOUNTID) != m_account_id.end();
        if (!isAccountFound && !isToAccountFound)
            continue; // skip account

        // Process all possible recurring transactions for this BD
        while (1) {
            if (nextOccurDate > endDate) break;

            TransactionModel::Data trx;
            trx.TRANSDATE = nextOccurDate.FormatISODate();
            trx.ACCOUNTID = entry.ACCOUNTID;
            trx.TOACCOUNTID = entry.TOACCOUNTID;
            trx.PAYEEID =  entry.PAYEEID;
            trx.TRANSCODE = entry.TRANSCODE;
            trx.TRANSAMOUNT = entry.TRANSAMOUNT;
            trx.TOTRANSAMOUNT = entry.TOTRANSAMOUNT;
            if (!ScheduledModel::split(entry).empty()) {
                for (const auto& split_item : ScheduledModel::split(entry)) {
                    trx.CATEGID = split_item.CATEGID;
                    trx.TRANSAMOUNT = split_item.SPLITTRANSAMOUNT;
                    trx.TRANSAMOUNT = trueAmount(trx);
                    m_forecastVector.push_back(trx);
                }
            }
            else {
                trx.CATEGID = entry.CATEGID;
                trx.TRANSAMOUNT = trueAmount(trx);
                m_forecastVector.push_back(trx);
            }

            if ((repeats == ScheduledModel::REPEAT_ONCE) || ((repeats < ScheduledModel::REPEAT_IN_X_DAYS || repeats > ScheduledModel::REPEAT_EVERY_X_MONTHS) && numRepeats == 1))
                break;

            nextOccurDate = ScheduledModel::nextOccurDate(repeats, numRepeats, nextOccurDate);

            if ((repeats < ScheduledModel::REPEAT_IN_X_DAYS || repeats > ScheduledModel::REPEAT_EVERY_X_MONTHS) &&
                numRepeats > 1
            ) {
                numRepeats--;
            }
            else if (repeats >= ScheduledModel::REPEAT_IN_X_DAYS &&
                repeats <= ScheduledModel::REPEAT_IN_X_MONTHS
            ) {
                // change repeat type to REPEAT_ONCE
                repeats = ScheduledModel::REPEAT_ONCE;
            }
        }
    }

    // Sort by transaction date
    sort(
        m_forecastVector.begin(), m_forecastVector.end(),
        [] (TransactionModel::Data const& a, TransactionModel::Data const& b) {
            return a.TRANSDATE < b.TRANSDATE;
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

    while(dt.IsEarlierThan(et))
    {
        dateMap[dt.FormatISODate()] = 0;
        if (monthly)
            dt.Add(wxDateSpan::Month());
        else
            dt.Add(wxDateSpan::Day());
    }

    // squash the data by month or day
    for (const auto& trx : m_forecastVector)
    {
        dt = TransactionModel::getTransDateTime(trx);
        wxString date = dt.FormatISODate();
        if (monthly)
        {
            date = dt.SetDay(1).FormatISODate();
        }
        dateMap[date] += trx.TRANSAMOUNT;
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
    for (const auto& entry : dateMap)
    {
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
                    dt.ParseDate(entry.first);
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
    for (const auto& entry : m_forecastVector)
    {
        runningBalance += entry.TRANSAMOUNT;
        gs.values.push_back(runningBalance);
        gd.labels.push_back(entry.TRANSDATE);
    }
    gd.series.push_back(gs);

    if (getChartSelection() == 0 && !gd.series.empty())
    {
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
    for (const auto& trx : m_forecastVector)
    {
        wxDateTime dt;
        int rowDate;
        dt.ParseDate(trx.TRANSDATE);
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
        hb.addTableCellDate(trx.TRANSDATE);
        hb.addTableCell(AccountModel::cache_id_name(trx.ACCOUNTID));
        hb.addTableCell((trx.TOACCOUNTID == -1) ? PayeeModel::get_payee_name(trx.PAYEEID)
            : "> " + AccountModel::cache_id_name(trx.TOACCOUNTID));
        hb.addTableCell(CategoryModel::full_name(trx.CATEGID));
        double amount = trx.TRANSAMOUNT;
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
