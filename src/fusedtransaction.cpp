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

#include "Model.h"
#include "Model_Checking.h"
#include "Model_Billsdeposits.h"
#include "Model_Splittransaction.h"
#include "Model_Budgetsplittransaction.h"
#include "Model_Taglink.h"
#include "fusedtransaction.h"

Model_Checking::Data Fused_Transaction::execute_bill(const Model_Billsdeposits::Data& r, wxString date)
{
    Model_Checking::Data t;
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

Model_Checking::Full_Data Fused_Transaction::execute_bill_full(const Model_Billsdeposits::Data& r, wxString date)
{
    Model_Checking::Full_Data t;
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

Model_Splittransaction::Data_Set Fused_Transaction::execute_splits(const Budgetsplit_Data_Set& rs)
{
    Model_Splittransaction::Data_Set ts;
    for (auto &rs1 : rs)
    {
        Model_Splittransaction::Data ts1;
        ts1.SPLITTRANSID     = rs1.SPLITTRANSID;
        ts1.TRANSID          = 0;
        ts1.CATEGID          = rs1.CATEGID;
        ts1.SPLITTRANSAMOUNT = rs1.SPLITTRANSAMOUNT;
        ts1.NOTES            = rs1.NOTES;
        ts.push_back(ts1);
    }
    return ts;
}

Fused_Transaction::Data::Data(const Model_Checking::Data& t)
    : Model_Checking::Data(t), m_bdid(0), m_repeat_num(0)
{
}

Fused_Transaction::Data::Data(const Model_Billsdeposits::Data& r)
    : Data(r, r.TRANSDATE, 1)
{
}

Fused_Transaction::Data::Data(const Model_Billsdeposits::Data& r, wxString date, int repeat_num)
    : Model_Checking::Data(execute_bill(r, date)), m_bdid(r.BDID), m_repeat_num(repeat_num)
{
    if (m_repeat_num < 1)
        wxFAIL;
}

Fused_Transaction::Data::~Data()
{
}

Fused_Transaction::Full_Data::Full_Data(const Model_Checking::Data& t)
    : Model_Checking::Full_Data(t), m_bdid(0), m_repeat_num(0)
{
}

Fused_Transaction::Full_Data::Full_Data(const Model_Checking::Data& t,
    const std::map<int /* TRANSID */, Split_Data_Set>& splits,
    const std::map<int /* TRANSID */, Taglink_Data_Set>& tags)
:
    Model_Checking::Full_Data(t, splits, tags), m_bdid(0), m_repeat_num(0)
{
}

Fused_Transaction::Full_Data::Full_Data(const Model_Billsdeposits::Data& r)
    : Full_Data(r, r.TRANSDATE, 1)
{
}

Fused_Transaction::Full_Data::Full_Data(const Model_Billsdeposits::Data& r,
    wxString date, int repeat_num)
:
    Model_Checking::Full_Data(execute_bill_full(r, date)),
    m_bdid(r.BDID), m_repeat_num(repeat_num)
{
    if (m_repeat_num < 1)
        wxFAIL;

    m_tags = Model_Billsdeposits::taglink(r);

    m_splits = execute_splits(Model_Billsdeposits::split(r));

    Model_Checking::Full_Data::fill_data();
    displayID = wxString("");
}

Fused_Transaction::Full_Data::Full_Data(const Model_Billsdeposits::Data& r,
    wxString date, int repeat_num,
    const std::map<int /* BDID */, Budgetsplit_Data_Set>& budgetsplits,
    const std::map<int /* BDID */, Taglink_Data_Set>& tags)
:
    Model_Checking::Full_Data(execute_bill_full(r, date)),
    m_bdid(r.BDID), m_repeat_num(repeat_num)
{
    if (m_repeat_num < 1)
        wxFAIL;

    const auto budgetsplits_it = budgetsplits.find(m_bdid);
    if (budgetsplits_it != budgetsplits.end()) {
        m_splits = execute_splits(budgetsplits_it->second);
    }

    const auto tag_it = tags.find(m_bdid);
    if (tag_it != tags.end()) m_tags = tag_it->second;

    Model_Checking::Full_Data::fill_data();
    displayID = wxString("");
}

Fused_Transaction::Full_Data::~Full_Data()
{
}
