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

#ifndef _WX_CHARTS_WXCHARTSCIRCLE_H_
#define _WX_CHARTS_WXCHARTSCIRCLE_H_

#include "wxchartselement.h"
#include "wxchartscircleoptions.h"

/// This class is used to draw a circle on the chart.

/// \ingroup elementclasses
class wxChartsCircle : public wxChartsElement
{
public:
    wxChartsCircle(wxDouble x, wxDouble y, wxDouble radius,
        const wxChartTooltipProvider::ptr tooltipProvider,
        const wxChartsCircleOptions &options);

    virtual void Draw(wxGraphicsContext &gc) const;

    virtual bool HitTest(const wxPoint &point) const;

    virtual wxPoint2DDouble GetTooltipPosition() const;

    /// Sets the center of the circle.
    /// @param x The X coordinate of the center.
    /// @param y The Y coordinate of the center.
    void SetCenter(wxDouble x, wxDouble y);
    /// Sets the center of the circle.
    /// @param position The new center of the circle.
    void SetCenter(wxPoint2DDouble center);
    void SetRadius(wxDouble radius);

private:
    wxChartsCircleOptions m_options;
    wxDouble m_x;
    wxDouble m_y;
    wxDouble m_radius;
};

#endif
