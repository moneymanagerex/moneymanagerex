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

/// @file

#ifndef _WX_CHARTS_WXAREACHARTOPTIONS_H_
#define _WX_CHARTS_WXAREACHARTOPTIONS_H_

#include "wxchartoptions.h"
#include "wxchartsgridoptions.h"
#include <wx/sharedptr.h>

/// The options for the wxAreaChartCtrl control.
/**
    \ingroup chartclasses
*/
class wxAreaChartOptions : public wxChartOptions
{
public:
    /// Smart pointer typedef.
    typedef wxSharedPtr<wxAreaChartOptions> ptr;

    /// Constructor.
    wxAreaChartOptions();

    /// Gets the options for the grid.
    /**
        @return The options for the grid.
    */
    const wxChartsGridOptions& GetGridOptions() const;

    /// Gets the options for the grid.
    /**
        @return The options for the grid.
    */
    wxChartsGridOptions& GetGridOptions();

    /// Gets the radius for the dots representing points on the chart.
    /**
        @return The radius of the dots.
    */
    wxDouble GetDotRadius() const;

    /// Gets the width of the pen used to draw the outline of the dots.
    /**
        @return The width of the pen.
    */
    unsigned int GetDotStrokeWidth() const;

    /// Gets the width of the pen used to draw the lines connecting the points of the chart.
    /**
        Width of the axes and grid lines are part of the options returned by the
        GetGridOptions() function.

        @return The width of the pen.
    */
    unsigned int GetLineWidth() const;

    /// Gets the hit detection range.
    /**
        When the distance between the X coordinate of the mouse position and a point on the
        chart is less than this range the point is considered active. The Y coordinate is
        ignored in this calculation.
        
        @return The hit detection range.
    */
    wxDouble GetHitDetectionRange() const;

private:
    wxChartsGridOptions m_gridOptions;
    wxDouble m_dotRadius;
    unsigned int m_dotStrokeWidth;
    unsigned int m_lineWidth;
    wxDouble m_hitDetectionRange;
};

#endif
