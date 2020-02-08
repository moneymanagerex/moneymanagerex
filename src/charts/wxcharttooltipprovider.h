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

#ifndef _WX_CHARTS_WXCHARTTOOLTIPPROVIDER_H_
#define _WX_CHARTS_WXCHARTTOOLTIPPROVIDER_H_

#include <wx/colour.h>
#include <wx/string.h>
#include <wx/sharedptr.h>

/// Interface for the classes that are able to generate content for tooltips
class wxChartTooltipProvider
{
public:
    /// Smart pointer typedef.
    typedef wxSharedPtr<wxChartTooltipProvider> ptr;

    /// Constructs a wxChartTooltipProvider instance.
    wxChartTooltipProvider();
    virtual ~wxChartTooltipProvider();

    // Gets the title of the tooltip.
    /// @return A wxString containing the title of the tooltip.
    virtual wxString GetTooltipTitle() const = 0;
    /// Gets the text to display in the tooltip.
    /// @return A wxString containing the text of the tooltip.
    virtual wxString GetTooltipText() const = 0;
    /// Gets the color that can be used to identify which
    /// element of the chart this tooltip corresponds to.
    /// This is used when the tooltip is part of a wxChartMultiTooltip
    /// because in that case multiple tooltips will be shown at the
    /// same time.
    /// @return The color.
    virtual wxColor GetAssociatedColor() const = 0;
};

/// Implementation of the wxChartTooltipProvider interface that simply returns a static string.

/// An implementation of the wxChartTooltipProvider
/// interface where the content is a static string
/// passed in at construction time.
class wxChartTooltipProviderStatic : public wxChartTooltipProvider
{
public:
    /// Constructs a wxChartTooltipProviderStatic instance.
    /// @param title The title of the tooltip which will be returned
    /// by the GetTooltipTitle() function.
    /// @param text The text of the tooltip which will be returned by
    /// the GetTooltipText() function.
    wxChartTooltipProviderStatic(const wxString &title, const wxString &text,
        const wxColor &color);

    virtual wxString GetTooltipTitle() const;
    virtual wxString GetTooltipText() const;
    virtual wxColor GetAssociatedColor() const;

private:
    wxString m_title;
    wxString m_text;
    wxColor m_color;
};

#endif
