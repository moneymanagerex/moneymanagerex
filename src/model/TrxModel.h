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

const wxString mmGetTimeForDisplay(const wxString& datetime_iso);

class TrxModel : public TableFactory<TrxTable, TrxData>
{
public:
    using TrxSplitDataA = TrxSplitModel::DataA;
    using TagLinkDataA  = TagLinkModel::DataA;

public:
    struct Full_Data: public Data
    {
        // filled-in by constructor
        wxString displayID;
        wxString ACCOUNTNAME, TOACCOUNTNAME;
        wxString PAYEENAME;
        wxString CATEGNAME;
        TrxSplitDataA m_splits;
        TagLinkDataA m_tags;
        wxString TAGNAMES;

        // filled-in by constructor; overwritten by JournalPanel::filterTable()
        int64 ACCOUNTID_W, ACCOUNTID_D;
        double TRANSAMOUNT_W, TRANSAMOUNT_D;

        // filled-in by JournalPanel::filterTable()
        long SN;
        wxString displaySN;
        double ACCOUNT_FLOW;
        double ACCOUNT_BALANCE;
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

        Full_Data();
        explicit Full_Data(const Data& r);
        Full_Data(const Data& r,
            const std::map<int64 /* m_id */, TrxSplitModel::DataA> & splits,
            const std::map<int64 /* m_id */, TagLinkModel::DataA> & tags
        );
        ~Full_Data();

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

    typedef std::vector<Full_Data> Full_DataA;

public:
    static const RefTypeN s_ref_type;

public:
    // Initialize the global TrxModel table on initial call.
    // Resets the global table on subsequent calls.
    // Return the static instance address for TrxModel table
    // Note: Assigning the address to a local variable can destroy the instance.
    static TrxModel& instance(wxSQLite3Database* db);

    // Return the static instance address for TrxModel table
    // Note: Assigning the address to a local variable can destroy the instance.
    static TrxModel& instance();

public:
    // Data properties (do not require access to Model)
    // TODO: move to TrxData
    static void copy_from_trx(Data* this_n, const Data& other_d);
    static wxDateTime getTransDateTime(const Data& this_d);
    static double account_flow(const Data& this_d, int64 account_id);
    static double account_outflow(const Data& this_d, int64 account_id);
    static double account_inflow(const Data& this_d, int64 account_id);
    static double account_recflow(const Data& this_d, int64 account_id);
    static bool is_foreign(const Data& this_d);
    static bool is_foreignAsTransfer(const Data& this_d);

public:
    static TrxCol::TRANSDATE TRANSDATE(OP op, const wxString& date_iso_str);
    static TrxCol::TRANSDATE TRANSDATE(OP op, const mmDate& date);
    static TrxCol::TRANSDATE TRANSDATE(OP op, const wxDateTime& date);
    static TrxCol::TRANSCODE TRANSCODE(OP op, TrxType trx_type);
    static TrxCol::STATUS STATUS(OP op, TrxStatus trx_status);
    static TrxCol::TRANSACTIONNUMBER TRANSACTIONNUMBER(OP op, const wxString& num);
    static TrxCol::DELETEDTIME DELETEDTIME(OP op, const wxString& date);

public:
    static const TrxModel::DataA find_allByDateTimeId();
    static const TrxSplitDataA find_split(const Data& r);
    static void getFrequentUsedNotes(std::vector<wxString> &frequentNotes, int64 accountID = -1);
    static void setEmptyData(Data &data, int64 accountID);
    static bool is_locked(const Data& trx_d);

public:
    TrxModel();
    ~TrxModel();

public:
    bool purge_id(int64 id) override;
    void save_timestamp(int64 id);
    void update_timestamp(Data& trx_d);
    const Data* unsafe_save_trx_n(Data* trx_n);
    const Data* save_trx_n(Data& trx_d);
    bool save_trx_a(DataA& rows);

public:
    struct SorterByNUMBER
    {
        bool operator()(const Full_Data& x, const Full_Data& y)
        {
            return x.m_number.IsNumber() && y.m_number.IsNumber()
                ? (wxAtoi(x.m_number) < wxAtoi(y.m_number))
                : x.m_number < y.m_number;
        }
    };

    struct SorterByTRANSDATE_DATE
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.TRANSDATE.Left(10) < y.TRANSDATE.Left(10);
        }
    };

    struct SorterByTRANSDATE_TIME
    {
        bool operator()(const Data& x, const Data& y)
        {
            return mmGetTimeForDisplay(x.TRANSDATE) < mmGetTimeForDisplay(y.TRANSDATE);
        }
    };

    struct SorterByACCOUNTNAME
    {
        bool operator()(const Full_Data& x, const Full_Data& y)
        {
            return std::wcscoll(x.ACCOUNTNAME.Lower().wc_str(), y.ACCOUNTNAME.Lower().wc_str()) < 0;
        }
    };

    struct SorterByTOACCOUNTNAME
    {
        bool operator()(const Full_Data& x, const Full_Data& y)
        {
            return std::wcscoll(x.TOACCOUNTNAME.Lower().wc_str(), y.TOACCOUNTNAME.Lower().wc_str()) < 0;
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

    struct SorterByTAGNAMES
    {
        bool operator()(const Full_Data& x, const Full_Data& y)
        {
            return x.TAGNAMES < y.TAGNAMES;
        }
    };

    struct SorterByBALANCE
    {
        bool operator()(const Full_Data& x, const Full_Data& y)
        {
            return x.ACCOUNT_BALANCE < y.ACCOUNT_BALANCE;
        }
    };

    struct SorterByDEPOSIT
    {
        bool operator()(const Full_Data& x, const Full_Data& y)
        {
            return x.ACCOUNTID_D != -1 && (y.ACCOUNTID_D == -1 || x.TRANSAMOUNT_D < y.TRANSAMOUNT_D);
        }
    };

    struct SorterByWITHDRAWAL
    {
        bool operator()(const Full_Data& x, const Full_Data& y)
        {
            return x.ACCOUNTID_W != -1 && (y.ACCOUNTID_W == -1 || x.TRANSAMOUNT_W < y.TRANSAMOUNT_W);
        }
    };
};

//----------------------------------------------------------------------------

inline bool TrxModel::Full_Data::has_split() const
{
    return !this->m_splits.empty();
}

inline bool TrxModel::Full_Data::has_tags() const

{
    return !this->m_tags.empty();
}

inline bool TrxModel::Full_Data::has_attachment() const
{
    return !ATTACHMENT_DESCRIPTION.empty();
}

