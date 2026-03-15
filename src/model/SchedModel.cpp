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
#include "PrefModel.h"
#include "SchedModel.h"
#include "TagModel.h"

 // TODO: Move attachment management outside of AttachmentDialog
#include "dialog/AttachmentDialog.h"

const RefTypeN SchedModel::s_ref_type = RefTypeN(RefTypeN::e_sched);

// -- constructor --

SchedModel::SchedModel() :
    TableFactory<SchedTable, SchedData>()
{
}

SchedModel::~SchedModel()
{
}

// -- static methods --

// Initialize the global SchedModel table.
// Reset the SchedModel table or create the table if it does not exist.
SchedModel& SchedModel::instance(wxSQLite3Database* db)
{
    SchedModel& ins = Singleton<SchedModel>::instance();
    ins.reset_cache();
    ins.m_db = db;
    ins.ensure_table();

    return ins;
}

// Return the static instance of SchedModel table
SchedModel& SchedModel::instance()
{
    return Singleton<SchedModel>::instance();
}

SchedCol::TRANSCODE SchedModel::TYPE(OP op, TrxType sched_type)
{
    return SchedCol::TRANSCODE(op, sched_type.name());
}

SchedCol::STATUS SchedModel::STATUS(OP op, TrxStatus sched_status)
{
    return SchedCol::STATUS(op, sched_status.key());
}

SchedCol::STATUS SchedModel::IS_VOID(bool value)
{
    return SchedCol::STATUS(value ? OP_EQ : OP_NE, TrxStatus(TrxStatus::e_void).key());
}

bool SchedModel::encode_repeat_num(Data& this_d, const RepeatNum& rn)
{
    if (rn.freq == REPEAT_FREQ_INVALID)
        return false;

    this_d.REPEATS = rn.exec * BD_REPEATS_MULTIPLEX_BASE + rn.freq;
    this_d.NUMOCCURRENCES = (
        rn.freq >= SchedModel::REPEAT_FREQ_IN_X_DAYS &&
        rn.freq <= SchedModel::REPEAT_FREQ_EVERY_X_MONTHS
    ) ? rn.x : rn.num;

    return true;
}

bool SchedModel::decode_repeat_num(const Data& this_d, RepeatNum& rn)
{
    rn.exec = static_cast<REPEAT_EXEC>(this_d.REPEATS.GetValue() / BD_REPEATS_MULTIPLEX_BASE);
    rn.freq = static_cast<REPEAT_FREQ>(this_d.REPEATS.GetValue() % BD_REPEATS_MULTIPLEX_BASE);
    rn.num  = static_cast<int>(this_d.NUMOCCURRENCES.GetValue());
    rn.x    = REPEAT_X_VOID;

    if (rn.freq == REPEAT_FREQ_ONCE) {
        rn.num = 1;
    }
    else if (rn.freq >= REPEAT_FREQ_IN_X_DAYS && rn.freq <= REPEAT_FREQ_IN_X_MONTHS) {
        rn.x = rn.num > 0 ? rn.num : REPEAT_X_INVALID;
        rn.num = 2;
    }
    else if (rn.freq >= REPEAT_FREQ_EVERY_X_DAYS && rn.freq <= REPEAT_FREQ_EVERY_X_MONTHS) {
        rn.x = rn.num > 0 ? rn.num : REPEAT_X_INVALID;
        rn.num = REPEAT_NUM_INFINITY;
    }
    else if (rn.num < 1 && rn.num != REPEAT_NUM_INFINITY) {
        rn.num = REPEAT_NUM_INVALID;
    }

    return (rn.freq >= 0 && rn.freq < REPEAT_FREQ_size &&
        rn.num != REPEAT_NUM_INVALID && rn.x != REPEAT_X_INVALID
    );
}

bool SchedModel::next_repeat_num(RepeatNum& rn)
{
    if (rn.freq == REPEAT_FREQ_INVALID)
        return false;

    if (rn.num > 0)
        --rn.num;

    if (rn.num == 0)
        return false;

    // change REPEAT_FREQ_IN_X_* to REPEAT_FREQ_ONCE
    if (rn.x != REPEAT_X_VOID && rn.num == 1)
        rn.freq = REPEAT_FREQ_ONCE;

    return true;
}

bool SchedModel::requires_execution(const Data& this_d)
{
    return (
        this_d.m_due_date.getDateTime().
            Subtract(wxDate::Today()).GetSeconds().GetValue()
        / 86400 < 1
    );
}

const wxDateTime SchedModel::nextOccurDate(
    wxDateTime this_date,
    const RepeatNum& rn,
    bool reverse
) {
    int k = reverse ? -1 : 1;

    wxDateTime next_date = this_date;
    if (rn.freq == REPEAT_FREQ_WEEKLY)
        next_date.Add(wxTimeSpan::Weeks(k));
    else if (rn.freq == REPEAT_FREQ_BI_WEEKLY)
        next_date.Add(wxTimeSpan::Weeks(2 * k));
    else if (rn.freq == REPEAT_FREQ_MONTHLY)
        next_date.Add(wxDateSpan::Months(k));
    else if (rn.freq == REPEAT_FREQ_BI_MONTHLY)
        next_date.Add(wxDateSpan::Months(2 * k));
    else if (rn.freq == REPEAT_FREQ_FOUR_MONTHLY)
        next_date.Add(wxDateSpan::Months(4 * k));
    else if (rn.freq == REPEAT_FREQ_HALF_YEARLY)
        next_date.Add(wxDateSpan::Months(6 * k));
    else if (rn.freq == REPEAT_FREQ_YEARLY)
        next_date.Add(wxDateSpan::Years(k));
    else if (rn.freq == REPEAT_FREQ_QUARTERLY)
        next_date.Add(wxDateSpan::Months(3 * k));
    else if (rn.freq == REPEAT_FREQ_FOUR_WEEKLY)
        next_date.Add(wxDateSpan::Weeks(4 * k));
    else if (rn.freq == REPEAT_FREQ_DAILY)
        next_date.Add(wxDateSpan::Days(k));
    else if (rn.freq == REPEAT_FREQ_IN_X_DAYS)
        next_date.Add(wxDateSpan::Days(rn.x * k));
    else if (rn.freq == REPEAT_FREQ_IN_X_MONTHS)
        next_date.Add(wxDateSpan::Months(rn.x * k));
    else if (rn.freq == REPEAT_FREQ_EVERY_X_DAYS)
        next_date.Add(wxDateSpan::Days(rn.x * k));
    else if (rn.freq == REPEAT_FREQ_EVERY_X_MONTHS)
        next_date.Add(wxDateSpan::Months(rn.x * k));
    else if (rn.freq == REPEAT_FREQ_MONTHLY_LAST_DAY ||
             rn.freq == REPEAT_FREQ_MONTHLY_LAST_BUSINESS_DAY)
    {
        next_date.Add(wxDateSpan::Months(k));
        next_date.SetToLastMonthDay(next_date.GetMonth(), next_date.GetYear());
        if (rn.freq == REPEAT_FREQ_MONTHLY_LAST_BUSINESS_DAY) {
            // last weekday of month
            if (next_date.GetWeekDay() == wxDateTime::Sun ||
                next_date.GetWeekDay() == wxDateTime::Sat
            )
                next_date.SetToPrevWeekDay(wxDateTime::Fri);
        }
    }
    wxLogDebug("init date: %s -> next date: %s",
        this_date.FormatISOCombined(), next_date.FormatISOCombined()
    );
    return next_date;
}

wxArrayString SchedModel::unroll(const Data& sched_d, const wxString end_date, int limit)
{
    wxArrayString dates;

    RepeatNum rn;
    if (!decode_repeat_num(sched_d, rn))
        return dates;

    wxString date = sched_d.m_date_time.isoDateTime();
    while (date <= end_date && limit != 0) {
        if (limit > 0)
            --limit;
        dates.push_back(date);

        if (rn.num == 1)
            break;

        wxDateTime date_curr;
        date_curr.ParseDateTime(date) || date_curr.ParseDate(date);
        const wxDateTime& date_next = nextOccurDate(date_curr, rn);
        date = date_next.FormatISOCombined();

        next_repeat_num(rn);
    }

    return dates;
}

const SchedSplitModel::DataA SchedModel::split(const Data& sched_d)
{
    return SchedSplitModel::instance().find(
        SchedSplitCol::TRANSID(sched_d.m_id)
    );
}

const TagLinkModel::DataA SchedModel::taglink(const Data& sched_d)
{
    return TagLinkModel::instance().find(
        TagLinkCol::REFTYPE(SchedModel::s_ref_type.name_n()),
        TagLinkCol::REFID(sched_d.m_id)
    );
}

// -- instance methods --

// Remove the Data record instance from memory and the database
// including any splits associated with the Data Record.
bool SchedModel::purge_id(int64 sched_id)
{
    // purge SchedSplitData owned by sched_id
    for (auto& qp_d : SchedModel::split(*get_id_data_n(sched_id)))
        SchedSplitModel::instance().purge_id(qp_d.m_id);

    // remove TagLinkData owned by sched_id
    TagLinkModel::instance().purge_ref(s_ref_type, sched_id);

    // FIXME: remove FieldValueData owned by sched_id
    // FIXME: remove AttachmentData owned by sched_id

    return unsafe_remove_id(sched_id);
}

bool SchedModel::AllowTransaction(const Data& sched_d)
{
    if (sched_d.is_void())
        return true;
    if (!sched_d.is_withdrawal() && !sched_d.is_transfer())
        return true;

    const int64 acct_id = sched_d.m_account_id;
    const AccountData* account_n = AccountModel::instance().get_id_data_n(acct_id);

    if (account_n->m_min_balance == 0 && account_n->m_credit_limit == 0)
        return true;

    double current_balance = AccountModel::instance().get_data_balance(*account_n);
    double new_balance = current_balance - sched_d.m_amount;

    bool allow_transaction = true;
    wxString limitDescription;
    double limitAmount{ 0.0L };
    if (account_n->m_min_balance != 0 && new_balance < account_n->m_min_balance) {
        allow_transaction = false;
        limitDescription = _t("Minimum Balance");
        limitAmount = account_n->m_min_balance;
    }
    else if (account_n->m_credit_limit != 0 && new_balance < -(account_n->m_credit_limit)) {
        allow_transaction = false;
        limitDescription = _t("Credit Limit");
        limitAmount = account_n->m_credit_limit;
    }

    if (!allow_transaction) {
        wxString message = _t("A scheduled transaction will exceed the account limit.\n\n"
            "Account: %1$s\n"
            "Current Balance: %2$6.2f\n"
            "Transaction amount: %3$6.2f\n"
            "%4$s: %5$6.2f") + "\n\n" +
            _t("Do you want to continue?");
        message.Printf(message,
            account_n->m_name, current_balance, sched_d.m_amount,
            limitDescription, limitAmount
        );

        if (wxMessageBox(message,
            _t("MMEX Scheduled Transaction Check"),
            wxYES_NO | wxICON_WARNING
        ) == wxYES) {
            allow_transaction = true;
        }
    }

    return allow_transaction;
}

void SchedModel::completeBDInSeries(int64 sched_id)
{
    Data* sched_n = unsafe_get_id_data_n(sched_id);
    if (!sched_n)
        return;

    RepeatNum rn;
    if (!decode_repeat_num(*sched_n, rn) || rn.num == 1) {
        mmAttachmentManage::DeleteAllAttachments(s_ref_type, sched_id);
        purge_id(sched_id);
        return;
    }

    const wxDateTime& payment_date_current = sched_n->m_date_time.getDateTime();
    const wxDateTime& payment_date_update = nextOccurDate(payment_date_current, rn);
    sched_n->m_date_time = mmDateTime(payment_date_update);

    const wxDateTime& due_date_current = sched_n->m_due_date.getDateTime();
    const wxDateTime& due_date_update = nextOccurDate(due_date_current, rn);
    sched_n->m_due_date = mmDate(due_date_update);

    next_repeat_num(rn);
    encode_repeat_num(*sched_n, rn);

    unsafe_save_data_n(sched_n);
}

// -- Full_Data --

SchedModel::Full_Data::Full_Data()
{}

SchedModel::Full_Data::Full_Data(const Data& sched_d) :
    Data(sched_d),
    m_bill_splits(split(sched_d)),
    m_tags(TagLinkModel::instance().find(
        TagLinkCol::REFTYPE(SchedModel::s_ref_type.name_n()),
        TagLinkCol::REFID(sched_d.m_id)
    ))
{
    if (!m_tags.empty()) {
        wxArrayString tagnames;
        for (const auto& gl_d : m_tags)
            tagnames.Add(TagModel::instance().get_id_data_n(gl_d.m_tag_id)->m_name);
        // Sort TAGNAMES
        tagnames.Sort();
        for (const auto& name : tagnames)
            this->TAGNAMES += (this->TAGNAMES.empty() ? "" : " ") + name;
    }

    if (!m_bill_splits.empty()) {
        for (const auto& qp_d : m_bill_splits) {
            CATEGNAME += (CATEGNAME.empty() ? " + " : ", ")
                + CategoryModel::instance().get_id_fullname(qp_d.m_category_id);

            wxString splitTags;
            for (const auto& tag_name_id : TagLinkModel::instance().find_ref_tag_m(
                SchedSplitModel::s_ref_type, qp_d.m_id
            )) {
                splitTags.Append(tag_name_id.first + " ");
            }
            if (!splitTags.IsEmpty())
                TAGNAMES.Append((TAGNAMES.IsEmpty() ? "" : ", ") + splitTags.Trim());
        }
    }
    else
        CATEGNAME = CategoryModel::instance().get_id_fullname(sched_d.m_category_id_n);

    ACCOUNTNAME = AccountModel::instance().get_id_name(sched_d.m_account_id);

    PAYEENAME = PayeeModel::instance().get_id_name(sched_d.m_payee_id_n);
    if (sched_d.is_transfer()) {
        PAYEENAME = AccountModel::instance().get_id_name(sched_d.m_to_account_id_n);
    }
}

wxString SchedModel::Full_Data::real_payee_name() const
{
    return is_transfer() ? ("> " + this->PAYEENAME) : this->PAYEENAME;
}
