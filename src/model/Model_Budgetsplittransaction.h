/*******************************************************
 Copyright (C) 2013 James Higley

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

#ifndef MODEL_BUDGETSPLITTRANSACTIONS_H
#define MODEL_BUDGETSPLITTRANSACTIONS_H

#include "Model.h"
#include "db/DB_Table_Budgetsplittransactions_V1.h"

class Model_Budgetsplittransaction : public Model, public DB_Table_BUDGETSPLITTRANSACTIONS_V1
{
    using DB_Table_BUDGETSPLITTRANSACTIONS_V1::all;
    using DB_Table_BUDGETSPLITTRANSACTIONS_V1::get;
    using DB_Table_BUDGETSPLITTRANSACTIONS_V1::remove;
public:
    Model_Budgetsplittransaction() : Model(), DB_Table_BUDGETSPLITTRANSACTIONS_V1()
    {
    };
    ~Model_Budgetsplittransaction() {};

public:
    wxArrayString types_;

public:
    static Model_Budgetsplittransaction& instance()
    {
        return Singleton<Model_Budgetsplittransaction>::instance();
    }
    static Model_Budgetsplittransaction& instance(wxSQLite3Database* db)
    {
        Model_Budgetsplittransaction& ins = Singleton<Model_Budgetsplittransaction>::instance();
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

    /** Remove the Data record from the database and the memory table (cashe)*/
    bool remove(int id)
    {
        return this->remove(id, this->db_);
    }

public:
    double get_total(Data_Set& rows)
    {
        double total = 0.0;
        for (auto& r : rows) total += r.SPLITTRANSAMOUNT;
        return total;
    }
};

#endif // 
