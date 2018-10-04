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
#include "Model_Account.h"
#include "Model_Setting.h"
#include <wx/richtooltip.h>
#include <wx/combobox.h>

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

//mmMultiChoiceDialog
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

//  mmDialogComboBoxAutocomplete
mmDialogComboBoxAutocomplete::mmDialogComboBoxAutocomplete()
{
}
mmDialogComboBoxAutocomplete::mmDialogComboBoxAutocomplete(wxWindow *parent, const wxString& message, const wxString& caption,
    const wxString& defaultText, const wxArrayString& choices)
    : Message(message),
    Default(defaultText),
    Choices(choices)
{
    long style = wxCAPTION | wxRESIZE_BORDER | wxCLOSE_BOX;
    Create(parent, wxID_STATIC, caption, wxDefaultPosition, wxSize(300, 100), style);
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
    cbText_ = new wxComboBox(this, wxID_STATIC, Default, wxDefaultPosition, wxSize(150, -1), Choices);
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

wxString mmDialogComboBoxAutocomplete::getText()
{
    return cbText_->GetValue();
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
    wxRichToolTip tip(_("Invalid Category"), msg + "\n");
    tip.SetIcon(wxICON_WARNING);
    tip.ShowFor(win);
}

void mmErrorDialogs::InvalidFile(wxWindow *object, bool open)
{
    const wxString errorHeader = open ? _("Unable to open file.") : _("File name is empty.");
    const wxString errorMessage = _("Please select the file for this operation.");

    wxRichToolTip tip(errorHeader, errorMessage);
    tip.SetIcon(wxICON_WARNING);
    tip.ShowFor(object);
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

    wxRichToolTip tip(errorHeader, errorMessage);
    tip.SetIcon(wxICON_WARNING);
    tip.ShowFor(object);
}

void mmErrorDialogs::InvalidPayee(wxWindow *object, TOOL_TIP tm)
{
    const wxString& errorHeader = _("Invalid Payee");
    wxString errorMessage = _("Please type in a new payee,\n"
            "or make a selection using the dropdown button.")
        + "\n";

    if (tm == TOOL_TIP::MESSAGE_POPUP_BOX)
    {
        errorMessage = _("Activating the button will provide a selection box where the payee can be selected.");
    }

    wxRichToolTip tip(errorHeader, errorMessage);
    tip.SetIcon(wxICON_WARNING);
    tip.ShowFor(object);
}

void mmErrorDialogs::InvalidName(wxTextCtrl *textBox, bool alreadyexist)
{
    const wxString& errorHeader = _("Invalid Name");
    wxString errorMessage;
    if (alreadyexist)
        errorMessage = _("Already exist!");
    else
        errorMessage = _("Please type in a non empty name.");

    wxRichToolTip tip(errorHeader, errorMessage);
    tip.SetIcon(wxICON_WARNING);
    tip.ShowFor(textBox);
}

void mmErrorDialogs::InvalidSymbol(wxTextCtrl *textBox, bool alreadyexist)
{
    const wxString& errorHeader = _("Invalid Name");
    wxString errorMessage;
    if (alreadyexist)
        errorMessage = _("Already exist!");
    else
        errorMessage = _("Please type in a non empty symbol.");

    wxRichToolTip tip(errorHeader, errorMessage);
    tip.SetIcon(wxICON_WARNING);
    tip.ShowFor(textBox);
}

void mmErrorDialogs::ToolTip4Object(wxWindow *object, const wxString &message, const wxString &title, int ico)
{
    wxRichToolTip tip(title, message);
    tip.SetIcon(ico);
    tip.ShowFor(object);
}

void mmErrorDialogs::InvalidAmount(wxWindow * object)
{
    const wxString& errorHeader = _("Invalid Amount");
    const wxString& errorMessage = _("Please enter a calculated or fixed amount");

    wxRichToolTip tip(errorHeader, errorMessage);
    tip.SetIcon(wxICON_WARNING);
    tip.ShowFor(object);
}

void mmErrorDialogs::InvalidChoice(wxChoice *choice)
{
    const wxString& errorHeader = _("Invalid Choice");
    const wxString& errorMessage = _("Please select a valid choice");

    wxRichToolTip tip(errorHeader, errorMessage);
    tip.SetIcon(wxICON_WARNING);
    tip.ShowFor(choice);
}