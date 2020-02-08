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

/// @file

#include "wxlinechartctrl.h"
#include <wx/filedlg.h>

wxLineChartCtrl::wxLineChartCtrl(wxWindow *parent,
                                 wxWindowID id,
                                 wxChartsCategoricalData::ptr &data,
                                 const wxChartsLineType &lineType,
                                 const wxPoint &pos,
                                 const wxSize &size,
                                 long style)
    : wxChartCtrl(parent, id, pos, size, style),
    m_lineChart(data, lineType, size)
{
    CreateContextMenu();
}

wxLineChartCtrl::wxLineChartCtrl(wxWindow *parent,
                                 wxWindowID id,
                                 wxChartsCategoricalData::ptr &data,
                                 const wxChartsLineType &lineType,
                                 const wxLineChartOptions &options,
                                 const wxPoint &pos,
                                 const wxSize &size,
                                 long style)
    : wxChartCtrl(parent, id, pos, size, style),
    m_lineChart(data, lineType, options, size)
{
    CreateContextMenu();
}

wxLineChart& wxLineChartCtrl::GetChart()
{
    return m_lineChart;
}

void wxLineChartCtrl::CreateContextMenu()
{
    m_contextMenu.Append(wxID_SAVEAS, wxString("Save as"));

    Bind(wxEVT_CONTEXT_MENU, 
        [this](wxContextMenuEvent& evt) 
        {
            PopupMenu(&m_contextMenu, ScreenToClient(evt.GetPosition()));
        }
        );

    m_contextMenu.Bind(wxEVT_MENU,
        [this](wxCommandEvent &)
        {
            wxFileDialog saveFileDialog(this, _("Save file"), "", "",
                "JPEG files (*.jpg;*.jpeg)|*.jpg;*.jpeg|PNG files (*.png)|*.png",
                wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
            if (saveFileDialog.ShowModal() == wxID_CANCEL)
                return;

            wxString filename = saveFileDialog.GetPath();

            wxBitmapType type = wxBitmapType::wxBITMAP_TYPE_INVALID;
            switch (saveFileDialog.GetFilterIndex())
            {
            case 0:
                type = wxBitmapType::wxBITMAP_TYPE_JPEG;
                if (wxImage::FindHandler(wxBitmapType::wxBITMAP_TYPE_JPEG) == 0)
                {
                    wxImage::AddHandler(new wxJPEGHandler());
                }
                break;

            case 1:
                type = wxBitmapType::wxBITMAP_TYPE_PNG;
                if (wxImage::FindHandler(wxBitmapType::wxBITMAP_TYPE_PNG) == 0)
                {
                    wxImage::AddHandler(new wxPNGHandler());
                }
                break;
            }
            
            m_lineChart.Save(filename, type, GetSize(), GetBackgroundColour());
        },
        wxID_SAVEAS
        );
}
