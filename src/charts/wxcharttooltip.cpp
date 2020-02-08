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

#include "wxcharttooltip.h"
#include "wxchartbackground.h"
#include "wxchartsutilities.h"
#include <wx/brush.h>

wxChartTooltip::wxChartTooltip(const wxPoint2DDouble &position,
                               const wxString &text)
    : m_position(position), m_provider(new wxChartTooltipProviderStatic("", text, *wxBLACK))
{
}

wxChartTooltip::wxChartTooltip(const wxPoint2DDouble &position,
                               const wxChartTooltipProvider::ptr provider)
    : m_position(position), m_provider(provider)
{
}

void wxChartTooltip::Draw(wxGraphicsContext &gc)
{
    wxString text = m_provider->GetTooltipText();

    wxFont font(wxSize(0, m_options.GetFontSize()),
        m_options.GetFontFamily(), m_options.GetFontStyle(), wxFONTWEIGHT_NORMAL);
    wxDouble tooltipWidth;
    wxDouble tooltipHeight;
    wxChartsUtilities::GetTextSize(gc, font, text, tooltipWidth, tooltipHeight);
    tooltipWidth += 2 * m_options.GetHorizontalPadding();
    tooltipHeight += 2 * m_options.GetVerticalPadding();


    wxDouble tooltipX = m_position.m_x - (tooltipWidth / 2);
    wxDouble tooltipY = m_position.m_y - tooltipHeight;


    wxChartBackground background(m_options.GetBackgroundOptions());
    background.Draw(tooltipX, tooltipY, tooltipWidth, tooltipHeight, gc);

    gc.SetFont(font, m_options.GetFontColor());
    gc.DrawText(text, tooltipX + m_options.GetHorizontalPadding(), tooltipY + m_options.GetVerticalPadding());
}

const wxPoint2DDouble& wxChartTooltip::GetPosition() const
{
    return m_position;
}

const wxChartTooltipProvider::ptr& wxChartTooltip::GetProvider() const
{
    return m_provider;
}
