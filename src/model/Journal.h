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

#include "_ModelBase.h"
#include "TrxModel.h"
#include "SchedModel.h"
#include "TrxSplitModel.h"
#include "SchedSplitModel.h"
#include "TagLinkModel.h"

class Journal
{
public:
    // id represents TRANSID if !is_bill, or BDID otherwise
    typedef std::pair<int64 /* id */, bool /* is_bill */> IdB;

    // id represents TRANSID if repeat_i == 0, or BDID otherwise
    typedef std::pair<int64 /* id */, int /* repeat_i */> IdRepeat;

    typedef TrxSplitModel::DataA TrxSplitDataA;
    typedef SchedSplitModel::DataA SchedSplitDataA;
    typedef TagLinkModel::DataA TagLinkDataA;

    static TrxData execute_bill(const SchedData& r, wxString date);
    static TrxModel::DataExt execute_bill_full(const SchedData& r, wxString date);
    static TrxSplitDataA execute_splits(const SchedSplitDataA& rs);

    struct Data: public TrxData
    {
        int64 m_sched_id;
        int m_repeat_i;

        Data();
        explicit Data(const TrxData& t);
        explicit Data(const SchedData& r);
        Data(const SchedData& r, wxString date, int repeat_i);
        ~Data();
    };
    typedef std::vector<Data> DataA;

    struct DataExt: public TrxModel::DataExt
    {
        int64 m_sched_id;
        int m_repeat_i;

        explicit DataExt(const TrxData& t);
        DataExt(
            const TrxData& t,
            const std::map<int64 /* TRANSID */, TrxSplitDataA>& splits,
            const std::map<int64 /* TRANSID */, TagLinkDataA>& tags
        );
        DataExt(const SchedData& r);
        DataExt(const SchedData& r, wxString date, int repeat_i);
        DataExt(const SchedData& r, wxString date, int repeat_i,
            const std::map<int64 /* BDID */, SchedSplitDataA>& budgetsplits,
            const std::map<int64 /* BDID */, TagLinkDataA>& tags
        );
        ~DataExt();
    };
    typedef std::vector<DataExt> DataExtA;

    struct SorterByJOURNALID
    { 
        template<class DATA>
        bool operator()(const DATA& x, const DATA& y)
        {
            return (!x.m_repeat_i && y.m_repeat_i) ||
                (!x.m_repeat_i && !y.m_repeat_i && x.m_id < y.m_id) ||
                (x.m_repeat_i && y.m_repeat_i && x.m_sched_id < y.m_sched_id);
        }
    };

    struct SorterByJOURNALSN
    { 
        bool operator()(const DataExt& x, const DataExt& y)
        {
            return x.SN < y.SN;
        }
    };

    static void setEmptyData(Data &data, int64 account_id);
    static bool setJournalData(Data &data, IdB journal_id);
    static const TrxSplitModel::DataA split(Data &r);
};

