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

/// @file

#include "wxbarchart.h"
#include "wxchartstheme.h"
#include "wxchartscategoricalaxis.h"
#include "wxchartsnumericalaxis.h"
#include <sstream>

wxBarChart::Bar::Bar(wxDouble value,
                     const wxChartTooltipProvider::ptr tooltipProvider,
                     wxDouble x,
                     wxDouble y,
                     const wxChartsPenOptions &penOptions,
                     const wxChartsBrushOptions &brushOptions,
                     int borders)
    : wxChartsRectangle(x, y, tooltipProvider, wxChartsRectangleOptions(penOptions, brushOptions, borders)),
    m_value(value)
{
}

wxDouble wxBarChart::Bar::GetValue() const
{
    return m_value;
}

wxBarChart::Dataset::Dataset()
{
}

const wxVector<wxBarChart::Bar::ptr>& wxBarChart::Dataset::GetBars() const
{
    return m_bars;
}

void wxBarChart::Dataset::AppendBar(Bar::ptr bar)
{
    m_bars.push_back(bar);
}

wxBarChart::wxBarChart(wxChartsCategoricalData::ptr &data,
                       const wxSize &size)
    : m_options(wxChartsDefaultTheme->GetBarChartOptions()),
    m_grid(
        wxPoint2DDouble(m_options->GetPadding().GetLeft(), m_options->GetPadding().GetRight()),
        size,
        wxChartsCategoricalAxis::make_shared("x", data->GetCategories(), m_options->GetGridOptions().GetXAxisOptions()),
        wxChartsNumericalAxis::make_shared("y", GetMinValue(data->GetDatasets()), GetMaxValue(data->GetDatasets()), m_options->GetGridOptions().GetYAxisOptions()),
        m_options->GetGridOptions()
    )
{
    Initialize(data);
}

wxBarChart::wxBarChart(wxChartsCategoricalData::ptr &data,
                       wxBarChartOptions::ptr options,
                       const wxSize &size)
    : m_options(options),
    m_grid(
        wxPoint2DDouble(m_options->GetPadding().GetLeft(), m_options->GetPadding().GetRight()),
        size,
        wxChartsCategoricalAxis::make_shared("x", data->GetCategories(), m_options->GetGridOptions().GetXAxisOptions()),
        wxChartsNumericalAxis::make_shared("y", GetMinValue(data->GetDatasets()), GetMaxValue(data->GetDatasets()), m_options->GetGridOptions().GetYAxisOptions()),
        m_options->GetGridOptions()
        )
{
    Initialize(data);
}

const wxChartCommonOptions& wxBarChart::GetCommonOptions() const
{
    return m_options->GetCommonOptions();
}

void wxBarChart::Initialize(wxChartsCategoricalData::ptr &data)
{
    const wxVector<wxChartsDoubleDataset::ptr>& datasets = data->GetDatasets();
    for (size_t i = 0; i < datasets.size(); ++i)
    {
        wxSharedPtr<wxChartsDatasetTheme> datasetTheme = wxChartsDefaultTheme->GetDatasetTheme(wxChartsDatasetId::CreateImplicitId(i));
        wxSharedPtr<wxBarChartDatasetOptions> datasetOptions = datasetTheme->GetBarChartDatasetOptions();

        const wxChartsDoubleDataset& dataset = *datasets[i];
        Dataset::ptr newDataset(new Dataset());

        const wxVector<wxDouble>& datasetData = dataset.GetData();
        for (size_t j = 0; j < datasetData.size(); ++j)
        {
            std::stringstream tooltip;
            tooltip << datasetData[j];
            wxChartTooltipProvider::ptr tooltipProvider(
                new wxChartTooltipProviderStatic(data->GetCategories()[j], tooltip.str(), datasetOptions->GetBrushOptions().GetColor())
                );

            newDataset->AppendBar(Bar::ptr(new Bar(
                datasetData[j], tooltipProvider, 25, 50, datasetOptions->GetPenOptions(),
                datasetOptions->GetBrushOptions(), wxTOP | wxRIGHT | wxBOTTOM
                )));
        }

        m_datasets.push_back(newDataset);
    }
}

wxDouble wxBarChart::GetMinValue(const wxVector<wxChartsDoubleDataset::ptr>& datasets)
{
    wxDouble result = 0;
    bool foundValue = false;

    for (size_t i = 0; i < datasets.size(); ++i)
    {
        const wxVector<wxDouble>& values = datasets[i]->GetData();
        for (size_t j = 0; j < values.size(); ++j)
        {
            if (!foundValue)
            {
                result = values[j];
                foundValue = true;
            }
            else if (result > values[j])
            {
                result = values[j];
            }
        }
    }

    return result;
}

wxDouble wxBarChart::GetMaxValue(const wxVector<wxChartsDoubleDataset::ptr>& datasets)
{
    wxDouble result = 0;
    bool foundValue = false;

    for (size_t i = 0; i < datasets.size(); ++i)
    {
        const wxVector<wxDouble>& values = datasets[i]->GetData();
        for (size_t j = 0; j < values.size(); ++j)
        {
            if (!foundValue)
            {
                result = values[j];
                foundValue = true;
            }
            else if (result < values[j])
            {
                result = values[j];
            }
        }
    }

    return result;
}

void wxBarChart::DoSetSize(const wxSize &size)
{
    wxSize newSize(
        size.GetWidth() - m_options->GetPadding().GetTotalHorizontalPadding(),
        size.GetHeight() - m_options->GetPadding().GetTotalVerticalPadding()
        );
    m_grid.Resize(newSize);
}

void wxBarChart::DoFit()
{
    wxDouble barHeight = GetBarHeight();

    for (size_t i = 0; i < m_datasets.size(); ++i)
    {
        Dataset& currentDataset = *m_datasets[i];
        for (size_t j = 0; j < currentDataset.GetBars().size(); ++j)
        {
            Bar& bar = *(currentDataset.GetBars()[j]);

            wxPoint2DDouble upperLeftCornerPosition = m_grid.GetMapping().GetXAxis().GetTickMarkPosition(j + 1);
            upperLeftCornerPosition.m_y += m_options->GetBarSpacing() + (i * (barHeight + m_options->GetDatasetSpacing()));

            wxPoint2DDouble bottomLeftCornerPosition = upperLeftCornerPosition;
            bottomLeftCornerPosition.m_y += barHeight;

            wxPoint2DDouble upperRightCornerPosition = m_grid.GetMapping().GetWindowPositionAtTickMark(j + 1, bar.GetValue());

            bar.SetPosition(upperLeftCornerPosition);
            bar.SetSize(upperRightCornerPosition.m_x - upperLeftCornerPosition.m_x,
                bottomLeftCornerPosition.m_y - upperLeftCornerPosition.m_y);
        }
    }
}

void wxBarChart::DoDraw(wxGraphicsContext &gc,
                        bool suppressTooltips)
{
    m_grid.Fit(gc);
    m_grid.Draw(gc);

    Fit();

    for (size_t i = 0; i < m_datasets.size(); ++i)
    {
        Dataset& currentDataset = *m_datasets[i];
        for (size_t j = 0; j < currentDataset.GetBars().size(); ++j)
        {
            currentDataset.GetBars()[j]->Draw(gc);
        }
    }

    if (!suppressTooltips)
    {
        DrawTooltips(gc);
    }
}

wxSharedPtr<wxVector<const wxChartsElement*>> wxBarChart::GetActiveElements(const wxPoint &point)
{
    wxSharedPtr<wxVector<const wxChartsElement*>> activeElements(new wxVector<const wxChartsElement*>());

    for (size_t i = 0; i < m_datasets.size(); ++i)
    {
        const wxVector<Bar::ptr>& bars = m_datasets[i]->GetBars();
        for (size_t j = 0; j < bars.size(); ++j)
        {
            if (bars[j]->HitTest(point))
            {
                for (size_t k = 0; k < m_datasets.size(); ++k)
                {
                    activeElements->push_back(m_datasets[k]->GetBars()[j].get());
                }
            }
        }
    }

    return activeElements;
}

wxDouble wxBarChart::GetBarHeight() const
{
    wxDouble availableHeight = m_grid.GetMapping().GetXAxis().GetDistanceBetweenTickMarks() -
        (2 * m_options->GetBarSpacing()) - ((m_datasets.size() - 1) * m_options->GetDatasetSpacing());
    return (availableHeight / m_datasets.size());
}
