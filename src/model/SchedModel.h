/*******************************************************
 Copyright (C) 2013,2014 Guan Lisheng (guanlisheng@gmail.com)
 COPYRIGHT (C) 2022 Mark Whalley (mark@ipx.co.uk)

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

#include "base/defs.h"

#include "table/SchedTable.h"
#include "data/_DataEnum.h"
#include "data/SchedData.h"

#include "_ModelBase.h"
#include "TrxModel.h"
#include "TrxSplitModel.h"
#include "SchedSplitModel.h"
#include "TagLinkModel.h"

const int BD_REPEATS_MULTIPLEX_BASE = 100;

class SchedModel : public TableFactory<SchedTable, SchedData>
{
public:
    using TrxSplitDataA = SchedSplitModel::DataA;

    enum REPEAT_EXEC
    {
        REPEAT_EXEC_NONE   = 0,
        REPEAT_EXEC_MANUAL = 1,
        REPEAT_EXEC_SILENT = 2
    };
    enum REPEAT_FREQ
    {
        REPEAT_FREQ_INVALID = -1,   // this value is never stored in database
        REPEAT_FREQ_ONCE = 0,
        REPEAT_FREQ_WEEKLY,
        REPEAT_FREQ_BI_WEEKLY,      // FORTNIGHTLY
        REPEAT_FREQ_MONTHLY,
        REPEAT_FREQ_BI_MONTHLY,
        REPEAT_FREQ_QUARTERLY,      // TRI_MONTHLY
        REPEAT_FREQ_HALF_YEARLY,
        REPEAT_FREQ_YEARLY,
        REPEAT_FREQ_FOUR_MONTHLY,   // QUAD_MONTHLY
        REPEAT_FREQ_FOUR_WEEKLY,    // QUAD_WEEKLY
        REPEAT_FREQ_DAILY,
        REPEAT_FREQ_IN_X_DAYS,
        REPEAT_FREQ_IN_X_MONTHS,
        REPEAT_FREQ_EVERY_X_DAYS,
        REPEAT_FREQ_EVERY_X_MONTHS,
        REPEAT_FREQ_MONTHLY_LAST_DAY,
        REPEAT_FREQ_MONTHLY_LAST_BUSINESS_DAY,
        REPEAT_FREQ_size
    };
    enum REPEAT_NUM
    {
        REPEAT_NUM_INFINITY = -1,
        REPEAT_NUM_INVALID  = 0
    };
    enum REPEAT_X
    {
        REPEAT_X_VOID    = -1,
        REPEAT_X_INVALID = 0
    };
    // decoding of REPEATS and NUMOCCURRENCES
    struct RepeatNum
    {
        REPEAT_EXEC exec; // auto execution mode
        REPEAT_FREQ freq; // repetition frequency
        int num;          // occurrences if freq is WEEKLY .. DAILY, MONTHLY_LAST_*
        int x;            // x           if freq is IN_X_*, EVERY_X_*
        RepeatNum() = default;
    };

public:
    // Pre-initialised data structure
    struct Bill_Data : SchedData
    {
        std::vector<Split> local_splits;
        wxArrayInt64 TAGS;

        Bill_Data() {
            m_id              = 0;
            TRANSDATE         = wxDateTime::Now().FormatISOCombined();
            m_type            = TrxType(TrxType::e_withdrawal);
            m_status          = TrxStatus(TrxStatus::e_unreconciled);
            m_account_id      = -1;
            m_to_account_id_n = -1;
            m_payee_id_n      = -1;
            m_category_id_n   = -1;
            m_amount          = 0;
            m_to_amount       = 0;
            m_number          = "";
            m_notes           = "";
            m_followup_id     = -1;
            m_color           = -1;
            m_due_date        = mmDate::today();
            REPEATS           = 0;
            NUMOCCURRENCES    = 0;
        }
    };

    struct Full_Data : public Data
    {
        wxString ACCOUNTNAME;
        wxString PAYEENAME;
        wxString CATEGNAME;
        SchedSplitModel::DataA m_bill_splits;
        TagLinkModel::DataA m_tags;
        wxString TAGNAMES;

        Full_Data();
        explicit Full_Data(const Data& r);

        wxString real_payee_name() const;
    };
    typedef std::vector<Full_Data> Full_DataA;

public:
    static const RefTypeN s_ref_type;

public:
    // Initialize the global SchedModel table on initial call.
    // Resets the global table on subsequent calls.
    // Return the static instance address for SchedModel table
    // Note: Assigning the address to a local variable can destroy the instance.
    static SchedModel& instance(wxSQLite3Database* db);

    // Return the static instance address for SchedModel table
    // Note: Assigning the address to a local variable can destroy the instance.
    static SchedModel& instance();

public:
    // Data properties (do not require access to Model)
    // TODO: move to SchedData
    static wxDate getTransDateTime(const Data& this_d);
    static bool encode_repeat_num(Data& this_d, const RepeatNum& rn);
    static bool decode_repeat_num(const Data& this_d, RepeatNum& rn);
    static bool next_repeat_num(RepeatNum& rn);
    static bool requires_execution(const Data& this_d);
    static const wxDateTime nextOccurDate(
        wxDateTime this_date, const RepeatNum& rn, bool reverse = false
    );
    static wxArrayString unroll(const Data& sched_d, const wxString end_date, int limit = -1);

public:
    static SchedCol::TRANSCODE TRANSCODE(OP op, TrxType sched_type);
    static SchedCol::STATUS STATUS(OP op, TrxStatus sched_status);

public:
    static const SchedSplitModel::DataA split(const Data& sched_d);
    static const TagLinkModel::DataA taglink(const Data& sched_d);

public:
    SchedModel();
    ~SchedModel();

public:
    // Remove the Data record instance from memory and the database
    // including any splits associated with the Data Record.
    bool purge_id(int64 id) override;
    bool AllowTransaction(const Data& sched_d);
    void completeBDInSeries(int64 bdID);

public:
    struct SorterByACCOUNTNAME
    {
        bool operator()(const Full_Data& x, const Full_Data& y)
        {
            return std::wcscoll(x.ACCOUNTNAME.Lower().wc_str(), y.ACCOUNTNAME.Lower().wc_str()) < 0;
        }
    };

    struct SorterByPAYEENAME
    {
        bool operator()(const Full_Data& x, const Full_Data& y)
        {
            return std::wcscoll(x.PAYEENAME.Lower().wc_str(), y.PAYEENAME.Lower().wc_str()) < 0;
        }
    };

    struct SorterByCATEGNAME
    {
        bool operator()(const Full_Data& x, const Full_Data& y)
        {
            return std::wcscoll(x.CATEGNAME.Lower().wc_str(), y.CATEGNAME.Lower().wc_str()) < 0;
        }
    };

    struct SorterByWITHDRAWAL
    {
        bool operator()(const Full_Data& x, const Full_Data& y)
        {
            int64 x_accountid = -1, y_accountid = -1;
            double x_transamount = 0.0, y_transamount = 0.0;
            if (x.is_withdrawal()) {
                x_accountid = x.m_account_id; x_transamount = x.m_amount;
            }
            else if (x.is_transfer()) {
                x_accountid = x.m_account_id; x_transamount = x.m_amount;
            }
            if (y.is_withdrawal()) {
                y_accountid = y.m_account_id; y_transamount = y.m_amount;
            }
            else if (y.is_transfer()) {
                y_accountid = y.m_account_id; y_transamount = y.m_amount;
            }
            return x_accountid != -1 && (y_accountid == -1 || x_transamount < y_transamount);
        }
    };

    struct SorterByDEPOSIT
    {
        bool operator()(const Full_Data& x, const Full_Data& y)
        {
            int64 x_accountid = -1, y_accountid = -1;
            double x_transamount = 0.0, y_transamount = 0.0;
            if (x.is_deposit()) {
                x_accountid = x.m_account_id; x_transamount = x.m_amount;
            }
            else if (x.is_transfer()) {
                x_accountid = x.m_to_account_id_n; x_transamount = x.m_to_amount;
            }
            if (y.is_deposit()) {
                y_accountid = y.m_account_id; y_transamount = y.m_amount;
            }
            else if (y.is_transfer()) {
                y_accountid = y.m_to_account_id_n; y_transamount = y.m_to_amount;
            }
            return x_accountid != -1 && (y_accountid == -1 || x_transamount < y_transamount);
        }
    };
};
