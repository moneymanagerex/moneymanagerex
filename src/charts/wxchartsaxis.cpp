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

#include "wxchartsaxis.h"
#include <wx/pen.h>

void wxChartsAxis::Draw(wxGraphicsContext &gc) const
{
    wxPen pen(m_options.GetLineColor(), m_options.GetLineWidth());
    gc.SetPen(pen);

    // Draw the axis
    wxGraphicsPath path = gc.CreatePath();
    if (m_options.GetPosition() == wxCHARTSAXISPOSITION_LEFT)
    {
        path.MoveToPoint(m_startPoint);
        path.AddLineToPoint(m_endPoint.m_x, m_endPoint.m_y - m_options.GetOverhang());
    }
    else if (m_options.GetPosition() == wxCHARTSAXISPOSITION_BOTTOM)
    {
        path.MoveToPoint(m_startPoint);
        path.AddLineToPoint(m_endPoint.m_x + m_options.GetOverhang(), m_endPoint.m_y);
    }
    gc.StrokePath(path);

    DrawTickMarks(gc);	// Draw the little lines corresponding to the labels
    m_labels.Draw(gc);
}

bool wxChartsAxis::HitTest(const wxPoint &point) const
{
    return false;
}

wxPoint2DDouble wxChartsAxis::GetTooltipPosition() const
{
    return wxPoint2DDouble(0, 0);
}

void wxChartsAxis::Fit(wxPoint2DDouble startPoint,
                       wxPoint2DDouble endPoint)
{
    m_startPoint = startPoint;
    m_endPoint = endPoint;
}

void wxChartsAxis::UpdateLabelSizes(wxGraphicsContext &gc)
{
    m_labels.UpdateSizes(gc);
}

void wxChartsAxis::UpdateLabelPositions()
{
    for (size_t i = 0; i < m_labels.size(); ++i)
    {
        m_labels[i].SetPosition(CalculateLabelPosition(i));
    }
}

const wxChartsLabelGroup& wxChartsAxis::GetLabels() const
{
    return m_labels;
}

void wxChartsAxis::SetLabels(const wxVector<wxChartsLabel> &labels)
{
    m_labels.assign(labels.begin(), labels.end());
}

wxPoint2DDouble wxChartsAxis::CalculateLabelPosition(size_t index)
{
    wxDouble marginCorrection = 0;
    if (m_options.GetStartMarginType() == wxCHARTSAXISMARGINTYPE_TICKMARKOFFSET)
    {
        marginCorrection += GetDistanceBetweenTickMarks();
    }
    if (m_options.GetPosition() == wxCHARTSAXISPOSITION_LEFT)
    {
        wxDouble distance = GetDistanceBetweenTickMarks();
        wxPoint2DDouble position(
            m_startPoint.m_x - 10 - m_labels[index].GetSize().GetWidth(),
            m_startPoint.m_y - (distance * index) - (m_labels[index].GetSize().GetHeight() / 2) - marginCorrection
            );

        if (m_options.GetLabelType() == wxCHARTSAXISLABELTYPE_RANGE)
        {
            position.m_y -= (distance / 2);
        }

        return position;
    }
    else if (m_options.GetPosition() == wxCHARTSAXISPOSITION_BOTTOM)
    {
        wxDouble distance = GetDistanceBetweenTickMarks();
        wxPoint2DDouble position(
            m_startPoint.m_x + (distance * index) - (m_labels[index].GetSize().GetWidth() / 2) + marginCorrection,
            m_startPoint.m_y + 8
            );

        /*
        innerWidth = this.width - (this.xScalePaddingLeft + this.xScalePaddingRight),
        valueWidth = innerWidth/Math.max((this.valuesCount - ((this.offsetGridLines) ? 0 : 1)), 1),
        valueOffset = (valueWidth * index) + this.xScalePaddingLeft;
        */

        if (m_options.GetLabelType() == wxCHARTSAXISLABELTYPE_RANGE)
        {
            position.m_x += (distance / 2);
        }

        /*
        return Math.round(valueOffset);
        */

        return position;
    }

    wxTrap();
    return wxPoint2DDouble(0, 0);
}

size_t wxChartsAxis::GetNumberOfTickMarks() const
{
    if (m_options.GetLabelType() == wxCHARTSAXISLABELTYPE_POINT)
    {
        return m_labels.size();
    }
    else
    {
        return (m_labels.size() + 1);
    }
}

wxDouble wxChartsAxis::GetDistanceBetweenTickMarks() const
{
    size_t marginCorrection = 0;
    if (m_options.GetStartMarginType() == wxCHARTSAXISMARGINTYPE_TICKMARKOFFSET)
    {
        ++marginCorrection;
    }
    if (m_options.GetEndMarginType() == wxCHARTSAXISMARGINTYPE_TICKMARKOFFSET)
    {
        ++marginCorrection;
    }
    if (m_options.GetPosition() == wxCHARTSAXISPOSITION_LEFT)
    {
        return ((m_startPoint.m_y - m_endPoint.m_y) / (GetNumberOfTickMarks() + marginCorrection - 1));
    }
    else if (m_options.GetPosition() == wxCHARTSAXISPOSITION_BOTTOM)
    {
        return ((m_endPoint.m_x - m_startPoint.m_x) / (GetNumberOfTickMarks() + marginCorrection - 1));
    }

    wxTrap();
    return 0;
}

wxPoint2DDouble wxChartsAxis::GetTickMarkPosition(size_t index) const
{
    wxDouble marginCorrection = 0;
    if (m_options.GetStartMarginType() == wxCHARTSAXISMARGINTYPE_TICKMARKOFFSET)
    {
        marginCorrection += GetDistanceBetweenTickMarks();
    }
    if (m_options.GetPosition() == wxCHARTSAXISPOSITION_LEFT)
    {
        wxDouble valueOffset = m_startPoint.m_y - (GetDistanceBetweenTickMarks() * index) - marginCorrection;
        return wxPoint2DDouble(m_startPoint.m_x, valueOffset);
    }
    else if (m_options.GetPosition() == wxCHARTSAXISPOSITION_BOTTOM)
    {
        wxDouble valueOffset = m_startPoint.m_x + (GetDistanceBetweenTickMarks() * index) + marginCorrection;
        return wxPoint2DDouble(valueOffset, m_startPoint.m_y);
    }

    wxTrap();
    return wxPoint2DDouble(0, 0);
}

wxPoint2DDouble wxChartsAxis::GetPosition(wxDouble relativeValue) const
{
    if (m_options.GetPosition() == wxCHARTSAXISPOSITION_LEFT)
    {
        return wxPoint2DDouble(
            m_startPoint.m_x,
            m_startPoint.m_y - (relativeValue * (m_startPoint.m_y - m_endPoint.m_y))
            );
    }
    else if (m_options.GetPosition() == wxCHARTSAXISPOSITION_BOTTOM)
    {
        return wxPoint2DDouble(
            m_startPoint.m_x + (relativeValue * (m_endPoint.m_x - m_startPoint.m_x)),
            m_startPoint.m_y
            );
    }

    wxTrap();
    return wxPoint2DDouble(0, 0);
}

const std::string& wxChartsAxis::GetId() const
{
    return m_id;
}

const wxChartsAxisOptions& wxChartsAxis::GetOptions() const
{
    return m_options;
}

wxChartsAxis::wxChartsAxis(const std::string &id, 
                           const wxChartsAxisOptions &options)
    : m_options(options), m_id(id), 
    m_startPoint(0, 0), m_endPoint(0, 0)
{
}

wxChartsAxis::wxChartsAxis(const std::string &id, 
                           const wxVector<wxString> &labels,
                           const wxChartsAxisOptions &options)
    : m_options(options), m_id(id),
    m_startPoint(0, 0), m_endPoint(0, 0)
{
    for (size_t i = 0; i < labels.size(); ++i)
    {
        m_labels.push_back(
            wxChartsLabel(
                labels[i], 
                wxChartsLabelOptions(m_options.GetFontOptions(), false, wxChartBackgroundOptions(*wxWHITE, 0))
                )
            );
    }
}

void wxChartsAxis::DrawTickMarks(wxGraphicsContext &gc) const
{
    if (m_options.GetPosition() == wxCHARTSAXISPOSITION_LEFT)
    {
        size_t n = GetNumberOfTickMarks();
        for (size_t i = 0; i < n; ++i)
        {
            wxDouble linePositionY = GetTickMarkPosition(i).m_y;

            wxGraphicsPath path = gc.CreatePath();
            path.MoveToPoint(m_startPoint.m_x - 5, linePositionY);
            path.AddLineToPoint(m_startPoint.m_x, linePositionY);
            gc.StrokePath(path);
        }
    }
    else if (m_options.GetPosition() == wxCHARTSAXISPOSITION_BOTTOM)
    {
        size_t n = GetNumberOfTickMarks();
        for (size_t i = 0; i < n; ++i)
        {
            wxDouble linePosition = GetTickMarkPosition(i).m_x;

            wxGraphicsPath path = gc.CreatePath();
            path.MoveToPoint(linePosition, m_startPoint.m_y);
            path.AddLineToPoint(linePosition, m_startPoint.m_y + 5);
            gc.StrokePath(path);
        }
    }
}
