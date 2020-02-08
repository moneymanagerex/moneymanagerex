/*
    Copyright (c) 2017-2019 Xavier Leclercq

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

#ifndef _WX_CHARTS_WXCHARTSTHEME_H_
#define _WX_CHARTS_WXCHARTSTHEME_H_

#include "wxareachartoptions.h"
#include "wxbarchartoptions.h"
#include "wxboxplotoptions.h"
#include "wxbubblechartoptions.h"
#include "wxcandlestickchartoptions.h"
#include "wxcolumnchartoptions.h"
#include "wxdoughnutchartoptions.h"
#include "wxhistogramoptions.h"
#include "wxlinechartoptions.h"
#include "wxmath2dplotoptions.h"
#include "wxohlcchartoptions.h"
#include "wxpiechartoptions.h"
#include "wxpolarareachartoptions.h"
#include "wxradarchartoptions.h"
#include "wxscatterplotoptions.h"
#include "wxstackedbarchartoptions.h"
#include "wxstackedcolumnchartoptions.h"
#include "wxtimeserieschart.h"
#include "wxchartsdatasetid.h"
#include "wxchartsdatasettheme.h"
#include <wx/sharedptr.h>
#include <map>

/// \defgroup themeclasses

/// Represent a wxCharts theme.

/// \ingroup themeclasses
class wxChartsTheme
{
public:
    wxChartsTheme();

    wxSharedPtr<wxAreaChartOptions> GetAreaChartOptions();
    wxSharedPtr<wxBarChartOptions> GetBarChartOptions();
    wxSharedPtr<wxBoxPlotOptions> GetBoxPlotOptions();
    wxSharedPtr<wxBubbleChartOptions> GetBubbleChartOptions();
    wxSharedPtr<wxCandlestickChartOptions> GetCandlestickChartOptions();
    wxSharedPtr<wxColumnChartOptions> GetColumnChartOptions();
    wxSharedPtr<wxDoughnutChartOptions> GetDoughnutChartOptions();
    wxSharedPtr<wxHistogramOptions> GetHistogramOptions();
    wxSharedPtr<wxLineChartOptions> GetLineChartOptions();
    wxSharedPtr<wxMath2DPlotOptions> GetMath2DPlotOptions();
    wxSharedPtr<wxOHLCChartOptions> GetOHLCChartOptions();
    wxSharedPtr<wxPieChartOptions> GetPieChartOptions();
    wxSharedPtr<wxPolarAreaChartOptions> GetPolarAreaChartOptions();
    wxSharedPtr<wxRadarChartOptions> GetRadarChartOptions();
    wxSharedPtr<wxScatterPlotOptions> GetScatterPlotOptions();
    wxSharedPtr<wxStackedBarChartOptions> GetStackedBarChartOptions();
    wxSharedPtr<wxStackedColumnChartOptions> GetStackedColumnChartOptions();
    wxSharedPtr<wxTimeSeriesChartOptions> GetTimeSeriesChartOptions();

    wxSharedPtr<wxChartsDatasetTheme> GetDatasetTheme(const wxChartsDatasetId& id);
    void SetDatasetTheme(const wxChartsDatasetId& id, wxSharedPtr<wxChartsDatasetTheme> theme);

private:
    wxSharedPtr<wxAreaChartOptions> m_areaChartOptions;
    wxSharedPtr<wxBarChartOptions> m_barChartOptions;
    wxSharedPtr<wxBoxPlotOptions> m_boxPlotOptions;
    wxSharedPtr<wxBubbleChartOptions> m_bubbleChartOptions;
    wxSharedPtr<wxCandlestickChartOptions> m_candlestickChartOptions;
    wxSharedPtr<wxColumnChartOptions> m_columnChartOptions;
    wxSharedPtr<wxDoughnutChartOptions> m_doughnutChartOptions;
    wxSharedPtr<wxHistogramOptions> m_histogramOptions;
    wxSharedPtr<wxLineChartOptions> m_lineChartOptions;
    wxSharedPtr<wxMath2DPlotOptions> m_math2DPlotOptions;
    wxSharedPtr<wxOHLCChartOptions> m_ohlcChartOptions;
    wxSharedPtr<wxPieChartOptions> m_pieChartOptions;
    wxSharedPtr<wxPolarAreaChartOptions> m_polarAreaChartOptions;
    wxSharedPtr<wxRadarChartOptions> m_radarChartOptions;
    wxSharedPtr<wxScatterPlotOptions> m_scatterPlotOptions;
    wxSharedPtr<wxStackedBarChartOptions> m_stackedBarChartOptions;
    wxSharedPtr<wxStackedColumnChartOptions> m_stackedColumnChartOptions;
    wxSharedPtr<wxTimeSeriesChartOptions> m_timeSeriesChartOptions;
    std::map<wxChartsDatasetId, wxSharedPtr<wxChartsDatasetTheme>> m_datasetThemes;
};

extern wxSharedPtr<wxChartsTheme> wxChartsDefaultTheme;

#endif
