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

#include <algorithm>

htmlWidgetStocks::htmlWidgetStocks()
{
    title_ = _("Stocks");
}

htmlWidgetStocks::~htmlWidgetStocks()
{
}

wxString htmlWidgetStocks::getHTMLText()
{
    mmHTMLBuilder hb;

    if (Model_Stock::instance().all().size())
    {
        double stTotalBalance = 0.0, stTotalGain = 0.0;
        wxString tBalanceStr, tGainStr;

        hb.startTable("100%");
        //if (frame_->expandedStockAccounts())
        {
            hb.startTableRow();
            hb.addTableHeaderCell(_("Stocks"), false);
            hb.addTableHeaderCell(_("Gain/Loss"), true);
            hb.addTableHeaderCell(_("Total"), true);
            hb.endTableRow();
        }

        //TODO:
        double tRecBalance = 1, tBalance =2;

        std::vector<double> data;
        data.push_back(tRecBalance);
        data.push_back(tBalance);

        hb.startTableRow();
        hb.addTotalRow(_("Stocks Total:"), 3, data);
        hb.endTableRow();

        hb.endTable();
    }

    return hb.getHTMLinTableWraper(true);
}

