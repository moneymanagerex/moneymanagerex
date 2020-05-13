/*******************************************************
Copyright (C) 2014 Gabriele-V

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

#include "mmSimpleDialogs.h"
#include "constants.h"
#include "mmex.h"
#include "paths.h"
#include "util.h"

#include "model/Model_Account.h"
#include "model/Model_Setting.h"

#include <wx/richtooltip.h>

mmChoiceAmountMask::mmChoiceAmountMask(wxWindow* parent, wxWindowID id)
    : wxChoice(parent, id)
{
    static const std::vector <std::pair<wxString, wxString> > DATA = {
          {".", "."}
        , {",", ","}
        , {wxTRANSLATE("None"), ""}
    };

    for (const auto& entry : DATA) {
        this->Append(wxGetTranslation(entry.first)
            , new wxStringClientData(entry.second));
    }

    Model_Currency::Data* base_currency = Model_Currency::GetBaseCurrency();
    const auto decimal_point = base_currency->DECIMAL_POINT;

    SetDecimalChar(decimal_point);
}

void mmChoiceAmountMask::SetDecimalChar(const wxString& str)
{
    if (str == ".")
        SetSelection(0);
    else if (str == ",")
        SetSelection(1);
    else
        SetSelection(2);
}

//mmSingleChoiceDialog
mmSingleChoiceDialog::mmSingleChoiceDialog()
{
}
mmSingleChoiceDialog::mmSingleChoiceDialog(wxWindow *parent, const wxString& message,
    const wxString& caption, const wxArrayString& choices)
{
    wxSingleChoiceDialog::Create(parent, message, caption, choices);
}
mmSingleChoiceDialog::mmSingleChoiceDialog(wxWindow* parent, const wxString& message,
    const wxString& caption, const Model_Account::Data_Set& accounts)
{
    wxArrayString choices;
    for (const auto & item : accounts) choices.Add(item.ACCOUNTNAME);
    wxSingleChoiceDialog::Create(parent, message, caption, choices);
}

//  mmDialogComboBoxAutocomplete
mmDialogComboBoxAutocomplete::mmDialogComboBoxAutocomplete()
{
}

const wxString mmDialogComboBoxAutocomplete::getText() const
{
    return cbText_->GetValue();
};

mmDialogComboBoxAutocomplete::mmDialogComboBoxAutocomplete(wxWindow *parent, const wxString& message, const wxString& caption,
    const wxString& defaultText, const wxArrayString& choices)
    : Default(defaultText),
    Choices(choices),
    Message(message),
    cbText_(nullptr)
{
    long style = wxCAPTION | wxRESIZE_BORDER | wxCLOSE_BOX;
    Create(parent, wxID_STATIC, caption, wxDefaultPosition, wxDefaultSize, style);
    SetMinSize(wxSize(300, 100));
}

bool mmDialogComboBoxAutocomplete::Create(wxWindow* parent, wxWindowID id,
    const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
{
    wxDialog::Create(parent, id, caption, pos, size, style);
    const wxSizerFlags flags = wxSizerFlags().Align(wxALIGN_CENTER).Border(wxLEFT | wxRIGHT, 15);

    wxBoxSizer* Sizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(Sizer);

    Sizer->AddSpacer(10);
    wxStaticText* headerText = new wxStaticText(this, wxID_STATIC, Message);
    Sizer->Add(headerText, flags);
    Sizer->AddSpacer(15);
    cbText_ = new wxComboBox(this, wxID_STATIC, Default, wxDefaultPosition, wxDefaultSize, Choices);
    cbText_->SetMinSize(wxSize(150, -1));
    cbText_->AutoComplete(Choices);
    Sizer->Add(cbText_, wxSizerFlags().Border(wxLEFT | wxRIGHT, 15).Expand());
    Sizer->AddSpacer(20);
    wxSizer* Button = CreateButtonSizer(wxOK | wxCANCEL);
    Sizer->Add(Button, flags);
    Sizer->AddSpacer(10);

    cbText_->SetFocus();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();
    return true;
}

// mmMultiChoiceDialog --------------------------------------------
mmMultiChoiceDialog::mmMultiChoiceDialog()
{
}
mmMultiChoiceDialog::mmMultiChoiceDialog(wxWindow* parent, const wxString& message,
    const wxString& caption, const Model_Account::Data_Set& accounts)
{
    wxArrayString choices;
    for (const auto & item : accounts) choices.Add(item.ACCOUNTNAME);
    wxMultiChoiceDialog::Create(parent, message, caption, choices);
}

// mmDateYearMonth --------------------------------------------

wxBEGIN_EVENT_TABLE(mmDateYearMonth, wxPanel)
EVT_BUTTON(wxID_ANY, mmDateYearMonth::OnButtonPress)
wxEND_EVENT_TABLE()

mmDateYearMonth::mmDateYearMonth()
{
}

mmDateYearMonth::mmDateYearMonth(wxWindow *parent) :
    m_parent(parent)
    , m_shift(0)
{
    Create(parent, wxID_STATIC);
}

bool mmDateYearMonth::Create(wxWindow* parent, wxWindowID id)
{
    int y =
#ifdef __WXGTK__
        48;
#else
        24;
#endif
    wxWindow::Create(parent, id);

    wxBoxSizer* box_sizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* button12Left = new wxButton(this, wxID_BACKWARD, "<<");
    button12Left->SetMinSize(wxSize(y, -1));
    wxButton* buttonLeft = new wxButton(this, wxID_DOWN, "<");
    buttonLeft->SetMinSize(wxSize(y, -1));
    wxButton* buttonRight = new wxButton(this, wxID_UP, ">");
    buttonRight->SetMinSize(wxSize(y, -1));
    wxButton* button12Right = new wxButton(this, wxID_FORWARD, ">>");
    button12Right->SetMinSize(wxSize(y, -1));

    button12Left->SetToolTip(_("Subtract one year"));
    buttonLeft->SetToolTip(_("Subtract one month"));
    buttonRight->SetToolTip(_("Add one month"));
    button12Right->SetToolTip(_("Add one year"));

    box_sizer->Add(button12Left);
    box_sizer->Add(buttonLeft);
    box_sizer->Add(buttonRight);
    box_sizer->Add(button12Right);

    this->SetSizer(box_sizer);
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();
    Fit();
    return TRUE;
}

void mmDateYearMonth::OnButtonPress(wxCommandEvent& event)
{
    int button_id = event.GetId();
    switch (button_id)
    {
    case wxID_DOWN:
        m_shift--;
        break;
    case wxID_UP:
        m_shift++;
        break;
    case wxID_FORWARD:
        m_shift += 12;
        break;
    case wxID_BACKWARD:
        m_shift -= 12;
        break;
    }
    event.SetInt(m_shift);

    m_parent->GetEventHandler()->AddPendingEvent(event);
}


/* Error Messages --------------------------------------------------------*/

void mmErrorDialogs::MessageError(wxWindow *parent
    , const wxString &message, const wxString &title)
{
    wxMessageDialog msgDlg(parent, message, title, wxOK | wxICON_ERROR);
    msgDlg.ShowModal();
}

void mmErrorDialogs::MessageWarning(wxWindow *parent
    , const wxString &message, const wxString &title)
{
    wxMessageDialog msgDlg(parent, message, title, wxOK | wxICON_WARNING);
    msgDlg.ShowModal();
}

void mmErrorDialogs::MessageInvalid(wxWindow *parent, const wxString &message)
{
    const wxString& msg = wxString::Format(_("Entry %s is invalid"), message);
    MessageError(parent, msg, _("Invalid Entry"));
}

void mmErrorDialogs::InvalidCategory(wxWindow *win, bool simple)
{
    const wxString& msg = simple
        ? _("Please use this button for category selection.")
        : _("Please use this button for category selection\n"
            "or use the 'Split' checkbox for multiple categories.");

    ToolTip4Object(win, msg + "\n", _("Invalid Category"));
}

void mmErrorDialogs::InvalidFile(wxWindow *object, bool open)
{
    const wxString errorHeader = open ? _("Unable to open file.") : _("File name is empty.");
    const wxString errorMessage = _("Please select the file for this operation.");

    ToolTip4Object(object, errorMessage, errorHeader);
}

void mmErrorDialogs::InvalidAccount(wxWindow *object, bool transfer, TOOL_TIP tm)
{
    const wxString& errorHeader = _("Invalid Account");
    wxString errorMessage;
    if (!transfer)
        errorMessage = _("Please select the account for this transaction.");
    else
        errorMessage = _("Please specify which account the transfer is going to.");

    wxString errorTips = _("Selection can be made by using the dropdown button.");
    if (tm == TOOL_TIP::MESSAGE_POPUP_BOX)
    {
        errorTips = _("Activating the button will provide a selection box where the account can be selected.");
    }
    errorMessage = errorMessage + "\n\n" + errorTips + "\n";

    ToolTip4Object(object, errorMessage, errorHeader);
}

void mmErrorDialogs::InvalidPayee(wxWindow *object)
{
    const wxString& errorHeader = _("Invalid Payee");
    const wxString& errorMessage = _("Please type in a new payee,\n"
            "or make a selection using the dropdown button.")
        + "\n";
    ToolTip4Object(object, errorMessage, errorHeader);
}

void mmErrorDialogs::InvalidName(wxTextCtrl *textBox, bool alreadyexist)
{
    const wxString& errorHeader = _("Invalid Name");
    wxString errorMessage;
    if (alreadyexist)
        errorMessage = _("Already exist!");
    else
        errorMessage = _("Please type in a non empty name.");

    ToolTip4Object(textBox, errorMessage, errorHeader);
}

void mmErrorDialogs::InvalidSymbol(wxTextCtrl *textBox, bool alreadyexist)
{
    const wxString& errorHeader = _("Invalid Name");
    wxString errorMessage;
    if (alreadyexist)
        errorMessage = _("Already exist!");
    else
        errorMessage = _("Please type in a non empty symbol.");
 
    ToolTip4Object(textBox, errorMessage, errorHeader);
}

void mmErrorDialogs::ToolTip4Object(wxWindow *object, const wxString &message, const wxString &title, int ico)
{
    wxRichToolTip tip(title, message);
    tip.SetIcon(ico);
    tip.SetBackgroundColour(object->GetParent()->GetBackgroundColour());
    tip.ShowFor(object);
}
