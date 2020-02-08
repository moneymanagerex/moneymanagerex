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

#ifndef _WX_CHARTS_WXCHARTSNUMERICALAXIS_H_
#define _WX_CHARTS_WXCHARTSNUMERICALAXIS_H_

#include "wxchartsaxis.h"

enum wxChartsAxisScale
{
    wxCHARTSAXISSCALE_LINEAR = 0,
    wxCHARTSAXISSCALE_LOGARITHMIC = 1
};

/// \ingroup elementclasses
class wxChartsNumericalAxis : public wxChartsAxis
{
public:
    /// Constructs a wxChartsNumericalAxis axis. The
    /// tick marks on the axis are computed automatically 
    /// using the minimum and maximum values that need to
    /// be displayed on the grid.
    /// @param minYValue The minimum of the values that
    /// will be shown on the axis.
    /// @param maxYValue The maximum of the values that
    /// will be shown on the axis.
    /// @param options The settings to be used for the
    /// axis.
    wxChartsNumericalAxis(const std::string &id, wxDouble minValue, wxDouble maxValue,
        const wxChartsAxisOptions &options);
    static ptr make_shared(const std::string &id, wxDouble minValue, wxDouble maxValue,
        const wxChartsAxisOptions &options);

    wxDouble GetMinValue() const;
    void SetMinValue(wxDouble minValue);
    wxDouble GetMaxValue() const;
    void SetMaxValue(wxDouble maxValue);

private:
    wxDouble m_minValue;
    wxDouble m_maxValue;
};

#endif
