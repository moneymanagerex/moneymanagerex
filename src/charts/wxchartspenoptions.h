/*
    Copyright (c) 2019 Xavier Leclercq

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

#ifndef _WX_CHARTS_WXCHARTSPENOPTIONS_H_
#define _WX_CHARTS_WXCHARTSPENOPTIONS_H_

#include <wx/colour.h>
#include <wx/pen.h>

/// The options for a pen.
/**
    \ingroup elementclasses
*/
class wxChartsPenOptions
{
public:
    /// Constructor.
    /**
        @param color The color of the pen.
    */
    wxChartsPenOptions(const wxColor &color, int width,
        wxPenStyle style = wxPENSTYLE_SOLID);

    /// Gets the color of the pen.
    /**
        @return The color of the pen.
    */
    const wxColor& GetColor() const;

    /// Gets the width of the pen.
    /**
        @return The width of the pen.
    */
    int GetWidth() const;

    /// Gets the style of the pen.
    /**
        @return The style of the pen.
    */
    wxPenStyle GetStyle() const;

private:
    wxColor m_color;
    int m_width;
    wxPenStyle m_style;
};

#endif
