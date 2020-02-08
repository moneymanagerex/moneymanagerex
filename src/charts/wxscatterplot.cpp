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

#include "wxscatterplot.h"
#include "wxchartstheme.h"
#include <sstream>

wxScatterPlotDataset::wxScatterPlotDataset(const wxColor& fillColor,
                                           const wxColor& strokeColor,
                                           wxVector<wxPoint2DDouble> &data)
    : m_fillColor(fillColor), m_strokeColor(strokeColor), m_data(data)
{
}

const wxColor& wxScatterPlotDataset::GetFillColor() const
{
    return m_fillColor;
}

const wxColor& wxScatterPlotDataset::GetStrokeColor() const
{
    return m_strokeColor;
}

const wxVector<wxPoint2DDouble>& wxScatterPlotDataset::GetData() const
{
    return m_data;
}

wxScatterPlotData::wxScatterPlotData()
{
}

void wxScatterPlotData::AddDataset(wxScatterPlotDataset::ptr dataset)
{
    m_datasets.push_back(dataset);
}

const wxVector<wxScatterPlotDataset::ptr>& wxScatterPlotData::GetDatasets() const
{
    return m_datasets;
}

wxScatterPlot::Point::Point(wxPoint2DDouble value,
                            const wxChartTooltipProvider::ptr tooltipProvider,
                            wxDouble x,
                            wxDouble y,
                            const wxChartsPointOptions &options)
    : wxChartsPoint(x, y, 5, 20, tooltipProvider, options), m_value(value)
{
}

wxPoint2DDouble wxScatterPlot::Point::GetTooltipPosition() const
{
    wxPoint2DDouble position = wxChartsPoint::GetTooltipPosition();
    position.m_y -= 10;
    return position;
}

wxPoint2DDouble wxScatterPlot::Point::GetValue() const
{
    return m_value;
}

wxScatterPlot::Dataset::Dataset()
{
}

const wxVector<wxScatterPlot::Point::ptr>& wxScatterPlot::Dataset::GetPoints() const
{
    return m_points;
}

void wxScatterPlot::Dataset::AppendPoint(Point::ptr point)
{
    m_points.push_back(point);
}

wxScatterPlot::wxScatterPlot(const wxScatterPlotData &data,
                             const wxSize &size)
    : m_options(wxChartsDefaultTheme->GetScatterPlotOptions()),
    m_grid(
        wxPoint2DDouble(m_options->GetPadding().GetLeft(), m_options->GetPadding().GetRight()),
        size,
        GetMinXValue(data.GetDatasets()), GetMaxXValue(data.GetDatasets()),
        GetMinYValue(data.GetDatasets()), GetMaxYValue(data.GetDatasets()),
        m_options->GetGridOptions()
        )
{
    Initialize(data);
}

wxScatterPlot::wxScatterPlot(const wxScatterPlotData &data,
                             wxSharedPtr<wxScatterPlotOptions> &options, 
                             const wxSize &size)
    : m_options(options),
    m_grid(
        wxPoint2DDouble(m_options->GetPadding().GetLeft(), m_options->GetPadding().GetRight()),
        size,
        GetMinXValue(data.GetDatasets()), GetMaxXValue(data.GetDatasets()),
        GetMinYValue(data.GetDatasets()), GetMaxYValue(data.GetDatasets()),
        m_options->GetGridOptions()
        )
{
    Initialize(data);
}

const wxChartCommonOptions& wxScatterPlot::GetCommonOptions() const
{
    return m_options->GetCommonOptions();
}

void wxScatterPlot::Initialize(const wxScatterPlotData &data)
{
    const wxVector<wxScatterPlotDataset::ptr>& datasets = data.GetDatasets();
    for (size_t i = 0; i < datasets.size(); ++i)
    {
        Dataset::ptr newDataset(new Dataset());

        const wxVector<wxPoint2DDouble>& datasetData = datasets[i]->GetData();
        for (size_t j = 0; j < datasetData.size(); ++j)
        {
            std::stringstream tooltip;
            tooltip << "(" << datasetData[j].m_x << "," << datasetData[j].m_y << ")";
            wxChartTooltipProvider::ptr tooltipProvider(
                new wxChartTooltipProviderStatic("", tooltip.str(), datasets[i]->GetFillColor())
                );

            Point::ptr point(
                new Point(datasetData[j], tooltipProvider, 20 + j * 10, 0,
                    wxChartsPointOptions(2, datasets[i]->GetStrokeColor(), datasets[i]->GetFillColor()))
                );

            newDataset->AppendPoint(point);
        }

        m_datasets.push_back(newDataset);
    }
}

wxDouble wxScatterPlot::GetMinXValue(const wxVector<wxScatterPlotDataset::ptr>& datasets)
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

wxDouble wxScatterPlot::GetMaxXValue(const wxVector<wxScatterPlotDataset::ptr>& datasets)
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

wxDouble wxScatterPlot::GetMinYValue(const wxVector<wxScatterPlotDataset::ptr>& datasets)
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

wxDouble wxScatterPlot::GetMaxYValue(const wxVector<wxScatterPlotDataset::ptr>& datasets)
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

void wxScatterPlot::DoSetSize(const wxSize &size)
{
    wxSize newSize(
        size.GetWidth() - m_options->GetPadding().GetTotalHorizontalPadding(),
        size.GetHeight() - m_options->GetPadding().GetTotalVerticalPadding()
        );
    m_grid.Resize(newSize);
}

void wxScatterPlot::DoFit()
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

void wxScatterPlot::DoDraw(wxGraphicsContext &gc,
                           bool suppressTooltips)
{
    m_grid.Fit(gc);
    m_grid.Draw(gc);

    Fit();

    for (size_t i = 0; i < m_datasets.size(); ++i)
    {
        const wxVector<Point::ptr>& points = m_datasets[i]->GetPoints();
        for (size_t j = 0; j < points.size(); ++j)
        {
            points[j]->Draw(gc);
        }
    }

    if (!suppressTooltips)
    {
        DrawTooltips(gc);
    }
}

wxSharedPtr<wxVector<const wxChartsElement*>> wxScatterPlot::GetActiveElements(const wxPoint &point)
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
