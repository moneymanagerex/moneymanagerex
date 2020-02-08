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

/*
    Part of this file were copied from the Chart.js project (http://chartjs.org/)
    and translated into C++.

    The files of the Chart.js project have the following copyright and license.

    Copyright (c) 2013-2017 Nick Downie
    Released under the MIT license
    https://github.com/nnnick/Chart.js/blob/master/LICENSE.md
*/

/// @file

#include "wxareachart.h"
#include "wxchartstheme.h"
#include <wx/dcmemory.h>
#include <sstream>

wxAreaChartDataset::wxAreaChartDataset(wxVector<wxPoint2DDouble> &data)
    : m_data(data)
{
}

const wxVector<wxPoint2DDouble>& wxAreaChartDataset::GetData() const
{
    return m_data;
}

wxAreaChartData::wxAreaChartData()
{
}

void wxAreaChartData::AddDataset(wxAreaChartDataset::ptr dataset)
{
    m_datasets.push_back(dataset);
}

const wxVector<wxAreaChartDataset::ptr>& wxAreaChartData::GetDatasets() const
{
    return m_datasets;
}

wxAreaChart::Point::Point(
    wxPoint2DDouble value,
    const wxChartTooltipProvider::ptr tooltipProvider,
    wxDouble x,
    wxDouble y,
    wxDouble radius,
    unsigned int strokeWidth,
    const wxColor &fillColor,
    wxDouble hitDetectionRange)
    : wxChartsPoint(x, y, radius, tooltipProvider, wxChartsPointOptions(strokeWidth,fillColor,fillColor)),
    m_value(value), m_hitDetectionRange(hitDetectionRange)
{
}

wxPoint2DDouble wxAreaChart::Point::GetTooltipPosition() const
{
    wxPoint2DDouble position = wxChartsPoint::GetTooltipPosition();
    position.m_y -= 10;
    return position;
}

bool wxAreaChart::Point::HitTest(const wxPoint &point) const
{
    wxDouble distance = (point.x - GetPosition().m_x);
    if (distance < 0)
    {
        distance = -distance;
    }
    return (distance < m_hitDetectionRange);
}

wxPoint2DDouble wxAreaChart::Point::GetValue() const
{
    return m_value;
}

wxAreaChart::Dataset::Dataset(bool showDots,
    bool showLine,
    const wxColor &lineColor)
    : m_showDots(showDots), m_showLine(showLine),
    m_lineColor(lineColor)
{
}

bool wxAreaChart::Dataset::ShowDots() const
{
    return m_showDots;
}

bool wxAreaChart::Dataset::ShowLine() const
{
    return m_showLine;
}

const wxColor& wxAreaChart::Dataset::GetLineColor() const
{
    return m_lineColor;
}

const wxVector<wxAreaChart::Point::ptr>& wxAreaChart::Dataset::GetPoints() const
{
    return m_points;
}

void wxAreaChart::Dataset::AppendPoint(Point::ptr point)
{
    m_points.push_back(point);
}

wxAreaChart::wxAreaChart(const wxAreaChartData &data,
                         const wxSize &size)
    : m_options(wxChartsDefaultTheme->GetAreaChartOptions()),
    m_grid(
        wxPoint2DDouble(m_options->GetPadding().GetLeft(), m_options->GetPadding().GetRight()),
        size,
        GetMinXValue(data.GetDatasets()), GetMaxXValue(data.GetDatasets()),
        GetMinYValue(data.GetDatasets()), GetMaxYValue(data.GetDatasets()),
        m_options->GetGridOptions())
{
    Initialize(data);
}

wxAreaChart::wxAreaChart(const wxAreaChartData &data,
                         wxAreaChartOptions::ptr &options,
                         const wxSize &size)
    : m_options(options),
    m_grid(
        wxPoint2DDouble(m_options->GetPadding().GetLeft(), m_options->GetPadding().GetRight()),
        size,
        GetMinXValue(data.GetDatasets()), GetMaxXValue(data.GetDatasets()),
        GetMinYValue(data.GetDatasets()), GetMaxYValue(data.GetDatasets()),
        m_options->GetGridOptions())
{
    Initialize(data);
}

const wxChartCommonOptions& wxAreaChart::GetCommonOptions() const
{
    return m_options->GetCommonOptions();
}

void wxAreaChart::Save(const wxString &filename,
                       const wxBitmapType &type,
                       const wxSize &size)
{
    wxBitmap bmp(size.GetWidth(), size.GetHeight());
    wxMemoryDC mdc(bmp);
    mdc.Clear();
    wxGraphicsContext* gc = wxGraphicsContext::Create(mdc);
    if (gc)
    {
        DoDraw(*gc, true);
        bmp.SaveFile(filename, type);
        delete gc;
    }
}

void wxAreaChart::Initialize(const wxAreaChartData &data)
{
    const wxVector<wxAreaChartDataset::ptr>& datasets = data.GetDatasets();
    for (size_t i = 0; i < datasets.size(); ++i)
    {
        wxSharedPtr<wxChartsDatasetTheme> datasetTheme = wxChartsDefaultTheme->GetDatasetTheme(wxChartsDatasetId::CreateImplicitId(i));
        wxSharedPtr<wxAreaChartDatasetOptions> datasetOptions = datasetTheme->GetAreaChartDatasetOptions();

        Dataset::ptr newDataset(new Dataset(datasetOptions->ShowDots(), datasetOptions->ShowLine(),
            datasetOptions->GetLineColor()));

        const wxVector<wxPoint2DDouble>& datasetData = datasets[i]->GetData();
        for (size_t j = 0; j < datasetData.size(); ++j)
        {
            std::stringstream tooltip;
            tooltip << "(" << datasetData[j].m_x << "," << datasetData[j].m_y << ")";
            wxChartTooltipProvider::ptr tooltipProvider(
                new wxChartTooltipProviderStatic("", tooltip.str(), datasetOptions->GetLineColor())
                );

            Point::ptr point(
                new Point(datasetData[j], tooltipProvider, 20 + j * 10, 0,
                    m_options->GetDotRadius(), m_options->GetDotStrokeWidth(),
                    datasetOptions->GetDotColor(),m_options->GetHitDetectionRange()));

            newDataset->AppendPoint(point);
        }

        m_datasets.push_back(newDataset);
    }
}

wxDouble wxAreaChart::GetMinXValue(const wxVector<wxAreaChartDataset::ptr>& datasets)
{
    wxDouble result = 0;
    bool foundValue = false;

    for (size_t i = 0; i < datasets.size(); ++i)
    {
        const wxVector<wxPoint2DDouble>& values = datasets[i]->GetData();
        for (size_t j = 0; j < values.size(); ++j)
        {
            if (!foundValue)
            {
                result = values[j].m_x;
                foundValue = true;
            }
            else if (result > values[j].m_x)
            {
                result = values[j].m_x;
            }
        }
    }

    return result;
}

wxDouble wxAreaChart::GetMaxXValue(const wxVector<wxAreaChartDataset::ptr>& datasets)
{
    wxDouble result = 0;
    bool foundValue = false;

    for (size_t i = 0; i < datasets.size(); ++i)
    {
        const wxVector<wxPoint2DDouble>& values = datasets[i]->GetData();
        for (size_t j = 0; j < values.size(); ++j)
        {
            if (!foundValue)
            {
                result = values[j].m_x;
                foundValue = true;
            }
            else if (result < values[j].m_x)
            {
                result = values[j].m_x;
            }
        }
    }

    return result;
}

wxDouble wxAreaChart::GetMinYValue(const wxVector<wxAreaChartDataset::ptr>& datasets)
{
    wxDouble result = 0;
    bool foundValue = false;

    for (size_t i = 0; i < datasets.size(); ++i)
    {
        const wxVector<wxPoint2DDouble>& values = datasets[i]->GetData();
        for (size_t j = 0; j < values.size(); ++j)
        {
            if (!foundValue)
            {
                result = values[j].m_y;
                foundValue = true;
            }
            else if (result > values[j].m_y)
            {
                result = values[j].m_y;
            }
        }
    }

    return result;
}

wxDouble wxAreaChart::GetMaxYValue(const wxVector<wxAreaChartDataset::ptr>& datasets)
{
    wxDouble result = 0;
    bool foundValue = false;

    for (size_t i = 0; i < datasets.size(); ++i)
    {
        const wxVector<wxPoint2DDouble>& values = datasets[i]->GetData();
        for (size_t j = 0; j < values.size(); ++j)
        {
            if (!foundValue)
            {
                result = values[j].m_y;
                foundValue = true;
            }
            else if (result < values[j].m_y)
            {
                result = values[j].m_y;
            }
        }
    }

    return result;
}

void wxAreaChart::DoSetSize(const wxSize &size)
{
    wxSize newSize(
        size.GetWidth() - m_options->GetPadding().GetTotalHorizontalPadding(),
        size.GetHeight() - m_options->GetPadding().GetTotalVerticalPadding()
        );
    m_grid.Resize(newSize);
}

void wxAreaChart::DoFit()
{
    for (size_t i = 0; i < m_datasets.size(); ++i)
    {
        const wxVector<Point::ptr>& points = m_datasets[i]->GetPoints();
        for (size_t j = 0; j < points.size(); ++j)
        {
            const Point::ptr& point = points[j];
            point->SetPosition(m_grid.GetMapping().GetWindowPosition(point->GetValue().m_x, point->GetValue().m_y));
        }
    }
}

void wxAreaChart::DoDraw(wxGraphicsContext &gc,
                         bool suppressTooltips)
{
    m_grid.Fit(gc);
    m_grid.Draw(gc);
    Fit();

    for (size_t i = 0; i < m_datasets.size(); ++i)
    {
        const wxVector<Point::ptr>& points = m_datasets[i]->GetPoints();

        wxGraphicsPath path = gc.CreatePath();

        if (points.size() > 0)
        {
            auto value = points[0]->GetValue();
            wxPoint2DDouble firstPosition = m_grid.GetMapping().GetWindowPosition(value.m_x,value.m_y );
            path.MoveToPoint(firstPosition);

            wxPoint2DDouble lastPosition;
            for (size_t j = 1; j < points.size(); ++j)
            {
                value = points[j]->GetValue();
                lastPosition =  m_grid.GetMapping().GetWindowPosition(value.m_x,value.m_y );
                path.AddLineToPoint(lastPosition);
            }
            path.AddLineToPoint(firstPosition);

            wxBrush brush(m_datasets[i]->GetLineColor());
            gc.SetBrush(brush);
            gc.FillPath(path);
            gc.StrokePath(path);
        }

        if (m_datasets[i]->ShowDots())
        {
            for (size_t j = 0; j < points.size(); ++j)
            {
                points[j]->Draw(gc);
            }
        }
    }

    if (!suppressTooltips)
    {
        DrawTooltips(gc);
    }
}

wxSharedPtr<wxVector<const wxChartsElement*>> wxAreaChart::GetActiveElements(const wxPoint &point)
{
    wxSharedPtr<wxVector<const wxChartsElement*>> activeElements(new wxVector<const wxChartsElement*>());
    for (size_t i = 0; i < m_datasets.size(); ++i)
    {
        const wxVector<Point::ptr>& points = m_datasets[i]->GetPoints();
        for (size_t j = 0; j < points.size(); ++j)
        {
            if (points[j]->HitTest(point))
            {
                activeElements->push_back(points[j].get());
            }
        }
    }
    return activeElements;
}
