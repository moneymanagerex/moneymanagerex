/*
    Copyright (c) 2019 Xavier Leclercq

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

#ifndef _WX_CHARTS_WXLINECHARTDATASETOPTIONS_H_
#define _WX_CHARTS_WXLINECHARTDATASETOPTIONS_H_

#include "wxchartspenoptions.h"
#include "wxchartsbrushoptions.h"

class wxLineChartDatasetOptions
{
public:
    wxLineChartDatasetOptions(const wxColor &dotColor, const wxColor &dotStrokeColor,
        const wxColor &fillColor);

public:
    /// Whether to show the points on the chart.
    /// @retval true Show the points.
    /// @retval false Don't show the points.
    bool ShowDots() const;

    /// Gets the options for the pen used to draw the dots representing points on the chart.
    /**
        @return The options for the pen used to draw the dots representing points on the chart.
    */
    const wxChartsPenOptions& GetDotPenOptions() const;
    /// Gets the options for the brush used to draw the dots representing points on the chart.
    /**
        @return The options for the brush used to draw the dots representing points on the chart.
    */
    const wxChartsBrushOptions& GetDotBrushOptions() const;

    /// Gets the radius for the dots representing
    /// points on the chart.
    /// @return The radius of the dots.
    wxDouble GetDotRadius() const;

    /// Whether to show the line on the chart.
    /// @retval true Show the line.
    /// @retval false Don't show the line.
    bool ShowLine() const;

    const wxColor& GetLineColor() const;

    /// Gets the width of the pen used to
    /// draw the lines connecting the points
    /// of the chart. Width of the axes and grid
    /// lines are part of the options returned
    /// by the GetGridOptions() function.
    /// @return The width of the pen.
    unsigned int GetLineWidth() const;

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

private:
    bool m_showDots;
    wxChartsPenOptions m_dotPenOptions;
    wxChartsBrushOptions m_dotBrushOptions;
    wxDouble m_dotRadius;
    bool m_showLine;
    wxColor m_lineColor;
    unsigned int m_lineWidth;
    bool m_fill;
    wxColor m_fillColor;
};

#endif
