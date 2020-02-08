/*
    Copyright (c) 2016-2019 Xavier Leclercq and the wxCharts contributors.

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

/*
    Part of this file were copied from the Chart.js project (http://chartjs.org/)
    and translated into C++.

    The files of the Chart.js project have the following copyright and license.

    Copyright (c) 2013-2017 Nick Downie
    Released under the MIT license
    https://github.com/nnnick/Chart.js/blob/master/LICENSE.md
*/

/// @file

#ifndef _WX_CHARTS_WXMATH2DPLOTCTRL_H_
#define _WX_CHARTS_WXMATH2DPLOTCTRL_H_

#include "wxchartctrl.h"
#include "wxmath2dplot.h"
#include <wx/menu.h>

/// A control that displays a math2d plot.

/// \ingroup chartclasses
class wxMath2DPlotCtrl : public wxChartCtrl
{
public:
    /// Constructs a wxMath2DPlotCtrl control.
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
    wxMath2DPlotCtrl(wxWindow *parent, wxWindowID id, const wxMath2DPlotData &data,
        const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize,
        long style = 0);
    wxMath2DPlotCtrl(wxWindow *parent, wxWindowID id, const wxMath2DPlotData &data,
        wxSharedPtr<wxMath2DPlotOptions> &options, const wxPoint &pos = wxDefaultPosition,
        const wxSize &size = wxDefaultSize, long style = 0);

    bool UpdateData(std::size_t index,const wxVector<wxPoint2DDouble> &points);
    bool AddData(std::size_t index,const wxVector<wxPoint2DDouble> &points);
    bool RemoveDataset(std::size_t index);
    void AddDataset(const wxMath2DPlotDataset::ptr &newset);

    const wxChartsGridOptions& GetGridOptions() const;
    void  SetGridOptions(const wxChartsGridOptions& opt);
    const wxMath2DPlotOptions& GetChartOptions() const;
    void  SetChartOptions(const wxMath2DPlotOptions& opt);
    void  SetChartType(std::size_t index,const wxChartType &type);

private:
    virtual wxMath2DPlot& GetChart();

    void CreateContextMenu();
    void Update();

private:
    wxMath2DPlot m_math2dPlot;
    wxMenu m_contextMenu;
    wxMenu *m_subMenu;
    int m_posX;
    int m_posY;
};

#endif
