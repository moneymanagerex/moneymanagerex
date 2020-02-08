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

#include "wxpolarareachart.h"
#include "wxchartstheme.h"

wxPolarAreaChartData::wxPolarAreaChartData()
{
}

void wxPolarAreaChartData::AppendSlice(const wxChartSliceData &slice)
{
    m_slices.push_back(slice);
}

const wxVector<wxChartSliceData>& wxPolarAreaChartData::GetSlices() const
{
    return m_slices;
}

wxPolarAreaChart::SliceArc::SliceArc(const wxChartSliceData &slice,
                                     wxDouble x,
                                     wxDouble y,
                                     wxDouble startAngle,
                                     wxDouble endAngle,
                                     wxDouble radius)
    : wxChartsArc(x, y, startAngle, endAngle, radius, 0,
        slice.GetTooltipText(), wxChartsArcOptions(2, slice.GetColor())),
    m_value(slice.GetValue())
{
}

void wxPolarAreaChart::SliceArc::Resize(const wxSize &size)
{
    wxDouble x = (size.GetX() / 2);
    wxDouble y = (size.GetY() / 2);
    wxDouble radius = ((x < y) ? x : y);

    SetCenter(x, y);
}

wxDouble wxPolarAreaChart::SliceArc::GetValue() const
{
    return m_value;
}

wxPolarAreaChart::wxPolarAreaChart(const wxPolarAreaChartData &data,
                                   const wxSize &size)
    : m_options(wxChartsDefaultTheme->GetPolarAreaChartOptions()),
    m_grid(size, GetMinValue(data.GetSlices()), GetMaxValue(data.GetSlices()),
        m_options->GetGridOptions())
{
    const wxVector<wxChartSliceData>& slices = data.GetSlices();
    for (size_t i = 0; i < slices.size(); ++i)
    {
        Add(slices[i], size);
    }
}

wxPolarAreaChart::wxPolarAreaChart(const wxPolarAreaChartData &data,
                                   wxSharedPtr<wxPolarAreaChartOptions> &options,
                                   const wxSize &size)
    : m_options(options),
    m_grid(size, GetMinValue(data.GetSlices()), GetMaxValue(data.GetSlices()),
        m_options->GetGridOptions())
{
    const wxVector<wxChartSliceData>& slices = data.GetSlices();
    for (size_t i = 0; i < slices.size(); ++i)
    {
        Add(slices[i], size);
    }
}

const wxChartCommonOptions& wxPolarAreaChart::GetCommonOptions() const
{
    return m_options->GetCommonOptions();
}

void wxPolarAreaChart::Add(const wxChartSliceData &slice,
                           const wxSize &size)
{
    Add(slice, m_slices.size(), size);
}

void wxPolarAreaChart::Add(const wxChartSliceData &slice, 
                           size_t index,
                           const wxSize &size)
{
    wxDouble x = (size.GetX() / 2);
    wxDouble y = (size.GetY() / 2);
    wxDouble radius = ((x < y) ? x : y);

    SliceArc::ptr newSlice = SliceArc::ptr(new SliceArc(slice,
        x, y, 0, 0, radius));
    m_slices.insert(m_slices.begin() + index, newSlice);
}

wxDouble wxPolarAreaChart::GetMinValue(const wxVector<wxChartSliceData> &slices)
{
    wxDouble result = 0;
    if (slices.size() > 0)
    {
        result = slices[0].GetValue();
    }
    for (size_t i = 1; i < slices.size(); ++i)
    {
        wxDouble v = slices[i].GetValue();
        if (v < result)
        {
            result = v;
        }
    }
    return result;
}

wxDouble wxPolarAreaChart::GetMaxValue(const wxVector<wxChartSliceData> &slices)
{
    wxDouble result = 0;
    if (slices.size() > 0)
    {
        result = slices[0].GetValue();
    }
    for (size_t i = 1; i < slices.size(); ++i)
    {
        wxDouble v = slices[i].GetValue();
        if (v > result)
        {
            result = v;
        }
    }
    return result;
}

void wxPolarAreaChart::DoSetSize(const wxSize &size)
{
    wxSize newSize(
        size.GetWidth() - m_options->GetPadding().GetTotalHorizontalPadding(),
        size.GetHeight() - m_options->GetPadding().GetTotalVerticalPadding()
        );

    m_grid.Resize(newSize);
    for (size_t i = 0; i < m_slices.size(); ++i)
    {
        m_slices[i]->Resize(newSize);
    }
}

void wxPolarAreaChart::DoFit()
{
    wxDouble startAngle = m_options->GetStartAngle();
    wxDouble angleIncrement = ((2 * M_PI) / m_slices.size());

    for (size_t i = 0; i < m_slices.size(); ++i)
    {
        SliceArc& currentSlice = *m_slices[i];

        wxDouble endAngle = startAngle + angleIncrement;
        currentSlice.SetAngles(startAngle, endAngle);
        currentSlice.SetRadiuses(m_grid.GetRadius(currentSlice.GetValue()), 0);
        startAngle = endAngle;
    }
}

void wxPolarAreaChart::DoDraw(wxGraphicsContext &gc,
                              bool suppressTooltips)
{
    Fit();

    for (size_t i = 0; i < m_slices.size(); ++i)
    {
        m_slices[i]->Draw(gc);
    }

    m_grid.Fit(gc);
    m_grid.Draw(gc);

    if (!suppressTooltips)
    {
        DrawTooltips(gc);
    }
}

wxSharedPtr<wxVector<const wxChartsElement*>> wxPolarAreaChart::GetActiveElements(const wxPoint &point)
{
    wxSharedPtr<wxVector<const wxChartsElement*>> activeElements(new wxVector<const wxChartsElement*>());
    for (size_t i = 0; i < m_slices.size(); ++i)
    {
        if (m_slices[i]->HitTest(point))
        {
            activeElements->push_back(m_slices[i].get());
        }
    }
    return activeElements;
}
