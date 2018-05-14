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
#include "Model_Checking.h"
#include "Model_Account.h"
#include "Model_Currency.h"
#include "Model_Category.h"

mmExportTransaction::mmExportTransaction()
{}

mmExportTransaction::~mmExportTransaction()
{}

const wxString mmExportTransaction::getTransactionQIF(const Model_Checking::Full_Data& full_tran
    , const wxString& dateMask, bool reverce)
{
    bool transfer = Model_Checking::is_transfer(full_tran.TRANSCODE);

    wxString buffer = "";
    wxString categ = full_tran.m_splits.empty() ? full_tran.CATEGNAME : "";
    wxString transNum = full_tran.TRANSACTIONNUMBER;
    wxString notes = (full_tran.NOTES);
    wxString payee = full_tran.PAYEENAME;

    if (transfer)
    {
        const auto acc_in = Model_Account::instance().get(full_tran.ACCOUNTID);
        const auto acc_to = Model_Account::instance().get(full_tran.TOACCOUNTID);
        const auto curr_in = Model_Currency::instance().get(acc_in->CURRENCYID);
        const auto curr_to = Model_Currency::instance().get(acc_to->CURRENCYID);

        categ = "[" + (reverce ? full_tran.ACCOUNTNAME : full_tran.TOACCOUNTNAME) + "]";
        payee = wxString::Format("%s %s %s -> %s %s %s"
            , wxString::FromCDouble(full_tran.TRANSAMOUNT, 2), curr_in->CURRENCY_SYMBOL, acc_in->ACCOUNTNAME
            , wxString::FromCDouble(full_tran.TOTRANSAMOUNT, 2), curr_to->CURRENCY_SYMBOL, acc_to->ACCOUNTNAME);
        //Transaction number used to make transaction unique
        // to proper merge transfer records
        if (transNum.IsEmpty() && notes.IsEmpty())
            transNum = wxString::Format("#%i", full_tran.id());
    }
    
    buffer << "D" << Model_Checking::TRANSDATE(full_tran).Format(dateMask) << "\n";
    buffer << "C" << (full_tran.STATUS == "R" ? "R" : "") << "\n";
    double value = Model_Checking::balance(full_tran
        , (reverce ? full_tran.TOACCOUNTID : full_tran.ACCOUNTID));
    const wxString& s = wxString::FromCDouble(value, 2);
    buffer << "T" << s << "\n";
    if (!payee.empty())
        buffer << "P" << payee << "\n";
    if (!transNum.IsEmpty())
        buffer << "N" << transNum << "\n";
    if (!categ.IsEmpty())
        buffer << "L" << categ << "\n";
    if (!notes.IsEmpty())
    {
        notes.Replace("''", "'");
        notes.Replace("\n", "\nM");
        buffer << "M" << notes << "\n";
    }

    for (const auto &split_entry : full_tran.m_splits)
    {
        double valueSplit = split_entry.SPLITTRANSAMOUNT;
        if (Model_Checking::type(full_tran) == Model_Checking::WITHDRAWAL)
            valueSplit = -valueSplit;
        const wxString split_amount = wxString::FromCDouble(valueSplit, 2);
        const wxString split_categ = Model_Category::full_name(split_entry.CATEGID, split_entry.SUBCATEGID);
        buffer << "S" << split_categ << "\n"
            << "$" << split_amount << "\n";
    }

    buffer << "^" << "\n";
    return buffer;
}

const wxString mmExportTransaction::getAccountHeaderQIF(int accountID)
{
    wxString buffer = "";
    wxString currency_symbol = Model_Currency::GetBaseCurrency()->CURRENCY_SYMBOL;
    Model_Account::Data *account = Model_Account::instance().get(accountID);
    if (account)
    {
        double dInitBalance = account->INITIALBAL;
        Model_Currency::Data *currency = Model_Currency::instance().get(account->CURRENCYID);
        if (currency)
        {
            currency_symbol = currency->CURRENCY_SYMBOL;
        }

        const wxString currency_code = "[" + currency_symbol + "]";
        const wxString sInitBalance = Model_Currency::toString(dInitBalance, currency);

        buffer = wxString("!Account") << "\n"
            << "N" << account->ACCOUNTNAME << "\n"
            << "T" << qif_acc_type(account->ACCOUNTTYPE) << "\n"
            << "D" << currency_code << "\n"
            << (dInitBalance != 0 ? wxString::Format("$%s\n", sInitBalance) : "")
            << "^" << "\n"
            << "!Type:Cash" << "\n";
    }

    return buffer;
}

const wxString mmExportTransaction::getCategoriesQIF()
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

//map Quicken !Account type strings to Model_Account::TYPE
// (not sure whether these need to be translated)
const std::unordered_map<wxString, int> mmExportTransaction::m_QIFaccountTypes =
{
    std::make_pair(wxString("Cash"), Model_Account::CASH), //Cash Flow: Cash Account
    std::make_pair(wxString("Bank"), Model_Account::CHECKING), //Cash Flow: Checking Account
    std::make_pair(wxString("CCard"), Model_Account::CREDIT_CARD), //Cash Flow: Credit Card Account
    std::make_pair(wxString("Invst"), Model_Account::INVESTMENT), //Investing: Investment Account
    std::make_pair(wxString("Oth A"), Model_Account::CHECKING), //Property & Debt: Asset
    std::make_pair(wxString("Oth L"), Model_Account::CHECKING), //Property & Debt: Liability
    std::make_pair(wxString("Invoice"), Model_Account::CHECKING), //Invoice (Quicken for Business only)
};

const wxString mmExportTransaction::qif_acc_type(const wxString& mmex_type)
{
    wxString qif_acc_type = m_QIFaccountTypes.begin()->first;
    for (const auto &item : m_QIFaccountTypes)
    {
        if (item.second == Model_Account::all_type().Index(mmex_type))
        {
            qif_acc_type = item.first;
            break;
        }
    }
    return qif_acc_type;
}

const wxString mmExportTransaction::mm_acc_type(const wxString& qif_type)
{
    wxString mm_acc_type = Model_Account::all_type()[Model_Account::CASH];
    for (const auto &item : m_QIFaccountTypes)
    {
        if (item.first == qif_type)
        {
            mm_acc_type = Model_Account::all_type()[(item.second)];
            break;
        }
    }
    return mm_acc_type;
}
