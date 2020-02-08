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

#ifndef _WX_CHARTS_WXRADARCHART_H_
#define _WX_CHARTS_WXRADARCHART_H_

#include "wxchart.h"
#include "wxradarchartoptions.h"
#include "wxchartsradialgrid.h"

/// Data for the wxRadarChartCtrl control.
class wxRadarChartData
{
public:
    /// Constructs a wxRadarChartData instance.
    wxRadarChartData(const wxVector<wxString> &labels);

    /// Gets the labels of the X axis.
    /// @return A vector containing the labels of the
    /// X axis.
    const wxVector<wxString>& GetLabels() const;

private:
    wxVector<wxString> m_labels;
};

/// A radar chart.

/// \ingroup chartclasses
class wxRadarChart : public wxChart
{
public:
    wxRadarChart(const wxSize &size);

    virtual const wxChartCommonOptions& GetCommonOptions() const;

private:
    static wxDouble GetMinValue();
    static wxDouble GetMaxValue();

    virtual void DoSetSize(const wxSize &size);
    virtual void DoFit();
    virtual void DoDraw(wxGraphicsContext &gc, bool suppressTooltips);
    virtual wxSharedPtr<wxVector<const wxChartsElement*>> GetActiveElements(const wxPoint &point);

private:
    wxSharedPtr<wxRadarChartOptions> m_options;
    wxChartsRadialGrid m_grid;
};

#endif
