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

#include "util.h"
#include "model/Model_Stock.h"
#include "model/Model_Account.h"

#include <algorithm>

htmlWidgetStocks::htmlWidgetStocks()
: title_(_("Stocks"))
{
    enable_details_ = true;
    grand_gain_lost_ = 0.0;
    grand_total_ = 0.0;
}

htmlWidgetStocks::~htmlWidgetStocks()
{
}

wxString htmlWidgetStocks::getHTMLText()
{
    wxString output = "";
    std::map<int, std::pair<double, double> > stockStats;
    calculate_stats(stockStats);
    if (!stockStats.empty())
    {
        if (enable_details_)
        {
            output = "<table class = \"table\"><thead><tr><th>";
            output += _("Stocks") + "</th><th>" + _("Gain/Loss") + "</th><th>" + _("Total") + "</th></tr></thead><tbody>";
            const auto &accounts = Model_Account::instance().all(Model_Account::COL_ACCOUNTNAME);
            wxString body = "";
            for (const auto& account : accounts)
            {
                if (Model_Account::type(account) != Model_Account::INVESTMENT) continue;
                if (Model_Account::status(account) != Model_Account::OPEN) continue;
                body += "<tr>";
                body += wxString::Format("<td><a href=\"STOCK:%d\">%s</a></td>", account.ACCOUNTID, account.ACCOUNTNAME);
                body += wxString::Format("<td class = \"money, text-right\">%f</td>", stockStats[account.ACCOUNTID].first);
                body += wxString::Format("<td class = \"money, text-right\">%f</td>", stockStats[account.ACCOUNTID].second);
                body += "</tr>";
            }

            output += body;
            output += "</tbody><tfoot><tr class = \"total\"><td>" + _("Total:") + "</td>";
            output += "<td class =\"money, text-right\">" + wxString::Format("%f", grand_gain_lost_) + "</td>";
            output += "<td class =\"money, text-right\">" + wxString::Format("%f", grand_total_) + "</td></tr></tfoot></table>";
            if (body.empty()) output.clear();
        }
    }

    return output;
}

void htmlWidgetStocks::enable_detailes(bool enable)
{
    enable_details_ = enable;
}

void htmlWidgetStocks::calculate_stats(std::map<int, std::pair<double, double> > &stockStats)
{
    this->grand_total_ = 0;
    this->grand_gain_lost_ = 0;
    const auto &stocks = Model_Stock::instance().all();
    for (const auto& stock : stocks)
    {
        double conv_rate = 1;
        Model_Account::Data *account = Model_Account::instance().get(stock.HELDAT);
        if (account)
        {
            Model_Currency::Data *currency = Model_Currency::instance().get(account->CURRENCYID);
            conv_rate = currency->BASECONVRATE;
        }
        std::pair<double, double>& values = stockStats[stock.HELDAT];
        double gain_lost = (stock.VALUE - (stock.PURCHASEPRICE * stock.NUMSHARES) - stock.COMMISSION);
        values.first += gain_lost;
        values.second += stock.VALUE;
        if (account && account->STATUS == "Open")
        {
            grand_total_ += stock.VALUE * conv_rate;
            grand_gain_lost_ += gain_lost * conv_rate;
        }
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
