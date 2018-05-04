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

#include "Model_Subcategory.h"
#include "Model_Checking.h"
#include "Model_Billsdeposits.h"

Model_Subcategory::Model_Subcategory()
: Model<DB_Table_SUBCATEGORY>()
{
}

Model_Subcategory::~Model_Subcategory()
{
}

/**
* Initialize the global Model_Subcategory table.
* Reset the Model_Subcategory table or create the table if it does not exist.
*/
Model_Subcategory& Model_Subcategory::instance(wxSQLite3Database* db)
{
    Model_Subcategory& ins = Singleton<Model_Subcategory>::instance();
    ins.db_ = db;
    ins.destroy_cache();
    ins.ensure(db);
    ins.preload();

    return ins;
}

/** Return the static instance of Model_Subcategory table */
Model_Subcategory& Model_Subcategory::instance()
{
    return Singleton<Model_Subcategory>::instance();
}

/** Return the Data record instance for the given subcategory name and category ID */
Model_Subcategory::Data* Model_Subcategory::get(const wxString& name, int category_id)
{
    //FIXME: return wrong value
    Data* category = this->get_one(SUBCATEGNAME(name), CATEGID(category_id));
    if (category) return category;

    Data_Set items = this->find(SUBCATEGNAME(name), CATEGID(category_id));
    if (!items.empty()) category = this->get(items[0].SUBCATEGID, this->db_);
    return category;
}

bool Model_Subcategory::is_used(int id)
{
    int cat_id = instance().get(id)->CATEGID;
    const auto &deposits = Model_Billsdeposits::instance().find(Model_Billsdeposits::CATEGID(cat_id), Model_Billsdeposits::SUBCATEGID(id));
    const auto &trans = Model_Checking::instance().find(Model_Checking::CATEGID(cat_id), Model_Checking::SUBCATEGID(id));
    return !deposits.empty() || !trans.empty();
}

