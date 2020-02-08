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

#include "wxcandlestickchart.h"
#include "wxchartstheme.h"
#include "wxchartscategoricalaxis.h"
#include "wxchartsnumericalaxis.h"
#include <wx/brush.h>
#include <wx/pen.h>
#include <sstream>

wxCandlestickChartData::wxCandlestickChartData(const wxVector<wxString> &labels,
                                               const wxVector<wxChartsOHLCData> &data)
    : m_labels(labels), m_lineColor(0, 0, 0, 0x80), m_lineWidth(3), 
    m_upFillColor(0, 205, 0, 0x60), m_downFillColor(225, 0, 0, 0x60), m_rectangleWidth(20), 
    m_data(data)
{
}

const wxVector<wxString>& wxCandlestickChartData::GetLabels() const
{
    return m_labels;
}

const wxColor& wxCandlestickChartData::GetLineColor() const
{
    return m_lineColor;
}

unsigned int wxCandlestickChartData::GetLineWidth() const
{
    return m_lineWidth;
}

const wxColor& wxCandlestickChartData::GetUpFillColor() const
{
    return m_upFillColor;
}

const wxColor& wxCandlestickChartData::GetDownFillColor() const
{
    return m_downFillColor;
}

unsigned int wxCandlestickChartData::GetRectangleWidth() const
{
    return m_rectangleWidth;
}

const wxVector<wxChartsOHLCData>& wxCandlestickChartData::GetData() const
{
    return m_data;
}

wxCandlestickChart::Candlestick::Candlestick(const wxChartsOHLCData &data,
                                             const wxColor &lineColor,
                                             unsigned int lineWidth,
                                             const wxColor &upFillColor,
                                             const wxColor &downFillColor,
                                             unsigned int rectangleWidth,
                                             const wxChartTooltipProvider::ptr tooltipProvider)
    : wxChartsElement(tooltipProvider), m_data(data), m_lowPoint(0, 0), m_highPoint(0, 0),
    m_openPoint(0, 0), m_closePoint(0, 0), m_lineColor(lineColor), m_lineWidth(lineWidth),
    m_upFillColor(upFillColor), m_downFillColor(downFillColor), m_rectangleWidth(rectangleWidth)
{
}

void wxCandlestickChart::Candlestick::Draw(wxGraphicsContext &gc) const
{
    if (m_data.GetCloseValue() >= m_data.GetOpenValue())
    {
        wxGraphicsPath path = gc.CreatePath();
        path.MoveToPoint(m_highPoint);
        path.AddLineToPoint(m_closePoint);

        wxDouble halfWidth = m_rectangleWidth / 2;
        wxDouble rectangleHeight = m_openPoint.m_y - m_closePoint.m_y;
        if (rectangleHeight < 2)
        {
            rectangleHeight = 2;
        }
        path.AddRectangle(m_closePoint.m_x - halfWidth, m_closePoint.m_y,
            m_rectangleWidth, rectangleHeight);

        path.MoveToPoint(m_openPoint);
        path.AddLineToPoint(m_lowPoint);

        wxBrush brush(m_upFillColor);
        gc.SetBrush(brush);
        gc.FillPath(path);

        wxPen pen(m_lineColor, m_lineWidth);
        gc.SetPen(pen);

        gc.StrokePath(path);
    }
    else
    {
        wxGraphicsPath path = gc.CreatePath();
        path.MoveToPoint(m_highPoint);
        path.AddLineToPoint(m_openPoint);

        wxDouble halfWidth = m_rectangleWidth / 2;
        wxDouble rectangleHeight = m_closePoint.m_y - m_openPoint.m_y;
        if (rectangleHeight < 2)
        {
            rectangleHeight = 2;
        }
        path.AddRectangle(m_openPoint.m_x - halfWidth, m_openPoint.m_y,
            m_rectangleWidth, rectangleHeight);

        path.MoveToPoint(m_closePoint);
        path.AddLineToPoint(m_lowPoint);

        wxBrush brush(m_downFillColor);
        gc.SetBrush(brush);
        gc.FillPath(path);

        wxPen pen(m_lineColor, m_lineWidth);
        gc.SetPen(pen);

        gc.StrokePath(path);
    }
}

bool wxCandlestickChart::Candlestick::HitTest(const wxPoint &point) const
{
    return ((point.y <= m_lowPoint.m_y) && (point.y >= m_highPoint.m_y) &&
        (point.x >= (m_lowPoint.m_x - (m_rectangleWidth / 2))) &&
        (point.x <= (m_lowPoint.m_x + (m_rectangleWidth / 2))));
}

wxPoint2DDouble wxCandlestickChart::Candlestick::GetTooltipPosition() const
{
    return wxPoint2DDouble(m_lowPoint.m_x, m_highPoint.m_y + (m_lowPoint.m_y - m_highPoint.m_y) / 2);
}

void wxCandlestickChart::Candlestick::Update(const wxChartsGridMapping& mapping,
    size_t index)
{
    m_lowPoint = mapping.GetWindowPositionAtTickMark(index, m_data.GetLowValue());
    m_highPoint = mapping.GetWindowPositionAtTickMark(index, m_data.GetHighValue());
    m_openPoint = mapping.GetWindowPositionAtTickMark(index, m_data.GetOpenValue());
    m_closePoint = mapping.GetWindowPositionAtTickMark(index, m_data.GetCloseValue());
}

wxCandlestickChart::wxCandlestickChart(const wxCandlestickChartData &data,
                                       const wxSize &size)
    : m_options(wxChartsDefaultTheme->GetCandlestickChartOptions()),
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

wxCandlestickChart::wxCandlestickChart(const wxCandlestickChartData &data,
                                       wxCandlestickChartOptions::ptr options,
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

void wxCandlestickChart::Initialize(const wxCandlestickChartData &data)
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

        Candlestick::ptr newCandlestick(new Candlestick(
            data.GetData()[i],
            data.GetLineColor(),
            data.GetLineWidth(),
            data.GetUpFillColor(),
            data.GetDownFillColor(),
            data.GetRectangleWidth(),
            tooltipProvider
        ));
        m_data.push_back(newCandlestick);
    }
}

const wxChartCommonOptions& wxCandlestickChart::GetCommonOptions() const
{
    return m_options->GetCommonOptions();
}

wxDouble wxCandlestickChart::GetMinValue(const wxCandlestickChartData &data)
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

wxDouble wxCandlestickChart::GetMaxValue(const wxCandlestickChartData &data)
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

void wxCandlestickChart::DoSetSize(const wxSize &size)
{
    wxSize newSize(
        size.GetWidth() - m_options->GetPadding().GetTotalHorizontalPadding(),
        size.GetHeight() - m_options->GetPadding().GetTotalVerticalPadding()
        );
    m_grid.Resize(newSize);
}

void wxCandlestickChart::DoFit()
{
    for (size_t i = 0; i < m_data.size(); ++i)
    {
        m_data[i]->Update(m_grid.GetMapping(), i);
    }
}

void wxCandlestickChart::DoDraw(wxGraphicsContext &gc,
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

wxSharedPtr<wxVector<const wxChartsElement*>> wxCandlestickChart::GetActiveElements(const wxPoint &point)
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
