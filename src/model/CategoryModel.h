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

#pragma once

#include "base/defs.h"
#include <wx/sharedptr.h>

#include "table/CategoryTable.h"
#include "data/CategoryData.h"

#include "_ModelBase.h"

class mmDateRange;
class CategoryModel : public TableFactory<CategoryTable, CategoryData>
{
public:
    CategoryModel();
    ~CategoryModel();

public:
    static CategoryModel& instance(wxSQLite3Database* db);
    static CategoryModel& instance();

public:
    bool is_used(int64 cat_id);

    auto get_key_data_n(const wxString& name, const int64 parentid) -> const Data*;
    auto find_data_sub_a(const Data& cat_d) -> CategoryModel::DataA;
    auto find_data_subtree_a(const Data& cat_d) -> CategoryModel::DataA;

    auto all_categories(bool excludeHidden = false) -> const std::map<wxString, int64>;
    auto full_name(const Data* cat_n) -> const wxString;
    auto full_name(int64 cat_id) -> const wxString;
    auto full_name(int64 cat_id, wxString delimiter) -> const wxString;
    bool is_hidden(int64 cat_id);
    bool has_income(int64 cat_id);

    auto get_name(const wxString& name, const wxString& parentname) -> const Data*;
    auto FilterCategory(const wxString& cat_pattern) -> const wxArrayString;
    static void getCategoryStats(
        std::map<int64, std::map<int, double>>& categoryStats,
        wxSharedPtr<wxArrayString> accountArray,
        mmDateRange* date_range,
        bool ignoreFuture,
        bool group_by_month = true,
        std::map<int64, double>* budgetAmt = nullptr,
        bool fin_months = false
    );

public:
    struct SorterByFULLNAME
    {
        bool operator()(const Data& x, const Data& y)
        {
            return CategoryModel::instance().full_name(&x) < CategoryModel::instance().full_name(&y);
        }
    };
};

