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

/*
    Part of this file were copied from the Chart.js project (http://chartjs.org/)
    and translated into C++.

    The files of the Chart.js project have the following copyright and license.

    Copyright (c) 2013-2016 Nick Downie
    Released under the MIT license
    https://github.com/nnnick/Chart.js/blob/master/LICENSE.md
*/

#include "wxradarchart.h"
#include "wxchartstheme.h"

wxRadarChartData::wxRadarChartData(const wxVector<wxString> &labels)
    : m_labels(labels)
{
}

const wxVector<wxString>& wxRadarChartData::GetLabels() const
{
    return m_labels;
};

wxRadarChart::wxRadarChart(const wxSize &size)
    : m_options(wxChartsDefaultTheme->GetRadarChartOptions()),
    m_grid(size, GetMinValue(), GetMaxValue(),
        m_options->GetGridOptions())
{
}

const wxChartCommonOptions& wxRadarChart::GetCommonOptions() const
{
    return m_options->GetCommonOptions();
}

wxDouble wxRadarChart::GetMinValue()
{
    wxDouble result = 0;
    return result;
}

wxDouble wxRadarChart::GetMaxValue()
{
    wxDouble result = 0;
    return result;
}

void wxRadarChart::DoSetSize(const wxSize &size)
{
    m_grid.Resize(size);
}

void wxRadarChart::DoFit()
{
}

void wxRadarChart::DoDraw(wxGraphicsContext &gc,
                          bool suppressTooltips)
{
    m_grid.Fit(gc);
    m_grid.Draw(gc);

    if (!suppressTooltips)
    {
        DrawTooltips(gc);
    }
}

wxSharedPtr<wxVector<const wxChartsElement*>> wxRadarChart::GetActiveElements(const wxPoint &point)
{
    wxSharedPtr<wxVector<const wxChartsElement*>> activeElements(new wxVector<const wxChartsElement*>());
    return activeElements;
}
