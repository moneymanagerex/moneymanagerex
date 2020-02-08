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

#ifndef _WX_CHARTS_WXCHARTTOOLTIPOPTIONS_H_
#define _WX_CHARTS_WXCHARTTOOLTIPOPTIONS_H_

#include "wxchartbackgroundoptions.h"
#include <wx/colour.h>
#include <wx/font.h>

/// The options for the wxChartTooltip element.
class wxChartTooltipOptions
{
public:
    /// Constructs a wxChartTooltipOptions instance.
    wxChartTooltipOptions();

    /// Gets the font family to be used for 
    /// the text.
    /// @return The font family.
    wxFontFamily GetFontFamily() const;
    /// Gets the font size to be used for 
    /// the text.
    /// @return The font size in pixels.
    int GetFontSize() const;
    /// Gets the font style to be used for 
    /// the text.
    /// @return The font style.
    wxFontStyle GetFontStyle() const;
    /// Gets the font color to be used for 
    /// the text.
    /// @return The font color.
    const wxColor& GetFontColor() const;

    /// Gets the horizontal padding. The padding will
    /// be added on the left and on the right of the text 
    /// and will be painted using the background color.
    /// @return The horizontal padding.
    wxDouble GetHorizontalPadding() const;
    /// Gets the vertical padding. The padding will
    /// be added above and below the text and will be
    /// painted using the background color.
    /// @return The vertical padding.
    wxDouble GetVerticalPadding() const;
    const wxChartBackgroundOptions& GetBackgroundOptions() const;

private:
    wxFontFamily m_fontFamily;
    int m_fontSize;
    wxFontStyle m_fontStyle;
    wxColor m_fontColor;
    wxDouble m_horizontalPadding;
    wxDouble m_verticalPadding;
    wxChartBackgroundOptions m_backgroundOptions;
};

#endif
