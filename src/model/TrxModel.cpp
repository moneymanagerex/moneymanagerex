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

#include <queue>

#include "util/_util.h"

#include "AccountModel.h"
#include "CategoryModel.h"
#include "FieldValueModel.h"
#include "PayeeModel.h"
#include "PrefModel.h"
#include "TagModel.h"
#include "TrxLinkModel.h"
#include "TrxShareModel.h"
#include "TrxModel.h"

#include "dialog/AttachmentDialog.h"

const RefTypeN TrxModel::s_ref_type = RefTypeN(RefTypeN::e_trx);

TrxModel::TrxModel() :
    TableFactory<TrxTable, TrxData>()
{
}

TrxModel::~TrxModel()
{
}

// Initialize the global TrxModel table.
// Reset the TrxModel table or create the table if it does not exist.
TrxModel& TrxModel::instance(wxSQLite3Database* db)
{
    TrxModel& ins = Singleton<TrxModel>::instance();
    ins.reset_cache();
    ins.m_db = db;
    ins.ensure_table();

    return ins;
}

// Return the static instance of TrxModel table
TrxModel& TrxModel::instance()
{
    return Singleton<TrxModel>::instance();
}

void TrxModel::copy_from_trx(Data *this_n, const Data& other_d)
{
    this_n->TRANSDATE         = other_d.TRANSDATE;
    this_n->m_type            = other_d.m_type;
    this_n->m_status          = other_d.m_status;
    this_n->m_account_id      = other_d.m_account_id;
    this_n->m_to_account_id_n = other_d.m_to_account_id_n;
    this_n->m_payee_id_n      = other_d.m_payee_id_n;
    this_n->m_category_id_n   = other_d.m_category_id_n;
    this_n->m_amount          = other_d.m_amount;
    this_n->m_to_amount       = other_d.m_to_amount;
    this_n->m_notes           = other_d.m_notes;
    this_n->m_number          = other_d.m_number;
    this_n->m_followup_id     = other_d.m_followup_id;
    this_n->m_color           = other_d.m_color;
}

wxDateTime TrxModel::getTransDateTime(const Data& this_d)
{
    return parseDateTime(this_d.TRANSDATE);
}

double TrxModel::account_flow(const Data& this_d, int64 account_id)
{
    // Self Transfer as Revaluation
    if (this_d.m_account_id == this_d.m_to_account_id_n && this_d.is_transfer())
        return 0.0;

    if (this_d.is_void() || !this_d.DELETEDTIME.IsEmpty())
        return 0.0;

    if (account_id == this_d.m_account_id && this_d.is_withdrawal())
        return -(this_d.m_amount);
    if (account_id == this_d.m_account_id && this_d.is_deposit())
        return this_d.m_amount;
    if (account_id == this_d.m_account_id && this_d.is_transfer())
        return -(this_d.m_amount);
    if (account_id == this_d.m_to_account_id_n && this_d.is_transfer())
        return this_d.m_to_amount;
    return 0.0;
}

double TrxModel::account_outflow(const Data& this_d, int64 account_id)
{
    double bal = account_flow(this_d, account_id);
    return bal <= 0 ? -bal : 0;
}

double TrxModel::account_inflow(const Data& this_d, int64 account_id)
{
    double bal = account_flow(this_d, account_id);
    return bal >= 0 ? bal : 0;
}

double TrxModel::account_recflow(const Data& this_d, int64 account_id)
{
    return (this_d.is_reconciled())
        ? account_flow(this_d, account_id)
        : 0;
}

// same as TrxModel::Full_Data::is_foreign()
bool TrxModel::is_foreign(const Data& this_d)
{
    return (!this_d.is_transfer() && this_d.m_to_account_id_n > 0);
}

// see also TrxModel::Full_Data::is_foreign_transfer()
bool TrxModel::is_foreignAsTransfer(const Data& this_d)
{
    return is_foreign(this_d) && (
        this_d.m_to_account_id_n == TrxLinkModel::AS_TRANSFER ||
        this_d.m_to_account_id_n == this_d.m_account_id
    );
}

TrxCol::TRANSDATE TrxModel::TRANSDATE(OP op, const wxString& date_iso_str)
{
    return TrxCol::TRANSDATE(op, date_iso_str);
}

TrxCol::TRANSDATE TrxModel::TRANSDATE(OP op, const mmDate& date)
{
    // OP_EQ and OP_NE should not be used for date comparisons.
    // if needed, create an equivalent AND/OR combination of two other operators.
    wxString bound =
        (op == OP_GE || op == OP_LT) ? date.isoStart()
        : (op == OP_LE || op == OP_GT) ? date.isoEnd()
        : date.isoDate();
    return TrxCol::TRANSDATE(op, bound);
}

TrxCol::TRANSDATE TrxModel::TRANSDATE(OP op, const wxDateTime& date)
{
    // the boundary has granularity of a day
    return TrxModel::TRANSDATE(op, mmDate(date));
}

TrxCol::DELETEDTIME TrxModel::DELETEDTIME(OP op, const wxString& date)
{
    return TrxCol::DELETEDTIME(op, date);
}

TrxCol::STATUS TrxModel::STATUS(OP op, TrxStatus trx_status)
{
    return TrxCol::STATUS(op, trx_status.key());
}

TrxCol::TRANSCODE TrxModel::TRANSCODE(OP op, TrxType trx_type)
{
    return TrxCol::TRANSCODE(op, trx_type.name());
}

TrxCol::TRANSACTIONNUMBER TrxModel::TRANSACTIONNUMBER(OP op, const wxString& num)
{
    return TrxCol::TRANSACTIONNUMBER(op, num);
}

const TrxModel::DataA TrxModel::find_allByDateTimeId()
{
    auto trx_a = TrxModel::instance().find_all();
    // first sort by id, then stable sort by datetime or date only
    std::sort(trx_a.begin(), trx_a.end());
    if (PrefModel::instance().UseTransDateTime())
        std::stable_sort(trx_a.begin(), trx_a.end(), TrxData::SorterByTRANSDATE());
    else
        std::stable_sort(trx_a.begin(), trx_a.end(), TrxModel::SorterByTRANSDATE_DATE());
    return trx_a;
}

const TrxSplitModel::DataA TrxModel::find_split(const Data& trx_d)
{
    return TrxSplitModel::instance().find(
        TrxSplitCol::TRANSID(trx_d.m_id)
    );
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

void TrxModel::getFrequentUsedNotes(std::vector<wxString> &frequentNotes, int64 accountID)
{
    frequentNotes.clear();
    size_t max = 20;

    const auto trx_a = instance().find(
        TrxCol::NOTES(OP_NE, ""),
        accountID > 0 ? TrxCol::ACCOUNTID(accountID) : TrxCol::ACCOUNTID(OP_NE, -1)
    );

    // Count frequency
    std::map <wxString, std::pair<int, wxString> > counterMap;
    for (const auto& trx_d : trx_a) {
        auto& counter = counterMap[trx_d.m_notes];
        counter.first--;
        if (trx_d.TRANSDATE > counter.second)
            counter.second = trx_d.TRANSDATE;
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

void TrxModel::setEmptyData(Data &trx_d, int64 accountID)
{
    trx_d.m_id = -1;
    trx_d.m_payee_id_n = -1;
    const wxString today_date = wxDate::Now().FormatISOCombined();
    wxString max_trx_date;
    if (PrefModel::instance().getTransDateDefault() != PrefModel::NONE) {
        auto trx_a = instance().find_or(
            TrxCol::ACCOUNTID(accountID),
            TrxCol::TOACCOUNTID(accountID)
        );

        for (const auto& t_d: trx_a) {
            if (t_d.DELETEDTIME.IsNull() && max_trx_date < t_d.TRANSDATE && today_date >= t_d.TRANSDATE) {
                max_trx_date = t_d.TRANSDATE;
            }
        }
    }

    if (max_trx_date.empty()) {
        max_trx_date = today_date;
    }

    trx_d.TRANSDATE       = max_trx_date;
    trx_d.m_type          = TrxType(TrxType::e_withdrawal);
    trx_d.m_status        = TrxStatus(PrefModel::instance().getTransStatusReconciled());
    trx_d.m_account_id    = accountID;
    trx_d.m_category_id_n = -1;
    trx_d.m_amount        = 0;
    trx_d.m_to_amount     = 0;
    trx_d.m_number        = "";
    trx_d.m_followup_id   = -1;
    trx_d.m_color         = -1;
}

bool TrxModel::is_locked(const Data& trx_d)
{
    // FIXME: check if m_to_account_id_n is locked
    const AccountData* account_n = AccountModel::instance().get_id_data_n(trx_d.m_account_id);
    mmDateN trx_date_n = mmDateN(trx_d.TRANSDATE);
    return trx_date_n.has_value() && account_n->is_locked_for(trx_date_n.value());
}

bool TrxModel::purge_id(int64 trx_id)
{
    // TODO: remove all split at once
    // TrxSplitModel::instance().purge_id(TrxSplitModel::instance().find(TrxSplitCol::TRANSID(trx_id)));
    for (const auto& tp_d : TrxSplitModel::instance().find(
        TrxSplitCol::TRANSID(trx_id)
    )) {
        TrxSplitModel::instance().purge_id(tp_d.m_id);
    }

    if (is_foreign(*instance().get_id_data_n(trx_id))) {
        const TrxLinkData* tl_n = TrxLinkModel::instance().get_trx_data_n(trx_id);
        if (tl_n) {
            TrxShareModel::instance().purge_trxId(tl_n->m_trx_id);
            TrxLinkModel::instance().purge_id(tl_n->m_id);
            if (tl_n->m_ref_type == AssetModel::s_ref_type) {
                AssetData* asset_n = AssetModel::instance().unsafe_get_id_data_n(tl_n->m_ref_id);
                TrxLinkModel::UpdateAssetValue(asset_n);
            }
            else if (tl_n->m_ref_type == StockModel::s_ref_type) {
                StockData* stock_n = StockModel::instance().unsafe_get_id_data_n(tl_n->m_ref_id);
                StockModel::instance().update_data_position(stock_n);
            }
        }
    }

    // remove all attachments
    mmAttachmentManage::DeleteAllAttachments(TrxModel::s_ref_type, trx_id);
    // remove all custom fields for the transaction
    FieldValueModel::instance().purge_ref(s_ref_type, trx_id);
    TagLinkModel::instance().purge_ref(s_ref_type, trx_id);
    return unsafe_remove_id(trx_id);
}

void TrxModel::save_timestamp(int64 trx_id)
{
    Data* trx_n = instance().unsafe_get_id_data_n(trx_id);
    if (trx_n && trx_n->m_id == trx_id) {
        trx_n->LASTUPDATEDTIME = wxDateTime::Now().ToUTC().FormatISOCombined();
        unsafe_update_data_n(trx_n);
    }
}

void TrxModel::update_timestamp(Data& trx_d)
{
    // get record from database bypassing the cache
    TrxModel::DataA trx_a = TrxModel::instance().find(
        TrxCol::TRANSID(trx_d.m_id)
    );
    if (trx_a.size() == 0 || (!trx_a[0].equals(&trx_d)
        && trx_a[0].DELETEDTIME.IsEmpty() && trx_d.DELETEDTIME.IsEmpty()
    )) {
        trx_d.LASTUPDATEDTIME = wxDateTime::Now().ToUTC().FormatISOCombined();
    }
}

const TrxData* TrxModel::unsafe_save_trx_n(Data* trx_n)
{
    update_timestamp(*trx_n);
    return unsafe_save_data_n(trx_n);
}

const TrxData* TrxModel::save_trx_n(Data& trx_d)
{
    update_timestamp(trx_d);
    return save_data_n(trx_d);
}

bool TrxModel::save_trx_a(DataA& trx_a)
{
    bool ok = true;

    db_savepoint();
    for (auto& trx_d : trx_a) {
        if (trx_d.id() < 0)
            wxLogDebug("Incorrect function call to save %s", trx_d.to_json().utf8_str());
        if (!save_trx_n(trx_d)) {
            ok = false;
            break;
        }
    }
    db_release_savepoint();

    return ok;
}

TrxModel::Full_Data::Full_Data() :
    Data(), TAGNAMES(""),
    ACCOUNTID_W(-1), ACCOUNTID_D(-1), TRANSAMOUNT_W(0), TRANSAMOUNT_D(0),
    SN(0), ACCOUNT_FLOW(0), ACCOUNT_BALANCE(0)
{
}

TrxModel::Full_Data::Full_Data(const Data& r) :
    Data(r),
    m_splits(TrxSplitModel::instance().find(
        TrxSplitCol::TRANSID(r.m_id))),
    m_tags(TagLinkModel::instance().find(
        TagLinkCol::REFTYPE(TrxModel::s_ref_type.name_n()),
        TagLinkCol::REFID(r.m_id))),
    ACCOUNTID_W(-1), ACCOUNTID_D(-1), TRANSAMOUNT_W(0), TRANSAMOUNT_D(0),
    SN(0), ACCOUNT_FLOW(0), ACCOUNT_BALANCE(0)
{
    fill_data();
}

TrxModel::Full_Data::Full_Data(
    const Data& r,
    const std::map<int64 /* m_id */, TrxSplitModel::DataA>& splits,
    const std::map<int64 /* m_id */, TagLinkModel::DataA>& tags
) :
    Data(r),
    ACCOUNTID_W(-1), ACCOUNTID_D(-1), TRANSAMOUNT_W(0), TRANSAMOUNT_D(0),
    SN(0), ACCOUNT_FLOW(0), ACCOUNT_BALANCE(0)
{
    if (const auto it = splits.find(this->id()); it != splits.end()) m_splits = it->second;

    if (const auto tag_it = tags.find(this->id()); tag_it != tags.end()) m_tags = tag_it->second;

    fill_data();
}

void TrxModel::Full_Data::fill_data()
{
    displayID = wxString::Format("%lld", m_id);
    ACCOUNTNAME = AccountModel::instance().get_id_name(m_account_id);

    if (is_transfer()) {
        TOACCOUNTNAME = AccountModel::instance().get_id_name(m_to_account_id_n);
        PAYEENAME = TOACCOUNTNAME;
    }
    else {
        PAYEENAME = PayeeModel::instance().get_id_name(m_payee_id_n);
    }

    if (!m_splits.empty()) {
        for (const auto& tp_d : m_splits)
            CATEGNAME += (CATEGNAME.empty() ? " + " : ", ")
                + CategoryModel::instance().full_name(tp_d.m_category_id);
    }
    else {
        CATEGNAME = CategoryModel::instance().full_name(m_category_id_n);
    }

    if (!m_tags.empty()) {
        wxArrayString tag_name_a;
        for (const auto& gl_d : m_tags)
            tag_name_a.Add(TagModel::instance().get_id_data_n(gl_d.m_tag_id)->m_name);
        // Sort TAGNAMES
        tag_name_a.Sort(CaseInsensitiveCmp);
        for (const auto& tag_name : tag_name_a)
            TAGNAMES += (TAGNAMES.empty() ? "" : " ") + tag_name;
    }

    if (is_withdrawal()) {
        ACCOUNTID_W = m_account_id; TRANSAMOUNT_W = m_amount;
    }
    else if (is_deposit()) {
        ACCOUNTID_D = m_account_id; TRANSAMOUNT_D = m_amount;
    }
    else if (is_transfer()) {
        ACCOUNTID_W = m_account_id; TRANSAMOUNT_W = m_amount;
        ACCOUNTID_D = m_to_account_id_n; TRANSAMOUNT_D = m_to_amount;
    }
}

TrxModel::Full_Data::~Full_Data()
{
}

wxString TrxModel::Full_Data::real_payee_name(int64 account_id) const
{
    if (is_transfer()) {
        if (this->m_account_id == account_id || account_id < 0)
            return ("> " + this->TOACCOUNTNAME);
        else
            return ("< " + this->ACCOUNTNAME);
    }

    return this->PAYEENAME;
}

const wxString TrxModel::Full_Data::get_currency_code(int64 account_id) const
{
    if (is_transfer()) {
        if (this->m_account_id == account_id || account_id == -1)
            account_id = this->m_account_id;
        else
            account_id = this->m_to_account_id_n;
    }
    const AccountData* account_n = AccountModel::instance().get_id_data_n(account_id);
    int64 currency_id = account_n ? account_n->m_currency_id: -1;
    const CurrencyData* curr = CurrencyModel::instance().get_id_data_n(currency_id);

    return curr ? curr->m_symbol : "";
}

const wxString TrxModel::Full_Data::get_account_name(int64 account_id) const
{
    if (!is_transfer())
        return ACCOUNTNAME;
    else if (m_account_id == account_id || account_id == -1) {
        return ACCOUNTNAME;
    }
    else {
        const AccountData* account_n = AccountModel::instance().get_id_data_n(
            m_to_account_id_n
        );
        return account_n ? account_n->m_name : "";
    }
}

// same as TrxModel::is_foreign()
bool TrxModel::Full_Data::is_foreign() const
{
    return (!is_transfer() && m_to_account_id_n > 0);
}

// see also TrxModel::is_foreignAsTransfer()
bool TrxModel::Full_Data::is_foreign_transfer() const
{
    return is_foreign() && (this->m_to_account_id_n == TrxLinkModel::AS_TRANSFER);
}

wxString TrxModel::Full_Data::info() const
{
    // TODO more info
    wxDate date = TrxModel::getTransDateTime(*this);
    wxString info = wxGetTranslation(wxDate::GetEnglishWeekDayName(date.GetWeekDay()));
    return info;
}

const wxString TrxModel::Full_Data::to_json()
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    json_writer.StartObject();

    TrxData::as_json(json_writer);

    json_writer.Key("ACCOUNTNAME");
    json_writer.String(this->ACCOUNTNAME.utf8_str());

    if (is_transfer()) {
        json_writer.Key("TOACCOUNTNAME");
        json_writer.String(this->TOACCOUNTNAME.utf8_str());
    }
    else {
        json_writer.Key("PAYEENAME");
        json_writer.String(this->PAYEENAME.utf8_str());
    }
    if (this->has_tags()) {
        json_writer.Key("TAGS");
        json_writer.StartArray();
        for (const auto& tp_d : m_splits) {
            json_writer.StartObject();
            json_writer.Key(CategoryModel::instance().full_name(tp_d.m_category_id).utf8_str());
            json_writer.Double(tp_d.m_amount);
            json_writer.EndObject();
        }
        json_writer.EndArray();
    }
    if (this->has_split()) {
        json_writer.Key("CATEGS");
        json_writer.StartArray();
        for (const auto & tp_d : m_splits) {
            json_writer.StartObject();
            json_writer.Key(CategoryModel::instance().full_name(tp_d.m_category_id).utf8_str());
            json_writer.Double(tp_d.m_amount);
            json_writer.EndObject();
        }
        json_writer.EndArray();
    }
    else {
        json_writer.Key("CATEG");
        json_writer.String(CategoryModel::instance().full_name(this->m_category_id_n).utf8_str());
    }

    json_writer.EndObject();

    wxLogDebug("======= TrxModel::FullData::to_json =======");
    wxLogDebug("FullData using rapidjson:\n%s", wxString::FromUTF8(json_buffer.GetString()));

    return wxString::FromUTF8(json_buffer.GetString());
}

