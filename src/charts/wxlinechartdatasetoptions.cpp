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

#include "wxlinechartdatasetoptions.h"

wxLineChartDatasetOptions::wxLineChartDatasetOptions(const wxColor &dotColor,
                                                     const wxColor &dotStrokeColor,
                                                     const wxColor &fillColor)
    : m_showDots(true), m_dotPenOptions(dotStrokeColor, 1), m_dotBrushOptions(dotColor),
    m_dotRadius(4), m_showLine(true), m_lineColor(dotColor), m_lineWidth(2), m_fill(true),
    m_fillColor(fillColor)
{
}

bool wxLineChartDatasetOptions::ShowDots() const
{
    return m_showDots;
}

const wxChartsPenOptions& wxLineChartDatasetOptions::GetDotPenOptions() const
{
    return m_dotPenOptions;
}

const wxChartsBrushOptions& wxLineChartDatasetOptions::GetDotBrushOptions() const
{
    return m_dotBrushOptions;
}

wxDouble wxLineChartDatasetOptions::GetDotRadius() const
{
    return m_dotRadius;
}

bool wxLineChartDatasetOptions::ShowLine() const
{
    return m_showLine;
}

const wxColor& wxLineChartDatasetOptions::GetLineColor() const
{
    return m_lineColor;
}

unsigned int wxLineChartDatasetOptions::GetLineWidth() const
{
    return m_lineWidth;
}

bool wxLineChartDatasetOptions::Fill() const
{
    return m_fill;
}

const wxColor& wxLineChartDatasetOptions::GetFillColor() const
{
    return m_fillColor;
}
