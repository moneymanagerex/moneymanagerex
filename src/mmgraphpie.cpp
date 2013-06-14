#include "mmgraphpie.h"
#include "chart.h"
#include "util.h"
#include <algorithm>

mmGraphPie::mmGraphPie() : 
        mmGraphGenerator(PIE),
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
    int othersum=0;
    std::vector<ChartData> pieData;
    pieData.reserve(cnt);

    for (size_t i = 0; i < cnt; ++i) 
        pieData.push_back(ChartData(valueList[i].label, valueList[i].amount));
    if (valueList.size()>14)
    {
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
