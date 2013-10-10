#include "payee.h"

#include "htmlbuilder.h"
#include "util.h"
#include "mmgraphpie.h"
#include "model/Model_Currency.h"
#include "model/Model_Payee.h"
#include "model/Model_Account.h"

#include <algorithm>

#define PAYEE_SORT_BY_NAME      1
#define PAYEE_SORT_BY_INCOME    2
#define PAYEE_SORT_BY_EXPENSE   3
#define PAYEE_SORT_BY_DIFF      4

mmReportPayeeExpenses::mmReportPayeeExpenses(const wxString& title, mmDateRange* date_range)
    : mmPrintableBase(PAYEE_SORT_BY_DIFF)
    , title_(title)
    , date_range_(date_range)
{
}

mmReportPayeeExpenses::~mmReportPayeeExpenses()
{
    if (date_range_) delete date_range_;
}

wxString mmReportPayeeExpenses::getHTMLText()
{
    double positiveTotal = 0.0, negativeTotal = 0.0;
    std::vector<ValuePair> valueList;
    valueList.clear();
    struct data_holder {wxString name; double incomes; double expences;} line;
    std::vector<data_holder> data;

    std::map<int, std::pair<double, double> > payeeStats;
    getPayeeStats(payeeStats, date_range_
        , mmIniOptions::instance().ignoreFutureTransactions_ );

    for (const auto& entry : payeeStats)
    {
        positiveTotal += entry.second.first;
        negativeTotal += entry.second.second;

        Model_Payee::Data* payee = Model_Payee::instance().get(entry.first);
        if (payee)
            line.name = payee->PAYEENAME;
        line.incomes = entry.second.first;
        line.expences = entry.second.second;
        data.push_back(line);
    }

    switch (sortColumn_)
    {
    case PAYEE_SORT_BY_NAME:
        std::stable_sort(data.begin(), data.end()
            , [] (const data_holder& x, const data_holder& y)
            {
                return x.name < y.name;
            }
        );
        break;
    case PAYEE_SORT_BY_INCOME:
        std::stable_sort(data.begin(), data.end()
            , [] (const data_holder& x, const data_holder& y)
            {
                if (x.incomes != y.incomes) return x.incomes < y.incomes;
                else return x.name < y.name;
            }
        );
        break;
    case PAYEE_SORT_BY_EXPENSE:
        std::stable_sort(data.begin(), data.end()
            , [] (const data_holder& x, const data_holder& y)
            {
                if (x.expences != y.expences) return x.expences < y.expences;
                else return x.name < y.name;
            }
        );
        break;
    default:
        sortColumn_ = PAYEE_SORT_BY_DIFF;
        std::stable_sort(data.begin(), data.end()
            , [] (const data_holder& x, const data_holder& y)
            {
                if (x.expences+x.incomes != y.expences+y.incomes) return x.expences+x.incomes < y.expences+y.incomes;
                else return x.name < y.name;
            }
        );
    }

    mmHTMLBuilder hb;
    hb.init();
    hb.addHeader(2, title_);
    hb.DisplayDateHeading(date_range_->start_date(), date_range_->end_date(), date_range_->is_with_date());
    hb.startCenter();

    // Add the graph
    mmGraphPie gg;
    hb.addImage(gg.getOutputFileName());

    hb.startTable("75%");
    hb.startTableRow();
    if(PAYEE_SORT_BY_NAME == sortColumn_)
        hb.addTableHeaderCell(_("Payee"));
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", PAYEE_SORT_BY_NAME), _("Payee"));
    if(PAYEE_SORT_BY_INCOME == sortColumn_)
        hb.addTableHeaderCell(_("Incomes"), true);
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", PAYEE_SORT_BY_INCOME), _("Incomes"), true);
    if(PAYEE_SORT_BY_EXPENSE == sortColumn_)
        hb.addTableHeaderCell(_("Expences"), true);
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", PAYEE_SORT_BY_EXPENSE), _("Expences"), true);
    if(PAYEE_SORT_BY_DIFF == sortColumn_)
        hb.addTableHeaderCell(_("Difference"), true);
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", PAYEE_SORT_BY_DIFF), _("Difference"), true);
    hb.endTableRow();

    for (const auto& entry : data)
    {
        hb.startTableRow();
        hb.addTableCell(entry.name);
        hb.addMoneyCell(entry.incomes);
        hb.addMoneyCell(entry.expences);
        hb.addMoneyCell(entry.incomes + entry.expences);
        hb.endTableRow();

        if (entry.incomes + entry.expences < 0)
        {
            ValuePair vp;
            vp.label = entry.name;
            vp.amount = entry.incomes + entry.expences;
            valueList.push_back(vp);
        }
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

    gg.init(valueList);
    gg.Generate(title_);

    return hb.getHTMLText();
}

void mmReportPayeeExpenses::getPayeeStats(std::map<int, std::pair<double, double> > &payeeStats
                                          , mmDateRange* date_range, bool ignoreFuture) const
{
    //Get base currency rates for all accounts
    std::map<int, double> acc_conv_rates;
    for (const auto& account: Model_Account::instance().all())
    {
        Model_Currency::Data* currency = Model_Account::currency(account);
        acc_conv_rates[account.ACCOUNTID] = currency->BASECONVRATE;
    }

    Model_Checking::Data_Set transactions = Model_Checking::instance().all();
    for (const auto & trx: transactions)
    {
        if (Model_Checking::status(trx) == Model_Checking::VOID) continue;
        if (Model_Checking::type(trx) == Model_Checking::TRANSFER) continue;

        wxDateTime trx_date = mmGetStorageStringAsDate(trx.TRANSDATE);
        if (ignoreFuture)
        {
            if (trx_date.IsLaterThan(wxDateTime::Now()))
                continue; //skip future dated transactions
        }

        if (!trx_date.IsBetween(date_range->start_date(), date_range->end_date()))
            continue; //skip

        double convRate = acc_conv_rates[trx.ACCOUNTID];

        if (trx.STATUS == TRANS_TYPE_DEPOSIT_STR)
            payeeStats[trx.PAYEEID].first += trx.TRANSAMOUNT * convRate;
        else
            payeeStats[trx.PAYEEID].second -= trx.TRANSAMOUNT * convRate;

        //TODO: some split transaction may be withdrawal some deposit for one transaction
        /*if (trx.CATEGID > -1)
        {
            if (trx->value(-1) >= 0)
                payeeStats[trx.PAYEEID].first += trx->value(-1) * convRate;
            else
                payeeStats[trx.PAYEEID].second += trx->value(-1) * convRate;
        }
        else
        {
            mmSplitTransactionEntries* splits = trx->splitEntries_;
            trx->getSplitTransactions(splits);
            for (const auto& entry : splits->entries_)
            {
                if (trx->value(-1) * entry->splitAmount_ > 0)
                    payeeStats[trx.PAYEEID].first -= entry->splitAmount_ * convRate;
                else
                    payeeStats[trx.PAYEEID].second -= entry->splitAmount_ * convRate;
            }
        }*/
    }
}
