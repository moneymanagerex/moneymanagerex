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

#ifndef MODEL_BUDGETYEAR_H
#define MODEL_BUDGETYEAR_H

#include "Model.h"
#include "Model_Budget.h"
#include "db/DB_Table_Budgetyear_V1.h"

class Model_Budgetyear : public Model, public DB_Table_BUDGETYEAR_V1
{
    using DB_Table_BUDGETYEAR_V1::all;
    using DB_Table_BUDGETYEAR_V1::remove;
public:
    Model_Budgetyear(): Model(), DB_Table_BUDGETYEAR_V1() {};
    ~Model_Budgetyear() 
    {
    };

public:
    static Model_Budgetyear& instance()
    {
        return Singleton<Model_Budgetyear>::instance();
    }
    static Model_Budgetyear& instance(wxSQLite3Database* db)
    {
        Model_Budgetyear& ins = Singleton<Model_Budgetyear>::instance();
        ins.db_ = db;
        ins.destroy_cache();
        ins.all();
        return ins;
    }
    Data_Set all(COLUMN col = COLUMN(0), bool asc = true)
    {
        this->ensure(this->db_);
        return this->all(this->db_, col, asc);
    }
    bool remove(int id)
    {
        for (Model_Budget::Data& d : Model_Budget::instance().find(Model_Budget::BUDGETYEARID(id)))
            Model_Budget::instance().remove(d.BUDGETENTRYID);
        return this->remove(id, db_);
    }

public:
    // Setter
    void Set(int year_id, const wxString& value)
    {
        Data* info = this->get(year_id, this->db_);
        if (info)
        {
            info->BUDGETYEARNAME = value;
            info->save(this->db_);
        }
        else
        {
            info = this->create();
            info->BUDGETYEARID = year_id;
            info->BUDGETYEARNAME = value;
            info->save(this->db_);
        }
    }

    int Add(const wxString& value)
    {
        int year_id = this->Get(value);
        if (year_id < 0)
        {
            Data* e = this->create();
            e->BUDGETYEARNAME = value;
            e->save(this->db_);
            year_id = e->id();
        }
        return year_id;
    }
public:
    // Getter
    wxString Get(int year_id)
    {
        Data* e = this->get(year_id, this->db_);
        if (e) return e->BUDGETYEARNAME;

        return "";
    }
    int Get(const wxString& year_name)
    {
        for (const auto& record: this->all())
        {
            if (record.BUDGETYEARNAME == year_name)
                return record.BUDGETYEARID;
        }

        return -1;
    }
    bool Exists(int year_id)
    {
        Data* e = this->get(year_id, this->db_);
        if (e) return true;

        return false;
    }
    bool Exists(const wxString& year_name)
    {
        for (const auto& record: this->all())
        {
            if (record.BUDGETYEARNAME == year_name) 
                return true;
        }
        return false;
    }
};

#endif // 
