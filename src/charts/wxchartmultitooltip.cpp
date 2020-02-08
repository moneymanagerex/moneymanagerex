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

#include "wxchartmultitooltip.h"
#include "wxchartsutilities.h"
#include <wx/brush.h>

wxChartMultiTooltip::wxChartMultiTooltip(const wxString &title)
    : m_title(title)
{
}

wxChartMultiTooltip::wxChartMultiTooltip(const wxString &title,
    const wxChartMultiTooltipOptions &options)
    : m_title(title), m_options(options)
{
}

void wxChartMultiTooltip::Draw(wxGraphicsContext &gc)
{
    // First we will compute the size of each of the lines
    // of the multi-tooltip and its total size.
    /////

    wxFont titleFont(m_options.GetTitleFontOptions().GetFont());
    wxFont textFont(m_options.GetTextFontOptions().GetFont());

    // Get the size of the title
    wxDouble titleWidth = 0;
    wxDouble titleHeight = 0;
    if (m_options.ShowTitle())
    {
        wxChartsUtilities::GetTextSize(gc, titleFont, m_title, titleWidth, titleHeight);
    }

    // Update the size of each line to reflect the currently
    // selected options and the contents of each line.
    for (size_t i = 0; i < m_lines.size(); ++i)
    {
        m_lines[i].UpdateSize(gc);
    }

    // Compute the total outer and inner size of the
    // multi-tooltip.
    wxDouble totalInnerWidth = titleWidth;
    wxDouble totalInnerHeight = titleHeight;
    for (size_t i = 0; i < m_lines.size(); ++i)
    {
        wxSize size = m_lines[i].GetSize();

        totalInnerHeight += size.GetHeight();
        if (m_options.ShowTitle() || (i != 0))
        {
            totalInnerHeight += m_options.GetLineSpacing();
        }

        if (size.GetWidth() > totalInnerWidth)
        {
            totalInnerWidth = size.GetWidth();
        }
    }

    wxDouble totalOuterWidth = totalInnerWidth + (2 * m_options.GetHorizontalPadding());
    wxDouble totalOuterHeight = totalInnerHeight + (2 * m_options.GetVerticalPadding());


    
    // Now that we have the size of all the lines we
    // can calculate their positions.
    /////

    // First calculate the position of the center of
    // the tooltip
    wxPoint2DDouble tooltipCenter(0, 0);
    if (m_options.GetAlignment() == wxALIGN_CENTER)
    {
        if (m_tooltipPositions.size() > 0)
        {
            for (size_t i = 0; i < m_tooltipPositions.size(); ++i)
            {
                tooltipCenter.m_x += m_tooltipPositions[i].m_x;
                tooltipCenter.m_y += m_tooltipPositions[i].m_y;
            }

            tooltipCenter.m_x /= m_tooltipPositions.size();
            tooltipCenter.m_y /= m_tooltipPositions.size();
        }
    }
    else if (m_options.GetAlignment() == wxALIGN_TOP)
    {
        if (m_tooltipPositions.size() > 0)
        {
            tooltipCenter.m_y = m_tooltipPositions[0].m_y;
            for (size_t i = 0; i < m_tooltipPositions.size(); ++i)
            {
                tooltipCenter.m_x += m_tooltipPositions[i].m_x;
                if (tooltipCenter.m_y > m_tooltipPositions[i].m_y)
                {
                    tooltipCenter.m_y = m_tooltipPositions[i].m_y;
                }
            }

            tooltipCenter.m_x /= m_tooltipPositions.size();
        }
    }

    // Calculate the coordinates of the upper left corner of
    // the multi-tooltip both with and without the margins
    wxDouble outerX = (tooltipCenter.m_x - (totalOuterWidth / 2));
    wxDouble innerX = (tooltipCenter.m_x - (totalInnerWidth / 2));
    wxDouble outerY = tooltipCenter.m_y;
    wxDouble innerY = tooltipCenter.m_y;
    if (m_options.GetAlignment() == wxALIGN_CENTER)
    {
        outerY -= (totalOuterHeight / 2);
        innerY -= (totalInnerHeight / 2);
    }
    else if (m_options.GetAlignment() == wxALIGN_TOP)
    {
        outerY -= totalOuterHeight;
        innerY = outerY + m_options.GetVerticalPadding();
    }

    // Set the position of each line based on the size of
    // the lines that precede it.
    wxDouble y = innerY + titleHeight;
    if (titleHeight != 0)
    {
        y += m_options.GetLineSpacing();
    }
    for (size_t i = 0; i < m_lines.size(); ++i)
    {
        m_lines[i].SetPosition(innerX, y);
        y += (m_lines[i].GetSize().GetHeight() + m_options.GetLineSpacing());
    }



    // And finally draw everything
    /////

    // Draw the background
    wxGraphicsPath path = gc.CreatePath();
    path.AddRoundedRectangle(outerX, outerY, totalOuterWidth, totalOuterHeight,
        m_options.GetCornerRadius());
    wxBrush brush(m_options.GetBackgroundColor());
    gc.SetBrush(brush);
    gc.FillPath(path);

    if (m_options.ShowTitle())
    {
        // Draw the title
        gc.SetFont(titleFont, m_options.GetTitleFontOptions().GetColor());
        gc.DrawText(m_title, innerX, innerY);
    }

    // Draw the lines
    for (size_t i = 0; i < m_lines.size(); ++i)
    {
        m_lines[i].Draw(gc);
    }
}

void wxChartMultiTooltip::AddTooltip(const wxChartTooltip &tooltip)
{
    m_tooltipPositions.push_back(tooltip.GetPosition());
    m_tooltipProviders.push_back(tooltip.GetProvider());
    m_lines.push_back(wxChartsLegendLine(
        tooltip.GetProvider()->GetAssociatedColor(), 
        tooltip.GetProvider()->GetTooltipText(), 
        wxChartsLegendLineOptions(m_options.GetTextFontOptions())
        ));
}
