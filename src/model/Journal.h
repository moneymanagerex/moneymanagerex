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

#include "TrxModel.h"
#include "SchedModel.h"
#include "TrxSplitModel.h"
#include "SchedSplitModel.h"
#include "TagLinkModel.h"

struct JournalKey
{
    int   m_repeat_id; // un-initialized: -1 | realized: -1            | scheduled: > 0
    int64 m_ref_id;    // un-initialized: -1 | realized: > 0 (TRANSID) | scheduled: > 0 (BDID)

    JournalKey() : m_repeat_id(-1), m_ref_id(-1) {}
    JournalKey(int repeat_id, int64 ref_id) :m_repeat_id(repeat_id), m_ref_id(ref_id) {}

    bool is_initialized() const { return m_ref_id > 0; }
    bool is_realized() const { return m_repeat_id < 0 && m_ref_id > 0; }
    bool is_scheduled() const { return m_repeat_id > 0 && m_ref_id > 0; }
    auto ref_type() const -> RefTypeN {
        return is_scheduled() ? SchedModel::s_ref_type : TrxModel::s_ref_type;
    }
    auto ref_id() const -> int64 { return m_ref_id; }
    auto rid() const -> int64 { return m_repeat_id < 0 ? m_ref_id : -1; }
    auto sid() const -> int64 { return m_repeat_id > 0 ? m_ref_id : -1; }

    bool operator== (const JournalKey& other) const {
        return m_repeat_id == other.m_repeat_id && m_ref_id == other.m_ref_id;
    }
    bool operator< (const JournalKey& other) const {
        return m_repeat_id < other.m_repeat_id || (
            m_repeat_id == other.m_repeat_id && m_ref_id < other.m_ref_id
        );
    }
};

class Journal
{
public:
    struct Data: public TrxData
    {
        int64 m_sched_id;
        int m_repeat_id;

        Data();
        explicit Data(const TrxData& t);
        explicit Data(const SchedData& sched_d);
        Data(const SchedData& sched_d, mmDateTime date, int repeat_i);
        ~Data();

        JournalKey key() const {
            return JournalKey{m_repeat_id, m_repeat_id > 0 ? m_sched_id : m_id};
        }
    };
    typedef std::vector<Data> DataA;

    struct DataExt: public TrxModel::DataExt
    {
        int64 m_sched_id;
        int m_repeat_id;

        explicit DataExt(const TrxData& t);
        DataExt(
            const TrxData& t,
            const std::map<int64, TrxSplitModel::DataA>& trxId_tpA_m,
            const std::map<int64, TagLinkModel::DataA>& trxId_glA_m
        );
        DataExt(const SchedData& sched_d);
        DataExt(const SchedData& sched_d, mmDateTime date, int repeat_i);
        DataExt(const SchedData& sched_d, mmDateTime date, int repeat_i,
            const std::map<int64, SchedSplitModel::DataA>& schedId_qpA_m,
            const std::map<int64, TagLinkModel::DataA>& schedId_glA_m
        );
        ~DataExt();

        JournalKey key() const {
            return JournalKey{m_repeat_id, m_repeat_id > 0 ? m_sched_id : m_id};
        }
    };
    typedef std::vector<DataExt> DataExtA;

    static auto execute_bill(const SchedData& sched_d, mmDateTime date_time) -> TrxData;
    static auto execute_bill_full(const SchedData& sched_d, mmDateTime date_time) -> TrxModel::DataExt;
    static auto execute_splits(const SchedSplitModel::DataA& rs) -> TrxSplitModel::DataA;

    static void setEmptyData(Data& journal_d, int64 account_id);
    static bool setJournalData(Data& journal_d, JournalKey journal_key);
    static auto split(Data& journal_d) -> const TrxSplitModel::DataA;
    static auto get_id_data(JournalKey journal_key) -> Journal::Data;
    static auto get_id_data_x(JournalKey journal_key) -> Journal::DataExt;

    struct SorterByJOURNALID
    { 
        bool operator()(const DataExt& x, const DataExt& y)
        {
            JournalKey x_key = x.key();
            JournalKey y_key = y.key();
            return (x_key.is_realized() && !y_key.is_realized()) || (
                x_key.is_realized() == y_key.is_realized() && (
                    x_key.m_ref_id < y_key.m_ref_id || (
                        x_key.m_ref_id == y_key.m_ref_id &&
                        x_key.m_repeat_id < y_key.m_repeat_id
                    )
                )
            );
        }
    };

    struct SorterByJOURNALSN
    { 
        bool operator()(const DataExt& x, const DataExt& y)
        {
            return x.SN < y.SN;
        }
    };
};
