/*
    Copyright (c) 2016-2017 Xavier Leclercq

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

#include "wxchartbackground.h"
#include <wx/brush.h>

wxChartBackground::wxChartBackground(const wxChartBackgroundOptions &options)
    : m_options(options)
{
}

void wxChartBackground::Draw(const wxPoint2DDouble &position,
                             const wxSize &size,
                             wxGraphicsContext &gc)
{
    Draw(position.m_x, position.m_y, size.GetWidth(), size.GetHeight(), gc);
}

void wxChartBackground::Draw(wxDouble x, 
                             wxDouble y, 
                             wxDouble width, 
                             wxDouble height,
                             wxGraphicsContext &gc)
{
    wxGraphicsPath path = gc.CreatePath();

    path.AddRoundedRectangle(x, y, width, height, m_options.GetCornerRadius());

    wxBrush brush(m_options.GetColor());
    gc.SetBrush(brush);
    gc.FillPath(path);
}
