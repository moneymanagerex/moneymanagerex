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

TrxData Journal::execute_bill(const SchedData& sched_d, wxString date)
{
    TrxData trx_d;
    trx_d.m_id              = 0;
    trx_d.m_account_id      = sched_d.m_account_id;
    trx_d.m_to_account_id_n = sched_d.m_to_account_id_n;
    trx_d.m_payee_id_n      = sched_d.m_payee_id_n;
    trx_d.TRANSCODE         = sched_d.TRANSCODE;
    trx_d.m_amount          = sched_d.m_amount;
    trx_d.STATUS            = sched_d.STATUS;
    trx_d.m_number          = sched_d.m_number;
    trx_d.m_notes           = sched_d.m_notes;
    trx_d.m_category_id_n   = sched_d.m_category_id_n;
    trx_d.TRANSDATE         = date;
    trx_d.m_followup_id     = sched_d.m_followup_id;
    trx_d.m_to_amount       = sched_d.m_to_amount;
    trx_d.m_color           = sched_d.m_color;
    return trx_d;
}

TrxModel::Full_Data Journal::execute_bill_full(const SchedData& sched_d, wxString date)
{
    TrxModel::Full_Data trx_xd;
    trx_xd.m_id              = 0;
    trx_xd.m_account_id      = sched_d.m_account_id;
    trx_xd.m_to_account_id_n = sched_d.m_to_account_id_n;
    trx_xd.m_payee_id_n      = sched_d.m_payee_id_n;
    trx_xd.TRANSCODE         = sched_d.TRANSCODE;
    trx_xd.m_amount          = sched_d.m_amount;
    trx_xd.STATUS            = sched_d.STATUS;
    trx_xd.m_number          = sched_d.m_number;
    trx_xd.m_notes           = sched_d.m_notes;
    trx_xd.m_category_id_n   = sched_d.m_category_id_n;
    trx_xd.TRANSDATE         = date;
    trx_xd.m_followup_id     = sched_d.m_followup_id;
    trx_xd.m_to_amount       = sched_d.m_to_amount;
    trx_xd.m_color           = sched_d.m_color;
    return trx_xd;
}

TrxSplitModel::DataA Journal::execute_splits(const SchedSplitDataA& qp_a)
{
    TrxSplitModel::DataA tp_a;
    for (auto& qp_d : qp_a) {
        TrxSplitData tp_d;
        // FIXME: tp_d.m_id is invalid
        tp_d.m_id          = qp_d.m_id;
        tp_d.m_trx_id      = 0;
        tp_d.m_category_id = qp_d.m_category_id;
        tp_d.m_amount      = qp_d.m_amount;
        tp_d.m_notes       = qp_d.m_notes;
        tp_a.push_back(tp_d);
    }
    return tp_a;
}

Journal::Data::Data()
    : TrxData(), m_bdid(0), m_repeat_num(0)
{
}

Journal::Data::Data(const TrxData& trx_d)
    : TrxData(trx_d), m_bdid(0), m_repeat_num(0)
{
}

Journal::Data::Data(const SchedData& sched_d)
    : Data(sched_d, sched_d.TRANSDATE, 1)
{
}

Journal::Data::Data(const SchedData& sched_d, wxString date, int repeat_num)
    : TrxData(execute_bill(sched_d, date)), m_bdid(sched_d.m_id), m_repeat_num(repeat_num)
{
    if (m_repeat_num < 1) {
        wxFAIL;
    }
}

Journal::Data::~Data()
{
}

Journal::Full_Data::Full_Data(const TrxData& trx_d)
    : TrxModel::Full_Data(trx_d), m_bdid(0), m_repeat_num(0)
{
}

Journal::Full_Data::Full_Data(const TrxData& trx_d,
    const std::map<int64 /* m_id */, TrxSplitDataA>& splits,
    const std::map<int64 /* m_id */, TagLinkDataA>& tags)
:
    TrxModel::Full_Data(trx_d, splits, tags), m_bdid(0), m_repeat_num(0)
{
}

Journal::Full_Data::Full_Data(const SchedData& sched_d)
    : Full_Data(sched_d, sched_d.TRANSDATE, 1)
{
}

Journal::Full_Data::Full_Data(const SchedData& sched_d,
    wxString date, int repeat_num)
:
    TrxModel::Full_Data(execute_bill_full(sched_d, date), {}, {}),
    m_bdid(sched_d.m_id), m_repeat_num(repeat_num)
{
    if (m_repeat_num < 1) {
        wxFAIL;
    }

    m_splits = execute_splits(SchedModel::split(sched_d));

    m_tags = SchedModel::taglink(sched_d);

    TrxModel::Full_Data::fill_data();
    displayID = wxString("");
}

Journal::Full_Data::Full_Data(const SchedData& sched_d,
    wxString date, int repeat_num,
    const std::map<int64 /* m_id */, SchedSplitDataA>& budgetsplits,
    const std::map<int64 /* m_id */, TagLinkDataA>& tags)
:
    TrxModel::Full_Data(execute_bill_full(sched_d, date), {}, {}),
    m_bdid(sched_d.m_id), m_repeat_num(repeat_num)
{
    if (m_repeat_num < 1) {
        wxFAIL;
    }

    const auto budgetsplits_it = budgetsplits.find(m_bdid);
    if (budgetsplits_it != budgetsplits.end()) {
        m_splits = execute_splits(budgetsplits_it->second);
    }

    const auto tag_it = tags.find(m_bdid);
    if (tag_it != tags.end()) m_tags = tag_it->second;

    TrxModel::Full_Data::fill_data();
    displayID = wxString("");
}

Journal::Full_Data::~Full_Data()
{
}


void Journal::setEmptyData(Journal::Data &data, int64 accountID)
{
    TrxModel::setEmptyData(data, accountID);
    data.m_bdid = 0;
    data.m_repeat_num = 0;
}

bool Journal::setJournalData(Journal::Data& journal_d, Journal::IdB journal_id)
{
    if (!journal_id.second) {
        const TrxData *trx_n = TrxModel::instance().get_id_data_n(journal_id.first);
        if (!trx_n)
            return false;
        journal_d.m_repeat_num      = 0;
        journal_d.m_bdid            = 0;
        journal_d.m_id              = trx_n->m_id;
        journal_d.m_account_id      = trx_n->m_account_id;
        journal_d.m_to_account_id_n = trx_n->m_to_account_id_n;
        journal_d.m_payee_id_n      = trx_n->m_payee_id_n;
        journal_d.TRANSCODE         = trx_n->TRANSCODE;
        journal_d.m_amount          = trx_n->m_amount;
        journal_d.STATUS            = trx_n->STATUS;
        journal_d.m_number          = trx_n->m_number;
        journal_d.m_notes           = trx_n->m_notes;
        journal_d.m_category_id_n   = trx_n->m_category_id_n;
        journal_d.TRANSDATE         = trx_n->TRANSDATE;
        journal_d.LASTUPDATEDTIME   = trx_n->LASTUPDATEDTIME;
        journal_d.DELETEDTIME       = trx_n->DELETEDTIME;
        journal_d.m_followup_id     = trx_n->m_followup_id;
        journal_d.m_to_amount       = trx_n->m_to_amount;
        journal_d.m_color           = trx_n->m_color;
    }
    else {
        const SchedData *sched_n = SchedModel::instance().get_id_data_n(journal_id.first);
        if (!sched_n)
            return false;
        journal_d.m_repeat_num = 1;
        journal_d.m_id              = 0;
        journal_d.m_bdid            = sched_n->m_id;
        journal_d.m_account_id      = sched_n->m_account_id;
        journal_d.m_to_account_id_n = sched_n->m_to_account_id_n;
        journal_d.m_payee_id_n      = sched_n->m_payee_id_n;
        journal_d.TRANSCODE         = sched_n->TRANSCODE;
        journal_d.m_amount          = sched_n->m_amount;
        journal_d.STATUS            = sched_n->STATUS;
        journal_d.m_number          = sched_n->m_number;
        journal_d.m_notes           = sched_n->m_notes;
        journal_d.m_category_id_n   = sched_n->m_category_id_n;
        journal_d.TRANSDATE         = sched_n->TRANSDATE;
        journal_d.LASTUPDATEDTIME   = "";
        journal_d.DELETEDTIME       = "";
        journal_d.m_followup_id     = sched_n->m_followup_id;
        journal_d.m_to_amount       = sched_n->m_to_amount;
        journal_d.m_color           = sched_n->m_color;
    }
    return true;
}

const TrxSplitModel::DataA Journal::split(Journal::Data& journal_d)
{
    return (journal_d.m_repeat_num == 0)
        ? TrxSplitModel::instance().find(
            TrxSplitCol::TRANSID(journal_d.m_id)
        ) : Journal::execute_splits(SchedSplitModel::instance().find(
            SchedSplitCol::TRANSID(journal_d.m_bdid)
        ));
}
