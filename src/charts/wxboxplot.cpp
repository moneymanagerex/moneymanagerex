/*
    Copyright (c) 2018-2019 Xavier Leclercq and the wxCharts contributors.

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

/// @file

#include "wxboxplot.h"
#include "wxchartstheme.h"
#include "wxchartscategoricalaxis.h"
#include "wxchartsnumericalaxis.h"
#include <wx/brush.h>
#include <wx/pen.h>
#include <sstream>
#include <algorithm>

wxBoxPlotData::wxBoxPlotData(const wxVector<wxString> &labels,
                             const wxVector<wxVector<wxDouble>> &data)
    : m_labels(labels), m_data(data)
{
    for (auto &vec : m_data)
    {
        std::sort(vec.begin(), vec.end());
    }
}

const wxVector<wxString>& wxBoxPlotData::GetLabels() const
{
    return m_labels;
}

const wxVector<wxVector<wxDouble>>& wxBoxPlotData::GetData() const
{
    return m_data;
}

wxBoxPlot::Box::Box(const wxVector<wxDouble> &data,
                    const wxColor &Color, const wxColor &upFillColor, unsigned int lineWidth,
                    unsigned int rectangleWidth, const wxChartTooltipProvider::ptr tooltipProvider)
    : wxChartsElement(tooltipProvider), m_data(data), m_lineColor(Color),
      m_upFillColor(upFillColor), m_width(lineWidth), m_rectangleWidth(rectangleWidth)
{
}

void wxBoxPlot::Box::Draw(wxGraphicsContext &gc) const
{
    wxGraphicsPath path = gc.CreatePath();

    path.MoveToPoint(m_minPoint);
    path.AddLineToPoint(m_q1Point);

    path.MoveToPoint(m_q3Point);
    path.AddLineToPoint(m_maxPoint);

    wxDouble width = m_rectangleWidth / 4;

    path.MoveToPoint(m_minPoint.m_x-width,m_minPoint.m_y);
    path.AddLineToPoint(m_minPoint.m_x+width,m_minPoint.m_y);

    path.MoveToPoint(m_maxPoint.m_x-width,m_maxPoint.m_y);
    path.AddLineToPoint(m_maxPoint.m_x+width,m_maxPoint.m_y);
    path.MoveToPoint(m_medianPoint.m_x-2*width,m_medianPoint.m_y);
    path.AddLineToPoint(m_medianPoint.m_x+2*width,m_medianPoint.m_y);

    wxDouble rectangleHeight = std::max(2.0,m_q1Point.m_y-m_q3Point.m_y);
    path.AddRectangle(m_q3Point.m_x-2*width,m_q3Point.m_y,m_rectangleWidth,rectangleHeight);

    wxBrush brush(m_upFillColor);
    gc.SetBrush(brush);
    gc.FillPath(path);

    wxPen pen(m_lineColor, m_width);
    gc.SetPen(pen);

    gc.StrokePath(path);
}

bool wxBoxPlot::Box::HitTest(const wxPoint &point) const
{
    return ((point.y <= m_minPoint.m_y) && (point.y >= m_maxPoint.m_y) &&
            (point.x >= (m_minPoint.m_x - (m_rectangleWidth / 2))) &&
            (point.x <= (m_minPoint.m_x + (m_rectangleWidth / 2))));
}

wxPoint2DDouble wxBoxPlot::Box::GetTooltipPosition() const
{
    return wxPoint2DDouble(m_minPoint.m_x, m_maxPoint.m_y + (m_minPoint.m_y - m_maxPoint.m_y) / 2);
}

void wxBoxPlot::Box::Update(const wxChartsGridMapping& mapping,
                            size_t index)
{
    auto len = m_data.size();
    m_minPoint = mapping.GetWindowPositionAtTickMark(index,
                 *m_data.begin());
    m_maxPoint = mapping.GetWindowPositionAtTickMark(index,
                 *m_data.rbegin());
    m_medianPoint = mapping.GetWindowPositionAtTickMark(index,
                GetMedian(m_data,0,len));
    m_q1Point = mapping.GetWindowPositionAtTickMark(index,
                GetMedian(m_data,0,len/2));
    m_q3Point = mapping.GetWindowPositionAtTickMark(index,
                GetMedian(m_data,len/2,m_data.size()));
}

wxBoxPlot::wxBoxPlot(const wxBoxPlotData &data,
                     const wxSize &size)
    : m_options(wxChartsDefaultTheme->GetBoxPlotOptions()),
    m_grid(
          wxPoint2DDouble(m_options->GetPadding().GetLeft(), m_options->GetPadding().GetTop()),
          size,
          wxChartsCategoricalAxis::make_shared("x", data.GetLabels(), m_options->GetGridOptions().GetXAxisOptions()),
          wxChartsNumericalAxis::make_shared("y", GetMinValue(data), GetMaxValue(data), m_options->GetGridOptions().GetYAxisOptions()),
          m_options->GetGridOptions()
      )
{
    Initialize(data);
}

wxBoxPlot::wxBoxPlot(const wxBoxPlotData &data,
                     wxBoxPlotOptions::ptr options,
                     const wxSize &size)
    : m_options(options),
    m_grid(
        wxPoint2DDouble(m_options->GetPadding().GetLeft(), m_options->GetPadding().GetTop()),
        size,
        wxChartsCategoricalAxis::make_shared("x", data.GetLabels(), m_options->GetGridOptions().GetXAxisOptions()),
        wxChartsNumericalAxis::make_shared("y", GetMinValue(data), GetMaxValue(data), m_options->GetGridOptions().GetYAxisOptions()),
        m_options->GetGridOptions()
    )
{
    Initialize(data);
}

const wxChartCommonOptions& wxBoxPlot::GetCommonOptions() const
{
    return m_options->GetCommonOptions();
}

void wxBoxPlot::Initialize(const wxBoxPlotData &data)
{
    for (size_t i = 0; i < data.GetData().size(); ++i)
    {
        wxSharedPtr<wxChartsDatasetTheme> datasetTheme = wxChartsDefaultTheme->GetDatasetTheme(wxChartsDatasetId::CreateImplicitId(i));
        wxSharedPtr<wxBoxPlotDatasetOptions> datasetOptions = datasetTheme->GetBoxPlotDatasetOptions();

        auto cur = data.GetData()[i];
        auto len = cur.size();

        std::stringstream tooltip;
        tooltip << "Max: " << *cur.begin()
            << "\r\nMin: " << *cur.rbegin()
            << "\r\nMedian: " << GetMedian(cur, 0, len)
            << "\r\nQ1: " << GetMedian(cur, 0, len / 2)
            << "\r\nQ3: " << GetMedian(cur, len / 2, cur.size());
        wxChartTooltipProvider::ptr tooltipProvider(
            new wxChartTooltipProviderStatic(data.GetLabels()[i], tooltip.str(), *wxWHITE)
        );

        Box::ptr newBox(new Box(
            cur,
            datasetOptions->GetLineColor(),
            datasetOptions->GetUpFillColor(),
            datasetOptions->GetWidth(),
            datasetOptions->GetRectangleWidth(),
            tooltipProvider
        ));
        m_data.push_back(newBox);
    }
}

wxDouble wxBoxPlot::GetMinValue(const wxBoxPlotData &data)
{
    wxDouble result = 0;
    for(const auto &vec : data.GetData())
    {
        auto it = std::min_element(vec.begin(),vec.end());
        if (it != vec.end() && *it < result)
        {
            result = *it;
        }
    }
    return result;
}

wxDouble wxBoxPlot::GetMaxValue(const wxBoxPlotData &data)
{
    wxDouble result = 0;
    for(const auto &vec : data.GetData())
    {
        auto it = std::max_element(vec.begin(),vec.end());
        if (it != vec.end() && *it > result)
        {
            result = *it;
        }
    }
    return result;
}

wxDouble wxBoxPlot::GetMedian(const wxVector<wxDouble> &data,
                              size_t begin,
                              size_t end)
{
    auto len = end - begin;
    if (len % 2 == 0)
    {
        return (data[begin + len / 2] + data[begin + len / 2 - 1]) / 2;
    }
    else
    {
        return data[begin + len / 2];
    }
}


void wxBoxPlot::DoSetSize(const wxSize &size)
{
    wxSize newSize(
        size.GetWidth() - m_options->GetPadding().GetTotalHorizontalPadding(),
        size.GetHeight() - m_options->GetPadding().GetTotalVerticalPadding()
    );
    m_grid.Resize(newSize);
}

void wxBoxPlot::DoFit()
{
    for (size_t i = 0; i < m_data.size(); ++i)
    {
        m_data[i]->Update(m_grid.GetMapping(), i);
    }
}

void wxBoxPlot::DoDraw(wxGraphicsContext &gc,
                       bool suppressTooltips)
{
    m_grid.Fit(gc);
    m_grid.Draw(gc);

    Fit();

    for (size_t i = 0; i < m_data.size(); ++i)
    {
        m_data[i]->Draw(gc);
    }

    if (!suppressTooltips)
    {
        DrawTooltips(gc);
    }
}

wxSharedPtr<wxVector<const wxChartsElement*>> wxBoxPlot::GetActiveElements(const wxPoint &point)
{
    wxSharedPtr<wxVector<const wxChartsElement*>> activeElements(new wxVector<const wxChartsElement*>());
    for (size_t i = 0; i < m_data.size(); ++i)
    {
        if (m_data[i]->HitTest(point))
        {
            activeElements->push_back(m_data[i].get());
        }
    }
    return activeElements;
}
