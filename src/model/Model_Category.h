/*******************************************************
 Copyright (C) 2013,2014 Guan Lisheng (guanlisheng@gmail.com)
 Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)

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
#include <wx/sharedptr.h>
#include "db/DB_Table_Category_V1.h"

class mmDateRange;
class Model_Category : public Model<DB_Table_CATEGORY_V1>
{
public:
    using Model<DB_Table_CATEGORY_V1>::get;
    struct SorterByFULLNAME
    {
        template<class DATA>
        bool operator()(const DATA& x, const DATA& y)
        {
            return full_name(x.CATEGID) < full_name(y.CATEGID);
        }
    };

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
    Data* get(const wxString& name, const int64 parentid);
    Data* get(const wxString& name, const wxString& parentname);

    const wxArrayString FilterCategory(const wxString& category_pattern);
    static const std::map<wxString, int64 > all_categories(bool excludeHidden = false);
    static Model_Category::Data_Set sub_category(const Data* r);
    static Model_Category::Data_Set sub_category(const Data& r);
    static Model_Category::Data_Set sub_tree(const Data& r);
    static Model_Category::Data_Set sub_tree(const Data* r);
    static const wxString full_name(int64 category_id);
    static const wxString full_name(int64 category_id, wxString delimiter);
    static bool is_hidden(int64 catID);
    static bool is_used(int64 id);
    static bool has_income(int64 id);
    static void getCategoryStats(
        std::map<int64, std::map<int, double>> &categoryStats
        , wxSharedPtr<wxArrayString> accountArray
        , mmDateRange* date_range, bool ignoreFuture
        , bool group_by_month = true
        , std::map<int64, double >*budgetAmt = nullptr
        , bool fin_months = false);
    static const wxString full_name(const Data* category);

};

#endif //
