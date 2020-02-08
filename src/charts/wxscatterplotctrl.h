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

#ifndef _WX_CHARTS_WXSCATTERPLOTCTRL_H_
#define _WX_CHARTS_WXSCATTERPLOTCTRL_H_

#include "wxchartctrl.h"
#include "wxscatterplot.h"

/// A control that displays a scatter plot.

/// \ingroup chartclasses
class wxScatterPlotCtrl : public wxChartCtrl
{
public:
    /// Constructs a wxScatterPlotCtrl control.
    /// @param parent Pointer to a parent window.
    /// @param id Control identifier. If wxID_ANY, will automatically
    /// create an identifier.
    /// @param data The data that will be used to initialize the chart.
    /// @param pos Control position. wxDefaultPosition indicates that 
    /// wxWidgets should generate a default position for the control.
    /// @param size Control size. wxDefaultSize indicates that wxWidgets
    /// should generate a default size for the window. If no suitable 
    /// size can  be found, the window will be sized to 20x20 pixels 
    /// so that the window is visible but obviously not correctly sized.
    /// @param style Control style. For generic window styles, please 
    /// see wxWindow.
    wxScatterPlotCtrl(wxWindow *parent, wxWindowID id, const wxScatterPlotData &data,
        const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize,
        long style = 0);
    wxScatterPlotCtrl(wxWindow *parent, wxWindowID id, const wxScatterPlotData &data,
        wxSharedPtr<wxScatterPlotOptions> &options, const wxPoint &pos = wxDefaultPosition,
        const wxSize &size = wxDefaultSize, long style = 0);

private:
    virtual wxScatterPlot& GetChart();

private:
    wxScatterPlot m_scatterPlot;
};

#endif
