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

#include "Model_Billsdeposits.h"
#include "option.h"
#include "Model_Account.h"
#include "Model_Attachment.h"
#include "Model_Category.h"
#include "Model_Payee.h"
#include "Model_Tag.h"
#include "Model_CustomFieldData.h"

 /* TODO: Move attachment management outside of attachmentdialog */
#include "attachmentdialog.h"

Model_Billsdeposits::Model_Billsdeposits()
    : Model<DB_Table_BILLSDEPOSITS_V1>()
    , m_autoExecute (REPEAT_AUTO_NONE)
    , m_requireExecution (false)
    , m_allowExecution (false)
{
}

Model_Billsdeposits::~Model_Billsdeposits()
{
}

/** Return the static instance of Model_Billsdeposits table */
Model_Billsdeposits& Model_Billsdeposits::instance()
{
    return Singleton<Model_Billsdeposits>::instance();
}

/**
* Initialize the global Model_Billsdeposits table.
* Reset the Model_Billsdeposits table or create the table if it does not exist.
*/
Model_Billsdeposits& Model_Billsdeposits::instance(wxSQLite3Database* db)
{
    Model_Billsdeposits& ins = Singleton<Model_Billsdeposits>::instance();
    ins.db_ = db;
    ins.destroy_cache();
    ins.ensure(db);

    return ins;
}

wxDate Model_Billsdeposits::TRANSDATE(const Data* r)
{
    return Model::to_date(r->TRANSDATE);
}

wxDate Model_Billsdeposits::TRANSDATE(const Data& r)
{
    return Model::to_date(r.TRANSDATE);
}

wxDate Model_Billsdeposits::NEXTOCCURRENCEDATE(const Data* r)
{
    return Model::to_date(r->NEXTOCCURRENCEDATE);
}

wxDate Model_Billsdeposits::NEXTOCCURRENCEDATE(const Data& r)
{
    return Model::to_date(r.NEXTOCCURRENCEDATE);
}

Model_Checking::TYPE_ID Model_Billsdeposits::type_id(const Data& r)
{
    return Model_Checking::type_id(r.TRANSCODE);
}
Model_Checking::TYPE_ID Model_Billsdeposits::type_id(const Data* r)
{
    return Model_Checking::type_id(r->TRANSCODE);
}

Model_Checking::STATUS_ID Model_Billsdeposits::status_id(const Data& r)
{
    return Model_Checking::status_id(r.STATUS);
}
Model_Checking::STATUS_ID Model_Billsdeposits::status_id(const Data* r)
{
    return Model_Checking::status_id(r->STATUS);
}

/**
* Remove the Data record instance from memory and the database
* including any splits associated with the Data Record.
*/
bool Model_Billsdeposits::remove(int64 id)
{
    for (auto &item : Model_Billsdeposits::split(get(id)))
        Model_Budgetsplittransaction::instance().remove(item.SPLITTRANSID);
    // Delete tags for the scheduled transaction
    Model_Taglink::instance().DeleteAllTags(Model_Attachment::REFTYPE_STR_BILLSDEPOSIT, id);
    return this->remove(id, db_);
}

DB_Table_BILLSDEPOSITS_V1::STATUS Model_Billsdeposits::STATUS(Model_Checking::STATUS_ID status, OP op)
{
    return DB_Table_BILLSDEPOSITS_V1::STATUS(Model_Checking::STATUS_KEY[status], op);
}

DB_Table_BILLSDEPOSITS_V1::TRANSCODE Model_Billsdeposits::TRANSCODE(Model_Checking::TYPE_ID type, OP op)
{
    return DB_Table_BILLSDEPOSITS_V1::TRANSCODE(Model_Checking::TYPE_STR[type], op);
}

const Model_Budgetsplittransaction::Data_Set Model_Billsdeposits::split(const Data* r)
{
    return Model_Budgetsplittransaction::instance().find(
        Model_Budgetsplittransaction::TRANSID(r->BDID));
}

const Model_Budgetsplittransaction::Data_Set Model_Billsdeposits::split(const Data& r)
{
    return split(&r);
}

const Model_Taglink::Data_Set Model_Billsdeposits::taglink(const Data* r)
{
    return Model_Taglink::instance().find(
        Model_Taglink::REFTYPE(Model_Attachment::REFTYPE_STR_BILLSDEPOSIT),
        Model_Taglink::REFID(r->BDID));
}

const Model_Taglink::Data_Set Model_Billsdeposits::taglink(const Data& r)
{
    return taglink(&r);
}

void Model_Billsdeposits::decode_fields(const Data& q1)
{
    // DeMultiplex the Auto Executable fields from the db entry: REPEATS
    m_autoExecute = q1.REPEATS.GetValue() / BD_REPEATS_MULTIPLEX_BASE;
    m_allowExecution = true;

    int repeats = q1.REPEATS.GetValue() % BD_REPEATS_MULTIPLEX_BASE;
    int numRepeats = q1.NUMOCCURRENCES.GetValue();
    if (repeats >= Model_Billsdeposits::REPEAT_IN_X_DAYS && repeats <= Model_Billsdeposits::REPEAT_EVERY_X_MONTHS && numRepeats < 1)
    {
        // old inactive entry
        m_autoExecute = REPEAT_AUTO_NONE;
        m_allowExecution = false;
    }

    m_requireExecution = (Model_Billsdeposits::NEXTOCCURRENCEDATE(&q1)
        .Subtract(wxDate::Today()).GetSeconds().GetValue() / 86400 < 1);
}

bool Model_Billsdeposits::autoExecuteManual()
{
    return m_autoExecute == REPEAT_AUTO_MANUAL;
}

bool Model_Billsdeposits::autoExecuteSilent()
{
    return m_autoExecute == REPEAT_AUTO_SILENT;
}

bool Model_Billsdeposits::requireExecution()
{
    return m_requireExecution;
}

bool Model_Billsdeposits::allowExecution()
{
    return m_allowExecution;
}

bool Model_Billsdeposits::AllowTransaction(const Data& r)
{
    if (r.STATUS == Model_Checking::STATUS_KEY_VOID)
        return true;
    if (r.TRANSCODE != Model_Checking::TYPE_STR_WITHDRAWAL && r.TRANSCODE != Model_Checking::TYPE_STR_TRANSFER)
        return true;

    const int64 acct_id = r.ACCOUNTID;
    Model_Account::Data* account = Model_Account::instance().get(acct_id);

    if (account->MINIMUMBALANCE == 0 && account->CREDITLIMIT == 0)
        return true;

    double current_balance = Model_Account::balance(account);
    double new_balance = current_balance - r.TRANSAMOUNT;

    bool allow_transaction = true;
    wxString limitDescription;
    double limitAmount{ 0.0L };
    if (account->MINIMUMBALANCE != 0 && new_balance < account->MINIMUMBALANCE)
    {
        allow_transaction = false;
        limitDescription = _("Minimum Balance");
        limitAmount = account->MINIMUMBALANCE;
    }
    else if (account->CREDITLIMIT != 0 && new_balance < -(account->CREDITLIMIT))
    {
        allow_transaction = false;
        limitDescription = _("Credit Limit");
        limitAmount = account->CREDITLIMIT;
    }

    if (!allow_transaction)
    {
        wxString message = _("A scheduled transaction will exceed your account limit.\n\n"
            "Account: %1$s\n"
            "Current Balance: %2$6.2f\n"
            "Transaction amount: %3$6.2f\n"
            "%4$s: %5$6.2f") + "\n\n" +
            _("Do you want to continue?")
        );
        message.Printf(message, account->ACCOUNTNAME, current_balance, r.TRANSAMOUNT, limitDescription, limitAmount);

        if (wxMessageBox(message, _("MMEX Scheduled Transaction Check"), wxYES_NO | wxICON_WARNING) == wxYES)
            allow_transaction = true;
    }

    return allow_transaction;
}

void Model_Billsdeposits::completeBDInSeries(int64 bdID)
{
    Data* bill = get(bdID);
    if (!bill) return;

    int repeats = bill->REPEATS.GetValue() % BD_REPEATS_MULTIPLEX_BASE; // DeMultiplex the Auto Executable fields.
    int numRepeats = bill->NUMOCCURRENCES.GetValue();

    if ((repeats == REPEAT_TYPE::REPEAT_ONCE) || ((repeats < REPEAT_TYPE::REPEAT_IN_X_DAYS || repeats > REPEAT_TYPE::REPEAT_EVERY_X_MONTHS) && numRepeats == 1))
    {
        mmAttachmentManage::DeleteAllAttachments(Model_Attachment::REFTYPE_STR_BILLSDEPOSIT, bdID);
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

const wxDateTime Model_Billsdeposits::nextOccurDate(int repeatsType, int numRepeats, wxDateTime nextOccurDate, bool reverse)
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

wxArrayString Model_Billsdeposits::unroll(const Data* r, const wxString end_date, int limit)
{
    wxArrayString dates;
    int repeats = r->REPEATS.GetValue() % BD_REPEATS_MULTIPLEX_BASE;
    int numRepeats = r->NUMOCCURRENCES.GetValue();

    // ignore old inactive entries
    if (repeats >= Model_Billsdeposits::REPEAT_IN_X_DAYS && repeats <= Model_Billsdeposits::REPEAT_EVERY_X_MONTHS && numRepeats == -1)
        return dates;

    // ignore invalid entries
    if (repeats != Model_Billsdeposits::REPEAT_ONCE && (numRepeats == 0 || numRepeats < -1))
        return dates;

    wxString date = r->TRANSDATE;
    while (date <= end_date && limit != 0) {
        if (limit > 0) limit--;
        dates.push_back(date);

        if (repeats == Model_Billsdeposits::REPEAT_ONCE)
            break;
        if ((repeats < Model_Billsdeposits::REPEAT_IN_X_DAYS || repeats > Model_Billsdeposits::REPEAT_EVERY_X_MONTHS) && numRepeats == 1)
            break;

        wxDateTime date_curr;
        date_curr.ParseDateTime(date) || date_curr.ParseDate(date);
        const wxDateTime& date_next = Model_Billsdeposits::nextOccurDate(repeats, numRepeats, date_curr);
        date = date_next.FormatISOCombined();

        if ((repeats < Model_Billsdeposits::REPEAT_IN_X_DAYS || repeats > Model_Billsdeposits::REPEAT_EVERY_X_MONTHS) && numRepeats > 1)
            numRepeats--;
        else if (repeats >= Model_Billsdeposits::REPEAT_IN_X_DAYS && repeats <= Model_Billsdeposits::REPEAT_IN_X_MONTHS)
            repeats = Model_Billsdeposits::REPEAT_ONCE;
    }

    return dates;
}

wxArrayString Model_Billsdeposits::unroll(const Data& r, const wxString end_date, int limit)
{
    return unroll(&r, end_date, limit);
}

Model_Billsdeposits::Full_Data::Full_Data()
{}

Model_Billsdeposits::Full_Data::Full_Data(const Data& r) :
    Data(r),
    m_bill_splits(split(r)),
    m_tags(Model_Taglink::instance().find(
        Model_Taglink::REFTYPE(Model_Attachment::REFTYPE_STR_BILLSDEPOSIT),
        Model_Taglink::REFID(r.BDID)))
{
    if (!m_tags.empty()) {
        wxArrayString tagnames;
        for (const auto& entry : m_tags)
            tagnames.Add(Model_Tag::instance().get(entry.TAGID)->TAGNAME);
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
                + Model_Category::full_name(entry.CATEGID);

            wxString splitTags;
            for (const auto& tag : Model_Taglink::instance().get(Model_Attachment::REFTYPE_STR_BILLSDEPOSITSPLIT, entry.SPLITTRANSID))
                splitTags.Append(tag.first + " ");
            if (!splitTags.IsEmpty())
                TAGNAMES.Append((TAGNAMES.IsEmpty() ? "" : ", ") + splitTags.Trim());
        }
    }
    else
        CATEGNAME = Model_Category::full_name(r.CATEGID);

    ACCOUNTNAME = Model_Account::get_account_name(r.ACCOUNTID);

    PAYEENAME = Model_Payee::get_payee_name(r.PAYEEID);
    if (Model_Billsdeposits::type_id(r) == Model_Checking::TYPE_ID_TRANSFER)
    {
        PAYEENAME = Model_Account::get_account_name(r.TOACCOUNTID);
    }

}

wxString Model_Billsdeposits::Full_Data::real_payee_name() const
{
    if (Model_Checking::TYPE_ID_TRANSFER == Model_Checking::type_id(this->TRANSCODE))
    {
        return ("> " + this->PAYEENAME);
    }
    return this->PAYEENAME;
}
