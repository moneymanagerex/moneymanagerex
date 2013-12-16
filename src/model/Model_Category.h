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

#ifndef MODEL_CATEGORY_H
#define MODEL_CATEGORY_H

#include "Model.h"
#include "db/DB_Table_Category_V1.h"
#include "Model_Subcategory.h"
#include "Model_Billsdeposits.h"

class mmDateRange;
class Model_Category : public Model_Mix<DB_Table_CATEGORY_V1>
{
public:
    using Model_Mix<DB_Table_CATEGORY_V1>::get;
public:
    Model_Category();
    ~Model_Category();

public:
    /** Return the static instance of Model_Category table */
    static Model_Category& instance()
    {
        return Singleton<Model_Category>::instance();
    }

    /**
    * Initialize the global Model_Category table.
    * Reset the Model_Category table or create the table if it does not exist.
    */
    static Model_Category& instance(wxSQLite3Database* db)
    {
        Model_Category& ins = Singleton<Model_Category>::instance();
        ins.db_ = db;
        ins.destroy_cache();
        bool init_categories = !ins.exists(db);
        ins.ensure(db);
        if (init_categories)
            ins.initialize();

        return ins;
    }
public:
    void initialize();
    Data* get(const wxString& name)
    {
        Data* category = 0;
        Data_Set items = this->find(CATEGNAME(name));
        if (!items.empty()) category = this->get(items[0].CATEGID, this->db_);
        return category;
    }
public:
    static Model_Subcategory::Data_Set sub_category(const Data* r)
    {
        return Model_Subcategory::instance().find(Model_Subcategory::CATEGID(r->CATEGID));
    }
    static Model_Subcategory::Data_Set sub_category(const Data& r)
    {
        return Model_Subcategory::instance().find(Model_Subcategory::CATEGID(r.CATEGID));
    }
    static wxString full_name(const Data* category, const Model_Subcategory::Data* sub_category = 0)
    {
        if (!category) return "";
        if (!sub_category)
            return category->CATEGNAME;
        else
            return category->CATEGNAME + ":" + sub_category->SUBCATEGNAME;
    }
    static wxString full_name(int category_id, int subcategory_id = -1)
    {
        Data* category = Model_Category::instance().get(category_id);
        Model_Subcategory::Data* sub_category = Model_Subcategory::instance().get(subcategory_id);
        return full_name(category, sub_category);
    }
    static bool is_used(int id, int sub_id = -1);
    static bool has_income(int id, int sub_id = -1);
    static void getCategoryStats(
        std::map<int, std::map<int, std::map<int, double> > > &categoryStats
        , mmDateRange* date_range, bool ignoreFuture
        , bool group_by_month = true, bool with_date = true
        , std::map<int, std::map<int, double> > *budgetAmt = 0);
};

#endif //
