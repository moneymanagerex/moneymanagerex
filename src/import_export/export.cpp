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

#include "export.h"
#include "constants.h"
#include "util.h"
#include "model/Model_Infotable.h"
#include "model/Model_Account.h"
#include "model/Model_Category.h"
#include "model/Model_Payee.h"

mmExportTransaction::mmExportTransaction()
{}

mmExportTransaction::mmExportTransaction(int accountID): mmExportBase()
    , m_account_id(accountID)
{}

mmExportTransaction::mmExportTransaction(int transactionID, int accountID): mmExportBase(transactionID)
    , m_account_id(accountID)
{
    m_transaction_id = transactionID;
}

mmExportTransaction::~mmExportTransaction()
{}

wxString mmExportTransaction::getTransactionQIF(const Model_Checking::Full_Data& full_tran, int accountID)
{
    bool out = full_tran.ACCOUNTID == accountID;

    wxString buffer = "";
    wxString categ = full_tran.m_splits.empty() ? full_tran.CATEGNAME : "";
    wxString transNum = full_tran.TRANSACTIONNUMBER;
    wxString notes = (full_tran.NOTES);
    notes.Replace("''", "'");
    notes.Replace("\n", " ");

    if (Model_Checking::type(full_tran) == Model_Checking::TRANSFER)
    {
        categ = "[" + (!out ? full_tran.ACCOUNTNAME : full_tran.PAYEENAME) + "]";

        //Transaction number used to make transaction unique
        // to proper merge transfer records
        if (transNum.IsEmpty() && notes.IsEmpty())
            transNum = wxString::Format("#%i", full_tran.id());
    }
    
    buffer << "D" << full_tran.TRANSDATE << "\n";
    buffer << "T" << Model_Checking::balance(full_tran, (out ? full_tran.ACCOUNTID : full_tran.TOACCOUNTID)) << "\n";
    if (!full_tran.PAYEENAME.empty())
        buffer << "P" << full_tran.PAYEENAME << "\n";
    if (!transNum.IsEmpty())
        buffer << "N" << transNum << "\n";
    if (!categ.IsEmpty())
        buffer << "L" << categ << "\n";
    if (!notes.IsEmpty())
        buffer << "M" << notes << "\n";

    for (const auto &split_entry : full_tran.m_splits)
    {
        double value = split_entry.SPLITTRANSAMOUNT;
        if (Model_Checking::type(full_tran) == Model_Checking::WITHDRAWAL)
            value = -value;
        const wxString split_amount = wxString() << value;
        const wxString split_categ = Model_Category::full_name(split_entry.CATEGID, split_entry.SUBCATEGID);
        buffer << "S" << split_categ << "\n"
            << "$" << split_amount << "\n";
    }

    buffer << "^" << "\n";
    return buffer;
}

wxString mmExportTransaction::getTransactionCSV(const Model_Checking::Full_Data & full_tran, int accountID)
{
    bool out = full_tran.ACCOUNTID == accountID;

    wxString delimit = Model_Infotable::instance().GetStringInfo("DELIMITER", mmex::DEFDELIMTER);

    wxString buffer = "";
    int trans_id = full_tran.id();
    wxString accountName = (!out ? full_tran.PAYEENAME : full_tran.ACCOUNTNAME);
    wxString categ = full_tran.m_splits.empty() ? full_tran.CATEGNAME : "";
    wxString transNum = full_tran.TRANSACTIONNUMBER;
    wxString notes = (full_tran.NOTES);
    notes.Replace("''", "'");
    notes.Replace("\n", " ");

    if (Model_Checking::type(full_tran) == Model_Checking::TRANSFER)
    {
        categ = "[" + (!out ? full_tran.ACCOUNTNAME : full_tran.PAYEENAME) + "]";

        //Transaction number used to make transaction unique
        // to proper merge transfer records
        if (transNum.IsEmpty() && notes.IsEmpty())
            transNum = wxString::Format("#%i", full_tran.id());
    }

    if (!full_tran.m_splits.empty())
    {
        for (const auto &split_entry : full_tran.m_splits)
        {
            double value = split_entry.SPLITTRANSAMOUNT;
            if (Model_Checking::type(full_tran) == Model_Checking::WITHDRAWAL)
                value = -value;
            const wxString split_amount = wxString() << value;

            const wxString split_categ = Model_Category::full_name(split_entry.CATEGID, split_entry.SUBCATEGID);

            buffer << trans_id << delimit
                << inQuotes(accountName, delimit) << delimit
                << inQuotes(mmGetDateForDisplay(Model_Checking::TRANSDATE(full_tran)), delimit) << delimit
                << inQuotes(full_tran.PAYEENAME, delimit) << delimit
                << full_tran.STATUS << delimit
                << full_tran.TRANSCODE << delimit
                << inQuotes(split_categ, delimit) << delimit
                << inQuotes(split_amount, delimit) << delimit
                << "" << delimit
                << inQuotes(notes, delimit)
                << "\n";
        }

    }
    else
    {
        buffer << trans_id << delimit
            << inQuotes(accountName, delimit) << delimit
            << inQuotes(mmGetDateForDisplay(Model_Checking::TRANSDATE(full_tran)), delimit) << delimit
            << inQuotes(full_tran.PAYEENAME, delimit) << delimit
            << full_tran.STATUS << delimit
            << full_tran.TRANSCODE << delimit
            << inQuotes(categ, delimit) << delimit
            << Model_Checking::balance(full_tran, (out ? full_tran.ACCOUNTID : full_tran.TOACCOUNTID)) << delimit
            << "" << delimit
            << inQuotes(notes, delimit)
            << "\n";
    }
    return buffer;
}

wxString mmExportTransaction::getAccountHeaderQIF()
{
    wxString buffer = "";
    wxString account_name = "";
    wxString currency_symbol = "";
    double dInitBalance = 0;
    Model_Account::Data *account = Model_Account::instance().get(m_account_id);
    if (account)
    {
        account_name = account->ACCOUNTNAME;
        dInitBalance = account->INITIALBAL;
        Model_Currency::Data *currency = Model_Currency::instance().get(account->CURRENCYID);
        if (currency)
        {
            currency_symbol = currency->CURRENCY_SYMBOL;
        }
    }

    wxString currency_code = "[" + currency_symbol + "]";

    const wxString sInitBalance = wxString::Format("%f", dInitBalance);

    buffer = wxString("!Account") << "\n"
        << "N" << account_name <<  "\n"
        << "TBank" << "\n"
        << "D" << currency_code << "\n"
        << (dInitBalance != 0 ? wxString("$") << sInitBalance << "\n" : "")
        << "^" <<  "\n"
        << "!Type:Cash" << "\n";

    return buffer;
}

wxString mmExportTransaction::getCategoriesQIF()
{
    wxString buffer_qif = "";

    buffer_qif << "!Type:Cat" << "\n";
    for (const auto& category: Model_Category::instance().all())
    {
        const wxString& categ_name = category.CATEGNAME;
        bool bIncome = Model_Category::has_income(category.CATEGID);
        buffer_qif << "N" << categ_name <<  "\n"
            << (bIncome ? "I" : "E") << "\n"
            << "^" << "\n";

        for (const auto& sub_category: Model_Category::sub_category(category))
        {
            bIncome = Model_Category::has_income(category.CATEGID, sub_category.SUBCATEGID);
            bool bSubcateg = sub_category.CATEGID != -1;
            wxString full_categ_name = wxString()
                << categ_name << (bSubcateg ? wxString()<<":" : wxString()<<"")
                << sub_category.SUBCATEGNAME;
            buffer_qif << "N" << full_categ_name << "\n"
                << (bIncome ? "I" : "E") << "\n"
                << "^" << "\n";
        }
    }
    return buffer_qif;

}

wxString mmExportTransaction::getCategoriesCSV()
{
    wxString buffer_csv ="";
    wxString delimit = Model_Infotable::instance().GetStringInfo("DELIMITER", mmex::DEFDELIMTER);

    for (const auto& category: Model_Category::instance().all())
    {
        const wxString& categ_name = category.CATEGNAME;
//        bool bIncome = Model_Category::has_income(category.CATEGID);
        buffer_csv << categ_name << delimit << "\n";

        for (const auto& sub_category: Model_Category::sub_category(category))
        {
//            bIncome = Model_Category::has_income(category.CATEGID, sub_category.SUBCATEGID);
            wxString full_categ_name = wxString()
                << categ_name << delimit
                << sub_category.SUBCATEGNAME;
            buffer_csv << full_categ_name << "\n";
        }
    }
    return buffer_csv;
}

