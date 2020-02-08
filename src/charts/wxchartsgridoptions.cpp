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

#include "wxchartsgridoptions.h"

wxChartsGridLineOptions::wxChartsGridLineOptions()
    : m_showGridLines(true),
    m_majorGridLineWidth(1), m_majorGridLineColor(0, 0, 0, 0x0C),
    m_numberOfMinorGridLinesBetweenTickMarks(0)
{
}

bool wxChartsGridLineOptions::ShowGridLines() const
{
    return m_showGridLines;
}

void wxChartsGridLineOptions::SetShowGridLines(bool show)
{
    m_showGridLines = show;
}

unsigned int wxChartsGridLineOptions::GetMajorGridLineWidth() const
{
    return m_majorGridLineWidth;
}

void wxChartsGridLineOptions::SetMajorGridLineWidth(unsigned int width)
{
    m_majorGridLineWidth = width;
}

const wxColor& wxChartsGridLineOptions::GetMajorGridLineColor() const
{
    return m_majorGridLineColor;
}

void wxChartsGridLineOptions::SetMajorGridLineColor(const wxColor &color)
{
    m_majorGridLineColor = color;
}

unsigned int wxChartsGridLineOptions::GetNumberOfMinorGridLinesBetweenTickMarks() const
{
    return m_numberOfMinorGridLinesBetweenTickMarks;
}

void wxChartsGridLineOptions::SetNumberOfMinorGridLinesBetweenTickMarks(unsigned int n)
{
    m_numberOfMinorGridLinesBetweenTickMarks = n;
}

wxChartsGridOptions::wxChartsGridOptions()
    : m_XAxisOptions(wxCHARTSAXISPOSITION_BOTTOM), m_YAxisOptions(wxCHARTSAXISPOSITION_LEFT)
{
}

wxChartsGridOptions::wxChartsGridOptions(const wxChartsAxisOptions &xAxisOptions,
                                         const wxChartsAxisOptions &yAxisOptions)
    : m_XAxisOptions(xAxisOptions), m_YAxisOptions(yAxisOptions)
{
}

const wxChartsAxisOptions& wxChartsGridOptions::GetXAxisOptions() const
{
    return m_XAxisOptions;
}

wxChartsAxisOptions& wxChartsGridOptions::GetXAxisOptions()
{
    return m_XAxisOptions;
}

const wxChartsAxisOptions& wxChartsGridOptions::GetYAxisOptions() const
{
    return m_YAxisOptions;
}

wxChartsAxisOptions& wxChartsGridOptions::GetYAxisOptions()
{
    return m_YAxisOptions;
}

const wxChartsGridLineOptions& wxChartsGridOptions::GetHorizontalGridLineOptions() const
{
    return m_horizontalGridLineOptions;
}

wxChartsGridLineOptions& wxChartsGridOptions::GetHorizontalGridLineOptions()
{
    return m_horizontalGridLineOptions;
}

const wxChartsGridLineOptions& wxChartsGridOptions::GetVerticalGridLineOptions() const
{
    return m_verticalGridLinesOptions;
}

wxChartsGridLineOptions& wxChartsGridOptions::GetVerticalGridLineOptions()
{
    return m_verticalGridLinesOptions;
}

void wxChartsGridOptions::SetShowGridLines(bool show)
{
    if(m_horizontalGridLineOptions.ShowGridLines() != show)
        m_horizontalGridLineOptions.SetShowGridLines(show);

    if(m_verticalGridLinesOptions.ShowGridLines() != show)
        m_verticalGridLinesOptions.SetShowGridLines(show);
}
