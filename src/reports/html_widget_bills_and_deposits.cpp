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

    //                    days, payee, description, amount, account
    std::vector< std::tuple<int, wxString, wxString, double, Model_Account::Data*> > bd_days;
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

        wxString daysRemainingStr = wxString::Format(_("%d days remaining"), daysRemaining);
        if (daysRemaining == 0)
        {
            if (((repeats > 10) && (repeats < 15)) && (q1.NUMOCCURRENCES < 0))
                continue; // Inactive
        }
        if (daysRemaining < 0)
        {
            daysRemainingStr = wxString::Format(_("%d days overdue!"), abs(daysRemaining));
            if (((repeats > 10) && (repeats < 15)) && (q1.NUMOCCURRENCES < 0))
                continue; // Inactive
        }

        wxString payeeStr = "";
        if (Model_Billsdeposits::type(q1) == Model_Billsdeposits::TRANSFER)
        {
            const Model_Account::Data *account = Model_Account::instance().get(q1.TOACCOUNTID);
            if (account) payeeStr = account->ACCOUNTNAME;
        }
        else
        {
            const Model_Payee::Data* payee = Model_Payee::instance().get(q1.PAYEEID);
            if (payee) payeeStr = payee->PAYEENAME;
        }
        Model_Account::Data *account = Model_Account::instance().get(q1.ACCOUNTID);
        bd_days.push_back(std::make_tuple(daysRemaining, payeeStr, daysRemainingStr, q1.TRANSAMOUNT, account));
    }

    //std::sort(bd_days.begin(), bd_days.end());
    //std::reverse(bd_days.begin(), bd_days.end());
    ////////////////////////////////////

    if (!bd_days.empty())
    {
        wxString colorStr;

        hb.startTable("100%");
        hb.addTableHeaderRowLink("billsdeposits", title_, 3);

        for (const auto& item : bd_days)
        {
            colorStr = "#9999FF";
            if (std::get<0>(item) < 0)
                colorStr = "#FF6600";

            hb.startTableRow();
            hb.addTableCell(std::get<1>(item), false, true); //payee
            hb.addTableCell(Model_Account::toCurrency(std::get<3>(item)
                , std::get<4>(item)), true);
            //Draw it as numeric that mean align right
            hb.addTableCell(std::get<2>(item), true, false, false, colorStr);
            hb.endTableRow();
        }
        hb.endTable();
    }
    return hb.getHTMLinTableWraper(true);
}
