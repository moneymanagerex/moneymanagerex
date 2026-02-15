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

TransactionModel::Data Journal::execute_bill(const ScheduledModel::Data& r, wxString date)
{
    TransactionModel::Data t;
    t.TRANSID           = 0;
    t.ACCOUNTID         = r.ACCOUNTID;
    t.TOACCOUNTID       = r.TOACCOUNTID;
    t.PAYEEID           = r.PAYEEID;
    t.TRANSCODE         = r.TRANSCODE;
    t.TRANSAMOUNT       = r.TRANSAMOUNT;
    t.STATUS            = r.STATUS;
    t.TRANSACTIONNUMBER = r.TRANSACTIONNUMBER;
    t.NOTES             = r.NOTES;
    t.CATEGID           = r.CATEGID;
    t.TRANSDATE         = date;
    t.FOLLOWUPID        = r.FOLLOWUPID;
    t.TOTRANSAMOUNT     = r.TOTRANSAMOUNT;
    t.COLOR             = r.COLOR;
    return t;
}

TransactionModel::Full_Data Journal::execute_bill_full(const ScheduledModel::Data& r, wxString date)
{
    TransactionModel::Full_Data t;
    t.TRANSID           = 0;
    t.ACCOUNTID         = r.ACCOUNTID;
    t.TOACCOUNTID       = r.TOACCOUNTID;
    t.PAYEEID           = r.PAYEEID;
    t.TRANSCODE         = r.TRANSCODE;
    t.TRANSAMOUNT       = r.TRANSAMOUNT;
    t.STATUS            = r.STATUS;
    t.TRANSACTIONNUMBER = r.TRANSACTIONNUMBER;
    t.NOTES             = r.NOTES;
    t.CATEGID           = r.CATEGID;
    t.TRANSDATE         = date;
    t.FOLLOWUPID        = r.FOLLOWUPID;
    t.TOTRANSAMOUNT     = r.TOTRANSAMOUNT;
    t.COLOR             = r.COLOR;
    return t;
}

TransactionSplitModel::Data_Set Journal::execute_splits(const Budgetsplit_Data_Set& rs)
{
    TransactionSplitModel::Data_Set ts;
    for (auto &rs1 : rs)
    {
        TransactionSplitModel::Data ts1;
        ts1.SPLITTRANSID     = rs1.SPLITTRANSID;
        ts1.TRANSID          = 0;
        ts1.CATEGID          = rs1.CATEGID;
        ts1.SPLITTRANSAMOUNT = rs1.SPLITTRANSAMOUNT;
        ts1.NOTES            = rs1.NOTES;
        ts.push_back(ts1);
    }
    return ts;
}

Journal::Data::Data()
    : TransactionModel::Data(), m_bdid(0), m_repeat_num(0)
{
}

Journal::Data::Data(const TransactionModel::Data& t)
    : TransactionModel::Data(t), m_bdid(0), m_repeat_num(0)
{
}

Journal::Data::Data(const ScheduledModel::Data& r)
    : Data(r, r.TRANSDATE, 1)
{
}

Journal::Data::Data(const ScheduledModel::Data& r, wxString date, int repeat_num)
    : TransactionModel::Data(execute_bill(r, date)), m_bdid(r.BDID), m_repeat_num(repeat_num)
{
    if (m_repeat_num < 1) {
        wxFAIL;
    }
}

Journal::Data::~Data()
{
}

Journal::Full_Data::Full_Data(const TransactionModel::Data& t)
    : TransactionModel::Full_Data(t), m_bdid(0), m_repeat_num(0)
{
}

Journal::Full_Data::Full_Data(const TransactionModel::Data& t,
    const std::map<int64 /* TRANSID */, Split_Data_Set>& splits,
    const std::map<int64 /* TRANSID */, Taglink_Data_Set>& tags)
:
    TransactionModel::Full_Data(t, splits, tags), m_bdid(0), m_repeat_num(0)
{
}

Journal::Full_Data::Full_Data(const ScheduledModel::Data& r)
    : Full_Data(r, r.TRANSDATE, 1)
{
}

Journal::Full_Data::Full_Data(const ScheduledModel::Data& r,
    wxString date, int repeat_num)
:
    TransactionModel::Full_Data(execute_bill_full(r, date), {}, {}),
    m_bdid(r.BDID), m_repeat_num(repeat_num)
{
    if (m_repeat_num < 1) {
        wxFAIL;
    }

    m_splits = execute_splits(ScheduledModel::split(r));

    m_tags = ScheduledModel::taglink(r);

    TransactionModel::Full_Data::fill_data();
    displayID = wxString("");
}

Journal::Full_Data::Full_Data(const ScheduledModel::Data& r,
    wxString date, int repeat_num,
    const std::map<int64 /* BDID */, Budgetsplit_Data_Set>& budgetsplits,
    const std::map<int64 /* BDID */, Taglink_Data_Set>& tags)
:
    TransactionModel::Full_Data(execute_bill_full(r, date), {}, {}),
    m_bdid(r.BDID), m_repeat_num(repeat_num)
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

    TransactionModel::Full_Data::fill_data();
    displayID = wxString("");
}

Journal::Full_Data::~Full_Data()
{
}


void Journal::getEmptyData(Journal::Data &data, int64 accountID)
{
    TransactionModel::getEmptyData(data, accountID);
    data.m_bdid = 0;
    data.m_repeat_num = 0;
}

bool Journal::getJournalData(Journal::Data &data, Journal::IdB journal_id)
{
    if (!journal_id.second) {
        TransactionModel::Data *tran = TransactionModel::instance().cache_id(journal_id.first);
        if (!tran)
            return false;
        data.m_repeat_num = 0;
        data.m_bdid = 0;
        data.TRANSID           = tran->TRANSID;
        data.ACCOUNTID         = tran->ACCOUNTID;
        data.TOACCOUNTID       = tran->TOACCOUNTID;
        data.PAYEEID           = tran->PAYEEID;
        data.TRANSCODE         = tran->TRANSCODE;
        data.TRANSAMOUNT       = tran->TRANSAMOUNT;
        data.STATUS            = tran->STATUS;
        data.TRANSACTIONNUMBER = tran->TRANSACTIONNUMBER;
        data.NOTES             = tran->NOTES;
        data.CATEGID           = tran->CATEGID;
        data.TRANSDATE         = tran->TRANSDATE;
        data.LASTUPDATEDTIME   = tran->LASTUPDATEDTIME;
        data.DELETEDTIME       = tran->DELETEDTIME;
        data.FOLLOWUPID        = tran->FOLLOWUPID;
        data.TOTRANSAMOUNT     = tran->TOTRANSAMOUNT;
        data.COLOR             = tran->COLOR;
    }
    else {
        ScheduledModel::Data *bill = ScheduledModel::instance().cache_id(journal_id.first);
        if (!bill)
            return false;
        data.m_repeat_num = 1;
        data.TRANSID = 0;
        data.m_bdid            = bill->BDID;
        data.ACCOUNTID         = bill->ACCOUNTID;
        data.TOACCOUNTID       = bill->TOACCOUNTID;
        data.PAYEEID           = bill->PAYEEID;
        data.TRANSCODE         = bill->TRANSCODE;
        data.TRANSAMOUNT       = bill->TRANSAMOUNT;
        data.STATUS            = bill->STATUS;
        data.TRANSACTIONNUMBER = bill->TRANSACTIONNUMBER;
        data.NOTES             = bill->NOTES;
        data.CATEGID           = bill->CATEGID;
        data.TRANSDATE         = bill->TRANSDATE;
        data.LASTUPDATEDTIME   = "";
        data.DELETEDTIME       = "";
        data.FOLLOWUPID        = bill->FOLLOWUPID;
        data.TOTRANSAMOUNT     = bill->TOTRANSAMOUNT;
        data.COLOR             = bill->COLOR;
    }
    return true;
}

const TransactionSplitModel::Data_Set Journal::split(Journal::Data &r)
{
    return (r.m_repeat_num == 0) ?
        TransactionSplitModel::instance().find(
            TransactionSplitModel::TRANSID(r.TRANSID)) :
        Journal::execute_splits(ScheduledSplitModel::instance().find(
            ScheduledSplitModel::TRANSID(r.m_bdid)));
}
