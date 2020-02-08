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

#ifndef _WX_CHARTS_WXCHARTSLEGENDLINE_H_
#define _WX_CHARTS_WXCHARTSLEGENDLINE_H_

#include "wxchartselement.h"
#include "wxchartslegendlineoptions.h"

/// This class is used to draw a line in a legend.

/// It can be used wherever a single line of text is needed
/// with a single colored square at the start of the line.
class wxChartsLegendLine : public wxChartsElement
{
public:
    /// Constructs a wxChartsLegendLine element.
    /// @param color The color associated with this
    /// item on the chart.
    /// @param text The text of the legend.
    /// @param options The settings to be used for the
    /// legend line.
    wxChartsLegendLine(const wxColor &color, const wxString &text, 
        const wxChartsLegendLineOptions& options);

    virtual void Draw(wxGraphicsContext &gc) const;
    
    virtual bool HitTest(const wxPoint &point) const;

    virtual wxPoint2DDouble GetTooltipPosition() const;

    const wxPoint2DDouble& GetPosition() const;
    void SetPosition(wxDouble x, wxDouble y);
    /// Gets the size of the line. The size must
    /// have been previously computed by calling
    /// the UpdateSize(wxGraphicsContext &gc) function.
    /// @return The size of the line.
    const wxSize& GetSize() const;
    /// Computes the size of the line based on the 
    /// contents and options.
    /// @param gc The graphics context.
    void UpdateSize(wxGraphicsContext &gc);
    const wxString& GetText() const;
    void SetText(const wxString &text);

private:
    wxChartsLegendLineOptions m_options;
    wxPoint2DDouble m_position;
    wxSize m_size;
    wxColor m_color;
    wxString m_text;
};

#endif
