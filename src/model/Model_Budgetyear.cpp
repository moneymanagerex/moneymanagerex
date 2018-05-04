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

#include "Model_Budgetyear.h"
#include "Model_Budget.h"

Model_Budgetyear::Model_Budgetyear()
: Model<DB_Table_BUDGETYEAR>()
{
}

Model_Budgetyear::~Model_Budgetyear()
{
}

/**
* Initialize the global Model_Budgetyear table.
* Reset the Model_Budgetyear table or create the table if it does not exist.
*/
Model_Budgetyear& Model_Budgetyear::instance(wxSQLite3Database* db)
{
    Model_Budgetyear& ins = Singleton<Model_Budgetyear>::instance();
    ins.db_ = db;
    ins.destroy_cache();
    ins.ensure(db);

    return ins;
}

/** Return the static instance of Model_Budgetyear table */
Model_Budgetyear& Model_Budgetyear::instance()
{
    return Singleton<Model_Budgetyear>::instance();
}

bool Model_Budgetyear::remove(int id)
{
    for (const Model_Budget::Data& d : Model_Budget::instance().find(Model_Budget::BUDGETYEARID(id)))
        Model_Budget::instance().remove(d.BUDGETENTRYID);
    return this->remove(id, db_);
}

// Setter
void Model_Budgetyear::Set(int year_id, const wxString& value)
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

int Model_Budgetyear::Add(const wxString& value)
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

// Getter
wxString Model_Budgetyear::Get(int year_id)
{
    Data* e = this->get(year_id, this->db_);
    if (e) return e->BUDGETYEARNAME;

    return "";
}

int Model_Budgetyear::Get(const wxString& year_name)
{
    for (const auto& record: this->all())
    {
        if (record.BUDGETYEARNAME == year_name)
            return record.BUDGETYEARID;
    }

    return -1;
}

bool Model_Budgetyear::Exists(int year_id)
{
    Data* e = this->get(year_id, this->db_);
    if (e) return true;

    return false;
}

bool Model_Budgetyear::Exists(const wxString& year_name)
{
    for (const auto& record: this->all())
    {
        if (record.BUDGETYEARNAME == year_name) 
            return true;
    }
    return false;
}
