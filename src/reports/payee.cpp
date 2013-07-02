#include "payee.h"
#include "budget.h"

#include "../htmlbuilder.h"
#include "../util.h"
#include "../mmgraphpie.h"

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

    // Add the graph
    mmGraphPie gg;

    double total = 0.0, positiveTotal = 0.0, negativeTotal = 0.0;
    std::vector<ValuePair> valueList;
    bool ignore_date = !date_range_->is_with_date();

    for (int i=1; i>-2; i-=2)
    {
        valueList.clear();
        for (const auto& payee: core_->payeeList_.entries_)
        {
            double amt = core_->bTransactionList_.getAmountForPayee(payee->id_
                , ignore_date, date_range_->start_date()
                , date_range_->end_date(), mmIniOptions::instance().ignoreFutureTransactions_);

            if (amt<0 && i==-1)
            {
                ValuePair vp;
                vp.label = payee->name_;
                vp.amount = amt;
                valueList.push_back(vp);
                negativeTotal += amt;
            }
            else if(amt>0 && i==1)
            {
                ValuePair vp;
                vp.label = payee->name_;
                vp.amount = amt;
                valueList.push_back(vp);
                positiveTotal += amt;
            }
            else
                continue;

            total += amt;
        }
        std::sort(valueList.begin(), valueList.end(),
            [](const ValuePair& x, const ValuePair& y)
                {
                    if (x.amount < 0 && y.amount < 0)
                        return x.amount < y.amount ;
                    else
                        return x.amount > y.amount;
                }
        );

        if (i==-1)
        {
            gg.init(valueList);
            gg.Generate(title_);
            hb.startTableRow();
            hb.startTableCell();
            hb.addImage(gg.getOutputFileName());
            hb.endTableCell();
            hb.endTableRow();
        }

        hb.startTableRow();
        hb.addTableHeaderCell(_("Payee"));
        hb.addTableHeaderCell(_("Amount"), true);
        hb.endTableRow();

        for (const auto& payee : valueList)
            hb.addTableRow(payee.label, payee.amount);

        hb.addRowSeparator(2);
    }

    hb.addTableRowBold(_("Income:"), positiveTotal);
    hb.addTableRowBold(_("Expenses:"), negativeTotal);
    hb.addRowSeparator(2);
    hb.addTotalRow(_("Payees Total:"), 1, total);
    //hb.addTableRowBold(_("Payees Total:"), total);

    hb.endTable();
    hb.endCenter();
    hb.end();

    return hb.getHTMLText();
}
