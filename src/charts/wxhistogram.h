/*
    Copyright (c) 2018-2019 Xavier Leclercq and the wxCharts contributors.

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

#ifndef _WX_CHARTS_WXHISTOGRAM_H_
#define _WX_CHARTS_WXHISTOGRAM_H_

#include "wxchart.h"
#include "wxhistogramoptions.h"
#include "wxchartsgrid.h"
#include "wxchartspoint.h"

#include <wx/scopedptr.h>

/// \ingroup dataclasses
class wxHistogramDataset
{
public:
    /// Smart pointer typedef.
    typedef wxSharedPtr<wxHistogramDataset> ptr;

    /// Constructs a HistogramChart instance.
    wxHistogramDataset(const wxColor &lineColor, const wxColor &fillColor,
        wxVector<wxDouble> &data);

    const wxColor& GetLineColor() const;
    /// Whether to fill the part of the chart
    /// between the line and X-axis with the
    /// color returned by GetFillColor().
    /// @retval true Fill.
    /// @retval false Don't fill.
    bool Fill() const;
    /// Returns the color with which to
    /// fill the part of the chart between
    /// the line and the X-axis.
    /// @return The fill color.
    const wxColor& GetFillColor() const;
    const wxVector<wxDouble>& GetData() const;

private:
    bool m_fill;
    wxColor m_fillColor;
    wxColor m_lineColor;
    wxVector<wxDouble> m_data;
};

/// Data for the wxHistogramCtrl control.

/// \ingroup dataclasses
class wxHistogramData
{
public:
    /// Constructs a wxHistogramData instance.
    wxHistogramData(wxHistogramDataset::ptr dataset, std::size_t n = 10);

    const wxHistogramDataset::ptr& GetDataset() const;
    std::size_t GetNBins() const;

private:
    wxHistogramDataset::ptr m_dataset;
    std::size_t m_nbins;
};

/// A histogram.

/// \ingroup chartclasses
class wxHistogram : public wxChart
{
public:
    wxHistogram(const wxHistogramData &data, const wxSize &size);
    wxHistogram(const wxHistogramData &data, wxSharedPtr<wxHistogramOptions> &options,
        const wxSize &size);

    virtual const wxChartCommonOptions& GetCommonOptions() const;

    void Save(const wxString &filename, const wxBitmapType &type,
        const wxSize &size);

private:
    void Initialize(const wxHistogramData &data);

    virtual void DoSetSize(const wxSize &size);
    virtual void DoFit();
    virtual void DoDraw(wxGraphicsContext &gc, bool suppressTooltips);
    virtual wxSharedPtr<wxVector<const wxChartsElement*>> GetActiveElements(const wxPoint &point);

private:
    struct Bucket
    {
        wxDouble m_begin;
        wxDouble m_end;
        std::size_t m_count;
    };

    class Dataset
    {
    public:
        Dataset(wxDouble min,wxDouble max,std::size_t n,
            const wxColor &lineColor,const wxColor &fillColor);

        bool Fill() const;
        const wxColor& GetFillColor() const;
        const wxColor& GetLineColor() const;
        const wxVector<Bucket>& GetBuckets() const;
        void AppendValue(wxDouble value);

    private:
        bool m_fill;
        wxColor m_fillColor;
        wxColor m_lineColor;
        wxVector<Bucket> m_buckets;
    };

private:
    wxSharedPtr<wxHistogramOptions> m_options;
    wxChartsGrid m_grid;
    wxScopedPtr<Dataset> m_dataset;
};

#endif
