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

#ifndef _WX_CHARTS_WXCHARTSGRIDOPTIONS_H_
#define _WX_CHARTS_WXCHARTSGRIDOPTIONS_H_

#include "wxchartsaxisoptions.h"
#include <wx/colour.h>

/// The options for the grid lines of wxChartsGrid element.
class wxChartsGridLineOptions
{
public:
    /// Constructs a wxChartsGridOptions
    /// instance.
    wxChartsGridLineOptions();

    /// Whether to show grid lines. This doesn't affect
    /// the visibility of the X or Y axis.
    /// @retval true Show grid lines.
    /// @retval false Don't show grid lines.
    bool ShowGridLines() const;
    /// Sets whether grid lines should be shown.
    /// @param show True if grid lines should be shown, false
    /// otherwise.
    void SetShowGridLines(bool show);

    /// Gets the width of the major grid lines. This doesn't affect
    /// the width of the axes.
    /// @return The width of the major grid lines.
    unsigned int GetMajorGridLineWidth() const;
    /// Sets the width of the major grid lines.
    /// @param width The width of the major grid lines.
    void SetMajorGridLineWidth(unsigned int width);
    /// Gets the color of the major grid lines. This doesn't affect
    /// the color of the axes.
    /// @return The color of the major grid lines.
    const wxColor& GetMajorGridLineColor() const;
    /// Sets the color of the major grid lines.
    /// @param color The color of the major grid lines.
    void SetMajorGridLineColor(const wxColor &color);

    /// Gets the number of minor grid lines that should be drawn between
    /// major grid lines. By default this is 0 which means no minor lines
    /// will be drawn.
    /// @return The number of minor grid lines that should be drawn.
    unsigned int GetNumberOfMinorGridLinesBetweenTickMarks() const;
    /// Sets the number of minor grid lines to draw between
    /// major grid lines.
    /// @param n The number of minor grid lines to draw.
    void SetNumberOfMinorGridLinesBetweenTickMarks(unsigned int n);

private:
    // Whether to show grid lines (the X and Y axes are always shown)
    bool m_showGridLines;
    unsigned int m_majorGridLineWidth;
    wxColor m_majorGridLineColor;
    unsigned int m_numberOfMinorGridLinesBetweenTickMarks;
};

/// The options for the wxChartsGrid element.
class wxChartsGridOptions
{
public:
    /// Constructs a wxChartsGridOptions instance with
    /// default options for the X and Y axes.
    wxChartsGridOptions();
    /// Constructs a wxChartsGridOptions instance with
    /// the options for the X and Y axes passed in as
    /// arguments.
    /// @param xAxisOptions The options for the X axis.
    /// @param yAxisOptions The options for the Y axis.
    wxChartsGridOptions(const wxChartsAxisOptions &xAxisOptions,
        const wxChartsAxisOptions &yAxisOptions);

    /// Gets the options for the X axis.
    /// @return The options for the X axis.
    const wxChartsAxisOptions& GetXAxisOptions() const;
    /// Gets the options for the X axis.
    /// @return The options for the X axis.
    wxChartsAxisOptions& GetXAxisOptions();
    /// Gets the options for the Y axis.
    /// @return The options for the Y axis.
    const wxChartsAxisOptions& GetYAxisOptions() const;
    /// Gets the options for the Y axis.
    /// @return The options for the Y axis.
    wxChartsAxisOptions& GetYAxisOptions();

    /// Gets the options for the horizontal grid lines.
    /// @return The options for the horizontal grid lines.
    const wxChartsGridLineOptions& GetHorizontalGridLineOptions() const;
    /// Gets the options for the horizontal grid lines.
    /// @return The options for the horizontal grid lines.
    wxChartsGridLineOptions& GetHorizontalGridLineOptions();
    /// Gets the options for the vertical grid lines.
    /// @return The options for the vertical grid lines.
    const wxChartsGridLineOptions& GetVerticalGridLineOptions() const;
    /// Gets the options for the vertical grid lines.
    /// @return The options for the vertical grid lines.
    wxChartsGridLineOptions& GetVerticalGridLineOptions();

    void SetShowGridLines(bool show);

private:
    wxChartsAxisOptions m_XAxisOptions;
    wxChartsAxisOptions m_YAxisOptions;
    wxChartsGridLineOptions m_horizontalGridLineOptions;
    wxChartsGridLineOptions m_verticalGridLinesOptions;
};

#endif
