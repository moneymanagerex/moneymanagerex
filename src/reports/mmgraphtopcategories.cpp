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

//----------------------------------------------------------------------------
#include "mmgraphtopcategories.h"
#include "chart.h"
#include <algorithm>
//----------------------------------------------------------------------------

mmGraphTopCategories::mmGraphTopCategories() : 
        mmGraphGenerator(),
        m_chart(new BarChart(320, 240))
{
}
//----------------------------------------------------------------------------

mmGraphTopCategories::~mmGraphTopCategories()
{
    if (m_chart) delete m_chart;
}
//----------------------------------------------------------------------------

void mmGraphTopCategories::init(const std::vector<CategInfo> &categList)
{
    const size_t max_cnt = std::min(categList.size(), size_t(10));

    std::vector<ChartData> barData;
    barData.reserve(max_cnt);

    for (size_t i = 0; i < max_cnt; ++i) 
    {
        if (categList[i].amount < 0) // show withdrawal only
            barData.push_back(ChartData(categList[i].categ, -categList[i].amount));
    }

    m_chart->SetData(barData);
    m_chart->Init(140, CHART_LEGEND_FIXED, BAR_CHART_SIMPLE);
}
//----------------------------------------------------------------------------

bool mmGraphTopCategories::Generate(const wxString& chartTitle)
{
    m_chart->Render(chartTitle);
    return m_chart->Save(getOutputFileName());
}
//----------------------------------------------------------------------------
