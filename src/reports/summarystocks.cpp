/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel

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

#include "summarystocks.h"
#include "htmlbuilder.h"

#include "constants.h"
#include "stockspanel.h"
#include "util.h"
#include "model/Model_Account.h"

#include <algorithm>

#define STOCK_SORT_BY_NAME			1
#define STOCK_SORT_BY_SYMBOL		2
#define STOCK_SORT_BY_DATE			3
#define STOCK_SORT_BY_QTY			4
#define STOCK_SORT_BY_PUR_PRICE		5
#define STOCK_SORT_BY_CUR_PRICE		6
#define STOCK_SORT_BY_COMMISSION	7
#define STOCK_SORT_BY_GAIN_LOSS		8
#define STOCK_SORT_BY_VALUE			9

mmReportSummaryStocks::mmReportSummaryStocks(mmCoreDB* core)
: mmPrintableBase(core)
{
	// set initial sort column
	sortColumn_ = STOCK_SORT_BY_NAME;
}

static const char SELECT_ROW_SYMBOL_FROM_STOCK_V1[] =
    "SELECT "
           "STOCKNAME, "
           "TOTAL(NUMSHARES) AS NUMSHARES, "
           "SYMBOL, "
           "TOTAL(CURRENTPRICE*NUMSHARES)/TOTAL(NUMSHARES) AS CURRENTPRICE, "
           "TOTAL(PURCHASEPRICE*NUMSHARES)/TOTAL(NUMSHARES) AS PURCHASEPRICE, "
           "TOTAL(VALUE) AS VALUE, "
           "TOTAL(COMMISSION) AS COMMISSION, "
           "MIN(PURCHASEDATE) AS PURCHASEDATE "
    "FROM STOCK_V1 "
    "WHERE HELDAT = ? "
    "GROUP BY UPPER(SYMBOL) ";

wxString mmReportSummaryStocks::getHTMLText()
{
	// structure for sorting of data
    struct data_holder {wxString name; wxString symbol; wxString date; double qty; double purchase; double current; double commission; double gainloss; double value;} line;
	struct account_holder {wxString name; std::vector<data_holder> data; double gainloss; double total;} account;
    std::vector<account_holder> stocks;

    double gain_loss_sum_total = 0.0;
    double stockBalance = 0.0;

    for (const auto& a: Model_Account::instance().all())
    {
        if (Model_Account::type(a) != Model_Account::INVESTMENT) continue;
        if (Model_Account::status(a) != Model_Account::OPEN) continue;

		account.name = a.ACCOUNTNAME;
		account.gainloss = 0.0;
		account.total = Model_Account::investment_balance(a).second; 
		account.data.clear();

        wxSQLite3Statement st = core_->db_->PrepareStatement(SELECT_ROW_SYMBOL_FROM_STOCK_V1);
        st.Bind(1, a.ACCOUNTID);
        wxSQLite3ResultSet q2 = st.ExecuteQuery();

        while (q2.NextRow())
        {
            mmStockTransactionHolder th;

            //th.id_              = q2.GetInt("STOCKID");
            th.shareName_       = q2.GetString("STOCKNAME");
            th.numShares_       = q2.GetDouble("NUMSHARES");
            th.numSharesStr_    = q2.GetString("NUMSHARES");
            th.stockSymbol_     = q2.GetString("SYMBOL");

            th.currentPrice_    = q2.GetDouble("CURRENTPRICE");
            th.purchasePrice_   = q2.GetDouble("PURCHASEPRICE");
            th.value_           = q2.GetDouble("VALUE");
            double commission   = q2.GetDouble("COMMISSION");
            wxDateTime dtdt     = q2.GetDate("PURCHASEDATE");
            wxString dt         = mmGetDateForDisplay(dtdt);

            th.gainLoss_        = th.value_ - ((th.numShares_ * th.purchasePrice_) + commission);
            double base_conv_rate = core_->accountList_.getAccountBaseCurrencyConvRate(a.ACCOUNTID);
            stockBalance += base_conv_rate * th.value_;
            account.gainloss += th.gainLoss_;
            gain_loss_sum_total += th.gainLoss_ * base_conv_rate;

			line.name = th.shareName_;
			line.symbol = th.stockSymbol_;
			line.date = dt;
			line.qty = th.numShares_;
			line.purchase = th.purchasePrice_;
			line.current = th.currentPrice_;
			line.commission = commission;
			line.gainloss = th.gainLoss_;
			line.value = th.value_;
			account.data.push_back(line);
        }
        q2.Finalize();

		stocks.push_back(account);
    }

    for (auto& acct : stocks)
	{
		switch (sortColumn_)
		{
		case STOCK_SORT_BY_SYMBOL:
			std::stable_sort(acct.data.begin(), acct.data.end()
					, [] (const data_holder& x, const data_holder& y)
					{
						return x.symbol < y.symbol;
					}
			);
			break;
		case STOCK_SORT_BY_DATE:
			std::stable_sort(acct.data.begin(), acct.data.end()
					, [] (const data_holder& x, const data_holder& y)
					{
						if (x.date != y.date) return x.date < y.date;
						else return x.name < y.name;
					}
			);
			break;
		case STOCK_SORT_BY_QTY:
			std::stable_sort(acct.data.begin(), acct.data.end()
					, [] (const data_holder& x, const data_holder& y)
					{
						if (x.qty != y.qty) return x.qty < y.qty;
						else return x.name < y.name;
					}
			);
			break;
		case STOCK_SORT_BY_PUR_PRICE:
			std::stable_sort(acct.data.begin(), acct.data.end()
					, [] (const data_holder& x, const data_holder& y)
					{
						if (x.purchase != y.purchase) return x.purchase < y.purchase;
						else return x.name < y.name;
					}
			);
			break;
		case STOCK_SORT_BY_CUR_PRICE:
			std::stable_sort(acct.data.begin(), acct.data.end()
					, [] (const data_holder& x, const data_holder& y)
					{
						if (x.current != y.current) return x.current < y.current;
						else return x.name < y.name;
					}
			);
			break;
		case STOCK_SORT_BY_COMMISSION:
			std::stable_sort(acct.data.begin(), acct.data.end()
					, [] (const data_holder& x, const data_holder& y)
					{
						if (x.commission != y.commission) return x.commission < y.commission;
						else return x.name < y.name;
					}
			);
			break;
		case STOCK_SORT_BY_GAIN_LOSS:
			std::stable_sort(acct.data.begin(), acct.data.end()
					, [] (const data_holder& x, const data_holder& y)
					{
						if (x.gainloss != y.gainloss) return x.gainloss < y.gainloss;
						else return x.name < y.name;
					}
			);
			break;
		case STOCK_SORT_BY_VALUE:
			std::stable_sort(acct.data.begin(), acct.data.end()
					, [] (const data_holder& x, const data_holder& y)
					{
						if (x.value != y.value) return x.value < y.value;
						else return x.name < y.name;
					}
			);
			break;
		default:
			sortColumn_ = STOCK_SORT_BY_NAME;
			std::stable_sort(acct.data.begin(), acct.data.end()
					, [] (const data_holder& x, const data_holder& y)
					{
						return x.name < y.name;
					}
			);
		}
	}

    mmHTMLBuilder hb;
    hb.init();

    hb.addHeader(2, _("Summary of Stocks"));
    hb.addDateNow();

    hb.startTable("95%");
    for (const auto& acct : stocks)
	{
        hb.addTotalRow("", 9, "");
		hb.addTotalRow(acct.name, 9, "");

        display_header(hb);

		for (const auto& entry : acct.data)
		{
            hb.startTableRow();
			hb.addTableCell(entry.name, false, true);
			hb.addTableCell(entry.symbol);
			hb.addTableCell(entry.date);
			hb.addTableCell(wxString::Format("%.4f", entry.qty), true);
			hb.addMoneyCell(entry.purchase);
			hb.addMoneyCell(entry.current);
			hb.addMoneyCell(entry.commission);
			hb.addMoneyCell(entry.gainloss);
			hb.addMoneyCell(entry.value);
            hb.endTableRow();
		}

        hb.addRowSeparator(9);
		hb.addTotalRow(_("Total:"), 8, acct.gainloss);
		hb.addMoneyCell(acct.total);
	}
    core_->currencyList_.LoadBaseCurrencySettings();

    hb.addRowSeparator(9);
    hb.addTotalRow(_("Grand Total:"), 8, gain_loss_sum_total);
    hb.addMoneyCell(stockBalance);
    hb.endTableRow();
    hb.endTable();

    hb.endCenter();
    hb.end();
    return hb.getHTMLText();
}

void mmReportSummaryStocks::display_header(mmHTMLBuilder& hb) {
    hb.startTableRow();
	if(STOCK_SORT_BY_NAME == sortColumn_)
	    hb.addTableHeaderCell(_("Name"));
	else
	    hb.addTableHeaderCellLink(wxString::Format("SORT:%d", STOCK_SORT_BY_NAME), _("Name"));
	if(STOCK_SORT_BY_SYMBOL == sortColumn_)
		hb.addTableHeaderCell(_("Symbol"));
	else
		hb.addTableHeaderCellLink(wxString::Format("SORT:%d", STOCK_SORT_BY_SYMBOL), _("Symbol"));
	if(STOCK_SORT_BY_DATE == sortColumn_)
		hb.addTableHeaderCell(_("Purchase Date"));
	else
		hb.addTableHeaderCellLink(wxString::Format("SORT:%d", STOCK_SORT_BY_DATE), _("Purchase Date"));
	if(STOCK_SORT_BY_QTY == sortColumn_)
		hb.addTableHeaderCell(_("Quantity"), true);
	else
		hb.addTableHeaderCellLink(wxString::Format("SORT:%d", STOCK_SORT_BY_QTY), _("Quantity"), true);
	if(STOCK_SORT_BY_PUR_PRICE == sortColumn_)
		hb.addTableHeaderCell(_("Purchase Price"), true);
	else
		hb.addTableHeaderCellLink(wxString::Format("SORT:%d", STOCK_SORT_BY_PUR_PRICE), _("Purchase Price"), true);
	if(STOCK_SORT_BY_CUR_PRICE == sortColumn_)
		hb.addTableHeaderCell(_("Current Price"), true);
	else
		hb.addTableHeaderCellLink(wxString::Format("SORT:%d", STOCK_SORT_BY_CUR_PRICE), _("Current Price"), true);
	if(STOCK_SORT_BY_COMMISSION == sortColumn_)
		hb.addTableHeaderCell(_("Commission"), true);
	else
		hb.addTableHeaderCellLink(wxString::Format("SORT:%d", STOCK_SORT_BY_COMMISSION), _("Commission"), true);
	if(STOCK_SORT_BY_GAIN_LOSS == sortColumn_)
		hb.addTableHeaderCell(_("Gain/Loss"), true);
	else
		hb.addTableHeaderCellLink(wxString::Format("SORT:%d", STOCK_SORT_BY_GAIN_LOSS), _("Gain/Loss"), true);
	if(STOCK_SORT_BY_VALUE == sortColumn_)
		hb.addTableHeaderCell(_("Value"), true);
	else
		hb.addTableHeaderCellLink(wxString::Format("SORT:%d", STOCK_SORT_BY_VALUE), _("Value"), true);
    hb.endTableRow();
}
