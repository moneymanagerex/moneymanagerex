/*
    Copyright (c) 2016-2019 Xavier Leclercq

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
    IN THE SOFTWARE.
*/

#include "wxdoughnutchartctrl.h"

wxDoughnutChartCtrl::wxDoughnutChartCtrl(wxWindow *parent,
                                         wxWindowID id,
                                         wxPieChartData::ptr data,
                                         const wxPoint &pos,
                                         const wxSize &size,
                                         long style)
    : wxChartCtrl(parent, id, pos, size, style),
    m_doughnutChart(data, size)
{
    data->AddObserver(this);
}

wxDoughnutChartCtrl::wxDoughnutChartCtrl(wxWindow *parent,
                                         wxWindowID id,
                                         wxPieChartData::ptr data,
                                         wxSharedPtr<wxDoughnutChartOptions> &options,
                                         const wxPoint &pos,
                                         const wxSize &size,
                                         long style)
    : wxChartCtrl(parent, id, pos, size, style),
    m_doughnutChart(data, options, size)
{
    data->AddObserver(this);
}

wxDoughnutChart& wxDoughnutChartCtrl::GetChart()
{
    return m_doughnutChart;
}

void wxDoughnutChartCtrl::OnUpdate(const std::map<wxString, 
	                               wxChartSliceData> &data)
{
    m_doughnutChart.SetData(data);
    auto parent = this->GetParent();
    if(parent)
        parent->Layout();
}
