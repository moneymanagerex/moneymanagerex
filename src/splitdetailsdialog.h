/*******************************************************
Copyright (C) 2006-2012 Madhan Kanagavel
                        Modified by: Stefano Giorgio, Nikolay

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

#ifndef MM_EX_SPLITDETAILSDIALOG_H_
#define MM_EX_SPLITDETAILSDIALOG_H_

#include "defs.h"
#include "mmTextCtrl.h"
#include "Model_Splittransaction.h"

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
#define wxFIXED_MINSIZE 0
#endif

class SplitDetailDialog: public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(SplitDetailDialog);
    wxDECLARE_EVENT_TABLE();

public:
    /// Constructors
    SplitDetailDialog();
    SplitDetailDialog(
        wxWindow* parent
        , Split &split
        , int transType
        , int accountID
    );
    Split getResult() { return split_; }

private:
    bool Create(wxWindow* parent);
    void CreateControls();
    void DataToControls();

    void OnButtonCategoryClick( wxCommandEvent& event );

    void OnButtonOKClick( wxCommandEvent& event );
    void onTextEntered(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& WXUNUSED(event));

    Split split_;
    Model_Currency::Data *m_currency;
    int transType_;

    wxChoice* m_choice_type;
    mmTextCtrl* m_text_mount;
    wxButton* m_bcategory;
    wxButton* m_cancel_button;
};

#endif
