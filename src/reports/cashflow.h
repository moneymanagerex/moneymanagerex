/*******************************************************
Copyright (C) 2006-2012
Copyright (C) 2017 James Higley

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

#ifndef MM_EX_REPORTCASHFLOW_H_
#define MM_EX_REPORTCASHFLOW_H_

#include "reportbase.h"
#include <vector>
#include "util.h"

class mmReportCashFlow : public mmPrintableBase
{
public:
    enum TYPE { MONTHLY = 0, DAILY };
    explicit mmReportCashFlow(TYPE cashflowreporttype);
    virtual ~mmReportCashFlow();

    virtual wxString getHTMLText();
    virtual int report_parameters();

protected:
    wxString getHTMLText_i();
    void getStats(double& tInitialBalance, std::vector<ValueTrio>& forecastVector);

protected:
    struct mmRepeatForecast
    {
        wxDateTime date;
        double amount;
    };

    typedef std::vector<mmRepeatForecast> forecastVec;
    std::vector<forecastVec> bdForecastVec;

    TYPE cashFlowReportType_;

    static const int yearsNum_ = 10;
    const wxDateTime today_;
    int colorId_;

};

#endif // MM_EX_REPORTCASHFLOW_H_
