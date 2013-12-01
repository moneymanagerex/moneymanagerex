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

mmExportTransaction::mmExportTransaction(int accountID)
: m_account_id(accountID)
{}

mmExportTransaction::mmExportTransaction(int transactionID, int accountID)
: m_account_id(accountID)
{
    m_transaction_id = transactionID;
}

mmExportTransaction::~mmExportTransaction()
{}

wxString mmExportTransaction::getTransactionQIF()
{
    Model_Checking::Data *transaction = Model_Checking::instance().get(m_transaction_id);
    if (!transaction) return "";
    Model_Checking::Full_Data full_tran(*transaction);
    bool out = transaction->ACCOUNTID == m_account_id;

    const Model_Account::Data* account = Model_Account::instance().get(transaction->ACCOUNTID);
    if (account) full_tran.ACCOUNTNAME = account->ACCOUNTNAME;
    if (Model_Checking::type(transaction) == Model_Checking::TRANSFER)
    {
        account = Model_Account::instance().get(transaction->TOACCOUNTID);
        if (account) full_tran.PAYEENAME = account->ACCOUNTNAME;
    }
    else
    {
        const Model_Payee::Data* payee = Model_Payee::instance().get(transaction->PAYEEID);
        if (payee) full_tran.PAYEENAME = payee->PAYEENAME;
    }
    full_tran.CATEGNAME = Model_Category::instance().full_name(transaction->CATEGID, transaction->SUBCATEGID);

    wxString buffer = "";
    int trans_id = transaction->TRANSID;
    wxString accountName = full_tran.ACCOUNTNAME;
    wxString categ = full_tran.CATEGNAME;
    wxString payee = full_tran.PAYEENAME;
    wxString transNum = full_tran.TRANSACTIONNUMBER;
    wxString notes = (full_tran.NOTES);
    notes.Replace("''", "'");
    notes.Replace("\n", " ");

    if (Model_Checking::type(transaction) == Model_Checking::TRANSFER)
    {
        categ = "[" + (!out ? accountName : payee) + "]";
        payee = "";

        //Transaction number used to make transaction unique
        // to proper merge transfer records
        if (transNum.IsEmpty() && notes.IsEmpty())
            transNum = wxString::Format("#%i", trans_id);
    }
    
    buffer << "D" << full_tran.TRANSDATE << "\n";
    buffer << "T" << Model_Checking::balance(*transaction, (out ? transaction->ACCOUNTID : transaction->TOACCOUNTID)) << "\n";
    if (!payee.IsEmpty())
        buffer << "P" << payee << "\n";
    if (!transNum.IsEmpty())
        buffer << "N" << transNum << "\n";
    if (!categ.IsEmpty())
        buffer << "L" << categ << "\n";
    if (!notes.IsEmpty())
        buffer << "M" << notes << "\n";

    Model_Splittransaction::Data_Set splits = Model_Checking::splittransaction(transaction);
    if (!splits.empty())
    {
        for (const auto &split_entry : splits)
        {
            double value = split_entry.SPLITTRANSAMOUNT;
            if (Model_Checking::type(transaction) == Model_Checking::WITHDRAWAL)
                value = -value;
            const wxString split_amount = wxString()<<value;
            const wxString split_categ = Model_Category::full_name(split_entry.CATEGID, split_entry.SUBCATEGID);
            buffer << "S" << split_categ << "\n"
                << "$" << split_amount << "\n";
        }
    }

    buffer << "^" << "\n";
    return buffer;
}

wxString mmExportTransaction::getTransactionCSV()
{
    Model_Checking::Data *transaction = Model_Checking::instance().get(m_transaction_id);
    if (!transaction) return "";
    Model_Checking::Full_Data full_tran(*transaction);
    bool out = transaction->ACCOUNTID == m_account_id;

    const Model_Account::Data* account = Model_Account::instance().get(transaction->ACCOUNTID);
    if (account) full_tran.ACCOUNTNAME = account->ACCOUNTNAME;
    if (Model_Checking::type(transaction) == Model_Checking::TRANSFER)
    {
        account = Model_Account::instance().get(transaction->TOACCOUNTID);
        if (account) full_tran.PAYEENAME = account->ACCOUNTNAME;
    }
    else
    {
        const Model_Payee::Data* payee = Model_Payee::instance().get(transaction->PAYEEID);
        if (payee) full_tran.PAYEENAME = payee->PAYEENAME;
    }
    full_tran.CATEGNAME = Model_Category::instance().full_name(transaction->CATEGID, transaction->SUBCATEGID);

    wxString delimit = Model_Infotable::instance().GetStringInfo("DELIMITER", mmex::DEFDELIMTER);

    wxString buffer = "";
    int trans_id = transaction->TRANSID;
    wxString accountName = (!out ? full_tran.PAYEENAME : full_tran.ACCOUNTNAME);
    wxString categ = full_tran.CATEGNAME;
    wxString payee = full_tran.PAYEENAME;
    wxString transNum = full_tran.TRANSACTIONNUMBER;
    wxString notes = (full_tran.NOTES);
    notes.Replace("''", "'");
    notes.Replace("\n", " ");

    if (Model_Checking::type(transaction) == Model_Checking::TRANSFER)
    {
        categ = "[" + (!out ? full_tran.ACCOUNTNAME : payee) + "]";
        payee = "";

        //Transaction number used to make transaction unique
        // to proper merge transfer records
        if (transNum.IsEmpty() && notes.IsEmpty())
            transNum = wxString::Format("#%i", trans_id);
    }

    Model_Splittransaction::Data_Set splits = Model_Checking::splittransaction(transaction);
    if (!splits.empty())
    {
        for (const auto &split_entry : splits)
        {
            double value = split_entry.SPLITTRANSAMOUNT;
            if (Model_Checking::type(transaction) == Model_Checking::WITHDRAWAL)
                value = -value;
            const wxString split_amount = wxString() << value;

            const wxString split_categ = Model_Category::full_name(split_entry.CATEGID, split_entry.SUBCATEGID);

            buffer << trans_id << delimit
                << inQuotes(accountName, delimit) << delimit
                << inQuotes(mmGetDateForDisplay(Model_Checking::TRANSDATE(transaction)), delimit) << delimit
                << inQuotes(payee, delimit) << delimit
                << transaction->STATUS << delimit
                << transaction->TRANSCODE << delimit
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
            << inQuotes(mmGetDateForDisplay(Model_Checking::TRANSDATE(transaction)), delimit) << delimit
            << inQuotes(payee, delimit) << delimit
            << transaction->STATUS << delimit
            << transaction->TRANSCODE << delimit
            << inQuotes(categ, delimit) << delimit
            << Model_Checking::balance(*transaction, (out ? transaction->ACCOUNTID : transaction->TOACCOUNTID)) << delimit
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

