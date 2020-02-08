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

/// @file

#include "wxohlcchart.h"
#include "wxchartstheme.h"
#include "wxchartscategoricalaxis.h"
#include "wxchartsnumericalaxis.h"
#include <wx/brush.h>
#include <wx/pen.h>
#include <sstream>

wxOHLCChartData::wxOHLCChartData(const wxVector<wxString> &labels,
    const wxVector<wxChartsOHLCData> &data)
    : m_labels(labels), m_lineWidth(3),
    m_upLineColor(0, 185, 0), m_downLineColor(200, 0, 0),
    m_openLineLength(10), m_closeLineLength(10), m_data(data)
{
}

const wxVector<wxString>& wxOHLCChartData::GetLabels() const
{
    return m_labels;
}

unsigned int wxOHLCChartData::GetLineWidth() const
{
    return m_lineWidth;
}

const wxColor& wxOHLCChartData::GetUpLineColor() const
{
    return m_upLineColor;
}

const wxColor& wxOHLCChartData::GetDownLineColor() const
{
    return m_downLineColor;
}

unsigned int wxOHLCChartData::GetOpenLineLength() const
{
    return m_openLineLength;
}

unsigned int wxOHLCChartData::GetCloseLineLength() const
{
    return m_closeLineLength;
}

const wxVector<wxChartsOHLCData>& wxOHLCChartData::GetData() const
{
    return m_data;
}

wxOHLCChart::OHLCLines::OHLCLines(const wxChartsOHLCData &data,
                                  unsigned int lineWidth,
                                  const wxColor& upLineColor,
                                  const wxColor& downLineColor,
                                  unsigned int openLineLength,
                                  unsigned int closeLineLength,
                                  const wxChartTooltipProvider::ptr tooltipProvider)
    : wxChartsElement(tooltipProvider), m_data(data), m_lowPoint(0, 0), m_highPoint(0, 0),
    m_openPoint(0, 0), m_closePoint(0, 0), m_lineWidth(lineWidth),
    m_upLineColor(upLineColor), m_downLineColor(downLineColor),
    m_openLineLength(openLineLength), m_closeLineLength(closeLineLength)
{
}

void wxOHLCChart::OHLCLines::Draw(wxGraphicsContext &gc) const
{
    wxGraphicsPath path = gc.CreatePath();
    path.MoveToPoint(m_highPoint);
    path.AddLineToPoint(m_lowPoint);

    path.MoveToPoint(wxPoint2DDouble(m_openPoint.m_x - m_openLineLength, m_openPoint.m_y));
    path.AddLineToPoint(m_openPoint);

    path.MoveToPoint(m_closePoint);
    path.AddLineToPoint(wxPoint2DDouble(m_closePoint.m_x + m_closeLineLength, m_closePoint.m_y));

    if (m_data.GetCloseValue() >= m_data.GetOpenValue())
    {
        wxPen pen(m_upLineColor, m_lineWidth);
        gc.SetPen(pen);
    }
    else
    {
        wxPen pen(m_downLineColor, m_lineWidth);
        gc.SetPen(pen);
    }
    gc.StrokePath(path);
}

bool wxOHLCChart::OHLCLines::HitTest(const wxPoint &point) const
{
    return ((point.y <= m_lowPoint.m_y) && (point.y >= m_highPoint.m_y) &&
        (point.x >= (m_lowPoint.m_x - m_openLineLength)) &&
        (point.x <= (m_lowPoint.m_x + m_closeLineLength)));
}

wxPoint2DDouble wxOHLCChart::OHLCLines::GetTooltipPosition() const
{
    return wxPoint2DDouble(m_lowPoint.m_x, m_highPoint.m_y + (m_lowPoint.m_y - m_highPoint.m_y) / 2);
}

void wxOHLCChart::OHLCLines::Update(const wxChartsGridMapping& mapping,
                                    size_t index)
{
    m_lowPoint = mapping.GetWindowPositionAtTickMark(index, m_data.GetLowValue());
    m_highPoint = mapping.GetWindowPositionAtTickMark(index, m_data.GetHighValue());
    m_openPoint = mapping.GetWindowPositionAtTickMark(index, m_data.GetOpenValue());
    m_closePoint = mapping.GetWindowPositionAtTickMark(index, m_data.GetCloseValue());
}

wxOHLCChart::wxOHLCChart(const wxOHLCChartData &data,
                         const wxSize &size)
    : m_options(wxChartsDefaultTheme->GetOHLCChartOptions()),
    m_grid(
        wxPoint2DDouble(m_options->GetPadding().GetLeft(), m_options->GetPadding().GetTop()),
        size,
        wxChartsCategoricalAxis::make_shared("x", data.GetLabels(), m_options->GetGridOptions().GetXAxisOptions()),
        wxChartsNumericalAxis::make_shared("y", GetMinValue(data), GetMaxValue(data), m_options->GetGridOptions().GetYAxisOptions()),
        m_options->GetGridOptions()
        )
{
    for (size_t i = 0; i < data.GetData().size(); ++i)
    {
        std::stringstream tooltip;
        tooltip << "O: " << data.GetData()[i].GetOpenValue()
            << "\r\nH: " << data.GetData()[i].GetHighValue()
            << "\r\nL: " << data.GetData()[i].GetLowValue()
            << "\r\nC: " << data.GetData()[i].GetCloseValue();
        wxChartTooltipProvider::ptr tooltipProvider(
            new wxChartTooltipProviderStatic(data.GetLabels()[i], tooltip.str(), *wxWHITE)
            );

        OHLCLines::ptr newOHLCLines(new OHLCLines(
            data.GetData()[i],
            data.GetLineWidth(),
            data.GetUpLineColor(),
            data.GetDownLineColor(),
            data.GetOpenLineLength(),
            data.GetCloseLineLength(),
            tooltipProvider
            ));
        m_data.push_back(newOHLCLines);
    }
}

const wxChartCommonOptions& wxOHLCChart::GetCommonOptions() const
{
    return m_options->GetCommonOptions();
}

wxDouble wxOHLCChart::GetMinValue(const wxOHLCChartData &data)
{
    wxDouble result = 0;
    bool foundValue = false;

    for (size_t i = 0; i < data.GetData().size(); ++i)
    {
        if (!foundValue)
        {
            result = data.GetData()[i].GetLowValue();
            foundValue = true;
        }
        else if (result > data.GetData()[i].GetLowValue())
        {
            result = data.GetData()[i].GetLowValue();
        }
    }

    return result;
}

wxDouble wxOHLCChart::GetMaxValue(const wxOHLCChartData &data)
{
    wxDouble result = 0;
    bool foundValue = false;

    for (size_t i = 0; i < data.GetData().size(); ++i)
    {
        if (!foundValue)
        {
            result = data.GetData()[i].GetHighValue();
            foundValue = true;
        }
        else if (result < data.GetData()[i].GetHighValue())
        {
            result = data.GetData()[i].GetHighValue();
        }
    }

    return result;
}

void wxOHLCChart::DoSetSize(const wxSize &size)
{
    wxSize newSize(
        size.GetWidth() - m_options->GetPadding().GetTotalHorizontalPadding(),
        size.GetHeight() - m_options->GetPadding().GetTotalVerticalPadding()
        );
    m_grid.Resize(newSize);
}

void wxOHLCChart::DoFit()
{
    for (size_t i = 0; i < m_data.size(); ++i)
    {
        m_data[i]->Update(m_grid.GetMapping(), i);
    }
}

void wxOHLCChart::DoDraw(wxGraphicsContext &gc,
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

wxSharedPtr<wxVector<const wxChartsElement*>> wxOHLCChart::GetActiveElements(const wxPoint &point)
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
