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
