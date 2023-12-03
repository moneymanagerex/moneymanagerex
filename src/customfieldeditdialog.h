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

#ifndef MM_EX_CUSTOMFIELDEDITDIALOG_H_
#define MM_EX_CUSTOMFIELDEDITDIALOG_H_

#include <wx/dialog.h>
#include "Model_CustomField.h"
class wxChoice;
class wxCheckBox;
class wxSpinCtrl;

class mmCustomFieldEditDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmCustomFieldEditDialog);
    wxDECLARE_EVENT_TABLE();

public:
    mmCustomFieldEditDialog() {};
    mmCustomFieldEditDialog(wxWindow *parent, Model_CustomField::Data* field);

    Model_CustomField::Data* m_field = nullptr;
private:
    bool Create(wxWindow* parent
        , wxWindowID id = wxID_ANY
        , const wxString& caption = _("New/Edit Custom Field")
        , const wxPoint& pos = wxDefaultPosition
        , const wxSize& size = wxDefaultSize
        , long style = wxCAPTION | wxSYSTEM_MENU | wxRESIZE_BORDER | wxCLOSE_BOX);
    void CreateControls();
    void dataToControls();
    void OnOk(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnQuit(wxCloseEvent& event);
    void OnChangeType(wxCommandEvent& event);
    void OnChangeType(wxCommandEvent& event, bool OnDataToControls);

    wxString m_fieldRefType;
    wxTextCtrl* m_itemDescription = nullptr;
    wxChoice* m_itemType = nullptr;
    wxChoice* m_itemReference = nullptr;
    wxChoice* m_itemUDFC = nullptr;
    wxTextCtrl* m_itemTooltip = nullptr;
    wxTextCtrl* m_itemRegEx = nullptr;
    wxCheckBox* m_itemAutocomplete = nullptr;
    wxTextCtrl* m_itemDefault = nullptr;
    wxTextCtrl* m_itemChoices = nullptr;
    wxSpinCtrl* m_itemDigitScale = nullptr;

    enum
    {
        IDC_COMBO_TYPE = wxID_HIGHEST + 1100,
        IDC_NOTES,
        IDC_VALUE,
        IDC_RATE,
    };
};
#endif // MM_EX_CUSTOMFIELDEDITDIALOG_H_