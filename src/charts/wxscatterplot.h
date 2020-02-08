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

#ifndef _WX_CHARTS_WXSCATTERPLOT_H_
#define _WX_CHARTS_WXSCATTERPLOT_H_

#include "wxchart.h"
#include "wxscatterplotoptions.h"
#include "wxchartsgrid.h"
#include "wxchartspoint.h"

class wxScatterPlotDataset
{
public:
    /// Smart pointer typedef.
    typedef wxSharedPtr<wxScatterPlotDataset> ptr;

    /// Constructs a wxScatterPlotDataset instance.
    /// @param data The list of values.
    wxScatterPlotDataset(const wxColor& fillColor, const wxColor& strokeColor,
        wxVector<wxPoint2DDouble> &data);

    const wxColor& GetFillColor() const;
    const wxColor& GetStrokeColor() const;

    const wxVector<wxPoint2DDouble>& GetData() const;

private:
    wxColor m_fillColor;
    wxColor m_strokeColor;
    wxVector<wxPoint2DDouble> m_data;
};

/// Data for the wxScatterPlotCtrl control.
class wxScatterPlotData
{
public:
    /// Constructs a wxScatterPlotData instance.
    wxScatterPlotData();

    /// Adds a dataset.
    /// @param dataset The dataset to add.
    void AddDataset(wxScatterPlotDataset::ptr dataset);

    const wxVector<wxScatterPlotDataset::ptr>& GetDatasets() const;

private:
    wxVector<wxScatterPlotDataset::ptr> m_datasets;
};

/// A scatter plot.

/// \ingroup chartclasses
class wxScatterPlot : public wxChart
{
public:
    wxScatterPlot(const wxScatterPlotData &data, const wxSize &size);
    wxScatterPlot(const wxScatterPlotData &data, wxSharedPtr<wxScatterPlotOptions> &options,
        const wxSize &size);

    virtual const wxChartCommonOptions& GetCommonOptions() const;

private:
    void Initialize(const wxScatterPlotData &data);
    static wxDouble GetMinXValue(const wxVector<wxScatterPlotDataset::ptr>& datasets);
    static wxDouble GetMaxXValue(const wxVector<wxScatterPlotDataset::ptr>& datasets);
    static wxDouble GetMinYValue(const wxVector<wxScatterPlotDataset::ptr>& datasets);
    static wxDouble GetMaxYValue(const wxVector<wxScatterPlotDataset::ptr>& datasets);

    virtual void DoSetSize(const wxSize &size);
    virtual void DoFit();
    virtual void DoDraw(wxGraphicsContext &gc, bool suppressTooltips);
    virtual wxSharedPtr<wxVector<const wxChartsElement*>> GetActiveElements(const wxPoint &point);

private:
    class Point : public wxChartsPoint
    {
    public:
        typedef wxSharedPtr<Point> ptr;

        Point(wxPoint2DDouble value,
            const wxChartTooltipProvider::ptr tooltipProvider,
            wxDouble x, wxDouble y,
            const wxChartsPointOptions &options);

        virtual wxPoint2DDouble GetTooltipPosition() const;

        wxPoint2DDouble GetValue() const;

    private:
        wxPoint2DDouble m_value;
    };

    class Dataset
    {
    public:
        typedef wxSharedPtr<Dataset> ptr;

        Dataset();

        const wxVector<Point::ptr>& GetPoints() const;
        void AppendPoint(Point::ptr point);

    private:
        wxVector<Point::ptr> m_points;
    };

private:
    wxSharedPtr<wxScatterPlotOptions> m_options;
    wxChartsGrid m_grid;
    wxVector<Dataset::ptr> m_datasets;
};

#endif
