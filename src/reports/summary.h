/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel

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

#ifndef _MM_EX_REPORTSUMMARY_H_
#define _MM_EX_REPORTSUMMARY_H_

#include "reportbase.h"

#if 0
#include <vector>

// structure for sorting of data
struct summary_data_holder { wxString name; wxString link; double balance; };

class mmReportSummary : public mmPrintableBase
{
public:
    mmReportSummary();
    virtual void RefreshData();
    virtual wxString getHTMLText();

private:
    std::vector<summary_data_holder> dataChecking_;
    std::vector<summary_data_holder> dataTerm_;
    double tBalance_;
    double tTBalance_;
    double stockBalance_;
    double asset_balance_;
    double totalBalance_;
};
#endif

class mmReportSummaryByDate : public mmPrintableBase
{
public:
    mmReportSummaryByDate(int mode);
    wxString getHTMLText();

private:
    int mode_;

    typedef std::map<wxDate, double> balanceMap;
};

#endif //_MM_EX_REPORTSUMMARY_H_
