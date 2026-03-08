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

    // id represents TRANSID if repeat_num == 0, or BDID otherwise
    typedef std::pair<int64 /* id */, int /* repeat_num */> IdRepeat;

    typedef TrxSplitModel::DataA TrxSplitDataA;
    typedef SchedSplitModel::DataA SchedSplitDataA;
    typedef TagLinkModel::DataA TagLinkDataA;

    static TrxData execute_bill(const SchedData& r, wxString date);
    static TrxModel::Full_Data execute_bill_full(const SchedData& r, wxString date);
    static TrxSplitDataA execute_splits(const SchedSplitDataA& rs);

    struct Data: public TrxData
    {
        int64 m_bdid;
        int m_repeat_num;

        Data();
        explicit Data(const TrxData& t);
        explicit Data(const SchedData& r);
        Data(const SchedData& r, wxString date, int repeat_num);
        ~Data();
    };
    typedef std::vector<Data> DataA;

    struct Full_Data: public TrxModel::Full_Data
    {
        int64 m_bdid;
        int m_repeat_num;

        explicit Full_Data(const TrxData& t);
        Full_Data(
            const TrxData& t,
            const std::map<int64 /* TRANSID */, TrxSplitDataA>& splits,
            const std::map<int64 /* TRANSID */, TagLinkDataA>& tags
        );
        Full_Data(const SchedData& r);
        Full_Data(const SchedData& r, wxString date, int repeat_num);
        Full_Data(const SchedData& r, wxString date, int repeat_num,
            const std::map<int64 /* BDID */, SchedSplitDataA>& budgetsplits,
            const std::map<int64 /* BDID */, TagLinkDataA>& tags
        );
        ~Full_Data();
    };
    typedef std::vector<Full_Data> Full_DataA;

    struct SorterByJOURNALID
    { 
        template<class DATA>
        bool operator()(const DATA& x, const DATA& y)
        {
            return (!x.m_repeat_num && y.m_repeat_num) ||
                (!x.m_repeat_num && !y.m_repeat_num && x.m_id < y.m_id) ||
                (x.m_repeat_num && y.m_repeat_num && x.m_bdid < y.m_bdid);
        }
    };

    struct SorterByJOURNALSN
    { 
        bool operator()(const Full_Data& x, const Full_Data& y)
        {
            return x.SN < y.SN;
        }
    };

    static void setEmptyData(Data &data, int64 account_id);
    static bool setJournalData(Data &data, IdB journal_id);
    static const TrxSplitModel::DataA split(Data &r);
};

