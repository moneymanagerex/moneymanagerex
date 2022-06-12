/*******************************************************
Copyright (C) 2014 Gabriele-V
Copyright (C) 2015, 2016, 2020, 2022 Nikolay Akimov
Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)

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
#include "images_list.h"
#include "mmex.h"
#include "paths.h"
#include "util.h"

#include "model/Model_Account.h"
#include "model/Model_Setting.h"

#include <wx/richtooltip.h>

wxBEGIN_EVENT_TABLE(mmComboBox, wxComboBox)
EVT_TEXT(wxID_ANY, mmComboBox::OnTextUpdated)
wxEND_EVENT_TABLE()

mmComboBox::mmComboBox(wxWindow* parent, wxWindowID id, wxSize size)
    : wxComboBox(parent, id, "", wxDefaultPosition, size)
{
    Bind(wxEVT_CHAR_HOOK, &mmComboBox::OnKeyPressed, this);
}

void mmComboBox::Create()
{
    unsigned int i = 0;
    wxArrayString auto_complite;
    for (const auto& item : all_elements_)
    {
        auto_complite.Add(item.first);
        this->Insert(item.first, i++);
    }
    this->AutoComplete(auto_complite);
    if (auto_complite.GetCount() == 1)
        Select(0);
}

void mmComboBox::mmSetId(int id)
{
    for (const auto& item : all_elements_)
    {
        if (item.second == id) {
            ChangeValue(item.first);
            element_id_ = id;
            break;
        }
    }
}

void mmComboBox::OnTextUpdated(wxCommandEvent& event)
{
    this->SetEvtHandlerEnabled(false);
    element_id_ = -1;
    const auto& typedText = event.GetString();
#if defined (__WXMAC__)
    // Filtering the combobox as the user types because on Mac autocomplete function doesn't work
    // PLEASE DO NOT REMOVE!!
    if (this->GetSelection() == -1) // make sure nothing is selected (ex. user presses down arrow)
    {
        this->Clear();

        for (auto& entry : all_elements_)
        {
            if (entry.first.Lower().Matches(typedText.Lower().Append("*")))
                this->Append(entry.first);
        }

        this->ChangeValue(typedText);
        this->SetInsertionPointEnd();
        this->Popup();
    }
#endif

    for (const auto& item : all_elements_) {
        if (item.first.CmpNoCase(typedText) == 0) {
            element_id_ = all_elements_.at(item.first);
            ChangeValue(item.first);
            break;
        }
    }
    this->SetEvtHandlerEnabled(true);
    event.Skip();
}

void mmComboBox::OnKeyPressed(wxKeyEvent& event)
{
    auto text = GetValue();
    if (event.GetKeyCode() == WXK_RETURN)
    {
        for (const auto& item : all_elements_)
        {
            if (item.first.CmpNoCase(text) == 0) {
                SetValue(item.first);
                Dismiss();
                break;
            }
        }
    }
    event.Skip();
}

const wxString mmComboBox::mmGetPattern() const
{
    auto value = GetValue();
    if (all_elements_.find(value) != all_elements_.end())
    {
        wxString buffer;
        for (const wxString& c : value) {
            if (wxString(R"(.^$*+?()[{\|)").Contains(c)) {
                buffer += R"(\)";
            }
            buffer += c;
        }
        return buffer;
    }
    return value;
}

bool mmComboBox::mmIsValid() const
{
    int i = element_id_;
    auto it = std::find_if(all_elements_.begin(), all_elements_.end(),
        [&i](const std::pair<wxString, int>& element) { return element.second == i; });

    return it != all_elements_.end();
}

mmComboBoxAccount::mmComboBoxAccount(wxWindow* parent, wxWindowID id, wxSize size)
    : mmComboBox(parent, id, size)
{
    all_elements_ = Model_Account::instance().all_accounts();
    Create();
}

/* --------------------------------------------------------- */

mmComboBoxPayee::mmComboBoxPayee(wxWindow* parent, wxWindowID id, wxSize size)
    : mmComboBox(parent, id, size)
{
    all_elements_ = Model_Payee::instance().all_payees();
    Create();
}

/* --------------------------------------------------------- */

mmComboBoxCurrency::mmComboBoxCurrency(wxWindow* parent, wxWindowID id, wxSize size)
    : mmComboBox(parent, id, size)
{
    all_elements_ = Model_Currency::instance().all_currency();
    Create();
}

/* --------------------------------------------------------- */

mmComboBoxCategory::mmComboBoxCategory(wxWindow* parent, wxWindowID id, wxSize size)
    : mmComboBox(parent, id, size)
{
    int i = 0;
    all_categories_ = Model_Category::instance().all_categories();
    for (const auto& item : all_categories_)
    {
        all_elements_[item.first] = i++;
    }
    Create();
}

int mmComboBoxCategory::mmGetCategoryId() const
{
    int i = mmGetId();
    auto it = std::find_if(all_elements_.begin(), all_elements_.end(),
        [&i](const std::pair<wxString, int>& element) { return element.second == i; });
    if (it != all_elements_.end())
        return all_categories_.at(it->first).first;
    return -1;
}

int mmComboBoxCategory::mmGetSubcategoryId() const
{
    int i = mmGetId();
    auto it = std::find_if(all_elements_.begin(), all_elements_.end(),
        [&i](const std::pair<wxString, int>& element) { return element.second == i; });
    if (it != all_elements_.end())
        return all_categories_.at(it->first).second;
    return -1;
}

/*/////////////////////////////////////////////////////////////*/

wxBEGIN_EVENT_TABLE(mmColorButton, wxButton)
EVT_MENU(wxID_ANY, mmColorButton::OnMenuSelected)
EVT_BUTTON(wxID_ANY, mmColorButton::OnColourButton)
wxEND_EVENT_TABLE()
mmColorButton::mmColorButton(wxWindow* parent, wxWindowID id, wxSize size)
    :wxButton(parent, id, "", wxDefaultPosition, size)
    , m_color_value(-1)
{
}

void mmColorButton::OnMenuSelected(wxCommandEvent& event)
{
    m_color_value = event.GetId() - wxID_HIGHEST;
    SetBackgroundColour(getUDColour(m_color_value));
    if (GetSize().GetX() > 40)
    {
        if (m_color_value <= 0) {
            SetLabel(wxString::Format(_("Clear color")));
        }
        else {
            SetLabel(wxString::Format(_("Color #%i"), m_color_value));
        }
    }
    event.Skip();
}

void mmColorButton::OnColourButton(wxCommandEvent& event)
{
    wxMenu mainMenu;
    wxMenuItem* menuItem = new wxMenuItem(&mainMenu, wxID_HIGHEST, wxString::Format(_("Clear color"), 0));
    mainMenu.Append(menuItem);

    for (int i = 1; i <= 7; ++i)
    {
        menuItem = new wxMenuItem(&mainMenu, wxID_HIGHEST + i, wxString::Format(_("Color #%i"), i));
#ifdef __WXMSW__
        menuItem->SetBackgroundColour(getUDColour(i)); //only available for the wxMSW port.
#endif
        wxBitmap bitmap(mmBitmap(png::EMPTY, mmBitmapButtonSize).GetSize());
        wxMemoryDC memoryDC(bitmap);
        wxRect rect(memoryDC.GetSize());

        memoryDC.SetBackground(wxBrush(getUDColour(i)));
        memoryDC.Clear();
        memoryDC.DrawBitmap(mmBitmap(png::EMPTY, mmBitmapButtonSize), 0, 0, true);
        memoryDC.SelectObject(wxNullBitmap);
        menuItem->SetBitmap(bitmap);

        mainMenu.Append(menuItem);
    }

    PopupMenu(&mainMenu);
    event.Skip();
}

int mmColorButton::GetColorId() const
{
    return m_color_value;
}

void mmColorButton::SetBackgroundColor(int color_id)
{
    SetBackgroundColour(getUDColour(color_id));
    m_color_value = color_id;
}

/*/////////////////////////////////////////////////////////////*/

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

/*/////////////////////////////////////////////////////////////*/

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

/* --------------------------------------------------------- */

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

/* --------------------------------------------------------- */

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

/*/////////////////////////////////////////////////////////////*/

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

    mmToolTip(button12Left, _("Subtract one year"));
    mmToolTip(buttonLeft, _("Subtract one month"));
    mmToolTip(buttonRight, _("Add one month"));
    mmToolTip(button12Right, _("Add one year"));

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

/*////////////////// Error Messages //////////////////////////////////////*/

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
    const wxString& msg = wxString::Format(_("Entry %s is invalid"), message, wxICON_ERROR);
    MessageError(parent, msg, _("Invalid Entry"));
}

void mmErrorDialogs::InvalidCategory(wxWindow *win, bool simple)
{
    const wxString& msg = simple
        ? _("Please use this button for category selection.")
        : _("Please use this button for category selection\n"
            "or use the 'Split' checkbox for multiple categories.");

    ToolTip4Object(win, msg + "\n", _("Invalid Category"), wxICON_ERROR);
}

void mmErrorDialogs::InvalidFile(wxWindow *object, bool open)
{
    const wxString errorHeader = open ? _("Unable to open file.") : _("File name is empty.");
    const wxString errorMessage = _("Please select the file for this operation.");

    ToolTip4Object(object, errorMessage, errorHeader, wxICON_ERROR);
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

    ToolTip4Object(object, errorMessage, errorHeader, wxICON_ERROR);
}

void mmErrorDialogs::InvalidPayee(wxWindow *object)
{
    const wxString& errorHeader = _("Invalid Payee");
    const wxString& errorMessage = _("Please type in a new payee,\n"
            "or make a selection using the dropdown button.")
        + "\n";
    ToolTip4Object(object, errorMessage, errorHeader, wxICON_ERROR);
}

void mmErrorDialogs::InvalidName(wxTextCtrl *textBox, bool alreadyexist)
{
    const wxString& errorHeader = _("Invalid Name");
    wxString errorMessage;
    if (alreadyexist)
        errorMessage = _("Already exist!");
    else
        errorMessage = _("Please type in a non empty name.");

    ToolTip4Object(textBox, errorMessage, errorHeader, wxICON_ERROR);
}

void mmErrorDialogs::InvalidSymbol(wxTextCtrl *textBox, bool alreadyexist)
{
    const wxString& errorHeader = _("Invalid Name");
    wxString errorMessage;
    if (alreadyexist)
        errorMessage = _("Already exist!");
    else
        errorMessage = _("Please type in a non empty symbol.");
 
    ToolTip4Object(textBox, errorMessage, errorHeader, wxICON_ERROR);
}

void mmErrorDialogs::ToolTip4Object(wxWindow *object, const wxString &message, const wxString &title, int ico)
{
    wxRichToolTip tip(title, message);
    tip.SetIcon(ico);
    tip.SetBackgroundColour(object->GetParent()->GetBackgroundColour());
    tip.ShowFor(object);
}
