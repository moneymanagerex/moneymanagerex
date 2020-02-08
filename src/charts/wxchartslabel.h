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

#ifndef _WX_CHARTS_WXCHARTSLABEL_H_
#define _WX_CHARTS_WXCHARTSLABEL_H_

#include "wxchartselement.h"
#include "wxchartslabeloptions.h"
#include "wxchartpadding.h"
#include <wx/graphics.h>
#include <wx/string.h>

/// This class is used to display text on a chart.

/// In addition to the text this class stores
/// the width and height of the label. However
/// this class itself is unable to compute them
/// as that requires a graphics context. The code
/// that uses this class needs to set and update 
/// the width and height.
class wxChartsLabel : public wxChartsElement
{
public:
    /// Constructs a wxChartsLabel element. The width
    /// and height are set to 0.
    /// @param text The text to display.
    /// @param options The settings to be used for the label.
    wxChartsLabel(const wxString &text, const wxChartsLabelOptions &options);
    /// Constructs a wxChartsLabel element.
    /// @param text The text to display.
    /// @param width The width of the label.
    /// @param height The height of the label.
    /// @param options The settings to be used for the label.
    wxChartsLabel(const wxString &text, wxDouble width,
        wxDouble height, const wxChartsLabelOptions &options);

    virtual void Draw(wxGraphicsContext &gc) const;

    virtual bool HitTest(const wxPoint &point) const;

    virtual wxPoint2DDouble GetTooltipPosition() const;

    wxChartsLabelOptions& GetOptions();
    /// Gets the text of the label.
    /// @return The text of the label.
    const wxString& GetText() const;
    /// Gets the position of the label. The position
    /// must have been set by the 
    /// SetPosition(wxDouble x, wxDouble y) function.
    /// @return The position of the label.
    const wxPoint2DDouble& GetPosition() const;
    void SetPosition(const wxPoint2DDouble &position);
    /// Sets the position of the label.
    /// @param x The X coordinate of the label.
    /// @param y The Y coordinate of the label.
    void SetPosition(wxDouble x, wxDouble y);
    /// Gets the size of the label. The size must
    /// have been set at construction or by one of
    /// the functions that set the size.
    /// @return The size of the label.
    /// @see SetSize(const wxSize &size)
    /// @see SetSize(wxDouble width, wxDouble height)
    /// @see UpdateSize(wxGraphicsContext &gc)
    const wxSize& GetSize() const;
    /// Sets the size of the label.
    /// @param size The new size.
    void SetSize(const wxSize &size);
    /// Sets the size of the label.
    /// @param width The new width.
    /// @param height The new height.
    void SetSize(wxDouble width, wxDouble height);
    void UpdateSize(wxGraphicsContext &gc);
    const wxChartPadding& GetPadding() const;
    void SetPadding(const wxChartPadding &padding);

private:
    wxChartsLabelOptions m_options;
    wxString m_text;
    wxPoint2DDouble m_position;
    wxSize m_size;
    wxChartPadding m_padding;
};

#endif
