/*******************************************************
 Copyright (C) 2016 Gabriele-V

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

#ifndef MM_EX_CUSTOMFIELDDIALOG_H_
#define MM_EX_CUSTOMFIELDDIALOG_H_

#include "model/Model_Currency.h"

class mmTextCtrl;

enum
{
    ID_DIALOG_CURRENCY = wxID_HIGHEST + 400,
    ID_DIALOG_CURRENCY_RATE,
    ID_CUSTOMFIELD,
};

class mmCustomFieldDialog : public wxFrame
{
    wxDECLARE_DYNAMIC_CLASS(mmCustomFieldDialog);
    wxDECLARE_EVENT_TABLE();

public:
    mmCustomFieldDialog();
    ~mmCustomFieldDialog();
    mmCustomFieldDialog(wxWindow* parent, const wxPoint& RefPos, const wxSize& RefSize, const wxString& RefType, int RefId);

private:
    bool Create(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size);
    void CreateFillControls();

    void OnAddEdit(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnQuit(wxCloseEvent& event);
    void OnResize(wxSizeEvent& event);

    wxString m_RefType;
    int m_RefId;
    bool m_RefreshRequested;

public:
    void OnMove(const wxPoint& RefPos, const wxSize& RefSize);
    void OnSave(bool OpenStatus);
};

#endif
