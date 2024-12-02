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

#ifndef MODEL_BILLSDEPOSITS_H
#define MODEL_BILLSDEPOSITS_H

#include "Model.h"
#include "db/DB_Table_Billsdeposits_V1.h"
#include "Model_Checking.h"
#include "Model_Splittransaction.h"
#include "Model_Budgetsplittransaction.h"
#include "Model_Taglink.h"

const int BD_REPEATS_MULTIPLEX_BASE = 100;

class Model_Billsdeposits : public Model<DB_Table_BILLSDEPOSITS_V1>
{
public:
    using Model<DB_Table_BILLSDEPOSITS_V1>::remove;
    typedef Model_Budgetsplittransaction::Data_Set Split_Data_Set;

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
    Model_Billsdeposits();
    ~Model_Billsdeposits();

public:
    /** Pre-initialised data structure*/
    struct Bill_Data
    {
        int BDID = 0;
        // This relates the 'Date Due' field.
        wxString TRANSDATE = wxDateTime::Now().FormatISOCombined();
        wxString STATUS = Model_Checking::STATUS_STR_NONE;
        int ACCOUNTID = -1;
        int TOACCOUNTID = -1;
        wxString TRANSCODE = Model_Checking::TYPE_STR_WITHDRAWAL;
        int CATEGID = -1;
        double TRANSAMOUNT = 0;
        double TOTRANSAMOUNT = 0;
        int FOLLOWUPID = -1;
        wxString NOTES;
        wxString TRANSACTIONNUMBER;
        int PAYEEID = -1;
        std::vector<Split> local_splits;
        int REPEATS;
        int NUMOCCURRENCES;
        // This relates the 'Date Paid' field.
        wxString NEXTOCCURRENCEDATE;
        int COLOR = -1;
        wxArrayInt TAGS;
    };

    struct Full_Data : public Data
    {
        Full_Data();
        explicit Full_Data(const Data& r);
        wxString ACCOUNTNAME;
        wxString PAYEENAME;
        wxString CATEGNAME;
        Model_Budgetsplittransaction::Data_Set m_bill_splits;
        Model_Taglink::Data_Set m_tags;
        wxString real_payee_name() const;
        wxString TAGNAMES;
    };
    typedef std::vector<Full_Data> Full_Data_Set;

public:
    /**
    Initialize the global Model_Billsdeposits table on initial call.
    Resets the global table on subsequent calls.
    * Return the static instance address for Model_Billsdeposits table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_Billsdeposits& instance(wxSQLite3Database* db);

    /**
    * Return the static instance address for Model_Billsdeposits table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_Billsdeposits& instance();

public:
    // This relates the 'Date Due' field
    static wxDate TRANSDATE(const Data* r);
    // This relates the 'Date Due' field
    static wxDate TRANSDATE(const Data& r);
    // This relates the 'Date Paid' field
    static wxDate NEXTOCCURRENCEDATE(const Data* r);
    // This relates the 'Date Paid' field
    static wxDate NEXTOCCURRENCEDATE(const Data& r);
    static Model_Checking::TYPE_ID type_id(const Data* r);
    static Model_Checking::TYPE_ID type_id(const Data& r);
    static Model_Checking::STATUS_ID status_id(const Data* r);
    static Model_Checking::STATUS_ID status_id(const Data& r);

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
    bool remove(int id);

    static DB_Table_BILLSDEPOSITS_V1::STATUS STATUS(Model_Checking::STATUS_ID status, OP op = EQUAL);
    static DB_Table_BILLSDEPOSITS_V1::TRANSCODE TRANSCODE(Model_Checking::TYPE_ID type, OP op = EQUAL);

    static const Model_Budgetsplittransaction::Data_Set split(const Data* r);
    static const Model_Budgetsplittransaction::Data_Set split(const Data& r);
    static const Model_Taglink::Data_Set taglink(const Data* r);
    static const Model_Taglink::Data_Set taglink(const Data& r);
    static wxArrayString unroll(const Data* r, const wxString end_date, int limit = -1);
    static wxArrayString unroll(const Data& r, const wxString end_date, int limit = -1);

    void completeBDInSeries(int bdID);
    static const wxDateTime nextOccurDate(int type, int numRepeats, wxDateTime nextOccurDate, bool reverse = false);
};

#endif // 

