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

#ifndef MODEL_SPLITTRANSACTIONS_H
#define MODEL_SPLITTRANSACTIONS_H

#include "Model.h"
#include "db/DB_Table_Splittransactions_V1.h"

class Model_Splittransaction : public Model, public DB_Table_SPLITTRANSACTIONS_V1
{
    using DB_Table_SPLITTRANSACTIONS_V1::all;
    using DB_Table_SPLITTRANSACTIONS_V1::get;
public:
    Model_Splittransaction(): Model(), DB_Table_SPLITTRANSACTIONS_V1() 
    {
    };
    ~Model_Splittransaction() {};

public:
    wxArrayString types_;

public:
    /** Return the static instance of Model_Splittransaction table */
    static Model_Splittransaction& instance()
    {
        return Singleton<Model_Splittransaction>::instance();
    }

    /**
    * Initialize the global Model_Splittransaction table.
    * Reset the Model_Splittransaction table or create the table if it does not exist.
    */
    static Model_Splittransaction& instance(wxSQLite3Database* db)
    {
        Model_Splittransaction& ins = Singleton<Model_Splittransaction>::instance();
        ins.db_ = db;
        ins.destroy_cache();
        ins.ensure(db);

        return ins;
    }
public:
    /** Return a list of Data records (Data_Set) derived directly from the database. */
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

    /** Create a new record or update the existing record in the database.*/
    int save(Data* r)
    {
        r->save(this->db_);
        return r->id();
    }

    /** Create new or update existing records in the database for the record list.*/
    int save(Data_Set& rows)
    {
        this->Begin();
        for (auto& r : rows) this->save(&r);
        this->Commit();

        return rows.size();
    }
    bool remove(Data_Set& rows)
    {
        this->Begin();
        for (auto& r : rows) this->remove(r.SPLITTRANSID);
        this->Commit();
    }
    bool remove(int id)
    {
        Data *entry = get(id);
        return entry->remove(this->db_);
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
