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

#include "Model_Billsdeposits.h"
#include "option.h"
#include "Model_Account.h"
#include "Model_Attachment.h"
#include "Model_Category.h"
#include "Model_Payee.h"

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
    {Model_Billsdeposits::NONE, wxString(wxTRANSLATE("None"))}
    , {Model_Billsdeposits::RECONCILED, wxString(wxTRANSLATE("Reconciled"))}
    , {Model_Billsdeposits::VOID_, wxString(wxTRANSLATE("Void"))}
    , {Model_Billsdeposits::FOLLOWUP, wxString(wxTRANSLATE("Follow up"))}
    , {Model_Billsdeposits::DUPLICATE_, wxString(wxTRANSLATE("Duplicate"))}
};

Model_Billsdeposits::Model_Billsdeposits()
: Model<DB_Table_BILLSDEPOSITS>()
, m_autoExecuteManual (false)
, m_autoExecuteSilent (false)
, m_requireExecution (false)
, m_allowExecution (false)

{
}

Model_Billsdeposits::~Model_Billsdeposits()
{
}

wxArrayString Model_Billsdeposits::all_type()
{
    static wxArrayString types;
    if (types.empty())
    {
        for (const auto& item : TYPE_CHOICES)
            types.Add(item.second);
    }
    return types;
}

wxArrayString Model_Billsdeposits::all_status()
{
    static wxArrayString status;
    if (status.empty())
    {
        for (const auto& item : STATUS_ENUM_CHOICES)
            status.Add(item.second);
    }
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
    return fullStatus.Left(1) == "N" ? "" : fullStatus.Left(1);
}

/**
* Remove the Data record instance from memory and the database
* including any splits associated with the Data Record.
*/
bool Model_Billsdeposits::remove(int id)
{
    for (auto &item : Model_Billsdeposits::splittransaction(get(id)))
        Model_Budgetsplittransaction::instance().remove(item.SPLITTRANSID);
    return this->remove(id, db_);
}

DB_Table_BILLSDEPOSITS::STATUS Model_Billsdeposits::STATUS(STATUS_ENUM status, OP op)
{
    return DB_Table_BILLSDEPOSITS::STATUS(toShortStatus(all_status()[status]), op);
}

DB_Table_BILLSDEPOSITS::TRANSCODE Model_Billsdeposits::TRANSCODE(TYPE type, OP op)
{
    return DB_Table_BILLSDEPOSITS::TRANSCODE(all_type()[type], op);
}

const Model_Budgetsplittransaction::Data_Set Model_Billsdeposits::splittransaction(const Data* r)
{
    return Model_Budgetsplittransaction::instance().find(Model_Budgetsplittransaction::TRANSID(r->BDID));
}

const Model_Budgetsplittransaction::Data_Set Model_Billsdeposits::splittransaction(const Data& r)
{
    return Model_Budgetsplittransaction::instance().find(Model_Budgetsplittransaction::TRANSID(r.BDID));
}

bool Model_Billsdeposits::Full_Data::has_split() const
{
    return !this->m_splits.empty();
}

void Model_Billsdeposits::decode_fields(const Data& q1)
{
    m_autoExecuteManual = false; // Used when decoding: REPEATS
    m_autoExecuteSilent = false;
    m_requireExecution = false;
    m_allowExecution = false;

    // DeMultiplex the Auto Executable fields from the db entry: REPEATS
    int repeats = q1.REPEATS;
    int numRepeats = q1.NUMOCCURRENCES;

    if (repeats >= BD_REPEATS_MULTIPLEX_BASE)    // Auto Execute User Acknowlegement required
    {
        m_autoExecuteManual = true;
        repeats -= BD_REPEATS_MULTIPLEX_BASE;
    }

    if (repeats >= BD_REPEATS_MULTIPLEX_BASE)    // Auto Execute Silent mode
    {
        m_autoExecuteManual = false;               // Can only be manual or auto. Not both
        m_autoExecuteSilent = true;
        repeats -= BD_REPEATS_MULTIPLEX_BASE;
    }

    if ((repeats < Model_Billsdeposits::REPEAT_IN_X_DAYS) || (numRepeats > Model_Billsdeposits::REPEAT_NONE) || (repeats > Model_Billsdeposits::REPEAT_EVERY_X_MONTHS))
    {
        m_allowExecution = true;
    }

    if (this->daysPayment(&q1) < 1)
    {
        m_requireExecution = true;
    }
}

bool Model_Billsdeposits::autoExecuteManual()
{
    return m_autoExecuteManual;
}

bool Model_Billsdeposits::autoExecuteSilent()
{
    return m_autoExecuteSilent;
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

    if (abort_transaction && wxMessageBox(_(
        "A recurring transaction will exceed your account limit.\n\n"
        "Do you wish to continue?")
        , _("MMEX Recurring Transaction Check"), wxYES_NO | wxICON_WARNING) == wxYES)
    {
        abort_transaction = false;
    }

	if (!abort_transaction)
	{
		bal[acct_id] = new_value;
	}

    return !abort_transaction;
}

int Model_Billsdeposits::daysPayment(const Data* r)
{
    const wxDate& payment_date = Model_Billsdeposits::NEXTOCCURRENCEDATE(r);
    wxTimeSpan ts = payment_date.Subtract(wxDateTime::Now());
    int daysRemaining = ts.GetDays();
    int minutesRemaining = ts.GetMinutes();

    if (minutesRemaining > 0)
        daysRemaining += 1;

    return daysRemaining;
}

int Model_Billsdeposits::daysOverdue(const Data* r)
{
    const wxDate& overdue_date = Model_Billsdeposits::TRANSDATE(r);
    wxTimeSpan ts = overdue_date.Subtract(wxDateTime::Now());
    int daysRemaining = ts.GetDays();
    int minutesRemaining = ts.GetMinutes();

    if (minutesRemaining > 0)
        daysRemaining += 1;

    return daysRemaining;
}

void Model_Billsdeposits::completeBDInSeries(int bdID)
{
    Data* bill = get(bdID);
    if (bill)
    {
        int repeats = bill->REPEATS;
        // DeMultiplex the Auto Executable fields.
        if (repeats >= BD_REPEATS_MULTIPLEX_BASE)    // Auto Execute User Acknowlegement required
            repeats -= BD_REPEATS_MULTIPLEX_BASE;
        if (repeats >= BD_REPEATS_MULTIPLEX_BASE)    // Auto Execute Silent mode
            repeats -= BD_REPEATS_MULTIPLEX_BASE;
        int numRepeats = bill->NUMOCCURRENCES;
        const wxDateTime& payment_date_current = NEXTOCCURRENCEDATE(bill);
        const wxDateTime& payment_date_update = nextOccurDate(repeats, numRepeats, payment_date_current);

        const wxDateTime& due_date_current = TRANSDATE(bill);
        const wxDateTime& due_date_update = nextOccurDate(repeats, numRepeats, due_date_current);

        if (numRepeats != REPEAT_TYPE::REPEAT_INACTIVE)
        {
            if ((repeats < REPEAT_TYPE::REPEAT_IN_X_DAYS) || (repeats > REPEAT_TYPE::REPEAT_EVERY_X_MONTHS))
                numRepeats--;
        }

        if (repeats == REPEAT_TYPE::REPEAT_NONE)
            numRepeats = 0;
        else if ((repeats == REPEAT_TYPE::REPEAT_IN_X_DAYS)
            || (repeats == REPEAT_TYPE::REPEAT_IN_X_MONTHS))
        {
            if (numRepeats != -1) numRepeats = -1;
        }

        bill->NEXTOCCURRENCEDATE = payment_date_update.FormatISODate();
        bill->TRANSDATE = due_date_update.FormatISODate();

        // Ensure that TRANDSATE is set correctly
        if (payment_date_current > due_date_current)
            bill->TRANSDATE = payment_date_update.FormatISODate();

        bill->NUMOCCURRENCES = numRepeats;
        save(bill);

        if (bill->NUMOCCURRENCES == REPEAT_TYPE::REPEAT_NONE)
        {
            mmAttachmentManage::DeleteAllAttachments(Model_Attachment::reftype_desc(Model_Attachment::BILLSDEPOSIT), bdID);
            remove(bdID);
        }
    }
}

const wxDateTime Model_Billsdeposits::nextOccurDate(int repeatsType, int numRepeats, const wxDateTime& nextOccurDate)
{
    wxDateTime dt = nextOccurDate;
    if (repeatsType == REPEAT_WEEKLY)
        dt = dt.Add(wxTimeSpan::Week());
    else if (repeatsType == REPEAT_BI_WEEKLY)
        dt = dt.Add(wxTimeSpan::Weeks(2));
    else if (repeatsType == REPEAT_MONTHLY)
        dt = dt.Add(wxDateSpan::Month());
    else if (repeatsType == REPEAT_BI_MONTHLY)
        dt = dt.Add(wxDateSpan::Months(2));
    else if (repeatsType == REPEAT_FOUR_MONTHLY)
        dt = dt.Add(wxDateSpan::Months(4));
    else if (repeatsType == REPEAT_HALF_YEARLY)
        dt = dt.Add(wxDateSpan::Months(6));
    else if (repeatsType == REPEAT_YEARLY)
        dt = dt.Add(wxDateSpan::Year());
    else if (repeatsType == REPEAT_QUARTERLY)
        dt = dt.Add(wxDateSpan::Months(3));
    else if (repeatsType == REPEAT_FOUR_WEEKLY)
        dt = dt.Add(wxDateSpan::Weeks(4));
    else if (repeatsType == REPEAT_DAILY)
        dt = dt.Add(wxDateSpan::Days(1));
    else if (repeatsType == REPEAT_IN_X_DAYS) // repeat in numRepeats Days (Once only)
        dt = dt.Add(wxDateSpan::Days(numRepeats));
    else if (repeatsType == REPEAT_IN_X_MONTHS) // repeat in numRepeats Months (Once only)
        dt = dt.Add(wxDateSpan::Months(numRepeats));
    else if (repeatsType == REPEAT_EVERY_X_DAYS) // repeat every numRepeats Days
        dt = dt.Add(wxDateSpan::Days(numRepeats));
    else if (repeatsType == REPEAT_EVERY_X_MONTHS) // repeat every numRepeats Months
        dt = dt.Add(wxDateSpan::Months(numRepeats));
    else if (repeatsType == REPEAT_MONTHLY_LAST_DAY
        || REPEAT_MONTHLY_LAST_BUSINESS_DAY == repeatsType)
    {
        dt = dt.Add(wxDateSpan::Month());
        dt = dt.SetToLastMonthDay(dt.GetMonth(), dt.GetYear());
        if (repeatsType == REPEAT_MONTHLY_LAST_BUSINESS_DAY) // last weekday of month
        {
            if (dt.GetWeekDay() == wxDateTime::Sun || dt.GetWeekDay() == wxDateTime::Sat)
                dt.SetToPrevWeekDay(wxDateTime::Fri);
        }
    }
    //wxLogDebug("init date: %s -> next date: %s", nextOccurDate.FormatISODate(), dt.FormatISODate());
    return dt;
}

Model_Billsdeposits::Full_Data::Full_Data()
{}

Model_Billsdeposits::Full_Data::Full_Data(const Data& r) : Data(r)
{
    m_splits = splittransaction(r);
    if (!m_splits.empty())
    {
        for (const auto& entry : m_splits)
            CATEGNAME += (CATEGNAME.empty() ? " * " : ", ")
            + Model_Category::full_name(entry.CATEGID, entry.SUBCATEGID);
    }
    else
        CATEGNAME = Model_Category::full_name(r.CATEGID, r.SUBCATEGID);

    ACCOUNTNAME = Model_Account::get_account_name(r.ACCOUNTID);

    PAYEENAME = Model_Payee::get_payee_name(r.PAYEEID);
    if (Model_Billsdeposits::type(r) == Model_Billsdeposits::TRANSFER)
    {
        PAYEENAME = Model_Account::get_account_name(r.TOACCOUNTID);
    }
}
