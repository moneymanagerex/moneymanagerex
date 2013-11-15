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

#include "mmgraphpie.h"
#include "chart.h"
#include "util.h"
#include <algorithm>

mmGraphPie::mmGraphPie() : 
        mmGraphGenerator(),
        chart_(new PieChart(480, 320))
{
}

mmGraphPie::~mmGraphPie()
{
    if (chart_) delete chart_;
}

void mmGraphPie::init(std::vector<ValuePair>& valueList)
{
    std::sort(valueList.begin(), valueList.end(), [](const ValuePair& x, const ValuePair& y){ return fabs(x.amount) > fabs(y.amount);});
    size_t cnt = std::min(valueList.size(), size_t(14));
    std::vector<ChartData> pieData;
    pieData.reserve(cnt);

    for (size_t i = 0; i < cnt; ++i) 
        pieData.push_back(ChartData(valueList[i].label, valueList[i].amount));
    if (valueList.size()>14)
    {
        int othersum=0;
        for (size_t j=14; j<valueList.size(); j++)
             othersum = othersum+valueList[j].amount;
        pieData.push_back(ChartData(_("All Others"), othersum));
    }

    chart_->SetData(pieData);
    chart_->Init(240, CHART_LEGEND_FIXED, PIE_CHART_ABSOLUT);
}

bool mmGraphPie::Generate(const wxString& chartTitle)
{
    chart_->Render(chartTitle);
    return chart_->Save(getOutputFileName());
}
