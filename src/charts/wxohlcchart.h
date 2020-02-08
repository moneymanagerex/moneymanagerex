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

#ifndef _WX_CHARTS_WXOHLCCHART_H_
#define _WX_CHARTS_WXOHLCCHART_H_

#include "wxchart.h"
#include "wxchartsohlcdata.h"
#include "wxohlcchartoptions.h"
#include "wxchartsgrid.h"

/// Data for the wxOHLCChartCtrl control.

/// \ingroup dataclasses
class wxOHLCChartData
{
public:
    wxOHLCChartData(const wxVector<wxString> &labels, const wxVector<wxChartsOHLCData> &data);

    /// Gets the labels of the X axis.
    /// @return A vector containing the labels of the
    /// X axis.
    const wxVector<wxString>& GetLabels() const;
    unsigned int GetLineWidth() const;
    const wxColor& GetUpLineColor() const;
    const wxColor& GetDownLineColor() const;
    unsigned int GetOpenLineLength() const;
    unsigned int GetCloseLineLength() const;
    const wxVector<wxChartsOHLCData>& GetData() const;

private:
    wxVector<wxString> m_labels;
    unsigned int m_lineWidth;
    wxColor m_upLineColor;
    wxColor m_downLineColor;
    unsigned int m_openLineLength;
    unsigned int m_closeLineLength;
    wxVector<wxChartsOHLCData> m_data;
};

/// An open-high-low-close chart.

/// \ingroup chartclasses
class wxOHLCChart : public wxChart
{
public:
    wxOHLCChart(const wxOHLCChartData &data, const wxSize &size);

    virtual const wxChartCommonOptions& GetCommonOptions() const;

private:
    static wxDouble GetMinValue(const wxOHLCChartData &data);
    static wxDouble GetMaxValue(const wxOHLCChartData &data);

    virtual void DoSetSize(const wxSize &size);
    virtual void DoFit();
    virtual void DoDraw(wxGraphicsContext &gc, bool suppressTooltips);
    virtual wxSharedPtr<wxVector<const wxChartsElement*>> GetActiveElements(const wxPoint &point);

private:
    class OHLCLines : public wxChartsElement
    {
    public:
        typedef wxSharedPtr<OHLCLines> ptr;

        OHLCLines(const wxChartsOHLCData &data, unsigned int lineWidth,
            const wxColor& upLineColor, const wxColor& downLineColor,
            unsigned int openLineLength, unsigned int closeLineLength,
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
        unsigned int m_lineWidth;
        wxColor m_upLineColor;
        wxColor m_downLineColor;
        unsigned int m_openLineLength;
        unsigned int m_closeLineLength;
    };

private:
    wxSharedPtr<wxOHLCChartOptions> m_options;
    wxChartsGrid m_grid;
    wxVector<OHLCLines::ptr> m_data;
};

#endif
