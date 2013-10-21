#include "incexpenses.h"
#include "budget.h"

#include "htmlbuilder.h"
#include "util.h"
#include "mmgraphincexpensesmonth.h"
#include "model/Model_Checking.h"
#include "model/Model_Account.h"

mmReportIncomeExpenses::mmReportIncomeExpenses(mmDateRange* date_range)
    : date_range_(date_range)
    , title_(_("Income vs Expenses: %s"))
{
}

mmReportIncomeExpenses::~mmReportIncomeExpenses()
{
    if(date_range_)
        delete date_range_;
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

    std::pair<double, double> income_expemses_rair;
    for (const auto& transaction : Model_Checking::instance().find(
        Model_Checking::TRANSDATE(date_range_->start_date(), GREATER_OR_EQUAL)
        , Model_Checking::TRANSDATE(date_range_->end_date(), LESS_OR_EQUAL)
        , Model_Checking::STATUS(Model_Checking::VOID_, NOT_EQUAL)))
    {
        // We got this far, get the currency conversion rate for this account
        Model_Account::Data *account = Model_Account::instance().get(transaction.ACCOUNTID);
        double convRate = 1;
        if (account) convRate = Model_Account::currency(account)->BASECONVRATE;

        if (Model_Checking::type(transaction) == Model_Checking::DEPOSIT)
            income_expemses_rair.first += transaction.TRANSAMOUNT * convRate;
        else if (Model_Checking::type(transaction) == Model_Checking::WITHDRAWAL)
            income_expemses_rair.second += transaction.TRANSAMOUNT * convRate;
    }

    hb.startTable("75%");
    hb.addTableHeaderRow("", 2);

    hb.startTableRow();
    hb.startTableCell();
    mmGraphIncExpensesMonth gg;
    gg.init(income_expemses_rair.first, income_expemses_rair.second);
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
    hb.addTableRow(_("Income:"), income_expemses_rair.first);
    hb.addTableRow(_("Expenses:"), income_expemses_rair.second);

    hb.addRowSeparator(2);
    hb.addTotalRow(_("Difference:"), 2, income_expemses_rair.first - income_expemses_rair.second);

    hb.endTable();

    hb.endTableCell();
    hb.endTableRow();
    hb.addRowSeparator(2);
    hb.endTable();

    hb.endCenter();
    hb.end();
    return hb.getHTMLText();
}

mmReportIncomeExpensesMonthly::mmReportIncomeExpensesMonthly(int day, int month, mmDateRange* date_range)
    : day_(day)
    , month_(month)
    , date_range_(date_range)
    , title_(_("Income vs Expenses: %s"))
{
}

mmReportIncomeExpensesMonthly::~mmReportIncomeExpensesMonthly()
{
	if(date_range_)
		delete date_range_;
}

wxString mmReportIncomeExpensesMonthly::getHTMLText()
{
    //FIXME: runing twice
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

    wxLogDebug("from %s till %s", date_range_->start_date().FormatISODate(), date_range_->end_date().FormatISODate());
    std::map<int, std::pair<double, double> > incomeExpensesStats;
    for (const auto& transaction : Model_Checking::instance().find(
        Model_Checking::TRANSDATE(date_range_->start_date(), GREATER_OR_EQUAL)
            , Model_Checking::TRANSDATE(date_range_->end_date(), LESS_OR_EQUAL)
            , Model_Checking::STATUS(Model_Checking::VOID_, NOT_EQUAL)))
    {
        // We got this far, get the currency conversion rate for this account
        Model_Account::Data *account = Model_Account::instance().get(transaction.ACCOUNTID);
        double convRate = (account ? Model_Account::currency(account)->BASECONVRATE : 1);

        int idx = (Model_Checking::TRANSDATE(transaction).GetYear() * 100
            + (int) Model_Checking::TRANSDATE(transaction).GetMonth());

        if (Model_Checking::type(transaction) == Model_Checking::DEPOSIT)
            incomeExpensesStats[idx].first += transaction.TRANSAMOUNT * convRate;
        else if (Model_Checking::type(transaction) == Model_Checking::WITHDRAWAL)
            incomeExpensesStats[idx].second += transaction.TRANSAMOUNT * convRate;
    }

    double total_expenses = 0.0;
    double total_income = 0.0;
    for (const auto &stats : incomeExpensesStats)
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
