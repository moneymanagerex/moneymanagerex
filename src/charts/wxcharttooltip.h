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

#ifndef _WX_CHARTS_WXCHARTTOOLTIP_H_
#define _WX_CHARTS_WXCHARTTOOLTIP_H_

#include "wxcharttooltipoptions.h"
#include "wxcharttooltipprovider.h"
#include <wx/graphics.h>

/// This class is used to display a tooltip.

/// Charts can display tooltips in response to a user
/// action (e.g. hoverin with the mouse over some part
/// of the chart). This class implements such 
/// tooltips.
class wxChartTooltip
{
public:
    /// Constructs a new wxChartTooltip instance.
    /// @param position The position of the tooltip.
    /// @param text The text to display in the tooltip.
    wxChartTooltip(const wxPoint2DDouble &position,
        const wxString &text);
    wxChartTooltip(const wxPoint2DDouble &position,
        const wxChartTooltipProvider::ptr provider);

    /// Draws the tooltip using the graphics context passed
    /// in as argument.
    /// @param gc The graphics context.
    void Draw(wxGraphicsContext &gc);

    /// Gets the position of the tooltip.
    /// @return The position of the tooltip.
    const wxPoint2DDouble& GetPosition() const;
    /// Gets the tooltip text provider.
    /// @return The tooltip provider.
    const wxChartTooltipProvider::ptr& GetProvider() const;

private:
    wxChartTooltipOptions m_options;
    wxPoint2DDouble m_position;
    wxChartTooltipProvider::ptr m_provider;
};

#endif
