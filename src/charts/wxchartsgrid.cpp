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

/*
    Part of this file were copied from the Chart.js project (http://chartjs.org/)
    and translated into C++.

    The files of the Chart.js project have the following copyright and license.

    Copyright (c) 2013-2016 Nick Downie
    Released under the MIT license
    https://github.com/nnnick/Chart.js/blob/master/LICENSE.md
*/

#include "wxchartsgrid.h"
#include "wxchartsnumericalaxis.h"
#include "wxchartscategoricalaxis.h"
#include <wx/pen.h>

static const wxDouble MinDistance = 1.0e-3;
static const wxDouble MaxDistance = 1.0e3;

wxChartsGrid::wxChartsGrid(const wxPoint2DDouble &position,
                           const wxSize &size,
                           wxChartsAxis::ptr xAxis,
                           wxChartsAxis::ptr yAxis,
                           const wxChartsGridOptions& options)
    : m_options(options), m_position(position),
    m_XAxis(xAxis), m_YAxis(yAxis),
    m_mapping(size, m_XAxis, m_YAxis),
    m_needsFit(true)
{
}

wxChartsGrid::wxChartsGrid(const wxPoint2DDouble &position,
                           const wxSize &size,
                           wxDouble minXValue,
                           wxDouble maxXValue,
                           wxDouble minYValue,
                           wxDouble maxYValue,
                           const wxChartsGridOptions& options)
    : m_options(options), m_position(position),
      m_XAxis(new wxChartsNumericalAxis("x", minXValue, maxXValue, options.GetXAxisOptions())),
      m_YAxis(new wxChartsNumericalAxis("y", minYValue, maxYValue, options.GetYAxisOptions())),
      m_mapping(size, m_XAxis, m_YAxis),
      m_needsFit(true),
      m_origAxisLimits(minXValue,maxXValue,minYValue,maxYValue),
      m_curAxisLimits(minXValue,maxXValue,minYValue,maxYValue)
{
}

void wxChartsGrid::Draw(wxGraphicsContext &gc) const
{
    const wxChartsAxis* verticalAxis = 0;
    if (m_XAxis->GetOptions().GetPosition() == wxCHARTSAXISPOSITION_LEFT)
    {
        verticalAxis = m_XAxis.get();
    }
    else if (m_YAxis->GetOptions().GetPosition() == wxCHARTSAXISPOSITION_LEFT)
    {
        verticalAxis = m_YAxis.get();
    }

    const wxChartsAxis* horizontalAxis = 0;
    if (m_XAxis->GetOptions().GetPosition() == wxCHARTSAXISPOSITION_BOTTOM)
    {
        horizontalAxis = m_XAxis.get();
    }
    else if (m_YAxis->GetOptions().GetPosition() == wxCHARTSAXISPOSITION_BOTTOM)
    {
        horizontalAxis = m_YAxis.get();
    }

    if (m_options.GetHorizontalGridLineOptions().ShowGridLines())
    {
        DrawHorizontalGridLines(*horizontalAxis, *verticalAxis, m_options.GetHorizontalGridLineOptions(), gc);
    }

    if (m_options.GetVerticalGridLineOptions().ShowGridLines())
    {
        DrawVerticalGridLines(*horizontalAxis, *verticalAxis, m_options.GetVerticalGridLineOptions(), gc);
    }

    m_XAxis->Draw(gc);
    m_YAxis->Draw(gc);
}

bool wxChartsGrid::HitTest(const wxPoint &point) const
{
    return false;
}

wxPoint2DDouble wxChartsGrid::GetTooltipPosition() const
{
    return wxPoint2DDouble(0, 0);
}

void wxChartsGrid::Fit(wxGraphicsContext &gc)
{
    if (!m_needsFit)
    {
        return;
    }

    wxDouble startPoint = m_mapping.GetSize().GetHeight() - (m_YAxis->GetOptions().GetFontOptions().GetSize() + 15) - 5; // -5 to pad labels
    wxDouble endPoint = m_YAxis->GetOptions().GetFontOptions().GetSize();

    // Apply padding settings to the start and end point.
    //this.startPoint += this.padding;
    //this.endPoint -= this.padding;

    m_YAxis->UpdateLabelSizes(gc);
    m_XAxis->UpdateLabelSizes(gc);

    wxDouble leftPadding = 0;
    wxDouble rightPadding = 0;
    CalculatePadding(*m_XAxis, *m_YAxis, leftPadding, rightPadding);

    if (m_XAxis->GetOptions().GetPosition() == wxCHARTSAXISPOSITION_BOTTOM)
    {
        m_XAxis->Fit(wxPoint2DDouble(leftPadding, startPoint), wxPoint2DDouble(m_mapping.GetSize().GetWidth() - rightPadding, startPoint));
        m_YAxis->Fit(wxPoint2DDouble(leftPadding, startPoint), wxPoint2DDouble(leftPadding, endPoint));
    }
    else if (m_XAxis->GetOptions().GetPosition() == wxCHARTSAXISPOSITION_LEFT)
    {
        m_XAxis->Fit(wxPoint2DDouble(leftPadding, startPoint), wxPoint2DDouble(leftPadding, endPoint));
        m_YAxis->Fit(wxPoint2DDouble(leftPadding, startPoint), wxPoint2DDouble(m_mapping.GetSize().GetWidth() - rightPadding, startPoint));
    }

    m_XAxis->UpdateLabelPositions();
    m_YAxis->UpdateLabelPositions();

    m_needsFit = false;
}

void wxChartsGrid::Resize(const wxSize &size)
{
    m_mapping.SetSize(size);
    m_needsFit = true;
}

const wxChartsGridMapping& wxChartsGrid::GetMapping() const
{
    return m_mapping;
}

void wxChartsGrid::SetOptions(const wxChartsGridOptions& opt)
{
    m_options = opt;
}

const wxChartsGridOptions& wxChartsGrid::GetOptions() const
{
    return m_options;
}

bool wxChartsGrid::Scale(int c)
{
    if(c)
    {
        c = c>0 ? 4 : -2;
        double deltaX = (m_curAxisLimits.MaxX-m_curAxisLimits.MinX)/c;
        double deltaY = (m_curAxisLimits.MaxY-m_curAxisLimits.MinY)/c;

        auto absDX = std::abs(deltaX);
        auto absDY = std::abs(deltaY);

        if(absDX > MaxDistance || absDX < MinDistance
                || absDY > MaxDistance || absDY < MinDistance)
            return false;

        m_curAxisLimits.MinX+=deltaX;
        m_curAxisLimits.MaxX-=deltaX;
        m_curAxisLimits.MinY+=deltaY;
        m_curAxisLimits.MaxY-=deltaY;
    }
    else
        m_curAxisLimits = m_origAxisLimits;
    Update();
    return true;
}

void wxChartsGrid::Shift(double dx,double dy)
{
    double deltaX = (m_curAxisLimits.MaxX-m_curAxisLimits.MinX)*dx;
    m_curAxisLimits.MinX+=deltaX;
    m_curAxisLimits.MaxX+=deltaX;

    double deltaY = (m_curAxisLimits.MaxY-m_curAxisLimits.MinY)*dy;
    m_curAxisLimits.MinY+=deltaY;
    m_curAxisLimits.MaxY+=deltaY;

    Update();
}

void wxChartsGrid::UpdateAxisLimit(const std::string& axisId, wxDouble min, wxDouble max)
{
    if(axisId == "x")
    {
        m_origAxisLimits.MinX = min;
        m_origAxisLimits.MaxX = max;

        m_curAxisLimits.MinX = min;
        m_curAxisLimits.MaxX = max;

        m_XAxis = new wxChartsNumericalAxis(axisId, m_curAxisLimits.MinX,
            m_curAxisLimits.MaxX,m_options.GetXAxisOptions());
    }
    else if(axisId == "y")
    {
        m_origAxisLimits.MinY = min;
        m_origAxisLimits.MaxY = max;

        m_curAxisLimits.MinY = min;
        m_curAxisLimits.MaxY = max;
        m_YAxis = new wxChartsNumericalAxis(axisId, m_curAxisLimits.MinY,
            m_curAxisLimits.MaxY, m_options.GetYAxisOptions());
    }
    m_mapping = wxChartsGridMapping(m_mapping.GetSize(), m_XAxis, m_YAxis);
    m_needsFit = true;
}

void wxChartsGrid::ChangeLabels(const std::string& axisId, const wxVector<wxString> &labels, wxChartsAxisOptions options)
{
    if(axisId == "x")
        m_XAxis = wxChartsCategoricalAxis::make_shared(axisId,labels,options);
    else if(axisId == "y")
        m_YAxis = wxChartsCategoricalAxis::make_shared(axisId,labels,options);

    m_mapping = wxChartsGridMapping(m_mapping.GetSize(), m_XAxis, m_YAxis);
    m_needsFit = true;
}

void wxChartsGrid::Update()
{
    m_XAxis = new wxChartsNumericalAxis("x", m_curAxisLimits.MinX,
        m_curAxisLimits.MaxX,m_options.GetXAxisOptions());
    m_YAxis = new wxChartsNumericalAxis("y", m_curAxisLimits.MinY,
        m_curAxisLimits.MaxY, m_options.GetYAxisOptions());
    m_mapping = wxChartsGridMapping(m_mapping.GetSize(), m_XAxis, m_YAxis);
    m_needsFit = true;
}

void wxChartsGrid::CalculatePadding(const wxChartsAxis &xAxis,
                                    const wxChartsAxis &yAxis,
                                    wxDouble &left,
                                    wxDouble &right)
{
    if (xAxis.GetOptions().GetPosition() == wxCHARTSAXISPOSITION_BOTTOM)
    {
        // Either the first x label or any of the y labels can be the widest
        // so they are all taken into account to compute the left padding
        left = yAxis.GetLabels().GetMaxWidth() + 10;
        if ((xAxis.GetLabels().size() > 0) && ((xAxis.GetLabels().front().GetSize().GetWidth() / 2) > left))
        {
            left = (xAxis.GetLabels().front().GetSize().GetWidth() / 2);
        }

        right = 0;
        if (xAxis.GetLabels().size() > 0)
        {
            right = (xAxis.GetLabels().back().GetSize().GetWidth() / 2);
        }
    }
    else if (xAxis.GetOptions().GetPosition() == wxCHARTSAXISPOSITION_LEFT)
    {
        // Either the first y label or any of the x labels can be the widest
        // so they are all taken into account to compute the left padding
        left = xAxis.GetLabels().GetMaxWidth() + 10;
        if ((yAxis.GetLabels().size() > 0) && ((yAxis.GetLabels().front().GetSize().GetWidth() / 2) > left))
        {
            left = (yAxis.GetLabels().front().GetSize().GetWidth() / 2);
        }

        right = 0;
        if (yAxis.GetLabels().size() > 0)
        {
            right = (yAxis.GetLabels().back().GetSize().GetWidth() / 2);
        }
    }
}

void wxChartsGrid::DrawHorizontalGridLines(const wxChartsAxis &horizontalAxis,
                                           const wxChartsAxis &verticalAxis,
                                           const wxChartsGridLineOptions &options,
                                           wxGraphicsContext &gc)
{
    for (size_t i = 1; i < verticalAxis.GetNumberOfTickMarks(); ++i)
    {
        wxPoint2DDouble lineStartPosition = verticalAxis.GetTickMarkPosition(i);
        wxPoint2DDouble lineEndPosition = horizontalAxis.GetTickMarkPosition(horizontalAxis.GetNumberOfTickMarks() - 1);
        if (horizontalAxis.GetOptions().GetEndMarginType() == wxCHARTSAXISMARGINTYPE_TICKMARKOFFSET)
        {
            lineEndPosition.m_x += horizontalAxis.GetDistanceBetweenTickMarks();
        }

        wxGraphicsPath path = gc.CreatePath();
        path.MoveToPoint(lineStartPosition);
        path.AddLineToPoint(lineEndPosition.m_x + horizontalAxis.GetOptions().GetOverhang(), lineStartPosition.m_y);

        wxPen pen1(options.GetMajorGridLineColor(), options.GetMajorGridLineWidth());
        gc.SetPen(pen1);
        gc.StrokePath(path);

        unsigned int n = options.GetNumberOfMinorGridLinesBetweenTickMarks();
        if (n != 0)
        {
            wxDouble spacing = verticalAxis.GetDistanceBetweenTickMarks() / (n + 1);
            wxDouble currentSpacing = spacing;
            for (size_t j = 0; j < n; ++j)
            {
                wxGraphicsPath path = gc.CreatePath();
                path.MoveToPoint(lineStartPosition.m_x, lineStartPosition.m_y + currentSpacing);
                path.AddLineToPoint(lineEndPosition.m_x + horizontalAxis.GetOptions().GetOverhang(), lineStartPosition.m_y + currentSpacing);

                wxPen pen1(wxColor(0, 0, 0, 0x0C), 1);
                gc.SetPen(pen1);
                gc.StrokePath(path);

                currentSpacing += spacing;
            }
        }
    }
}

void wxChartsGrid::DrawVerticalGridLines(const wxChartsAxis &horizontalAxis,
                                         const wxChartsAxis &verticalAxis,
                                         const wxChartsGridLineOptions &options,
                                         wxGraphicsContext &gc)
{
    size_t i = 1;
    if (horizontalAxis.GetOptions().GetStartMarginType() == wxCHARTSAXISMARGINTYPE_TICKMARKOFFSET)
    {
        i = 0;
    }
    size_t end = horizontalAxis.GetNumberOfTickMarks();
    if (horizontalAxis.GetOptions().GetEndMarginType() == wxCHARTSAXISMARGINTYPE_TICKMARKOFFSET)
    {
        ++end;
    }
    for (; i < end; ++i)
    {
        wxPoint2DDouble lineStartPosition = horizontalAxis.GetTickMarkPosition(i);
        wxPoint2DDouble lineEndPosition = verticalAxis.GetTickMarkPosition(verticalAxis.GetNumberOfTickMarks() - 1);

        wxGraphicsPath path = gc.CreatePath();
        path.MoveToPoint(lineStartPosition);
        path.AddLineToPoint(lineStartPosition.m_x, lineEndPosition.m_y - verticalAxis.GetOptions().GetOverhang());

        wxPen pen1(options.GetMajorGridLineColor(), options.GetMajorGridLineWidth());
        gc.SetPen(pen1);
        gc.StrokePath(path);

        unsigned int n = options.GetNumberOfMinorGridLinesBetweenTickMarks();
        if (n != 0)
        {
            wxDouble spacing = horizontalAxis.GetDistanceBetweenTickMarks() / (n + 1);
            wxDouble currentSpacing = spacing;
            for (size_t j = 0; j < n; ++j)
            {
                wxGraphicsPath path = gc.CreatePath();
                path.MoveToPoint(lineStartPosition.m_x - currentSpacing, lineStartPosition.m_y);
                path.AddLineToPoint(lineStartPosition.m_x - currentSpacing, lineEndPosition.m_y - verticalAxis.GetOptions().GetOverhang());

                wxPen pen1(wxColor(0, 0, 0, 0x0C), 1);
                gc.SetPen(pen1);
                gc.StrokePath(path);

                currentSpacing += spacing;
            }
        }
    }
}
