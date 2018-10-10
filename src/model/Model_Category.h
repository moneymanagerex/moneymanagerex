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

#ifndef MODEL_CATEGORY_H
#define MODEL_CATEGORY_H

#include "Model.h"
#include "Table_Category.h"
#include "Model_Subcategory.h"

class mmDateRange;
class Model_Category : public Model<DB_Table_CATEGORY>
{
public:
    using Model<DB_Table_CATEGORY>::get;

public:
    Model_Category();
    ~Model_Category();

public:
    /**
    Initialize the global Model_Category table on initial call.
    Resets the global table on subsequent calls.
    * Return the static instance address for Model_Category table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_Category& instance(wxSQLite3Database* db);

    /**
    * Return the static instance address for Model_Category table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_Category& instance();

public:
    /** Return the Data record for the given category name */
    Data* get(const wxString& name);

    static const std::map<wxString, std::pair<int, int> > all_categories();
    static Model_Subcategory::Data_Set sub_category(const Data* r);
    static Model_Subcategory::Data_Set sub_category(const Data& r);
    static const wxString full_name(const Data* category, const Model_Subcategory::Data* sub_category = nullptr);
    static const wxString full_name(const int category_id, const int subcategory_id);
    static bool is_used(int id, int sub_id = -1);
    static bool has_income(int id, int sub_id = -1);
    static void getCategoryStats(
        std::map<int, std::map<int, std::map<int, double> > > &categoryStats
        , mmDateRange* date_range, bool ignoreFuture
        , bool group_by_month = true
        , std::map<int, std::map<int, double> > *budgetAmt = nullptr);
};

#endif //
