/*******************************************************
Copyright (C) 2014 Gabriele-V
Copyright (C) 2015, 2016, 2020, 2022 Nikolay Akimov
Copyright (C) 2022, 2023 Mark Whalley (mark@ipx.co.uk)

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

#include "wx/dcbuffer.h"
#include "wx/popupwin.h"
#include "wx/spinctrl.h"
#include <wx/graphics.h>
#include <wx/renderer.h>
#include <wx/richtooltip.h>

#include "mmSimpleDialogs.h"
#include "constants.h"
#include "images_list.h"
#include "mmex.h"
#include "paths.h"
#include "platfdep.h"
#include "tagdialog.h"
#include "util.h"
#include "validators.h"

#include "model/Model_Setting.h"
#include "model/Model_Account.h"
#include "model/Model_Payee.h"
#include "model/Model_Category.h"
#include "model/Model_Tag.h"

//------- Pop-up calendar, currently only used for MacOS only
// See: https://github.com/moneymanagerex/moneymanagerex/issues/3139

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

//----------------------------------------------------------------------------
// mmCalculatorPopup
//----------------------------------------------------------------------------

mmCalculatorPopup::mmCalculatorPopup(wxWindow* parent, mmTextCtrl* target) : wxPopupTransientWindow(parent, wxBORDER_THEME | wxPU_CONTAINS_CONTROLS), target_(target)
{
    wxWindow* panel = new wxWindow(this, wxID_ANY);
    wxFont font = parent->GetFont();
    SetFont(font);
    int fontSize = font.GetPointSize();
    wxSize btnSize = wxSize(fontSize + 25, fontSize + 25);
    wxFlexGridSizer* sizer;
    sizer = new wxFlexGridSizer(2, 1, 0, 0);
    sizer->SetFlexibleDirection(wxVERTICAL);
    sizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

    valueTextCtrl_ = new mmTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    valueTextCtrl_->SetFont(font);
    valueTextCtrl_->SetIgnoreFocusChange(true);
    sizer->Add(valueTextCtrl_, g_flagsExpand);

    wxGridSizer* buttonSizer;
    buttonSizer = new wxGridSizer(5, 4, 0, 0);
    
    button_lparen_ = new wxButton(panel, wxID_ANY, "(", wxDefaultPosition, btnSize);
    button_lparen_->Bind(wxEVT_BUTTON, &mmCalculatorPopup::OnButtonPressed, this);
    button_lparen_->SetFont(font);
    buttonSizer->Add(button_lparen_, wxSizerFlags(g_flagsH).Border(wxALL, 1));

    button_rparen_ = new wxButton(panel, wxID_ANY, ")", wxDefaultPosition, btnSize);
    button_rparen_->Bind(wxEVT_BUTTON, &mmCalculatorPopup::OnButtonPressed, this);
    button_rparen_->SetFont(font);
    buttonSizer->Add(button_rparen_, wxSizerFlags(g_flagsH).Border(wxALL, 1));

    button_clear_ = new wxButton(panel, wxID_ANY, "C", wxDefaultPosition, btnSize);
    button_clear_->Bind(wxEVT_BUTTON, &mmCalculatorPopup::OnButtonPressed, this);
    button_clear_->SetFont(font);
    buttonSizer->Add(button_clear_, wxSizerFlags(g_flagsH).Border(wxALL, 1));

    button_del_ = new wxButton(panel, mmID_DELETE, wxString::FromUTF8Unchecked("\u232b"), wxDefaultPosition, btnSize);
    button_del_->Bind(wxEVT_BUTTON, &mmCalculatorPopup::OnButtonPressed, this);
    button_del_->SetFont(font);
    buttonSizer->Add(button_del_, wxSizerFlags(g_flagsH).Border(wxALL, 1));

    button_7_ = new wxButton(panel, wxID_ANY, "7", wxDefaultPosition, btnSize);
    button_7_->Bind(wxEVT_BUTTON, &mmCalculatorPopup::OnButtonPressed, this);
    button_7_->SetFont(font);
    buttonSizer->Add(button_7_, wxSizerFlags(g_flagsH).Border(wxALL, 1));

    button_8_ = new wxButton(panel, wxID_ANY, "8", wxDefaultPosition, btnSize);
    button_8_->Bind(wxEVT_BUTTON, &mmCalculatorPopup::OnButtonPressed, this);
    button_8_->SetFont(font);
    buttonSizer->Add(button_8_, wxSizerFlags(g_flagsH).Border(wxALL, 1));

    button_9_ = new wxButton(panel, wxID_ANY, "9", wxDefaultPosition, btnSize);
    button_9_->Bind(wxEVT_BUTTON, &mmCalculatorPopup::OnButtonPressed, this);
    button_9_->SetFont(font);
    buttonSizer->Add(button_9_, wxSizerFlags(g_flagsH).Border(wxALL, 1));

    button_div_ = new wxButton(panel, mmID_DIVIDE, wxString::FromUTF8Unchecked("\u00f7"), wxDefaultPosition, btnSize);
    button_div_->Bind(wxEVT_BUTTON, &mmCalculatorPopup::OnButtonPressed, this);
    button_div_->SetFont(font);
    buttonSizer->Add(button_div_, wxSizerFlags(g_flagsH).Border(wxALL, 1));

    button_4_ = new wxButton(panel, wxID_ANY, "4", wxDefaultPosition, btnSize);
    button_4_->Bind(wxEVT_BUTTON, &mmCalculatorPopup::OnButtonPressed, this);
    button_4_->SetFont(font);
    buttonSizer->Add(button_4_, wxSizerFlags(g_flagsH).Border(wxALL, 1));

    button_5_ = new wxButton(panel, wxID_ANY, "5", wxDefaultPosition, btnSize);
    button_5_->Bind(wxEVT_BUTTON, &mmCalculatorPopup::OnButtonPressed, this);
    button_5_->SetFont(font);
    buttonSizer->Add(button_5_, wxSizerFlags(g_flagsH).Border(wxALL, 1));

    button_6_ = new wxButton(panel, wxID_ANY, "6", wxDefaultPosition, btnSize);
    button_6_->Bind(wxEVT_BUTTON, &mmCalculatorPopup::OnButtonPressed, this);
    button_6_->SetFont(font);
    buttonSizer->Add(button_6_, wxSizerFlags(g_flagsH).Border(wxALL, 1));

    button_mult_ = new wxButton(panel, mmID_MULTIPLY, wxString::FromUTF8Unchecked("\u00d7"), wxDefaultPosition, btnSize);
    button_mult_->Bind(wxEVT_BUTTON, &mmCalculatorPopup::OnButtonPressed, this);
    button_mult_->SetFont(font);
    buttonSizer->Add(button_mult_, wxSizerFlags(g_flagsH).Border(wxALL, 1));

    button_1_ = new wxButton(panel, wxID_ANY, "1", wxDefaultPosition, btnSize);
    button_1_->Bind(wxEVT_BUTTON, &mmCalculatorPopup::OnButtonPressed, this);
    button_1_->SetFont(font);
    buttonSizer->Add(button_1_, wxSizerFlags(g_flagsH).Border(wxALL, 1));

    button_2_ = new wxButton(panel, wxID_ANY, "2", wxDefaultPosition, btnSize);
    button_2_->Bind(wxEVT_BUTTON, &mmCalculatorPopup::OnButtonPressed, this);
    button_2_->SetFont(font);
    buttonSizer->Add(button_2_, wxSizerFlags(g_flagsH).Border(wxALL, 1));

    button_3_ = new wxButton(panel, wxID_ANY, "3", wxDefaultPosition, btnSize);
    button_3_->Bind(wxEVT_BUTTON, &mmCalculatorPopup::OnButtonPressed, this);
    button_3_->SetFont(font);
    buttonSizer->Add(button_3_, wxSizerFlags(g_flagsH).Border(wxALL, 1));

    button_minus_ = new wxButton(panel, wxID_ANY, "-", wxDefaultPosition, btnSize);
    button_minus_->Bind(wxEVT_BUTTON, &mmCalculatorPopup::OnButtonPressed, this);
    button_minus_->SetFont(font);
    buttonSizer->Add(button_minus_, wxSizerFlags(g_flagsH).Border(wxALL, 1));

    button_0_ = new wxButton(panel, wxID_ANY, "0", wxDefaultPosition, btnSize);
    button_0_->Bind(wxEVT_BUTTON, &mmCalculatorPopup::OnButtonPressed, this);
    button_0_->SetFont(font);
    buttonSizer->Add(button_0_, wxSizerFlags(g_flagsH).Border(wxALL, 1));

    button_dec_ = new wxButton(panel, wxID_ANY, ".", wxDefaultPosition, btnSize);
    button_dec_->Bind(wxEVT_BUTTON, &mmCalculatorPopup::OnButtonPressed, this);
    button_dec_->SetFont(font);
    buttonSizer->Add(button_dec_, wxSizerFlags(g_flagsH).Border(wxALL, 1));

    button_equal_ = new wxButton(panel, wxID_ANY, "=", wxDefaultPosition, btnSize);
    button_equal_->Bind(wxEVT_BUTTON, &mmCalculatorPopup::OnButtonPressed, this);
    button_equal_->SetFont(font);
    buttonSizer->Add(button_equal_, wxSizerFlags(g_flagsH).Border(wxALL, 1));

    button_plus_ = new wxButton(panel, wxID_ANY, "+", wxDefaultPosition, btnSize);
    button_plus_->Bind(wxEVT_BUTTON, &mmCalculatorPopup::OnButtonPressed, this);
    button_plus_->SetFont(font);
    buttonSizer->Add(button_plus_, wxSizerFlags(g_flagsH).Border(wxALL, 1));

    sizer->Add(buttonSizer, g_flagsH);

    panel->SetSizer(sizer);
    sizer->Fit(panel);
    SetClientSize(panel->GetSize());
}

mmCalculatorPopup::~mmCalculatorPopup()
{
}

void mmCalculatorPopup::SetValue(wxString& value)
{
    if (target_)
        valueTextCtrl_->SetCurrency(target_->GetCurrency());
    valueTextCtrl_->ChangeValue(value);
    valueTextCtrl_->SelectNone();
    valueTextCtrl_->SetInsertionPointEnd();
}

void mmCalculatorPopup::OnButtonPressed(wxCommandEvent& event)
{
    wxButton* btn = dynamic_cast<wxButton*>(event.GetEventObject());
    int id = event.GetId();
    long from;
    long to;
    valueTextCtrl_->GetSelection(&from, &to);
    wxString text = btn->GetLabel();
    wxString value = valueTextCtrl_->GetValue();
    int ip = valueTextCtrl_->GetInsertionPoint();

    if (text == "=")
    {
        valueTextCtrl_->Calculate();
        ip = valueTextCtrl_->GetLastPosition();
    }
    else if (text == "C")
    {
        valueTextCtrl_->ChangeValue("");
        ip = 0;
    }
    else if (id == mmID_DELETE)
    {
        if (from != to)
            valueTextCtrl_->ChangeValue(value.Remove(from, to - from));
        else if (ip > 0)
        {
            valueTextCtrl_->Remove(ip - 1, ip);
            ip -= 1;
        }
    }
    else if (id == mmID_MULTIPLY)
    {
        valueTextCtrl_->WriteText("*");
        ip += 1;
    }
    else if (id == mmID_DIVIDE)
    {
        valueTextCtrl_->WriteText("/");
        ip += 1;
    }
    else
    {
        valueTextCtrl_->WriteText(text);
        ip += 1;
    }
    valueTextCtrl_->SetFocus();
    valueTextCtrl_->SelectNone();
    valueTextCtrl_->SetInsertionPoint(ip);
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

void mmComboBox::OnDropDown(wxCommandEvent&)
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
           ChangeValue(selection);
           if (!selection.IsEmpty() && FindString(selection, false) != wxNOT_FOUND)
               SetStringSelection(selection);
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

void mmComboBox::mmSetId(int64 id)
{
    auto result = std::find_if(all_elements_.begin(), all_elements_.end(),
        [id](const std::pair<wxString, int64>& mo) {return mo.second == id; });

    if (result != all_elements_.end())
        ChangeValue(result->first);
}

int64 mmComboBox::mmGetId() const
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
    if ((is_initialized_) && (typedText.IsEmpty() || (this->GetSelection() == -1)))
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
        ChangeValue(text.Trim().Append(Model_Infotable::instance().getString("CATEG_DELIMITER", ":")));
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
    , wxSize size, int64 accountID, bool excludeClosed)
    : mmComboBox(parent, id, size)
    , accountID_(accountID)
    , excludeClosed_(excludeClosed)     
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
                    , wxSize size, int64 payeeID, bool excludeHidden)
    : mmComboBox(parent, id, size)
    , payeeID_(payeeID)
    , excludeHidden_(excludeHidden)    
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
                    , wxSize size, int64 catID, bool excludeHidden)
    : mmComboBox(parent, id, size)    
    , catID_(catID)
    , excludeHidden_(excludeHidden)
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

int64 mmComboBoxCategory::mmGetCategoryId() const
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


mmDatePickerCtrl::mmDatePickerCtrl(wxWindow* parent, wxWindowID id, wxDateTime dt, wxPoint pos, wxSize size, long style)
    : wxPanel(parent, id, pos, size, style)
    , dt_(dt), parent_(parent)
{
    if (!dt.IsValid())
        dt_ = wxDateTime::Now();
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    SetSizer(sizer);
    datePicker_ = new wxDatePickerCtrl(this, id, dt, wxDefaultPosition, wxDefaultSize, style);
    datePicker_->SetRange(wxDateTime(), DATE_MAX);
    SetValue(dt_);
    datePicker_->Bind(wxEVT_DATE_CHANGED, &mmDatePickerCtrl::OnDateChanged, this);
    sizer->Add(datePicker_);
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
        itemStaticTextWeek_ = new wxStaticText(parent_, wxID_ANY, "", wxDefaultPosition, WeekDayNameMaxSize, wxST_NO_AUTORESIZE);
        // Force update
        wxDateEvent dateEvent(this, datePicker_->GetValue(), wxEVT_DATE_CHANGED);
        OnDateChanged(dateEvent);
    }
    return itemStaticTextWeek_;
}

wxSpinButton* mmDatePickerCtrl::getSpinButton()
{
    if (!spinButton_)
    {
        spinButton_ = new wxSpinButton(parent_, wxID_ANY
            , wxDefaultPosition, wxSize(-1, GetBestSize().GetHeight())
            , wxSP_VERTICAL | wxSP_ARROW_KEYS | wxSP_WRAP);
        spinButton_->Bind(wxEVT_SPIN, &mmDatePickerCtrl::OnDateSpin, this);
        spinButton_->SetRange(-32768, 32768);
    }
    return spinButton_;
}

void mmDatePickerCtrl::SetValue(const wxDateTime &dt)
{
    if (dt > DATE_MAX.GetDateOnly())
        datePicker_->SetValue(DATE_MAX.GetDateOnly());
    else
        datePicker_->SetValue(dt);

    if (timePicker_)
        timePicker_->SetValue(dt);

    //trigger date change event
    wxDateEvent dateEvent(this, dt, wxEVT_DATE_CHANGED);
    OnDateChanged(dateEvent);
}

bool mmDatePickerCtrl::Enable(bool state)
{
    bool response = datePicker_->Enable(state);
    if (timePicker_)
        timePicker_->Enable(state);
    if (itemStaticTextWeek_) itemStaticTextWeek_->Enable(state);
    if (spinButton_) spinButton_->Enable(state);
    return response;
}

// Gets the full layout including spin buttons, time picker, and day of week
wxBoxSizer* mmDatePickerCtrl::mmGetLayout(bool showTimeCtrl)
{
    wxBoxSizer* date_sizer = new wxBoxSizer(wxHORIZONTAL);
    date_sizer->Add(this, g_flagsH);
#if defined(__WXMSW__) || defined(__WXGTK__)
    date_sizer->Add(this->getSpinButton(), g_flagsH);
#endif
    // If time picker is requested and turned on in Options, add it to the layout
    if (showTimeCtrl && Option::instance().UseTransDateTime())
    {
        timePicker_ = new wxTimePickerCtrl(parent_, GetId(), dt_, wxDefaultPosition, wxDefaultSize);
        timePicker_->Bind(wxEVT_TIME_CHANGED, &mmDatePickerCtrl::OnDateChanged, this);
        date_sizer->Add(timePicker_, g_flagsH);
    }
    date_sizer->Add(this->getTextWeek(), g_flagsH);

    return date_sizer;
}

// Adds only the time picker to the layout if "Use time" is turned on in Options
wxBoxSizer* mmDatePickerCtrl::mmGetLayoutWithTime()
{
    wxBoxSizer* date_sizer = new wxBoxSizer(wxHORIZONTAL);
    date_sizer->Add(this, g_flagsH);
    if (Option::instance().UseTransDateTime())
    {
        timePicker_ = new wxTimePickerCtrl(parent_, GetId(), dt_, wxDefaultPosition, wxDefaultSize);
        timePicker_->Bind(wxEVT_TIME_CHANGED, &mmDatePickerCtrl::OnDateChanged, this);
        date_sizer->Add(timePicker_, g_flagsH);
    }

    return date_sizer;
}

void mmDatePickerCtrl::OnDateChanged(wxDateEvent& event)
{
    if (timePicker_)
        dt_.ParseISOCombined(datePicker_->GetValue().FormatISODate() + "T" + timePicker_->GetValue().FormatISOTime());
    else
        dt_ = datePicker_->GetValue();

    if (itemStaticTextWeek_)
    {
        itemStaticTextWeek_->SetLabelText(wxGetTranslation(dt_.GetEnglishWeekDayName(dt_.GetWeekDay())));
    }

    event.SetDate(dt_);
    event.Skip();
}

void mmDatePickerCtrl::OnDateSpin(wxSpinEvent&)
{
    if (spinButton_)
    {
        wxDateTime date = GetValue();
        date = date.Add(wxDateSpan::Days(spinButton_->GetValue()));
        SetValue(date);
        wxDateEvent evt(this, GetValue(), wxEVT_DATE_CHANGED);
        GetEventHandler()->AddPendingEvent(evt);
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
            SetLabel(wxString::Format(_("&Clear color")));
        }
        else {
            SetLabel(wxString::Format(_("Color #&%i"), m_color_value));
        }
    }
    event.Skip();
}

void mmColorButton::OnColourButton(wxCommandEvent& event)
{
    wxMenu mainMenu;
    wxMenuItem* menuItem = new wxMenuItem(&mainMenu, wxID_HIGHEST, wxString::Format(_("&Clear color"), 0));
    mainMenu.Append(menuItem);

    for (int i = 1; i <= 7; ++i)
    {
        menuItem = new wxMenuItem(&mainMenu, wxID_HIGHEST + i, wxString::Format(_("Color #&%i"), i));
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
        { ".", "." },
        { ",", "," },
        { _n("None"), "" }
    };

    for (const auto& entry : DATA) {
        this->Append(wxGetTranslation(entry.first), new wxStringClientData(entry.second));
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
}

mmDialogComboBoxAutocomplete::mmDialogComboBoxAutocomplete(wxWindow *parent, const wxString& message, const wxString& caption,
    const wxString& defaultText, const wxArrayString& choices)
    : m_default_str(defaultText),
    m_choices(choices),
    m_message(message)
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
    return true;
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
    const wxString errorHeader = _("Invalid Account");
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
    const wxString errorHeader = _("Invalid Payee");
    const wxString errorMessage = _("Please type in a new payee,\n"
        "or make a selection using the dropdown button.")
        + "\n";
    ToolTip4Object(object, errorMessage, errorHeader, wxICON_ERROR);
}

void mmErrorDialogs::InvalidName(wxTextCtrl *textBox, bool alreadyexist)
{
    const wxString errorHeader = _("Invalid Name");
    wxString errorMessage;
    if (alreadyexist)
        errorMessage = _("Already exist!");
    else
        errorMessage = _("Please type in a non empty name.");

    ToolTip4Object(textBox, errorMessage, errorHeader, wxICON_ERROR);
}

void mmErrorDialogs::InvalidSymbol(wxTextCtrl *textBox, bool alreadyexist)
{
    const wxString errorHeader = _("Invalid Name");
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
    : wxPanel(), operatorAllowed_(operatorAllowed)
{
#ifdef __WXMAC__
    if (mmex::isDarkMode())
    {
        style |= wxBORDER_SIMPLE;
        bgColorEnabled_ = wxColour(58, 58, 58);
        bgColorDisabled_ = wxColour(45, 45, 45);
    }
    else
    {
        style |= wxBORDER_THEME;
        bgColorDisabled_ = wxColour(251, 251, 251);
    }
#else
    style |= wxBORDER_NONE;
#endif
    Create(parent, id, pos, size, style);
    SetFont(parent->GetFont());
    SetBackgroundColour(bgColorEnabled_);
    wxBoxSizer* panel_sizer = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* v_sizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* h_sizer = new wxBoxSizer(wxHORIZONTAL);
    textCtrl_ = new wxStyledTextCtrl(this, wxID_DEFAULT, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    if (operatorAllowed_)
        mmToolTip(textCtrl_,
            _("Enter tags to search, optionally separated by the operators '&' and '|'.") + "\n" +
            _("The default operator between consecutive tags is AND, use '|' to change the operator to OR.")
        );
    // Set options for the text ctrl
    textCtrl_->SetLexer(wxSTC_LEX_NULL);
    textCtrl_->SetWrapMode(wxSTC_WRAP_NONE);
    textCtrl_->SetMarginWidth(1, 0);
    textCtrl_->SetMarginSensitive(1, false);
    textCtrl_->SetUseVerticalScrollBar(false);
    textCtrl_->SetUseHorizontalScrollBar(false);
    textCtrl_->AutoCompSetIgnoreCase(true);
    textCtrl_->AutoCompSetCancelAtStart(true);
    textCtrl_->AutoCompSetAutoHide(false);
    textCtrl_->StyleSetFont(1, parent->GetFont());
    textCtrl_->StyleSetBackground(1, wxColour(186, 226, 185));
    textCtrl_->StyleSetForeground(1, *wxBLACK);
    textCtrl_->StyleSetFont(0, parent->GetFont());
    textCtrl_->StyleSetBackground(0, bgColorEnabled_);
    textCtrl_->StyleSetForeground(0, wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXTEXT));
    textCtrl_->StyleSetBackground(wxSTC_STYLE_DEFAULT, bgColorEnabled_);
    textCtrl_->StyleSetForeground(wxSTC_STYLE_DEFAULT, wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXTEXT));
    textCtrl_->SetCaretForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXTEXT));
    textCtrl_->SetMaxClientSize(wxSize(-1, textCtrl_->TextHeight(0)));
    textCtrl_->SetWordChars("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.,;:!?'\"()[]{}<>/\\|-_=+*&^%@#$~");

    // Text ctrl event handlers
    textCtrl_->Bind(wxEVT_CHAR, &mmTagTextCtrl::OnTextChanged, this);
    textCtrl_->Bind(wxEVT_STC_CLIPBOARD_PASTE, &mmTagTextCtrl::OnPaste, this);
    textCtrl_->Bind(wxEVT_PAINT, &mmTagTextCtrl::OnPaint, this);
    Bind(wxEVT_PAINT, &mmTagTextCtrl::OnPaint, this);
    textCtrl_->Bind(wxEVT_KILL_FOCUS, &mmTagTextCtrl::OnKillFocus, this);
    textCtrl_->Bind(wxEVT_CHAR_HOOK, &mmTagTextCtrl::OnKeyPressed, this);
    textCtrl_->Bind(wxEVT_STC_ZOOM, [this](wxStyledTextEvent& ) {
        // Disable zoom
        textCtrl_->SetEvtHandlerEnabled(false);
        textCtrl_->SetZoom(0);
        textCtrl_->SetEvtHandlerEnabled(true);
    });

    wxSize btnSize;
#ifdef __WXMSW__
    panelHeight_ = textCtrl_->TextHeight(0) + 8;
    textOffset_ = 2;
    h_sizer->Add(textCtrl_, 1, wxALIGN_CENTER_VERTICAL | wxLEFT, textOffset_);
    wxWindowDC dc(this);
    btnSize = wxRendererNative::Get().GetCollapseButtonSize(this, dc);
#elif __WXGTK__
    panelHeight_ = std::max(textCtrl_->TextHeight(0) + 2, 34);
    textOffset_ = 8;
    h_sizer->Add(textCtrl_, 1, wxALIGN_CENTER_VERTICAL | wxLEFT, textOffset_);
    btnSize = wxSize(panelHeight_ + 3, panelHeight_);
#else
    panelHeight_ = textCtrl_->TextHeight(0) + 4;
    textOffset_ = 1;
    h_sizer->Add(textCtrl_, 1, wxALIGN_CENTER_VERTICAL | wxLEFT, textOffset_);
    btnSize = wxSize(textCtrl_->TextHeight(0), textCtrl_->TextHeight(0));
#endif

    createDropButton(btnSize);

    btn_dropdown_->Bind(wxEVT_BUTTON, &mmTagTextCtrl::OnDropDown, this);
    btn_dropdown_->Bind(wxEVT_NAVIGATION_KEY, [this](wxNavigationKeyEvent& ) { textCtrl_->SetFocus(); });

#ifndef __WXMAC__
    // Event handlers for custom control painting in Windows & Linux
    Bind(wxEVT_ENTER_WINDOW, &mmTagTextCtrl::OnMouseCaptureChange, this);
    Bind(wxEVT_LEAVE_WINDOW, &mmTagTextCtrl::OnMouseCaptureChange, this);
    textCtrl_->Bind(wxEVT_ENTER_WINDOW, &mmTagTextCtrl::OnMouseCaptureChange, this);
    textCtrl_->Bind(wxEVT_LEAVE_WINDOW, &mmTagTextCtrl::OnMouseCaptureChange, this);
    textCtrl_->Bind(wxEVT_SIZE, [this](wxSizeEvent& ) {textCtrl_->Refresh(); });
    btn_dropdown_->Bind(wxEVT_LEAVE_WINDOW, &mmTagTextCtrl::OnMouseCaptureChange, this);
    btn_dropdown_->Bind(wxEVT_SET_FOCUS, &mmTagTextCtrl::OnFocusChange, this);
    btn_dropdown_->Bind(wxEVT_ENTER_WINDOW, &mmTagTextCtrl::OnMouseCaptureChange, this);
    btn_dropdown_->Bind(wxEVT_KILL_FOCUS, &mmTagTextCtrl::OnFocusChange, this);
#endif

    v_sizer->Add(h_sizer, 1, wxEXPAND, 0);
    panel_sizer->Add(v_sizer, 1, wxEXPAND, 0);
    panel_sizer->Add(btn_dropdown_, 0, wxEXPAND | wxALL, 0);

    // Popup window
    popupWindow_ = new mmTagCtrlPopupWindow(this, btn_dropdown_);
    wxScrolledWindow* scrolledWindow = new wxScrolledWindow(popupWindow_);
    tagCheckListBox_ = new wxCheckListBox(scrolledWindow, wxID_ANY, wxDefaultPosition, wxDefaultSize, {}, wxLB_SORT);
    tagCheckListBox_->SetFont(GetFont());
    tagCheckListBox_->Bind(wxEVT_CHECKLISTBOX, &mmTagTextCtrl::OnPopupCheckboxSelected, this);

    wxBoxSizer* sw_sizer = new wxBoxSizer(wxVERTICAL);
    sw_sizer->Add(tagCheckListBox_, 1, wxEXPAND);
    scrolledWindow->SetSizer(sw_sizer);
    init();
    scrolledWindow->Fit();
    sw_sizer->Fit(popupWindow_);

    SetSizer(panel_sizer);
    SetSizeHints(-1, panelHeight_, -1, panelHeight_);
    Layout();
    btn_dropdown_->Refresh();
    btn_dropdown_->Update();
}

void mmTagTextCtrl::createDropButton(wxSize btnSize)
{
#ifdef __WXMSW__
    // On Windows draw the drop arrow and store the bitmap to be used in the paint override
    wxWindowDC dc(this);
    btnSize.SetWidth(btnSize.GetWidth() - 2);
    btnSize.SetHeight(panelHeight_ - 3);
    btn_dropdown_ = new wxBitmapButton(this, wxID_ANY, wxNullBitmap, wxPoint(-1, 1), btnSize, wxBORDER_NONE, wxDefaultValidator, "btn_dropdown_");
    btn_dropdown_->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
    btn_dropdown_->Bind(wxEVT_PAINT, &mmTagTextCtrl::OnPaintButton, this);

    // Recreate the Native windows-style drop arrow
    // Begin the image with a transparent background
    wxImage img(btnSize);
    img.InitAlpha();
    unsigned char* alpha = img.GetAlpha();
    memset(alpha, wxIMAGE_ALPHA_TRANSPARENT, img.GetWidth() * img.GetHeight());

    dropArrow_ = wxBitmap(img);
    // Normal wxDC doesn't play nice with an alpha channel in Windows
    // so use a wxGraphicsContext to draw the image in the bitmap
    wxGraphicsContext* gc = wxGraphicsContext::Create(dropArrow_);
    gc->SetAntialiasMode(wxANTIALIAS_NONE);
    int arrowX = (btnSize.GetWidth() - 8) / 2;
    int arrowY = btnSize.GetHeight() / 2;

    wxPoint2DDouble ptstart[2] = {
        wxPoint(arrowX, arrowY),     // top L
        wxPoint(arrowX + 7, arrowY), // top R
    };

    wxPoint2DDouble ptend[2] = {
        wxPoint(arrowX + 3, arrowY + 3), // bottom center L
        wxPoint(arrowX + 4, arrowY + 3)  // bottom center R
    };

    wxPoint2DDouble sptstart[2] = {
        wxPoint(arrowX, arrowY - 1),     // shadow top L
        wxPoint(arrowX + 7, arrowY - 1), // shadow top R
    };

    wxPoint2DDouble sptend[2] = {
        wxPoint(arrowX + 3, arrowY + 2), // shadow bottom center L
        wxPoint(arrowX + 4, arrowY + 2)  // shadow bottom center R
    };

    wxPoint2DDouble lsptstart[2] = {
        wxPoint(arrowX - 1, arrowY), // light shadow top L
        wxPoint(arrowX + 8, arrowY), // light shadow top R
    };

    wxPoint2DDouble lsptend[2] = {
        wxPoint(arrowX + 3, arrowY + 4), // light shadow bottom center L
        wxPoint(arrowX + 4, arrowY + 4)  // light shadow bottom center R
    };

    // draw shadow
    wxColour color = wxSystemSettings::GetColour(wxSYS_COLOUR_3DSHADOW);
    color.SetRGBA(0xA6000000 | color.GetRGB());
    gc->SetPen(color);
    gc->StrokeLines(2, sptstart, sptend);

    // draw triangle
    color = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT);
    color.SetRGBA(0xA6000000 | color.GetRGB());
    gc->SetPen(color);
    gc->StrokeLines(2, ptstart, ptend);

    // draw light shadow
    color = wxSystemSettings::GetColour(wxSYS_COLOUR_3DLIGHT);
    color.SetRGBA(0xA6000000 | color.GetRGB());
    gc->SetPen(color);
    gc->StrokeLines(2, lsptstart, lsptend);

    delete gc;

    // Duplicate the image and adjust opacity to create the "inactive" drop arrow
    wxImage inactiveImg(dropArrow_.ConvertToImage());
    for (int i = 0; i < inactiveImg.GetWidth(); i++)
        for (int j = 0; j < inactiveImg.GetHeight(); j++)
            inactiveImg.SetAlpha(i, j, inactiveImg.GetAlpha(i, j) * 0.5);

    dropArrowInactive_ = wxBitmap(inactiveImg);
#else
    // Begin the image with a transparent background
    wxImage img(btnSize);
    img.InitAlpha();
    unsigned char* alpha = img.GetAlpha();
    memset(alpha, wxIMAGE_ALPHA_TRANSPARENT, img.GetWidth() * img.GetHeight());
    dropArrow_ = wxBitmap(img);
    // On Linux wxDC works fine with an alpha channel, so use a wxMemoryDC to draw right into the bitmap
    wxMemoryDC memDC;
    memDC.SelectObject(dropArrow_);

#ifdef __WXGTK__
    // On Linux just draw a drop arrow bitmap for the normal bitmap button
    btn_dropdown_ = new wxBitmapButton(this, wxID_ANY, wxNullBitmap, wxDefaultPosition, btnSize, wxBORDER_DEFAULT, wxDefaultValidator, "btn_dropdown_");

    memDC.SetPen(wxPen(btn_dropdown_->GetForegroundColour()));
    memDC.SetBrush(wxBrush(btn_dropdown_->GetForegroundColour()));

    wxRect rect(btnSize);
    rect.width *= 0.75;
    rect.height *= 0.75;
    rect.x += rect.width / 5;
    rect.y += rect.height / 5 - 1;

    wxRendererNative::GetGeneric().DrawDropArrow(this, memDC, rect);
    dropArrowInactive_ = dropArrow_;

#else
    btn_dropdown_ = new wxBitmapButton(this, wxID_ANY, wxNullBitmap, wxDefaultPosition, btnSize, wxBORDER_NONE, wxDefaultValidator, "btn_dropdown_");
    dropArrowInactive_ = wxBitmap(img);

    // Use the native renderer to draw a combobox. We can't use the text control portion due to macOS dark mode bug
    // so we can isolate the button instead
    wxRect rect(wxSize(textCtrl_->GetSize().GetWidth(), panelHeight_));
    rect.x -= textCtrl_->GetSize().GetWidth() - panelHeight_ + 1;
    rect.y -= 2;
    wxRendererNative::Get().DrawComboBox(this, memDC, rect);

    // macOS button has rounded corners but wxBitmap has square, so need to cover the overhang
    wxRect borderRect(btnSize);
    borderRect.Inflate(1);
    memDC.SetPen(wxPen(bgColorEnabled_));
    memDC.SetBrush(wxBrush(*wxTRANSPARENT_BRUSH));
    memDC.DrawRoundedRectangle(borderRect, 4);

    // Inactive arrow
    memDC.SelectObject(dropArrowInactive_);
    wxRendererNative::Get().DrawComboBox(this, memDC, rect, wxCONTROL_DISABLED);
    memDC.DrawRoundedRectangle(borderRect, 4);
#endif
    memDC.SelectObject(wxNullBitmap);
    btn_dropdown_->SetBitmap(dropArrow_);
#endif
}

void mmTagTextCtrl::OnMouseCaptureChange(wxMouseEvent& event)
{
    textCtrl_->Refresh(false);
    event.Skip();
}

void mmTagTextCtrl::OnFocusChange(wxFocusEvent& event)
{
    textCtrl_->Refresh(false);
    event.Skip();
}

void mmTagTextCtrl::OnDropDown(wxCommandEvent& )
{
#ifndef __WXMAC__    
    if (!popupWindow_->dismissedByButton_)
    {
        Validate();
        wxPoint pos = ClientToScreen(textCtrl_->GetPosition());
        pos.y += (textCtrl_->GetSize().GetHeight() + panelHeight_) / 2;
        pos.x -= textOffset_;
        popupWindow_->Position(pos, wxSize(0, 0));
        popupWindow_->SetSize(GetSize().GetWidth(), -1);
        tagCheckListBox_->GetParent()->SetSize(popupWindow_->GetSize());
        tagCheckListBox_->SetSize(popupWindow_->GetSize());
        for (unsigned int i = 0; i < tagCheckListBox_->GetCount(); i++)
        {
            if (tags_.find(tagCheckListBox_->GetString(i)) != tags_.end())
                tagCheckListBox_->Check(i);
            else
                tagCheckListBox_->Check(i, false);
        }

        popupWindow_->Popup();
        tagCheckListBox_->SetFocus();
    }
    else
        popupWindow_->dismissedByButton_ = false;
#else
    // mac code has a bug where the tag dropdown doesn't process clicks over underlying controls.
    // just show the tag dialog instead until the issue is fixed.
    wxKeyEvent evt(wxEVT_CHAR_HOOK);
    evt.SetEventObject(textCtrl_);
    evt.m_keyCode = WXK_RETURN;
    OnKeyPressed(evt);
#endif
}

void mmTagTextCtrl::OnKeyPressed(wxKeyEvent& event)
{
    int keyCode = event.GetKeyCode();
    if (keyCode == WXK_RETURN || keyCode == WXK_NUMPAD_ENTER)
    {
        int ip = textCtrl_->GetInsertionPoint();
        if (textCtrl_->GetText().IsEmpty() || ip == 0 || textCtrl_->GetTextRange(ip - 1, ip) == " ")
        {
            mmTagDialog dlg(this, true, parseTags(textCtrl_->GetText()));
            if (dlg.ShowModal() == wxID_OK)
            {
                wxString selection;
                for (const auto& tag : dlg.getSelectedTags())
                    selection.Append(tag + " ");
                textCtrl_->SetText(selection);
                textCtrl_->GotoPos(textCtrl_->GetLastPosition());
            }
            if (dlg.getRefreshRequested())
                init();
        }
        else if (textCtrl_->AutoCompActive())
        {
            textCtrl_->AutoCompComplete();
        }

        Validate();
        return;
    }
    else if (keyCode == WXK_TAB && !event.AltDown())
    {
        bool prev = event.ShiftDown();
        wxWindow* next_control = prev ? GetPrevSibling() : GetNextSibling();
        while (next_control && !next_control->IsFocusable())
            next_control = prev ? next_control->GetPrevSibling() : next_control->GetNextSibling();
        if (next_control)
            next_control->SetFocus();
        return;
    }
    else if (keyCode == WXK_SPACE)
    {
        textCtrl_->AutoCompCancel();
        textCtrl_->InsertText(textCtrl_->GetInsertionPoint(), " ");
        Validate();
        return;
    }
    event.Skip();
}

void mmTagTextCtrl::init()
{
    // Initialize the tag map and dropdown checkboxes
    tag_map_.clear();
    tagCheckListBox_->Clear();
    for (const auto& tag : Model_Tag::instance().all(DB_Table_TAG_V1::COL_TAGNAME))
    {
        tag_map_[tag.TAGNAME] = tag.TAGID;
        tagCheckListBox_->Append(tag.TAGNAME);
    }

    tagCheckListBox_->Fit();
}

void mmTagTextCtrl::OnTextChanged(wxKeyEvent& event)
{
    int keyCode = event.GetUnicodeKey();
    if (!event.AltDown() && keyCode > 32 && keyCode < 127)
    {

        int ip = textCtrl_->GetInsertionPoint();
        int anchor = textCtrl_->GetAnchor();
        int tag_start = textCtrl_->WordStartPosition(std::min(anchor, ip), true);
        int tag_end = textCtrl_->WordEndPosition(std::max(anchor, ip), true);

        // Show autocomplete
        wxString pattern = textCtrl_->GetText().Mid(tag_start, std::min(ip, anchor) - tag_start).Append(event.GetUnicodeKey());
        autocomplete_string_.Clear();
        for (const auto& tag : tag_map_)
        {
            if (tag.first.Lower().Matches(pattern.Lower().Prepend("*").Append("*")))
                autocomplete_string_.Append(tag.first + " ");
        }
        if (!autocomplete_string_.IsEmpty())
        {
            textCtrl_->AutoCompShow(tag_end - tag_start, autocomplete_string_);
        }
        else
            textCtrl_->AutoCompCancel();
    }
    event.Skip();
}

void mmTagTextCtrl::OnPopupCheckboxSelected(wxCommandEvent& event)
{
    // When the user clicks in item in the checkbox update the textCtrl
    wxString selectedText = event.GetString();
    // If they checked a box, append to the string
    if (tagCheckListBox_->IsChecked(event.GetSelection()))
        textCtrl_->SetText(textCtrl_->GetText().Trim() + " " + selectedText +  " ");
    else
    {
        // If they unchecked a box, remove the tag from the string.
        int pos = 0;
        while (pos <= textCtrl_->GetLastPosition())
        {
            pos = textCtrl_->FindText(pos, textCtrl_->GetLastPosition(), selectedText);

            if (pos == wxNOT_FOUND)
                break;

            pos = textCtrl_->WordStartPosition(pos, true);
            int end = textCtrl_->WordEndPosition(pos, true);
            if (textCtrl_->GetTextRange(pos, end) == selectedText)
            {
                textCtrl_->Remove(pos, end);
                break;
            }
            pos = end + 1;
        }
    }
    Validate();
}

void mmTagTextCtrl::OnPaste(wxStyledTextEvent& event)
{
    wxString currText = textCtrl_->GetText();
    Validate(currText.insert(textCtrl_->GetInsertionPoint(), event.GetString()));
    event.SetString("");
}

void mmTagTextCtrl::OnKillFocus(wxFocusEvent& event)
{
    textCtrl_->AutoCompCancel();
    // Remove any non-tags
    Validate();
    wxString tagString;
    wxArrayString tags = parseTags(textCtrl_->GetText());
    for (const auto& tag : tags)
        if (tags_.find(tag) != tags_.end()
            || (operatorAllowed_ && (tag == "&" || tag == "|")))
            tagString.Append(tag + " ");

    textCtrl_->SetText(tagString.Trim());
    event.Skip();
}

bool mmTagTextCtrl::Enable(bool enable)
{
    if (enable == textCtrl_->IsEnabled())
        return false;

    wxPanel::Enable(enable);
    textCtrl_->Enable(enable);
    btn_dropdown_->Enable(enable);

    if (enable)
    {
        textCtrl_->StyleSetBackground(wxSTC_STYLE_DEFAULT, bgColorEnabled_);
        SetBackgroundColour(bgColorEnabled_);
        btn_dropdown_->SetBitmap(dropArrow_);
    }
    else
    {
        textCtrl_->StyleSetBackground(wxSTC_STYLE_DEFAULT, bgColorDisabled_);
        SetBackgroundColour(bgColorDisabled_);
        btn_dropdown_->SetBitmap(dropArrowInactive_);
    }

    return true;
}

void mmTagTextCtrl::OnPaint(wxPaintEvent& event)
{
    int end = textCtrl_->GetTextLength();
    int position = 0;

    // Reset the text style
    textCtrl_->ClearDocumentStyle();

    while (position < end)
    {
        // Find start and end of word
        int wordStart = textCtrl_->WordStartPosition(position, true);
        int wordEnd = textCtrl_->WordEndPosition(position, true);
        wxString word = textCtrl_->GetTextRange(wordStart, wordEnd);

        auto it = tag_map_.find(word);
        // If the word is a valid tag, color it
        if (it != tag_map_.end())
        {
            textCtrl_->StartStyling(wordStart);
            textCtrl_->SetStyling(wordEnd - wordStart, 1);
        }

        position = wordEnd + 1;
    }
    
#ifndef __WXMAC__
    // paint a TextCtrl over the background -- not currently used on macOS due to dark mode bug
    wxWindowDC dc(this);
    wxRegion clipRegion(GetClientRect());
    if (btn_dropdown_->GetClientRect().Contains(btn_dropdown_->ScreenToClient(wxGetMousePosition())))
    {
        wxRect btnRect(btn_dropdown_->GetClientRect());
        btnRect.x += textCtrl_->GetSize().GetWidth() + textOffset_;
        btnRect.y = 1;
        btnRect.height -= 2;
        btnRect.width -= 1;
        clipRegion.Subtract(btnRect); // Exclude the button face (minus top, right, and bottom borders)
    }
    dc.SetDeviceClippingRegion(clipRegion);
    wxRendererNative::Get().DrawTextCtrl(this, dc, GetClientRect(), textCtrl_->IsEnabled() ? wxCONTROL_NONE : wxCONTROL_DISABLED);
#endif

#ifdef __WXMSW__
    dc.DrawBitmap(textCtrl_->IsEnabled() ? dropArrow_ : dropArrowInactive_, wxPoint(textCtrl_->GetSize().GetWidth() + textOffset_, 0));

    dc.DestroyClippingRegion();

    borderColor_ = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWFRAME);
    if (!textCtrl_->IsEnabled())
        borderColor_ = wxSystemSettings::GetColour(wxSYS_COLOUR_SCROLLBAR);
    else if (textCtrl_->HasFocus() || btn_dropdown_->HasFocus() || popupWindow_->IsShown())
        borderColor_ = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
    else if (GetClientRect().Contains(ScreenToClient(wxGetMousePosition())))
        borderColor_ = *wxBLACK;

    dc.SetPen(borderColor_);

    wxRect rect(GetClientRect());

    wxPoint pt[5]{
        wxPoint(0, 0),
        wxPoint(rect.width - 1, 0),
        wxPoint(rect.width -1, rect.height - 1),
        wxPoint(0, rect.height - 1),
        wxPoint(0, 0)
    };

    dc.DrawLines(5, pt);

    if (!initialRefreshDone_)
    {
        btn_dropdown_->Refresh();
        initialRefreshDone_ = true;
    }

#endif

    event.Skip();
}

void mmTagTextCtrl::OnPaintButton(wxPaintEvent& )
{
    wxPaintDC dc(btn_dropdown_);

    wxRect rect = btn_dropdown_->GetClientRect();
    
    // Figure out what style the button needs
    long style = wxCONTROL_NONE;
    if (popupWindow_->IsShown())
        style = wxCONTROL_PRESSED;
    else if (btn_dropdown_->IsEnabled() && rect.Contains(btn_dropdown_->ScreenToClient(wxGetMousePosition())))
        style = wxCONTROL_CURRENT;

    if(style != wxCONTROL_NONE)
        wxRendererNative::Get().DrawComboBoxDropButton(this, dc, rect, style);
    else
    {
        // If we aren't interacting with the button, draw the drop arrow
        // directly on the texctrl like the native combobox
        dc.DrawBitmap(btn_dropdown_->IsEnabled() ? dropArrow_ : dropArrowInactive_, wxPoint(0, 0));
    }

    // Redraw the top, right, and bottom borders to match the window border
    if (style != wxCONTROL_CURRENT && style != wxCONTROL_PRESSED)
    {
        dc.SetPen(borderColor_);
        dc.DrawLine(rect.x, 0, rect.x + rect.width, 0);
        dc.DrawLine(rect.x + rect.width - 1, 0, rect.x + rect.width - 1, rect.height);
        dc.DrawLine(rect.x + rect.width - 1, rect.height - 1, rect.x, rect.height - 1);
    }
}

/* Validates all tags passed in tagText, or the contents of the text control if tagText is blank */
bool mmTagTextCtrl::Validate(const wxString& tagText)
{
    // Clear stored tags
    tags_.clear();

    int ip = textCtrl_->GetInsertionPoint();

    // If we are passed a string validate it, otherwise validate the text control contents
    wxString tags_in = tagText;
    if (tags_in.IsEmpty())
        tags_in = textCtrl_->GetText();

    if (tags_in.IsEmpty()) return true;

    textCtrl_->SetEvtHandlerEnabled(false);
    wxString tags_out;
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
                Model_Tag::Data* newTag = Model_Tag::instance().create();
                newTag->TAGNAME = tag;
                newTag->ACTIVE = 1;
                Model_Tag::instance().save(newTag);
                // Save the new tag to reference
                tag_map_[tag] = newTag->TAGID;
                tagCheckListBox_->Append(tag);

                // Generate an event to tell the parent that a new tag has been added
                // This is necessary for dialogs that contain multiple tag controls (e.g. split transaction)
                // since the parent must reinitialize other tag controls to include the new tag
                wxListEvent evt(wxEVT_COMMAND_LIST_INSERT_ITEM);
                evt.SetId(GetId());
                GetEventHandler()->AddPendingEvent(evt);
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
    textCtrl_->SetText(tags_out);
    textCtrl_->GotoPos(textCtrl_->WordEndPosition(ip, true) + 1);
    textCtrl_->SetEvtHandlerEnabled(true);
    return is_valid;
}

/* Return a list of tag IDs contained in the control */
const wxArrayInt64 mmTagTextCtrl::GetTagIDs() const
{
    wxArrayInt64 tags_out;
    for (const auto& tag : tags_)
        tags_out.push_back(tag.second);

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
            if (operatorAllowed_) tags.push_back(token);
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
            tags.push_back((*it).first);
        else
            tags.push_back(token);
    }

    return tags;
}

void mmTagTextCtrl::SetTags(const wxArrayInt64& tagIds)
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

    textCtrl_->SetText(tagString);
}
