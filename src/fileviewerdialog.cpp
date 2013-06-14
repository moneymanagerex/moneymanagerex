/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel

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
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ********************************************************/

#include "fileviewerdialog.h"

IMPLEMENT_DYNAMIC_CLASS( fileviewer, wxDialog )

BEGIN_EVENT_TABLE( fileviewer, wxDialog )
END_EVENT_TABLE()

fileviewer::fileviewer( )
{
}

fileviewer::fileviewer(const wxString& fileName, wxWindow* parent,
                       wxWindowID id,const wxString& caption,
                       const wxPoint& pos, const wxSize& size, long style )
: fileName_(fileName)
{
    Create(parent, id, caption, pos, size, style);
}

bool fileviewer::Create( wxWindow* parent, wxWindowID id, const wxString& caption,
                        const wxPoint& pos, const wxSize& size, long style )
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();
    return TRUE;
}

void fileviewer::CreateControls()
{
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer2);

    textCtrl_ = new wxTextCtrl( this,
        wxID_ANY, "",
        wxDefaultPosition, SYMBOL_FILEVIEWER_SIZE,
        wxTE_MULTILINE|wxTE_READONLY );

    itemBoxSizer2->Add(textCtrl_, 1, wxALIGN_CENTER_HORIZONTAL|wxALL|wxGROW, 10);

    /* Load the Text from the file */
    if ( !fileName_.empty() )
    {
         wxFileInputStream input( fileName_ );
         wxTextInputStream text( input );
         while (!input.Eof() )
         {
            wxString line = text.ReadLine();
            if (!line.IsEmpty())
                *textCtrl_ << line << "\n";
            else
                *textCtrl_ << "\n";
        }
    }
    //Scroll to the top of file
    textCtrl_->SetInsertionPoint(0);

    wxStaticLine* line = new wxStaticLine (this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
    itemBoxSizer2->Add(line, 0, wxGROW|wxALL, 5);
    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer10, 0, wxALIGN_RIGHT|wxALL, 5);

    wxButton* itemButtonCANCEL = new wxButton(this, wxID_CANCEL);
    itemBoxSizer10->Add(itemButtonCANCEL, 0, wxALIGN_RIGHT|wxALL, 4);
    wxButton* itemButtonOK = new wxButton( this, wxID_OK);
    itemBoxSizer10->Add(itemButtonOK, 0, wxALIGN_RIGHT|wxALL, 4);
    itemButtonOK->SetFocus();
}
