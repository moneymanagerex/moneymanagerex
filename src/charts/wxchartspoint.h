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

#ifndef _WX_CHARTS_WXCHARTSPOINT_H_
#define _WX_CHARTS_WXCHARTSPOINT_H_

#include "wxchartselement.h"
#include "wxchartspointoptions.h"
#include <wx/graphics.h>

/// This class is used to display a point on a chart.

/// \ingroup elementclasses
class wxChartsPoint : public wxChartsElement
{
public:
    /// Constructs a wxChartPoint element.
    /// @param x The X coordinate of the center of the point.
    /// @param y The Y coordinate of the center of the point.
    /// @param radius The radius of the circle used to represent
    /// the point.
    /// @param tooltipProvider The tooltip provider.
    /// @param options The settings to be used for the
    /// point.
    wxChartsPoint(wxDouble x, wxDouble y, wxDouble radius,
        const wxChartTooltipProvider::ptr tooltipProvider,
        const wxChartsPointOptions &options);
    wxChartsPoint(wxDouble x, wxDouble y, wxDouble radius,
        wxDouble hitDetectionRange,
        const wxChartTooltipProvider::ptr tooltipProvider,
        const wxChartsPointOptions &options);

    virtual void Draw(wxGraphicsContext &gc) const;

    virtual bool HitTest(const wxPoint &point) const;

    virtual wxPoint2DDouble GetTooltipPosition() const;

    /// Gets the position of the point.
    /// @return The position of the point.
    const wxPoint2DDouble& GetPosition() const;
    /// Sets the position of the point.
    /// @param x The new X coordinate of the point.
    /// @param y The new Y coordinate of the point.
    void SetPosition(wxDouble x, wxDouble y);
    /// Sets the position of the point.
    /// @param position The new position of the point.
    void SetPosition(wxPoint2DDouble position);

private:
    wxChartsPointOptions m_options;
    wxPoint2DDouble m_position;
    wxDouble m_radius;
    wxDouble m_hitDetectionRange;
};

#endif
