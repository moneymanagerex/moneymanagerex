/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2017 James Higley
 Copyright (C) 2021 Mark Whalley (mark@ipx.co.uk)

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

#include <vector>
#include "util/_util.h"
#include "model/CategoryModel.h"
#include "_ReportBase.h"

class CategoryReport : public ReportBase
{
public:
    // structure for sorting of data
    struct data_holder { int64 catID; int64 subCatID; wxString name; double amount; int64 categs; int level; };
    enum TYPE { GOES = 0, COME , MONTHLY, SUMMARY };
    explicit CategoryReport(const wxString& title, enum TYPE type);
    virtual ~CategoryReport();

    virtual void refreshData();
    double AppendData(const std::vector<data_holder>& data, std::map<int64, std::map<int, double>>& categoryStats,
        const CategoryTable::Data* category, int64 groupID, int level);
    virtual wxString getHTMLText();

protected:
    enum TYPE type_;

private:
    std::vector<data_holder> data_;
};

class mmReportCategoryExpensesGoes: public CategoryReport
{
public:
    mmReportCategoryExpensesGoes();
};

class mmReportCategoryExpensesComes : public CategoryReport
{
public:
    mmReportCategoryExpensesComes();
};

class mmReportCategoryExpensesCategories : public CategoryReport
{
public:
    mmReportCategoryExpensesCategories();
};

//----------------------------------------------------------------------------

class mmReportCategoryOverTimePerformance : public ReportBase
{
public:
    mmReportCategoryOverTimePerformance();
    ~mmReportCategoryOverTimePerformance();

    wxString getHTMLText();

protected:
    enum TYPE { INCOME = 0, EXPENSES, TOTAL, MAX };
};

