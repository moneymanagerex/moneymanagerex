/*******************************************************
 Copyright (C) 2013,2014 Guan Lisheng (guanlisheng@gmail.com)

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
#include "util/mmChoice.h"
#include "util/mmDate.h"

#include "table/TrxTable.h"
#include "data/TrxData.h"

#include "_ModelBase.h"
#include "TrxSplitModel.h"
#include "FieldModel.h"
#include "TagLinkModel.h"
// cannot include "util/util.h"

class TrxModel : public TableFactory<TrxTable, TrxData>
{
public:
    using SplitDataA = TrxSplitModel::DataA;

public:
    struct DataExt: public Data
    {
        // filled-in by constructor
        wxString displayID;
        wxString ACCOUNTNAME, TOACCOUNTNAME;
        wxString PAYEENAME;
        wxString CATEGNAME;
        TrxSplitModel::DataA m_tp_a;
        TagLinkModel::DataA m_gl_a;
        wxString TAGNAMES;

        // filled-in by constructor; overwritten by JournalPanel::filterList()
        int64 m_account_w_id_n, m_account_d_id_n;
        double m_amount_w, m_amount_d;

        // filled-in by JournalPanel::filterList()
        long SN;
        wxString displaySN;
        double m_account_flow;
        double m_account_balance;
        wxArrayString ATTACHMENT_DESCRIPTION;
        FieldTypeN UDFC_type[5] = {
            FieldTypeN(),
            FieldTypeN(),
            FieldTypeN(),
            FieldTypeN(),
            FieldTypeN()
        };
        wxString UDFC_content[5];
        double UDFC_value[5] = {0, 0, 0, 0, 0};

        DataExt();
        explicit DataExt(const Data& trx_d);
        DataExt(const Data& trx_d,
            const std::map<int64, TrxSplitModel::DataA>& trxId_tpA_m,
            const std::map<int64, TagLinkModel::DataA>& trxId_glA_m
        );
        ~DataExt();

        void fill_data();
        wxString real_payee_name(int64 account_id) const;
        const wxString get_currency_code(int64 account_id) const;
        const wxString get_account_name(int64 account_id) const;
        bool has_split() const;
        bool has_tags() const;
        bool has_attachment() const;
        bool is_foreign() const;
        bool is_foreign_transfer() const;
        wxString info() const;
        const wxString to_json();
    };
    typedef std::vector<DataExt> DataExtA;

public:
    static const RefTypeN s_ref_type;

public:
    TrxModel();
    ~TrxModel();

public:
    static TrxModel& instance(wxSQLite3Database* db);
    static TrxModel& instance();

public:
    static auto DATE(OP op, const mmDate& date) -> TrxCol::TRANSDATE;
    static auto TYPE(OP op, TrxType trx_type) -> TrxCol::TRANSCODE;
    static auto STATUS(OP op, TrxStatus trx_status) -> TrxCol::STATUS;
    static auto IS_VOID(bool value) -> TrxCol::STATUS;
    static auto IS_DELETED(bool value) -> TrxCol::DELETEDTIME;

public:
    // TODO: move to TrxData
    static void copy_from_trx(Data* this_n, const Data& other_d);
    static bool is_foreign(const Data& this_d);
    static bool is_foreignAsTransfer(const Data& this_d);

public:
    // override
    bool purge_id(int64 id) override;

    void save_timestamp(int64 id);
    void update_timestamp(Data& trx_d);
    auto unsafe_save_trx_n(Data* trx_n) -> const Data*;
    auto save_trx_n(Data& trx_d) -> const Data*;
    bool save_trx_a(DataA& rows);

    auto find_id_tp_a(int64 trx_id) -> const TrxSplitModel::DataA;
    auto find_id_gl_a(int64 trx_id) -> const TagLinkModel::DataA;
    auto find_all_aDateTimeId() -> const TrxModel::DataA;

    void getFrequentUsedNotes(std::vector<wxString> &frequentNotes, int64 accountID = -1);
    void setEmptyData(Data& trx_d, int64 account_id);
    bool is_locked(const Data& trx_d);

public:
    struct SorterByACCOUNTNAME
    {
        bool operator()(const DataExt& x, const DataExt& y)
        {
            return std::wcscoll(x.ACCOUNTNAME.Lower().wc_str(), y.ACCOUNTNAME.Lower().wc_str()) < 0;
        }
    };

    struct SorterByTOACCOUNTNAME
    {
        bool operator()(const DataExt& x, const DataExt& y)
        {
            return std::wcscoll(x.TOACCOUNTNAME.Lower().wc_str(), y.TOACCOUNTNAME.Lower().wc_str()) < 0;
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

    struct SorterByTAGNAMES
    {
        bool operator()(const DataExt& x, const DataExt& y)
        {
            return x.TAGNAMES < y.TAGNAMES;
        }
    };

    struct SorterByDEPOSIT
    {
        bool operator()(const DataExt& x, const DataExt& y)
        {
            return x.m_account_d_id_n != -1 && (
                y.m_account_d_id_n == -1 || x.m_amount_d < y.m_amount_d
            );
        }
    };

    struct SorterByWITHDRAWAL
    {
        bool operator()(const DataExt& x, const DataExt& y)
        {
            return x.m_account_w_id_n != -1 && (
                y.m_account_w_id_n == -1 || x.m_amount_w < y.m_amount_w
            );
        }
    };

    struct SorterByBALANCE
    {
        bool operator()(const DataExt& x, const DataExt& y)
        {
            return x.m_account_balance < y.m_account_balance;
        }
    };
};

//----------------------------------------------------------------------------

inline bool TrxModel::DataExt::has_split() const
{
    return !this->m_tp_a.empty();
}

inline bool TrxModel::DataExt::has_tags() const

{
    return !this->m_gl_a.empty();
}

inline bool TrxModel::DataExt::has_attachment() const
{
    return !ATTACHMENT_DESCRIPTION.empty();
}

