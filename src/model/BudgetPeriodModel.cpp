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

#include "BudgetPeriodModel.h"
#include "BudgetModel.h"

BudgetPeriodModel::BudgetPeriodModel()
: Model<DB_Table_BUDGETYEAR_V1>()
{
}

BudgetPeriodModel::~BudgetPeriodModel()
{
}

/**
* Initialize the global BudgetPeriodModel table.
* Reset the BudgetPeriodModel table or create the table if it does not exist.
*/
BudgetPeriodModel& BudgetPeriodModel::instance(wxSQLite3Database* db)
{
    BudgetPeriodModel& ins = Singleton<BudgetPeriodModel>::instance();
    ins.db_ = db;
    ins.destroy_cache();
    ins.ensure(db);

    return ins;
}

/** Return the static instance of BudgetPeriodModel table */
BudgetPeriodModel& BudgetPeriodModel::instance()
{
    return Singleton<BudgetPeriodModel>::instance();
}

bool BudgetPeriodModel::remove(int64 id)
{
    for (const BudgetModel::Data& d : BudgetModel::instance().find(BudgetModel::BUDGETYEARID(id)))
        BudgetModel::instance().remove(d.BUDGETENTRYID);
    return this->remove(id, db_);
}

// Setter
void BudgetPeriodModel::Set(int64 year_id, const wxString& value)
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

int64 BudgetPeriodModel::Add(const wxString& value)
{
    int64 year_id = this->Get(value);
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
wxString BudgetPeriodModel::Get(int64 year_id)
{
    Data* e = this->get(year_id, this->db_);
    if (e) return e->BUDGETYEARNAME;

    return "";
}

int64 BudgetPeriodModel::Get(const wxString& year_name)
{
    for (const auto& record: this->all())
    {
        if (record.BUDGETYEARNAME == year_name)
            return record.BUDGETYEARID;
    }

    return -1;
}

bool BudgetPeriodModel::Exists(int64 year_id)
{
    Data* e = this->get(year_id, this->db_);
    if (e) return true;

    return false;
}

bool BudgetPeriodModel::Exists(const wxString& year_name)
{
    for (const auto& record: this->all())
    {
        if (record.BUDGETYEARNAME == year_name) 
            return true;
    }
    return false;
}
