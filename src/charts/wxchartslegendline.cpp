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

#include "wxchartslegendline.h"
#include "wxchartsutilities.h"
#include <wx/brush.h>

wxChartsLegendLine::wxChartsLegendLine(const wxColor &color,
                                       const wxString &text,
                                       const wxChartsLegendLineOptions& options)
    : m_options(options), m_position(0, 0), m_size(0, 0),
    m_color(color), m_text(text)
{
}

void wxChartsLegendLine::Draw(wxGraphicsContext &gc) const
{
    wxFont font = m_options.GetFontOptions().GetFont();
    gc.SetFont(font, m_options.GetFontOptions().GetColor());

    wxDouble fontSize = m_options.GetFontOptions().GetSize();

    wxGraphicsPath backgroundPath = gc.CreatePath();
    backgroundPath.AddRoundedRectangle(m_position.m_x, m_position.m_y, fontSize + 2, fontSize + 2, 3);
    wxBrush backgroundBrush(*wxWHITE);
    gc.SetBrush(backgroundBrush);
    gc.FillPath(backgroundPath);

    wxGraphicsPath path = gc.CreatePath();
    path.AddRoundedRectangle(m_position.m_x, m_position.m_y, fontSize + 2, fontSize + 2, 3);
    wxBrush brush(m_color);
    gc.SetBrush(brush);
    gc.FillPath(path);

    gc.DrawText(m_text, m_position.m_x + 20, m_position.m_y);
}

bool wxChartsLegendLine::HitTest(const wxPoint &point) const
{
    return false;
}

wxPoint2DDouble wxChartsLegendLine::GetTooltipPosition() const
{
    return wxPoint2DDouble(0, 0);
}

const wxPoint2DDouble& wxChartsLegendLine::GetPosition() const
{
    return m_position;
}

void wxChartsLegendLine::SetPosition(wxDouble x, wxDouble y)
{
    m_position.m_x = x;
    m_position.m_y = y;
}

const wxSize& wxChartsLegendLine::GetSize() const
{
    return m_size;
}

void wxChartsLegendLine::UpdateSize(wxGraphicsContext &gc)
{
    wxDouble width;
    wxDouble height;
    wxChartsUtilities::GetTextSize(gc, m_options.GetFontOptions().GetFont(), m_text, width, height);
    m_size.x = width + 20;
    m_size.y = height;
}

const wxString& wxChartsLegendLine::GetText() const
{
    return m_text;
}

void wxChartsLegendLine::SetText(const wxString &text)
{
    m_text = text;
}
