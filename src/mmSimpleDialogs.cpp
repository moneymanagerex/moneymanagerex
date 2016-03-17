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

//mmSingleChoiceDialog
mmSingleChoiceDialog::mmSingleChoiceDialog()
{
}
mmSingleChoiceDialog::mmSingleChoiceDialog(wxWindow *parent, const wxString& message,
    const wxString& caption, const wxArrayString& choices)
{
    wxSingleChoiceDialog::Create(parent, message, caption, choices);
    fix_translation();
}
mmSingleChoiceDialog::mmSingleChoiceDialog(wxWindow* parent, const wxString& message,
    const wxString& caption, const Model_Account::Data_Set& accounts)
{
    wxArrayString choices;
    for (const auto & item : accounts) choices.Add(item.ACCOUNTNAME);
    wxSingleChoiceDialog::Create(parent, message, caption, choices);
    fix_translation();
}
void mmSingleChoiceDialog::fix_translation()
{
    wxButton* ok = (wxButton*)FindWindow(wxID_OK);
    if (ok) ok->SetLabel(_("&OK "));
    wxButton* ca = (wxButton*)FindWindow(wxID_CANCEL);
    if (ca) ca->SetLabel(wxGetTranslation(g_CancelLabel));
}

//  mmDialogComboBoxAutocomplete
mmDialogComboBoxAutocomplete::mmDialogComboBoxAutocomplete()
{
}
mmDialogComboBoxAutocomplete::mmDialogComboBoxAutocomplete(wxWindow *parent, const wxString& message, const wxString& caption,
    const wxString& defaultText, const wxArrayString& choices)
    : Default(defaultText),
    Choices(choices),
    Message(message)
{
    long style = wxCAPTION | wxRESIZE_BORDER | wxCLOSE_BOX;
    Create(parent, wxID_STATIC, caption, wxDefaultPosition, wxSize(300, 100), style);
}

bool mmDialogComboBoxAutocomplete::Create(wxWindow* parent, wxWindowID id,
    const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
{
    wxDialog::Create(parent, id, caption, pos, size, style);
    const wxSizerFlags flags = wxSizerFlags().Align(wxALIGN_CENTER | wxALIGN_CENTER_VERTICAL).Border(wxLEFT | wxRIGHT, 15);

    wxBoxSizer* Sizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(Sizer);

    Sizer->AddSpacer(10);
    wxStaticText* headerText = new wxStaticText(this, wxID_STATIC, Message);
    Sizer->Add(headerText, flags);
    Sizer->AddSpacer(15);
    cbText_ = new wxComboBox(this, wxID_STATIC, Default, wxDefaultPosition, wxSize(150, -1), Choices);
    cbText_->AutoComplete(Choices);
    Sizer->Add(cbText_, wxSizerFlags(flags).Expand());
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

const wxString mmDialogs::selectLanguageDlg(wxWindow *parent, const wxString &langPath, bool verbose)
{
    wxArrayString lang_files;
    wxFileName fn(langPath, "");
    fn.AppendDir("en");
    size_t cnt = wxDir::GetAllFiles(fn.GetPath(), &lang_files, "*.mo");

    if (!cnt)
    {
        if (verbose)
        {
            wxString s = wxString::Format("Can't find language files (.mo) at \"%s\"", fn.GetPath());

            wxMessageDialog dlg(parent, s, "Error", wxOK | wxICON_ERROR);
            dlg.ShowModal();
        }
        return "english";
    }

    int os_lang_id = wxLocale::GetSystemLanguage();
    const wxString os_language_name = wxLocale::GetLanguageName(os_lang_id);
    int sel = 0;

    for (size_t i = 0; i < cnt; ++i)
    {
        wxFileName fname(lang_files[i]);
        lang_files[i] = fname.GetName().Capitalize();
        //wxLogDebug("%s | %s | %s", lang_files[i], fname.GetFullName(), os_language_name);
        if (lang_files[i] == os_language_name)
            sel = i;
    }
    lang_files.Sort(CaseInsensitiveCmp);

    const wxString lang = wxGetSingleChoice("Please choose language", "Languages", lang_files, sel, parent);
    return lang.Lower();
}

/*
locale.AddCatalog(lang) calls wxLogWarning and returns true for corrupted .mo file,
so I should use locale.IsLoaded(lang) also.
*/
const wxString mmDialogs::mmSelectLanguage(mmGUIApp *app, wxWindow* window, bool forced_show_dlg, bool save_setting)
{
    const wxString langPath = mmex::getPathShared(mmex::LANG_DIR);
    wxLocale &locale = app->getLocale();

    if (wxDir::Exists(langPath))
    {
        locale.AddCatalogLookupPathPrefix(langPath);
    }
    else
    {
        if (forced_show_dlg)
        {
            wxMessageDialog dlg(window
                , wxString::Format(_("Directory of language files does not exist:\n%s"), langPath)
                , _("Error"), wxOK | wxICON_ERROR);
            dlg.ShowModal();
        }

        return wxEmptyString;
    }

    if (!forced_show_dlg)
    {
        const wxString lang = Model_Setting::instance().GetStringSetting(LANGUAGE_PARAMETER, "english");
        if (!lang.empty() && locale.AddCatalog(lang) && locale.IsLoaded(lang))
        {
            mmOptions::instance().language_ = lang;
            return lang;
        }
    }

    wxString lang = selectLanguageDlg(window, langPath, forced_show_dlg);
    if (save_setting && !lang.empty())
    {
        bool ok = locale.AddCatalog(lang) && locale.IsLoaded(lang);
        if (!ok)  lang.clear(); // bad .mo file
        mmOptions::instance().language_ = lang;
        Model_Setting::instance().Set(LANGUAGE_PARAMETER, lang);
    }

    return lang;
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

void mmErrorDialogs::InvalidAccount(wxWindow *object, bool transfer)
{
    const wxString& errorHeader = _("Invalid Account");
    wxString errorMessage;
    if (!transfer)
        errorMessage = _("Please select the account for this transaction.");
    else
        errorMessage = _("Please specify which account the transfer is going to.");

    wxString errorTips = _("Selection can be made by using the dropdown button.");
    errorMessage = errorMessage + "\n\n" + errorTips + "\n";

    wxRichToolTip tip(errorHeader, errorMessage);
    tip.SetIcon(wxICON_WARNING);
    tip.ShowFor(object);
}

void mmErrorDialogs::InvalidPayee(wxWindow *object)
{
    const wxString& errorHeader = _("Invalid Payee");
    const wxString& errorMessage = _("Please type in a new payee,\n"
            "or make a selection using the dropdown button.")
        + "\n";
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
