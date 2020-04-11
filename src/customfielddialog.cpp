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

#include "customfielddialog.h"
#include "constants.h"
#include "customfieldlistdialog.h"
#include "images_list.h"
#include "util.h"
#include "model/Model_Currency.h"
#include "defs.h"
#include "paths.h"
#include "validators.h"
#include "mmTextCtrl.h"
#include "mmSimpleDialogs.h"

#include "model/Model_CustomField.h"
#include "model/Model_CustomFieldData.h"

#include <wx/combobox.h>
#include <wx/spinctrl.h>
#include <wx/timectrl.h>
#include <wx/valnum.h>

wxIMPLEMENT_DYNAMIC_CLASS(mmCustomFieldDialog, wxFrame);

wxBEGIN_EVENT_TABLE(mmCustomFieldDialog, wxFrame)
    EVT_BUTTON(wxID_EDIT, mmCustomFieldDialog::OnAddEdit)
    EVT_BUTTON(wxID_CANCEL, mmCustomFieldDialog::OnCancel)
    EVT_CLOSE(mmCustomFieldDialog::OnQuit)
    EVT_SIZE(mmCustomFieldDialog::OnResize)
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
    , m_RefreshRequested(false)
{
    Create(parent, wxID_STATIC, wxPoint(RefPos.x + RefSize.GetWidth(), RefPos.y), wxSize(300, RefSize.GetHeight()));
}

bool mmCustomFieldDialog::Create(wxWindow* parent, wxWindowID id
    , const wxPoint& pos, const wxSize& size)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    long style = wxFRAME_TOOL_WINDOW | wxFRAME_FLOAT_ON_PARENT | wxRESIZE_BORDER | wxCLOSE_BOX | wxCAPTION;

    wxString WindowTitle;
    if (m_RefId > 0)
        WindowTitle = wxString::Format(_("Custom Fields | %s %i"), wxGetTranslation(m_RefType), m_RefId);
    else
        WindowTitle = wxString::Format(_("Custom Fields | New %s"), wxGetTranslation(m_RefType));

    wxFrame::Create(parent, id, WindowTitle, pos, size, style);

    SetEvtHandlerEnabled(false);
    CreateFillControls();
    SetEvtHandlerEnabled(true);

    wxSize dlgSize = Model_Infotable::instance().CustomDialogSize(m_RefType);
    if (dlgSize.GetWidth() > 0)
        this->SetSize(dlgSize);
    else
    {
        GetSizer()->Fit(this);
        GetSizer()->SetSizeHints(this);
        this->SetInitialSize();
    }

    SetIcon(mmex::getProgramIcon());
    return TRUE;
}

void mmCustomFieldDialog::CreateFillControls()
{
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    itemFlexGridSizer3->AddGrowableCol(1);
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
            fieldData->CONTENT = Model_CustomField::getDefault(field.PROPERTIES);
            Model_CustomFieldData::instance().save(fieldData);
        }

        int controlID = ID_CUSTOMFIELD + fieldData->FIELDATADID;
        wxStaticText* Description = new wxStaticText(this, wxID_STATIC, field.DESCRIPTION);
        itemFlexGridSizer3->Add(Description, g_flagsH);

        switch (Model_CustomField::type(field))
        {
        case Model_CustomField::STRING:
            {
                wxTextCtrl* CustomString = new wxTextCtrl(this, controlID, fieldData->CONTENT, wxDefaultPosition, wxDefaultSize);
                CustomString->SetToolTip(Model_CustomField::getTooltip(field.PROPERTIES));
                if (Model_CustomField::getAutocomplete(field.PROPERTIES))
                {
                    wxArrayString values = Model_CustomFieldData::instance().allValue(field.FIELDID);
                    CustomString->AutoComplete(values);
                }
                itemFlexGridSizer3->Add(CustomString, g_flagsExpand);
            }
            break;
        case Model_CustomField::INTEGER:
            {
                int value = (wxAtoi(fieldData->CONTENT)) ? wxAtoi(fieldData->CONTENT) : 0;
                wxSpinCtrl* CustomInteger = new wxSpinCtrl(this, controlID,
                    wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -2147483647, 2147483647, value);
                CustomInteger->SetToolTip(Model_CustomField::getTooltip(field.PROPERTIES));
                itemFlexGridSizer3->Add(CustomInteger, g_flagsExpand);
            }
            break;
        case Model_CustomField::DECIMAL:
            {
                double value;
                if (!fieldData->CONTENT.ToDouble(&value)) value = 0;
                wxSpinCtrlDouble* CustomDecimal = new wxSpinCtrlDouble(this, controlID
                    ,wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -2147483647, 2147483647, value, 0.01);
                CustomDecimal->SetToolTip(Model_CustomField::getTooltip(field.PROPERTIES));
                itemFlexGridSizer3->Add(CustomDecimal, g_flagsExpand);
            }
            break;
        case Model_CustomField::BOOLEAN:
            {
                wxCheckBox* CustomBoolean = new wxCheckBox(this, controlID, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
                CustomBoolean->SetValue((fieldData->CONTENT == "TRUE") ? TRUE : FALSE);
                CustomBoolean->SetToolTip(Model_CustomField::getTooltip(field.PROPERTIES));
                itemFlexGridSizer3->Add(CustomBoolean, g_flagsExpand);
            }
            break;
        case Model_CustomField::DATE:
            {
                wxDate value;
                if (fieldData->CONTENT.CmpNoCase("Now") == 0)
                    value = wxDate::Today();
                else if (!value.ParseDate(fieldData->CONTENT))
                    value.ParseDate("1900-01-01");
                wxDatePickerCtrl* CustomDate = new wxDatePickerCtrl(this, controlID, value, wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN | wxDP_SHOWCENTURY);
                CustomDate->SetToolTip(Model_CustomField::getTooltip(field.PROPERTIES));
                itemFlexGridSizer3->Add(CustomDate, g_flagsExpand);
            }
            break;
        case Model_CustomField::TIME:
            {
                wxDateTime value;
                if (fieldData->CONTENT.CmpNoCase("Now") == 0)
                    value = wxDateTime::Now();
                else if (!value.ParseTime(fieldData->CONTENT))
                    value.ParseTime("00:00:00");
                wxTimePickerCtrl* CustomDate = new wxTimePickerCtrl(this, controlID, value, wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN);
                CustomDate->SetToolTip(Model_CustomField::getTooltip(field.PROPERTIES));
                itemFlexGridSizer3->Add(CustomDate, g_flagsExpand);
            }
            break;
        case Model_CustomField::SINGLECHOICE:
            {
                wxArrayString Choices = Model_CustomField::getChoices(field.PROPERTIES);
                Choices.Sort();

                wxChoice* CustomChoice = new wxChoice(this, controlID, wxDefaultPosition, wxDefaultSize, Choices);
                CustomChoice->SetToolTip(Model_CustomField::getTooltip(field.PROPERTIES));
                itemFlexGridSizer3->Add(CustomChoice, g_flagsExpand);

                if (Choices.size() == 0)
                    CustomChoice->Enable(false);

                CustomChoice->SetStringSelection(fieldData->CONTENT);
            }
            break;
        default: break;
        }
    }

    wxBoxSizer* itemBoxSizer22 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer22, wxSizerFlags(g_flagsV).Centre());

    wxButton* itemButtonClose = new wxButton(this, wxID_CANCEL, _("Close"));
    itemBoxSizer22->Add(itemButtonClose, g_flagsH);
    itemButtonClose->SetToolTip(_("Close custom field window"));

    wxBitmapButton* itemButtonAddEdit = new wxBitmapButton(this, wxID_EDIT
        , mmBitmap(png::OPTIONS), wxDefaultPosition
        , wxSize(itemButtonClose->GetSize().GetY(), itemButtonClose->GetSize().GetY()));
    itemButtonAddEdit->SetToolTip(_("Add, edit and delete custom fields"));
    itemBoxSizer22->Add(itemButtonAddEdit, g_flagsH);
}

void mmCustomFieldDialog::OnAddEdit(wxCommandEvent& /*event*/)
{
    mmCustomFieldListDialog dlg(this, m_RefType);
    dlg.ShowModal();
    m_RefreshRequested = dlg.GetRefreshRequested();
}

void mmCustomFieldDialog::OnCancel(wxCommandEvent& /*event*/)
{
    mmCustomFieldDialog::OnSave(false);
}

void mmCustomFieldDialog::OnQuit(wxCloseEvent& /*event*/)
{
    mmCustomFieldDialog::OnSave(false);
}

void mmCustomFieldDialog::OnResize(wxSizeEvent& event)
{
    Model_Infotable::instance().SetCustomDialogSize(m_RefType, event.GetSize());
    event.Skip();
}

void mmCustomFieldDialog::OnMove(const wxPoint & RefPos, const wxSize & RefSize)
{
    this->Move(wxPoint(RefPos.x + RefSize.GetWidth(), RefPos.y));
}

void mmCustomFieldDialog::OnSave(bool OpenStatus)
{
    if (!IsShown())
        return;

    if (m_RefreshRequested)
    {
        Hide();
        return;
    }

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
                wxTextCtrl* CustomString = static_cast<wxTextCtrl*>(FindWindow(controlID));
                if (CustomString != nullptr)
                    Data = CustomString->GetValue().Trim();
            }
            break;
        case Model_CustomField::INTEGER:
            {
                wxSpinCtrl* CustomInteger = static_cast<wxSpinCtrl*>(FindWindow(controlID));
                if (CustomInteger) Data = wxString::Format("%i", CustomInteger->GetValue());
            }
            break;
        case Model_CustomField::DECIMAL:
            {
                wxSpinCtrlDouble* CustomDecimal = static_cast<wxSpinCtrlDouble*>(FindWindow(controlID));
                if (CustomDecimal) Data = wxString::Format("%f", CustomDecimal->GetValue());
            }
            break;
        case Model_CustomField::BOOLEAN:
            {
                wxCheckBox* CustomBoolean = static_cast<wxCheckBox*>(FindWindow(controlID));
                if (CustomBoolean) Data = (CustomBoolean->GetValue()) ? "TRUE" : "FALSE";
            }
            break;
        case Model_CustomField::DATE:
            {
                wxDatePickerCtrl* CustomDate = static_cast<wxDatePickerCtrl*>(FindWindow(controlID));
                if (CustomDate) Data = CustomDate->GetValue().FormatISODate();
            }
            break;
        case Model_CustomField::TIME:
            {
                wxTimePickerCtrl* CustomTime = static_cast<wxTimePickerCtrl*>(FindWindow(controlID));
                if (CustomTime) Data = CustomTime->GetValue().FormatISOTime();
            }
            break;
        case Model_CustomField::SINGLECHOICE:
            {
                wxChoice* CustomSingleChoice = static_cast<wxChoice*>(FindWindow(controlID));
                if (CustomSingleChoice) Data = CustomSingleChoice->GetStringSelection();
            }
            break;
        default: break;
        }

        fieldData->FIELDID = field.FIELDID;
        fieldData->CONTENT = Data;
        Model_CustomFieldData::instance().save(fieldData);
    }

    Model_Infotable::instance().SetOpenCustomDialog(m_RefType, OpenStatus);
    Hide();
}
