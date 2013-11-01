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

class Model_Billsdeposits : public Model, public DB_Table_BILLSDEPOSITS_V1
{
    using DB_Table_BILLSDEPOSITS_V1::all;
    using DB_Table_BILLSDEPOSITS_V1::get;
    using DB_Table_BILLSDEPOSITS_V1::remove;
public:
    enum TYPE { WITHDRAWAL = 0, DEPOSIT, TRANSFER };
    enum STATUS_ENUM { NONE = 0, RECONCILED, VOID_, FOLLOWUP, DUPLICATE_ };
public:
    Model_Billsdeposits(): Model(), DB_Table_BILLSDEPOSITS_V1() 
    {
    };
    ~Model_Billsdeposits() {};

public:
    static wxArrayString all_type()
    {
        wxArrayString types;
        // keep the sequence with TYPE
        types.Add(("Withdrawal"));
        types.Add(("Deposit"));
        types.Add(("Transfer"));

        return types;
    }
    static wxArrayString all_status()
    {
        wxArrayString status;
        // keep the sequence with STATUS
        status.Add(("None"));
        status.Add(("Reconciled"));
        status.Add(("Void"));
        status.Add(("Follow up"));
        status.Add(("Duplicate"));

        return status;
    }

public:
    /** Return the address of the global database table*/
    static Model_Billsdeposits& instance()
    {
        return Singleton<Model_Billsdeposits>::instance();
    }

    /**
     * Initialize the global database table.
     * Create the table if it does not exist.
    */
    static Model_Billsdeposits& instance(wxSQLite3Database* db)
    {
        Model_Billsdeposits& ins = Singleton<Model_Billsdeposits>::instance();
        ins.db_ = db;
        ins.destroy_cache();
        ins.all();
        return ins;
    }
public:
    static wxDate NEXTOCCURRENCEDATE(const Data* r) { return Model::to_date(r->NEXTOCCURRENCEDATE); }
    static wxDate NEXTOCCURRENCEDATE(const Data& r) { return Model::to_date(r.NEXTOCCURRENCEDATE); }
    static TYPE type(const Data* r)
    {
        if (r->TRANSCODE.CmpNoCase("Withdrawal") == 0)
            return WITHDRAWAL;
        else if (r->TRANSCODE.CmpNoCase("Deposit") == 0)
            return DEPOSIT;
        else
            return TRANSFER;
    }
    static TYPE type(const Data& r) { return type(&r); }
    static STATUS_ENUM status(const Data* r)
    {
        if (r->STATUS.CmpNoCase("None") == 0)
            return NONE;
        else if (r->STATUS.CmpNoCase("Reconciled") == 0 || r->STATUS.CmpNoCase("R") == 0)
            return RECONCILED;
        else if (r->STATUS.CmpNoCase("Void") == 0 || r->STATUS.CmpNoCase("V") == 0)
            return VOID_;
        else if (r->STATUS.CmpNoCase("Follow up") == 0 || r->STATUS.CmpNoCase("F") == 0)
            return FOLLOWUP;
        else if (r->STATUS.CmpNoCase("Duplicate") == 0 || r->STATUS.CmpNoCase("D") == 0)
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
    /** Return a list of all the records in the database table*/
    Data_Set all(COLUMN col = COLUMN(0), bool asc = true)
    {
        this->ensure(this->db_);
        return all(db_, col, asc);
    }
    template<typename... Args>
    Data_Set find(const Args&... args)
    {
        return find_by(this, db_, true, args...);
    }
    template<typename... Args>
    Data_Set find_or(const Args&... args)
    {
        return find_by(this, db_, false, args...);
    }
    Data* get(int id)
    {
        return this->get(id, this->db_);
    }
    int save(Data* r)
    {
        r->save(this->db_);
        return r->id();
    }
    int save(Data_Set& rows)
    {
        this->Begin();
        for (auto& r : rows) this->save(&r);
        this->Commit();

        return rows.size();
    }
    bool remove(int id)
    {
        // Remove any splits associated with id
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
            if (numRepeats != -1)
            {
                if ((repeats < 11) || (repeats > 14)) --numRepeats;
            }

            if (repeats == 0)
            {
                numRepeats = 0;
            }
            else if (repeats == 1)
            {
                updateOccur = dtno.Add(wxTimeSpan::Week());
            }
            else if (repeats == 2)
            {
                updateOccur = dtno.Add(wxTimeSpan::Weeks(2));
            }
            else if (repeats == 3)
            {
                updateOccur = dtno.Add(wxDateSpan::Month());
            }
            else if (repeats == 4)
            {
                updateOccur = dtno.Add(wxDateSpan::Months(2));
            }
            else if (repeats == 5)
            {
                updateOccur = dtno.Add(wxDateSpan::Months(3));
            }
            else if (repeats == 6)
            {
                updateOccur = dtno.Add(wxDateSpan::Months(6));
            }
            else if (repeats == 7)
            {
                updateOccur = dtno.Add(wxDateSpan::Year());
            }
            else if (repeats == 8)
            {
                updateOccur = dtno.Add(wxDateSpan::Months(4));
            }
            else if (repeats == 9)
            {
                updateOccur = dtno.Add(wxDateSpan::Weeks(4));
            }
            else if (repeats == 10)
            {
                updateOccur = dtno.Add(wxDateSpan::Days(1));
            }
            else if ((repeats == 11) || (repeats == 12))
            {
                if (numRepeats != -1) numRepeats = -1;
            }
            else if (repeats == 13)
            {
                if (numRepeats > 0) updateOccur = dtno.Add(wxDateSpan::Days(numRepeats));
            }
            else if (repeats == 14)
            {
                if (numRepeats > 0) updateOccur = dtno.Add(wxDateSpan::Months(numRepeats));
            }
            else if ((repeats == 15) || (repeats == 16))
            {
                updateOccur = dtno.Add(wxDateSpan::Month());
                updateOccur = updateOccur.SetToLastMonthDay(updateOccur.GetMonth(), updateOccur.GetYear());
                if (repeats == 16) // last weekday of month
                {
                    if (updateOccur.GetWeekDay() == wxDateTime::Sun || updateOccur.GetWeekDay() == wxDateTime::Sat)
                        updateOccur.SetToPrevWeekDay(wxDateTime::Fri);
                }
            }

            bill->NEXTOCCURRENCEDATE = updateOccur.FormatISODate();
            bill->NUMOCCURRENCES = numRepeats;
            save(bill);

            if (bill->NUMOCCURRENCES)
                remove(bdID);

            mmOptions::instance().databaseUpdated_ = true;
        }
    }
};

#endif // 
