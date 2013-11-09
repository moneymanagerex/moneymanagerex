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

#ifndef MODEL_PAYEE_H
#define MODEL_PAYEE_H

#include "Model.h"
#include "db/DB_Table_Payee_V1.h"
#include "Model_Checking.h" // detect whether the payee is used or not
#include "Model_Billsdeposits.h"

class Model_Payee : public Model, public DB_Table_PAYEE_V1
{
    using DB_Table_PAYEE_V1::all;
    using DB_Table_PAYEE_V1::get;
    using DB_Table_PAYEE_V1::remove;
public:
    Model_Payee(): Model(), DB_Table_PAYEE_V1() 
    {
    };
    ~Model_Payee() {};

public:
    static Model_Payee& instance()
    {
        return Singleton<Model_Payee>::instance();
    }
    static Model_Payee& instance(wxSQLite3Database* db)
    {
        Model_Payee& ins = Singleton<Model_Payee>::instance();
        ins.db_ = db;
        ins.destroy_cache();
        ins.all();
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
    Data_Set FilterPayees(const wxString& payee_pattern)
    {
        Data_Set payees;
        for (auto &payee: this->all())
        {
            if (payee.PAYEENAME.Lower().Matches(payee_pattern.Lower().Append("*")))
                payees.push_back(payee);
        }
        return payees;
    }
    Data* get(int id)
    {
        return this->get(id, this->db_);
    }
    Data* get(const wxString& name)
    {
        Data* payee = 0;
        Data_Set items = this->find(PAYEENAME(name));
        if (!items.empty()) payee = this->get(items[0].PAYEEID, this->db_);
        return payee;
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
        if (is_used(id)) return false;

        return this->remove(id, db_);
    }
public:
    wxArrayString all_payee_names()
    {
        wxArrayString payees;
        for (const auto &payee: this->all(COL_PAYEENAME))
        {
            payees.Add(payee.PAYEENAME);
        }
        return payees;
    }
public:
    static bool is_used(int id)
    {
        Model_Checking::Data_Set trans = Model_Checking::instance().find(Model_Checking::PAYEEID(id));
        Model_Billsdeposits::Data_Set bills = Model_Billsdeposits::instance().find(Model_Billsdeposits::PAYEEID(id));
        return !trans.empty() || !bills.empty();
    }
    static bool is_used(const Data* record)
    {
        return is_used(record->PAYEEID);
    }
    static bool is_used(const Data& record)
    {
        return is_used(&record);
    }
};

#endif // 
