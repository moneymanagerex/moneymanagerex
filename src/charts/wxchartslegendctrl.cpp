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

#include "wxchartslegendctrl.h"
#include <wx/dcbuffer.h>
#include <wx/graphics.h>

wxChartsLegendCtrl::wxChartsLegendCtrl(wxWindow *parent,
                                       wxWindowID id,
                                       const wxChartsLegendData &data,
                                       const wxPoint &pos,
                                       const wxSize &size,
                                       long style)
    : wxControl(parent, id, pos, size, style)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    SetBackgroundColour(*wxWHITE);

    const wxVector<wxChartsLegendItem>& items = data.GetItems();
    for (size_t i = 0; i < items.size(); ++i)
    {
        m_lines.push_back(
            wxChartsLegendLine(items[i].GetColor(), items[i].GetLabel(), 
                m_options.GetLegendLineOptions())
            );
    }
}

void wxChartsLegendCtrl::OnPaint(wxPaintEvent &evt)
{
    wxAutoBufferedPaintDC dc(this);

    dc.Clear();

    wxGraphicsContext* gc = wxGraphicsContext::Create(dc);
    if (gc)
    {
        // Update the size of each line to reflect
        // the currently selected options and the
        // contents of each line.
        for (size_t i = 0; i < m_lines.size(); ++i)
        {
            m_lines[i].UpdateSize(*gc);
        }

        // Set the position of each line based on
        // the size of the lines that precede it.
        wxDouble y = 1;
        for (size_t i = 0; i < m_lines.size(); ++i)
        {
            m_lines[i].SetPosition(0, y);
            y += (m_lines[i].GetSize().GetHeight() + 5);
        }

        // Draw the lines
        for (size_t i = 0; i < m_lines.size(); ++i)
        {
            m_lines[i].Draw(*gc);
        }

        delete gc;
    }
}

BEGIN_EVENT_TABLE(wxChartsLegendCtrl, wxControl)
    EVT_PAINT(wxChartsLegendCtrl::OnPaint)
END_EVENT_TABLE()
