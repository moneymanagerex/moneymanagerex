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

#ifndef _WX_CHARTS_WXCHARTFONTOPTIONS_H_
#define _WX_CHARTS_WXCHARTFONTOPTIONS_H_

#include <wx/colour.h>
#include <wx/font.h>

/// Font settings.

/// Many elements display text and require
/// font settings. This class groups all
/// settings related to the choice of a font.
class wxChartFontOptions
{
public:
    /// Constructs a wxChartFontOptions instance.
    /// @param family The font family.
    /// @param size The font size in pixels.
    /// @param style The font style.
    /// @param weight The font weight.
    /// @param color The color of the text.
    wxChartFontOptions(wxFontFamily family, int size,
        wxFontStyle style, wxFontWeight weight,
        const wxColor &color);

    wxFont GetFont() const;

    /// Gets the font family.
    /// @return The font family.
    wxFontFamily GetFamily() const;
    /// Gets the font size.
    /// @return The font size in pixels.
    int GetSize() const;
    /// Gets the font style.
    /// @return The font style.
    wxFontStyle GetStyle() const;
    /// Gets the font weight.
    /// @return The font weight.
    wxFontWeight GetWeight() const;
    /// Gets the font color.
    /// @return The font color.
    const wxColor& GetColor() const;

private:
    wxFontFamily m_family;
    int m_size;
    wxFontStyle m_style;
    wxFontWeight m_weight;
    wxColor m_color;
};

#endif
