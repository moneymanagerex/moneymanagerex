/*******************************************************
Copyright (C) 2006-2012

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

#include "incexpenses.h"
#include "budget.h"

#include "htmlbuilder.h"
#include "util.h"
#include "mmgraphincexpensesmonth.h"
#include "model/Model_Checking.h"
#include "model/Model_Account.h"

mmReportIncomeExpenses::mmReportIncomeExpenses(mmDateRange* date_range)
    : mmPrintableBaseSpecificAccounts(_("Income vs Expenses"))
    , date_range_(date_range)
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
    wxString headerMsg = _("Accounts: ");
    if (accountArray_ == nullptr)
    {
        headerMsg << _("All Accounts");
    }
    else
    {
        int arrIdx = 0;
        if ((int)accountArray_->size() == 0)
            headerMsg << "?";

        if (!accountArray_->empty())
        {
            headerMsg << accountArray_->Item(arrIdx);
            arrIdx++;
        }
        while (arrIdx < (int)accountArray_->size())
        {
            headerMsg << ", " << accountArray_->Item(arrIdx);
            arrIdx++;
        }
    }
    mmHTMLBuilder hb;
    hb.init();
    hb.addDivContainer();
    hb.addDivContainer();
    hb.addHeader(2, this->title());
    hb.DisplayDateHeading(date_range_->start_date(), date_range_->end_date(), date_range_->is_with_date());
    hb.addHeader(3, headerMsg);
    hb.addLineBreak();

    std::pair<double, double> income_expemses_rair;
    for (const auto& transaction : Model_Checking::instance().find(
        Model_Checking::TRANSDATE(date_range_->start_date(), GREATER_OR_EQUAL)
        , Model_Checking::TRANSDATE(date_range_->end_date(), LESS_OR_EQUAL)
        , Model_Checking::STATUS(Model_Checking::VOID_, NOT_EQUAL)))
    {
        // We got this far, get the currency conversion rate for this account
        Model_Account::Data *account = Model_Account::instance().get(transaction.ACCOUNTID);
        if (accountArray_)
        {
            if (wxNOT_FOUND == accountArray_->Index(account->ACCOUNTNAME)) continue;
        }
        double convRate = 1;
        if (account) convRate = Model_Account::currency(account)->BASECONVRATE;

        if (Model_Checking::type(transaction) == Model_Checking::DEPOSIT)
            income_expemses_rair.first += transaction.TRANSAMOUNT * convRate;
        else if (Model_Checking::type(transaction) == Model_Checking::WITHDRAWAL)
            income_expemses_rair.second += transaction.TRANSAMOUNT * convRate;
    }

    hb.addDivRow();
    hb.addDivCol8();
    hb.startTable();

    hb.startTableRow();
    hb.startTableCell();
    mmGraphIncExpensesMonth gg;
    gg.init(income_expemses_rair.first, income_expemses_rair.second);
    gg.Generate(_("Income vs Expenses"));
    hb.addImage(gg.getOutputFileName());
    hb.endTableCell();

    hb.startTableCell();
    hb.startTable();
    hb.startThead();
    hb.startTableRow();
    hb.addTableHeaderCell(_("Type"));
    hb.addTableHeaderCell(_("Amount"), true);
    hb.endTableRow();
    hb.endThead();

    hb.startTableRow();
    hb.addTableRow(_("Income:"), income_expemses_rair.first);
    hb.addTableRow(_("Expenses:"), income_expemses_rair.second);

    hb.addTotalRow(_("Difference:"), 2, income_expemses_rair.first - income_expemses_rair.second);

    hb.endTable();

    hb.endTableCell();
    hb.endTableRow();
    hb.endTable();

    hb.endDiv();
    hb.endDiv();
    hb.endDiv();
    hb.end();
    return hb.getHTMLText();
}

mmReportIncomeExpensesMonthly::mmReportIncomeExpensesMonthly(int day, int month, mmDateRange* date_range)
    : mmPrintableBaseSpecificAccounts(_("Income vs Expenses"))
    , day_(day)
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
    wxString headerMsg = _("Accounts: ");
    if (accountArray_ == nullptr)
    {
        headerMsg << _("All Accounts");
    }
    else
    {
        int arrIdx = 0;
        if ((int)accountArray_->size() == 0)
            headerMsg << "?";

        if (!accountArray_->empty())
        {
            headerMsg << accountArray_->Item(arrIdx);
            arrIdx++;
        }
        while (arrIdx < (int)accountArray_->size())
        {
            headerMsg << ", " << accountArray_->Item(arrIdx);
            arrIdx++;
        }
    }

    mmHTMLBuilder hb;
    hb.init();
    hb.addDivContainer();
    hb.addHeader(2, this->title());
    hb.DisplayDateHeading(date_range_->start_date(), date_range_->end_date(), date_range_->is_with_date());
    hb.addHeader(3, headerMsg);
    hb.addLineBreak();
    hb.addDivRow();
    hb.addDivCol8();

    hb.addHorizontalLine();
    hb.startSortTable();
    hb.startThead();
    hb.startTableRow();
    hb.addTableHeaderCell(_("Year"));
    hb.addTableHeaderCell(_("Month"));
    hb.addTableHeaderCell(_("Income"), true);
    hb.addTableHeaderCell(_("Expenses"), true);
    hb.addTableHeaderCell(_("Difference"), true);
    hb.endTableRow();
    hb.endThead();
    wxLogDebug("from %s till %s", date_range_->start_date().FormatISODate(), date_range_->end_date().FormatISODate());
    std::map<int, std::pair<double, double> > incomeExpensesStats;
    for (const auto& transaction : Model_Checking::instance().find(
        Model_Checking::TRANSDATE(date_range_->start_date(), GREATER_OR_EQUAL)
            , Model_Checking::TRANSDATE(date_range_->end_date(), LESS_OR_EQUAL)
            , Model_Checking::STATUS(Model_Checking::VOID_, NOT_EQUAL)))
    {
        // We got this far, get the currency conversion rate for this account
        Model_Account::Data *account = Model_Account::instance().get(transaction.ACCOUNTID);
        if (accountArray_)
        {
            if (wxNOT_FOUND == accountArray_->Index(account->ACCOUNTNAME)) continue;
        }
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
    hb.startTbody();
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
    hb.endTbody();

    std::vector<double> data;
    data.push_back(total_income);
    data.push_back(total_expenses);
    data.push_back(total_income - total_expenses);

    hb.addTotalRow(_("Total:"), 5, data);

    hb.endTable();
    hb.endDiv();
    hb.endDiv();
    hb.endDiv(); 
    hb.end();

    return hb.getHTMLText();
}
