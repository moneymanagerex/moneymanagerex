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

#include "billsdepositspanel.h"
#include "htmlbuilder.h"
#include "util.h"
#include "html_widget_bills_and_deposits.h"
#include "model/Model_Billsdeposits.h"
#include "model/Model_Payee.h"
#include "model/Model_Account.h"

#include <algorithm>

htmlWidgetBillsAndDeposits::htmlWidgetBillsAndDeposits(const wxString& title, mmDateRange* date_range)
    : title_(title)
    , date_range_(date_range)
{}

htmlWidgetBillsAndDeposits::~htmlWidgetBillsAndDeposits()
{
    if (date_range_) delete date_range_;
}

wxString htmlWidgetBillsAndDeposits::getHTMLText()
{
    mmHTMLBuilder hb;

    std::map<int, std::pair<int, wxString>> bd_days;
    const wxDateTime &today = date_range_->today();
    for (const auto& q1 : Model_Billsdeposits::instance().all(Model_Billsdeposits::COL_NEXTOCCURRENCEDATE))
    {
        wxTimeSpan ts = Model_Billsdeposits::NEXTOCCURRENCEDATE(q1).Subtract(today);
        int daysRemaining = ts.GetDays();
        int minutesRemaining = ts.GetMinutes();
        if (minutesRemaining > 0)
            daysRemaining += 1;

        if (daysRemaining > 14)
            break; // Done searching for all to include

        int repeats        = q1.REPEATS;
        // DeMultiplex the Auto Executable fields.
        if (repeats >= BD_REPEATS_MULTIPLEX_BASE)    // Auto Execute User Acknowlegement required
            repeats -= BD_REPEATS_MULTIPLEX_BASE;
        if (repeats >= BD_REPEATS_MULTIPLEX_BASE)    // Auto Execute Silent mode
            repeats -= BD_REPEATS_MULTIPLEX_BASE;

        wxString daysRemainingStr = wxString::Format("%d", daysRemaining) + _(" days remaining");
        if (daysRemaining == 0)
        {
            if (((repeats > 10) && (repeats < 15)) && (q1.NUMOCCURRENCES < 0))
                continue; // Inactive
        }
        if (daysRemaining < 0)
        {
            daysRemainingStr = wxString::Format("%d", abs(daysRemaining)) + _(" days overdue!");
            if (((repeats > 10) && (repeats < 15)) && (q1.NUMOCCURRENCES < 0))
                continue; // Inactive
        }

        bd_days[q1.BDID].first = daysRemaining;
        bd_days[q1.BDID].second = daysRemainingStr;
    }

    ////////////////////////////////////
    if (bd_days.size() > 0)
    {
        wxString colorStr;

        hb.startTable("100%");
        hb.addTableHeaderRowLink("billsdeposits", title_, 3);

        for (const auto& item : bd_days)
        {
            const Model_Billsdeposits::Data* data = Model_Billsdeposits::instance().get(item.first);

            wxString payeeStr = "";
            if (Model_Billsdeposits::type(data) == Model_Billsdeposits::TRANSFER)
            {
                const Model_Account::Data *account = Model_Account::instance().get(data->TOACCOUNTID);
                if (account)
                    payeeStr = account->ACCOUNTNAME;
            }
            else
            {
                const Model_Payee::Data* payee = Model_Payee::instance().get(data->PAYEEID);
                if (payee)
                    payeeStr = payee->PAYEENAME;
            }

            colorStr = "#9999FF";
            if (item.second.first < 0)
                colorStr = "#FF6600";

            hb.startTableRow();
            hb.addTableCell(payeeStr, false, true);
            hb.addTableCell(Model_Account::toCurrency(data->TRANSAMOUNT, Model_Account::instance().get(data->ACCOUNTID)), true);
            //Draw it as numeric that mean align right
            hb.addTableCell(item.second.second, true, false, false, colorStr);
            hb.endTableRow();
        }
        hb.endTable();
    }
    return hb.getHTMLinTableWraper(true);
}
