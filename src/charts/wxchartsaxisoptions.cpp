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

#include "wxchartsaxisoptions.h"

wxChartsAxisOptions::wxChartsAxisOptions(wxChartsAxisPosition position)
    : m_position(position), m_labelType(wxCHARTSAXISLABELTYPE_POINT),
    m_startMarginType(wxCHARTSAXISMARGINTYPE_NONE), m_endMarginType(wxCHARTSAXISMARGINTYPE_NONE),
    m_startValueMode(wxCHARTSAXISVALUEMODE_AUTO), m_startValue(0),
    m_endValueMode(wxCHARTSAXISVALUEMODE_AUTO), m_endValue(0),
    m_lineWidth(1), m_lineColor(0, 0, 0, 0x19), m_overhang(3),
    m_fontOptions(wxFONTFAMILY_SWISS, 12, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, 0x666666)
{
}

wxChartsAxisOptions::wxChartsAxisOptions(wxChartsAxisPosition position,
                                         wxChartsAxisLabelType labelType)
    : m_position(position), m_labelType(labelType),
    m_startMarginType(wxCHARTSAXISMARGINTYPE_NONE), m_endMarginType(wxCHARTSAXISMARGINTYPE_NONE),
    m_startValueMode(wxCHARTSAXISVALUEMODE_AUTO), m_startValue(0),
    m_endValueMode(wxCHARTSAXISVALUEMODE_AUTO), m_endValue(0),
    m_lineWidth(1), m_lineColor(0, 0, 0, 0x19), m_overhang(3),
    m_fontOptions(wxFONTFAMILY_SWISS, 12, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, 0x666666)
{
}

wxChartsAxisPosition wxChartsAxisOptions::GetPosition() const
{
    return m_position;
}

wxChartsAxisLabelType wxChartsAxisOptions::GetLabelType() const
{
    return m_labelType;
}

wxChartsAxisMarginType wxChartsAxisOptions::GetStartMarginType() const
{
    return m_startMarginType;
}

void wxChartsAxisOptions::SetStartMarginType(wxChartsAxisMarginType type)
{
    m_startMarginType = type;
}

wxChartsAxisMarginType wxChartsAxisOptions::GetEndMarginType() const
{
    return m_endMarginType;
}

void wxChartsAxisOptions::SetEndMarginType(wxChartsAxisMarginType type)
{
    m_endMarginType = type;
}

wxChartsAxisValueMode wxChartsAxisOptions::GetStartValueMode() const
{
    return m_startValueMode;
}

wxDouble wxChartsAxisOptions::GetStartValue() const
{
    return m_startValue;
}

void wxChartsAxisOptions::SetExplicitStartValue(wxDouble startValue)
{
    m_startValueMode = wxCHARTSAXISVALUEMODE_EXPLICIT;
    m_startValue = startValue;
}

wxChartsAxisValueMode wxChartsAxisOptions::GetEndValueMode() const
{
    return m_endValueMode;
}

wxDouble wxChartsAxisOptions::GetEndValue() const
{
    return m_endValue;
}

void wxChartsAxisOptions::SetExplicitEndValue(wxDouble endValue)
{
    m_endValueMode = wxCHARTSAXISVALUEMODE_EXPLICIT;
    m_endValue = endValue;
}

unsigned int wxChartsAxisOptions::GetLineWidth() const
{
    return m_lineWidth;
}

const wxColor& wxChartsAxisOptions::GetLineColor() const
{
    return m_lineColor;
}

wxDouble wxChartsAxisOptions::GetOverhang() const
{
    return m_overhang;
}

const wxChartFontOptions& wxChartsAxisOptions::GetFontOptions() const
{
    return m_fontOptions;
}
