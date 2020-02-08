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

#include "wxchartsradialgrid.h"
#include "wxchartsutilities.h"
#include <wx/pen.h>

wxChartsRadialGrid::wxChartsRadialGrid(const wxSize &size,
                                       wxDouble minValue,
                                       wxDouble maxValue,
                                       const wxChartsRadialGridOptions& options)
    : m_options(options), m_size(size), m_center(CalculateCenter(size)),
    m_needsFit(true)
{
    m_drawingArea = (size.x < size.y) ? size.x / 2 : size.y / 2;
    wxDouble valueRange = 0;
    wxDouble stepValue;
    wxChartsUtilities::CalculateGridRange(minValue, maxValue, 
        m_graphMinValue, m_graphMaxValue, valueRange, m_steps, stepValue);
    wxChartsUtilities::BuildNumericalLabels(m_graphMinValue, m_steps, stepValue, options.GetLabelOptions(), m_labels);
    // We don't want to display the label at the center of the grid
    m_labels.erase(m_labels.begin());
    for (size_t i = 0; i < m_labels.size(); ++i)
    {
        m_labels[i].SetPadding(wxChartPadding(10, 10, 10, 10));
    }
}

void wxChartsRadialGrid::Draw(wxGraphicsContext &gc) const
{
    switch (m_options.GetStyle())
    {
    case wxCHARTSRADIALGRIDSTYLE_CIRCULAR:
        DrawCircular(gc);
        break;

    case wxCHARTSRADIALGRIDSTYLE_POLYGONAL:
        DrawPolygonal(gc);
        break;
    }
}

bool wxChartsRadialGrid::HitTest(const wxPoint &point) const
{
    return false;
}

wxPoint2DDouble wxChartsRadialGrid::GetTooltipPosition() const
{
    return wxPoint2DDouble(0, 0);
}

void wxChartsRadialGrid::Fit(wxGraphicsContext &gc)
{
    if (!m_needsFit)
    {
        return;
    }

    m_labels.UpdateSizes(gc);

    if (m_options.GetStyle() == wxCHARTSRADIALGRIDSTYLE_CIRCULAR)
    {
        for (size_t i = 0; i < m_labels.size(); ++i)
        {
            wxDouble yCenterOffset = (i + 1) * (m_drawingArea / m_steps);
            wxDouble yHeight = m_center.m_y - yCenterOffset;
            if (m_options.ShowLabels())
            {
                m_labels[i].SetPosition(
                    m_center.m_x - (m_labels[i].GetSize().GetX() / 2),
                    yHeight - (m_labels[i].GetSize().GetY() / 2)
                );
            }
        }
    }

    m_needsFit = false;
}

void wxChartsRadialGrid::Resize(const wxSize &size)
{
    m_size = size;
    m_drawingArea = (size.x < size.y) ? size.x / 2 : size.y / 2;
    m_center = CalculateCenter(size);
}

wxDouble wxChartsRadialGrid::GetRadius(wxDouble value) const
{
    return (((value - m_graphMinValue) / (m_graphMaxValue - m_graphMinValue)) * m_drawingArea);
}

void wxChartsRadialGrid::DrawCircular(wxGraphicsContext &gc) const
{
    for (size_t i = 0; i < m_labels.size(); ++i)
    {
        wxDouble yCenterOffset = (i + 1) * (m_drawingArea / m_steps);
        wxDouble yHeight = m_center.m_y - yCenterOffset;

        wxGraphicsPath path = gc.CreatePath();
        path.AddArc(m_center.m_x, m_center.m_y, yCenterOffset, 0, 2 * M_PI, true);
        path.CloseSubpath();

        wxPen pen(m_options.GetLineColor(), m_options.GetLineWidth());
        gc.SetPen(pen);
        gc.StrokePath(path);
    }

    if (m_options.ShowLabels())
    {
        m_labels.Draw(gc);
    }
}

void wxChartsRadialGrid::DrawPolygonal(wxGraphicsContext &gc) const
{
    // Don't draw a centre value so start from 1
    for (size_t i = 1; i < m_labels.size(); ++i)
    {
        wxGraphicsPath path = gc.CreatePath();
        path.MoveToPoint(100, 100);
        for (size_t j = 1; j < 4; ++j)
        {
            path.AddLineToPoint(50, 50);
            //wxPoint2DDouble pointPosition = 
            //CalculateCenterOffset();

        }
        path.CloseSubpath();

        wxPen pen(m_options.GetLineColor(), m_options.GetLineWidth());
        gc.SetPen(pen);
        gc.StrokePath(path);
    }
}

wxPoint2DDouble wxChartsRadialGrid::CalculateCenter(const wxSize& size)
{
    return wxPoint2DDouble(size.GetWidth() / 2, size.GetHeight() / 2);
}

wxDouble wxChartsRadialGrid::CalculateCenterOffset(wxDouble value,
                                                   wxDouble drawingArea,
                                                   wxDouble minValue,
                                                   wxDouble maxValue)
{
    wxDouble scalingFactor = drawingArea / (maxValue - minValue);

    return ((value - minValue) * scalingFactor);
}
