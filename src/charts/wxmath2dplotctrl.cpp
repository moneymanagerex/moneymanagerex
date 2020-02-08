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

    Copyright (c) 2013-2017 Nick Downie
    Released under the MIT license
    https://github.com/nnnick/Chart.js/blob/master/LICENSE.md
*/

/// @file

#include "wxmath2dplotctrl.h"
#include <wx/filedlg.h>

wxMath2DPlotCtrl::wxMath2DPlotCtrl(wxWindow *parent,
                                   wxWindowID id,
                                   const wxMath2DPlotData &data,
                                   const wxPoint &pos,
                                   const wxSize &size,
                                   long style)
    : wxChartCtrl(parent, id, pos, size, style),
      m_math2dPlot(data, size)
{
    CreateContextMenu();
}

wxMath2DPlotCtrl::wxMath2DPlotCtrl(wxWindow *parent,
                                   wxWindowID id,
                                   const wxMath2DPlotData &data,
                                   wxSharedPtr<wxMath2DPlotOptions> &options,
                                   const wxPoint &pos,
                                   const wxSize &size,
                                   long style)
    : wxChartCtrl(parent, id, pos, size, style),
      m_math2dPlot(data, options, size)
{
    CreateContextMenu();
}

wxMath2DPlot& wxMath2DPlotCtrl::GetChart()
{
    return m_math2dPlot;
}

const wxChartsGridOptions& wxMath2DPlotCtrl::GetGridOptions() const
{
    return m_math2dPlot.GetGridOptions();
}

void wxMath2DPlotCtrl::SetGridOptions(const wxChartsGridOptions& opt)
{
    m_math2dPlot.SetGridOptions(opt);
    Update();
}

const wxMath2DPlotOptions& wxMath2DPlotCtrl::GetChartOptions() const
{
    return m_math2dPlot.GetChartOptions();
}

void  wxMath2DPlotCtrl::SetChartOptions(const wxMath2DPlotOptions& opt)
{
    m_math2dPlot.SetChartOptions(opt);
    Update();
}

void wxMath2DPlotCtrl::SetChartType(std::size_t index,const wxChartType &type)
{
    if(!m_math2dPlot.SetChartType(index,type))
        return;
    Update();
}

bool wxMath2DPlotCtrl::UpdateData(std::size_t index,const wxVector<wxPoint2DDouble> &points)
{
    if (!m_math2dPlot.UpdateData(index,points))
        return false;
    Update();
    return true;
}

bool wxMath2DPlotCtrl::AddData(std::size_t index,const wxVector<wxPoint2DDouble> &points)
{
    if (!m_math2dPlot.AddData(index,points))
        return false;
    Update();
    return true;
}

void wxMath2DPlotCtrl::AddDataset(const wxMath2DPlotDataset::ptr &newset)
{
    m_math2dPlot.AddDataset(newset);
    Update();
}

bool wxMath2DPlotCtrl::RemoveDataset(std::size_t index)
{
    if (!m_math2dPlot.RemoveDataset(index))
        return false;
    Update();
    return true;
}

void wxMath2DPlotCtrl::Update()
{
    auto parent = this->GetParent();
    if(parent)
        parent->Layout();
}

void wxMath2DPlotCtrl::CreateContextMenu()
{
    m_posX = 0;
    m_posY = 0;
    m_contextMenu.Append(wxID_SAVEAS, wxString("Save as"));
    m_subMenu = new wxMenu;
    m_subMenu->Append(wxID_DEFAULT, wxString("Set default zoom"));
    m_subMenu->Append(wxID_UP, wxString("Zoom +"));
    m_subMenu->Append(wxID_DOWN, wxString("Zoom -"));

    m_contextMenu.AppendSubMenu(m_subMenu,wxString("Zoom"));
    Bind(wxEVT_CONTEXT_MENU,
         [this](wxContextMenuEvent& evt)
    {
        PopupMenu(&m_contextMenu, ScreenToClient(evt.GetPosition()));
    }
        );
    m_contextMenu.Bind(wxEVT_MENU,
                       [this](wxCommandEvent &)
    {
        m_contextMenu.Enable(wxID_DOWN,true);
        m_contextMenu.Enable(wxID_UP,true);
        m_math2dPlot.Scale(0);
        auto parent = this->GetParent();
        if(parent)
            parent->Layout();
    },wxID_DEFAULT);
    m_contextMenu.Bind(wxEVT_MENU,
                       [this](wxCommandEvent &)
    {

        if(!m_math2dPlot.Scale(2))
            m_contextMenu.Enable(wxID_UP,false);
        auto parent = this->GetParent();
        if(parent)
            parent->Layout();
    },wxID_UP);
    m_contextMenu.Bind(wxEVT_MENU,
                       [this](wxCommandEvent &)
    {
        if(!m_math2dPlot.Scale(-2))
            m_contextMenu.Enable(wxID_DOWN,false);
        auto parent = this->GetParent();
        if(parent)
            parent->Layout();
    },wxID_DOWN);

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

        m_math2dPlot.Save(filename, type, GetSize(), GetBackgroundColour());
    },
    wxID_SAVEAS
                      );

    this->Bind(wxEVT_LEFT_DOWN,
               [this](wxMouseEvent&  evt)
    {
        m_posX = evt.m_x;
        m_posY = evt.m_y;
        this->SetCursor(wxCURSOR_HAND);
        evt.Skip();
    });
    this->Bind(wxEVT_LEFT_UP,
               [this](wxMouseEvent&  evt)
    {
        this->SetCursor(wxCURSOR_ARROW);
        evt.Skip();
    });
    this->Bind(wxEVT_MOTION,
               [this](wxMouseEvent& evt)
    {
        evt.Skip();

        if(evt.ButtonIsDown(wxMouseButton::wxMOUSE_BTN_LEFT))
        {
            double dx = m_posX-evt.m_x;
            double dy = m_posY-evt.m_y;
            if( std::abs(dx) > 5 || std::abs(dy) > 5)
            {
                auto parent = this->GetParent();
                if(parent)
                {
                    auto Size = parent->GetSize();
                    m_math2dPlot.Shift(dx/Size.GetX(),dy/Size.GetY());
                    m_posX = evt.m_x;
                    m_posY = evt.m_y;
                    parent->Layout();
                }
            }
        }
    });
}
