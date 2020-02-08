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

#ifndef _WX_CHARTS_WXBUBBLECHART_H_
#define _WX_CHARTS_WXBUBBLECHART_H_

#include "wxchart.h"
#include "wxbubblechartoptions.h"
#include "wxchartsgrid.h"
#include "wxchartscircle.h"

class wxDoubleTriplet
{
public:
    wxDoubleTriplet(wxDouble x, wxDouble y, wxDouble z);

    wxDouble m_x;
    wxDouble m_y;
    wxDouble m_z;
};

/// \ingroup dataclasses
class wxBubbleChartDataset
{
public:
    /// Smart pointer typedef.
    typedef wxSharedPtr<wxBubbleChartDataset> ptr;

    /// Constructs a wxBubbleChartDataset instance.
    /// @param data The list of values.
    wxBubbleChartDataset(const wxColor& fillColor, const wxColor& outlineColor,
        wxVector<wxDoubleTriplet> &data);

    const wxColor& GetFillColor() const;
    unsigned int GetOutlineWidth() const;
    const wxColor& GetOutlineColor() const;
    unsigned int GetMinRadius() const;
    unsigned int GetMaxRadius() const;

    const wxVector<wxDoubleTriplet>& GetData() const;

private:
    wxColor m_fillColor;
    unsigned int m_outlineWidth;
    wxColor m_outlineColor;
    unsigned int m_minRadius;
    unsigned int m_maxRadius;
    wxVector<wxDoubleTriplet> m_data;
};

/// Data for the wxBubbleChartCtrl control.

/// \ingroup dataclasses
class wxBubbleChartData
{
public:
    /// Constructs a wxBubbleChartData instance.
    wxBubbleChartData();

    /// Adds a dataset.
    /// @param dataset The dataset to add.
    void AddDataset(wxBubbleChartDataset::ptr dataset);

    const wxVector<wxBubbleChartDataset::ptr>& GetDatasets() const;

private:
    wxVector<wxBubbleChartDataset::ptr> m_datasets;
};

/// A bubble chart.
/**
    \ingroup chartclasses
*/
class wxBubbleChart : public wxChart
{
public:
    /// Constructor.
    /**
        The chart options will be defined by the default theme.

        @param data The data that will be used to initialize the chart.
        @param size The initial size of the chart.
    */
    wxBubbleChart(const wxBubbleChartData &data, const wxSize &size);

    /// Constructor.
    /**
        @param data The data that will be used to initialize the chart.
        @param options The options to use for the chart.
        @param size The initial size of the chart.
    */
    wxBubbleChart(const wxBubbleChartData &data, wxBubbleChartOptions::ptr options,
        const wxSize &size);

    virtual const wxChartCommonOptions& GetCommonOptions() const;

private:
    void Initialize(const wxBubbleChartData &data);
    static wxDouble GetMinXValue(const wxVector<wxBubbleChartDataset::ptr>& datasets);
    static wxDouble GetMaxXValue(const wxVector<wxBubbleChartDataset::ptr>& datasets);
    static wxDouble GetMinYValue(const wxVector<wxBubbleChartDataset::ptr>& datasets);
    static wxDouble GetMaxYValue(const wxVector<wxBubbleChartDataset::ptr>& datasets);
    static wxDouble GetMinZValue(const wxVector<wxBubbleChartDataset::ptr>& datasets);
    static wxDouble GetMaxZValue(const wxVector<wxBubbleChartDataset::ptr>& datasets);

    virtual void DoSetSize(const wxSize &size);
    virtual void DoFit();
    virtual void DoDraw(wxGraphicsContext &gc, bool suppressTooltips);
    virtual wxSharedPtr<wxVector<const wxChartsElement*>> GetActiveElements(const wxPoint &point);

private:
    class Circle : public wxChartsCircle
    {
    public:
        typedef wxSharedPtr<Circle> ptr;

        Circle(wxDoubleTriplet value, wxDouble x, wxDouble y, wxDouble radius,
            const wxChartTooltipProvider::ptr tooltipProvider,
            const wxChartsCircleOptions &options);

        wxDoubleTriplet GetValue() const;

    private:
        wxDoubleTriplet m_value;
    };

    class Dataset
    {
    public:
        typedef wxSharedPtr<Dataset> ptr;

        Dataset(unsigned int minRadius, unsigned int maxRadius);

        const wxVector<Circle::ptr>& GetCircles() const;
        void AppendCircle(Circle::ptr circle);

        unsigned int GetMinRadius() const;
        unsigned int GetMaxRadius() const;

    private:
        unsigned int m_minRadius;
        unsigned int m_maxRadius;
        wxVector<Circle::ptr> m_circles;
    };

private:
    wxSharedPtr<wxBubbleChartOptions> m_options;
    wxChartsGrid m_grid;
    wxVector<Dataset::ptr> m_datasets;
    wxDouble m_minZValue;
    wxDouble m_maxZValue;
};

#endif
