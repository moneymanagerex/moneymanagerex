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

#include "wxchartsrectangle.h"
#include "wxchartsutilities.h"
#include <wx/pen.h>
#include <wx/brush.h>

wxChartsRectangle::wxChartsRectangle(wxDouble x,
                                     wxDouble y,
                                     const wxChartTooltipProvider::ptr tooltipProvider,
                                     const wxChartsRectangleOptions &options)
    : wxChartsElement(tooltipProvider), 
    m_position(x, y), m_width(0), m_height(0), m_options(options)
{
}

void wxChartsRectangle::Draw(wxGraphicsContext &gc) const
{
    wxGraphicsPath path = gc.CreatePath();

    path.AddRectangle(m_position.m_x, m_position.m_y, m_width, m_height);

    wxBrush brush(m_options.GetBrushOptions().GetColor());
    gc.SetBrush(brush);
    gc.FillPath(path);

    if (m_options.GetBorders() == wxALL)
    {
        wxPen pen = wxChartsUtilities::CreatePen(m_options.GetPenOptions());
        gc.SetPen(pen);
        gc.StrokePath(path);
    }
    else
    {
        wxPen pen = wxChartsUtilities::CreatePen(m_options.GetPenOptions());
        gc.SetPen(pen);

        int borders = m_options.GetBorders();
        if (borders & wxTOP)
        {
            gc.StrokeLine(m_position.m_x, m_position.m_y, 
                m_position.m_x + m_width, m_position.m_y);
        }
        if (borders & wxRIGHT)
        {
            gc.StrokeLine(m_position.m_x + m_width, m_position.m_y, 
                m_position.m_x + m_width, m_position.m_y + m_height);
        }
        if (borders & wxBOTTOM)
        {
            gc.StrokeLine(m_position.m_x, m_position.m_y + m_height,
                m_position.m_x + m_width, m_position.m_y + m_height);
        }
        if (borders & wxLEFT)
        {
            gc.StrokeLine(m_position.m_x, m_position.m_y,
                m_position.m_x, m_position.m_y + m_height);
        }
    }
}

bool wxChartsRectangle::HitTest(const wxPoint &point) const
{
    bool x = ((m_position.m_x <= point.x) && (point.x <= (m_position.m_x + m_width)));
    bool y = ((m_position.m_y <= point.y) && (point.y <= (m_position.m_y + m_height)));
    return (x && y);
}

wxPoint2DDouble wxChartsRectangle::GetTooltipPosition() const
{
    return wxPoint2DDouble(m_position.m_x + (m_width / 2),
        m_position.m_y + (m_height / 2));
}

const wxPoint2DDouble& wxChartsRectangle::GetPosition() const
{
    return m_position;
}

void wxChartsRectangle::SetPosition(wxDouble x, wxDouble y)
{
    m_position.m_x = x;
    m_position.m_y = y;
}

void wxChartsRectangle::SetPosition(wxPoint2DDouble position)
{
    m_position = position;
}

wxDouble wxChartsRectangle::GetWidth() const
{
    return m_width;
}

wxDouble wxChartsRectangle::GetHeight() const
{
    return m_height;
}

void wxChartsRectangle::SetSize(wxDouble width, wxDouble height)
{
    m_width = width;
    m_height = height;
}
