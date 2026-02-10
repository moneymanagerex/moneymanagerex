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

#pragma once

#include "model/_Model.h"
#include "model/TransactionModel.h"
#include "model/ScheduledModel.h"
#include "model/TransactionSplitModel.h"
#include "model/ScheduledSplitModel.h"
#include "model/TagLinkModel.h"

class Journal
{
public:
    // id represents TRANSID if !is_bill, or BDID otherwise
    typedef std::pair<int64 /* id */, bool /* is_bill */> IdB;

    // id represents TRANSID if repeat_num == 0, or BDID otherwise
    typedef std::pair<int64 /* id */, int /* repeat_num */> IdRepeat;

    typedef TransactionSplitModel::Data_Set Split_Data_Set;
    typedef ScheduledSplitModel::Data_Set Budgetsplit_Data_Set;
    typedef TagLinkModel::Data_Set Taglink_Data_Set;

    static TransactionModel::Data execute_bill(const ScheduledModel::Data& r, wxString date);
    static TransactionModel::Full_Data execute_bill_full(const ScheduledModel::Data& r, wxString date);
    static Split_Data_Set execute_splits(const Budgetsplit_Data_Set& rs);

    struct Data: public TransactionModel::Data
    {
        Data();
        explicit Data(const TransactionModel::Data& t);
        explicit Data(const ScheduledModel::Data& r);
        Data(const ScheduledModel::Data& r, wxString date, int repeat_num);
        ~Data();

        int64 m_bdid;
        int m_repeat_num;
    };
    typedef std::vector<Data> Data_Set;

    struct Full_Data: public TransactionModel::Full_Data
    {
        explicit Full_Data(const TransactionModel::Data& t);
        Full_Data(const TransactionModel::Data& t,
            const std::map<int64 /* TRANSID */, Split_Data_Set>& splits,
            const std::map<int64 /* TRANSID */, Taglink_Data_Set>& tags
        );
        Full_Data(const ScheduledModel::Data& r);
        Full_Data(const ScheduledModel::Data& r, wxString date, int repeat_num);
        Full_Data(const ScheduledModel::Data& r, wxString date, int repeat_num,
            const std::map<int64 /* BDID */, Budgetsplit_Data_Set>& budgetsplits,
            const std::map<int64 /* BDID */, Taglink_Data_Set>& tags
        );
        ~Full_Data();

        int64 m_bdid;
        int m_repeat_num;
    };
    typedef std::vector<Full_Data> Full_Data_Set;

    struct SorterByJOURNALID
    { 
        template<class DATA>
        bool operator()(const DATA& x, const DATA& y)
        {
            return (!x.m_repeat_num && y.m_repeat_num) ||
                (!x.m_repeat_num && !y.m_repeat_num && x.TRANSID < y.TRANSID) ||
                (x.m_repeat_num && y.m_repeat_num && x.m_bdid < y.m_bdid);
        }
    };

    struct SorterByJOURNALSN
    { 
        template<class DATA>
        bool operator()(const DATA& x, const DATA& y)
        {
            return (x.SN < y.SN);
        }
    };

    static void getEmptyData(Data &data, int64 account_id);
    static bool getJournalData(Data &data, IdB journal_id);
    static const TransactionSplitModel::Data_Set split(Data &r);
};

