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

#ifndef MODEL_BILLSDEPOSITS_H
#define MODEL_BILLSDEPOSITS_H

#include "Model.h"
#include "Table_Billsdeposits.h"
#include "Model_Splittransaction.h"
#include "Model_Budgetsplittransaction.h"

const int BD_REPEATS_MULTIPLEX_BASE = 100;

class Model_Billsdeposits : public Model<DB_Table_BILLSDEPOSITS>
{
public:
    using Model<DB_Table_BILLSDEPOSITS>::remove;
    typedef Model_Budgetsplittransaction::Data_Set Split_Data_Set;

public:
    enum TYPE { WITHDRAWAL = 0, DEPOSIT, TRANSFER };
    enum STATUS_ENUM { NONE = 0, RECONCILED, VOID_, FOLLOWUP, DUPLICATE_ };
    enum REPEAT_TYPE {
        REPEAT_INACTIVE = -1,
        REPEAT_NONE,
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

    static const std::vector<std::pair<TYPE, wxString> > TYPE_CHOICES;
    static const std::vector<std::pair<STATUS_ENUM, wxString> > STATUS_ENUM_CHOICES;

public:
    Model_Billsdeposits();
    ~Model_Billsdeposits();

public:
    /** Pre-initialised data structure*/
    struct Bill_Data
    {
        int BDID = 0;
        // This relates the 'Date Due' field.
        wxString TRANSDATE = wxDateTime::Now().FormatISODate();
        wxString STATUS = Model_Billsdeposits::all_status()[Model_Billsdeposits::NONE];;
        int ACCOUNTID = -1;
        int TOACCOUNTID = -1;
        wxString TRANSCODE = Model_Billsdeposits::all_type()[Model_Billsdeposits::WITHDRAWAL];;
        int CATEGID = -1;
        int SUBCATEGID = -1;
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
    };

    struct Full_Data : public Data
    {
        Full_Data();
        explicit Full_Data(const Data& r);
        wxString ACCOUNTNAME;
        wxString PAYEENAME;
        wxString CATEGNAME;
        Model_Budgetsplittransaction::Data_Set m_splits;
        bool has_split() const;
    };
    typedef std::vector<Full_Data> Full_Data_Set;

public:
    static wxArrayString all_type();
    static wxArrayString all_status();

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
    static TYPE type(const wxString& r);
    static TYPE type(const Data* r);
    static TYPE type(const Data& r);
    static STATUS_ENUM status(const wxString& r);
    static STATUS_ENUM status(const Data* r);
    static STATUS_ENUM status(const Data& r);
    static wxString toShortStatus(const wxString& fullStatus);

    /**
    * Decodes the internal fields and sets the condition of the following parameters:
    * autoExecuteManual(), autoExecuteSilent(), requireExecution(), allowExecution();
    */
    void decode_fields(const Data& r);
    bool autoExecuteManual();
    bool autoExecuteSilent();
    bool requireExecution();
    bool allowExecution();
	typedef std::map<int, double> AccountBalance;
	bool AllowTransaction(const Data& r, AccountBalance& bal);

    static int daysPayment(const Data* r);
    static int daysOverdue(const Data* r);

private:
    bool m_autoExecuteManual;
    bool m_autoExecuteSilent;
    bool m_requireExecution;
    bool m_allowExecution;

public:
    /**
    * Remove the Data record instance from memory and the database
    * including any splits associated with the Data Record.
    */
    bool remove(int id);

    static DB_Table_BILLSDEPOSITS::STATUS STATUS(STATUS_ENUM status, OP op = EQUAL);
    static DB_Table_BILLSDEPOSITS::TRANSCODE TRANSCODE(TYPE type, OP op = EQUAL);

    static const Model_Budgetsplittransaction::Data_Set splittransaction(const Data* r);
    static const Model_Budgetsplittransaction::Data_Set splittransaction(const Data& r);

    void completeBDInSeries(int bdID);
    static const wxDateTime nextOccurDate(int type, int numRepeats, const wxDateTime& nextOccurDate);
};

#endif // 

