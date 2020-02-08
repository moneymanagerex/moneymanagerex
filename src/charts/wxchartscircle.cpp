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

#include "wxchartscircle.h"
#include <wx/pen.h>
#include <wx/brush.h>

wxChartsCircle::wxChartsCircle(wxDouble x, 
                               wxDouble y, 
                               wxDouble radius,
                               const wxChartTooltipProvider::ptr tooltipProvider,
                               const wxChartsCircleOptions &options)
    : wxChartsElement(tooltipProvider), m_options(options),
    m_x(x), m_y(y), m_radius(radius)
{
}

void wxChartsCircle::Draw(wxGraphicsContext &gc) const
{
    wxGraphicsPath path = gc.CreatePath();
    
    path.AddCircle(m_x, m_y, m_radius);
        
    wxBrush brush(m_options.GetFillColor());
    gc.SetBrush(brush);
    gc.FillPath(path);

    wxPen pen(m_options.GetOutlineColor(), m_options.GetOutlineWidth());
    gc.SetPen(pen);
    gc.StrokePath(path);
}

bool wxChartsCircle::HitTest(const wxPoint &point) const
{
    wxDouble distanceFromXCenter = point.x - m_x;
    wxDouble distanceFromYCenter = point.y - m_y;
    wxDouble radialDistanceFromCenter = sqrt((distanceFromXCenter * distanceFromXCenter) + (distanceFromYCenter * distanceFromYCenter));

    return (radialDistanceFromCenter <= m_radius);
}

wxPoint2DDouble wxChartsCircle::GetTooltipPosition() const
{
    return wxPoint2DDouble(m_x, m_y);
}

void wxChartsCircle::SetCenter(wxDouble x, wxDouble y)
{
    m_x = x;
    m_y = y;
}

void wxChartsCircle::SetCenter(wxPoint2DDouble center)
{
    m_x = center.m_x;
    m_y = center.m_y;
}

void wxChartsCircle::SetRadius(wxDouble radius)
{
    m_radius = radius;
}
