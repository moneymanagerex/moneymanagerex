/*******************************************************
 Copyright (C) 2006-2021

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

#include "reportbase.h"
#include "util.h"
#include <map>
#include <vector>

class mmReportPayeeExpenses : public ReportBase
{
public:
    enum TYPE {
        INCOME = 0,
        EXPENSES,
        MAX
    };

private:
    // structure for sorting of data
    struct data_holder
    {
        wxString name;
        int64 payee;
        double incomes;
        double expenses;
    };

public:
    mmReportPayeeExpenses();
    virtual ~mmReportPayeeExpenses();

public:
    virtual void refreshData();
    virtual wxString getHTMLText();

protected:
    void getPayeeStats(
        std::map<int64, std::pair<double, double> > &payeeStats,
        mmDateRange* date_range, bool ignoreFuture
    ) const;

private:
    std::vector<data_holder> data_;
    std::vector<ValuePair> valueList_;
    double positiveTotal_;
    double negativeTotal_;
};

