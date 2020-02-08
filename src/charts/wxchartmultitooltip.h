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

#ifndef _WX_CHARTS_WXCHARTMULTITOOLTIP_H_
#define _WX_CHARTS_WXCHARTMULTITOOLTIP_H_

#include "wxchartmultitooltipoptions.h"
#include "wxcharttooltip.h"
#include "wxchartslegendline.h"
#include <wx/graphics.h>

/// This class is used to display a tooltip.

/// Charts can display tooltips in response to a user
/// action (e.g. hoverin with the mouse over some part
/// of the chart). This class implements such 
/// tooltips.
/// Unlike the wxChartTooltip class this class can
/// display information about multiple items at once
/// for those case where the user selects more than 
/// one item at a time.
class wxChartMultiTooltip
{
public:
    /// Constructs a wxChartMultiTooltip
    /// instance.
    wxChartMultiTooltip(const wxString &title);
    wxChartMultiTooltip(const wxString &title, const wxChartMultiTooltipOptions &options);

    /// Draws the tooltip using the graphics context passed
    /// in as argument.
    /// @param gc The graphics context.
    void Draw(wxGraphicsContext &gc);

    void AddTooltip(const wxChartTooltip &tooltip);

private:
    wxChartMultiTooltipOptions m_options;
    wxString m_title;
    wxVector<wxPoint2DDouble> m_tooltipPositions;
    wxVector<wxChartTooltipProvider::ptr> m_tooltipProviders;
    wxVector<wxChartsLegendLine> m_lines;
};

#endif
