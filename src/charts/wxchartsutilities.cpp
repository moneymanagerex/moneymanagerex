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

#include "wxchartsutilities.h"
#include <sstream>

wxPen wxChartsUtilities::CreatePen(const wxChartsPenOptions& options)
{
    return wxPen(options.GetColor(), options.GetWidth(), options.GetStyle());
}

wxBrush wxChartsUtilities::CreateBrush(const wxChartsBrushOptions& options)
{
    return wxBrush(options.GetColor());
}

size_t wxChartsUtilities::GetDecimalPlaces()
{
    return 1;
}

void wxChartsUtilities::CalculateGridRange(wxDouble minValue,
                                           wxDouble maxValue, 
                                           wxDouble &graphMinValue,
                                           wxDouble &graphMaxValue,
                                           wxDouble &valueRange,
                                           size_t &steps,
                                           wxDouble &stepValue)
{
    // Set a minimum step of two - a point at the top of the graph, and a point at the base
    steps = 2;

    // We need some degree of separation here to calculate the scales if all the values are the same
    // Adding/minusing 0.5 will give us a range of 1.
    if (maxValue == minValue)
    {
        maxValue += 0.5;
        minValue -= 0.5;
    }

    valueRange = maxValue - minValue;
    if (valueRange < 0)
    {
        valueRange = -valueRange;
    }
    wxDouble rangeOrderOfMagnitude = CalculateOrderOfMagnitude(valueRange);
    graphMaxValue = ceil(maxValue / (1 * pow(10, rangeOrderOfMagnitude))) * pow(10, rangeOrderOfMagnitude);
    graphMinValue = floor(minValue / (1 * pow(10, rangeOrderOfMagnitude))) * pow(10, rangeOrderOfMagnitude);
    wxDouble graphRange = graphMaxValue - graphMinValue;
    stepValue = pow(10, rangeOrderOfMagnitude);
    steps = round(graphRange / stepValue);
}

wxDouble wxChartsUtilities::CalculateOrderOfMagnitude(wxDouble value)
{
    return floor(log10(value));
}

void wxChartsUtilities::BuildNumericalLabels(wxDouble minValue,
                                             size_t steps,
                                             wxDouble stepValue,
                                             const wxChartsLabelOptions &options,
                                             wxVector<wxChartsLabel> &labels)
{
    size_t stepDecimalPlaces = GetDecimalPlaces();

    for (size_t i = 0; i <= steps; ++i)
    {
        wxDouble value = minValue + (i * stepValue);//.toFixed(stepDecimalPlaces);
        std::stringstream valueStr;
        valueStr << value;

        labels.push_back(wxChartsLabel(valueStr.str(), options));
    }
}

void wxChartsUtilities::GetTextSize(wxGraphicsContext &gc,
                                    const wxFont &font, 
                                    const wxString &string,
                                    wxDouble &width,
                                    wxDouble &height)
{
    wxDouble descent;
    wxDouble externalLeading;
    gc.SetFont(font, *wxBLACK);
    gc.GetTextExtent(string, &width, &height, &descent, &externalLeading);
}
