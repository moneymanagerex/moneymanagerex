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

#ifndef _WX_CHARTS_WXCHARTBACKGROUNDOPTIONS_H_
#define _WX_CHARTS_WXCHARTBACKGROUNDOPTIONS_H_

#include <wx/colour.h>

/// The options for the wxChartBackground class.
class wxChartBackgroundOptions
{
public:
    wxChartBackgroundOptions(const wxColor &color, wxDouble cornerRadius);

    /// Gets the background color.
    /// @return The background color.
    const wxColor& GetColor() const;
    /// Gets the corner radius. The background
    /// is a rounded rectangle. This gets the radius
    /// used for the rounded corners.
    /// @return The corner radius.
    wxDouble GetCornerRadius() const;

private:
    wxColor m_color;
    wxDouble m_cornerRadius;
};

#endif
