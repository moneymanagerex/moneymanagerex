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

#ifndef MODEL_BUDGET_H
#define MODEL_BUDGET_H

#include "Model.h"
#include "db/DB_Table_Budgettable_V1.h"

class Model_Budget : public Model, public DB_Table_BUDGETTABLE_V1
{
    using DB_Table_BUDGETTABLE_V1::all;
    using DB_Table_BUDGETTABLE_V1::get;
    using DB_Table_BUDGETTABLE_V1::save;
    using DB_Table_BUDGETTABLE_V1::remove;
public:
    Model_Budget(): Model(), DB_Table_BUDGETTABLE_V1() {};
    ~Model_Budget() 
    {
    };

public:
    static Model_Budget& instance()
    {
        return Singleton<Model_Budget>::instance();
    }
    static Model_Budget& instance(wxSQLite3Database* db)
    {
        Model_Budget& ins = Singleton<Model_Budget>::instance();
        ins.db_ = db;
        return ins;
    }
    Data_Set all(COLUMN col = COLUMN(0), bool asc = true)
    {
        this->ensure(this->db_);
        return this->all(this->db_, col, asc);
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
    bool remove(int id)
    {
        return this->remove(id, db_);
    }
    static void getBudgetEntry(int budgetYearID, int categID, int subCategID, wxString& period, double& amt)
    {
        const Data_Set budget = instance().find(BUDGETYEARID(budgetYearID), CATEGID(categID), SUBCATEGID(subCategID));
        if (!budget.empty())
        {
            period = budget[0].PERIOD;
            amt = budget[0].AMOUNT;
        }
        else
        {
            period = "";
            amt = 0;
        }
    }
    static void copyBudgetYear(int newYearID, int baseYearID)
    {
        for (const Data& data : instance().find(BUDGETYEARID(baseYearID)))
        {
            Data budgetEntry(data);
            budgetEntry.BUDGETYEARID = newYearID;
            instance().save(&budgetEntry);
        }
    }
};

#endif // 
