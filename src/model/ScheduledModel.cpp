/*******************************************************
 Copyright (C) 2013,2014 Guan Lisheng (guanlisheng@gmail.com)
 Copyright (C) 2021,2022 Mark Whalley (mark@ipx.co.uk)

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

#include "AccountModel.h"
#include "AttachmentModel.h"
#include "CategoryModel.h"
#include "FieldValueModel.h"
#include "PayeeModel.h"
#include "PreferencesModel.h"
#include "ScheduledModel.h"
#include "TagModel.h"

 // TODO: Move attachment management outside of AttachmentDialog
#include "dialog/AttachmentDialog.h"

ScheduledModel::ScheduledModel()
    : Model<ScheduledTable>()
    , m_autoExecute (REPEAT_AUTO_NONE)
    , m_requireExecution (false)
    , m_allowExecution (false)
{
}

ScheduledModel::~ScheduledModel()
{
}

/** Return the static instance of ScheduledModel table */
ScheduledModel& ScheduledModel::instance()
{
    return Singleton<ScheduledModel>::instance();
}

/**
* Initialize the global ScheduledModel table.
* Reset the ScheduledModel table or create the table if it does not exist.
*/
ScheduledModel& ScheduledModel::instance(wxSQLite3Database* db)
{
    ScheduledModel& ins = Singleton<ScheduledModel>::instance();
    ins.m_db = db;
    ins.destroy_cache();
    ins.ensure_table();

    return ins;
}

wxDate ScheduledModel::getTransDateTime(const Data* r)
{
    return parseDateTime(r->TRANSDATE);
}
wxDate ScheduledModel::getTransDateTime(const Data& r)
{
    return parseDateTime(r.TRANSDATE);
}

wxDate ScheduledModel::NEXTOCCURRENCEDATE(const Data* r)
{
    return parseDateTime(r->NEXTOCCURRENCEDATE);
}
wxDate ScheduledModel::NEXTOCCURRENCEDATE(const Data& r)
{
    return parseDateTime(r.NEXTOCCURRENCEDATE);
}

TransactionModel::TYPE_ID ScheduledModel::type_id(const Data* r)
{
    return static_cast<TransactionModel::TYPE_ID>(TransactionModel::type_id(r->TRANSCODE));
}
TransactionModel::TYPE_ID ScheduledModel::type_id(const Data& r)
{
    return type_id(&r);
}

TransactionModel::STATUS_ID ScheduledModel::status_id(const Data* r)
{
    return static_cast<TransactionModel::STATUS_ID>(TransactionModel::status_id(r->STATUS));
}
TransactionModel::STATUS_ID ScheduledModel::status_id(const Data& r)
{
    return status_id(&r);
}

/**
* Remove the Data record instance from memory and the database
* including any splits associated with the Data Record.
*/
bool ScheduledModel::remove(const int64 id)
{
    for (auto &item : ScheduledModel::split(get_id(id)))
        ScheduledSplitModel::instance().remove(item.SPLITTRANSID);
    // Delete tags for the scheduled transaction
    TagLinkModel::instance().DeleteAllTags(this->refTypeName, id);
    return Model<ScheduledTable>::remove(id);
}

ScheduledTable::STATUS ScheduledModel::STATUS(OP op, TransactionModel::STATUS_ID status)
{
    return ScheduledTable::STATUS(op, TransactionModel::status_key(status));
}

ScheduledTable::TRANSCODE ScheduledModel::TRANSCODE(OP op, TransactionModel::TYPE_ID type)
{
    return ScheduledTable::TRANSCODE(op, TransactionModel::type_name(type));
}

const ScheduledSplitModel::Data_Set ScheduledModel::split(const Data* r)
{
    return ScheduledSplitModel::instance().find(
        ScheduledSplitModel::TRANSID(r->BDID));
}

const ScheduledSplitModel::Data_Set ScheduledModel::split(const Data& r)
{
    return split(&r);
}

const TagLinkModel::Data_Set ScheduledModel::taglink(const Data* r)
{
    return TagLinkModel::instance().find(
        TagLinkModel::REFTYPE(ScheduledModel::refTypeName),
        TagLinkModel::REFID(r->BDID));
}

const TagLinkModel::Data_Set ScheduledModel::taglink(const Data& r)
{
    return taglink(&r);
}

void ScheduledModel::decode_fields(const Data& q1)
{
    // DeMultiplex the Auto Executable fields from the db entry: REPEATS
    m_autoExecute = q1.REPEATS.GetValue() / BD_REPEATS_MULTIPLEX_BASE;
    m_allowExecution = true;

    int repeats = q1.REPEATS.GetValue() % BD_REPEATS_MULTIPLEX_BASE;
    int numRepeats = q1.NUMOCCURRENCES.GetValue();
    if (repeats >= ScheduledModel::REPEAT_IN_X_DAYS && repeats <= ScheduledModel::REPEAT_EVERY_X_MONTHS && numRepeats < 1)
    {
        // old inactive entry
        m_autoExecute = REPEAT_AUTO_NONE;
        m_allowExecution = false;
    }

    m_requireExecution = (ScheduledModel::NEXTOCCURRENCEDATE(&q1)
        .Subtract(wxDate::Today()).GetSeconds().GetValue() / 86400 < 1);
}

bool ScheduledModel::autoExecuteManual()
{
    return m_autoExecute == REPEAT_AUTO_MANUAL;
}

bool ScheduledModel::autoExecuteSilent()
{
    return m_autoExecute == REPEAT_AUTO_SILENT;
}

bool ScheduledModel::requireExecution()
{
    return m_requireExecution;
}

bool ScheduledModel::allowExecution()
{
    return m_allowExecution;
}

bool ScheduledModel::AllowTransaction(const Data& r)
{
    if (r.STATUS == TransactionModel::STATUS_KEY_VOID)
        return true;
    if (r.TRANSCODE != TransactionModel::TYPE_NAME_WITHDRAWAL && r.TRANSCODE != TransactionModel::TYPE_NAME_TRANSFER)
        return true;

    const int64 acct_id = r.ACCOUNTID;
    AccountModel::Data* account = AccountModel::instance().get_id(acct_id);

    if (account->MINIMUMBALANCE == 0 && account->CREDITLIMIT == 0)
        return true;

    double current_balance = AccountModel::balance(account);
    double new_balance = current_balance - r.TRANSAMOUNT;

    bool allow_transaction = true;
    wxString limitDescription;
    double limitAmount{ 0.0L };
    if (account->MINIMUMBALANCE != 0 && new_balance < account->MINIMUMBALANCE)
    {
        allow_transaction = false;
        limitDescription = _t("Minimum Balance");
        limitAmount = account->MINIMUMBALANCE;
    }
    else if (account->CREDITLIMIT != 0 && new_balance < -(account->CREDITLIMIT))
    {
        allow_transaction = false;
        limitDescription = _t("Credit Limit");
        limitAmount = account->CREDITLIMIT;
    }

    if (!allow_transaction)
    {
        wxString message = _t("A scheduled transaction will exceed the account limit.\n\n"
            "Account: %1$s\n"
            "Current Balance: %2$6.2f\n"
            "Transaction amount: %3$6.2f\n"
            "%4$s: %5$6.2f") + "\n\n" +
            _t("Do you want to continue?");
        message.Printf(message, account->ACCOUNTNAME, current_balance, r.TRANSAMOUNT, limitDescription, limitAmount);

        if (wxMessageBox(message, _t("MMEX Scheduled Transaction Check"), wxYES_NO | wxICON_WARNING) == wxYES)
            allow_transaction = true;
    }

    return allow_transaction;
}

void ScheduledModel::completeBDInSeries(int64 bdID)
{
    Data* bill = get_id(bdID);
    if (!bill) return;

    int repeats = bill->REPEATS.GetValue() % BD_REPEATS_MULTIPLEX_BASE; // DeMultiplex the Auto Executable fields.
    int numRepeats = bill->NUMOCCURRENCES.GetValue();

    if ((repeats == REPEAT_TYPE::REPEAT_ONCE) || ((repeats < REPEAT_TYPE::REPEAT_IN_X_DAYS || repeats > REPEAT_TYPE::REPEAT_EVERY_X_MONTHS) && numRepeats == 1))
    {
        mmAttachmentManage::DeleteAllAttachments(this->refTypeName, bdID);
        remove(bdID);
        return;
    }

    wxDateTime transdate;
    transdate.ParseDateTime(bill->TRANSDATE) || transdate.ParseDate(bill->TRANSDATE);
    const wxDateTime& payment_date_current = transdate;
    const wxDateTime& payment_date_update = nextOccurDate(repeats, numRepeats, payment_date_current);
    bill->TRANSDATE = payment_date_update.FormatISOCombined();

    const wxDateTime& due_date_current = NEXTOCCURRENCEDATE(bill);
    const wxDateTime& due_date_update = nextOccurDate(repeats, numRepeats, due_date_current);
    bill->NEXTOCCURRENCEDATE = due_date_update.FormatISODate();

    if ((repeats < REPEAT_TYPE::REPEAT_IN_X_DAYS || repeats > REPEAT_TYPE::REPEAT_EVERY_X_MONTHS) && numRepeats > 1)
    {
        bill->NUMOCCURRENCES = numRepeats - 1;
    }
    else if (repeats >= REPEAT_TYPE::REPEAT_IN_X_DAYS && repeats <= REPEAT_TYPE::REPEAT_IN_X_MONTHS)
    {
        // preserve the Auto Executable fields, change type to REPEAT_ONCE
        bill->REPEATS += REPEAT_TYPE::REPEAT_ONCE - repeats;
        bill->NUMOCCURRENCES = -1;
    }

    save(bill);
}

const wxDateTime ScheduledModel::nextOccurDate(int repeatsType, int numRepeats, wxDateTime nextOccurDate, bool reverse)
{
    int k = reverse ? -1 : 1;

    wxDateTime dt = nextOccurDate;
    if (repeatsType == REPEAT_WEEKLY)
        dt.Add(wxTimeSpan::Weeks(k));
    else if (repeatsType == REPEAT_BI_WEEKLY)
        dt.Add(wxTimeSpan::Weeks(2 * k));
    else if (repeatsType == REPEAT_MONTHLY)
        dt.Add(wxDateSpan::Months(k));
    else if (repeatsType == REPEAT_BI_MONTHLY)
        dt.Add(wxDateSpan::Months(2 * k));
    else if (repeatsType == REPEAT_FOUR_MONTHLY)
        dt.Add(wxDateSpan::Months(4 * k));
    else if (repeatsType == REPEAT_HALF_YEARLY)
        dt.Add(wxDateSpan::Months(6 * k));
    else if (repeatsType == REPEAT_YEARLY)
        dt.Add(wxDateSpan::Years(k));
    else if (repeatsType == REPEAT_QUARTERLY)
        dt.Add(wxDateSpan::Months(3 * k));
    else if (repeatsType == REPEAT_FOUR_WEEKLY)
        dt.Add(wxDateSpan::Weeks(4 * k));
    else if (repeatsType == REPEAT_DAILY)
        dt.Add(wxDateSpan::Days(k));
    else if (repeatsType == REPEAT_IN_X_DAYS) // repeat in numRepeats Days (Once only)
        dt.Add(wxDateSpan::Days(numRepeats * k));
    else if (repeatsType == REPEAT_IN_X_MONTHS) // repeat in numRepeats Months (Once only)
        dt.Add(wxDateSpan::Months(numRepeats * k));
    else if (repeatsType == REPEAT_EVERY_X_DAYS) // repeat every numRepeats Days
        dt.Add(wxDateSpan::Days(numRepeats * k));
    else if (repeatsType == REPEAT_EVERY_X_MONTHS) // repeat every numRepeats Months
        dt.Add(wxDateSpan::Months(numRepeats * k));
    else if (repeatsType == REPEAT_MONTHLY_LAST_DAY
        ||   repeatsType == REPEAT_MONTHLY_LAST_BUSINESS_DAY)
    {
        dt.Add(wxDateSpan::Months(k));

        dt.SetToLastMonthDay(dt.GetMonth(), dt.GetYear());
        if (repeatsType == REPEAT_MONTHLY_LAST_BUSINESS_DAY) // last weekday of month
        {
            if (dt.GetWeekDay() == wxDateTime::Sun || dt.GetWeekDay() == wxDateTime::Sat)
                dt.SetToPrevWeekDay(wxDateTime::Fri);
        }
    }
    wxLogDebug("init date: %s -> next date: %s", nextOccurDate.FormatISOCombined(), dt.FormatISOCombined());
    return dt;
}

wxArrayString ScheduledModel::unroll(const Data* r, const wxString end_date, int limit)
{
    wxArrayString dates;
    int repeats = r->REPEATS.GetValue() % BD_REPEATS_MULTIPLEX_BASE;
    int numRepeats = r->NUMOCCURRENCES.GetValue();

    // ignore old inactive entries
    if (repeats >= ScheduledModel::REPEAT_IN_X_DAYS && repeats <= ScheduledModel::REPEAT_EVERY_X_MONTHS && numRepeats == -1)
        return dates;

    // ignore invalid entries
    if (repeats != ScheduledModel::REPEAT_ONCE && (numRepeats == 0 || numRepeats < -1))
        return dates;

    wxString date = r->TRANSDATE;
    while (date <= end_date && limit != 0) {
        if (limit > 0) limit--;
        dates.push_back(date);

        if (repeats == ScheduledModel::REPEAT_ONCE)
            break;
        if ((repeats < ScheduledModel::REPEAT_IN_X_DAYS || repeats > ScheduledModel::REPEAT_EVERY_X_MONTHS) && numRepeats == 1)
            break;

        wxDateTime date_curr;
        date_curr.ParseDateTime(date) || date_curr.ParseDate(date);
        const wxDateTime& date_next = ScheduledModel::nextOccurDate(repeats, numRepeats, date_curr);
        date = date_next.FormatISOCombined();

        if ((repeats < ScheduledModel::REPEAT_IN_X_DAYS || repeats > ScheduledModel::REPEAT_EVERY_X_MONTHS) && numRepeats > 1)
            numRepeats--;
        else if (repeats >= ScheduledModel::REPEAT_IN_X_DAYS && repeats <= ScheduledModel::REPEAT_IN_X_MONTHS)
            repeats = ScheduledModel::REPEAT_ONCE;
    }

    return dates;
}

wxArrayString ScheduledModel::unroll(const Data& r, const wxString end_date, int limit)
{
    return unroll(&r, end_date, limit);
}

ScheduledModel::Full_Data::Full_Data()
{}

ScheduledModel::Full_Data::Full_Data(const Data& r) :
    Data(r),
    m_bill_splits(split(r)),
    m_tags(TagLinkModel::instance().find(
        TagLinkModel::REFTYPE(ScheduledModel::refTypeName),
        TagLinkModel::REFID(r.BDID)))
{
    if (!m_tags.empty()) {
        wxArrayString tagnames;
        for (const auto& entry : m_tags)
            tagnames.Add(TagModel::instance().get_id(entry.TAGID)->TAGNAME);
        // Sort TAGNAMES
        tagnames.Sort();
        for (const auto& name : tagnames)
            this->TAGNAMES += (this->TAGNAMES.empty() ? "" : " ") + name;
    }

    if (!m_bill_splits.empty())
    {
        for (const auto& entry : m_bill_splits)
        {
            CATEGNAME += (CATEGNAME.empty() ? " + " : ", ")
                + CategoryModel::full_name(entry.CATEGID);

            wxString splitTags;
            for (const auto& tag : TagLinkModel::instance().cache_ref(ScheduledSplitModel::refTypeName, entry.SPLITTRANSID))
                splitTags.Append(tag.first + " ");
            if (!splitTags.IsEmpty())
                TAGNAMES.Append((TAGNAMES.IsEmpty() ? "" : ", ") + splitTags.Trim());
        }
    }
    else
        CATEGNAME = CategoryModel::full_name(r.CATEGID);

    ACCOUNTNAME = AccountModel::cache_id_name(r.ACCOUNTID);

    PAYEENAME = PayeeModel::get_payee_name(r.PAYEEID);
    if (ScheduledModel::type_id(r) == TransactionModel::TYPE_ID_TRANSFER)
    {
        PAYEENAME = AccountModel::cache_id_name(r.TOACCOUNTID);
    }

}

wxString ScheduledModel::Full_Data::real_payee_name() const
{
    if (TransactionModel::TYPE_ID_TRANSFER == TransactionModel::type_id(this->TRANSCODE))
    {
        return ("> " + this->PAYEENAME);
    }
    return this->PAYEENAME;
}
