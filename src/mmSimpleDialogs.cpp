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
#include "tagdialog.h"
#include "util.h"

#include "model/Model_Account.h"
#include "model/Model_Setting.h"

#include <wx/richtooltip.h>

//------- Pop-up calendar, currently only used for MacOS only
// See: https://github.com/moneymanagerex/moneymanagerex/issues/3139

#include "wx/dcbuffer.h"
#include "wx/popupwin.h"
#include "wx/spinctrl.h"

//----------------------------------------------------------------------------
// mmCalendarPopup
//----------------------------------------------------------------------------
class mmCalendarPopup: public wxPopupTransientWindow
{
public:
    mmCalendarPopup(wxWindow *parent, mmDatePickerCtrl* datePicker);
    virtual ~mmCalendarPopup();

private:
    mmDatePickerCtrl* m_datePicker;
    void OnDateSelected(wxCalendarEvent& event);
    void OnEndSelection(wxCalendarEvent& event);
    wxDECLARE_ABSTRACT_CLASS(mmCalendarPopup);
};

wxIMPLEMENT_CLASS(mmCalendarPopup, wxPopupTransientWindow);

mmCalendarPopup::mmCalendarPopup( wxWindow *parent, mmDatePickerCtrl* datePicker)
    : wxPopupTransientWindow(parent,
        wxBORDER_NONE)
    , m_datePicker(datePicker)
{
    wxWindow* panel = new wxWindow(this, wxID_ANY);

    wxCalendarCtrl* m_calendarCtrl = new wxCalendarCtrl(panel, wxID_ANY, datePicker->GetValue()
        , wxDefaultPosition, wxDefaultSize
        , wxCAL_SEQUENTIAL_MONTH_SELECTION | wxCAL_SHOW_HOLIDAYS | wxCAL_SHOW_SURROUNDING_WEEKS);
    m_calendarCtrl->Bind(wxEVT_CALENDAR_SEL_CHANGED, &mmCalendarPopup::OnDateSelected, this);
    m_calendarCtrl->Bind(wxEVT_CALENDAR_DOUBLECLICKED, &mmCalendarPopup::OnEndSelection, this);

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(m_calendarCtrl, 0, wxALL, 5);
    panel->SetSizer(sizer);

    sizer->Fit(panel);
    SetClientSize(panel->GetSize());
}

mmCalendarPopup::~mmCalendarPopup()
{

}

void mmCalendarPopup::OnDateSelected(wxCalendarEvent& event)
{
    m_datePicker->SetValue(event.GetDate());
    wxDateEvent evt(m_datePicker, m_datePicker->GetValue(), wxEVT_DATE_CHANGED);
    m_datePicker->GetEventHandler()->AddPendingEvent(evt);
}

void mmCalendarPopup::OnEndSelection(wxCalendarEvent& event)
{
    m_datePicker->SetValue(event.GetDate());
    this->Dismiss();
    wxDateEvent evt(m_datePicker, m_datePicker->GetValue(), wxEVT_DATE_CHANGED);
    m_datePicker->GetEventHandler()->AddPendingEvent(evt);
}

//------------

wxBEGIN_EVENT_TABLE(mmComboBox, wxComboBox)
EVT_SET_FOCUS(mmComboBox::OnSetFocus)
EVT_COMBOBOX_DROPDOWN(wxID_ANY, mmComboBox::OnDropDown)
EVT_TEXT(wxID_ANY, mmComboBox::OnTextUpdated)
wxEND_EVENT_TABLE()

mmComboBox::mmComboBox(wxWindow* parent, wxWindowID id, wxSize size)
    : wxComboBox(parent, id, "", wxDefaultPosition, size)
    , is_initialized_(false)
{
    Bind(wxEVT_CHAR_HOOK, &mmComboBox::OnKeyPressed, this);
    Bind(wxEVT_CHAR, &mmComboBox::OnKeyPressed, this);
}

void mmComboBox::OnDropDown(wxCommandEvent& event)
{
    wxFocusEvent evt;
    OnSetFocus(evt);
}

void mmComboBox::OnSetFocus(wxFocusEvent& event)
{
    if (!is_initialized_)
    {
       wxArrayString auto_complete;
       for (const auto& item : all_elements_) {
           auto_complete.Add(item.first);
       }
       auto_complete.Sort(CaseInsensitiveLocaleCmp);

       this->AutoComplete(auto_complete);
       if (!auto_complete.empty()) {
           wxString selection = GetValue();
           Set(auto_complete);
           if (!selection.IsEmpty()) SetStringSelection(selection);
       }
       if (auto_complete.GetCount() == 1) {
           Select(0);
       }
        is_initialized_ = true;
    }
    event.Skip();
}

void mmComboBox::mmDoReInitialize()
{
    this->Clear();
    init();
    is_initialized_ = false;
    wxFocusEvent evt(wxEVT_SET_FOCUS);
    OnSetFocus(evt);
}

void mmComboBox::mmSetId(int id)
{
    auto result = std::find_if(all_elements_.begin(), all_elements_.end(),
        [id](const std::pair<wxString, int>& mo) {return mo.second == id; });

    if (result != all_elements_.end())
        ChangeValue(result->first);
}

int mmComboBox::mmGetId() const
{
    auto text = GetValue();
    if (all_elements_.count(text) == 1)
        return all_elements_.at(text);
    else
        return -1;
}

void mmComboBox::OnTextUpdated(wxCommandEvent& event)
{
    this->SetEvtHandlerEnabled(false);
    const auto& typedText = event.GetString();
#if defined (__WXMAC__)
    // Filtering the combobox as the user types because on Mac autocomplete function doesn't work
    // PLEASE DO NOT REMOVE!!
    if (typedText.IsEmpty() || (this->GetSelection() == -1))
    {
        this->Clear();

        for (auto& entry : all_elements_)
        {
            if (entry.first.Lower().Matches(typedText.Lower().Prepend("*").Append("*")))
                this->Append(entry.first);
        }

        this->ChangeValue(typedText);
        this->SetInsertionPointEnd();
        if (!typedText.IsEmpty())
            this->Popup();
        else
            this->Dismiss();
    }
#endif
    for (const auto& item : all_elements_) {
        if ((item.first.CmpNoCase(typedText) == 0) && (item.first.Cmp(typedText) != 0)) {
            ChangeValue(item.first);
            SetInsertionPointEnd();
            wxCommandEvent evt(wxEVT_COMBOBOX, this->GetId());
            AddPendingEvent(evt);
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
        event.Skip();
    }
    else if (event.GetId() == mmID_CATEGORY && event.GetUnicodeKey() == ':')
    {
        this->SetEvtHandlerEnabled(false);
        ChangeValue(text.Trim().Append(Model_Infotable::instance().GetStringInfo("CATEG_DELIMITER", ":")));
        SetInsertionPointEnd();
        this->SetEvtHandlerEnabled(true);
    }
    else {
        event.Skip();
    }
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
    return (all_elements_.count(GetValue()) == 1);
}

/* --------------------------------------------------------- */

void mmComboBoxAccount::init()
{
    all_elements_ = Model_Account::instance().all_accounts(excludeClosed_);
    if (accountID_ > -1)
        all_elements_[Model_Account::get_account_name(accountID_)] = accountID_;
}

// accountID = always include this account even if it would have been excluded as closed
// excludeClosed = set to true if closed accounts should be excluded
mmComboBoxAccount::mmComboBoxAccount(wxWindow* parent, wxWindowID id
    , wxSize size, int accountID, bool excludeClosed)
    : mmComboBox(parent, id, size)
    , excludeClosed_(excludeClosed)
    , accountID_(accountID)
{
    init();
    wxArrayString choices;
    for (const auto& item : all_elements_) {
        choices.Add(item.first);
    }
    parent->SetEvtHandlerEnabled(false);
    Set(choices);
    parent->SetEvtHandlerEnabled(true);
}

/* --------------------------------------------------------- */

void mmComboBoxPayee::init()
{
    all_elements_ = Model_Payee::instance().all_payees(excludeHidden_);
    if (payeeID_ > -1)
        all_elements_[Model_Payee::get_payee_name(payeeID_)] = payeeID_;
}

// payeeID = always include this payee even if it would have been excluded as inactive
// excludeHidden = set to true if inactive payees should be excluded
mmComboBoxPayee::mmComboBoxPayee(wxWindow* parent, wxWindowID id
                    , wxSize size, int payeeID, bool excludeHidden)
    : mmComboBox(parent, id, size)
    , excludeHidden_(excludeHidden)
    , payeeID_(payeeID)
{
    init();
    wxArrayString choices;
    for (const auto& item : all_elements_) {
        choices.Add(item.first);
    }
    parent->SetEvtHandlerEnabled(false);
    Set(choices);
    parent->SetEvtHandlerEnabled(true);
}

void mmComboBoxUsedPayee::init()
{
    all_elements_ = Model_Payee::instance().used_payee();
}

mmComboBoxUsedPayee::mmComboBoxUsedPayee(wxWindow* parent, wxWindowID id, wxSize size)
    : mmComboBox(parent, id, size)
{
    init();
    wxArrayString choices;
    for (const auto& item : all_elements_) {
        choices.Add(item.first);
    }
    parent->SetEvtHandlerEnabled(false);
    Set(choices);
    parent->SetEvtHandlerEnabled(true);
}

/* --------------------------------------------------------- */

void mmComboBoxCurrency::init()
{
    all_elements_ = Model_Currency::instance().all_currency();
}

mmComboBoxCurrency::mmComboBoxCurrency(wxWindow* parent, wxWindowID id, wxSize size)
    : mmComboBox(parent, id, size)
{
    init();
    wxArrayString choices;
    for (const auto& item : all_elements_) {
        choices.Add(item.first);
    }
    parent->SetEvtHandlerEnabled(false);
    Set(choices);
    parent->SetEvtHandlerEnabled(true);
}

/* --------------------------------------------------------- */
void mmComboBoxCategory::init()
{
    int i = 0;
    all_elements_.clear();
    all_categories_ = Model_Category::instance().all_categories(excludeHidden_);
    if (catID_ > -1)
        all_categories_.insert(std::make_pair(Model_Category::full_name(catID_)
                                    , catID_));
    for (const auto& item : all_categories_)
    {
        all_elements_[item.first] = i++;
    }
}

// catID/subCatID = always include this category even if it would have been excluded as inactive
// excludeHidden = set to true if hidden categories should be excluded
mmComboBoxCategory::mmComboBoxCategory(wxWindow* parent, wxWindowID id
                    , wxSize size, int catID, bool excludeHidden)
    : mmComboBox(parent, id, size)
    , excludeHidden_(excludeHidden)
    , catID_(catID)
{
    init();
    wxArrayString choices;
    for (const auto& item : all_elements_) {
        choices.Add(item.first);
    }
    parent->SetEvtHandlerEnabled(false);
    Set(choices);
    parent->SetEvtHandlerEnabled(true);
}

int mmComboBoxCategory::mmGetCategoryId() const
{
    auto text = GetValue();
    if (all_categories_.count(text) == 1)
        return all_categories_.at(text);
    else
        return -1;
}

/* --------------------------------------------------------- */

mmComboBoxCustom::mmComboBoxCustom(wxWindow* parent, wxArrayString& a, wxWindowID id, wxSize size)
    : mmComboBox(parent, id, size)
{
    int i = 0;
    for (const auto& item : a)
    {
        all_elements_[item] = i++;
    }
}

/* --------------------------------------------------------- */

wxBEGIN_EVENT_TABLE(mmDatePickerCtrl, wxDatePickerCtrl)
EVT_DATE_CHANGED(wxID_ANY, mmDatePickerCtrl::OnDateChanged)
EVT_SPIN(wxID_ANY, mmDatePickerCtrl::OnDateSpin)
wxEND_EVENT_TABLE()

mmDatePickerCtrl::mmDatePickerCtrl(wxWindow* parent, wxWindowID id, wxDateTime dt, wxPoint pos, wxSize size, long style)
    : wxDatePickerCtrl(parent, id, dt, pos, size, style)
    , parent_(parent)
    , itemStaticTextWeek_(nullptr)
    , spinButton_(nullptr)
{
    // The standard date control for MacOS does not have a date picker so make one available when right-click
    // over the date field.
#if defined (__WXMAC__)
    Bind(wxEVT_RIGHT_DOWN, &mmDatePickerCtrl::OnCalendar, this);
#endif
}

mmDatePickerCtrl::~mmDatePickerCtrl()
{
    if (!itemStaticTextWeek_)
        delete itemStaticTextWeek_;
    if (!spinButton_)
        delete spinButton_;
}

wxStaticText* mmDatePickerCtrl::getTextWeek()
{
    if (!itemStaticTextWeek_)
    {
        //Text field for name of day of the week
        wxSize WeekDayNameMaxSize(wxDefaultSize);
        for (wxDateTime::WeekDay d = wxDateTime::Sun;
            d != wxDateTime::Inv_WeekDay;
            d = wxDateTime::WeekDay(d+1))
            WeekDayNameMaxSize.IncTo(GetTextExtent(
                wxGetTranslation(wxDateTime::GetEnglishWeekDayName(d))+ " "));
        WeekDayNameMaxSize.SetHeight(-1);
        itemStaticTextWeek_ = new wxStaticText(this->GetParent(), wxID_ANY, "", wxDefaultPosition, WeekDayNameMaxSize, wxST_NO_AUTORESIZE);
        // Force update
        wxDateEvent dateEvent(this, this->GetValue(), wxEVT_DATE_CHANGED);
        OnDateChanged(dateEvent);
    }
    return itemStaticTextWeek_;
}

wxSpinButton* mmDatePickerCtrl::getSpinButton()
{
    if (!spinButton_)
    {
        spinButton_ = new wxSpinButton(this->GetParent(), wxID_ANY
            , wxDefaultPosition, wxSize(-1, this->GetSize().GetHeight())
            , wxSP_VERTICAL | wxSP_ARROW_KEYS | wxSP_WRAP);
        spinButton_->Connect(wxID_ANY, wxEVT_SPIN
            , wxSpinEventHandler(mmDatePickerCtrl::OnDateSpin), nullptr, this);
        spinButton_->SetRange(-32768, 32768);
    }
    return spinButton_;
}

void mmDatePickerCtrl::SetValue(const wxDateTime &dt)
{
    wxDatePickerCtrl::SetValue(dt);
    //trigger date change event
    wxDateEvent dateEvent(this, dt, wxEVT_DATE_CHANGED);
    OnDateChanged(dateEvent);
}

bool mmDatePickerCtrl::Enable(bool state)
{
    bool response = wxDatePickerCtrl::Enable(state);
    if (itemStaticTextWeek_) itemStaticTextWeek_->Enable(state);
    if (spinButton_) spinButton_->Enable(state);
    return response;
}

wxBoxSizer* mmDatePickerCtrl::mmGetLayout()
{
    wxBoxSizer* date_sizer = new wxBoxSizer(wxHORIZONTAL);
    date_sizer->Add(this, g_flagsH);
#if defined(__WXMSW__) || defined(__WXGTK__)
    date_sizer->Add(this->getSpinButton(), g_flagsH);
#endif
    date_sizer->Add(this->getTextWeek(), g_flagsH);

    return date_sizer;
}

void mmDatePickerCtrl::OnCalendar(wxMouseEvent& event)
{
    mmCalendarPopup* m_simplePopup = new mmCalendarPopup( parent_, this );

    // make sure we correctly position the popup below the date
    wxWindow *dateCtrl = static_cast<wxWindow*>(event.GetEventObject());
    wxSize dimensions = dateCtrl->GetSize();
    wxPoint pos = dateCtrl->ClientToScreen(wxPoint(0, dimensions.GetHeight()));
    m_simplePopup->SetPosition(pos);

    m_simplePopup->Popup();
}

void mmDatePickerCtrl::OnDateChanged(wxDateEvent& event)
{
    if (itemStaticTextWeek_)
    {
        wxDateTime dt = event.GetDate();
        itemStaticTextWeek_->SetLabelText(wxGetTranslation(dt.GetEnglishWeekDayName(dt.GetWeekDay())));
    }
    event.Skip();
}

void mmDatePickerCtrl::OnDateSpin(wxSpinEvent& event)
{
    if (spinButton_)
    {
        wxDateTime date = this->GetValue();
        date = date.Add(wxDateSpan::Days(spinButton_->GetValue()));
        this->SetValue(date);
        wxDateEvent evt(this, this->GetValue(), wxEVT_DATE_CHANGED);
        this->GetEventHandler()->AddPendingEvent(evt);
        spinButton_->SetValue(0);
    }
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
    SetForegroundColour(*bestFontColour(getUDColour(m_color_value)));
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
        menuItem->SetTextColour(*bestFontColour(getUDColour(i)));
#endif
        wxBitmap bitmap(mmBitmapBundle(png::EMPTY, mmBitmapButtonSize).GetDefaultSize());
        wxMemoryDC memoryDC(bitmap);
        wxRect rect(memoryDC.GetSize());

        memoryDC.SetBackground(wxBrush(getUDColour(i)));
        memoryDC.Clear();
        memoryDC.DrawBitmap(mmBitmapBundle(png::EMPTY, mmBitmapButtonSize).GetBitmap(wxDefaultSize), 0, 0, true);
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

mmDialogComboBoxAutocomplete::mmDialogComboBoxAutocomplete()
{
}

const wxString mmDialogComboBoxAutocomplete::getText() const
{
    return cbText_->GetValue();
};

mmDialogComboBoxAutocomplete::mmDialogComboBoxAutocomplete(wxWindow *parent, const wxString& message, const wxString& caption,
    const wxString& defaultText, const wxArrayString& choices)
    : m_default_str(defaultText),
    m_choices(choices),
    m_message(message),
    cbText_(nullptr)
{
    if (m_choices.Index(m_default_str) == wxNOT_FOUND)
    {
        m_choices.Add(m_default_str);
        m_choices.Sort(CaseInsensitiveLocaleCmp);
    }
    this->SetFont(parent->GetFont());
    Create(parent, wxID_ANY, caption);
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
    wxStaticText* headerText = new wxStaticText(this, wxID_STATIC, m_message);
    Sizer->Add(headerText, flags);
    Sizer->AddSpacer(15);
    cbText_ = new mmComboBoxCustom(this, m_choices);
    cbText_->SetMinSize(wxSize(150, -1));
    Sizer->Add(cbText_, wxSizerFlags().Border(wxLEFT | wxRIGHT, 15).Expand());
    Sizer->AddSpacer(20);
    wxSizer* Button = CreateButtonSizer(wxOK | wxCANCEL);
    Sizer->Add(Button, flags);
    Sizer->AddSpacer(10);

    cbText_->SetFocus();
    cbText_->ChangeValue(m_default_str);
    cbText_->SelectAll();
    Centre();
    Fit();
    return true;
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

void mmErrorDialogs::InvalidCategory(wxWindow *win)
{
    ToolTip4Object(win, _("Please select an existing category"), _("Invalid Category"), wxICON_ERROR);
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

// -------------------------------------------------------------------------- //

mmMultiChoiceDialog::mmMultiChoiceDialog()
{
}

mmMultiChoiceDialog::mmMultiChoiceDialog(
    wxWindow* parent
    , const wxString& message
    , const wxString& caption
    , const wxArrayString& items)
{
    if (parent) this->SetFont(parent->GetFont());

    wxMultiChoiceDialog::Create(parent, message, caption, items);
    SetMinSize(wxSize(220, 384));
    SetIcon(mmex::getProgramIcon());

    wxButton* ok = static_cast<wxButton*>(FindWindow(wxID_OK));
    if (ok) ok->SetLabel(_("&OK "));
    wxButton* ca = static_cast<wxButton*>(FindWindow(wxID_CANCEL));
    if (ca) ca->SetLabel(wxGetTranslation(g_CancelLabel));
    Fit();
}

/* --------------------------------------------------------- */

mmSingleChoiceDialog::mmSingleChoiceDialog()
{
}
mmSingleChoiceDialog::mmSingleChoiceDialog(wxWindow* parent, const wxString& message,
    const wxString& caption, const wxArrayString& choices)
{
    if (parent) this->SetFont(parent->GetFont());
    wxSingleChoiceDialog::Create(parent, message, caption, choices);
    SetMinSize(wxSize(220, 384));
    SetIcon(mmex::getProgramIcon());

    wxButton* ok = static_cast<wxButton*>(FindWindow(wxID_OK));
    if (ok) ok->SetLabel(wxGetTranslation(g_OkLabel));
    wxButton* ca = static_cast<wxButton*>(FindWindow(wxID_CANCEL));
    if (ca) ca->SetLabel(wxGetTranslation(g_CancelLabel));
    Fit();
}
mmSingleChoiceDialog::mmSingleChoiceDialog(wxWindow* parent, const wxString& message,
    const wxString& caption, const Model_Account::Data_Set& accounts)
{
    if (parent) this->SetFont(parent->GetFont());
    wxArrayString choices;
    for (const auto& item : accounts) choices.Add(item.ACCOUNTNAME);
    wxSingleChoiceDialog::Create(parent, message, caption, choices);
    SetMinSize(wxSize(220, 384));
    SetIcon(mmex::getProgramIcon());

    wxButton* ok = static_cast<wxButton*>(FindWindow(wxID_OK));
    if (ok) ok->SetLabel(wxGetTranslation(g_OkLabel));
    wxButton* ca = static_cast<wxButton*>(FindWindow(wxID_CANCEL));
    if (ca) ca->SetLabel(wxGetTranslation(g_CancelLabel));
    Fit();
}

//------------

mmTagTextCtrl::mmTagTextCtrl(wxWindow* parent, wxWindowID id,
    bool operatorAllowed, const wxPoint& pos, const wxSize& size, long style)
    : wxStyledTextCtrl(parent, id, pos, size, style), operatorAllowed_(operatorAllowed)
{
    SetLexer(wxSTC_LEX_NULL);
    SetWrapMode(wxSTC_WRAP_NONE);
    SetMarginWidth(1, 0);
    SetMarginSensitive(1, false);
    SetUseVerticalScrollBar(false);
    SetUseHorizontalScrollBar(false);
    AutoCompSetIgnoreCase(true);
    AutoCompSetCancelAtStart(true);
    AutoCompSetAutoHide(false);
    StyleSetFont(1, parent->GetFont());
    StyleSetBackground(1, wxColour(186, 226, 185));
    StyleSetForeground(1, *wxBLACK);
    StyleSetFont(0, parent->GetFont());
    StyleSetBackground(0, wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
    StyleSetForeground(0, wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXTEXT));
    SetExtraAscent(2);
    SetExtraDescent(2);
    SetMaxClientSize(wxSize(-1, TextHeight(0)));
    SetWordChars("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.,;:!?'\"()[]{}<>/\\|-_=+*&^%@#$~");
    init();

    Bind(wxEVT_CHAR, &mmTagTextCtrl::OnKeyPressed, this);
    Bind(wxEVT_STC_CLIPBOARD_PASTE, &mmTagTextCtrl::OnPaste, this);
    Bind(wxEVT_PAINT, &mmTagTextCtrl::OnPaint, this);
    Bind(wxEVT_UPDATE_UI, &mmTagTextCtrl::OnChange, this);
    Bind(wxEVT_KILL_FOCUS, &mmTagTextCtrl::OnKillFocus, this);
    Bind(wxEVT_CHAR_HOOK, [this](wxKeyEvent& event)
    {
        if (event.GetKeyCode() == WXK_RETURN)
        {
            int ip = GetInsertionPoint();
            if (GetText().IsEmpty() || GetTextRange(ip - 1, ip) == " ")
            {
                mmTagDialog dlg(this, true, parseTags(GetText()));
                dlg.ShowModal();
                wxString selection;
                for (const auto& tag : dlg.getSelectedTags())
                    selection.Append(tag + " ");
                SetText(selection);
                GotoPos(GetLastPosition());
                if (dlg.getRefreshRequested())
                    init();
            }
            else if (AutoCompActive())
                AutoCompComplete();

            Validate();
            return;
        }
        else if (event.GetKeyCode() == WXK_TAB)
        {
            wxWindow* next_control = GetNextSibling();
            while (next_control && !next_control->IsFocusable())
                next_control = next_control->GetNextSibling();
            if (next_control)
                next_control->SetFocus();
            return;
        }
        else
            event.Skip();
    });
}

void mmTagTextCtrl::init()
{
    // Initialize the tag map and autocomplete string
    tag_map_.clear();
    tags_autocomp_str.Clear();
    for (const auto& tag : Model_Tag::instance().all(DB_Table_TAG_V1::COL_TAGNAME))
    {
        tag_map_[tag.TAGNAME] = tag.TAGID;
        tags_autocomp_str.Append(tag.TAGNAME + " ");
    }
}

void mmTagTextCtrl::OnKeyPressed(wxKeyEvent& event)
{
    int code = event.GetUnicodeKey();

    if (code == WXK_SPACE)
    {
        AutoCompCancel();
        InsertText(GetInsertionPoint(), " ");
        Validate();
        return;
    }
    else if (code != WXK_NONE && code > 32) {

        int ip = GetInsertionPoint();
        int tag_start = WordStartPosition(ip, true);
        int tag_end = WordEndPosition(ip, true);

        // Show autocomplete
        wxString pattern = GetText().Mid(tag_start, ip).Append(wxChar(code));
        tags_autocomp_str.Clear();
        for (const auto& tag : tag_map_)
        {
            if (tag.first.Lower().Matches(pattern.Lower().Prepend("*").Append("*")))
                tags_autocomp_str.Append(tag.first + " ");
        }
        if (!tags_autocomp_str.IsEmpty() && code != WXK_NONE && code > 32)
        {
            AutoCompShow(tag_end - tag_start, tags_autocomp_str);
        }
    }
    event.Skip();
}

void mmTagTextCtrl::OnPaste(wxStyledTextEvent& event)
{
    wxString currText = GetText();
    Validate(currText.insert(GetInsertionPoint(), event.GetString()));
    event.SetString("");
}

void mmTagTextCtrl::OnKillFocus(wxFocusEvent& event)
{
    AutoCompCancel();
    // Remove any non-tags
    wxString tagString;
    wxArrayString tags = parseTags(GetText());
    for (const auto& tag : tags)
    {
        if (tag_map_.find(tag) != tag_map_.end())
            tagString.Append(tag + " ");
    }
    SetText(tagString.Trim());
    event.Skip();
}

void mmTagTextCtrl::OnChange(wxCommandEvent& WXUNUSED(event))
{
    if (!IsEnabled())
        StyleSetBackground(wxSTC_STYLE_DEFAULT, wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));
    else
        StyleSetBackground(wxSTC_STYLE_DEFAULT, wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
}

void mmTagTextCtrl::OnPaint(wxPaintEvent& event)
{
    int end = GetTextLength();
    int position = 0;

    // Reset the text style
    ClearDocumentStyle();

    while (position < end)
    {
        // Find start and end of word
        int wordStart = WordStartPosition(position, true);
        int wordEnd = WordEndPosition(position, true);
        wxString word = GetTextRange(wordStart, wordEnd);

        auto it = tag_map_.find(word);
        // If the word is a valid tag, color it
        if (it != tag_map_.end())
        {
            StartStyling(wordStart);
            SetStyling(wordEnd - wordStart, 1);
        }
        
        position = wordEnd + 1;
    }

    event.Skip();
}

/* Validates all tags passed in tagText, or the contents of the text control if tagText is blank */
bool mmTagTextCtrl::Validate(const wxString& tagText)
{
    // Clear stored tags
    tags_.clear();

    int ip = GetInsertionPoint();

    // If we are passed a string validate it, otherwise validate the text control contents
    wxString tags_in = tagText;
    if (tags_in.IsEmpty())
        tags_in = GetText();

    if (tags_in.IsEmpty()) return true;

    wxString tags_out;
    bool newTagCreated = false;
    bool is_valid = true;

    // parse the tags and prompt to create any which don't exist
    for (const auto& tag : parseTags(tags_in))
    {
        // ignore search operators
        if (tag == "&" || tag == "|")
        {
            tags_out.Append(tag + " ");
            continue;
        }

        if (tag_map_.find(tag) == tag_map_.end())
        {
            // Prompt user to create a new tag
            if (wxMessageDialog(nullptr, wxString::Format(_("Create new tag '%s'?"), tag), _("New tag entered"), wxYES_NO).ShowModal() == wxID_YES)
            {
                newTagCreated = true;
                Model_Tag::Data* newTag = Model_Tag::instance().create();
                newTag->TAGNAME = tag;
                newTag->ACTIVE = 1;
                Model_Tag::instance().save(newTag);
                // Save the new tag to reference
                tag_map_[tag] = newTag->TAGID;
            }
            else
            {
                is_valid = false;
                continue;
            }
        }

        tags_[tag] = tag_map_[tag];
        tags_out.Append(tag + " ");
    }

    // Replace tags with case-corrected versions and remove duplicates
    SetEvtHandlerEnabled(false);
    SetText(tags_out);
    GotoPos(WordEndPosition(ip, true) + 1);
    SetEvtHandlerEnabled(true);

    return is_valid;
}

/* Return a list of tag IDs contained in the control */
const wxArrayInt mmTagTextCtrl::GetTagIDs() const
{
    wxArrayInt tags_out;
    for (const auto& tag : tags_)
        tags_out.Add(tag.second);

    return tags_out;
}

wxArrayString mmTagTextCtrl::parseTags(const wxString& tagString)
{
    wxStringTokenizer tokenizer = wxStringTokenizer(tagString, " ");
    wxArrayString tags;
    while (tokenizer.HasMoreTokens())
    {
        wxString token = tokenizer.GetNextToken();


        // ignore search operators
        if (token == "&" || token == "|")
        {
            if (operatorAllowed_) tags.Add(token);
            continue;
        }

        bool tagUsed = false;
        // if the tag has already been entered, skip it to avoid duplicates
        for (const auto& tag : tags)
            if (tag.IsSameAs(token, false))
            {
                tagUsed = true;
                break;
            }

        if (tagUsed) continue;

        auto it = tag_map_.find(token);
        if (it != tag_map_.end())
            // case correction for existing tag
            tags.Add((*it).first);
        else
            tags.Add(token);
    }

    return tags;
}

void mmTagTextCtrl::SetTags(const wxArrayInt& tagIds)
{
    // Save the tag IDs and tag names
    tags_.clear();
    for (const auto& tagId : tagIds)
        for (const auto& tag : tag_map_)
            if (tag.second == tagId)
            {
                tags_[tag.first] = tagId;
                break;
            }

    // Set the text of the control (sorted)
    wxString tagString;
    for (const auto& tag : tags_)
        tagString.Append(tag.first + " ");

    SetText(tagString);
}
