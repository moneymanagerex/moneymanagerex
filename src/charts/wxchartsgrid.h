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

/*
    Part of this file were copied from the Chart.js project (http://chartjs.org/)
    and translated into C++.

    The files of the Chart.js project have the following copyright and license.

    Copyright (c) 2013-2016 Nick Downie
    Released under the MIT license
    https://github.com/nnnick/Chart.js/blob/master/LICENSE.md
*/

/// @file

#ifndef _WX_CHARTS_WXCHARTSGRID_H_
#define _WX_CHARTS_WXCHARTSGRID_H_

#include "wxchartselement.h"
#include "wxchartsgridoptions.h"
#include "wxchartsgridmapping.h"
#include "wxchartsaxis.h"
#include <wx/graphics.h>

/// This class is used to display a grid.

/// It represents the grid displayed in the background
/// of some of the chart controls like for instance line and
/// bar charts.
///
/// Grid lines can be drawn to see points on the grid corresponding to tick mark values. These lines
/// are called major grid lines.
///
/// Minor grid lines can be drawn in between major grid lines when more precise readings are
/// needed. By default minor grid lines are not shown. Use the wxChartsGridOptions::GetNumberOfHorizontalMinorGridLinesBetweenTickMarks,
/// wxChartsGridOptions::SetNumberOfHorizontalMinorGridLinesBetweenTickMarks, wxChartsGridOptions::GetNumberOfVerticalMinorGridLinesBetweenTickMarks
/// and wxChartsGridOptions::SetNumberOfVerticalMinorGridLinesBetweenTickMarks to control the
/// display of minor grid lines.
///
///
/// The wxChartsGridOptions::ShowHorizontalGridLines, wxChartsGridOptions::ShowVerticalGridLines,
/// wxChartsGridOptions::SetShowHorizontalGridLines and wxChartsGridOptions::SetShowVerticalGridLines
/// functions can be used to control the visibility of grid lines.
///
/// @see wxChartsGridOptions
/// \ingroup elementclasses
class wxChartsGrid : public wxChartsElement
{
public:
    typedef wxSharedPtr<wxChartsGrid> ptr;

    /// Constructs a wxChartsGrid element.
    /// @param position The position of the top left corner
    /// of the chart.
    /// @param size The size of the area where the grid
    /// will be displayed.
    /// @param xAxis The X axis.
    /// @param yAxis The Y axis.
    /// @param options The settings to be used for the
    /// grid.
    wxChartsGrid(const wxPoint2DDouble &position, const wxSize &size,
        wxChartsAxis::ptr xAxis, wxChartsAxis::ptr yAxis,
        const wxChartsGridOptions& options);
    /// Constructs a wxChartsGrid element. The tick marks on the X and
    /// Y axes are computed automatically using the
    /// minimum and maximum values that need to be displayed on the grid.
    /// @param position The position of the top left corner
    /// of the chart.
    /// @param size The size of the area where the grid
    /// will be displayed.
    /// @param minXValue The minimum of the values that
    /// will be shown on the chart for the X coordinate.
    /// @param maxXValue The maximum of the values that
    /// will be shown on the chart for the X coordinate.
    /// @param minYValue The minimum of the values that
    /// will be shown on the chart for the Y coordinate.
    /// @param maxYValue The maximum of the values that
    /// will be shown on the chart for the Y coordinate.
    /// @param options The settings to be used for the
    /// grid.
    wxChartsGrid(const wxPoint2DDouble &position, const wxSize &size,
        wxDouble minXValue, wxDouble maxXValue,
        wxDouble minYValue, wxDouble maxYValue,
        const wxChartsGridOptions& options);

    virtual void Draw(wxGraphicsContext &gc) const;

    virtual bool HitTest(const wxPoint &point) const;

    virtual wxPoint2DDouble GetTooltipPosition() const;

    void Fit(wxGraphicsContext &gc);

    /// Resizes the grid.
    /// @param size The new size of the area where the grid
    /// is displayed.
    void Resize(const wxSize &size);

    /// Gets the mapping that allows external code
    /// to correctly position points on the grid.
    /// @return The grid mapping.
    const wxChartsGridMapping& GetMapping() const;

    const wxChartsGridOptions& GetOptions() const;
    void  SetOptions(const wxChartsGridOptions& opt);

    bool Scale(int coeff);
    void Shift(double dx,double dy);
    void ChangeCorners(wxDouble minX,wxDouble maxX,
        wxDouble minY,wxDouble maxY);
    void UpdateAxisLimit(const std::string& axisId, wxDouble min, wxDouble max);
    void ChangeLabels(const std::string& axisId, const wxVector<wxString> &labels, wxChartsAxisOptions options);

private:
    void Update();
    void CalculatePadding(const wxChartsAxis &xAxis,
        const wxChartsAxis &yAxis, wxDouble &left, wxDouble &right);
    static void DrawHorizontalGridLines(const wxChartsAxis &horizontalAxis, const wxChartsAxis &verticalAxis,
        const wxChartsGridLineOptions &options, wxGraphicsContext &gc);
    static void DrawVerticalGridLines(const wxChartsAxis &horizontalAxis, const wxChartsAxis &verticalAxis,
        const wxChartsGridLineOptions &options, wxGraphicsContext &gc);

private:
    struct AxisLimits
    {
        AxisLimits(wxDouble p1,wxDouble p2,wxDouble p3,wxDouble p4)
            : MinX(p1),MaxX(p2),MinY(p3),MaxY(p4) {}
        AxisLimits() {}
        wxDouble MinX;
        wxDouble MaxX;
        wxDouble MinY;
        wxDouble MaxY;
    };

private:
    wxChartsGridOptions m_options;
    wxPoint2DDouble m_position;
    wxChartsAxis::ptr m_XAxis;
    wxChartsAxis::ptr m_YAxis;
    wxChartsGridMapping m_mapping;
    // Whether something has changed and we
    // need to rearrange the chart
    bool m_needsFit;
    AxisLimits m_curAxisLimits;
    AxisLimits m_origAxisLimits;
};

#endif
