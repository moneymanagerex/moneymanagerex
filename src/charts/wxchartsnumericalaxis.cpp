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

/*
    Part of this file were copied from the Chart.js project (http://chartjs.org/)
    and translated into C++.

    The files of the Chart.js project have the following copyright and license.

    Copyright (c) 2013-2016 Nick Downie
    Released under the MIT license
    https://github.com/nnnick/Chart.js/blob/master/LICENSE.md
*/

#include "wxchartsnumericalaxis.h"
#include "wxchartsutilities.h"

wxChartsNumericalAxis::wxChartsNumericalAxis(const std::string &id, 
                                             wxDouble minValue,
                                             wxDouble maxValue,
                                             const wxChartsAxisOptions &options)
    : wxChartsAxis(id, options),
    m_minValue(minValue), m_maxValue(maxValue)
{
    wxDouble effectiveMinXValue = minValue;
    if (options.GetStartValueMode() == wxCHARTSAXISVALUEMODE_EXPLICIT)
    {
        effectiveMinXValue = options.GetStartValue();
    }
    wxDouble effectiveMaxXValue = maxValue;
    if (options.GetEndValueMode() == wxCHARTSAXISVALUEMODE_EXPLICIT)
    {
        effectiveMaxXValue = options.GetEndValue();
    }

    wxDouble graphMinXValue;
    wxDouble graphMaxXValue;
    wxDouble xValueRange = 0;
    size_t steps = 0;
    wxDouble stepValue = 0;
    wxChartsUtilities::CalculateGridRange(effectiveMinXValue, effectiveMaxXValue,
        graphMinXValue, graphMaxXValue, xValueRange, steps, stepValue);

    SetMinValue(graphMinXValue);
    SetMaxValue(graphMaxXValue);

    wxVector<wxChartsLabel> xLabels;
    wxChartsUtilities::BuildNumericalLabels(
        GetMinValue(),
        steps,
        stepValue,
        wxChartsLabelOptions(options.GetFontOptions(), false, wxChartBackgroundOptions(*wxWHITE, 0)),
        xLabels);
    SetLabels(xLabels);
}

wxChartsNumericalAxis::ptr wxChartsNumericalAxis::make_shared(const std::string &id, 
                                                              wxDouble minValue,
                                                              wxDouble maxValue,
                                                              const wxChartsAxisOptions &options)
{
    return ptr(new wxChartsNumericalAxis(id, minValue, maxValue, options));
}

wxDouble wxChartsNumericalAxis::GetMinValue() const
{
    return m_minValue;
}

void wxChartsNumericalAxis::SetMinValue(wxDouble minValue)
{
    m_minValue = minValue;
}

wxDouble wxChartsNumericalAxis::GetMaxValue() const
{
    return m_maxValue;
}

void wxChartsNumericalAxis::SetMaxValue(wxDouble maxValue)
{
    m_maxValue = maxValue;
}
