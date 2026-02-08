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

class ReportFlowByPayee : public ReportBase
{
private:
    struct Data
    {
        wxString payee_name;
        double flow_pos;
       double flow_neg;
        double flow;

        Data();
    };

private:
    std::map<int64, Data> m_id_data;
    std::vector<int64> m_order_net_flow;
    std::vector<int64> m_order_abs_flow;
    Data m_total;

public:
    ReportFlowByPayee();
    virtual ~ReportFlowByPayee();

public:
    virtual void refreshData();
    virtual wxString getHTMLText();

private:
    static void updateData(Data& data, Model_Checking::TYPE_ID type_id, double amount);

private:
    void loadData(mmDateRange* date_range, bool ignoreFuture);
};

