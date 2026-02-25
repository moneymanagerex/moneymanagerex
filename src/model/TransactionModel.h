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
#include "util/_choices.h"
#include "util/mmDateDay.h"

#include "table/TransactionTable.h"
#include "_ModelBase.h"
#include "TransactionSplitModel.h"
#include "FieldModel.h"
#include "TagLinkModel.h"
// cannot include "util/util.h"

const wxString mmGetTimeForDisplay(const wxString& datetime_iso);

class TransactionModel : public Model<TransactionTable>
{
public:
    using Model<TransactionTable>::remove;
    using Model<TransactionTable>::save;
    typedef TransactionSplitModel::Data_Set Split_Data_Set;
    typedef TagLinkModel::Data_Set Taglink_Data_Set;

public:
    enum TYPE_ID
    {
        TYPE_ID_WITHDRAWAL = 0,
        TYPE_ID_DEPOSIT,
        TYPE_ID_TRANSFER,
        TYPE_ID_size
    };
    enum STATUS_ID
    {
        STATUS_ID_NONE = 0,
        STATUS_ID_RECONCILED,
        STATUS_ID_VOID,
        STATUS_ID_FOLLOWUP,
        STATUS_ID_DUPLICATE,
        STATUS_ID_size
    };
    static const wxString TYPE_NAME_WITHDRAWAL;
    static const wxString TYPE_NAME_DEPOSIT;
    static const wxString TYPE_NAME_TRANSFER;
    static const wxString STATUS_KEY_NONE;
    static const wxString STATUS_KEY_RECONCILED;
    static const wxString STATUS_KEY_VOID;
    static const wxString STATUS_KEY_FOLLOWUP;
    static const wxString STATUS_KEY_DUPLICATE;
    static const wxString STATUS_NAME_NONE;
    static const wxString STATUS_NAME_RECONCILED;
    static const wxString STATUS_NAME_VOID;
    static const wxString STATUS_NAME_FOLLOWUP;
    static const wxString STATUS_NAME_DUPLICATE;

private:
    static ChoicesName TYPE_CHOICES;
    static ChoicesName TRADE_TYPE_CHOICES;
    static ChoicesKeyName STATUS_CHOICES;

public:
    struct Full_Data: public Data
    {
        // filled-in by constructor
        wxString displayID;
        wxString ACCOUNTNAME, TOACCOUNTNAME;
        wxString PAYEENAME;
        wxString CATEGNAME;
        Split_Data_Set m_splits;
        Taglink_Data_Set m_tags;
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
        FieldModel::TYPE_ID UDFC_type[5] = {
            FieldModel::TYPE_ID_UNKNOWN,
            FieldModel::TYPE_ID_UNKNOWN,
            FieldModel::TYPE_ID_UNKNOWN,
            FieldModel::TYPE_ID_UNKNOWN,
            FieldModel::TYPE_ID_UNKNOWN
        };
        wxString UDFC_content[5];
        double UDFC_value[5] = {0, 0, 0, 0, 0};

        Full_Data();
        explicit Full_Data(const Data& r);
        Full_Data(const Data& r,
            const std::map<int64 /* TRANSID */, TransactionSplitModel::Data_Set> & splits,
            const std::map<int64 /* TRANSID */, TagLinkModel::Data_Set> & tags
        );
        ~Full_Data();

        void fill_data();
        wxString real_payee_name(int64 account_id) const;
        const wxString get_currency_code(int64 account_id) const;
        const wxString cache_id_name(int64 account_id) const;
        bool has_split() const;
        bool has_tags() const;
        bool has_attachment() const;
        bool is_foreign() const;
        bool is_foreign_transfer() const;
        wxString info() const;
        const wxString to_json();
    };

    typedef std::vector<Full_Data> Full_Data_Set;

    struct SorterByNUMBER
    {
        bool operator()(const Full_Data& x, const Full_Data& y)
        {
            return x.TRANSACTIONNUMBER.IsNumber() && y.TRANSACTIONNUMBER.IsNumber()
                ? (wxAtoi(x.TRANSACTIONNUMBER) < wxAtoi(y.TRANSACTIONNUMBER))
                : x.TRANSACTIONNUMBER < y.TRANSACTIONNUMBER;
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

public:
    TransactionModel();
    ~TransactionModel();

public:
    /**
    Initialize the global TransactionModel table on initial call.
    Resets the global table on subsequent calls.
    * Return the static instance address for TransactionModel table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static TransactionModel& instance(wxSQLite3Database* db);

    /**
    * Return the static instance address for TransactionModel table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static TransactionModel& instance();

public:
    bool remove(const int64 id);
    int64 save_trx(Data* r);
    int save_trx(std::vector<Data>& rows);
    int save_trx(std::vector<Data*>& rows);
    void updateTimestamp(int64 id);
public:
    static const TransactionModel::Data_Set allByDateTimeId();
    static const Split_Data_Set split(const Data* r);
    static const Split_Data_Set split(const Data& r);

public:
    static TransactionTable::TRANSDATE TRANSDATE(OP op, const wxString& date_iso_str);
    static TransactionTable::TRANSDATE TRANSDATE(OP op, const mmDateDay& date);
    static TransactionTable::TRANSDATE TRANSDATE(OP op, const wxDateTime& date);
    static TransactionTable::DELETEDTIME DELETEDTIME(OP op, const wxString& date);
    static TransactionTable::STATUS STATUS(OP op, STATUS_ID status);
    static TransactionTable::TRANSCODE TRANSCODE(OP op, TYPE_ID type);
    static TransactionTable::TRANSACTIONNUMBER TRANSACTIONNUMBER(OP op, const wxString& num);

public:
    static const wxString type_name(int id);
    static const wxString trade_type_name(int id);
    static int type_id(const wxString& name, int default_id = TYPE_ID_WITHDRAWAL);
    static TYPE_ID type_id(const Data* r);
    static TYPE_ID type_id(const Data& r);

    static const wxString status_key(int id);
    static const wxString status_key(const wxString& keyOrName);
    static const wxString status_name(int id);
    static const wxString status_name(const wxString& keyOrName);
    static int status_id(const wxString& keyOrName, int default_id = STATUS_ID_NONE);
    static STATUS_ID status_id(const Data* r);
    static STATUS_ID status_id(const Data& r);

    static wxDate getTransDateTime(const Data* r);
    static wxDate getTransDateTime(const Data& r);

    static double account_flow(const Data* r, int64 account_id);
    static double account_flow(const Data& r, int64 account_id);
    static double account_outflow(const Data* r, int64 account_id);
    static double account_outflow(const Data& r, int64 account_id);
    static double account_inflow(const Data* r, int64 account_id);
    static double account_inflow(const Data& r, int64 account_id);
    static double account_recflow(const Data* r, int64 account_id);
    static double account_recflow(const Data& r, int64 account_id);
    static bool is_locked(const Data* r);
    static bool is_transfer(const wxString& r);
    static bool is_transfer(const Data* r);
    static bool is_deposit(const wxString& r);
    static bool is_deposit(const Data* r);
    static bool is_split(const Data* r);
    static bool is_split(const Data& r);
    static void getFrequentUsedNotes(std::vector<wxString> &frequentNotes, int64 accountID = -1);
    static void getEmptyData(Data &data, int64 accountID);

    static bool getTransactionData(Data &data, const Data* r);
    static void putDataToTransaction(Data *r, const Data &data);
    static bool foreignTransaction(const Data& data);
    static bool foreignTransactionAsTransfer(const Data& data);

public:
    static const wxString refTypeName;
};

//----------------------------------------------------------------------------

inline const wxString TransactionModel::type_name(int id)
{
    return TYPE_CHOICES.getName(id);
}

inline const wxString TransactionModel::trade_type_name(int id)
{
    return TRADE_TYPE_CHOICES.getName(id);
}

inline int TransactionModel::type_id(const wxString& name, int default_id)
{
    return TYPE_CHOICES.findName(name, default_id);
}

inline TransactionModel::TYPE_ID TransactionModel::type_id(const Data* r)
{
    return static_cast<TYPE_ID>(type_id(r->TRANSCODE));
}

inline TransactionModel::TYPE_ID TransactionModel::type_id(const Data& r)
{
    return type_id(&r);
}

inline const wxString TransactionModel::status_key(int id)
{
    return STATUS_CHOICES.getKey(id);
}

inline const wxString TransactionModel::status_key(const wxString& keyOrName)
{
    return status_key(status_id(keyOrName));
}

inline const wxString TransactionModel::status_name(int id)
{
    return STATUS_CHOICES.getName(id);
}

inline const wxString TransactionModel::status_name(const wxString& keyOrName)
{
    return status_name(status_id(keyOrName));
}

inline int TransactionModel::status_id(const wxString& keyOrName, int default_id)
{
    return STATUS_CHOICES.findKeyName(keyOrName, default_id);
}

inline TransactionModel::STATUS_ID TransactionModel::status_id(const Data* r)
{
    return static_cast<STATUS_ID>(status_id(r->STATUS));
}

inline TransactionModel::STATUS_ID TransactionModel::status_id(const Data& r)
{
    return status_id(&r);
}

inline bool TransactionModel::Full_Data::has_split() const
{
    return !this->m_splits.empty();
}

inline bool TransactionModel::Full_Data::has_tags() const

{
    return !this->m_tags.empty();
}

inline bool TransactionModel::Full_Data::has_attachment() const
{
    return !ATTACHMENT_DESCRIPTION.empty();
}

