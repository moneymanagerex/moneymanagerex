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

const std::vector<std::pair<Model_Billsdeposits::TYPE, wxString> > Model_Billsdeposits::TYPE_CHOICES =
{
    {Model_Billsdeposits::WITHDRAWAL, wxString(wxTRANSLATE("Withdrawal"))}
    , {Model_Billsdeposits::DEPOSIT, wxString(wxTRANSLATE("Deposit"))}
    , {Model_Billsdeposits::TRANSFER, wxString(wxTRANSLATE("Transfer"))}
};

const std::vector<std::pair<Model_Billsdeposits::STATUS_ENUM, wxString> > Model_Billsdeposits::STATUS_ENUM_CHOICES =
{
    {Model_Billsdeposits::NONE, wxString(wxTRANSLATE("Unreconciled"))}
    , {Model_Billsdeposits::RECONCILED, wxString(wxTRANSLATE("Reconciled"))}
    , {Model_Billsdeposits::VOID_, wxString(wxTRANSLATE("Void"))}
    , {Model_Billsdeposits::FOLLOWUP, wxString(wxTRANSLATE("Follow Up"))}
    , {Model_Billsdeposits::DUPLICATE_, wxString(wxTRANSLATE("Duplicate"))}
};

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

wxArrayString Model_Billsdeposits::all_type()
{
    wxArrayString types;
    for (const auto& item : TYPE_CHOICES) types.Add(item.second);
    return types;
}

wxArrayString Model_Billsdeposits::all_status()
{
    wxArrayString status;
    for (const auto& item : STATUS_ENUM_CHOICES) status.Add(item.second);
    return status;
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

Model_Billsdeposits::TYPE Model_Billsdeposits::type(const wxString& r)
{
    static std::unordered_map<wxString, TYPE> cache;
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

    cache.insert(std::make_pair(r, WITHDRAWAL));
    return WITHDRAWAL;
}
Model_Billsdeposits::TYPE Model_Billsdeposits::type(const Data& r)
{
    return type(r.TRANSCODE);
}
Model_Billsdeposits::TYPE Model_Billsdeposits::type(const Data* r)
{
    return type(r->TRANSCODE);
}
Model_Billsdeposits::STATUS_ENUM Model_Billsdeposits::status(const wxString& r)
{
    static std::unordered_map<wxString, STATUS_ENUM> cache;
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
Model_Billsdeposits::STATUS_ENUM Model_Billsdeposits::status(const Data& r)
{
    return status(r.STATUS);
}
Model_Billsdeposits::STATUS_ENUM Model_Billsdeposits::status(const Data* r)
{
    return status(r->STATUS);
}

wxString Model_Billsdeposits::toShortStatus(const wxString& fullStatus)
{
    wxString s = fullStatus.Left(1);
    s.Replace("U", "");
    return s;
}

/**
* Remove the Data record instance from memory and the database
* including any splits associated with the Data Record.
*/
bool Model_Billsdeposits::remove(int id)
{
    for (auto &item : Model_Billsdeposits::splittransaction(get(id)))
        Model_Budgetsplittransaction::instance().remove(item.SPLITTRANSID);
    // Delete tags for the scheduled transaction
    Model_Taglink::instance().DeleteAllTags(Model_Attachment::reftype_desc(Model_Attachment::BILLSDEPOSIT), id);
    return this->remove(id, db_);
}

DB_Table_BILLSDEPOSITS_V1::STATUS Model_Billsdeposits::STATUS(STATUS_ENUM status, OP op)
{
    return DB_Table_BILLSDEPOSITS_V1::STATUS(toShortStatus(all_status()[status]), op);
}

DB_Table_BILLSDEPOSITS_V1::TRANSCODE Model_Billsdeposits::TRANSCODE(TYPE type, OP op)
{
    return DB_Table_BILLSDEPOSITS_V1::TRANSCODE(all_type()[type], op);
}

const Model_Budgetsplittransaction::Data_Set Model_Billsdeposits::splittransaction(const Data* r)
{
    return Model_Budgetsplittransaction::instance().find(Model_Budgetsplittransaction::TRANSID(r->BDID));
}

const Model_Budgetsplittransaction::Data_Set Model_Billsdeposits::splittransaction(const Data& r)
{
    return Model_Budgetsplittransaction::instance().find(Model_Budgetsplittransaction::TRANSID(r.BDID));
}

void Model_Billsdeposits::decode_fields(const Data& q1)
{
    // DeMultiplex the Auto Executable fields from the db entry: REPEATS
    m_autoExecute = q1.REPEATS / BD_REPEATS_MULTIPLEX_BASE;
    m_allowExecution = true;

    int repeats = q1.REPEATS % BD_REPEATS_MULTIPLEX_BASE;
    int numRepeats = q1.NUMOCCURRENCES;
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

bool Model_Billsdeposits::AllowTransaction(const Data& r, AccountBalance& bal)
{
    const int acct_id = r.ACCOUNTID;
    Model_Account::Data* account = Model_Account::instance().get(acct_id);
    double current_account_balance = 0;

    AccountBalance::iterator itr_bal = bal.find(acct_id);
    if (itr_bal != bal.end())
    {
        current_account_balance = itr_bal->second;
    }
    else
    {
        current_account_balance = Model_Account::balance(account);
        bal[acct_id] = current_account_balance;
    }

    double new_value = r.TRANSAMOUNT;

    if (r.TRANSCODE == Model_Checking::all_type()[Model_Checking::WITHDRAWAL])
    {
        new_value *= -1;
    }
    new_value += current_account_balance;

    bool abort_transaction = false;
    if ((account->MINIMUMBALANCE != 0) && (new_value < account->MINIMUMBALANCE))
    {
        abort_transaction = true;
    }

    if ((account->CREDITLIMIT != 0) && (new_value < (account->CREDITLIMIT * -1)))
    {
        abort_transaction = true;
    }

    if (abort_transaction)
    {
        wxString message = _("A scheduled transaction will exceed your account limit.\n\n"
            "Account: %1$s\n"
            "Current Balance: %2$6.2f\n"
            "Transaction amount: %3$6.2f\n"
            "%4$s: %5$6.2f\n\n"
            "Do you wish to continue?"
        );

        wxString limitDescription;
        double limitAmount{ 0.0L };

        if (account->MINIMUMBALANCE > 0)
        {
            limitDescription = _("Minimum Balance");
            limitAmount = account->MINIMUMBALANCE;
        }

        if (account->CREDITLIMIT > 0)
        {
            limitDescription = _("Credit Limit");
            limitAmount = account->CREDITLIMIT;
        }

        message.Printf(message, account->ACCOUNTNAME, current_account_balance, r.TRANSAMOUNT, limitDescription, limitAmount);

        if (wxMessageBox(message, _("MMEX Scheduled Transaction Check"), wxYES_NO | wxICON_WARNING) == wxYES)
        {
            abort_transaction = false;
        }
    }

    if (!abort_transaction)
    {
        bal[acct_id] = new_value;
    }

    return !abort_transaction;
}

void Model_Billsdeposits::completeBDInSeries(int bdID)
{
    Data* bill = get(bdID);
    if (!bill) return;

    int repeats = bill->REPEATS % BD_REPEATS_MULTIPLEX_BASE; // DeMultiplex the Auto Executable fields.
    int numRepeats = bill->NUMOCCURRENCES;

    if ((repeats == REPEAT_TYPE::REPEAT_ONCE) || ((repeats < REPEAT_TYPE::REPEAT_IN_X_DAYS || repeats > REPEAT_TYPE::REPEAT_EVERY_X_MONTHS) && numRepeats == 1))
    {
        mmAttachmentManage::DeleteAllAttachments(Model_Attachment::reftype_desc(Model_Attachment::BILLSDEPOSIT), bdID);
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

Model_Billsdeposits::Full_Data::Full_Data()
{}

Model_Billsdeposits::Full_Data::Full_Data(const Data& r) : Data(r)
{
    m_bill_splits = splittransaction(r);

    m_tags = Model_Taglink::instance().find(Model_Taglink::REFTYPE(Model_Attachment::reftype_desc(Model_Attachment::BILLSDEPOSIT)), Model_Taglink::REFID(r.BDID));

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
            for (const auto& tag : Model_Taglink::instance().get(Model_Attachment::reftype_desc(Model_Attachment::BILLSDEPOSITSPLIT), entry.SPLITTRANSID))
                splitTags.Append(tag.first + " ");
            if (!splitTags.IsEmpty())
                TAGNAMES.Append((TAGNAMES.IsEmpty() ? "" : ", ") + splitTags.Trim());
        }
    }
    else
        CATEGNAME = Model_Category::full_name(r.CATEGID);

    ACCOUNTNAME = Model_Account::get_account_name(r.ACCOUNTID);

    PAYEENAME = Model_Payee::get_payee_name(r.PAYEEID);
    if (Model_Billsdeposits::type(r) == Model_Billsdeposits::TRANSFER)
    {
        PAYEENAME = Model_Account::get_account_name(r.TOACCOUNTID);
    }

}

wxString Model_Billsdeposits::Full_Data::real_payee_name() const
{
    if (TYPE::TRANSFER == type(this->TRANSCODE))
    {
        return ("> " + this->PAYEENAME);
    }
    return this->PAYEENAME;
}
