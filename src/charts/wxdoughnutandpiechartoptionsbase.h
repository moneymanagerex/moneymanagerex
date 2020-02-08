/*
    Copyright (c) 2016-2017 Xavier Leclercq

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

#ifndef _WX_CHARTS_WXDOUGHNUTANDPIECHARTOPTIONSBASE_H_
#define _WX_CHARTS_WXDOUGHNUTANDPIECHARTOPTIONSBASE_H_

#include "wxchartoptions.h"

/// Options for the wxDoughnutAndPieChartBase class. 
class wxDoughnutAndPieChartOptionsBase : public wxChartOptions
{
public:
    /// Constructs a wxDoughnutAndPieChartOptionsBase instance.
    /// @param percentageInnerCutout The size of the inner radius
    /// expressed as a percentage of the outer radius.
    wxDoughnutAndPieChartOptionsBase(unsigned int percentageInnerCutout);

    /// Gets the width of the pen used to draw the outline
    /// of the slices.
    /// @return The width of the pen.
    unsigned int GetSliceStrokeWidth() const;
    /// Gets the size of the inner radius.
    /// @return The size of the inner radius
    /// expressed as a percentage of the outer radius.
    unsigned int GetPercentageInnerCutout() const;

protected:
    /// Sets the width of the pen used to draw the outline
    /// of the slices.
    /// @param strokeWidth The width of the pen.
    void SetSliceStrokeWidth(unsigned int strokeWidth);
    /// Sets the size of the inner radius.
    /// @param cutout The size of the inner radius
    /// expressed as a percentage of the outer radius.
    void SetPercentageInnerCutout(unsigned int cutout);

private:
    // The width of each slice stroke, this will increase the 
    // space between the slices themselves.
    unsigned int m_sliceStrokeWidth;
    // The percentage of the chart that we cut out of the middle.
    unsigned int m_percentageInnerCutout;
};

#endif
