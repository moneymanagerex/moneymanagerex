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
public:
    enum TYPE { WITHDRAWAL = 0, DEPOSIT, TRANSFER };
    enum STATUS { NONE = 0, RECONCILED, VOID_, FOLLOWUP, DUPLICATE_ };
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
        status.Add(("Reconciled"));
        status.Add(("Void"));
        status.Add(("Follow up"));
        status.Add(("Duplicate"));

        return status;
    }
public:
    static Model_Checking& instance()
    {
        return Singleton<Model_Checking>::instance();
    }
    static Model_Checking& instance(wxSQLite3Database* db)
    {
        Model_Checking& ins = Singleton<Model_Checking>::instance();
        ins.db_ = db;
        return ins;
    }
public:
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
    Data_Set find(bool op_and, const Args&... args)
    {
        return find_by(this, db_, op_and, args...);
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
    static wxDate TRANSDATE(const Data* r) { return Model::to_date(r->TRANSDATE); }
    static wxDate TRANSDATE(const Data& r) { return Model::to_date(r.TRANSDATE); }
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
    static STATUS status(const Data* r)
    {
        if (r->STATUS.CmpNoCase("None") == 0)
            return NONE;
        else if (r->STATUS.CmpNoCase("Reconciled") == 0 || r->STATUS.CmpNoCase("R") == 0)
            return RECONCILED;
        else if (r->STATUS.CmpNoCase("Void") == 0 || r->STATUS.CmpNoCase("V") == 0)
            return VOID_;
        else if (r->STATUS.CmpNoCase("Follow up") == 0 || r->STATUS.CmpNoCase("F") == 0)
            return FOLLOWUP;
        else if (r->STATUS.CmpNoCase("Duplicate") == 0)
            return DUPLICATE_;
        else 
            return NONE;
    }
    static STATUS status(const Data& r) { return status(&r); }
    static double balance(const Data* r, int account_id = -1)
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
            if (account_id == r->ACCOUNTID)     sum -= r->TRANSAMOUNT;
            if (account_id == r->TOACCOUNTID)   sum += r->TRANSAMOUNT;
            break;
        default:
            break;
        }
        return sum;
    }
    static double balance(const Data& r, int account_id = -1) { return balance(&r, account_id); }
    static double getAmountForSplit(const Data& r, int categID, int subcategID)
    {
        double splitAmount = 0.0;
        const Model_Splittransaction::Data_Set& splitEntries = Model_Checking::instance().splittransaction(r);
        if (splitEntries.size() > 0)
        {
            for (const auto & pSplitEntry: splitEntries)
            {
                if ((pSplitEntry.CATEGID == categID) &&
                    (pSplitEntry.SUBCATEGID == subcategID))
                {
                    splitAmount += pSplitEntry.SPLITTRANSAMOUNT;
                }
            }
        }
        else if ((r.CATEGID == categID) &&
            (r.SUBCATEGID == subcategID))
        {
            splitAmount = r.TRANSAMOUNT;
        }

        return splitAmount;
    }
};

#endif // 
