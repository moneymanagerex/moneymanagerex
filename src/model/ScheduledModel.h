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
#include "table/ScheduledTable.h"
#include "_ModelBase.h"
#include "TransactionModel.h"
#include "TransactionSplitModel.h"
#include "ScheduledSplitModel.h"
#include "TagLinkModel.h"

const int BD_REPEATS_MULTIPLEX_BASE = 100;

class ScheduledModel : public Model<ScheduledTable>
{
public:
    using Model<ScheduledTable>::remove;
    typedef ScheduledSplitModel::Data_Set Split_Data_Set;

public:
    enum REPEAT_TYPE {
        REPEAT_ONCE = 0,
        REPEAT_WEEKLY,
        REPEAT_BI_WEEKLY,      // FORTNIGHTLY
        REPEAT_MONTHLY,
        REPEAT_BI_MONTHLY,
        REPEAT_QUARTERLY,      // TRI_MONTHLY
        REPEAT_HALF_YEARLY,
        REPEAT_YEARLY,
        REPEAT_FOUR_MONTHLY,   // QUAD_MONTHLY
        REPEAT_FOUR_WEEKLY,    // QUAD_WEEKLY
        REPEAT_DAILY,
        REPEAT_IN_X_DAYS,
        REPEAT_IN_X_MONTHS,
        REPEAT_EVERY_X_DAYS,
        REPEAT_EVERY_X_MONTHS,
        REPEAT_MONTHLY_LAST_DAY,
        REPEAT_MONTHLY_LAST_BUSINESS_DAY
    };
    enum REPEAT_NUM {
        REPEAT_NUM_INFINITY = -1,
        REPEAT_NUM_UNKNOWN = 0
    };
    enum REPEAT_AUTO {
        REPEAT_AUTO_NONE = 0,
        REPEAT_AUTO_MANUAL = 1,
        REPEAT_AUTO_SILENT = 2
    };

public:
    ScheduledModel();
    ~ScheduledModel();

public:
    /** Pre-initialised data structure*/
    struct Bill_Data
    {
        int64 BDID = 0;
        // This relates the 'Date Due' field.
        wxString TRANSDATE = wxDateTime::Now().FormatISOCombined();
        wxString STATUS = TransactionModel::STATUS_NAME_NONE;
        int64 ACCOUNTID = -1;
        int64 TOACCOUNTID = -1;
        wxString TRANSCODE = TransactionModel::TYPE_NAME_WITHDRAWAL;
        int64 CATEGID = -1;
        double TRANSAMOUNT = 0;
        double TOTRANSAMOUNT = 0;
        int64 FOLLOWUPID = -1;
        wxString NOTES;
        wxString TRANSACTIONNUMBER;
        int64 PAYEEID = -1;
        std::vector<Split> local_splits;
        int64 REPEATS;
        int64 NUMOCCURRENCES;
        // This relates the 'Date Paid' field.
        wxString NEXTOCCURRENCEDATE;
        int64 COLOR = -1;
        wxArrayInt64 TAGS;
    };

    struct Full_Data : public Data
    {
        wxString ACCOUNTNAME;
        wxString PAYEENAME;
        wxString CATEGNAME;
        ScheduledSplitModel::Data_Set m_bill_splits;
        TagLinkModel::Data_Set m_tags;
        wxString TAGNAMES;

        Full_Data();
        explicit Full_Data(const Data& r);

        wxString real_payee_name() const;
    };
    typedef std::vector<Full_Data> Full_Data_Set;

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
            if (TransactionModel::type_id(x.TRANSCODE) == TransactionModel::TYPE_ID_WITHDRAWAL) {
                x_accountid = x.ACCOUNTID; x_transamount = x.TRANSAMOUNT;
            }
            else if (TransactionModel::type_id(x.TRANSCODE) == TransactionModel::TYPE_ID_TRANSFER) {
                x_accountid = x.ACCOUNTID; x_transamount = x.TRANSAMOUNT;
            }
            if (TransactionModel::type_id(y.TRANSCODE) == TransactionModel::TYPE_ID_WITHDRAWAL) {
                y_accountid = y.ACCOUNTID; y_transamount = y.TRANSAMOUNT;
            }
            else if (TransactionModel::type_id(y.TRANSCODE) == TransactionModel::TYPE_ID_TRANSFER) {
                y_accountid = y.ACCOUNTID; y_transamount = y.TRANSAMOUNT;
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
            if (TransactionModel::type_id(x.TRANSCODE) == TransactionModel::TYPE_ID_DEPOSIT) {
                x_accountid = x.ACCOUNTID; x_transamount = x.TRANSAMOUNT;
            }
            else if (TransactionModel::type_id(x.TRANSCODE) == TransactionModel::TYPE_ID_TRANSFER) {
                x_accountid = x.TOACCOUNTID; x_transamount = x.TOTRANSAMOUNT;
            }
            if (TransactionModel::type_id(y.TRANSCODE) == TransactionModel::TYPE_ID_DEPOSIT) {
                y_accountid = y.ACCOUNTID; y_transamount = y.TRANSAMOUNT;
            }
            else if (TransactionModel::type_id(y.TRANSCODE) == TransactionModel::TYPE_ID_TRANSFER) {
                y_accountid = y.TOACCOUNTID; y_transamount = y.TOTRANSAMOUNT;
            }
            return x_accountid != -1 && (y_accountid == -1 || x_transamount < y_transamount);
        }
    };

public:
    /**
    Initialize the global ScheduledModel table on initial call.
    Resets the global table on subsequent calls.
    * Return the static instance address for ScheduledModel table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static ScheduledModel& instance(wxSQLite3Database* db);

    /**
    * Return the static instance address for ScheduledModel table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static ScheduledModel& instance();

public:
    // This relates the 'Date Due' field
    static wxDate getTransDateTime(const Data* r);
    static wxDate getTransDateTime(const Data& r);
    // This relates the 'Date Paid' field
    static wxDate NEXTOCCURRENCEDATE(const Data* r);
    static wxDate NEXTOCCURRENCEDATE(const Data& r);
    static TransactionModel::TYPE_ID type_id(const Data* r);
    static TransactionModel::TYPE_ID type_id(const Data& r);
    static TransactionModel::STATUS_ID status_id(const Data* r);
    static TransactionModel::STATUS_ID status_id(const Data& r);

    /**
    * Decodes the internal fields and sets the condition of the following parameters:
    * autoExecuteManual(), autoExecuteSilent(), requireExecution(), allowExecution();
    */
    void decode_fields(const Data& r);
    bool autoExecuteManual();
    bool autoExecuteSilent();
    bool requireExecution();
    bool allowExecution();
    bool AllowTransaction(const Data& r);

private:
    int m_autoExecute;
    bool m_requireExecution;
    bool m_allowExecution;

public:
    /**
    * Remove the Data record instance from memory and the database
    * including any splits associated with the Data Record.
    */
    bool remove(const int64 id);

    static ScheduledTable::STATUS STATUS(OP op, TransactionModel::STATUS_ID status);
    static ScheduledTable::TRANSCODE TRANSCODE(OP op, TransactionModel::TYPE_ID type);

    static const ScheduledSplitModel::Data_Set split(const Data* r);
    static const ScheduledSplitModel::Data_Set split(const Data& r);
    static const TagLinkModel::Data_Set taglink(const Data* r);
    static const TagLinkModel::Data_Set taglink(const Data& r);
    static wxArrayString unroll(const Data* r, const wxString end_date, int limit = -1);
    static wxArrayString unroll(const Data& r, const wxString end_date, int limit = -1);

    void completeBDInSeries(int64 bdID);
    static const wxDateTime nextOccurDate(int type, int numRepeats, wxDateTime nextOccurDate, bool reverse = false);

public:
    static const wxString refTypeName;
};

