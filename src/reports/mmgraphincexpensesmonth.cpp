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

#include "mmgraphincexpensesmonth.h"
#include "chart.h"
#include <vector>

mmGraphIncExpensesMonth::mmGraphIncExpensesMonth() : 
        mmGraphGenerator(),
        chart_(new BarChart(200, 240))
{
}

mmGraphIncExpensesMonth::~mmGraphIncExpensesMonth()
{
    delete chart_;
}

void mmGraphIncExpensesMonth::init(double income, double expenses)
{
    std::vector<ChartData> barData;

    barData.push_back(ChartData(_("Income"), income));
    barData.push_back(ChartData(_("Expenses"), expenses));

    chart_->SetData(barData);
    chart_->Init(140, CHART_LEGEND_FLOAT, BAR_CHART_SIMPLE);
}

bool mmGraphIncExpensesMonth::Generate(const wxString& chartTitle)
{
    chart_->Render(chartTitle);
    return chart_->Save(getOutputFileName());
}
