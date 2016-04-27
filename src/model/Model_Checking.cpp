/*******************************************************
 Copyright (C) 2013,2014 Guan Lisheng (guanlisheng@gmail.com)

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

#include "option.h"
#include "Model_Checking.h"
#include "Model_Account.h"
#include "Model_Payee.h"
#include "Model_Category.h"
#include <queue>
#include "Model_Translink.h"

const std::vector<std::pair<Model_Checking::TYPE, wxString> > Model_Checking::TYPE_CHOICES = 
{
    std::make_pair(Model_Checking::WITHDRAWAL, wxString(wxTRANSLATE("Withdrawal")))
    , std::make_pair(Model_Checking::DEPOSIT, wxString(wxTRANSLATE("Deposit")))
    , std::make_pair(Model_Checking::TRANSFER, wxString(wxTRANSLATE("Transfer")))
};

const std::vector<std::pair<Model_Checking::STATUS_ENUM, wxString> > Model_Checking::STATUS_ENUM_CHOICES =
{
    std::make_pair(Model_Checking::NONE, wxTRANSLATE("None"))
    , std::make_pair(Model_Checking::RECONCILED, wxString(wxTRANSLATE("Reconciled")))
    , std::make_pair(Model_Checking::VOID_, wxString(wxTRANSLATE("Void")))
    , std::make_pair(Model_Checking::FOLLOWUP, wxString(wxTRANSLATE("Follow up")))
    , std::make_pair(Model_Checking::DUPLICATE_, wxString(wxTRANSLATE("Duplicate")))
};

Model_Checking::Model_Checking(): Model<DB_Table_CHECKINGACCOUNT_V1>()
{
}

Model_Checking::~Model_Checking() 
{
}

wxArrayString Model_Checking::all_type()
{
    wxArrayString types;
    for (const auto& r : TYPE_CHOICES) types.Add(r.second);

    return types;
}

wxArrayString Model_Checking::all_status()
{
    wxArrayString status;
    for (const auto& r : STATUS_ENUM_CHOICES) status.Add(r.second);

    return status;
}

/**
* Initialize the global Model_Checking table.
* Reset the Model_Checking table or create the table if it does not exist.
*/
Model_Checking& Model_Checking::instance(wxSQLite3Database* db)
{
    Model_Checking& ins = Singleton<Model_Checking>::instance();
    ins.db_ = db;
    ins.destroy_cache();
    ins.ensure(db);

    return ins;
}

/** Return the static instance of Model_Checking table */
Model_Checking& Model_Checking::instance()
{
    return Singleton<Model_Checking>::instance();
}

bool Model_Checking::remove(int id)
{
    //TODO: remove all split at once
    //Model_Splittransaction::instance().remove(Model_Splittransaction::instance().find(Model_Splittransaction::TRANSID(id)));
    for (const auto& r : Model_Splittransaction::instance().find(Model_Splittransaction::TRANSID(id)))
        Model_Splittransaction::instance().remove(r.SPLITTRANSID);
    return this->remove(id, db_);
}

const Model_Splittransaction::Data_Set Model_Checking::splittransaction(const Data* r)
{
    return Model_Splittransaction::instance().find(Model_Splittransaction::TRANSID(r->TRANSID));
}

const Model_Splittransaction::Data_Set Model_Checking::splittransaction(const Data& r)
{
    return Model_Splittransaction::instance().find(Model_Splittransaction::TRANSID(r.TRANSID));
}

DB_Table_CHECKINGACCOUNT_V1::TRANSDATE Model_Checking::TRANSDATE(const wxDate& date, OP op)
{
    return DB_Table_CHECKINGACCOUNT_V1::TRANSDATE(date.FormatISODate(), op);
}

DB_Table_CHECKINGACCOUNT_V1::STATUS Model_Checking::STATUS(STATUS_ENUM status, OP op)
{
    return DB_Table_CHECKINGACCOUNT_V1::STATUS(toShortStatus(all_status()[status]), op);
}

DB_Table_CHECKINGACCOUNT_V1::TRANSCODE Model_Checking::TRANSCODE(TYPE type, OP op)
{
    return DB_Table_CHECKINGACCOUNT_V1::TRANSCODE(all_type()[type], op);
}

wxDate Model_Checking::TRANSDATE(const Data* r)
{
    return Model::to_date(r->TRANSDATE);
}

wxDate Model_Checking::TRANSDATE(const Data& r)
{
    return Model::to_date(r.TRANSDATE);
}

Model_Checking::TYPE Model_Checking::type(const wxString& r)
{
    if (r.empty()) return TYPE::WITHDRAWAL;
    static std::map<wxString, TYPE> cache;
    const auto it = cache.find(r);
    if (it != cache.end()) return it->second;

    for (const auto& t : TYPE_CHOICES) 
    {
        if (r.CmpNoCase(t.second) == 0)
        {
            cache.insert(std::make_pair(r, t.first));
            return t.first;
        }
    }

    cache.insert(std::make_pair(r, TYPE::WITHDRAWAL));
    return TYPE::WITHDRAWAL;
}
Model_Checking::TYPE Model_Checking::type(const Data& r)
{
    return type(r.TRANSCODE);
}
Model_Checking::TYPE Model_Checking::type(const Data* r)
{
    return type(r->TRANSCODE);
}

Model_Checking::STATUS_ENUM Model_Checking::status(const wxString& r)
{
    static std::map<wxString, STATUS_ENUM> cache;
    const auto it = cache.find(r);
    if (it != cache.end()) return it->second;

    for (const auto & s : STATUS_ENUM_CHOICES)
    {
        if (r.CmpNoCase(s.second) == 0) 
        {
            cache.insert(std::make_pair(r, s.first));
            return s.first;
        }
    }

    STATUS_ENUM ret = NONE;
    if (r.CmpNoCase("R") == 0) ret = RECONCILED;
    else if (r.CmpNoCase("V") == 0) ret = VOID_;
    else if (r.CmpNoCase("F") == 0) ret = FOLLOWUP;
    else if (r.CmpNoCase("D") == 0) ret = DUPLICATE_;
    cache.insert(std::make_pair(r, ret));

    return ret;
}
Model_Checking::STATUS_ENUM Model_Checking::status(const Data& r)
{
    return status(r.STATUS);
}
Model_Checking::STATUS_ENUM Model_Checking::status(const Data* r)
{
    return status(r->STATUS);
}

double Model_Checking::amount(const Data* r, int account_id)
{
    double sum = 0;
    switch (type(r->TRANSCODE))
    {
    case WITHDRAWAL:
        sum -= r->TRANSAMOUNT;
        break;
    case DEPOSIT:
        sum += r->TRANSAMOUNT;
        break;
    case TRANSFER:
        if (account_id == r->ACCOUNTID)
            sum -= r->TRANSAMOUNT;
        else
            sum += r->TOTRANSAMOUNT;
        break;
    default:
        break;
    }
    return sum;
}

double Model_Checking::amount(const Data&r, int account_id)
{
    return amount(&r, account_id);
}

double Model_Checking::balance(const Data* r, int account_id)
{
    if (Model_Checking::status(r->STATUS) == Model_Checking::VOID_) return 0;
    return amount(r, account_id);
}

double Model_Checking::balance(const Data& r, int account_id)
{
    return balance(&r, account_id);
}

double Model_Checking::withdrawal(const Data* r, int account_id)
{
    double bal = balance(r, account_id);
    return bal <= 0 ? -bal : 0;
}

double Model_Checking::withdrawal(const Data& r, int account_id)
{
    return withdrawal(&r, account_id);
}

double Model_Checking::deposit(const Data* r, int account_id)
{
    double bal = balance(r, account_id);
    return bal > 0 ? bal : 0;
}

double Model_Checking::deposit(const Data& r, int account_id)
{
    return deposit(&r, account_id);
}

double Model_Checking::reconciled(const Data* r, int account_id)
{
    return (Model_Checking::status(r->STATUS) == Model_Checking::RECONCILED) ? balance(r, account_id) : 0;
}

double Model_Checking::reconciled(const Data& r, int account_id)
{
    return reconciled(&r, account_id);
}

bool Model_Checking::is_transfer(const wxString& r)
{
    return type(r) == Model_Checking::TRANSFER;
}
bool Model_Checking::is_transfer(const Data* r)
{
    return is_transfer(r->TRANSCODE);
}
bool Model_Checking::is_deposit(const wxString& r)
{
    return type(r) == Model_Checking::DEPOSIT;
}
bool Model_Checking::is_deposit(const Data* r)
{
    return is_deposit(r->TRANSCODE);
}

wxString Model_Checking::toShortStatus(const wxString& fullStatus)
{
    wxString s = fullStatus.Left(1);
    s.Replace("N", "");
    return s;
}

Model_Checking::Full_Data::Full_Data() : Data(0), BALANCE(0), AMOUNT(0)
{
}

Model_Checking::Full_Data::Full_Data(const Data& r) : Data(r), BALANCE(0), AMOUNT(0)
    , m_splits(Model_Splittransaction::instance().find(Model_Splittransaction::TRANSID(r.TRANSID)))
{
    ACCOUNTNAME = Model_Account::get_account_name(r.ACCOUNTID);

    if (Model_Checking::type(r) == Model_Checking::TRANSFER)
    {
        TOACCOUNTNAME = Model_Account::get_account_name(r.TOACCOUNTID);
        PAYEENAME = TOACCOUNTNAME;
    }
    else
    {
        PAYEENAME = Model_Payee::get_payee_name(r.PAYEEID);
    }
    
    if (!m_splits.empty())
    {
        for (const auto& entry : m_splits)
            this->CATEGNAME += (this->CATEGNAME.empty() ? " * " : ", ")
            + Model_Category::full_name(entry.CATEGID, entry.SUBCATEGID);
    }
    else
    {
        this->CATEGNAME = Model_Category::instance().full_name(r.CATEGID, r.SUBCATEGID);
    }
}

Model_Checking::Full_Data::Full_Data(const Data& r
    , const std::map<int /*trans id*/, Model_Splittransaction::Data_Set /*split trans*/ > & splits)
    : Data(r), BALANCE(0), AMOUNT(0)
{
    const auto it = splits.find(this->id());
    if (it != splits.end()) m_splits = it->second;

    ACCOUNTNAME = Model_Account::get_account_name(r.ACCOUNTID);
    if (Model_Checking::type(r) == Model_Checking::TRANSFER)
    {
        TOACCOUNTNAME = Model_Account::get_account_name(r.TOACCOUNTID);
        PAYEENAME = TOACCOUNTNAME;
    }
    else
    {
        PAYEENAME = Model_Payee::get_payee_name(r.PAYEEID);
    }
    
    if (!m_splits.empty())
    {
        for (const auto& entry : m_splits)
            this->CATEGNAME += (this->CATEGNAME.empty() ? " * " : ", ")
            + Model_Category::full_name(entry.CATEGID, entry.SUBCATEGID);
    }
    else
    {
        CATEGNAME = Model_Category::full_name(r.CATEGID, r.SUBCATEGID);
    }
}

Model_Checking::Full_Data::~Full_Data()
{
}

wxString Model_Checking::Full_Data::real_payee_name(int account_id) const
{
    if (TYPE::TRANSFER == type(this->TRANSCODE))
    {
        if (this->ACCOUNTID == account_id || account_id == -1)
            return ("> " + this->TOACCOUNTNAME);
        else
            return ("< " + this->ACCOUNTNAME);
    }

    return this->PAYEENAME;
}

bool Model_Checking::Full_Data::has_split() const
{
    return !this->m_splits.empty();
}

wxString Model_Checking::Full_Data::info() const
{
    // TODO more info
    wxDate date = Model_Checking::TRANSDATE(this);
    wxString info = wxGetTranslation(date.GetWeekDayName(date.GetWeekDay()));
    return info;
}

void Model_Checking::getFrequentUsedNotes(std::vector<wxString> &frequentNotes, int accountID)
{
    frequentNotes.clear();
    int max = 20;

    const auto notes = instance().find(NOTES("", NOT_EQUAL)
        , accountID > 0 ? ACCOUNTID(accountID) : ACCOUNTID(-1, NOT_EQUAL));

    std::map <wxString, int> counterMap;
    for (const auto& entry : notes)
        counterMap[entry.NOTES]--;

    std::priority_queue<std::pair<int, wxString> > q; // largest element to appear as the top
    for (const auto & kv: counterMap)
    {
        q.push(std::make_pair(kv.second, kv.first));
        if (q.size() > max) q.pop(); // keep fixed queue as max
    }

    while(!q.empty())
    {
        const auto & kv = q.top();
        frequentNotes.push_back(kv.second);
        q.pop();
    }
}

void Model_Checking::getEmptyTransaction(Data &data, int accountID)
{
    data.TRANSID = -1;
    wxDateTime trx_date = wxDateTime::Today();
    if (Option::instance().TransDateDefault() != 0)
    {
        auto trans = instance().find(ACCOUNTID(accountID), TRANSDATE(trx_date, LESS_OR_EQUAL));
        std::stable_sort(trans.begin(), trans.end(), SorterByTRANSDATE());
        std::reverse(trans.begin(), trans.end());
        if (!trans.empty())
            trx_date = to_date(trans.begin()->TRANSDATE);

        wxDateTime trx_date_b = wxDateTime::Today();
        auto trans_b = instance().find(TOACCOUNTID(accountID), TRANSDATE(trx_date_b, LESS_OR_EQUAL));
        std::stable_sort(trans_b.begin(), trans_b.end(), SorterByTRANSDATE());
        std::reverse(trans_b.begin(), trans_b.end());
        if (!trans_b.empty())
        {
            trx_date_b = to_date(trans_b.begin()->TRANSDATE);
            if (!trans.empty() && (trx_date_b > trx_date))
                trx_date = trx_date_b;
        }
    }

    data.TRANSDATE = trx_date.FormatISODate();
    data.ACCOUNTID = accountID;
    data.STATUS = toShortStatus(all_status()[Option::instance().TransStatusReconciled()]);
    data.TRANSCODE = all_type()[WITHDRAWAL];
    data.CATEGID = -1;
    data.SUBCATEGID = -1;
    data.FOLLOWUPID = -1;
    data.TRANSAMOUNT = 0;
    data.TOTRANSAMOUNT = 0;
    data.TRANSACTIONNUMBER = "";
    if (Option::instance().TransCategorySelectionNone() != 0) 
    {
        auto trx = instance().find(TRANSCODE(TRANSFER, NOT_EQUAL)
            , ACCOUNTID(accountID, EQUAL), TRANSDATE(trx_date, LESS_OR_EQUAL));

        if (!trx.empty())
        {
            std::stable_sort(trx.begin(), trx.end(), SorterByTRANSDATE());
            Model_Payee::Data* payee = Model_Payee::instance().get(trx.rbegin()->PAYEEID);
            if (payee) data.PAYEEID = payee->PAYEEID;
            if (payee && Option::instance().TransCategorySelectionNone() != 0)
            {
                data.CATEGID = payee->CATEGID;
                data.SUBCATEGID = payee->SUBCATEGID;
            }
        }
    }
}

bool Model_Checking::getTransactionData(Data &data, const Data* r)
{
    if (r) {
        data.TRANSDATE = r->TRANSDATE;
        data.STATUS = r->STATUS;
        data.ACCOUNTID = r->ACCOUNTID;
        data.TOACCOUNTID = r->TOACCOUNTID;
        data.TRANSCODE = r->TRANSCODE;
        data.CATEGID = r->CATEGID;
        data.SUBCATEGID = r->SUBCATEGID;
        data.TRANSAMOUNT = r->TRANSAMOUNT;
        data.TOTRANSAMOUNT = r->TOTRANSAMOUNT;
        data.FOLLOWUPID = r->FOLLOWUPID;
        data.NOTES = r->NOTES;
        data.TRANSACTIONNUMBER = r->TRANSACTIONNUMBER;
        data.PAYEEID = r->PAYEEID;
        data.TRANSID = r->TRANSID;
    }
    return r ? true : false;
}

void Model_Checking::putDataToTransaction(Data *r, const Data &data)
{
    r->STATUS = data.STATUS;
    r->TRANSCODE = data.TRANSCODE;
    r->TRANSDATE = data.TRANSDATE;
    r->PAYEEID = data.PAYEEID;
    r->ACCOUNTID = data.ACCOUNTID;
    r->TRANSAMOUNT = data.TRANSAMOUNT;
    r->CATEGID = data.CATEGID;
    r->SUBCATEGID = data.SUBCATEGID;
    r->TOACCOUNTID = data.TOACCOUNTID;
    r->TOTRANSAMOUNT = data.TOTRANSAMOUNT;
    r->NOTES = data.NOTES;
    r->TRANSACTIONNUMBER = data.TRANSACTIONNUMBER;
    r->FOLLOWUPID = data.FOLLOWUPID;
}

const wxString Model_Checking::Full_Data::to_json()
{
    json::Object o;
    Model_Checking::Data::to_json(o);
    o[L"ACCOUNTNAME"] = json::String(this->ACCOUNTNAME.ToStdWstring());
    if (is_transfer(this))
        o[L"TOACCOUNTNAME"] = json::String(this->TOACCOUNTNAME.ToStdWstring());
    else
        o[L"PAYEENAME"] = json::String(this->PAYEENAME.ToStdWstring());

    if (this->has_split())
    {
        json::Array a;
        for (const auto & item : m_splits)
        {
            json::Object s;
            const std::wstring categ = Model_Category::full_name(item.CATEGID, item.SUBCATEGID).ToStdWstring();
            s[categ] = json::Number(item.SPLITTRANSAMOUNT);
            a.Insert(s);
        }
        o[L"CATEGS"] = json::Array(a);
    }
    else
        o[L"CATEG"] = json::String(Model_Category::full_name(this->CATEGID, this->SUBCATEGID).ToStdWstring());

    std::wstringstream ss;
    json::Writer::Write(o, ss);

    return ss.str();
}

const bool Model_Checking::foreignTransaction(const Data& data)
{
    return (data.TOACCOUNTID > 0) && ((data.TRANSCODE == all_type()[DEPOSIT]) || (data.TRANSCODE == all_type()[WITHDRAWAL]));
}

const bool Model_Checking::foreignTransactionAsTransfer(const Data& data)
{
    return foreignTransaction(data) && (data.TOACCOUNTID == Model_Translink::AS_TRANSFER);
}
