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

#include "cashflow.h"
#include "mmex.h"
#include "mmframe.h"
#include "util.h"
#include "reports/htmlbuilder.h"
#include "model/Model_Account.h"
#include "model/Model_Billsdeposits.h"
#include "model/Model_CurrencyHistory.h"

// --------- CashFlow base class

mmReportCashFlow::mmReportCashFlow(const wxString& name)
    : mmPrintableBase(name), m_today(Option::instance().UseTransDateTime() ? wxDateTime::Now() : wxDateTime(23,59,59,999))
{
    m_only_active = true;
}

mmReportCashFlow::~mmReportCashFlow()
{
}

double mmReportCashFlow::trueAmount(const Model_Checking::Data& trx)
{
    double amount = 0.0;
    bool isAccountFound = m_account_id.Index(trx.ACCOUNTID) != wxNOT_FOUND;
    bool isToAccountFound = m_account_id.Index(trx.TOACCOUNTID) != wxNOT_FOUND;
    if (!(isAccountFound && isToAccountFound))
    {
        const double convRate = Model_CurrencyHistory::getDayRate(Model_Account::instance().get(trx.ACCOUNTID)->CURRENCYID, trx.TRANSDATE);
        switch (Model_Checking::type_id(trx.TRANSCODE)) {
        case Model_Checking::TYPE_ID_WITHDRAWAL:
            amount = -trx.TRANSAMOUNT * convRate;
            break;
        case Model_Checking::TYPE_ID_DEPOSIT:
            amount = +trx.TRANSAMOUNT * convRate;
            break;
        case Model_Checking::TYPE_ID_TRANSFER:
            if (isAccountFound)
                amount = -trx.TRANSAMOUNT * convRate;
            else
            {
                const double toConvRate = Model_CurrencyHistory::getDayRate(Model_Account::instance().get(trx.TOACCOUNTID)->CURRENCYID, trx.TRANSDATE);
                amount = +trx.TOTRANSAMOUNT * toConvRate;
            }
        }
    }
    return amount;
}

void mmReportCashFlow::getTransactions()
{
    m_balance = 0.0;
    m_account_id.clear();
    m_forecastVector.clear();

    wxString todayString = m_today.FormatISOCombined();
    wxDateTime endDate = mmDateRange::getDayEnd(m_today.Add(wxDateSpan::Months(getForwardMonths())));

    // Get initial Balance as of today
    for (const auto& account : Model_Account::instance().find(
        Model_Account::ACCOUNTTYPE(Model_Account::TYPE_STR_INVESTMENT, NOT_EQUAL)
        , Model_Account::STATUS(Model_Account::STATUS_ID_CLOSED, NOT_EQUAL)))
    {
        if (accountArray_ && accountArray_->Index(account.ACCOUNTNAME) == wxNOT_FOUND) {
            continue;
        }

        double convRate = Model_CurrencyHistory::getDayRate(account.CURRENCYID, todayString);
        m_balance += account.INITIALBAL * convRate;

        m_account_id.Add(account.ACCOUNTID);

        for (const auto& tran : Model_Account::transaction(account))
        {
            wxString strDate = Model_Checking::TRANSDATE(tran).FormatISOCombined();
            // Do not include asset or stock transfers in income expense calculations.
            if (Model_Checking::foreignTransactionAsTransfer(tran)
                || (strDate > todayString))
                continue;
            m_balance += Model_Checking::balance(tran, account.ACCOUNTID) * convRate;
        }
    }

    // Now gather all transations posted after today
    Model_Checking::Data_Set transactions = Model_Checking::instance().find(
        Model_Checking::TRANSDATE(m_today, GREATER),
        Model_Checking::TRANSDATE(endDate, LESS),
        Model_Checking::STATUS(Model_Checking::STATUS_ID_VOID, NOT_EQUAL));
    for (auto& trx : transactions)
    {
        if (!trx.DELETEDTIME.IsEmpty()) continue;
        bool isAccountFound = m_account_id.Index(trx.ACCOUNTID) != wxNOT_FOUND;
        bool isToAccountFound = m_account_id.Index(trx.TOACCOUNTID) != wxNOT_FOUND;
        if (!isAccountFound && !isToAccountFound)
            continue; // skip account
        if (trx.CATEGID == -1)
        {
            Model_Checking::Data *transaction = Model_Checking::instance().get(trx.TRANSID);
            for (const auto& split_item : Model_Checking::splittransaction(transaction))
            {
                trx.CATEGID = split_item.CATEGID;
                trx.TRANSAMOUNT = split_item.SPLITTRANSAMOUNT;
                trx.TRANSAMOUNT = trueAmount(trx);
                m_forecastVector.push_back(trx);
            }
        } else
        {
            trx.TRANSAMOUNT = trueAmount(trx);
            m_forecastVector.push_back(trx);
        }
    }

    // Now we gather the recurring transaction list
    for (const auto& entry : Model_Billsdeposits::instance().find(Model_Billsdeposits::STATUS(Model_Checking::STATUS_ID_VOID, NOT_EQUAL)))
    {
        wxDateTime nextOccurDate = Model_Billsdeposits::NEXTOCCURRENCEDATE(entry);
        if (nextOccurDate > endDate) continue;

        // demultiplex entry.REPEATS
        int repeats = entry.REPEATS % BD_REPEATS_MULTIPLEX_BASE;
        int numRepeats = entry.NUMOCCURRENCES;

        // ignore old inactive entries
        if (repeats >= Model_Billsdeposits::REPEAT_IN_X_DAYS && repeats <= Model_Billsdeposits::REPEAT_EVERY_X_MONTHS && numRepeats == -1)
            continue;

        // ignore invalid entries
        if (repeats != Model_Billsdeposits::REPEAT_ONCE && (numRepeats == 0 || numRepeats < -1))
            continue;

        bool isAccountFound = m_account_id.Index(entry.ACCOUNTID) != wxNOT_FOUND;
        bool isToAccountFound = m_account_id.Index(entry.TOACCOUNTID) != wxNOT_FOUND;
        if (!isAccountFound && !isToAccountFound)
            continue; // skip account

        // Process all possible recurring transactions for this BD
        while (1)
        {
            if (nextOccurDate > endDate) break;

            Model_Checking::Data trx;
            trx.TRANSDATE = nextOccurDate.FormatISODate();
            trx.ACCOUNTID = entry.ACCOUNTID;
            trx.TOACCOUNTID = entry.TOACCOUNTID;
            trx.PAYEEID =  entry.PAYEEID;
            trx.TRANSCODE = entry.TRANSCODE;
            trx.TRANSAMOUNT = entry.TRANSAMOUNT;
            trx.TOTRANSAMOUNT = entry.TOTRANSAMOUNT;
            if (entry.CATEGID == -1)
            {
                for (const auto& split_item : Model_Billsdeposits::splittransaction(entry))
                {
                    trx.CATEGID = split_item.CATEGID;
                    trx.TRANSAMOUNT = split_item.SPLITTRANSAMOUNT;
                    trx.TRANSAMOUNT = trueAmount(trx);
                    m_forecastVector.push_back(trx);
                }
            } else
            {
                trx.CATEGID = entry.CATEGID;
                trx.TRANSAMOUNT = trueAmount(trx);
                m_forecastVector.push_back(trx);
            }

            if ((repeats == Model_Billsdeposits::REPEAT_ONCE) || ((repeats < Model_Billsdeposits::REPEAT_IN_X_DAYS || repeats > Model_Billsdeposits::REPEAT_EVERY_X_MONTHS) && numRepeats == 1))
                break;

            nextOccurDate = Model_Billsdeposits::nextOccurDate(repeats, numRepeats, nextOccurDate);

            if ((repeats < Model_Billsdeposits::REPEAT_IN_X_DAYS || repeats > Model_Billsdeposits::REPEAT_EVERY_X_MONTHS) && numRepeats > 1)
            {
                numRepeats--;
            }
            else if (repeats >= Model_Billsdeposits::REPEAT_IN_X_DAYS && repeats <= Model_Billsdeposits::REPEAT_IN_X_MONTHS)
            {
                // change repeat type to REPEAT_ONCE
                repeats = Model_Billsdeposits::REPEAT_ONCE;
            }
        }
    }

    // Sort by transaction date
    sort(m_forecastVector.begin(), m_forecastVector.end(),
        [] (Model_Checking::Data const& a, Model_Checking::Data const& b) { return a.TRANSDATE < b.TRANSDATE; });
}

wxString mmReportCashFlow::getHTMLText_DayOrMonth(bool monthly)
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
        dt = Model_Checking::TRANSDATE(trx);
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
    const wxString& headingStr = wxString::Format(wxPLURAL("%1$s (%2$i month)", "%1$s (%2$i months)" , getForwardMonths())
        , getReportTitle(), getForwardMonths());
    hb.addReportHeader(headingStr, 1, false);
    hb.DisplayFooter(getAccountNames());

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
                hb.addTableHeaderCell(_("Date"));
                hb.addTableHeaderCell(_("Total"), "text-right");
                hb.addTableHeaderCell(_("Difference"), "text-right");
                hb.addTableHeaderCell(_("Cumulative Difference"), "text-right");
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

    wxLogDebug("======= mmReportCashFlow:getHTMLText =======");
    wxLogDebug("%s", hb.getHTMLText());

    return hb.getHTMLText();
}

//--------- Cash Flow - Daily

mmReportCashFlowDaily::mmReportCashFlowDaily()
    : mmReportCashFlow(wxTRANSLATE("Cash Flow - Daily"))
{
    this->setForwardMonths(12);
    setReportParameters(Reports::DailyCashFlow);
}

wxString mmReportCashFlowDaily::getHTMLText()
{
    return getHTMLText_DayOrMonth(false);
}

//--------- Cash Flow - Monthly

mmReportCashFlowMonthly::mmReportCashFlowMonthly()
    : mmReportCashFlow(wxTRANSLATE("Cash Flow - Monthly"))
{
    this->setForwardMonths(120);
    setReportParameters(Reports::MonthlyCashFlow);
}

wxString mmReportCashFlowMonthly::getHTMLText()
{
    return getHTMLText_DayOrMonth(true);
}

//--------- Cash Flow - Transactions

mmReportCashFlowTransactions::mmReportCashFlowTransactions()
    : mmReportCashFlow(wxTRANSLATE("Cash Flow - Transactions"))
{
    setReportParameters(Reports::TransactionsCashFlow);
}

wxString mmReportCashFlowTransactions::getHTMLText()
{
    // Grab the data
    getTransactions();

    // Build the report
    mmHTMLBuilder hb;
    hb.init();
    const wxString& headingStr = wxString::Format(wxPLURAL("%1$s (%2$i month)", "%1$s (%2$i months)" , getForwardMonths())
        , getReportTitle(), getForwardMonths());
    hb.addReportHeader(headingStr, 1, false);
    hb.DisplayFooter(getAccountNames());

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
    hb.addTableHeaderCell(_("Date"));
    hb.addTableHeaderCell(_("Account"));
    hb.addTableHeaderCell(_("Payee"));
    hb.addTableHeaderCell(_("Category"));
    hb.addTableHeaderCell(_("Amount"), "text-right");
    hb.addTableHeaderCell(_("Balance"), "text-right");
    hb.addTableHeaderCell(_("Cumulative Difference"), "text-right");
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
        hb.addTableCell(Model_Account::get_account_name(trx.ACCOUNTID));
        hb.addTableCell((trx.TOACCOUNTID == -1) ? Model_Payee::get_payee_name(trx.PAYEEID)
            : "> " + Model_Account::get_account_name(trx.TOACCOUNTID));
        hb.addTableCell(Model_Category::full_name(trx.CATEGID));
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
