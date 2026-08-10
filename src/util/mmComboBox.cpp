/*******************************************************
 Copyright (C) 2014 Gabriele-V
 Copyright (C) 2015, 2016, 2020, 2022 Nikolay Akimov
 Copyright (C) 2022, 2023 Mark Whalley (mark@ipx.co.uk)
 Copyright (C) 2025 Klaus Wich

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

#include "mmComboBox.h"

#include "base/_constants.h"
#include "_util.h"
#include "model/CurrencyModel.h"
#include "model/AccountModel.h"
#include "model/CategoryModel.h"
#include "model/PayeeModel.h"

wxBEGIN_EVENT_TABLE(mmComboBox, wxComboBox)
    EVT_SET_FOCUS(                  mmComboBox::onSetFocus)
    EVT_COMBOBOX_DROPDOWN(wxID_ANY, mmComboBox::onDropDown)
    EVT_TEXT(wxID_ANY,              mmComboBox::onTextUpdated)
wxEND_EVENT_TABLE()

mmComboBox::mmComboBox(
    wxWindow* parent_win,
    wxWindowID win_id,
    wxSize size
) :
    wxComboBox(parent_win, win_id, "", wxDefaultPosition, size),
    m_is_initialized(false)
{
    Bind(wxEVT_CHAR_HOOK, &mmComboBox::onKeyPressed, this);
    Bind(wxEVT_CHAR,      &mmComboBox::onKeyPressed, this);
    mmThemeAutoColour(this);
}

bool mmComboBox::mmIsValid() const
{
    return (m_name_id_m.count(GetValue()) == 1);
}

int64 mmComboBox::mmGetId() const
{
    auto text = GetValue();
    if (m_name_id_m.count(text) == 1)
        return m_name_id_m.at(text);
    else
        return -1;
}

void mmComboBox::mmSetId(int64 id)
{
    auto result = std::find_if(m_name_id_m.begin(), m_name_id_m.end(),
        [id](const std::pair<wxString, int64>& name_id) {
            return name_id.second == id;
        }
    );

    if (result != m_name_id_m.end())
        ChangeValue(result->first);
}

const wxString mmComboBox::mmGetPattern() const
{
    auto value = GetValue();
    if (m_name_id_m.find(value) == m_name_id_m.end())
        return value;

    wxString buffer;
    for (const wxString& c : value) {
        if (wxString(R"(.^$*+?()[{\|)").Contains(c)) {
            // editor match "}"
            buffer += R"(\)";
        }
        buffer += c;
    }
    return buffer;
}

void mmComboBox::mmDoReInitialize()
{
    Clear();
    init();
    m_is_initialized = false;
    wxFocusEvent evt(wxEVT_SET_FOCUS);
    onSetFocus(evt);
}

void mmComboBox::onSetFocus(wxFocusEvent& event)
{
   if (!m_is_initialized) {
      wxArrayString auto_complete;
      for (const auto& item : m_name_id_m) {
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
       m_is_initialized = true;
   }
   event.Skip();
}

void mmComboBox::onDropDown(wxCommandEvent&)
{
    wxFocusEvent evt;
    onSetFocus(evt);
}

void mmComboBox::onTextUpdated(wxCommandEvent& event)
{
    this->SetEvtHandlerEnabled(false);
    const auto& typedText = event.GetString();
#if defined (__WXMAC__)
    // Filtering the combobox as the user types because on Mac autocomplete function doesn't work
    // PLEASE DO NOT REMOVE!!
    if ((m_is_initialized) && (typedText.IsEmpty() || (this->GetSelection() == -1))) {
        this->Clear();

        for (auto& entry : m_name_id_m) {
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
    for (const auto& item : m_name_id_m) {
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

void mmComboBox::onKeyPressed(wxKeyEvent& event)
{
    auto text = GetValue();
    if (event.GetKeyCode() == WXK_RETURN) {
        for (const auto& item : m_name_id_m) {
            if (item.first.CmpNoCase(text) == 0) {
                SetValue(item.first);
                Dismiss();
                break;
            }
        }
        event.Skip();
    }
    else if (event.GetId() == mmID_CATEGORY && event.GetUnicodeKey() == ':') {
        this->SetEvtHandlerEnabled(false);
        ChangeValue(text.Trim().Append(InfoModel::instance().getString("CATEG_DELIMITER", ":")));
        SetInsertionPointEnd();
        this->SetEvtHandlerEnabled(true);
    }
    else {
        event.Skip();
    }
}

mmComboBoxCustom::mmComboBoxCustom(
    wxWindow* parent_win,
    wxArrayString& name_a,
    wxWindowID win_id,
    wxSize size
) :
    mmComboBox(parent_win, win_id, size)
{
    // note: m_name_id_m is a map from name to index (not to data id)
    int i = 0;
    for (const wxString& name : name_a) {
        m_name_id_m[name] = i++;
    }
}

mmComboBoxCurrency::mmComboBoxCurrency(
    wxWindow* parent_win,
    wxWindowID id,
    wxSize size
) :
    mmComboBox(parent_win, id, size)
{
    init();
    wxArrayString name_a;
    for (const auto& name_id : m_name_id_m) {
        name_a.Add(name_id.first);
    }
    parent_win->SetEvtHandlerEnabled(false);
    Set(name_a);
    parent_win->SetEvtHandlerEnabled(true);
}

void mmComboBoxCurrency::init()
{
    m_name_id_m = CurrencyModel::instance().find_all_name_id_m();
}

// account_id: always include this account (even if it is closed)
// only_open: exlude closed accounts (other than account_id)
mmComboBoxAccount::mmComboBoxAccount(
    wxWindow* parent_win,
    wxWindowID win_id,
    wxSize size,
    int64 account_id,
    bool only_open
) :
    mmComboBox(parent_win, win_id, size),
    m_account_id(account_id),
    m_only_open(only_open)
{
    init();
    wxArrayString name_a;
    for (const auto& name_id : m_name_id_m) {
        name_a.Add(name_id.first);
    }
    parent_win->SetEvtHandlerEnabled(false);
    Set(name_a);
    parent_win->SetEvtHandlerEnabled(true);
}

void mmComboBoxAccount::init()
{
    m_name_id_m = AccountModel::instance().find_all_name_id_m(m_only_open);
    if (m_account_id > -1) {
        const wxString account_name = AccountModel::instance().get_id_name(m_account_id);
        m_name_id_m[account_name] = m_account_id;
    }
}

// cat_id: always include this category (even if it is inactive)
// only_active: exclude inactive categories (other than cat_id)
mmComboBoxCategory::mmComboBoxCategory(
    wxWindow* parent_win,
    wxWindowID win_id,
    wxSize size,
    int64 cat_id,
    bool only_active,
    bool fillComboBox
) :
    mmComboBox(parent_win, win_id, size),
    m_cat_id(cat_id),
    m_only_active(only_active)
{
    if (!fillComboBox)
        return;
    
    wxLogDebug("mmComboBoxCategory::mmComboBoxCategory: fillComboBox = true");
    init();
    wxArrayString name_a;
    for (const auto& name_id : m_name_id_m) {
        name_a.Add(name_id.first);
    }
    parent_win->SetEvtHandlerEnabled(false);
    Set(name_a);
    parent_win->SetEvtHandlerEnabled(true);
}

void mmComboBoxCategory::init()
{
    m_name_id_m.clear();
    m_fullname_id_m = CategoryModel::instance().find_all_id_mFullname(m_only_active);
    if (m_cat_id > -1) {
        m_fullname_id_m.insert(std::make_pair(
            CategoryModel::instance().get_id_fullname(m_cat_id),
            m_cat_id
        ));
    }

    // note: m_name_id_m is a map from fullname to index (not to cat_id)
    int i = 0;
    for (const auto& fullname_id : m_fullname_id_m) {
        m_name_id_m[fullname_id.first] = i++;
    }
}

int64 mmComboBoxCategory::mmGetCategoryId() const
{
    const wxString text = GetValue();
    if (m_fullname_id_m.count(text) == 1)
        return m_fullname_id_m.at(text);
    else
        return -1;
}

// payee_id: always include this payee (even if it is inactive)
// only_active: exclude inactive payees (other than payee_id)
mmComboBoxPayee::mmComboBoxPayee(
    wxWindow* parent_win,
    wxWindowID win_id,
    wxSize size,
    int64 payee_id,
    bool only_active
) :
    mmComboBox(parent_win, win_id, size),
    m_payee_id(payee_id),
    m_only_active(only_active)
{
    init();
    wxArrayString name_a;
    for (const auto& name_id : m_name_id_m) {
        name_a.Add(name_id.first);
    }
    parent_win->SetEvtHandlerEnabled(false);
    Set(name_a);
    parent_win->SetEvtHandlerEnabled(true);
}

void mmComboBoxPayee::init()
{
    m_name_id_m = PayeeModel::instance().find_all_name_id_m(m_only_active);
    if (m_payee_id > -1) {
        const wxString payee_name = PayeeModel::instance().get_id_name(m_payee_id);
        m_name_id_m[payee_name] = m_payee_id;
    }
}

mmComboBoxUsedPayee::mmComboBoxUsedPayee(
    wxWindow* parent_win,
    wxWindowID win_id,
    wxSize size
) :
    mmComboBox(parent_win, win_id, size)
{
    init();
    wxArrayString name_a;
    for (const auto& name_id : m_name_id_m) {
        name_a.Add(name_id.first);
    }
    parent_win->SetEvtHandlerEnabled(false);
    Set(name_a);
    parent_win->SetEvtHandlerEnabled(true);
}

void mmComboBoxUsedPayee::init()
{
    std::set<int64> payee_id_m = PayeeModel::instance().find_used_id_m();
    m_name_id_m.clear();
    for (int64 payee_id : payee_id_m) {
        const wxString payee_name = PayeeModel::instance().get_id_name(payee_id);
        m_name_id_m[payee_name] = payee_id;
    }
}
