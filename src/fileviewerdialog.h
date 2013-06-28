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

#ifndef _MM_EX_FILEVIEWER_H_
#define _MM_EX_FILEVIEWER_H_

#include "guiid.h"
#include "defs.h"

#define SYMBOL_FILEVIEWER_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_FILEVIEWER_TITLE _("View File Contents")
#define SYMBOL_FILEVIEWER_IDNAME ID_DIALOG_FILEVIEWER
#define SYMBOL_FILEVIEWER_SIZE wxSize(640, 480)
#define SYMBOL_FILEVIEWER_POSITION wxDefaultPosition

class fileviewer: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( fileviewer )
    DECLARE_EVENT_TABLE()

public:
    fileviewer( );
    fileviewer( const wxString& fileName, wxWindow* parent,
                wxWindowID id = SYMBOL_FILEVIEWER_IDNAME,
                const wxString& caption = SYMBOL_FILEVIEWER_TITLE,
                const wxPoint& pos = SYMBOL_FILEVIEWER_POSITION,
                const wxSize& size = SYMBOL_FILEVIEWER_SIZE,
                long style = SYMBOL_FILEVIEWER_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_FILEVIEWER_IDNAME,
                 const wxString& caption = SYMBOL_FILEVIEWER_TITLE,
                 const wxPoint& pos = SYMBOL_FILEVIEWER_POSITION,
                 const wxSize& size = SYMBOL_FILEVIEWER_SIZE,
                 long style = SYMBOL_FILEVIEWER_STYLE );

    void CreateControls();
    wxTextCtrl *textCtrl_;

private:
    wxString fileName_;
};

#endif

