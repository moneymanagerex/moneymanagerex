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

#ifndef _WX_CHARTS_WXCOLUMNCHART_H_
#define _WX_CHARTS_WXCOLUMNCHART_H_

#include "wxchart.h"
#include "wxchartscategoricaldata.h"
#include "wxcolumnchartoptions.h"
#include "wxchartsgrid.h"
#include "wxchartsrectangle.h"
#include <wx/sharedptr.h>

/// A column chart.

/// \ingroup chartclasses
class wxColumnChart : public wxChart
{
public:
    wxColumnChart(wxChartsCategoricalData::ptr &data, const wxSize &size);

    virtual const wxChartCommonOptions& GetCommonOptions() const;

private:
    static wxDouble GetMinValue(const wxVector<wxChartsDoubleDataset::ptr>& datasets);
    static wxDouble GetMaxValue(const wxVector<wxChartsDoubleDataset::ptr>& datasets);

    virtual void DoSetSize(const wxSize &size);
    virtual void DoFit();
    virtual void DoDraw(wxGraphicsContext &gc, bool suppressTooltips);
    virtual wxSharedPtr<wxVector<const wxChartsElement*>> GetActiveElements(const wxPoint &point);

    wxDouble GetColumnWidth() const;

private:
    class Column : public wxChartsRectangle
    {
    public:
        typedef wxSharedPtr<Column> ptr;

        Column(wxDouble value,
            const wxChartTooltipProvider::ptr tooltipProvider,
            wxDouble x, wxDouble y,
            const wxChartsPenOptions &penOptions,
            const wxChartsBrushOptions &brushOptions,
            int borders);

        wxDouble GetValue() const;

    private:
        wxDouble m_value;
    };

    struct Dataset
    {
    public:
        typedef wxSharedPtr<Dataset> ptr;

        Dataset();

        const wxVector<Column::ptr>& GetColumns() const;
        void AppendColumn(Column::ptr column);

    private:
        wxVector<Column::ptr> m_columns;
    };

private:
    wxSharedPtr<wxColumnChartOptions> m_options;
    wxChartsGrid m_grid;
    wxVector<Dataset::ptr> m_datasets;
};

#endif
