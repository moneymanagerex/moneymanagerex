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

#include "wxtimeserieschart.h"
#include "wxchartstheme.h"

wxTimeSeriesChart::wxTimeSeriesChart(const wxSize &size)
    : m_options(wxChartsDefaultTheme->GetTimeSeriesChartOptions())
{
}

wxTimeSeriesChart::wxTimeSeriesChart(wxSharedPtr<wxTimeSeriesChartOptions> &options,
                                     const wxSize &size)
    : m_options(options)
{
}

const wxChartCommonOptions& wxTimeSeriesChart::GetCommonOptions() const
{
    return m_options->GetCommonOptions();
}

void wxTimeSeriesChart::DoSetSize(const wxSize &size)
{
}

void wxTimeSeriesChart::DoFit()
{
}

void wxTimeSeriesChart::DoDraw(wxGraphicsContext &gc,
                               bool suppressTooltips)
{
    if (!suppressTooltips)
    {
        DrawTooltips(gc);
    }
}

wxSharedPtr<wxVector<const wxChartsElement*>> wxTimeSeriesChart::GetActiveElements(const wxPoint &point)
{
    wxSharedPtr<wxVector<const wxChartsElement*>> activeElements(new wxVector<const wxChartsElement*>());
    return activeElements;
}
