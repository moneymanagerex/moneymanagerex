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

/// @file

#include "wxchartpadding.h"

wxChartPadding::wxChartPadding()
    : m_left(0), m_right(0), m_top(0), m_bottom(0)
{
}

wxChartPadding::wxChartPadding(wxDouble left, 
                               wxDouble right,
                               wxDouble top, 
                               wxDouble bottom)
    : m_left(left), m_right(right), m_top(top), m_bottom(bottom)
{
}

wxDouble wxChartPadding::GetLeft() const
{
    return m_left;
}

wxDouble wxChartPadding::GetRight() const
{
    return m_right;
}

wxDouble wxChartPadding::GetTop() const
{
    return m_top;
}

wxDouble wxChartPadding::GetBottom() const
{
    return m_bottom;
}

wxDouble wxChartPadding::GetTotalHorizontalPadding() const
{
    return m_left + m_right;
}

wxDouble wxChartPadding::GetTotalVerticalPadding() const
{
    return m_top + m_bottom;
}
