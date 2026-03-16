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

// -- instance methods --

// Remove the Data record instance from memory and the database
// including any splits associated with the Data Record.
bool SchedModel::purge_id(int64 sched_id)
{
    // purge SchedSplitData owned by sched_id
    for (auto& qp_d : SchedModel::get_data_qp_a(*get_id_data_n(sched_id)))
        SchedSplitModel::instance().purge_id(qp_d.m_id);

    // remove TagLinkData owned by sched_id
    TagLinkModel::instance().purge_ref(s_ref_type, sched_id);

    // FIXME: remove FieldValueData owned by sched_id
    // FIXME: remove AttachmentData owned by sched_id

    return unsafe_remove_id(sched_id);
}

const SchedSplitModel::DataA SchedModel::get_data_qp_a(const Data& sched_d)
{
    return SchedSplitModel::instance().find(
        SchedSplitCol::TRANSID(sched_d.m_id)
    );
}

const TagLinkModel::DataA SchedModel::get_data_gl_a(const Data& sched_d)
{
    return TagLinkModel::instance().find(
        TagLinkCol::REFTYPE(SchedModel::s_ref_type.name_n()),
        TagLinkCol::REFID(sched_d.m_id)
    );
}

bool SchedModel::is_data_allowed(const Data& sched_d)
{
    if (sched_d.is_void())
        return true;
    if (sched_d.is_deposit())
        return true;

    const AccountData* account_n = AccountModel::instance().get_id_data_n(sched_d.m_account_id);
    if (account_n->m_min_balance == 0 && account_n->m_credit_limit == 0)
        return true;

    double current_balance = AccountModel::instance().get_data_balance(*account_n);
    double new_balance = current_balance - sched_d.m_amount;

    bool is_allowed = true;
    wxString limitDescription;
    double limitAmount = 0.0L;
    if (account_n->m_min_balance != 0 && new_balance < account_n->m_min_balance) {
        is_allowed = false;
        limitDescription = _t("Minimum Balance");
        limitAmount = account_n->m_min_balance;
    }
    else if (account_n->m_credit_limit != 0 && new_balance < -(account_n->m_credit_limit)) {
        is_allowed = false;
        limitDescription = _t("Credit Limit");
        limitAmount = account_n->m_credit_limit;
    }

    if (!is_allowed) {
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
            is_allowed = true;
        }
    }

    return is_allowed;
}

void SchedModel::reschedule_id(int64 sched_id)
{
    Data* sched_n = unsafe_get_id_data_n(sched_id);
    if (!sched_n)
        return;

    if (sched_n->m_repeat.m_num == 1) {
        // this was the last repetition
        // FIXME: delete attachments inside purge_id()
        mmAttachmentManage::DeleteAllAttachments(s_ref_type, sched_id);
        purge_id(sched_id);
        return;
    }

    sched_n->m_date_time = mmDateTime(sched_n->m_repeat.next_datetime(
        sched_n->m_date_time.getDateTime()
    ));
    sched_n->m_due_date = mmDate(sched_n->m_repeat.next_datetime(
        sched_n->m_due_date.getDateTime()
    ));
    sched_n->m_repeat.next_repeat();
    unsafe_save_data_n(sched_n);
}

// -- DataExt --

SchedModel::DataExt::DataExt()
{}

SchedModel::DataExt::DataExt(const Data& sched_d) :
    Data(sched_d),
    m_bill_splits(SchedModel::instance().get_data_qp_a(sched_d)),
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

wxString SchedModel::DataExt::real_payee_name() const
{
    return is_transfer() ? ("> " + this->PAYEENAME) : this->PAYEENAME;
}
