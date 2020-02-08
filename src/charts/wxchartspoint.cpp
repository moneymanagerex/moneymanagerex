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

#include "wxchartspoint.h"
#include <wx/pen.h>
#include <wx/brush.h>

wxChartsPoint::wxChartsPoint(wxDouble x,
                             wxDouble y, 
                             wxDouble radius,
                             const wxChartTooltipProvider::ptr tooltipProvider,
                             const wxChartsPointOptions &options)
    : wxChartsElement(tooltipProvider), m_options(options),
    m_position(x, y), m_radius(radius), m_hitDetectionRange(radius)
{
}

wxChartsPoint::wxChartsPoint(wxDouble x,
                             wxDouble y,
                             wxDouble radius,
                             wxDouble hitDetectionRange,
                             const wxChartTooltipProvider::ptr tooltipProvider,
                             const wxChartsPointOptions &options)
    : wxChartsElement(tooltipProvider), m_options(options),
    m_position(x, y), m_radius(radius), m_hitDetectionRange(hitDetectionRange)
{
}

void wxChartsPoint::Draw(wxGraphicsContext &gc) const
{
    wxGraphicsPath path = gc.CreatePath();
    path.AddArc(m_position.m_x, m_position.m_y, m_radius, 0, 2 * M_PI, false);
    path.CloseSubpath();

    wxBrush brush(m_options.GetFillColor());
    gc.SetBrush(brush);
    gc.FillPath(path);

    wxPen pen(m_options.GetStrokeColor(), m_options.GetStrokeWidth());
    gc.SetPen(pen);
    gc.StrokePath(path);
}

bool wxChartsPoint::HitTest(const wxPoint &point) const
{
    wxDouble distanceFromXCenterSquared = point.x - m_position.m_x;
    distanceFromXCenterSquared *= distanceFromXCenterSquared;
    wxDouble distanceFromYCenterSquared = point.y - m_position.m_y;
    distanceFromYCenterSquared *= distanceFromYCenterSquared;
    
    return ((distanceFromXCenterSquared + distanceFromYCenterSquared) <= (m_hitDetectionRange * m_hitDetectionRange));
}

wxPoint2DDouble wxChartsPoint::GetTooltipPosition() const
{
    return m_position;
}

const wxPoint2DDouble& wxChartsPoint::GetPosition() const
{
    return m_position;
}

void wxChartsPoint::SetPosition(wxDouble x, wxDouble y)
{
    m_position.m_x = x;
    m_position.m_y = y;
}

void wxChartsPoint::SetPosition(wxPoint2DDouble position)
{
    m_position = position;
}
