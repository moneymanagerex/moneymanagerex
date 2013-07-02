#include "incexpenses.h"
#include "budget.h"

#include "../htmlbuilder.h"
#include "../util.h"
#include "../mmgraphincexpensesmonth.h"

mmReportIncomeExpenses::mmReportIncomeExpenses(mmCoreDB* core, mmDateRange* date_range)
    : mmPrintableBase(core)
    , date_range_(date_range)
    , title_(_("Income vs Expenses: %s"))
{
}

wxString mmReportIncomeExpenses::title() const
{
    return wxString::Format(this->title_, date_range_->title());
}

wxString mmReportIncomeExpenses::getHTMLText()
{
    mmHTMLBuilder hb;
    hb.init();
    hb.addHeader(2, this->title());
    hb.DisplayDateHeading(date_range_->start_date(), date_range_->end_date(), date_range_->is_with_date());
    hb.addLineBreak();
    hb.startCenter();

    std::map<int, std::pair<double, double> > incomeExpensesStats;
    double expenses = 0.0, income = 0.0;
    core_->bTransactionList_.getExpensesIncomeStats(incomeExpensesStats
        , date_range_
        , -1
    );
    core_->currencyList_.LoadBaseCurrencySettings();

    for (const auto &stats: incomeExpensesStats)
    {
        income = stats.second.first;
        expenses = stats.second.second;
    }

    hb.startTable("75%");
    hb.addTableHeaderRow("", 2);

    hb.startTableRow();
    hb.startTableCell();
    mmGraphIncExpensesMonth gg;
    gg.init(income, expenses);
    gg.Generate(_("Income vs Expenses"));
    hb.addImage(gg.getOutputFileName());
    hb.endTableCell();

    hb.startTableCell();
    hb.startTable("95%");
    hb.startTableRow();
    hb.addTableHeaderCell(_("Type"));
    hb.addTableHeaderCell(_("Amount"), true);
    hb.endTableRow();

    hb.startTableRow();
    hb.addTableRow(_("Income:"), income);
    hb.addTableRow(_("Expenses:"), expenses);

    hb.addRowSeparator(2);
    hb.addTotalRow(_("Difference:"), 2, income - expenses);

    hb.endTable();

    hb.endTableCell();
    hb.endTableRow();
    hb.addRowSeparator(2);
    hb.endTable();

    hb.endCenter();
    hb.end();
    return hb.getHTMLText();
}

mmReportIncomeExpensesMontly::mmReportIncomeExpensesMontly(mmCoreDB* core, int day, int month, mmDateRange* date_range)
    : mmPrintableBase(core)
    , day_(day)
    , month_(month)
    , date_range_(date_range)
    , title_(_("Income vs Expenses: %s"))
{
}

wxString mmReportIncomeExpensesMontly::getHTMLText()
{
    double total_expenses = 0.0;
    double total_income = 0.0;
    std::map<int, std::pair<double, double> > incomeExpensesStats;

    mmHTMLBuilder hb;
    hb.init();
    hb.addHeader(2, this->title());
    hb.DisplayDateHeading(date_range_->start_date(), date_range_->end_date(), date_range_->is_with_date());
    hb.startCenter();

    hb.addHorizontalLine();
    hb.startTable("75%");
    hb.startTableRow();
    hb.addTableHeaderCell(_("Year"));
    hb.addTableHeaderCell(_("Month"));
    hb.addTableHeaderCell(_("Income"), true);
    hb.addTableHeaderCell(_("Expenses"), true);
    hb.addTableHeaderCell(_("Difference"), true);
    hb.endTableRow();

    core_->bTransactionList_.getExpensesIncomeStats(incomeExpensesStats
        , date_range_, -1, false, true);
    core_->currencyList_.LoadBaseCurrencySettings();

    for (const auto &stats: incomeExpensesStats)
    {
        total_expenses += stats.second.second;
        total_income += stats.second.first;

        hb.startTableRow();
        hb.addTableCell(wxString()<< (int)(stats.first/100));
        hb.addTableCellMonth(stats.first%100);
        hb.addMoneyCell(stats.second.first);
        hb.addMoneyCell(stats.second.second);
        hb.addMoneyCell(stats.second.first - stats.second.second);
        hb.endTableRow();
    }

    std::vector<double> data;
    data.push_back(total_income);
    data.push_back(total_expenses);
    data.push_back(total_income - total_expenses);

    hb.addRowSeparator(5);
    hb.addTotalRow(_("Total:"), 5, data);

    hb.endTable();
    hb.endCenter();
    hb.end();

    return hb.getHTMLText();
}
