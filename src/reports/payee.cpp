#include "payee.h"
#include "budget.h"

#include "../htmlbuilder.h"
#include "../util.h"
#include "../mmgraphpie.h"

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
    mmHTMLBuilder hb;
    hb.init();
    hb.addHeader(2, title_);
    hb.DisplayDateHeading(date_range_->start_date(), date_range_->end_date(), date_range_->is_with_date());
    hb.startCenter();

    // Add the graph
    mmGraphPie gg;
    if (core_->payeeList_.Num())
    {
        hb.addImage(gg.getOutputFileName());
    }

    hb.startTable("50%");
    hb.startTableRow();
    hb.addTableHeaderCell(_("Payee"));
    hb.addTableHeaderCell(_("Amount"), true);
    hb.endTableRow();

    core_->currencyList_.LoadBaseCurrencySettings();

    double total = 0.0, positiveTotal = 0.0, negativeTotal = 0.0;
    std::vector<ValuePair> valueList;
    bool ignore_date = !date_range_->is_with_date();

    for (const auto& payee: core_->payeeList_.entries_)
    {
        double amt = core_->bTransactionList_.getAmountForPayee(payee->id_
            , ignore_date, date_range_->start_date()
            , date_range_->end_date(), mmIniOptions::instance().ignoreFutureTransactions_);

        if (amt != 0.0)
        {
            total += amt;
            if (amt>0.0)
                positiveTotal += amt;
            else
                negativeTotal += amt;
            ValuePair vp;
            vp.label = payee->name_;
            vp.amount = amt;
            valueList.push_back(vp);

            hb.startTableRow();
            hb.addTableCell(payee->name_, false, true);
            hb.addMoneyCell(amt);
            hb.endTableRow();
        }
    }

    hb.addRowSeparator(2);
    hb.startTableRow();
    hb.addTableCell(_("Income:"), false, true, true);
    hb.addMoneyCell(positiveTotal);
    hb.endTableRow();
    hb.startTableRow();
    hb.addTableCell(_("Expenses:"), false, true, true);
    hb.addMoneyCell(negativeTotal);
    hb.endTableRow();
    hb.addRowSeparator(2);
    hb.startTableRow();
    hb.addTableCell(_("Payees Total:"), false, true, true);
    hb.addMoneyCell(total);
    hb.endTableRow();

    hb.endTable();
    hb.endCenter();

    hb.end();

    gg.init(valueList);
    gg.Generate(title_);

    return hb.getHTMLText();
}
