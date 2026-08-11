/*******************************************************
 Copyright (C) 2014 Nikolay Akimov
 Copyright (C) 2014 Gabriele-V
 Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)
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

#pragma once

#include <map>
#include "base/_defs.h"
#include "base/_types.h"

class mmComboBox : public wxComboBox
{
    wxDECLARE_EVENT_TABLE();

protected:
    bool m_is_initialized;
    std::map<wxString, int64> m_name_id_m;

public:
    mmComboBox(
        wxWindow* parent_win,
        wxWindowID win_id = wxID_ANY,
        wxSize size = wxDefaultSize
    );

protected:
    virtual void init() = 0;

public:
    bool mmIsValid() const;
    auto mmGetId() const -> int64;
    void mmSetId(int64 id);
    auto mmGetPattern() const -> const wxString;
    void mmDoReInitialize();

protected:
    void onSetFocus(    wxFocusEvent&   event);
    void onDropDown(    wxCommandEvent& );
    void onTextUpdated( wxCommandEvent& event);
    void onKeyPressed(  wxKeyEvent&     event);
};

class mmComboBoxCurrency : public mmComboBox
{
public:
    mmComboBoxCurrency(
        wxWindow* parent_win,
        wxWindowID win_id = wxID_ANY,
        wxSize size = wxDefaultSize
    );

protected:
    virtual void init() override;
};

class mmComboBoxAccount : public mmComboBox
{
private:
    int64 m_account_id = -1;
    bool m_only_open = true;

public:
    mmComboBoxAccount(
        wxWindow* parent_win,
        wxWindowID win_id = wxID_ANY,
        wxSize size = wxDefaultSize,
        int64 account_id = -1,
        bool only_open = true
    );

protected:
    virtual void init() override;
};

class mmComboBoxCategory : public mmComboBox
{
private:
    int64 m_cat_id;
    bool m_only_active;
    std::map<wxString, int64> m_fullname_id_m;

public:
    mmComboBoxCategory(
        wxWindow* parent_win,
        wxWindowID win_id = wxID_ANY,
        wxSize size = wxDefaultSize,
        int64 cat_id = -1,
        bool only_active = false,
        bool fillComboBox = true
    );

protected:
    virtual void init() override;

public:
    int64 mmGetCategoryId() const;
};

class mmComboBoxPayee : public mmComboBox
{
private:
    int64 m_payee_id;
    bool m_only_active;

public:
    mmComboBoxPayee(
        wxWindow* parent_win,
        wxWindowID win_id = wxID_ANY,
        wxSize size = wxDefaultSize,
        int64 payee_id = -1,
        bool only_active = false
    );

protected:
    virtual void init() override;
};

class mmComboBoxUsedPayee : public mmComboBox
{
public:
    mmComboBoxUsedPayee(
        wxWindow* parent_win,
        wxWindowID win_id = wxID_ANY,
        wxSize size = wxDefaultSize
    );

protected:
    virtual void init() override;
};

class mmComboBoxCustom : public mmComboBox
{
public:
    mmComboBoxCustom(
        wxWindow* parent_win,
        wxArrayString& name_a,
        wxWindowID win_id = wxID_ANY,
        wxSize size = wxDefaultSize
    );

protected:
    virtual void init() override {}
};
