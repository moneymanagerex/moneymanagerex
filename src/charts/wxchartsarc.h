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

#ifndef _WX_CHARTS_WXCHARTSARC_H_
#define _WX_CHARTS_WXCHARTSARC_H_

#include "wxchartselement.h"
#include "wxchartsarcoptions.h"
#include <wx/graphics.h>

/// This class is used to draw one of the segments of a circular chart e.g. a pie chart or a doughnut chart.

/// \ingroup elementclasses
class wxChartsArc : public wxChartsElement
{
public:
    /// Constructs a wxChartsArc element. The arc is defined by the 
    /// coordinates of its centre (x, y), its inner radius and its 
    /// outer radius. The arc goes from the starting angle startAngle
    /// to endAngle clockwise. The angles are measured in radians but,
    /// contrary to the usual mathematical convention, are always 
    /// clockwise from the horizontal axis.
    /// @param x The X coordinate of the center of the arc.
    /// @param y The Y coordinate of the center of the arc.
    /// @param startAngle The angle (in radians) at which the arc starts.
    /// @param endAngle The angle (in radians) at which the arc ends.
    /// @param outerRadius The outer radius of the arc.
    /// @param innerRadius The inner radius of the arc. This can be 0.
    /// @param tooltip The tooltip.
    /// @param options The settings to be used for the arc.
    wxChartsArc(wxDouble x, wxDouble y, wxDouble startAngle,
        wxDouble endAngle, wxDouble outerRadius, wxDouble innerRadius,
        const wxString &tooltip, const wxChartsArcOptions &options);

    virtual void Draw(wxGraphicsContext &gc) const;

    virtual bool HitTest(const wxPoint &point) const;

    virtual wxPoint2DDouble GetTooltipPosition() const;

    /// Sets the center of the arc.
    /// @param x The X coordinate of the center.
    /// @param y The Y coordinate of the center.
    void SetCenter(wxDouble x, wxDouble y);
    /// Sets the start and end angles. The angles are measured
    /// in radians, they start from the 3 o'clock position
    /// and follow a clockwise direction.
    /// @param startAngle The start angle in radians. This must be a
    /// value between 0 and 2*PI.
    /// @param endAngle The end angle in radians. This must be greater
    /// than the start angle.
    void SetAngles(wxDouble startAngle, wxDouble endAngle);
    /// Sets the inner and outer radiuses.
    /// @param outerRadius The new outer radius.
    /// @param innerRadius The new inner radius.
    void SetRadiuses(wxDouble outerRadius, wxDouble innerRadius);

    /// Gets the arc options.
    /// @return The arc options.
    const wxChartsArcOptions& GetOptions() const;

private:
    wxChartsArcOptions m_options;
    wxDouble m_x;
    wxDouble m_y;
    wxDouble m_startAngle;
    wxDouble m_endAngle;
    wxDouble m_outerRadius;
    wxDouble m_innerRadius;
};

#endif
