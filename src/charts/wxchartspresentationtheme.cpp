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

#include "wxchartspresentationtheme.h"

wxChartsPresentationTheme::wxChartsPresentationTheme()
{
    SetDatasetTheme(wxChartsDatasetId::CreateImplicitId(0),
        wxSharedPtr<wxChartsDatasetTheme>(new wxChartsPresentationDatasetTheme0()));
    SetDatasetTheme(wxChartsDatasetId::CreateImplicitId(1),
        wxSharedPtr<wxChartsDatasetTheme>(new wxChartsPresentationDatasetTheme1()));
    SetDatasetTheme(wxChartsDatasetId::CreateImplicitId(2),
        wxSharedPtr<wxChartsDatasetTheme>(new wxChartsPresentationDatasetTheme2()));
}

wxChartsPresentationDatasetTheme0::wxChartsPresentationDatasetTheme0()
{
    SetAreaChartDatasetOptions(wxAreaChartDatasetOptions(
        wxColor(151, 187, 205, 0x7F)
    ));

    SetBarChartDatasetOptions(wxBarChartDatasetOptions(
        wxChartsPenOptions(wxColor(220, 220, 220, 0xCC), 2),
        wxChartsBrushOptions(wxColor(220, 220, 220, 0x7F))
    ));

    SetLineChartDatasetOptions(wxLineChartDatasetOptions(
        wxColor(220, 220, 220),
        wxColor(255, 255, 255),
        wxColor(220, 220, 220, 0x33)
    ));

    SetColumnChartDatasetOptions(wxColumnChartDatasetOptions(
        wxChartsPenOptions(wxColor(220, 220, 220, 0xCC), 2),
        wxChartsBrushOptions(wxColor(220, 220, 220, 0x7F))
    ));

    SetStackedBarChartDatasetOptions(wxStackedBarChartDatasetOptions(
        wxChartsPenOptions(wxColor(96, 83, 77, 0x6C), 2),
        wxChartsBrushOptions(wxColor(96, 83, 77, 0x3F))
    ));

    SetStackedColumnChartDatasetOptions(wxStackedColumnChartDatasetOptions(
        wxChartsPenOptions(wxColor(96, 83, 77, 0x6C), 2),
        wxChartsBrushOptions(wxColor(96, 83, 77, 0x3F))
    ));
}

wxChartsPresentationDatasetTheme1::wxChartsPresentationDatasetTheme1()
{
    SetBarChartDatasetOptions(wxBarChartDatasetOptions(
        wxChartsPenOptions(wxColor(151, 187, 205, 0xFF), 2),
        wxChartsBrushOptions(wxColor(151, 187, 205, 0x7F))
    ));

    SetLineChartDatasetOptions(wxLineChartDatasetOptions(
        wxColor(151, 187, 205),
        wxColor(255, 255, 255),
        wxColor(151, 187, 205, 0x33)
    ));
 
    SetColumnChartDatasetOptions(wxColumnChartDatasetOptions(
        wxChartsPenOptions(wxColor(151, 187, 205, 0xFF), 2),
        wxChartsBrushOptions(wxColor(151, 187, 205, 0x7F))
    ));
   
    SetStackedBarChartDatasetOptions(wxStackedBarChartDatasetOptions(
        wxChartsPenOptions(wxColor(220, 220, 220, 0xCC), 2),
        wxChartsBrushOptions(wxColor(220, 220, 220, 0x7F))
    ));
    
    SetStackedColumnChartDatasetOptions(wxStackedColumnChartDatasetOptions(
        wxChartsPenOptions(wxColor(220, 220, 220, 0xCC), 2),
        wxChartsBrushOptions(wxColor(220, 220, 220, 0x7F))
    ));
}

wxChartsPresentationDatasetTheme2::wxChartsPresentationDatasetTheme2()
{
    SetStackedBarChartDatasetOptions(wxStackedBarChartDatasetOptions(
        wxChartsPenOptions(wxColor(151, 187, 205, 0xFF), 2),
        wxChartsBrushOptions(wxColor(151, 187, 205, 0x7F))
    ));

    SetStackedColumnChartDatasetOptions(wxStackedColumnChartDatasetOptions(
        wxChartsPenOptions(wxColor(151, 187, 205, 0xFF), 2),
        wxChartsBrushOptions(wxColor(151, 187, 205, 0x7F))
    ));
}
