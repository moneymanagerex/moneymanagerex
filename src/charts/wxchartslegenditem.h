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

/// @file

#ifndef _WX_CHARTS_WXCHARTSLEGENDITEM_H_
#define _WX_CHARTS_WXCHARTSLEGENDITEM_H_

#include "wxchartslicedata.h"
#include "wxchartsdoubledataset.h"
#include "wxlinechartdatasetoptions.h"
#include <wx/vector.h>
#include <wx/colour.h>
#include <wx/string.h>
#include <map>

/// Class that represents items of the wxChartsLegendData class.
class wxChartsLegendItem
{
public:
    /// Constructs a wxChartsLegendItem instance.
    /// @param color The color associated with this
    /// item on the chart.
    /// @param label The text to be display in the legend.
    wxChartsLegendItem(const wxColor &color,
        const wxString &label);
    /// Constructs a wxChartsLegendItem instance from a
    /// slice data.
    /// @param slice The slice of the chart this legend item
    /// corresponds to.
    wxChartsLegendItem(const wxChartSliceData &slice);
    /// Constructs a wxChartsLegendItem instance from a
    /// line chart dataset.
    /// @param dataset The dataset this legend item
    /// corresponds to.
    wxChartsLegendItem(const wxChartsDoubleDataset &dataset,
        const wxLineChartDatasetOptions& datasetOptions);

    /// Gets the color of the item.
    /// @return The color of the item.
    const wxColor& GetColor() const;
    /// Gets the label of the item.
    /// @return The label of the item.
    const wxString& GetLabel() const;

private:
    wxColor m_color;
    wxString m_label;
};

typedef wxVector<wxChartsLegendItem> wxChartsLegendItems;

/// Data for the wxChartsLegendCtrl control.
class wxChartsLegendData
{
public:
    /// Constructs a wxChartsLegendData
    /// instance.
    wxChartsLegendData();
    /// Constructs a wxChartsLegendData instance from
    /// a list of slices. This can be used to build
    /// a legend for the wxPieChartCtrl and
    /// wxDoughnutChartCtrl controls for instance.
    /// @param slices List of slices.
    wxChartsLegendData(const std::map<wxString, wxChartSliceData>& slices);
    /// Constructs a wxChartsLegendData instance from
    /// a list of datasets. This can be used to build
    /// a legend for the wxLineChartCtrl control.
    /// @param datasets List of datasets.
    wxChartsLegendData(const wxVector<wxChartsDoubleDataset::ptr>& datasets);

    /// Appends an item to the legend.
    /// @param item The new item.
    void Append(const wxChartsLegendItem &item);

    /// Gets the list of items in the legend.
    /// @return The list of legend items.
    const wxVector<wxChartsLegendItem>& GetItems() const;

private:
    wxVector<wxChartsLegendItem> m_items;
};

#endif
