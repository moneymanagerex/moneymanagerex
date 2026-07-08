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

#include "base/_defs.h"
#include <wx/choicdlg.h>
#include <wx/spinbutt.h>
#include <wx/dialog.h>
#include <wx/display.h>
#include <wx/choice.h>
#include <wx/stc/stc.h>
#include <wx/timectrl.h>

#include "mmComboBox.h"
#include "mmTextCtrl.h"
#include "_util.h"
#include "model/AccountModel.h"

class wxComboBox;
class wxTextCtrl;
class wxChoice;
class wxButton;

class mmChoiceAmountMask : public wxChoice
{
public:
    mmChoiceAmountMask(wxWindow* parent, wxWindowID id);
    virtual void SetDecimalChar(const wxString& str);
};

class mmDialogComboBoxAutocomplete : public wxDialog
{
private:
    wxString m_default_str;
    wxArrayString m_choices;
    wxString m_message;

    mmComboBoxCustom* cbText_ = nullptr;

public:
    mmDialogComboBoxAutocomplete();
    mmDialogComboBoxAutocomplete(
        wxWindow *parent,
        const wxString& message,
        const wxString& caption,
        const wxString& defaultText,
        const wxArrayString& choices
    );

private:
    bool Create(
        wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxString& caption = "",
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxCAPTION | wxRESIZE_BORDER | wxCLOSE_BOX
    );

public:
    const wxString getText() const;
};

class mmErrorDialogs
{
public:
    enum TOOL_TIP
    {
        MESSAGE_DROPDOWN_BOX,
        MESSAGE_POPUP_BOX,
    };

    static void MessageInvalid(wxWindow *parent, const wxString &message);
    static void MessageError(wxWindow *parent, const wxString &message, const wxString &title);
    static void MessageWarning(wxWindow *parent, const wxString &message, const wxString &title);
    static void InvalidCategory(wxWindow *button);
    static void InvalidAccount(wxWindow *object, bool transfer = false, TOOL_TIP tm = MESSAGE_DROPDOWN_BOX);
    static void InvalidFile(wxWindow *object, bool open = false);
    static void InvalidPayee(wxWindow *object);
    static void InvalidName(wxTextCtrl *textBox, bool alreadyexist = false);
    static void InvalidSymbol(wxTextCtrl *textBox, bool alreadyexist = false);
    static void ToolTip4Object(wxWindow *object, const wxString &message, const wxString &title, int ico = wxICON_WARNING);
};

// --

class mmTagCtrlPopupWindow : public wxPopupTransientWindow {
public:
    mmTagCtrlPopupWindow(wxWindow* parent, wxWindow* button) : wxPopupTransientWindow(parent, wxPU_CONTAINS_CONTROLS) {
        button_ = button;
    }
    bool dismissedByButton_ = false;
protected:
    virtual void OnDismiss() override {
#ifdef __WXMSW__
        // On MSW check if the button was used to dismiss to prevent the popup from reopening
        wxPoint mousePos = wxGetMousePosition();
        if (button_->GetClientRect().Contains(button_->ScreenToClient(mousePos)))
        {
            dismissedByButton_ = true;
        }
        else
            dismissedByButton_ = false;
#endif
    }

private:
    wxWindow* button_ = nullptr;
};

class mmTagTextCtrl : public wxPanel
{
public:
    mmTagTextCtrl(wxWindow* parent, wxWindowID id = wxID_ANY,
        bool operatorAllowed = false,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize, long style = 0
    );
    bool IsValid();
    bool ValidateTagText(const wxString& tagText = wxEmptyString);
    const wxArrayInt64 GetTagIDs() const;
    const wxArrayString GetTagStrings();
    void Reinitialize();
    void SetTags(const wxArrayInt64& tagIds);
    void SetText(const wxString& text);
    void Clear();
    bool IsEmpty() const;
    bool Enable(bool enable = true) override;

protected:
    void OnTextChanged(wxKeyEvent& event);
    void OnPaste(wxStyledTextEvent& event);
    void OnKillFocus(wxFocusEvent& event);
    void OnPaint(wxPaintEvent& event);
    void OnPaintButton(wxPaintEvent&);
    void OnDropDown(wxCommandEvent&);
    void OnKeyPressed(wxKeyEvent& event);
    void OnPopupCheckboxSelected(wxCommandEvent& event);
    void OnMouseCaptureChange(wxMouseEvent& event);
    void OnFocusChange(wxFocusEvent& event);
private:
    void init();
    void createDropButton(wxSize btnSize);
    wxStyledTextCtrl* textCtrl_ = nullptr;
    wxBitmapButton* btn_dropdown_ = nullptr;
    wxString autocomplete_string_;
    std::map<wxString, int64, caseInsensitiveComparator> tag_map_;
    std::map<wxString, int64, caseInsensitiveComparator> tags_;
    wxArrayString parseTags(const wxString& tagString);
    bool operatorAllowed_;
    mmTagCtrlPopupWindow* popupWindow_ = nullptr;
    wxCheckListBox* tagCheckListBox_ = nullptr;
    wxColour borderColor_ = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWFRAME);
    wxBitmap dropArrow_;
    wxBitmap dropArrowInactive_;
#ifdef __WXMSW__
    bool repaint_allowed_ = true;
    bool initialRefreshDone_ = false;
#endif
    int panelHeight_, textOffset_;
    wxColour bgColorEnabled_ = wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX);
    wxColour bgColorDisabled_ = wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE);
};

inline bool mmTagTextCtrl::IsValid() { return ValidateTagText(); }
inline const wxArrayString mmTagTextCtrl::GetTagStrings() { return parseTags(textCtrl_->GetText()); }
inline void mmTagTextCtrl::Reinitialize() { init(); }
inline void mmTagTextCtrl::SetText(const wxString& text) { textCtrl_->SetText(text); }
inline bool mmTagTextCtrl::IsEmpty() const { return textCtrl_->IsEmpty(); }
inline void mmTagTextCtrl::Clear() { textCtrl_->ClearAll(); }
