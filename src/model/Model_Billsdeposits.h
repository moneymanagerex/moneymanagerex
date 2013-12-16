/*******************************************************
 Copyright (C) 2013 Guan Lisheng

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
#include "Model_Budgetsplittransaction.h"
#include "mmOption.h"

const int BD_REPEATS_MULTIPLEX_BASE = 100;

class Model_Billsdeposits : public Model_Mix<DB_Table_BILLSDEPOSITS_V1>
{
public:
    using Model_Mix<DB_Table_BILLSDEPOSITS_V1>::remove;
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

public:
    Model_Billsdeposits(): Model_Mix<DB_Table_BILLSDEPOSITS_V1>() 
    {
    };
    ~Model_Billsdeposits() {};
public:
    struct Full_Data : public Data
    {
        Full_Data(): Data(0)
        {}
        Full_Data(const Data& r): Data(r)
        {}
        wxString ACCOUNTNAME;
        wxString PAYEENAME;
        wxString CATEGNAME;
    };
    typedef std::vector<Full_Data> Full_Data_Set;
public:
    static wxArrayString all_type()
    {
        wxArrayString types;
        // keep the sequence with TYPE
        types.Add(wxTRANSLATE("Withdrawal"));
        types.Add(wxTRANSLATE("Deposit"));
        types.Add(wxTRANSLATE("Transfer"));

        return types;
    }
    static wxArrayString all_status()
    {
        wxArrayString status;
        // keep the sequence with STATUS
        status.Add(wxTRANSLATE("None"));
        status.Add(wxTRANSLATE("Reconciled"));
        status.Add(wxTRANSLATE("Void"));
        status.Add(wxTRANSLATE("Follow up"));
        status.Add(wxTRANSLATE("Duplicate"));

        return status;
    }

public:
    /** Return the static instance of Model_Billsdeposits table */
    static Model_Billsdeposits& instance()
    {
        return Singleton<Model_Billsdeposits>::instance();
    }

    /**
    * Initialize the global Model_Billsdeposits table.
    * Reset the Model_Billsdeposits table or create the table if it does not exist.
    */
    static Model_Billsdeposits& instance(wxSQLite3Database* db)
    {
        Model_Billsdeposits& ins = Singleton<Model_Billsdeposits>::instance();
        ins.db_ = db;
        ins.destroy_cache();
        ins.ensure(db);

        return ins;
    }
public:
    static wxDate NEXTOCCURRENCEDATE(const Data* r) { return Model::to_date(r->NEXTOCCURRENCEDATE); }
    static wxDate NEXTOCCURRENCEDATE(const Data& r) { return Model::to_date(r.NEXTOCCURRENCEDATE); }
    static TYPE type(const Data* r)
    {
        if (r->TRANSCODE.CmpNoCase(all_type()[WITHDRAWAL]) == 0)
            return WITHDRAWAL;
        else if (r->TRANSCODE.CmpNoCase(all_type()[DEPOSIT]) == 0)
            return DEPOSIT;
        else
            return TRANSFER;
    }
    static TYPE type(const Data& r) { return type(&r); }
    static STATUS_ENUM status(const Data* r)
    {
        if (r->STATUS.CmpNoCase(all_status()[NONE]) == 0)
            return NONE;
        else if (r->STATUS.CmpNoCase(all_status()[RECONCILED]) == 0 || r->STATUS.CmpNoCase("R") == 0)
            return RECONCILED;
        else if (r->STATUS.CmpNoCase(all_status()[VOID_]) == 0 || r->STATUS.CmpNoCase("V") == 0)
            return VOID_;
        else if (r->STATUS.CmpNoCase(all_status()[FOLLOWUP]) == 0 || r->STATUS.CmpNoCase("F") == 0)
            return FOLLOWUP;
        else if (r->STATUS.CmpNoCase(all_status()[DUPLICATE_]) == 0 || r->STATUS.CmpNoCase("D") == 0)
            return DUPLICATE_;
        else
            return NONE;
    }
    static STATUS_ENUM status(const Data& r) { return status(&r); }
    static wxString toShortStatus(const wxString& fullStatus)
    {
        wxString s = fullStatus.Left(1);
        s.Replace("N", "");
        return s;
    }

public:
    /**
    * Remove the Data record instance from memory and the database
    * including any splits associated with the Data Record.
    */
    bool remove(int id)
    {
        for (auto &item : Model_Billsdeposits::splittransaction(get(id)))
            Model_Budgetsplittransaction::instance().remove(item.SPLITTRANSID);
        return this->remove(id, db_);
    }

    static DB_Table_BILLSDEPOSITS_V1::STATUS STATUS(STATUS_ENUM status, OP op = EQUAL) { return DB_Table_BILLSDEPOSITS_V1::STATUS(toShortStatus(all_status()[status]), op); }
    static DB_Table_BILLSDEPOSITS_V1::TRANSCODE TRANSCODE(TYPE type, OP op = EQUAL) { return DB_Table_BILLSDEPOSITS_V1::TRANSCODE(all_type()[type], op); }

    static Model_Budgetsplittransaction::Data_Set splittransaction(const Data* r)
    {
        return Model_Budgetsplittransaction::instance().find(Model_Budgetsplittransaction::TRANSID(r->BDID));
    }
    static Model_Budgetsplittransaction::Data_Set splittransaction(const Data& r)
    {
        return Model_Budgetsplittransaction::instance().find(Model_Budgetsplittransaction::TRANSID(r.BDID));
    }

    void completeBDInSeries(int bdID)
    {
        Data* bill = get(bdID);
        if (bill)
        {
            wxDateTime dtno = NEXTOCCURRENCEDATE(bill);
            wxDateTime updateOccur = dtno;

            int repeats = bill->REPEATS;

            // DeMultiplex the Auto Executable fields.
            if (repeats >= BD_REPEATS_MULTIPLEX_BASE)    // Auto Execute User Acknowlegement required
                repeats -= BD_REPEATS_MULTIPLEX_BASE;
            if (repeats >= BD_REPEATS_MULTIPLEX_BASE)    // Auto Execute Silent mode
                repeats -= BD_REPEATS_MULTIPLEX_BASE;

            int numRepeats = bill->NUMOCCURRENCES;
            if (numRepeats != REPEAT_TYPE::REPEAT_INACTIVE)
            {
                if ((repeats < REPEAT_TYPE::REPEAT_IN_X_DAYS) || (repeats > REPEAT_TYPE::REPEAT_EVERY_X_MONTHS))
                    --numRepeats;
            }

            if (repeats == REPEAT_TYPE::REPEAT_NONE)
            {
                numRepeats = 0;
            }
            else if (repeats == REPEAT_TYPE::REPEAT_WEEKLY)
            {
                updateOccur = dtno.Add(wxTimeSpan::Week());
            }
            else if (repeats == REPEAT_TYPE::REPEAT_BI_WEEKLY)
            {
                updateOccur = dtno.Add(wxTimeSpan::Weeks(2));
            }
            else if (repeats == REPEAT_TYPE::REPEAT_MONTHLY)
            {
                updateOccur = dtno.Add(wxDateSpan::Month());
            }
            else if (repeats == REPEAT_TYPE::REPEAT_BI_MONTHLY)
            {
                updateOccur = dtno.Add(wxDateSpan::Months(2));
            }
            else if (repeats == REPEAT_TYPE::REPEAT_QUARTERLY)
            {
                updateOccur = dtno.Add(wxDateSpan::Months(3));
            }
            else if (repeats == REPEAT_TYPE::REPEAT_HALF_YEARLY)
            {
                updateOccur = dtno.Add(wxDateSpan::Months(6));
            }
            else if (repeats == REPEAT_TYPE::REPEAT_YEARLY)
            {
                updateOccur = dtno.Add(wxDateSpan::Year());
            }
            else if (repeats == REPEAT_TYPE::REPEAT_FOUR_MONTHLY)
            {
                updateOccur = dtno.Add(wxDateSpan::Months(4));
            }
            else if (repeats == REPEAT_TYPE::REPEAT_FOUR_WEEKLY)
            {
                updateOccur = dtno.Add(wxDateSpan::Weeks(4));
            }
            else if (repeats == REPEAT_TYPE::REPEAT_DAILY)
            {
                updateOccur = dtno.Add(wxDateSpan::Days(1));
            }
            else if ((repeats == REPEAT_TYPE::REPEAT_IN_X_DAYS) || (repeats == REPEAT_TYPE::REPEAT_IN_X_MONTHS))
            {
                if (numRepeats != -1) numRepeats = -1;
            }
            else if (repeats == REPEAT_TYPE::REPEAT_EVERY_X_DAYS)
            {
                if (numRepeats > 0) updateOccur = dtno.Add(wxDateSpan::Days(numRepeats));
            }
            else if (repeats == REPEAT_TYPE::REPEAT_EVERY_X_MONTHS)
            {
                if (numRepeats > 0) updateOccur = dtno.Add(wxDateSpan::Months(numRepeats));
            }
            else if ((repeats == REPEAT_TYPE::REPEAT_MONTHLY_LAST_DAY) || (repeats == REPEAT_TYPE::REPEAT_MONTHLY_LAST_BUSINESS_DAY))
            {
                updateOccur = dtno.Add(wxDateSpan::Month());
                updateOccur = updateOccur.SetToLastMonthDay(updateOccur.GetMonth(), updateOccur.GetYear());
                if (repeats == REPEAT_TYPE::REPEAT_MONTHLY_LAST_BUSINESS_DAY) // last weekday of month
                {
                    if (updateOccur.GetWeekDay() == wxDateTime::Sun || updateOccur.GetWeekDay() == wxDateTime::Sat)
                        updateOccur.SetToPrevWeekDay(wxDateTime::Fri);
                }
            }

            bill->NEXTOCCURRENCEDATE = updateOccur.FormatISODate();
            bill->NUMOCCURRENCES = numRepeats;
            save(bill);

            if (bill->NUMOCCURRENCES == 0)
                remove(bdID);

            mmOptions::instance().databaseUpdated_ = true;
        }
    }
};

#endif // 
