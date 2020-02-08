/*
    Copyright (c) 2016-2018 Xavier Leclercq

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

#ifndef _WX_CHARTS_WXCHARTCOMMONOPTIONS_H_
#define _WX_CHARTS_WXCHARTCOMMONOPTIONS_H_

#include "wxchartmultitooltipoptions.h"

/// Options that are common between the various chart types.

/// \ingroup chartclasses
class wxChartCommonOptions
{
public:
    /// Constructs a wxChartCommonOptions
    /// instance.
    wxChartCommonOptions();

    /// Whether the chart automatically resizes 
    /// when the control size changes.
    /// @retval true Resize the chart when control
    /// size changes.
    /// @retval false Don't automatically resize
    /// the chart when the control size changes.
    bool IsResponsive() const;

    /// Whether to display tooltips.
    /// @retval true Display tooltips.
    /// @retval false Don't display tooltips.
    bool ShowTooltips() const;
    /// Enables or disable tooltips.
    /// @param show True to enable tooltips, false to disable them.
    void SetShowTooltips(bool show);
    /// Gets the options for the multi-tooltips (const version).
    /// @return The options for the multi-tooltips.
    const wxChartMultiTooltipOptions& GetMultiTooltipOptions() const;
    /// Gets the options for the multi-tooltips (non-const version).
    /// @return The options for the multi-tooltips.
    wxChartMultiTooltipOptions& GetMultiTooltipOptions();

private:
    bool m_responsive;
    bool m_showTooltips;
    wxChartMultiTooltipOptions m_multiTooltipOptions;
};

#endif
