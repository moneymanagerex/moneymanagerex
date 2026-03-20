/*******************************************************
 Copyright (C) 2024 George Ef (george.a.ef@gmail.com)

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

#include "Journal.h"

TrxData Journal::execute_bill(const SchedData& sched_d, mmDateTime date_time)
{
    TrxData trx_d;
    trx_d.m_id              = -1;
    trx_d.m_date_time       = date_time;
    trx_d.m_type            = sched_d.m_type;
    trx_d.m_status          = sched_d.m_status;
    trx_d.m_account_id      = sched_d.m_account_id;
    trx_d.m_to_account_id_n = sched_d.m_to_account_id_n;
    trx_d.m_payee_id_n      = sched_d.m_payee_id_n;
    trx_d.m_category_id_n   = sched_d.m_category_id_n;
    trx_d.m_amount          = sched_d.m_amount;
    trx_d.m_to_amount       = sched_d.m_to_amount;
    trx_d.m_number          = sched_d.m_number;
    trx_d.m_notes           = sched_d.m_notes;
    trx_d.m_followup_id     = sched_d.m_followup_id;
    trx_d.m_color           = sched_d.m_color;
    return trx_d;
}

TrxModel::DataExt Journal::execute_bill_full(const SchedData& sched_d, mmDateTime date_time)
{
    TrxModel::DataExt trx_dx;
    trx_dx.m_id              = -1;
    trx_dx.m_date_time       = date_time;
    trx_dx.m_type            = sched_d.m_type;
    trx_dx.m_status          = sched_d.m_status;
    trx_dx.m_account_id      = sched_d.m_account_id;
    trx_dx.m_to_account_id_n = sched_d.m_to_account_id_n;
    trx_dx.m_payee_id_n      = sched_d.m_payee_id_n;
    trx_dx.m_category_id_n   = sched_d.m_category_id_n;
    trx_dx.m_amount          = sched_d.m_amount;
    trx_dx.m_to_amount       = sched_d.m_to_amount;
    trx_dx.m_number          = sched_d.m_number;
    trx_dx.m_notes           = sched_d.m_notes;
    trx_dx.m_followup_id     = sched_d.m_followup_id;
    trx_dx.m_color           = sched_d.m_color;
    return trx_dx;
}

TrxSplitModel::DataA Journal::execute_splits(const SchedSplitModel::DataA& qp_a)
{
    TrxSplitModel::DataA tp_a;
    for (auto& qp_d : qp_a) {
        TrxSplitData tp_d;
        // FIXME: tp_d.m_id is invalid
        tp_d.m_id          = qp_d.m_id;
        tp_d.m_trx_id      = -1;
        tp_d.m_category_id = qp_d.m_category_id;
        tp_d.m_amount      = qp_d.m_amount;
        tp_d.m_notes       = qp_d.m_notes;
        tp_a.push_back(tp_d);
    }
    return tp_a;
}

Journal::Data::Data() :
    TrxData(),
    m_sched_id(-1), m_repeat_id(-1)
{
}

Journal::Data::Data(const TrxData& trx_d) :
    TrxData(trx_d),
    m_sched_id(-1), m_repeat_id(-1)
{
}

Journal::Data::Data(const SchedData& sched_d) :
    Data(sched_d, sched_d.m_date_time, 1)
{
}

Journal::Data::Data(const SchedData& sched_d, mmDateTime date, int repeat_id) :
    TrxData(execute_bill(sched_d, date)),
    m_sched_id(sched_d.m_id), m_repeat_id(repeat_id)
{
    if (m_repeat_id < 1) {
        wxFAIL;
    }
}

Journal::Data::~Data()
{
}

Journal::DataExt::DataExt(const TrxData& trx_d) :
    TrxModel::DataExt(trx_d),
    m_sched_id(-1), m_repeat_id(-1)
{
}

Journal::DataExt::DataExt(
    const TrxData& trx_d,
    const std::map<int64, TrxSplitModel::DataA>& trxId_tpA_m,
    const std::map<int64, TagLinkModel::DataA>& trxId_glA_m
) :
    TrxModel::DataExt(trx_d, trxId_tpA_m, trxId_glA_m),
    m_sched_id(-1), m_repeat_id(-1)
{
}

Journal::DataExt::DataExt(const SchedData& sched_d) :
    DataExt(sched_d, sched_d.m_date_time.isoDateTime(), 1)
{
}

Journal::DataExt::DataExt(
    const SchedData& sched_d, mmDateTime date, int repeat_id
) :
    TrxModel::DataExt(execute_bill_full(sched_d, date), {}, {}),
    m_sched_id(sched_d.m_id), m_repeat_id(repeat_id)
{
    if (m_repeat_id < 1) {
        wxFAIL;
    }

    m_tp_a = execute_splits(SchedModel::instance().find_id_qp_a(sched_d.m_id));
    m_gl_a =                SchedModel::instance().find_id_gl_a(sched_d.m_id);

    TrxModel::DataExt::fill_data();
    displayID = wxString("");
}

Journal::DataExt::DataExt(
    const SchedData& sched_d, mmDateTime date, int repeat_id,
    const std::map<int64, SchedSplitModel::DataA>& schedId_qpA_m,
    const std::map<int64, TagLinkModel::DataA>& schedId_glA_m)
:
    TrxModel::DataExt(execute_bill_full(sched_d, date), {}, {}),
    m_sched_id(sched_d.m_id), m_repeat_id(repeat_id)
{
    if (m_repeat_id < 1) {
        wxFAIL;
    }

    const auto schedId_qpA = schedId_qpA_m.find(m_sched_id);
    if (schedId_qpA != schedId_qpA_m.end()) {
        m_tp_a = execute_splits(schedId_qpA->second);
    }

    const auto schedId_glA = schedId_glA_m.find(m_sched_id);
    if (schedId_glA != schedId_glA_m.end())
        m_gl_a = schedId_glA->second;

    TrxModel::DataExt::fill_data();
    displayID = wxString("");
}

Journal::DataExt::~DataExt()
{
}


void Journal::setEmptyData(Journal::Data& journal_d, int64 account_id)
{
    TrxModel::instance().setEmptyData(journal_d, account_id);
    journal_d.m_sched_id  = -1;
    journal_d.m_repeat_id = -1;
}

bool Journal::setJournalData(Journal::Data& journal_d, JournalKey journal_key)
{
    if (journal_key.is_realized()) {
        const TrxData *trx_n = TrxModel::instance().get_id_data_n(journal_key.rid());
        if (!trx_n)
            return false;
        journal_d.m_id              = trx_n->m_id;
        journal_d.m_sched_id        = -1;
        journal_d.m_repeat_id       = -1;
        journal_d.m_date_time       = trx_n->m_date_time;
        journal_d.m_type            = trx_n->m_type;
        journal_d.m_status          = trx_n->m_status;
        journal_d.m_account_id      = trx_n->m_account_id;
        journal_d.m_to_account_id_n = trx_n->m_to_account_id_n;
        journal_d.m_payee_id_n      = trx_n->m_payee_id_n;
        journal_d.m_category_id_n   = trx_n->m_category_id_n;
        journal_d.m_amount          = trx_n->m_amount;
        journal_d.m_to_amount       = trx_n->m_to_amount;
        journal_d.m_number          = trx_n->m_number;
        journal_d.m_notes           = trx_n->m_notes;
        journal_d.m_followup_id     = trx_n->m_followup_id;
        journal_d.m_color           = trx_n->m_color;
        journal_d.m_updated_time_n  = trx_n->m_updated_time_n;
        journal_d.m_deleted_time_n  = trx_n->m_deleted_time_n;
    }
    else {
        const SchedData *sched_n = SchedModel::instance().get_id_data_n(journal_key.sid());
        if (!sched_n)
            return false;
        journal_d.m_id              = -1;
        journal_d.m_sched_id        = sched_n->m_id;
        journal_d.m_repeat_id       = 1;
        journal_d.m_date_time       = sched_n->m_date_time;
        journal_d.m_type            = sched_n->m_type;
        journal_d.m_status          = sched_n->m_status;
        journal_d.m_account_id      = sched_n->m_account_id;
        journal_d.m_to_account_id_n = sched_n->m_to_account_id_n;
        journal_d.m_payee_id_n      = sched_n->m_payee_id_n;
        journal_d.m_category_id_n   = sched_n->m_category_id_n;
        journal_d.m_amount          = sched_n->m_amount;
        journal_d.m_to_amount       = sched_n->m_to_amount;
        journal_d.m_number          = sched_n->m_number;
        journal_d.m_notes           = sched_n->m_notes;
        journal_d.m_followup_id     = sched_n->m_followup_id;
        journal_d.m_color           = sched_n->m_color;
        journal_d.m_updated_time_n  = mmDateTimeN();
        journal_d.m_deleted_time_n  = mmDateTimeN();
    }
    return true;
}

const TrxSplitModel::DataA Journal::split(Journal::Data& journal_d)
{
    return journal_d.m_repeat_id < 0
        ? TrxModel::instance().find_id_tp_a(journal_d.m_id)
        : Journal::execute_splits(SchedModel::instance().find_id_qp_a(journal_d.m_sched_id));
}

Journal::Data Journal::get_id_data(JournalKey journal_key)
{
    return journal_key.is_realized()
        ? Journal::Data(*TrxModel::instance().get_id_data_n(journal_key.rid()))
        : Journal::Data(*SchedModel::instance().get_id_data_n(journal_key.sid()));
}

Journal::DataExt Journal::get_id_data_x(JournalKey journal_key)
{
    return journal_key.is_realized()
        ? Journal::DataExt(*TrxModel::instance().get_id_data_n(journal_key.rid()))
        : Journal::DataExt(*SchedModel::instance().get_id_data_n(journal_key.sid()));
}

