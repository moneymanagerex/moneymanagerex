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

#include "wxchartsarc.h"
#include <wx/pen.h>
#include <wx/brush.h>

wxChartsArc::wxChartsArc(wxDouble x,
                         wxDouble y, 
                         wxDouble startAngle,
                         wxDouble endAngle,
                         wxDouble outerRadius,
                         wxDouble innerRadius,
                         const wxString &tooltip,
                         const wxChartsArcOptions &options)
    : wxChartsElement(tooltip), m_x(x), m_y(y), 
    m_startAngle(startAngle), m_endAngle(endAngle), 
    m_outerRadius(outerRadius), m_innerRadius(innerRadius), 
    m_options(options)
{
    if (m_startAngle > (2 * M_PI))
    {
        m_startAngle -= 2 * M_PI;
    }
    if (m_endAngle > (2 * M_PI))
    {
        m_endAngle -= 2 * M_PI;
    }
}

void wxChartsArc::Draw(wxGraphicsContext &gc) const
{
    wxGraphicsPath path = gc.CreatePath();

    if (m_innerRadius > 0)
    {
        path.AddArc(m_x, m_y, m_innerRadius, m_startAngle, m_endAngle, true);
        path.AddArc(m_x, m_y, m_outerRadius, m_endAngle, m_startAngle, false);
    }
    else
    {
        path.AddArc(m_x, m_y, m_outerRadius, m_endAngle, m_startAngle, false);
        path.AddLineToPoint(m_x, m_y);
    }

    path.CloseSubpath();

    wxBrush brush(m_options.GetFillColor());
    gc.SetBrush(brush);
    gc.FillPath(path);

    wxPen pen(*wxWHITE, m_options.GetOutlineWidth());
    gc.SetPen(pen);
    gc.StrokePath(path);
}

bool wxChartsArc::HitTest(const wxPoint &point) const
{
    wxDouble distanceFromXCenter = point.x - m_x;
    wxDouble distanceFromYCenter = point.y - m_y;
    wxDouble radialDistanceFromCenter = sqrt((distanceFromXCenter * distanceFromXCenter) + (distanceFromYCenter * distanceFromYCenter));

    wxDouble angle = atan2(distanceFromYCenter, distanceFromXCenter);
    if (angle < 0)
    {
        angle += 2 * M_PI;
    }

    // Calculate wether the angle is between the start and the end angle
    bool betweenAngles = false;
    if (m_startAngle <= m_endAngle)
    {
        betweenAngles = ((angle >= m_startAngle) && (angle <= m_endAngle));
    }
    else
    {
        betweenAngles =
            (
                ((angle >= m_startAngle) && (angle <= (2 * M_PI)))
                ||
                ((angle >= 0) && (angle <= m_endAngle))
            );
    }

    // Ensure within the outside of the arc centre, but inside arc outer
    bool withinRadius = ((radialDistanceFromCenter >= m_innerRadius) && (radialDistanceFromCenter <= m_outerRadius));

    return (betweenAngles && withinRadius);
}

wxPoint2DDouble wxChartsArc::GetTooltipPosition() const
{
    wxDouble centreAngle = m_startAngle + (m_endAngle - m_startAngle) / 2;
    wxDouble rangeFromCentre = m_innerRadius + (m_outerRadius - m_innerRadius) / 2;
    wxDouble x = m_x + cos(centreAngle) * rangeFromCentre;
    wxDouble y = m_y + sin(centreAngle) * rangeFromCentre;
    return wxPoint2DDouble(x, y);
}

void wxChartsArc::SetCenter(wxDouble x, wxDouble y)
{
    m_x = x;
    m_y = y;
}

void wxChartsArc::SetAngles(wxDouble startAngle, wxDouble endAngle)
{
    m_startAngle = startAngle;
    if (m_startAngle > (2 * M_PI))
    {
        m_startAngle -= 2 * M_PI;
    }
    m_endAngle = endAngle;
    if (m_endAngle > (2 * M_PI))
    {
        m_endAngle -= 2 * M_PI;
    }
}

void wxChartsArc::SetRadiuses(wxDouble outerRadius, wxDouble innerRadius)
{
    m_outerRadius = outerRadius;
    m_innerRadius = innerRadius;
}

const wxChartsArcOptions& wxChartsArc::GetOptions() const
{
    return m_options;
}
