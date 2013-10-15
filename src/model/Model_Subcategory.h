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

#ifndef MODEL_SUBCATEGORY_H
#define MODEL_SUBCATEGORY_H

#include "Model.h"
#include "db/DB_Table_Subcategory_V1.h"
#include "Model_Category.h"

class Model_Subcategory : public Model, public DB_Table_SUBCATEGORY_V1
{
    using DB_Table_SUBCATEGORY_V1::all;
    using DB_Table_SUBCATEGORY_V1::get;
    using DB_Table_SUBCATEGORY_V1::remove;
public:
    Model_Subcategory(): Model(), DB_Table_SUBCATEGORY_V1() 
    {
    };
    ~Model_Subcategory() {};

public:
    wxArrayString types_;

public:
    static Model_Subcategory& instance()
    {
        return Singleton<Model_Subcategory>::instance();
    }
    static Model_Subcategory& instance(wxSQLite3Database* db)
    {
        Model_Subcategory& ins = Singleton<Model_Subcategory>::instance();
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

    Data* get(int id)
    {
        return this->get(id, this->db_);
    }
    Data* get(const wxString& name, int category_id = -1)
    {
        Data* category = 0;
        Data_Set items = this->find(SUBCATEGNAME(name), CATEGID(category_id));
        if (!items.empty()) category = this->get(items[0].SUBCATEGID, this->db_);
        return category;
    }
    int save(Data* r)
    {
        r->save(this->db_);
        return r->id();
    }
    bool remove(int id)
    {
        return this->remove(id, db_);
    }
public:
    static bool is_used(int /* id */)
    {
        // TODO
        return true;
    }
};

#endif // 
