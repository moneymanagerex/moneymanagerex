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
#include "util/mmSingleton.h"
#include "table/_TableFactory.h"
#include "data/SchedData.h"

#include "TrxModel.h"
#include "TrxSplitModel.h"
#include "SchedSplitModel.h"
#include "TagLinkModel.h"

class SchedModel : public TableFactory<SchedTable, SchedData>
{
public:
    using SplitDataA = SchedSplitModel::DataA;

public:
    struct DataExt : public Data
    {
        wxString ACCOUNTNAME;
        wxString PAYEENAME;
        wxString CATEGNAME;
        SchedSplitModel::DataA m_qp_a;
        TagLinkModel::DataA m_gl_a;
        wxString TAGNAMES;

        DataExt();
        explicit DataExt(const Data& sched_d);

        wxString real_payee_name() const;
    };
    typedef std::vector<DataExt> DataExtA;

public:
    static const RefTypeN s_ref_type;

public:
    SchedModel();
    ~SchedModel();

public:
    static SchedModel& instance(wxSQLite3Database* db);
    static SchedModel& instance();

public:
    static auto TYPE(OP op, TrxType sched_type) -> SchedCol::TRANSCODE;
    static auto STATUS(OP op, TrxStatus sched_status) -> SchedCol::STATUS;
    static auto IS_VOID(bool value) -> SchedCol::STATUS;

public:
    // override TableFactory
    virtual bool purge_id(int64 sched_id) override;

    auto find_id_qp_a(int64 sched_id) -> const SchedSplitModel::DataA;
    auto find_id_gl_a(int64 sched_id) -> const TagLinkModel::DataA;
    bool is_data_allowed(const Data& sched_d);
    void reschedule_id(int64 sched_id);

public:
    struct SorterByACCOUNTNAME
    {
        bool operator()(const DataExt& x, const DataExt& y)
        {
            return std::wcscoll(x.ACCOUNTNAME.Lower().wc_str(), y.ACCOUNTNAME.Lower().wc_str()) < 0;
        }
    };

    struct SorterByPAYEENAME
    {
        bool operator()(const DataExt& x, const DataExt& y)
        {
            return std::wcscoll(x.PAYEENAME.Lower().wc_str(), y.PAYEENAME.Lower().wc_str()) < 0;
        }
    };

    struct SorterByCATEGNAME
    {
        bool operator()(const DataExt& x, const DataExt& y)
        {
            return std::wcscoll(x.CATEGNAME.Lower().wc_str(), y.CATEGNAME.Lower().wc_str()) < 0;
        }
    };

    struct SorterByWITHDRAWAL
    {
        bool operator()(const DataExt& x, const DataExt& y)
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
        bool operator()(const DataExt& x, const DataExt& y)
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
