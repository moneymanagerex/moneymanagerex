/*******************************************************
Copyright (C) 2006-2012 Madhan Kanagavel
Copyright (C) 2017 James Higley
Copyright (C) 2021 - 2022 Mark Whalley (mark@ipx.co.uk)

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

#include "base/images_list.h"
#include "util/_util.h"
#include "util/mmDateRange.h"
#include "htmlbuilder.h"

#include "model/AccountModel.h"
#include "model/TrxModel.h"
#include "model/CurrencyHistoryModel.h"
#include "model/CategoryModel.h"

#include "InExReport.h"

InExReport::InExReport()
    : ReportBase(_n("Income vs. Expenses Summary"))
{
    setReportParameters(REPORT_ID::IncomevsExpensesSummary);
}

InExReport::~InExReport()
{
}

wxString InExReport::getHTMLText()
{
    // Grab the data
    std::pair<double, double> income_expenses_pair;
    for (const auto& trx_d : TrxModel::instance().find(
        TrxModel::TRANSDATE(OP_GE, mmDate(m_date_range->start_date())),
        TrxModel::TRANSDATE(OP_LE, mmDate(m_date_range->end_date())),
        TrxCol::DELETEDTIME(OP_EQ, wxEmptyString),
        TrxModel::STATUS(OP_NE, TrxStatus(TrxStatus::e_void))
    )) {
        // Do not include asset or stock transfers
        if (TrxModel::is_foreignAsTransfer(trx_d))
            continue;

        const AccountData *account = AccountModel::instance().get_id_data_n(trx_d.m_account_id);
        if (m_account_a) {
            if (!account || wxNOT_FOUND == m_account_a->Index(account->m_name))
                continue;
        }
        double convRate = 1;
        // We got this far, get the currency conversion rate for this account
        if (account) {
            convRate = CurrencyHistoryModel::getDayRate(
                AccountModel::instance().get_data_currency_p(*account)->m_id, trx_d.TRANSDATE
            );
        }

        if (trx_d.is_deposit())
            income_expenses_pair.first += trx_d.m_amount * convRate;
        else if (trx_d.is_withdrawal())
            income_expenses_pair.second += trx_d.m_amount * convRate;
    }

    // Build the report
    mmHTMLBuilder hb;
    hb.init();
    hb.addReportHeader(getTitle(), m_date_range->startDay(), m_date_range->isFutureIgnored());
    hb.displayDateHeading(m_date_range);
    hb.displayFooter(getAccountNames());

    // Chart
    GraphData gd;
    GraphSeries gs;

    gs.values = { income_expenses_pair.first };
    gs.name = _t("Income");
    gd.series.push_back(gs);
    gs.values = { income_expenses_pair.second };
    gs.name = _t("Expenses");
    gd.series.push_back(gs);

    gd.labels.push_back(m_date_range->local_title());

    if (!gd.series.empty())
    {
        gd.type = GraphData::BAR;
        gd.colors = { mmThemeMetaColour(meta::COLOR_REPORT_CREDIT)
                        , mmThemeMetaColour(meta::COLOR_REPORT_DEBIT) };  
        hb.addChart(gd);
    }

    hb.addDivContainer("shadow");
    {
        hb.startTable();
        {
            hb.startThead();
            {
                hb.startTableRow();
                    hb.addTableHeaderCell(_t("Type"));
                    hb.addTableHeaderCell(_t("Amount"), "text-right");
                hb.endTableRow();
            }
            hb.endThead();
            hb.startTbody();
            {
                hb.addTableRow(_t("Income:"), income_expenses_pair.first);
                hb.addTableRow(_t("Expenses:"), income_expenses_pair.second);
                hb.addTotalRow(_t("Difference:"), 2, income_expenses_pair.first - income_expenses_pair.second);
            }
            hb.endTbody();
        }
        hb.endTable();
    }
    hb.endDiv(); 
    hb.end();

    wxLogDebug("======= InExReport:getHTMLText =======");
    wxLogDebug("%s", hb.getHTMLText());

    return hb.getHTMLText();
}

mmReportIncomeExpensesMonthly::mmReportIncomeExpensesMonthly()
    : ReportBase(_n("Income vs. Expenses Monthly"))
{
    setReportParameters(REPORT_ID::IncomevsExpensesMonthly);
}

mmReportIncomeExpensesMonthly::~mmReportIncomeExpensesMonthly()
{
}

wxString mmReportIncomeExpensesMonthly::getHTMLText()
{
    // Grab the data
    const wxDateTime start_date = m_date_range->start_date();
    std::map<int, std::pair<double, double> > incomeExpensesStats;
    // TODO: init all the map values with 0.0
    for (const auto& trx_d : TrxModel::instance().find(
        TrxModel::TRANSDATE(OP_GE, mmDate(start_date)),
        TrxModel::TRANSDATE(OP_LE, mmDate(m_date_range->end_date())),
        TrxCol::DELETEDTIME(OP_EQ, wxEmptyString),
        TrxModel::STATUS(OP_NE, TrxStatus(TrxStatus::e_void))
    )) {
        // Do not include asset or stock transfers
        if (TrxModel::is_foreignAsTransfer(trx_d))
            continue;

        const AccountData *account = AccountModel::instance().get_id_data_n(trx_d.m_account_id);
        if (m_account_a) {
            if (!account || wxNOT_FOUND == m_account_a->Index(account->m_name))
                continue;
        }
        double convRate = 1;
        // We got this far, get the currency conversion rate for this account
        if (account) {
            convRate = CurrencyHistoryModel::getDayRate(
                AccountModel::instance().get_data_currency_p(*account)->m_id, trx_d.TRANSDATE
            );
        }
        int year = TrxModel::getTransDateTime(trx_d).GetYear();

        int idx = year * 100 + TrxModel::getTransDateTime(trx_d).GetMonth();

        if (trx_d.is_deposit()) {
            incomeExpensesStats[idx].first += trx_d.m_amount * convRate;
        }
        else if (trx_d.is_withdrawal()) {
            incomeExpensesStats[idx].second += trx_d.m_amount * convRate;
        }
    }

    // Build the report
    mmHTMLBuilder hb;
    hb.init();
    hb.addReportHeader(getTitle(), m_date_range->startDay(), m_date_range->isFutureIgnored());
    hb.displayDateHeading(m_date_range);
    hb.displayFooter(getAccountNames());

    // Chart
    wxDateSpan s = m_date_range->end_date().GetLastMonthDay().DiffAsDateSpan(start_date);
    int m = s.GetYears() * 12 + s.GetMonths() + 1;
    m = m > 60 ? 60 : m;

    wxLogDebug("%s %s %i", start_date.FormatISODate(), m_date_range->end_date().FormatISODate(), m);

    if (getChartSelection() == 0)
    {
        GraphData gd;
        GraphSeries data_negative, data_positive, data_difference, data_performance;

        double performance = 0;
        for (const auto &stats : incomeExpensesStats)
        {
            data_positive.values.push_back(stats.second.first);
            data_negative.values.push_back(stats.second.second);
            data_difference.values.push_back(stats.second.first - stats.second.second);
            performance = performance + stats.second.first - stats.second.second;
            data_performance.values.push_back(performance);

            const auto label = wxString::Format("%s %i"
                , wxGetTranslation(wxDateTime::GetEnglishMonthName(static_cast<wxDateTime::Month>(stats.first % 100))), stats.first / 100);
            gd.labels.push_back(label);
        }

        data_performance.name = _t("Cumulative");
        data_difference.name = _t("Difference");
        data_positive.name = _t("Income");
        data_negative.name = _t("Expenses");

        data_performance.type = "line";
        data_difference.type = "line";
        data_positive.type = "column";
        data_negative.type = "column";

        gd.series.push_back(data_performance);
        gd.series.push_back(data_difference);
        gd.series.push_back(data_positive);
        gd.series.push_back(data_negative);


        if (!gd.series.empty())
        {
            gd.type = GraphData::BARLINE; 
            gd.colors = { mmThemeMetaColour(meta::COLOR_REPORT_PERF)
                            , mmThemeMetaColour(meta::COLOR_REPORT_DELTA)
                            , mmThemeMetaColour(meta::COLOR_REPORT_CREDIT)
                            , mmThemeMetaColour(meta::COLOR_REPORT_DEBIT) }; 
            hb.addChart(gd);
        }
    }

    hb.addDivContainer("shadow"); // Table Container
    hb.startSortTable();
    {
        hb.startThead();
        {
            hb.startTableRow();
            hb.addTableHeaderCell(_t("Date"));
            hb.addTableHeaderCell(_t("Income"), "text-right");
            hb.addTableHeaderCell(_t("Expenses"), "text-right");
            hb.addTableHeaderCell(_t("Difference"), "text-right");
            hb.addTableHeaderCell(_t("Cumulative"), "text-right");
            hb.endTableRow();
        }
        hb.endThead();
        wxLogDebug("from %s till %s", m_date_range->start_date().FormatISODate(), m_date_range->end_date().FormatISODate());

        double total_expenses = 0.0;
        double total_income = 0.0;
        hb.startTbody();
        for (const auto &stats : incomeExpensesStats)
        {
            total_expenses += stats.second.second;
            total_income += stats.second.first;

            hb.startTableRow();
            {
                hb.addTableCellMonth(stats.first % 100, stats.first / 100);
                hb.addMoneyCell(stats.second.first);
                hb.addMoneyCell(stats.second.second);
                hb.addMoneyCell(stats.second.first - stats.second.second);
                hb.addMoneyCell(total_income - total_expenses);
            }
            hb.endTableRow();
        }
        hb.endTbody();

        std::vector<double> totals;
        totals.push_back(total_income);
        totals.push_back(total_expenses);
        totals.push_back(total_income - total_expenses);
        totals.push_back(total_income - total_expenses);

        hb.addMoneyTotalRow(_t("Total:"), 5, totals);
    }
    hb.endTable();

    hb.endDiv(); // Table container
    hb.endDiv(); // Report Container
    hb.endDiv(); // Main container
    hb.end();

    wxLogDebug("======= mmReportIncomeExpensesMonthly::getHTMLText =======");
    wxLogDebug("%s", hb.getHTMLText());

    return hb.getHTMLText();
}
