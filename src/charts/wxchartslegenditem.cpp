/*
    Copyright (c) 2018-2019 Xavier Leclercq and the wxCharts contributors

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

#include "wxchartslegenditem.h"
#include "wxchartstheme.h"

wxChartsLegendItem::wxChartsLegendItem(const wxColor &color,
                                       const wxString &label)
    : m_color(color), m_label(label)
{
}

wxChartsLegendItem::wxChartsLegendItem(const wxChartSliceData &slice)
    : m_color(slice.GetColor()), m_label(slice.GetLabel())
{
}

wxChartsLegendItem::wxChartsLegendItem(const wxChartsDoubleDataset &dataset,
                                       const wxLineChartDatasetOptions& datasetOptions)
    : m_color(*wxWHITE), m_label(dataset.GetName())
{
    if (datasetOptions.ShowDots())
    {
        m_color = datasetOptions.GetDotBrushOptions().GetColor();
    }
    else if (datasetOptions.ShowLine())
    {
        m_color = datasetOptions.GetLineColor();
    }
    else if (datasetOptions.Fill())
    {
        m_color = datasetOptions.GetFillColor();
    }
}

const wxColor& wxChartsLegendItem::GetColor() const
{
    return m_color;
}

const wxString& wxChartsLegendItem::GetLabel() const
{
    return m_label;
}

wxChartsLegendData::wxChartsLegendData()
{
}

wxChartsLegendData::wxChartsLegendData(const std::map<wxString, wxChartSliceData>& slices)
{
    for (const auto &slice : slices)
    {
        m_items.push_back(wxChartsLegendItem(slice.second));
    }
}

wxChartsLegendData::wxChartsLegendData(const wxVector<wxChartsDoubleDataset::ptr>& datasets)
{
    for (size_t i = 0; i < datasets.size(); ++i)
    {
        wxSharedPtr<wxChartsDatasetTheme> datasetTheme = wxChartsDefaultTheme->GetDatasetTheme(wxChartsDatasetId::CreateImplicitId(i));
        wxSharedPtr<wxLineChartDatasetOptions> datasetOptions = datasetTheme->GetLineChartDatasetOptions();

        m_items.push_back(wxChartsLegendItem(*datasets[i], *datasetOptions));
    }
}

void wxChartsLegendData::Append(const wxChartsLegendItem &item)
{
    m_items.push_back(item);
}

const wxVector<wxChartsLegendItem>& wxChartsLegendData::GetItems() const
{
    return m_items;
}
