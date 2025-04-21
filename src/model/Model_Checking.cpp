/*******************************************************
 Copyright (C) 2013,2014 Guan Lisheng (guanlisheng@gmail.com)
 Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)

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
#include "Model_Tag.h"
#include "Model_Translink.h"
#include "Model_CustomFieldData.h"
#include "attachmentdialog.h"
#include "util.h"

ChoicesName Model_Checking::TYPE_CHOICES = ChoicesName({
    { TYPE_ID_WITHDRAWAL, _n("Withdrawal") },
    { TYPE_ID_DEPOSIT,    _n("Deposit") },
    { TYPE_ID_TRANSFER,   _n("Transfer") }
});

ChoicesName Model_Checking::TRADE_TYPE_CHOICES = ChoicesName({
    { TYPE_ID_WITHDRAWAL,  _n("Buy") },
    { TYPE_ID_DEPOSIT, _n("Sell") },
    { TYPE_ID_TRANSFER, _n("Revalue") }
});

ChoicesKeyName Model_Checking::STATUS_CHOICES = ChoicesKeyName({
    { STATUS_ID_NONE,       "",  _n("Unreconciled") },
    { STATUS_ID_RECONCILED, "R", _n("Reconciled") },
    { STATUS_ID_VOID,       "V", _n("Void") },
    { STATUS_ID_FOLLOWUP,   "F", _n("Follow Up") },
    { STATUS_ID_DUPLICATE,  "D", _n("Duplicate") }
});

const wxString Model_Checking::TYPE_NAME_WITHDRAWAL = type_name(TYPE_ID_WITHDRAWAL);
const wxString Model_Checking::TYPE_NAME_DEPOSIT    = type_name(TYPE_ID_DEPOSIT);
const wxString Model_Checking::TYPE_NAME_TRANSFER   = type_name(TYPE_ID_TRANSFER);

const wxString Model_Checking::STATUS_KEY_NONE       = status_key(STATUS_ID_NONE);
const wxString Model_Checking::STATUS_KEY_RECONCILED = status_key(STATUS_ID_RECONCILED);
const wxString Model_Checking::STATUS_KEY_VOID       = status_key(STATUS_ID_VOID);
const wxString Model_Checking::STATUS_KEY_FOLLOWUP   = status_key(STATUS_ID_FOLLOWUP);
const wxString Model_Checking::STATUS_KEY_DUPLICATE  = status_key(STATUS_ID_DUPLICATE);

const wxString Model_Checking::STATUS_NAME_NONE       = status_name(STATUS_ID_NONE);
const wxString Model_Checking::STATUS_NAME_RECONCILED = status_name(STATUS_ID_RECONCILED);
const wxString Model_Checking::STATUS_NAME_VOID       = status_name(STATUS_ID_VOID);
const wxString Model_Checking::STATUS_NAME_FOLLOWUP   = status_name(STATUS_ID_FOLLOWUP);
const wxString Model_Checking::STATUS_NAME_DUPLICATE  = status_name(STATUS_ID_DUPLICATE);

Model_Checking::Model_Checking() : Model<DB_Table_CHECKINGACCOUNT_V1>()
{
}

Model_Checking::~Model_Checking()
{
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

bool Model_Checking::remove(int64 id)
{
    //TODO: remove all split at once
    //Model_Splittransaction::instance().remove(Model_Splittransaction::instance().find(Model_Splittransaction::TRANSID(id)));
    for (const auto& r : Model_Splittransaction::instance().find(Model_Splittransaction::TRANSID(id)))
        Model_Splittransaction::instance().remove(r.SPLITTRANSID);
    if(foreignTransaction(*instance().get(id))) Model_Translink::RemoveTranslinkEntry(id);

    const wxString& RefType = Model_Attachment::REFTYPE_NAME_TRANSACTION;
    // remove all attachments
    mmAttachmentManage::DeleteAllAttachments(RefType, id);
    // remove all custom fields for the transaction
    Model_CustomFieldData::DeleteAllData(RefType, id);
    Model_Taglink::instance().DeleteAllTags(RefType, id);
    return this->remove(id, db_);
}

int64 Model_Checking::save(Data* r)
{
    wxSharedPtr<Data> oldData(instance().get_record(r->TRANSID));
    if (!oldData || (!oldData->equals(r) && oldData->DELETEDTIME.IsEmpty() && r->DELETEDTIME.IsEmpty()))
        r->LASTUPDATEDTIME = wxDateTime::Now().ToUTC().FormatISOCombined();
    this->save(r, db_);
    return r->TRANSID;
}

int Model_Checking::save(std::vector<Data>& rows)
{
    this->Savepoint();
    for (auto& r : rows)
    {
        if (r.id() < 0)
            wxLogDebug("Incorrect function call to save %s", r.to_json().utf8_str());
        save(&r);
    }
    this->ReleaseSavepoint();

    return rows.size();
}

int Model_Checking::save(std::vector<Data*>& rows)
{
    this->Savepoint();
    for (auto& r : rows)
    {
        if (r->id() < 0)
            wxLogDebug("Incorrect function call to save %s", r->to_json().utf8_str());
        save(r);
    }
    this->ReleaseSavepoint();

    return rows.size();
}

const Model_Checking::Data_Set Model_Checking::allByDateTimeId()
{
    auto trans = Model_Checking::instance().all();
    std::sort(trans.begin(), trans.end());
    if (Option::instance().UseTransDateTime())
        std::stable_sort(trans.begin(), trans.end(), SorterByTRANSDATE());
    else
        std::stable_sort(trans.begin(), trans.end(), Model_Checking::SorterByTRANSDATE_DATE());
    return trans;
}

const Model_Splittransaction::Data_Set Model_Checking::split(const Data* r)
{
    return Model_Splittransaction::instance().find(Model_Splittransaction::TRANSID(r->TRANSID));
}

const Model_Splittransaction::Data_Set Model_Checking::split(const Data& r)
{
    return Model_Splittransaction::instance().find(Model_Splittransaction::TRANSID(r.TRANSID));
}

DB_Table_CHECKINGACCOUNT_V1::STATUS Model_Checking::STATUS(STATUS_ID status, OP op)
{
    return DB_Table_CHECKINGACCOUNT_V1::STATUS(status_key(status), op);
}

DB_Table_CHECKINGACCOUNT_V1::TRANSCODE Model_Checking::TRANSCODE(TYPE_ID type, OP op)
{
    return DB_Table_CHECKINGACCOUNT_V1::TRANSCODE(type_name(type), op);
}

DB_Table_CHECKINGACCOUNT_V1::TRANSACTIONNUMBER Model_Checking::TRANSACTIONNUMBER(const wxString& num, OP op)
{
    return DB_Table_CHECKINGACCOUNT_V1::TRANSACTIONNUMBER(num, op);
}

DB_Table_CHECKINGACCOUNT_V1::TRANSDATE Model_Checking::TRANSDATE(const wxDateTime& date, OP op)
{
    if (date.FormatISOTime() == "00:00:00")
        return DB_Table_CHECKINGACCOUNT_V1::TRANSDATE(date.FormatISODate(), op);
    return TRANSDATE(date.FormatISOCombined(), op);
}

DB_Table_CHECKINGACCOUNT_V1::DELETEDTIME Model_Checking::DELETEDTIME(const wxString& date, OP op)
{
    return DB_Table_CHECKINGACCOUNT_V1::DELETEDTIME(date, op);
}

DB_Table_CHECKINGACCOUNT_V1::TRANSDATE Model_Checking::TRANSDATE(const wxString& date_iso_str, OP op)
{
    return DB_Table_CHECKINGACCOUNT_V1::TRANSDATE(date_iso_str, op);
}

wxDateTime Model_Checking::TRANSDATE(const Data* r)
{
    return Model::to_date(r->TRANSDATE);
}

wxDateTime Model_Checking::TRANSDATE(const Data& r)
{
    return Model::to_date(r.TRANSDATE);
}

double Model_Checking::account_flow(const Data* r, int64 account_id)
{
    if (r->ACCOUNTID == r->TOACCOUNTID && type_id(r->TRANSCODE) == TYPE_ID_TRANSFER)
        return 0.0;  // Self Transfer as Revaluation
    if (Model_Checking::status_id(r->STATUS) == Model_Checking::STATUS_ID_VOID || !r->DELETEDTIME.IsEmpty())
        return 0.0;
    if (account_id == r->ACCOUNTID && type_id(r->TRANSCODE) == TYPE_ID_WITHDRAWAL)
        return -(r->TRANSAMOUNT);
    if (account_id == r->ACCOUNTID && type_id(r->TRANSCODE) == TYPE_ID_DEPOSIT)
        return r->TRANSAMOUNT;
    if (account_id == r->ACCOUNTID && type_id(r->TRANSCODE) == TYPE_ID_TRANSFER)
        return -(r->TRANSAMOUNT);
    if (account_id == r->TOACCOUNTID && type_id(r->TRANSCODE) == TYPE_ID_TRANSFER)
        return r->TOTRANSAMOUNT;
    return 0.0;
}

double Model_Checking::account_flow(const Data& r, int64 account_id)
{
    return account_flow(&r, account_id);
}

double Model_Checking::account_outflow(const Data* r, int64 account_id)
{
    double bal = account_flow(r, account_id);
    return bal <= 0 ? -bal : 0;
}

double Model_Checking::account_outflow(const Data& r, int64 account_id)
{
    return account_outflow(&r, account_id);
}

double Model_Checking::account_inflow(const Data* r, int64 account_id)
{
    double bal = account_flow(r, account_id);
    return bal >= 0 ? bal : 0;
}

double Model_Checking::account_inflow(const Data& r, int64 account_id)
{
    return account_inflow(&r, account_id);
}

double Model_Checking::account_recflow(const Data* r, int64 account_id)
{
    return (Model_Checking::status_id(r->STATUS) == Model_Checking::STATUS_ID_RECONCILED) ?
        account_flow(r, account_id) : 0;
}

double Model_Checking::account_recflow(const Data& r, int64 account_id)
{
    return account_recflow(&r, account_id);
}

bool Model_Checking::is_locked(const Data* r)
{
    bool val = false;
    Model_Account::Data* acc = Model_Account::instance().get(r->ACCOUNTID);

    if (Model_Account::BoolOf(acc->STATEMENTLOCKED))
    {
        wxDateTime transaction_date;
        if (transaction_date.ParseDate(r->TRANSDATE))
        {
            if (transaction_date <= Model_Account::DateOf(acc->STATEMENTDATE))
            {
                val = true;
            }
        }
    }
    return val;
}


bool Model_Checking::is_transfer(const wxString& r)
{
    return type_id(r) == Model_Checking::TYPE_ID_TRANSFER;
}
bool Model_Checking::is_transfer(const Data* r)
{
    return is_transfer(r->TRANSCODE);
}
bool Model_Checking::is_deposit(const wxString& r)
{
    return type_id(r) == Model_Checking::TYPE_ID_DEPOSIT;
}
bool Model_Checking::is_deposit(const Data* r)
{
    return is_deposit(r->TRANSCODE);
}

Model_Checking::Full_Data::Full_Data() :
    Data(0), TAGNAMES(""),
    ACCOUNTID_W(-1), ACCOUNTID_D(-1), TRANSAMOUNT_W(0), TRANSAMOUNT_D(0),
    SN(0), ACCOUNT_FLOW(0), ACCOUNT_BALANCE(0)
{
}

Model_Checking::Full_Data::Full_Data(const Data& r) :
    Data(r),
    m_splits(Model_Splittransaction::instance().find(
        Model_Splittransaction::TRANSID(r.TRANSID))),
    m_tags(Model_Taglink::instance().find(
        Model_Taglink::REFTYPE(Model_Attachment::REFTYPE_NAME_TRANSACTION),
        Model_Taglink::REFID(r.TRANSID))),
    ACCOUNTID_W(-1), ACCOUNTID_D(-1), TRANSAMOUNT_W(0), TRANSAMOUNT_D(0),
    SN(0), ACCOUNT_FLOW(0), ACCOUNT_BALANCE(0)
{
    fill_data();
}

Model_Checking::Full_Data::Full_Data(
    const Data& r,
    const std::map<int64 /* TRANSID */, Model_Splittransaction::Data_Set>& splits,
    const std::map<int64 /* TRANSID */, Model_Taglink::Data_Set>& tags
) :
    Data(r),
    ACCOUNTID_W(-1), ACCOUNTID_D(-1), TRANSAMOUNT_W(0), TRANSAMOUNT_D(0),
    SN(0), ACCOUNT_FLOW(0), ACCOUNT_BALANCE(0)
{
    const auto it = splits.find(this->id());
    if (it != splits.end()) m_splits = it->second;

    const auto tag_it = tags.find(this->id());
    if (tag_it != tags.end()) m_tags = tag_it->second;

    fill_data();
}

void Model_Checking::Full_Data::fill_data()
{
    displayID = wxString::Format("%lld", TRANSID);
    ACCOUNTNAME = Model_Account::get_account_name(ACCOUNTID);

    if (Model_Checking::type_id(TRANSCODE) == Model_Checking::TYPE_ID_TRANSFER) {
        TOACCOUNTNAME = Model_Account::get_account_name(TOACCOUNTID);
        PAYEENAME = TOACCOUNTNAME;
    }
    else {
        PAYEENAME = Model_Payee::get_payee_name(PAYEEID);
    }

    if (!m_splits.empty()) {
        for (const auto& entry : m_splits)
            CATEGNAME += (CATEGNAME.empty() ? " + " : ", ")
                + Model_Category::full_name(entry.CATEGID);
    }
    else {
        CATEGNAME = Model_Category::full_name(CATEGID);
    }

    if (!m_tags.empty()) {
        wxArrayString tagnames;
        for (const auto& entry : m_tags)
            tagnames.Add(Model_Tag::instance().get(entry.TAGID)->TAGNAME);
        // Sort TAGNAMES
        tagnames.Sort(CaseInsensitiveCmp);
        for (const auto& name : tagnames)
            TAGNAMES += (TAGNAMES.empty() ? "" : " ") + name;
    }

    if (type_id(TRANSCODE) == TYPE_ID_WITHDRAWAL) {
        ACCOUNTID_W = ACCOUNTID; TRANSAMOUNT_W = TRANSAMOUNT;
    }
    else if (type_id(TRANSCODE) == TYPE_ID_DEPOSIT) {
        ACCOUNTID_D = ACCOUNTID; TRANSAMOUNT_D = TRANSAMOUNT;
    }
    else if (type_id(TRANSCODE) == TYPE_ID_TRANSFER) {
        ACCOUNTID_W = ACCOUNTID; TRANSAMOUNT_W = TRANSAMOUNT;
        ACCOUNTID_D = TOACCOUNTID; TRANSAMOUNT_D = TOTRANSAMOUNT;
    }
}

Model_Checking::Full_Data::~Full_Data()
{
}

wxString Model_Checking::Full_Data::real_payee_name(int64 account_id) const
{
    if (TYPE_ID_TRANSFER == type_id(this->TRANSCODE))
    {
        if (this->ACCOUNTID == account_id || account_id < 0)
            return ("> " + this->TOACCOUNTNAME);
        else
            return ("< " + this->ACCOUNTNAME);
    }

    return this->PAYEENAME;
}

const wxString Model_Checking::Full_Data::get_currency_code(int64 account_id) const
{
    if (TYPE_ID_TRANSFER == type_id(this->TRANSCODE))
    {
        if (this->ACCOUNTID == account_id || account_id == -1)
            account_id = this->ACCOUNTID;
        else
            account_id = this->TOACCOUNTID;
    }
    Model_Account::Data* acc = Model_Account::instance().get(account_id);
    int64 currency_id = acc ? acc->CURRENCYID: -1;
    Model_Currency::Data* curr = Model_Currency::instance().get(currency_id);

    return curr ? curr->CURRENCY_SYMBOL : "";
}

const wxString Model_Checking::Full_Data::get_account_name(int64 account_id) const
{
    if (TYPE_ID_TRANSFER == type_id(this->TRANSCODE))
    {
        if (this->ACCOUNTID == account_id || account_id == -1) {
            return this->ACCOUNTNAME;
        }
        else {
            Model_Account::Data* acc = Model_Account::instance().get(TOACCOUNTID);
            return acc ? acc->ACCOUNTNAME : "";
        }
    }

    return this->ACCOUNTNAME;
}

bool Model_Checking::Full_Data::is_foreign() const
{
    return (this->TOACCOUNTID > 0) &&
        (type_id(this->TRANSCODE) == TYPE_ID_DEPOSIT || type_id(this->TRANSCODE) == TYPE_ID_WITHDRAWAL);
}

bool Model_Checking::Full_Data::is_foreign_transfer() const
{
    return is_foreign() && (this->TOACCOUNTID == Model_Translink::AS_TRANSFER);
}

wxString Model_Checking::Full_Data::info() const
{
    // TODO more info
    wxDate date = Model_Checking::TRANSDATE(this);
    wxString info = wxGetTranslation(wxDate::GetEnglishWeekDayName(date.GetWeekDay()));
    return info;
}

bool CompareUsedNotes(const std::tuple<int, wxString, wxString>& a, const std::tuple<int, wxString, wxString>& b)
{
    if (std::get<0>(a) < std::get<0>(b)) return true;
    if (std::get<0>(b) < std::get<0>(a)) return false;

    // a=b for primary condition, go to secondary (but reverse order)
    if (std::get<1>(a) > std::get<1>(b)) return true;
    if (std::get<1>(b) > std::get<1>(a)) return false;

    return false;
}

void Model_Checking::getFrequentUsedNotes(std::vector<wxString> &frequentNotes, int64 accountID)
{
    frequentNotes.clear();
    size_t max = 20;

    const auto notes = instance().find(NOTES("", NOT_EQUAL)
        , accountID > 0 ? ACCOUNTID(accountID) : ACCOUNTID(-1, NOT_EQUAL));

    // Count frequency
    std::map <wxString, std::pair<int, wxString> > counterMap;
    for (const auto& entry : notes)
    {
        auto& counter = counterMap[entry.NOTES];
        counter.first--;
        if (entry.TRANSDATE > counter.second)
            counter.second = entry.TRANSDATE;
    }

    // Convert to vector
    std::vector<std::tuple<int, wxString, wxString> > vec;
    for (const auto& [note, counter] : counterMap)
        vec.emplace_back(counter.first, counter.second, note);

    // Sort by frequency then date
    std::sort(vec.begin(), vec.end(), CompareUsedNotes);

    // Pull out top 20 (max)
    for (const auto& kv : vec)
    {
        if (0 == max--)
            break;
        frequentNotes.push_back(std::get<2>(kv));
    }
}

void Model_Checking::getEmptyData(Data &data, int64 accountID)
{
    data.TRANSID = -1;
    data.PAYEEID = -1;
    const wxString today_date = wxDate::Now().FormatISOCombined();
    wxString max_trx_date;
    if (Option::instance().getTransDateDefault() != Option::NONE)
    {
        auto trans = instance().find_or(ACCOUNTID(accountID), TOACCOUNTID(accountID));

        for (const auto& t: trans) {
            if (t.DELETEDTIME.IsNull() && max_trx_date < t.TRANSDATE && today_date >= t.TRANSDATE) {
                max_trx_date = t.TRANSDATE;
            }
        }
    }

    if (max_trx_date.empty()) {
        max_trx_date = today_date;
    }

    data.TRANSDATE = max_trx_date;
    data.ACCOUNTID = accountID;
    data.STATUS = status_key(Option::instance().getTransStatusReconciled());
    data.TRANSCODE = TYPE_NAME_WITHDRAWAL;
    data.CATEGID = -1;
    data.FOLLOWUPID = -1;
    data.TRANSAMOUNT = 0;
    data.TOTRANSAMOUNT = 0;
    data.TRANSACTIONNUMBER = "";
    data.COLOR = -1;
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
        data.TRANSAMOUNT = r->TRANSAMOUNT;
        data.TOTRANSAMOUNT = r->TOTRANSAMOUNT;
        data.FOLLOWUPID = r->FOLLOWUPID;
        data.NOTES = r->NOTES;
        data.TRANSACTIONNUMBER = r->TRANSACTIONNUMBER;
        data.PAYEEID = r->PAYEEID;
        data.TRANSID = r->TRANSID;
        data.LASTUPDATEDTIME = r->LASTUPDATEDTIME;
        data.DELETEDTIME = r->DELETEDTIME;
        data.COLOR = r->COLOR;
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
    r->TOACCOUNTID = data.TOACCOUNTID;
    r->TOTRANSAMOUNT = data.TOTRANSAMOUNT;
    r->NOTES = data.NOTES;
    r->TRANSACTIONNUMBER = data.TRANSACTIONNUMBER;
    r->FOLLOWUPID = data.FOLLOWUPID;
    r->COLOR = data.COLOR;
}

const wxString Model_Checking::Full_Data::to_json()
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    json_writer.StartObject();

    Model_Checking::Data::as_json(json_writer);

    json_writer.Key("ACCOUNTNAME");
    json_writer.String(this->ACCOUNTNAME.utf8_str());

    if (is_transfer(this))
    {
        json_writer.Key("TOACCOUNTNAME");
        json_writer.String(this->TOACCOUNTNAME.utf8_str());
    }
    else
    {
        json_writer.Key("PAYEENAME");
        json_writer.String(this->PAYEENAME.utf8_str());
    }
    if (this->has_tags())
    {
        json_writer.Key("TAGS");
        json_writer.StartArray();
        for (const auto& item : m_splits)
        {
            json_writer.StartObject();
            json_writer.Key(Model_Category::full_name(item.CATEGID).utf8_str());
            json_writer.Double(item.SPLITTRANSAMOUNT);
            json_writer.EndObject();
        }
        json_writer.EndArray();
    }
    if (this->has_split())
    {
        json_writer.Key("CATEGS");
        json_writer.StartArray();
        for (const auto & item : m_splits)
        {
            json_writer.StartObject();
            json_writer.Key(Model_Category::full_name(item.CATEGID).utf8_str());
            json_writer.Double(item.SPLITTRANSAMOUNT);
            json_writer.EndObject();
        }
        json_writer.EndArray();
    }
    else
    {
        json_writer.Key("CATEG");
        json_writer.String(Model_Category::full_name(this->CATEGID).utf8_str());
    }

    json_writer.EndObject();

    wxLogDebug("======= Model_Checking::FullData::to_json =======");
    wxLogDebug("FullData using rapidjson:\n%s", wxString::FromUTF8(json_buffer.GetString()));

    return wxString::FromUTF8(json_buffer.GetString());
}

bool Model_Checking::foreignTransaction(const Data& data)
{
    return (data.TOACCOUNTID > 0) && (data.TRANSCODE == TYPE_NAME_DEPOSIT || data.TRANSCODE == TYPE_NAME_WITHDRAWAL);
}

bool Model_Checking::foreignTransactionAsTransfer(const Data& data)
{
    return foreignTransaction(data) && (data.TOACCOUNTID == Model_Translink::AS_TRANSFER || data.TOACCOUNTID == data.ACCOUNTID);
}

void Model_Checking::updateTimestamp(int64 id)
{
    Data* r = instance().get(id);
    if (r && r->TRANSID == id) {
        r->LASTUPDATEDTIME = wxDateTime::Now().ToUTC().FormatISOCombined();
        this->save(r, db_);
    }
}
