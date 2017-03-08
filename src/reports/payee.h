/*******************************************************
 Copyright (C) 2006-2012

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

#ifndef MM_EX_REPORTPAYEE_H_
#define MM_EX_REPORTPAYEE_H_

#include "reportbase.h"
#include "util.h"
#include <map>
#include <vector>

class mmReportPayeeExpenses : public mmPrintableBase
{
public:
    mmReportPayeeExpenses();
    virtual ~mmReportPayeeExpenses();

    virtual void RefreshData();
    virtual int report_parameters();
    virtual wxString getHTMLText();

protected:
    void getPayeeStats(std::map<int, std::pair<double, double> > &payeeStats
        , mmDateRange* date_range, bool ignoreFuture) const;

    enum TYPE {INCOME = 0, EXPENSES, MAX};

private:
    // structure for sorting of data
    struct data_holder { wxString color; wxString name; double incomes; double expenses; };
    std::vector<data_holder> data_;
    std::vector<ValueTrio> valueList_;
    double positiveTotal_;
    double negativeTotal_;
};

#endif //MM_EX_REPORTPAYEE_H_
