/*
    Copyright (c) 2016-2017 Xavier Leclercq

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

#include "wxcharttooltipoptions.h"

wxChartTooltipOptions::wxChartTooltipOptions()
    : m_fontFamily(wxFONTFAMILY_SWISS), m_fontSize(14),
    m_fontStyle(wxFONTSTYLE_NORMAL), m_fontColor(0xFFFFFF),
    m_horizontalPadding(6), m_verticalPadding(6),
    m_backgroundOptions(wxColor(0, 0, 0, 0xCC), 6)
{
}

wxFontFamily wxChartTooltipOptions::GetFontFamily() const
{
    return m_fontFamily;
}

int wxChartTooltipOptions::GetFontSize() const
{
    return m_fontSize;
}

wxFontStyle wxChartTooltipOptions::GetFontStyle() const
{
    return m_fontStyle;
}

const wxColor& wxChartTooltipOptions::GetFontColor() const
{
    return m_fontColor;
}

wxDouble wxChartTooltipOptions::GetHorizontalPadding() const
{
    return m_horizontalPadding;
}

wxDouble wxChartTooltipOptions::GetVerticalPadding() const
{
    return m_verticalPadding;
}

const wxChartBackgroundOptions& wxChartTooltipOptions::GetBackgroundOptions() const
{
    return m_backgroundOptions;
}
