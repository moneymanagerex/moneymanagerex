/*******************************************************
Copyright (C) 2014 Stefano Giorgio

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Placeuite 330, Boston, MA  02111-1307  USA
 ********************************************************/

/****************************************************************************
This frame serves as a test base for dialogs.
*****************************************************************************/ 
#include <wx/frame.h>
#include <wx/menu.h>
#include <wx/msgdlg.h> 
#include <wx/infobar.h>
#include <wx/sizer.h>

#include "framebase_tests.h"

wxBEGIN_EVENT_TABLE(TestFrameBase, wxFrame)
// event table is empty for this frame.
wxEND_EVENT_TABLE()

TestFrameBase::TestFrameBase(int frame_count)
: wxFrame(NULL, wxID_ANY, "TEST Base Frame: MMEX Tests", wxPoint(50, 50), wxSize(400, 150))
{
    wxMenu *menuBase = new wxMenu;
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menuBase, "Testing...");
    SetMenuBar(menuBar);
    CreateStatusBar();
    wxString msg = wxString() << "TEST: Base Frame " << frame_count;
    SetStatusText(msg);
}

TestFrameBase::TestFrameBase(wxWindow* parent, int size_x, int size_y)
: wxFrame(parent, wxID_ANY, "MMEX Test Frame: Used to test Dialogs", wxPoint(60, 60), wxSize(size_x, size_y))
{
    CreateStatusBar();
}

TestFrameBase::TestFrameBase(wxWindow* parent, const wxString& heading)
: wxFrame(parent, wxID_ANY, heading, wxPoint(500, 50), wxSize(400, 150))
{
    m_infoBar = new wxInfoBar(this);
    wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(m_infoBar, wxSizerFlags().Expand().Border());
    SetSizer(sizer);
}

void TestFrameBase::Show_InfoBarMessage(const wxString& info_message)
{
    m_infoBar->ShowMessage(info_message, wxICON_INFORMATION);
}

TestFrameBase::~TestFrameBase()
{
    //wxMessageBox("Test Frame Completed.", "Test Frame Base", wxOK | wxICON_INFORMATION);
}

