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

#ifndef _WX_CHARTS_WXCHARTMULTITOOLTIPOPTIONS_H_
#define _WX_CHARTS_WXCHARTMULTITOOLTIPOPTIONS_H_

#include "wxchartfontoptions.h"

/// The options for the wxChartMultiTooltip element.
class wxChartMultiTooltipOptions
{
public:
    /// Constructs a wxChartMultiTooltipOptions instance.
    wxChartMultiTooltipOptions();

    bool ShowTitle() const;
    void SetShowTitle(bool show);
    const wxChartFontOptions& GetTitleFontOptions() const;
    const wxChartFontOptions& GetTextFontOptions() const;

    wxAlignment GetAlignment() const;
    void SetAlignment(wxAlignment alignment);

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
    /// Gets the background color.
    /// @return The background color.
    const wxColor& GetBackgroundColor() const;
    /// Gets the corner radius. The tooltip backround
    /// is a rounded rectangle. This gets the radius
    /// used for the rounded corners.
    /// @return The corner radius.
    wxDouble GetCornerRadius() const;
    wxDouble GetLineSpacing() const;

private:
    bool m_showTitle;
    wxChartFontOptions m_titleFontOptions;
    wxChartFontOptions m_textFontOptions;
    wxAlignment m_alignment;
    wxDouble m_horizontalPadding;
    wxDouble m_verticalPadding;
    wxColor m_backgroundColor;
    wxDouble m_cornerRadius;
    wxDouble m_lineSpacing;
};

#endif
