/*
    Copyright (c) 2019 Xavier Leclercq

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

#include "wxchartsdatasettheme.h"

wxChartsDatasetTheme::wxChartsDatasetTheme()
    : m_areaChartDatasetOptions(new wxAreaChartDatasetOptions(*wxBLACK)),
    m_barChartDatasetOptions(new wxBarChartDatasetOptions(wxChartsPenOptions(*wxBLACK, 2), wxChartsBrushOptions(*wxWHITE))),
    m_boxPlotDatasetOptions(new wxBoxPlotDatasetOptions()),
    m_columnChartDatasetOptions(new wxColumnChartDatasetOptions(wxChartsPenOptions(*wxBLACK, 2), wxChartsBrushOptions(*wxWHITE))),
    m_lineChartDatasetOptions(new wxLineChartDatasetOptions(*wxBLACK, *wxWHITE, *wxWHITE)),
    m_stackedBarChartDatasetOptions(new wxStackedBarChartDatasetOptions(wxChartsPenOptions(*wxBLACK, 2), wxChartsBrushOptions(*wxWHITE))),
    m_stackedColumnChartDatasetOptions(new wxStackedColumnChartDatasetOptions(wxChartsPenOptions(*wxBLACK, 2), wxChartsBrushOptions(*wxWHITE)))
{
}

wxSharedPtr<wxAreaChartDatasetOptions> wxChartsDatasetTheme::GetAreaChartDatasetOptions()
{
    return m_areaChartDatasetOptions;
}

wxSharedPtr<wxBarChartDatasetOptions> wxChartsDatasetTheme::GetBarChartDatasetOptions()
{
    return m_barChartDatasetOptions;
}

wxSharedPtr<wxBoxPlotDatasetOptions> wxChartsDatasetTheme::GetBoxPlotDatasetOptions()
{
    return m_boxPlotDatasetOptions;
}

wxSharedPtr<wxColumnChartDatasetOptions> wxChartsDatasetTheme::GetColumnChartDatasetOptions()
{
    return m_columnChartDatasetOptions;
}

wxSharedPtr<wxLineChartDatasetOptions> wxChartsDatasetTheme::GetLineChartDatasetOptions()
{
    return m_lineChartDatasetOptions;
}

wxSharedPtr<wxStackedBarChartDatasetOptions> wxChartsDatasetTheme::GetStackedBarChartDatasetOptions()
{
    return m_stackedBarChartDatasetOptions;
}

wxSharedPtr<wxStackedColumnChartDatasetOptions> wxChartsDatasetTheme::GetStackedColumnChartDatasetOptions()
{
    return m_stackedColumnChartDatasetOptions;
}

void wxChartsDatasetTheme::SetAreaChartDatasetOptions(const wxAreaChartDatasetOptions& options)
{
    m_areaChartDatasetOptions = new wxAreaChartDatasetOptions(options);
}

void wxChartsDatasetTheme::SetBarChartDatasetOptions(const wxBarChartDatasetOptions& options)
{
    m_barChartDatasetOptions = new wxBarChartDatasetOptions(options);
}

void wxChartsDatasetTheme::SetBoxPlotDatasetOptions(const wxBoxPlotDatasetOptions& options)
{
    m_boxPlotDatasetOptions = new wxBoxPlotDatasetOptions(options);
}

void wxChartsDatasetTheme::SetColumnChartDatasetOptions(const wxColumnChartDatasetOptions& options)
{
    m_columnChartDatasetOptions = new wxColumnChartDatasetOptions(options);
}

void wxChartsDatasetTheme::SetLineChartDatasetOptions(const wxLineChartDatasetOptions& options)
{
    m_lineChartDatasetOptions = new wxLineChartDatasetOptions(options);
}

void wxChartsDatasetTheme::SetStackedBarChartDatasetOptions(const wxStackedBarChartDatasetOptions& options)
{
    m_stackedBarChartDatasetOptions = new wxStackedBarChartDatasetOptions(options);
}

void wxChartsDatasetTheme::SetStackedColumnChartDatasetOptions(const wxStackedColumnChartDatasetOptions& options)
{
    m_stackedColumnChartDatasetOptions = new wxStackedColumnChartDatasetOptions(options);
}
