/*******************************************************
  Copyright (C) 2013 Nikolay
 ********************************************************/

#include "export.h"
#include "constants.h"
#include "util.h"
#include "model/Model_Infotable.h"
#include "model/Model_Account.h"
#include "model/Model_Category.h"

mmExportTransaction::mmExportTransaction(mmCoreDB* core)
    : mmExportBase(core)
    , pBankTransaction_(0)
    , accountID_(-1)
{}

mmExportTransaction::mmExportTransaction(mmCoreDB* core, mmBankTransaction* pBankTransaction)
    : mmExportBase(core)
    , pBankTransaction_(pBankTransaction)
    , accountID_(pBankTransaction->accountID_)
{}

mmExportTransaction::mmExportTransaction(mmCoreDB* core, int accountID)
    : mmExportBase(core)
    , pBankTransaction_(0)
    , accountID_(accountID)
{
    if (!core) wxASSERT(false);
}

mmExportTransaction::~mmExportTransaction()
{}

wxString mmExportTransaction::getTransactionQIF(bool from)
{
    mmBankTransaction* &transaction = pBankTransaction_;
    wxString buffer = "";
    int trans_id = transaction->transactionID();
    wxString accountName = "";
    Model_Account::Data *account = Model_Account::instance().get(transaction->accountID_);
    if (account) accountName = account->ACCOUNTNAME;
    wxString categ = transaction->fullCatStr_;
    wxString payee = transaction->payeeStr_;
    wxString transNum = transaction->transNum_;
    wxString notes = (transaction->notes_);
    notes.Replace("''", "'");
    notes.Replace("\n", " ");

    if (transaction->transType_ == TRANS_TYPE_TRANSFER_STR)
    {
        categ = wxString::Format("[%s]", from ? accountName : payee);
        payee = "";

        //Transaction number used to make transaction unique
        // to proper merge transfer records
        if (transNum.IsEmpty() && notes.IsEmpty())
            transNum = wxString::Format("#%i", trans_id);
    }

    buffer << "D" << mmGetDateForDisplay(transaction->date_) << "\n";
    buffer << "T" << transaction->value(!from ? transaction->accountID_ : transaction->toAccountID_)  << "\n";
    if (!payee.IsEmpty())
        buffer << "P" << payee << "\n";
    if (!transNum.IsEmpty())
        buffer << "N" << transNum << "\n";
    if (!categ.IsEmpty())
        buffer << "L" << categ << "\n";
    if (!notes.IsEmpty())
        buffer << "M" << notes << "\n";

    //if categ id is empty that mean this is split transaction
    if (transaction->categID_ == -1)
    {
        mmSplitTransactionEntries* splits = transaction->splitEntries_;
        transaction->getSplitTransactions(splits);

        for (const auto &split_entry : splits->entries_)
        {
            double value = split_entry->splitAmount_;
            if (transaction->transType_ == "Withdrawal")
                value = -value;
            const wxString split_amount = wxString()<<value;
            const wxString split_categ = core_->categoryList_.GetFullCategoryString(
                split_entry->categID_, split_entry->subCategID_);
            buffer << "S" << split_categ << "\n"
                << "$" << split_amount << "\n";
        }
    }

    buffer << "^" << "\n";
    return buffer;
}

wxString mmExportTransaction::getTransactionCSV(bool from)
{
    mmBankTransaction* &transaction = pBankTransaction_;
    wxString delimit = Model_Infotable::instance().GetStringInfo("DELIMITER", mmex::DEFDELIMTER);
    wxString buffer = "";
    wxString accountName = "";
    Model_Account::Data *account = Model_Account::instance().get(transaction->accountID_);
    if (account) accountName = account->ACCOUNTNAME;
    int trans_id = transaction->transactionID();
    wxString categ = transaction->fullCatStr_;
    wxString payee = transaction->payeeStr_;
    wxString transNum = transaction->transNum_;
    wxString notes = (transaction->notes_);
    notes.Replace("''", "'");
    notes.Replace("\n", " ");

    if (transaction->transType_ == TRANS_TYPE_TRANSFER_STR)
    {
        categ = wxString::Format("[%s]", from ? accountName : payee);
        payee = "";
        //Transaction number used to make transaction unique
        // to proper merge transfer records
        if (transNum.IsEmpty() && notes.IsEmpty())
            transNum = wxString::Format("#%i", trans_id);
    }

    if (transaction->categID_ == -1)
    {
        mmSplitTransactionEntries* splits = transaction->splitEntries_;
        transaction->getSplitTransactions(splits);

        for (const auto &split_entry : splits->entries_)
        {
            double value = split_entry->splitAmount_;
            if (transaction->transType_ == "Withdrawal")
                value = -value;
            const wxString split_amount = wxString()<<value;

            const wxString split_categ = core_->categoryList_.GetFullCategoryString(
                split_entry->categID_, split_entry->subCategID_);

            buffer << trans_id << delimit
                << inQuotes(accountName, delimit) << delimit
                << inQuotes(mmGetDateForDisplay(transaction->date_), delimit) << delimit
                << inQuotes(payee, delimit) << delimit
                << transaction->status_ << delimit
                << transaction->transType_ << delimit
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
            << inQuotes(mmGetDateForDisplay(transaction->date_), delimit) << delimit
            << inQuotes(payee, delimit) << delimit
            << transaction->status_ << delimit
            << transaction->transType_ << delimit
            << inQuotes(categ, delimit) << delimit
            << transaction->value(!from ? transaction->accountID_ : transaction->toAccountID_) << delimit
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
    Model_Account::Data *account = Model_Account::instance().get(accountID_);
    if (account)
    {
        account_name = account->ACCOUNTNAME;
        dInitBalance = account->INITIALBAL;
    }

    Model_Currency::Data *currency = Model_Currency::instance().get(account->CURRENCYID);
    if (currency)
    {
        currency_symbol = currency->CURRENCY_SYMBOL;
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
        bool bIncome = Model_Category::has_income(category.CATEGID);
        buffer_csv << categ_name << delimit << "\n";

        for (const auto& sub_category: Model_Category::sub_category(category))
        {
            bIncome = Model_Category::has_income(category.CATEGID, sub_category.SUBCATEGID);
            wxString full_categ_name = wxString()
                << categ_name << delimit
                << sub_category.SUBCATEGNAME;
            buffer_csv << full_categ_name << "\n";
        }
    }
    return buffer_csv;
}

