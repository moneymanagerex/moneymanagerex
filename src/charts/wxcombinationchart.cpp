/*
    Copyright (c) 2017-2019 Xavier Leclercq

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

#include "wxcombinationchart.h"
#include "wxchartscategoricalaxis.h"
#include "wxchartsnumericalaxis.h"

wxCombinationChart::wxCombinationChart()
{
}

void wxCombinationChart::AddColumnChart(const wxChartsCategoricalData &data)
{
    m_grid = new wxChartsGrid(
        wxPoint2DDouble(m_options.GetPadding().GetLeft(), m_options.GetPadding().GetRight()),
        wxSize(100, 100),
        wxChartsCategoricalAxis::make_shared("x", data.GetCategories(), wxChartsGridOptions().GetXAxisOptions()),
        wxChartsNumericalAxis::make_shared("y", 0, 200, wxChartsGridOptions().GetYAxisOptions()),
        wxChartsGridOptions()
    );
}

const wxChartCommonOptions& wxCombinationChart::GetCommonOptions() const
{
    return m_options.GetCommonOptions();
}

void wxCombinationChart::DoSetSize(const wxSize &size)
{
}

void wxCombinationChart::DoFit()
{
}

void wxCombinationChart::DoDraw(wxGraphicsContext &gc,
                                bool suppressTooltips)
{
    if (m_grid)
    {
        m_grid->Fit(gc);
        m_grid->Draw(gc);
    }

    if (!suppressTooltips)
    {
        DrawTooltips(gc);
    }
}

wxSharedPtr<wxVector<const wxChartsElement*>> wxCombinationChart::GetActiveElements(const wxPoint &point)
{
    wxSharedPtr<wxVector<const wxChartsElement*>> activeElements(new wxVector<const wxChartsElement*>());
    return activeElements;
}
