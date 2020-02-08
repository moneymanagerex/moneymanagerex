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

#ifndef _WX_CHARTS_WXCHARTSUTILITIES_H_
#define _WX_CHARTS_WXCHARTSUTILITIES_H_

#include "wxchartspenoptions.h"
#include "wxchartsbrushoptions.h"
#include "wxchartslabel.h"
#include <wx/pen.h>
#include <wx/brush.h>
#include <wx/graphics.h>

/// This class contains a set of static utility functions.
class wxChartsUtilities
{
public:
    /// Creates a pen from a wxChartsPenOptions instance.
    /**
        @return A pen with the specified options.
    */
    static wxPen CreatePen(const wxChartsPenOptions& options);

    /// Creates a brush from a wxChartsBrushOptions instance.
    /**
        @return A brush with the specified options.
    */
    static wxBrush CreateBrush(const wxChartsBrushOptions& options);

    static size_t GetDecimalPlaces();
    static void CalculateGridRange(wxDouble minValue,
        wxDouble maxValue, wxDouble &graphMinValue,
        wxDouble &graphMaxValue, wxDouble &valueRange, 
        size_t &steps, wxDouble &stepValue);
    static wxDouble CalculateOrderOfMagnitude(wxDouble value);
    static void BuildNumericalLabels(wxDouble minValue, size_t steps,
        wxDouble stepValue, const wxChartsLabelOptions &options,
        wxVector<wxChartsLabel> &labels);
    /// Gets the size of the text for the given font.
    /// @param gc The graphics context.
    /// @param font The font.
    /// @param string The text that we need to measure.
    /// @param width When the function returns this will contain
    /// the width of the string.
    /// @param height When the function returns this will contain
    /// the height of the string.
    static void GetTextSize(wxGraphicsContext &gc,
        const wxFont &font, const wxString &string,
        wxDouble &width, wxDouble &height);
};

#endif
