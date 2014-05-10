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
    wxString output = "";

    //                    days, payee, description, amount, account
    std::vector< std::tuple<int, wxString, wxString, double, Model_Account::Data*> > bd_days;
    for (const auto& q1 : Model_Billsdeposits::instance().all(Model_Billsdeposits::COL_NEXTOCCURRENCEDATE))
    {
        int daysRemaining = Model_Billsdeposits::instance().daysRemaining(&q1);
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
        double amount = (Model_Billsdeposits::type(q1) == Model_Billsdeposits::DEPOSIT ? q1.TRANSAMOUNT : -q1.TRANSAMOUNT);
        bd_days.push_back(std::make_tuple(daysRemaining, payeeStr, daysRemainingStr, amount, account));
    }

    //std::sort(bd_days.begin(), bd_days.end());
    //std::reverse(bd_days.begin(), bd_days.end());
    ////////////////////////////////////

    if (!bd_days.empty())
    {
        
        static const wxString FUNCTION =
            "<script> function toggleBills()\n"
            "{\n"
            "    var elem = document.getElementById(\"%s\");\n"
            "    var label = document.getElementById(\"bils_label\");\n"
            "    var hide = elem.style.display == \"none\";\n"
            "    if (hide) {\n"
            "        elem.style.display = \"\";\n"
            "        label.innerHTML = \"[-]\";\n"
            "    }\n"
            "    else {\n"
            "        elem.style.display = \"none\";\n"
            "        label.innerHTML = \"[+]\";\n"
            "    }\n"
            "}\n"
            "</script>\n";
        const wxString id = "BILLS_AND_DEPOSITS";

        output = "<table class=\"table\"><thead><tr class='active'><th>";
        output += wxString::Format("<a href=\"billsdeposits:\">%s</a></th>\n<th></th>", title_);
        output += wxString::Format("<th class='text-right'>%i <a id=\"bils_label\" onclick=\"toggleBills(); \" href=\"#\">[-]</a></th></tr>\n"
            , int(bd_days.size()));
        output += "</thead>";

        output += wxString::Format("<tbody id='%s'>", id);
        output += wxString::Format("<tr style='background-color: #d8ebf0'><th>%s</th><th class='text-right'>%s</th><th class='text-right'>%s</th></tr>"
            , _("Payee"), _("Amount"), _("Days"));

        for (const auto& item : bd_days)
        {
            output += wxString::Format("<tr %s>\n", std::get<0>(item) < 0 ? "class='danger'" : "");
            output += "<td>" + std::get<1>(item) +"</td>"; //payee
            output += wxString::Format("<td class = \"text-right\">%s</td>"
                , Model_Account::toCurrency(std::get<3>(item), std::get<4>(item))); 
            output += "<td  class = 'text-right'>" + std::get<2>(item) + "</td></tr>\n";
        }
        output += "</tbody></table>\n";
        output += wxString::Format(FUNCTION, id);
    }
    return output;
}
