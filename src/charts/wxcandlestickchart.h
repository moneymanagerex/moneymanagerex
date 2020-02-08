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

#ifndef _WX_CHARTS_WXCANDLESTICKCHART_H_
#define _WX_CHARTS_WXCANDLESTICKCHART_H_

#include "wxchart.h"
#include "wxchartsohlcdata.h"
#include "wxcandlestickchartoptions.h"
#include "wxchartsgrid.h"

/// Data for the wxCandlestickChartCtrl control.

/// \ingroup dataclasses
class wxCandlestickChartData
{
public:
    wxCandlestickChartData(const wxVector<wxString> &labels, const wxVector<wxChartsOHLCData> &data);

    /// Gets the labels of the X axis.
    /// @return A vector containing the labels of the
    /// X axis.
    const wxVector<wxString>& GetLabels() const;
    const wxColor& GetLineColor() const;
    unsigned int GetLineWidth() const;
    const wxColor& GetUpFillColor() const;
    const wxColor& GetDownFillColor() const;
    unsigned int GetRectangleWidth() const;
    const wxVector<wxChartsOHLCData>& GetData() const;

private:
    wxVector<wxString> m_labels;
    wxColor m_lineColor;
    unsigned int m_lineWidth;
    wxColor m_upFillColor;
    wxColor m_downFillColor;
    unsigned int m_rectangleWidth;
    wxVector<wxChartsOHLCData> m_data;
};

/// A candlestick chart.
/**
    \ingroup chartclasses
*/
class wxCandlestickChart : public wxChart
{
public:
    /// Constructor.
    /**
        The chart options will be defined by the default theme.

        @param data The data that will be used to initialize the chart.
        @param size The initial size of the chart.
    */
    wxCandlestickChart(const wxCandlestickChartData &data, const wxSize &size);

    /// Constructor.
    /**
        @param data The data that will be used to initialize the chart.
        @param options The options to use for the chart.
        @param size The initial size of the chart.
    */
    wxCandlestickChart(const wxCandlestickChartData &data, wxCandlestickChartOptions::ptr options,
        const wxSize &size);

    virtual const wxChartCommonOptions& GetCommonOptions() const;

private:
    void Initialize(const wxCandlestickChartData &data);
    static wxDouble GetMinValue(const wxCandlestickChartData &data);
    static wxDouble GetMaxValue(const wxCandlestickChartData &data);

    virtual void DoSetSize(const wxSize &size);
    virtual void DoFit();
    virtual void DoDraw(wxGraphicsContext &gc, bool suppressTooltips);
    virtual wxSharedPtr<wxVector<const wxChartsElement*>> GetActiveElements(const wxPoint &point);

private:
    class Candlestick : public wxChartsElement
    {
    public:
        typedef wxSharedPtr<Candlestick> ptr;

        Candlestick(const wxChartsOHLCData &data, const wxColor &lineColor, unsigned int lineWidth,
            const wxColor &upFillColor, const wxColor &downFillColor, unsigned int rectangleWidth,
            const wxChartTooltipProvider::ptr tooltipProvider);

        virtual void Draw(wxGraphicsContext &gc) const;
        virtual bool HitTest(const wxPoint &point) const;
        virtual wxPoint2DDouble GetTooltipPosition() const;

        void Update(const wxChartsGridMapping& mapping, size_t index);

    private:
        wxChartsOHLCData m_data;
        wxPoint2DDouble m_lowPoint;
        wxPoint2DDouble m_highPoint;
        wxPoint2DDouble m_openPoint;
        wxPoint2DDouble m_closePoint;
        wxColor m_lineColor;
        unsigned int m_lineWidth;
        wxColor m_upFillColor;
        wxColor m_downFillColor;
        unsigned int m_rectangleWidth;
    };

private:
    wxSharedPtr<wxCandlestickChartOptions> m_options;
    wxChartsGrid m_grid;
    wxVector<Candlestick::ptr> m_data;
};

#endif
