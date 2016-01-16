/*******************************************************
 Copyright (C) 2015 Gabriele-V

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

#include "customfielddialog.h"
#include "constants.h"
#include "customfieldlistdialog.h"
#include "images_list.h"
#include "util.h"
#include "model/Model_Currency.h"
#include "defs.h"
#include "paths.h"
#include "validators.h"
#include "mmtextctrl.h"
#include "mmSimpleDialogs.h"

#include "model/Model_CustomField.h"
#include "model/Model_CustomFieldData.h"

#include <wx/combobox.h>
#include <wx/spinctrl.h>
#include <wx/timectrl.h>
#include <wx/valnum.h>

wxIMPLEMENT_DYNAMIC_CLASS(mmCustomFieldDialog, wxDialog);

wxBEGIN_EVENT_TABLE(mmCustomFieldDialog, wxDialog)
    EVT_BUTTON(wxID_EDIT, mmCustomFieldDialog::OnAddEdit)
    EVT_BUTTON(wxID_CANCEL, mmCustomFieldDialog::OnClose)
wxEND_EVENT_TABLE()

mmCustomFieldDialog::mmCustomFieldDialog()
{
}

mmCustomFieldDialog::~mmCustomFieldDialog()
{
}

mmCustomFieldDialog::mmCustomFieldDialog(wxWindow* parent, const wxPoint& RefPos, const wxSize& RefSize, const wxString& RefType, int RefId) :
    m_RefType(RefType)
    , m_RefId(RefId)
{
    Create(parent, wxID_STATIC, wxPoint(RefPos.x + RefSize.GetWidth(), RefPos.y), wxSize(300, RefSize.GetHeight()));
}

bool mmCustomFieldDialog::Create(wxWindow* parent, wxWindowID id
    , const wxPoint& pos, const wxSize& size)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    long style = wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER;

    wxString WindowTitle;
    if (m_RefId > 0)
        WindowTitle = wxString::Format(_("Custom Fields | %s %i"), m_RefType, m_RefId);
    else
        WindowTitle = wxString::Format(_("Custom Fields | New %s"), m_RefType);

    wxDialog::Create(parent, id, WindowTitle, pos, size, style);

    CreateFillControls();
    SetEvtHandlerEnabled(false);
    SetEvtHandlerEnabled(true);

    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    this->SetInitialSize();
    SetIcon(mmex::getProgramIcon());

    return TRUE;
}

void mmCustomFieldDialog::CreateFillControls()
{
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    //itemFlexGridSizer3->AddGrowableCol(1);
    itemBoxSizer2->Add(itemFlexGridSizer3, g_flagsExpand);

    Model_CustomField::Data_Set fields = Model_CustomField::instance().find(Model_CustomField::DB_Table_CUSTOMFIELD_V1::REFTYPE(m_RefType));
    std::sort(fields.begin(), fields.end(), SorterByDESCRIPTION());

    for (const auto &field : fields)
    {
        Model_CustomFieldData::Data* fieldData = Model_CustomFieldData::instance().get(field.FIELDID, m_RefId);
        if (!fieldData)
        {
            fieldData = Model_CustomFieldData::instance().create();
            fieldData->FIELDID = field.FIELDID;
            fieldData->REFID = m_RefId;
            fieldData->CONTENT = wxEmptyString;
            Model_CustomFieldData::instance().save(fieldData);
        }

        int controlID = ID_CUSTOMFIELD + fieldData->FIELDATADID;
        itemFlexGridSizer3->Add(new wxStaticText(this, wxID_STATIC, field.DESCRIPTION), g_flags);

        switch (Model_CustomField::type(field))
        {
        case Model_CustomField::STRING:
            {
                wxTextCtrl* CustomString = new wxTextCtrl(this, controlID, fieldData->CONTENT, wxDefaultPosition, wxDefaultSize);
                itemFlexGridSizer3->Add(CustomString, g_flagsExpand);
            }
            break;
        case Model_CustomField::INTEGER:
            {
                int value = (wxAtoi(fieldData->CONTENT)) ? wxAtoi(fieldData->CONTENT) : 0;
                wxSpinCtrl* CustomInteger = new wxSpinCtrl(this, controlID,
                    wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -9999999, 9999999, value);
                itemFlexGridSizer3->Add(CustomInteger, g_flagsExpand);
            }
            break;
        case Model_CustomField::DECIMAL:
            {
                double value;
                if (!fieldData->CONTENT.ToDouble(&value)) value = 0;
                wxSpinCtrlDouble* CustomDecimal = new wxSpinCtrlDouble(this, controlID
                    ,wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -9999999, 9999999, value, 0.1);
                itemFlexGridSizer3->Add(CustomDecimal, g_flagsExpand);
            }
            break;
        case Model_CustomField::BOOLEAN:
            {
                wxCheckBox* CustomBoolean = new wxCheckBox(this, controlID, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
                CustomBoolean->SetValue((fieldData->CONTENT == "TRUE") ? TRUE : FALSE);
                itemFlexGridSizer3->Add(CustomBoolean, g_flagsExpand);
            }
            break;
        case Model_CustomField::DATE:
            {
                wxDate value;
                if (!value.ParseDate(fieldData->CONTENT)) value = wxDate::Today();
                wxDatePickerCtrl* CustomDate = new wxDatePickerCtrl(this, controlID, value
                    , wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN | wxDP_SHOWCENTURY);
                itemFlexGridSizer3->Add(CustomDate, g_flagsExpand);
            }
            break;
        case Model_CustomField::TIME:
            {
                wxDateTime value;
                if (!value.ParseTime(fieldData->CONTENT)) value = wxDateTime::Now();
                wxTimePickerCtrl* CustomDate = new wxTimePickerCtrl(this, controlID, value
                    , wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN);
                itemFlexGridSizer3->Add(CustomDate, g_flagsExpand);
            }
            break;
        default: break;
        }
    }

    wxBoxSizer* itemBoxSizer22 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer22, wxSizerFlags(g_flags).Centre());

    wxButton* itemButtonClose = new wxButton(this, wxID_CANCEL, _(" Close "));
    itemBoxSizer22->Add(itemButtonClose, g_flags);
    itemButtonClose->SetToolTip(_("Close custom field window"));

    wxBitmapButton* itemButtonAddEdit = new wxBitmapButton(this, wxID_EDIT
        , mmBitmap(png::OPTIONS), wxDefaultPosition
        , wxSize(itemButtonClose->GetSize().GetY(), itemButtonClose->GetSize().GetY()));
    itemButtonAddEdit->SetToolTip(_("Add, edit and delete custom fields"));
    itemBoxSizer22->Add(itemButtonAddEdit, g_flags);
}

void mmCustomFieldDialog::OnAddEdit(wxCommandEvent& /*event*/)
{
    mmCustomFieldListDialog dlg(this, m_RefType);
    dlg.ShowModal();
}

void mmCustomFieldDialog::OnClose(wxCommandEvent& /*event*/)
{
    mmCustomFieldDialog::OnClose(false);
}

 void mmCustomFieldDialog::OnClose(const bool OpenStatus)
{
    Model_CustomField::Data_Set fields = Model_CustomField::instance().find(Model_CustomField::DB_Table_CUSTOMFIELD_V1::REFTYPE(m_RefType));

    for (const auto &field : fields)
    {
        Model_CustomFieldData::Data* fieldData = Model_CustomFieldData::instance().get(field.FIELDID, m_RefId);
        if (!fieldData)
            fieldData = Model_CustomFieldData::instance().create();

        int controlID = ID_CUSTOMFIELD + fieldData->FIELDATADID;
        wxString Data = wxEmptyString;

        switch (Model_CustomField::type(field))
        {
        case Model_CustomField::STRING:
            {
                wxTextCtrl* CustomString = (wxTextCtrl*)FindWindow(controlID);
                if (CustomString) Data = CustomString->GetValue().Trim();
            }
            break;
        case Model_CustomField::INTEGER:
            {
                wxSpinCtrl* CustomInteger = (wxSpinCtrl*)FindWindow(controlID);
                if (CustomInteger) Data = wxString::Format(_("%i"), CustomInteger->GetValue());
            }
            break;
        case Model_CustomField::DECIMAL:
            {
                wxSpinCtrlDouble* CustomDecimal = (wxSpinCtrlDouble*)FindWindow(controlID);
                if (CustomDecimal) Data = wxString::Format(_("%f"), CustomDecimal->GetValue());
            }
            break;
        case Model_CustomField::BOOLEAN:
            {
                wxCheckBox* CustomBoolean = (wxCheckBox*)FindWindow(controlID);
                if (CustomBoolean) Data = (CustomBoolean->GetValue()) ? "TRUE" : "FALSE";
            }
            break;
        case Model_CustomField::DATE:
            {
                wxDatePickerCtrl* CustomDate = (wxDatePickerCtrl*)FindWindow(controlID);
                if (CustomDate) Data = CustomDate->GetValue().FormatISODate();
            }
            break;
        case Model_CustomField::TIME:
            {
                wxTimePickerCtrl* CustomTime = (wxTimePickerCtrl*)FindWindow(controlID);
                if (CustomTime) Data = CustomTime->GetValue().FormatISOTime();
            }
            break;
        default: break;
        }

        fieldData->FIELDID = field.FIELDID;
        fieldData->CONTENT = Data;
        Model_CustomFieldData::instance().save(fieldData);
    }

    Model_Infotable::instance().SetOpenCustomDialog(m_RefType, OpenStatus);
    EndModal(wxID_OK);
}
