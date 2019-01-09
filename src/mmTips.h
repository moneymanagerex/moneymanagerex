/*******************************************************
Copyright (C) 2013 Nikolay Akimov

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
#pragma once
#include <wx/intl.h>

static const wxString TIPS [] =
{
    wxTRANSLATE("Recommendation: Always backup your .mmb database file regularly."),
    wxTRANSLATE("Recommendation: If upgrading to a new version of MMEX, make sure you backup your .mmb database file before doing so."),
    wxTRANSLATE("Recommendation: Use copy (Ctrl+C) and paste (Ctrl+V) for frequently used transactions."),
    wxTRANSLATE("Tip: Remember to make backups of your .mmb."),
    wxTRANSLATE("Tip: The not encrypted .mmb file is insecure. That means anyone else having the proper know how can actually open the file and read the contents. So make sure that if you are storing any sensitive financial information it is properly guarded."),
    wxTRANSLATE("Tip: To mark a transaction as reconciled, just select the transaction and hit the 'r' or 'R' key. To mark a transaction as unreconciled, just select the transaction and hit the 'u' or 'U' key."),
    wxTRANSLATE("Tip: To mark a transaction as requiring followup, just select the transaction and hit the 'f' or 'F' key."),
    wxTRANSLATE("Tip: MMEX supports printing of all reports that can be viewed. The print options are available under the menu, File->Print."),
    wxTRANSLATE("Tip: You can modify some runtime behavior of MMEX by changing the options in the Options Dialog. "),
    wxTRANSLATE("Tip: To print a statement with transactions from any arbitrary set of criteria, use the transaction filter to select the transactions you want and then do a print from the menu."),
    wxTRANSLATE("Tip: Set exchange rate for currencies in case if you have accounts with different currencies."),

    wxTRANSLATE("Organize Categories Dialog Tip: Pressing the h key will cycle through all categories starting with the letter h"),
    wxTRANSLATE("Organize Categories Dialog Tip: Pressing 2 key combination will cycle through all categories starting with that key combination. Example: Pressing ho will select Homeneeds, Home, House Tax, etc..."),

    wxTRANSLATE("Tip to get out of debt: Pay yourself 10 percentage first. Put this into an account that is hard to touch. Make sure it is a chore to get the money out (you have to drive to the bank), so you will only tap it consciously and for major expenses."),
    wxTRANSLATE("Tip to get out of debt: Establish an emergency fund."),
    wxTRANSLATE("Tip to get out of debt: Stop acquiring new debt."),
    wxTRANSLATE("Tip to get out of debt: Create a realistic budget for your expenses."),
    wxTRANSLATE("Tip to get out of debt: Spend less than you earn."),
    wxTRANSLATE("Tip to get out of debt: Pay more than the minimum."),
    wxTRANSLATE("Before going to a shop and buy something: take the time making a list of what you really need. In the shop buy what is in your list.")
};
