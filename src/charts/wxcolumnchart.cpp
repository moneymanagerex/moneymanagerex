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

#include "wxcolumnchart.h"
#include "wxchartstheme.h"
#include "wxchartscategoricalaxis.h"
#include "wxchartsnumericalaxis.h"
#include <sstream>

wxColumnChart::Column::Column(wxDouble value,
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

wxDouble wxColumnChart::Column::GetValue() const
{
    return m_value;
}

wxColumnChart::Dataset::Dataset()
{
}

const wxVector<wxColumnChart::Column::ptr>& wxColumnChart::Dataset::GetColumns() const
{
    return m_columns;
}

void wxColumnChart::Dataset::AppendColumn(Column::ptr column)
{
    m_columns.push_back(column);
}

wxColumnChart::wxColumnChart(wxChartsCategoricalData::ptr &data,
                             const wxSize &size)
    : m_options(wxChartsDefaultTheme->GetColumnChartOptions()), 
    m_grid(
        wxPoint2DDouble(m_options->GetPadding().GetLeft(), m_options->GetPadding().GetRight()),
        size,
        wxChartsCategoricalAxis::make_shared("x", data->GetCategories(), m_options->GetGridOptions().GetXAxisOptions()),
        wxChartsNumericalAxis::make_shared("y", GetMinValue(data->GetDatasets()), GetMaxValue(data->GetDatasets()), m_options->GetGridOptions().GetYAxisOptions()),
        m_options->GetGridOptions()
        )
{
    const wxVector<wxChartsDoubleDataset::ptr>& datasets = data->GetDatasets();
    for (size_t i = 0; i < datasets.size(); ++i)
    {
        wxSharedPtr<wxChartsDatasetTheme> datasetTheme = wxChartsDefaultTheme->GetDatasetTheme(wxChartsDatasetId::CreateImplicitId(i));
        wxSharedPtr<wxColumnChartDatasetOptions> datasetOptions = datasetTheme->GetColumnChartDatasetOptions();

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

            newDataset->AppendColumn(Column::ptr(new Column(
                datasetData[j], tooltipProvider, 25, 50, datasetOptions->GetPenOptions(),
                datasetOptions->GetBrushOptions(), wxLEFT | wxTOP | wxRIGHT
                )));
        }

        m_datasets.push_back(newDataset);
    }
}

const wxChartCommonOptions& wxColumnChart::GetCommonOptions() const
{
    return m_options->GetCommonOptions();
}

wxDouble wxColumnChart::GetMinValue(const wxVector<wxChartsDoubleDataset::ptr>& datasets)
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

wxDouble wxColumnChart::GetMaxValue(const wxVector<wxChartsDoubleDataset::ptr>& datasets)
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

void wxColumnChart::DoSetSize(const wxSize &size)
{
    m_grid.Resize(size);
}

void wxColumnChart::DoFit()
{
    wxDouble columnWidth = GetColumnWidth();

    for (size_t i = 0; i < m_datasets.size(); ++i)
    {
        Dataset& currentDataset = *m_datasets[i];
        for (size_t j = 0; j < currentDataset.GetColumns().size(); ++j)
        {
            Column& column = *(currentDataset.GetColumns()[j]);
            wxPoint2DDouble position = m_grid.GetMapping().GetWindowPositionAtTickMark(j, column.GetValue());
            position.m_x += m_options->GetColumnSpacing() + (i * (columnWidth + m_options->GetDatasetSpacing()));

            wxPoint2DDouble bottomLeftCornerPosition = m_grid.GetMapping().GetXAxis().GetTickMarkPosition(j);

            column.SetPosition(position);
            column.SetSize(columnWidth, bottomLeftCornerPosition.m_y - position.m_y);
        }
    }
}

void wxColumnChart::DoDraw(wxGraphicsContext &gc,
                           bool suppressTooltips)
{
    m_grid.Fit(gc);
    m_grid.Draw(gc);

    Fit();

    for (size_t i = 0; i < m_datasets.size(); ++i)
    {
        Dataset& currentDataset = *m_datasets[i];
        for (size_t j = 0; j < currentDataset.GetColumns().size(); ++j)
        {
            currentDataset.GetColumns()[j]->Draw(gc);
        }
    }

    if (!suppressTooltips)
    {
        DrawTooltips(gc);
    }
}

wxSharedPtr<wxVector<const wxChartsElement*>> wxColumnChart::GetActiveElements(const wxPoint &point)
{
    wxSharedPtr<wxVector<const wxChartsElement*>> activeElements(new wxVector<const wxChartsElement*>());

    for (size_t i = 0; i < m_datasets.size(); ++i)
    {
        const wxVector<Column::ptr>& columns = m_datasets[i]->GetColumns();
        for (size_t j = 0; j < columns.size(); ++j)
        {
            if (columns[j]->HitTest(point))
            {
                for (size_t k = 0; k < m_datasets.size(); ++k)
                {
                    activeElements->push_back(m_datasets[k]->GetColumns()[j].get());
                }
            }
        }
    }

    return activeElements;
}

wxDouble wxColumnChart::GetColumnWidth() const
{
    wxDouble availableWidth = m_grid.GetMapping().GetXAxis().GetDistanceBetweenTickMarks() -
        (2 * m_options->GetColumnSpacing()) - ((m_datasets.size() - 1) * m_options->GetDatasetSpacing());
    return (availableWidth / m_datasets.size());
}
