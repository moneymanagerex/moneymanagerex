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

#ifndef MODEL_CHECKING_H
#define MODEL_CHECKING_H

#include "Model.h"
#include "db/DB_Table_Checkingaccount_V1.h"
#include "Model_Splittransaction.h"

class Model_Checking : public Model, public DB_Table_CHECKINGACCOUNT_V1
{
    using DB_Table_CHECKINGACCOUNT_V1::all;
    using DB_Table_CHECKINGACCOUNT_V1::get;
    using DB_Table_CHECKINGACCOUNT_V1::remove;
public:
    enum TYPE { WITHDRAWAL = 0, DEPOSIT, TRANSFER };
    enum STATUS_ENUM { NONE = 0, RECONCILED, VOID_, FOLLOWUP, DUPLICATE_ };
public:
    struct Full_Data: Data
    {
        Full_Data(): Data(0), BALANCE(0) {}
        Full_Data(const Data& r): Data(r), BALANCE(0) {}

        ~Full_Data(){}
        wxString ACCOUNTNAME;
        wxString PAYEENAME;
        wxString CATEGNAME;
        wxString DEPOSIT;
        wxString WITHDRAWAL;
        double BALANCE;
    };
    typedef std::vector<Full_Data> Full_Data_Set;
    struct SorterByBALANCE
    { 
        template<class DATA>
        bool operator()(const DATA& x, const DATA& y)
        {
            return x.BALANCE < y.BALANCE;
        }
    };
public:
    Model_Checking(): Model(), DB_Table_CHECKINGACCOUNT_V1() 
    {
    };
    ~Model_Checking() {};

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
    /** Return the static instance of Model_Checking table */
    static Model_Checking& instance()
    {
        return Singleton<Model_Checking>::instance();
    }

    /**
    * Initialize the global Model_Checking table.
    * Reset the Model_Checking table or create the table if it does not exist.
    */
    static Model_Checking& instance(wxSQLite3Database* db)
    {
        Model_Checking& ins = Singleton<Model_Checking>::instance();
        ins.db_ = db;
        ins.destroy_cache();
        ins.ensure(db);

        return ins;
    }
public:
    /** Return a list of Data records (Data_Set) directly from the database.*/
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

    /** Create a new record or update the existing record in the database.*/
    int save(Data* r)
    {
        r->save(this->db_);
        return r->id();
    }
    template<class DATA_SET>
    int save(DATA_SET& rows)
    {
        this->Begin();
        for (auto& r : rows) this->save(&r);
        this->Commit();

        return rows.size();
    }
    bool remove(int id)
    {
        //TODO: remove all split at once
        //Model_Splittransaction::instance().remove(Model_Splittransaction::instance().find(Model_Splittransaction::TRANSID(id)));
        for (const auto& r : Model_Splittransaction::instance().find(Model_Splittransaction::TRANSID(id)))
            Model_Splittransaction::instance().remove(r.SPLITTRANSID);
        return this->remove(id, db_);
    }
public:
    static Model_Splittransaction::Data_Set splittransaction(const Data* r)
    {
        return Model_Splittransaction::instance().find(Model_Splittransaction::TRANSID(r->TRANSID));
    }
    static Model_Splittransaction::Data_Set splittransaction(const Data& r)
    {
        return Model_Splittransaction::instance().find(Model_Splittransaction::TRANSID(r.TRANSID));
    }
public:
    static DB_Table_CHECKINGACCOUNT_V1::TRANSDATE TRANSDATE(const wxDate& date, OP op=EQUAL) { return DB_Table_CHECKINGACCOUNT_V1::TRANSDATE(date.FormatISODate(), op); }
    static DB_Table_CHECKINGACCOUNT_V1::STATUS STATUS(STATUS_ENUM status, OP op = EQUAL) { return DB_Table_CHECKINGACCOUNT_V1::STATUS(toShortStatus(all_status()[status]), op); }
    static DB_Table_CHECKINGACCOUNT_V1::TRANSCODE TRANSCODE(TYPE type, OP op=EQUAL) { return DB_Table_CHECKINGACCOUNT_V1::TRANSCODE(all_type()[type], op); }
public:
    static wxDate TRANSDATE(const Data* r) { return Model::to_date(r->TRANSDATE); }
    static wxDate TRANSDATE(const Data& r) { return Model::to_date(r.TRANSDATE); }
    static TYPE type(const Data* r)
    {
        if (r->TRANSCODE == all_type()[DEPOSIT])
            return DEPOSIT;
        else if (r->TRANSCODE == all_type()[TRANSFER])
            return TRANSFER;
        else
            return WITHDRAWAL;
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
    static double amount(const Data* r, int account_id = -1)
    {
        double sum = 0;
        switch (type(r))
        {
        case WITHDRAWAL:
            sum -= r->TRANSAMOUNT;
            break;
        case DEPOSIT:
            sum += r->TRANSAMOUNT;
            break;
        case TRANSFER:
            if (account_id == r->ACCOUNTID)
                sum -= r->TRANSAMOUNT;
            else
                sum += r->TOTRANSAMOUNT;
            break;
        default:
            break;
        }
        return sum;
    }
    static double amount(const Data&r, int account_id = -1) { return amount(&r, account_id); }
    static double balance(const Data* r, int account_id = -1)
    {
        if (Model_Checking::status(r) == Model_Checking::VOID_) return 0;
        return amount(r, account_id);
    }
    static double balance(const Data& r, int account_id = -1) { return balance(&r, account_id); }
    static double withdrawal(const Data* r, int account_id = -1)
    {
        double bal = balance(r, account_id);
        return bal <= 0 ? -bal : 0;
    }
    static double withdrawal(const Data& r, int account_id) { return withdrawal(&r, account_id); };
    static double deposit(const Data* r, int account_id)
    {
        double bal = balance(r, account_id);
        return bal > 0 ? bal : 0;
    }
    static double deposit(const Data& r, int account_id) { return deposit(&r, account_id); }
    static double reconciled(const Data* r, int account_id) 
    {
        double bal = balance(r, account_id);
        return Model_Checking::status(r) == Model_Checking::RECONCILED ? bal : 0;
    }
    static double reconciled(const Data& r, int account_id) { return reconciled(&r, account_id); }
    static wxString toShortStatus(const wxString& fullStatus)
    {
        wxString s = fullStatus.Left(1);
        s.Replace("N", "");
        return s;
    }
    };

#endif // 
