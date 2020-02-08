/*
    Copyright (c) 2016-2019 Xavier Leclercq and the wxCharts contributors.

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

#ifndef _WX_CHARTS_WXMATH2DPLOTOPTIONS_H_
#define _WX_CHARTS_WXMATH2DPLOTOPTIONS_H_

#include "wxchartoptions.h"
#include "wxchartsgridoptions.h"

#include <functional>

typedef std::function<wxDouble(wxDouble)> AxisFunc;

/// The options for the wxMath2DPlotCtrl control.

/// \ingroup chartclasses
class wxMath2DPlotOptions : public wxChartOptions
{
public:
    /// Constructs a wxMath2DPlotOptions instance.
    wxMath2DPlotOptions();

    /// Gets the options for the grid.
    /// @return The options for the grid.
    const wxChartsGridOptions& GetGridOptions() const;
    /// Gets the options for the grid.
    /// @return The grid options.
    wxChartsGridOptions& GetGridOptions();
    /// Gets the radius for the dots representing
    /// points on the chart.
    /// @return The radius of the dots.
    wxDouble GetDotRadius() const;
    /// Gets the width of the pen used to draw
    /// the outline of the dots.
    /// @return The width of the pen.
    unsigned int GetDotStrokeWidth() const;
    /// Gets the width of the pen used to
    /// draw the lines connecting the points
    /// of the chart. Width of the axes and grid
    /// lines are part of the options returned
    /// by the GetGridOptions() function.
    /// @return The width of the pen.
    unsigned int GetLineWidth() const;

    /// Gets the hit detection range. When the
    /// distance between the X coordinate of
    /// the mouse position and a point on the
    /// chart is less than this range the
    /// point is considered active. The Y
    /// coordinate is ignored in this calculation.
    /// @return The hit detection range.
    wxDouble GetHitDetectionRange() const;

    const AxisFunc& GetAxisFuncX() const;
    const AxisFunc& GetAxisFuncY() const;
    void SetAxisFuncX(const AxisFunc &newfunc);
    void SetAxisFuncY(const AxisFunc &newfunc);

private:
    wxChartsGridOptions m_gridOptions;
    wxDouble m_dotRadius;
    unsigned int m_dotStrokeWidth;
    unsigned int m_lineWidth;
    wxDouble m_hitDetectionRange;
    AxisFunc m_axisFuncX;
    AxisFunc m_axisFuncY;
};

#endif
