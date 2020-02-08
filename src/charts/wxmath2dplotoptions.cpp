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

#include "wxmath2dplotoptions.h"

wxMath2DPlotOptions::wxMath2DPlotOptions()
    : m_dotRadius(4), m_dotStrokeWidth(1),
    m_lineWidth(2), m_hitDetectionRange(24)
{
    auto temp = [](wxDouble x){return x;};
    m_axisFuncX = temp;
    m_axisFuncY = temp;
    wxChartMultiTooltipOptions& options = GetCommonOptions().GetMultiTooltipOptions();
    options.SetShowTitle(false);
    options.SetAlignment(wxALIGN_TOP);
}

const wxChartsGridOptions& wxMath2DPlotOptions::GetGridOptions() const
{
    return m_gridOptions;
}

wxChartsGridOptions& wxMath2DPlotOptions::GetGridOptions()
{
    return m_gridOptions;
}

wxDouble wxMath2DPlotOptions::GetDotRadius() const
{
    return m_dotRadius;
}

unsigned int wxMath2DPlotOptions::GetDotStrokeWidth() const
{
    return m_dotStrokeWidth;
}

unsigned int wxMath2DPlotOptions::GetLineWidth() const
{
    return m_lineWidth;
}

wxDouble wxMath2DPlotOptions::GetHitDetectionRange() const
{
    return m_hitDetectionRange;
}

const AxisFunc& wxMath2DPlotOptions::GetAxisFuncX() const
{
    return m_axisFuncX;
}

const AxisFunc& wxMath2DPlotOptions::GetAxisFuncY() const
{
    return m_axisFuncY;
}

void wxMath2DPlotOptions::SetAxisFuncX(const AxisFunc &newfunc)
{
    m_axisFuncX = newfunc;
}

void wxMath2DPlotOptions::SetAxisFuncY(const AxisFunc &newfunc)
{
    m_axisFuncY = newfunc;
}
