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

#ifndef _WX_CHARTS_WXAREACHART_H_
#define _WX_CHARTS_WXAREACHART_H_

#include "wxchart.h"
#include "wxareachartoptions.h"
#include "wxchartsgrid.h"
#include "wxchartspoint.h"

/// \ingroup dataclasses
class wxAreaChartDataset
{
public:
    /// Smart pointer typedef.
    typedef wxSharedPtr<wxAreaChartDataset> ptr;

    /// Constructs a AreaChart instance.
    /// @param dotColor The color of the points.
    /// @param data The list of values.
    wxAreaChartDataset(wxVector<wxPoint2DDouble> &data);

    const wxVector<wxPoint2DDouble>& GetData() const;

private:
    wxVector<wxPoint2DDouble> m_data;
};

/// Data for the wxAreaChartCtrl control.

/// \ingroup dataclasses
class wxAreaChartData
{
public:
    /// Constructs a wxAreaChartData instance.
    wxAreaChartData();

    /// Adds a dataset.
    /// @param dataset The dataset to add.
    void AddDataset(wxAreaChartDataset::ptr dataset);

    const wxVector<wxAreaChartDataset::ptr>& GetDatasets() const;

private:
    wxVector<wxAreaChartDataset::ptr> m_datasets;
};

/// An area chart.
/**
    \ingroup chartclasses
*/
class wxAreaChart : public wxChart
{
public:
    /// Constructor.
    /**
        The chart options will be defined by the default theme.

        @param data The data that will be used to initialize the chart.
        @param size The initial size of the chart.
    */
    wxAreaChart(const wxAreaChartData &data, const wxSize &size);

    /// Constructor.
    /**
        @param data The data that will be used to initialize the chart.
        @param options The options to use for the chart.
        @param size The initial size of the chart.
    */
    wxAreaChart(const wxAreaChartData &data, wxAreaChartOptions::ptr &options,
        const wxSize &size);

    virtual const wxChartCommonOptions& GetCommonOptions() const;

    void Save(const wxString &filename, const wxBitmapType &type,
        const wxSize &size);

private:
    void Initialize(const wxAreaChartData &data);
    static wxDouble GetMinXValue(const wxVector<wxAreaChartDataset::ptr>& datasets);
    static wxDouble GetMaxXValue(const wxVector<wxAreaChartDataset::ptr>& datasets);
    static wxDouble GetMinYValue(const wxVector<wxAreaChartDataset::ptr>& datasets);
    static wxDouble GetMaxYValue(const wxVector<wxAreaChartDataset::ptr>& datasets);

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
            wxDouble x, wxDouble y,wxDouble radius,
            unsigned int strokeWidth,const wxColor &fillColor,
            wxDouble hitDetectionRange);

        virtual wxPoint2DDouble GetTooltipPosition() const;
        virtual bool HitTest(const wxPoint &point) const;

        wxPoint2DDouble GetValue() const;

    private:
        wxPoint2DDouble m_value;
        wxDouble m_hitDetectionRange;
    };

    class Dataset
    {
    public:
        typedef wxSharedPtr<Dataset> ptr;

        Dataset(bool showDots, bool showLine,
            const wxColor &lineColor);

        bool ShowDots() const;
        bool ShowLine() const;
        const wxColor& GetLineColor() const;

        const wxVector<Point::ptr>& GetPoints() const;
        void AppendPoint(Point::ptr point);

    private:
        bool m_showDots;
        bool m_showLine;
        wxColor m_lineColor;
        wxVector<Point::ptr> m_points;
    };

private:
    wxSharedPtr<wxAreaChartOptions> m_options;
    wxChartsGrid m_grid;
    wxVector<Dataset::ptr> m_datasets;
};

#endif
