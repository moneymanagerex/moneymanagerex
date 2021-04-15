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
    mmCustomFieldEditDialog(wxWindow *parent, Model_CustomField::Data* field, const wxString& fieldRefType);

    Model_CustomField::Data* m_field;
private:
    bool Create(wxWindow* parent
        , wxWindowID id
        , const wxString& caption
        , const wxPoint& pos
        , const wxSize& size
        , long style);
    void CreateControls();
    void dataToControls();
    void OnOk(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnQuit(wxCloseEvent& event);
    void OnChangeType(wxCommandEvent& event);
    void OnChangeType(wxCommandEvent& event, bool OnDataToControls);

    wxString m_fieldRefType;
    wxTextCtrl* m_itemDescription;
    wxChoice*  m_itemType;
    wxChoice*  m_itemUDFC;
    wxTextCtrl* m_itemTooltip;
    wxTextCtrl* m_itemRegEx;
    wxCheckBox* m_itemAutocomplete;
    wxTextCtrl* m_itemDefault;
    wxTextCtrl* m_itemChoices;
    wxSpinCtrl* m_itemDigitScale;

    enum
    {
        IDC_COMBO_TYPE = wxID_HIGHEST + 1100,
        IDC_NOTES,
        IDC_VALUE,
        IDC_RATE,
    };
};
#endif // MM_EX_CUSTOMFIELDEDITDIALOG_H_