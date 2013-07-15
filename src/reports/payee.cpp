#include "payee.h"
#include "budget.h"

#include "htmlbuilder.h"
#include "util.h"
#include "mmgraphpie.h"

#include <algorithm>

mmReportPayeeExpenses::mmReportPayeeExpenses(mmCoreDB* core, const wxString& title, mmDateRange* date_range)
    : mmPrintableBase(core)
    , title_(title)
    , date_range_(date_range)
{}

mmReportPayeeExpenses::~mmReportPayeeExpenses()
{
    if (date_range_) delete date_range_;
}

wxString mmReportPayeeExpenses::getHTMLText()
{
    core_->currencyList_.LoadBaseCurrencySettings();
    mmHTMLBuilder hb;
    hb.init();
    hb.addHeader(2, title_);
    hb.DisplayDateHeading(date_range_->start_date(), date_range_->end_date(), date_range_->is_with_date());
    hb.startCenter();
    hb.startTable("50%");
    hb.startTableRow();
    hb.addTableHeaderCell(_("Payee"));
    hb.addTableHeaderCell(_("Deposit"), true);
    hb.addTableHeaderCell(_("Withdrawal"), true);
    hb.addTableHeaderCell(_("Difference"), true);
    hb.endTableRow();

    // Add the graph
    mmGraphPie gg;

    double total = 0.0, positiveTotal = 0.0, negativeTotal = 0.0;
    std::vector<ValuePair> valueList;

    std::map<int, std::pair<double, double> > payeeStats;
    core_->bTransactionList_.getPayeeStats(payeeStats, date_range_
        , mmIniOptions::instance().ignoreFutureTransactions_ );

        valueList.clear();
    for (const auto& entry : payeeStats)
    {
        if (entry.second.first < 0)
        {
            ValuePair vp;
            vp.label = core_->payeeList_.GetPayeeName(entry.first);
            vp.amount = entry.second.first;
            valueList.push_back(vp);
        }

        positiveTotal += entry.second.first;
        negativeTotal += entry.second.second;

        hb.startTableRow();
        hb.addTableCell(core_->payeeList_.GetPayeeName(entry.first));
        hb.addMoneyCell(entry.second.first);
        hb.addMoneyCell(entry.second.second);
        hb.addMoneyCell(entry.second.first + entry.second.second);
        hb.endTableRow();
    }

    hb.addRowSeparator(4);
    std::vector <double> totals;
    totals.push_back(positiveTotal);
    totals.push_back(negativeTotal);
    totals.push_back(positiveTotal + negativeTotal);
    hb.addTotalRow(_("Total:"), 3, totals);

    hb.endTable();
    hb.endCenter();
    hb.end();

    return hb.getHTMLText();
}
