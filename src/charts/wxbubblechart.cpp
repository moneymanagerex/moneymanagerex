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

#include "wxbubblechart.h"
#include "wxchartstheme.h"
#include <sstream>
#include <cmath>

wxDoubleTriplet::wxDoubleTriplet(wxDouble x, wxDouble y, wxDouble z)
    : m_x(x), m_y(y), m_z(z)
{
}

wxBubbleChartDataset::wxBubbleChartDataset(const wxColor& fillColor,
    const wxColor& outlineColor,
    wxVector<wxDoubleTriplet> &data)
    : m_fillColor(fillColor), m_outlineWidth(1),
    m_outlineColor(outlineColor), m_minRadius(2), m_maxRadius(40),
    m_data(data)
{
}

const wxColor& wxBubbleChartDataset::GetFillColor() const
{
    return m_fillColor;
}

unsigned int wxBubbleChartDataset::GetOutlineWidth() const
{
    return m_outlineWidth;
}

const wxColor& wxBubbleChartDataset::GetOutlineColor() const
{
    return m_outlineColor;
}

unsigned int wxBubbleChartDataset::GetMinRadius() const
{
    return m_minRadius;
}

unsigned int wxBubbleChartDataset::GetMaxRadius() const
{
    return m_maxRadius;
}

const wxVector<wxDoubleTriplet>& wxBubbleChartDataset::GetData() const
{
    return m_data;
}

wxBubbleChartData::wxBubbleChartData()
{
}

void wxBubbleChartData::AddDataset(wxBubbleChartDataset::ptr dataset)
{
    m_datasets.push_back(dataset);
}

const wxVector<wxBubbleChartDataset::ptr>& wxBubbleChartData::GetDatasets() const
{
    return m_datasets;
}

wxBubbleChart::Circle::Circle(wxDoubleTriplet value,
    wxDouble x,
    wxDouble y,
    wxDouble radius,
    const wxChartTooltipProvider::ptr tooltipProvider,
    const wxChartsCircleOptions &options)
    : wxChartsCircle(x, y, radius, tooltipProvider, options), m_value(value)
{
}

wxDoubleTriplet wxBubbleChart::Circle::GetValue() const
{
    return m_value;
}

wxBubbleChart::Dataset::Dataset(unsigned int minRadius,
    unsigned int maxRadius)
    : m_minRadius(minRadius), m_maxRadius(maxRadius)
{
}

const wxVector<wxBubbleChart::Circle::ptr>& wxBubbleChart::Dataset::GetCircles() const
{
    return m_circles;
}

void wxBubbleChart::Dataset::AppendCircle(Circle::ptr circle)
{
    m_circles.push_back(circle);
}

unsigned int wxBubbleChart::Dataset::GetMinRadius() const
{
    return m_minRadius;
}

unsigned int wxBubbleChart::Dataset::GetMaxRadius() const
{
    return m_maxRadius;
}

wxBubbleChart::wxBubbleChart(const wxBubbleChartData &data, 
                             const wxSize &size)
    : m_options(wxChartsDefaultTheme->GetBubbleChartOptions()),
    m_grid(
        wxPoint2DDouble(m_options->GetPadding().GetLeft(), m_options->GetPadding().GetRight()),
        size,
        GetMinXValue(data.GetDatasets()), GetMaxXValue(data.GetDatasets()),
        GetMinYValue(data.GetDatasets()), GetMaxYValue(data.GetDatasets()),
        m_options->GetGridOptions()
    ),
    m_minZValue(GetMinZValue(data.GetDatasets())), m_maxZValue(GetMaxZValue(data.GetDatasets()))
{
    Initialize(data);
}

wxBubbleChart::wxBubbleChart(const wxBubbleChartData &data,
                             wxBubbleChartOptions::ptr options,
                             const wxSize &size)
    : m_options(options),
    m_grid(
        wxPoint2DDouble(m_options->GetPadding().GetLeft(), m_options->GetPadding().GetRight()),
        size,
        GetMinXValue(data.GetDatasets()), GetMaxXValue(data.GetDatasets()),
        GetMinYValue(data.GetDatasets()), GetMaxYValue(data.GetDatasets()),
        m_options->GetGridOptions()
        ),
    m_minZValue(GetMinZValue(data.GetDatasets())), m_maxZValue(GetMaxZValue(data.GetDatasets()))
{
    Initialize(data);
}

const wxChartCommonOptions& wxBubbleChart::GetCommonOptions() const
{
    return m_options->GetCommonOptions();
}

void wxBubbleChart::Initialize(const wxBubbleChartData &data)
{
    const wxVector<wxBubbleChartDataset::ptr>& datasets = data.GetDatasets();
    for (size_t i = 0; i < datasets.size(); ++i)
    {
        Dataset::ptr newDataset(new Dataset(datasets[i]->GetMinRadius(), datasets[i]->GetMaxRadius()));

        const wxVector<wxDoubleTriplet>& datasetData = datasets[i]->GetData();
        for (size_t j = 0; j < datasetData.size(); ++j)
        {
            std::stringstream tooltip;
            tooltip << "(" << datasetData[j].m_x << "," << datasetData[j].m_y << ")";
            wxChartTooltipProvider::ptr tooltipProvider(
                new wxChartTooltipProviderStatic("", tooltip.str(), datasets[i]->GetFillColor())
                );

            Circle::ptr circle(
                new Circle(datasetData[j], 0, 0, 1, tooltipProvider,
                    wxChartsCircleOptions(datasets[i]->GetOutlineWidth(),
                        datasets[i]->GetOutlineColor(), datasets[i]->GetFillColor()))
                );

            newDataset->AppendCircle(circle);
        }

        m_datasets.push_back(newDataset);
    }
}

wxDouble wxBubbleChart::GetMinXValue(const wxVector<wxBubbleChartDataset::ptr>& datasets)
{
    wxDouble result = 0;
    bool foundValue = false;

    for (size_t i = 0; i < datasets.size(); ++i)
    {
        const wxVector<wxDoubleTriplet>& values = datasets[i]->GetData();
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

wxDouble wxBubbleChart::GetMaxXValue(const wxVector<wxBubbleChartDataset::ptr>& datasets)
{
    wxDouble result = 0;
    bool foundValue = false;

    for (size_t i = 0; i < datasets.size(); ++i)
    {
        const wxVector<wxDoubleTriplet>& values = datasets[i]->GetData();
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

wxDouble wxBubbleChart::GetMinYValue(const wxVector<wxBubbleChartDataset::ptr>& datasets)
{
    wxDouble result = 0;
    bool foundValue = false;

    for (size_t i = 0; i < datasets.size(); ++i)
    {
        const wxVector<wxDoubleTriplet>& values = datasets[i]->GetData();
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

wxDouble wxBubbleChart::GetMaxYValue(const wxVector<wxBubbleChartDataset::ptr>& datasets)
{
    wxDouble result = 0;
    bool foundValue = false;

    for (size_t i = 0; i < datasets.size(); ++i)
    {
        const wxVector<wxDoubleTriplet>& values = datasets[i]->GetData();
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

wxDouble wxBubbleChart::GetMinZValue(const wxVector<wxBubbleChartDataset::ptr>& datasets)
{
    wxDouble result = 0;
    bool foundValue = false;

    for (size_t i = 0; i < datasets.size(); ++i)
    {
        const wxVector<wxDoubleTriplet>& values = datasets[i]->GetData();
        for (size_t j = 0; j < values.size(); ++j)
        {
            if (!foundValue)
            {
                result = values[j].m_z;
                foundValue = true;
            }
            else if (result > values[j].m_z)
            {
                result = values[j].m_z;
            }
        }
    }

    return result;
}

wxDouble wxBubbleChart::GetMaxZValue(const wxVector<wxBubbleChartDataset::ptr>& datasets)
{
    wxDouble result = 0;
    bool foundValue = false;

    for (size_t i = 0; i < datasets.size(); ++i)
    {
        const wxVector<wxDoubleTriplet>& values = datasets[i]->GetData();
        for (size_t j = 0; j < values.size(); ++j)
        {
            if (!foundValue)
            {
                result = values[j].m_z;
                foundValue = true;
            }
            else if (result < values[j].m_z)
            {
                result = values[j].m_z;
            }
        }
    }

    return result;
}

void wxBubbleChart::DoFit()
{
    wxDouble zFactor = 1 / (m_maxZValue - m_minZValue);

    for (size_t i = 0; i < m_datasets.size(); ++i)
    {
        wxDouble minRadius = m_datasets[i]->GetMinRadius();
        wxDouble maxRadius = m_datasets[i]->GetMaxRadius();
        wxDouble radiusFactor = (maxRadius);
        const wxVector<Circle::ptr>& circles = m_datasets[i]->GetCircles();
        for (size_t j = 0; j < circles.size(); ++j)
        {
            const Circle::ptr& circle = circles[j];
            circle->SetCenter(m_grid.GetMapping().GetWindowPosition(circle->GetValue().m_x, circle->GetValue().m_y));
            circle->SetRadius(minRadius + (sqrt(circle->GetValue().m_z * zFactor) * radiusFactor));
        }
    }
}

void wxBubbleChart::DoDraw(wxGraphicsContext &gc,
                           bool suppressTooltips)
{
    m_grid.Fit(gc);
    m_grid.Draw(gc);

    Fit();

    for (size_t i = 0; i < m_datasets.size(); ++i)
    {
        const wxVector<Circle::ptr>& circles = m_datasets[i]->GetCircles();
        for (size_t j = 0; j < circles.size(); ++j)
        {
            const Circle::ptr& circle = circles[j];
            circle->Draw(gc);
        }
    }

    if (!suppressTooltips)
    {
        DrawTooltips(gc);
    }
}

void wxBubbleChart::DoSetSize(const wxSize &size)
{
    m_grid.Resize(size);
}

wxSharedPtr<wxVector<const wxChartsElement*>> wxBubbleChart::GetActiveElements(const wxPoint &point)
{
    wxSharedPtr<wxVector<const wxChartsElement*>> activeElements(new wxVector<const wxChartsElement*>());

    for (size_t i = 0; i < m_datasets.size(); ++i)
    {
        const wxVector<Circle::ptr>& circles = m_datasets[i]->GetCircles();
        for (size_t j = 0; j < circles.size(); ++j)
        {
            if (circles[j]->HitTest(point))
            {
                activeElements->push_back(circles[j].get());
            }
        }
    }

    return activeElements;
}
