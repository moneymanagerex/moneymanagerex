/*******************************************************
 Copyright (C) 2013 Nikolay

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

#include "html_widget_stocks.h"

#include "htmlbuilder.h"
#include "util.h"
#include "model/Model_Stock.h"
#include "model/Model_Account.h"

#include <algorithm>

htmlWidgetStocks::htmlWidgetStocks()
{
    title_ = _("Stocks");
    enable_details_ = true;
    grand_gain_lost_ = 0.0;
    grand_total_ = 0.0;
}

htmlWidgetStocks::~htmlWidgetStocks()
{
}

wxString htmlWidgetStocks::getHTMLText()
{
    mmHTMLBuilder hb;

    hb.startTable("100%");
    if (Model_Stock::instance().all().size())
    {
        std::map<int, std::pair<double, double> > stockStats;
        calculate_stats(stockStats);

        hb.startTableRow();
        hb.addTableHeaderCell(_("Stocks"), false);
        hb.addTableHeaderCell(_("Gain/Loss"), true);
        hb.addTableHeaderCell(_("Total"), true);
        hb.endTableRow();

        if (enable_details_)
        {
            Model_Account::Data_Set accounts = Model_Account::instance().all(Model_Account::COL_ACCOUNTNAME);
            for (const auto& account : accounts)
            {
                Model_Currency::Data *currency = Model_Currency::instance().get(account.ACCOUNTID);
                //TODO: How to load currency settings?
                if (Model_Account::type(account) != Model_Account::INVESTMENT) continue;
                hb.startTableRow();
                hb.addTableCellLink(wxString::Format("STOCK:%i", account.ACCOUNTID)
                    , account.ACCOUNTNAME, false, true);
                hb.addMoneyCell(stockStats[account.ACCOUNTID].first, true);
                hb.addMoneyCell(stockStats[account.ACCOUNTID].second, true);
                hb.endTableRow();
            }
        }

        std::vector<double> data;
        data.push_back(grand_gain_lost_);
        data.push_back(grand_total_);

        hb.addTotalRow(_("Stocks Total:"), 3, data);
        hb.endTable();
    }

    return hb.getHTMLinTableWraper(true);
}

void htmlWidgetStocks::enable_detailes(bool enable)
{
    enable_details_ = enable;
}

void htmlWidgetStocks::calculate_stats(std::map<int, std::pair<double, double> > &stockStats)
{
    Model_Stock::Data_Set stocks = Model_Stock::instance().all();
    for (const auto& stock : stocks)
    {
        double conv_rate = 1;
        Model_Account::Data *account = Model_Account::instance().get(stock.HELDAT);
        if (account)
        {
            Model_Currency::Data *currency = Model_Currency::instance().get(account->CURRENCYID);
            conv_rate = currency->BASECONVRATE;
        }
        std::pair<double, double> values = stockStats[stock.HELDAT];
        double gain_lost = (stock.VALUE - (stock.PURCHASEPRICE * stock.NUMSHARES) - stock.COMMISSION);
        values.first += gain_lost;
        values.second += stock.VALUE;
        stockStats[stock.HELDAT] = values;
        grand_gain_lost_ += gain_lost * conv_rate;
        grand_total_ += stock.VALUE * conv_rate;
    }
}

double htmlWidgetStocks::get_total()
{
    return grand_total_;
}

double htmlWidgetStocks::get_total_gein_lost()
{
    return grand_gain_lost_;
}
