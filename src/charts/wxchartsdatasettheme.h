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

#ifndef _WX_CHARTS_WXCHARTSDATASETTHEME_H_
#define _WX_CHARTS_WXCHARTSDATASETTHEME_H_

#include "wxareachartdatasetoptions.h"
#include "wxbarchartdatasetoptions.h"
#include "wxboxplotdatasetoptions.h"
#include "wxcolumnchartdatasetoptions.h"
#include "wxlinechartdatasetoptions.h"
#include "wxstackedbarchartdatasetoptions.h"
#include "wxstackedcolumnchartdatasetoptions.h"
#include <wx/sharedptr.h>

class wxChartsDatasetTheme
{
public:
    wxChartsDatasetTheme();

    wxSharedPtr<wxAreaChartDatasetOptions> GetAreaChartDatasetOptions();
    wxSharedPtr<wxBarChartDatasetOptions> GetBarChartDatasetOptions();
    wxSharedPtr<wxBoxPlotDatasetOptions> GetBoxPlotDatasetOptions();
    wxSharedPtr<wxColumnChartDatasetOptions> GetColumnChartDatasetOptions();
    wxSharedPtr<wxLineChartDatasetOptions> GetLineChartDatasetOptions();
    wxSharedPtr<wxStackedBarChartDatasetOptions> GetStackedBarChartDatasetOptions();
    wxSharedPtr<wxStackedColumnChartDatasetOptions> GetStackedColumnChartDatasetOptions();

    void SetAreaChartDatasetOptions(const wxAreaChartDatasetOptions& options);
    void SetBarChartDatasetOptions(const wxBarChartDatasetOptions& options);
    void SetBoxPlotDatasetOptions(const wxBoxPlotDatasetOptions& options);
    void SetColumnChartDatasetOptions(const wxColumnChartDatasetOptions& options);
    void SetLineChartDatasetOptions(const wxLineChartDatasetOptions& options);
    void SetStackedBarChartDatasetOptions(const wxStackedBarChartDatasetOptions& options);
    void SetStackedColumnChartDatasetOptions(const wxStackedColumnChartDatasetOptions& options);

private:
    wxSharedPtr<wxAreaChartDatasetOptions> m_areaChartDatasetOptions;
    wxSharedPtr<wxBarChartDatasetOptions> m_barChartDatasetOptions;
    wxSharedPtr<wxBoxPlotDatasetOptions> m_boxPlotDatasetOptions;
    wxSharedPtr<wxColumnChartDatasetOptions> m_columnChartDatasetOptions;
    wxSharedPtr<wxLineChartDatasetOptions> m_lineChartDatasetOptions;
    wxSharedPtr<wxStackedBarChartDatasetOptions> m_stackedBarChartDatasetOptions;
    wxSharedPtr<wxStackedColumnChartDatasetOptions> m_stackedColumnChartDatasetOptions;
};

#endif
