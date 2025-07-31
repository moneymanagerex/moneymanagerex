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

#ifndef MM_EX_MMSIMPLEDIALOGS_H_
#define MM_EX_MMSIMPLEDIALOGS_H_

#include "mmex.h"
#include "mmTextCtrl.h"
#include "util.h"

#include "model/Model_Account.h"

#include <wx/choicdlg.h>
#include <wx/spinbutt.h>
#include <wx/dialog.h>
#include <wx/display.h>
#include <wx/choice.h>
#include <wx/stc/stc.h>
#include <wx/timectrl.h>

class wxComboBox;
class wxTextCtrl;
class wxChoice;
class wxButton;

class mmCalculatorPopup : public wxPopupTransientWindow
{
public:
    mmCalculatorPopup(wxWindow* parent, mmTextCtrl* target = nullptr);
    virtual ~mmCalculatorPopup();

    void SetValue(wxString& value);
    void SetFocus() override;
    void SetTarget(mmTextCtrl* target);

    virtual void Popup(wxWindow* focus = NULL) override
    {
        if (dismissedByButton_ == false)
        {
            wxPoint pt = GetParent()->GetScreenPosition();
            wxRect displayRect = wxDisplay(wxDisplay::GetFromPoint(pt)).GetGeometry();

            int x = std::min(pt.x, displayRect.GetRight() - GetSize().GetWidth());
            int y = std::min(pt.y + GetParent()->GetSize().GetHeight(), displayRect.GetBottom() - GetSize().GetHeight());
            SetPosition(wxPoint(x, y));
            valueTextCtrl_->SetValue(target_->GetValue());
            target_->Enable(false);
            wxPopupTransientWindow::Popup(focus);
        }
        else dismissedByButton_ = false;
    }

protected:
    virtual void OnDismiss() override
    {
#ifdef __WXMSW__
        // On MSW check if the button was used to dismiss to prevent the popup from reopening
        wxPoint mousePos = wxGetMousePosition();
        if (GetParent()->GetClientRect().Contains(GetParent()->ScreenToClient(mousePos)))
        {
            dismissedByButton_ = true;
        }
        else
            dismissedByButton_ = false;
#endif
        if (target_)
        {
            valueTextCtrl_->Calculate();
            target_->ChangeValue(valueTextCtrl_->GetValue());
            target_->Enable(true);
            target_->SetFocus();
        }
    }

private:
    bool dismissedByButton_ = false;
    mmTextCtrl* target_;
    mmTextCtrl* valueTextCtrl_ = nullptr;
    wxButton* button_lparen_ = nullptr;
    wxButton* button_rparen_ = nullptr;
    wxButton* button_clear_ = nullptr;
    wxButton* button_del_ = nullptr;
    wxButton* button_7_ = nullptr;
    wxButton* button_8_ = nullptr;
    wxButton* button_9_ = nullptr;
    wxButton* button_div_ = nullptr;
    wxButton* button_4_ = nullptr;
    wxButton* button_5_ = nullptr;
    wxButton* button_6_ = nullptr;
    wxButton* button_mult_ = nullptr;
    wxButton* button_1_ = nullptr;
    wxButton* button_2_ = nullptr;
    wxButton* button_3_ = nullptr;
    wxButton* button_minus_ = nullptr;
    wxButton* button_dec_ = nullptr;
    wxButton* button_0_ = nullptr;
    wxButton* button_equal_ = nullptr;
    wxButton* button_plus_ = nullptr;
    ;
    void OnButtonPressed(wxCommandEvent& event);
    enum Buttons
    {
        mmID_MULTIPLY = wxID_HIGHEST,
        mmID_DIVIDE,
        mmID_DELETE
    };
};

inline void mmCalculatorPopup::SetFocus() { valueTextCtrl_->SetFocus(); }
inline void mmCalculatorPopup::SetTarget(mmTextCtrl* target) { target_ = target; button_dec_->SetLabel(target->GetDecimalPoint()); }


class mmComboBox : public wxComboBox
{
public:
    mmComboBox(wxWindow* parent
        , wxWindowID id = wxID_ANY
        , wxSize size = wxDefaultSize
    );
    void mmSetId(int64 id);
    int64 mmGetId() const;
    const wxString mmGetPattern() const;
    bool mmIsValid() const;
    void mmDoReInitialize();
protected:
    void OnTextUpdated(wxCommandEvent& event);
    void OnSetFocus(wxFocusEvent& event);
    void OnDropDown(wxCommandEvent&);
    void OnKeyPressed(wxKeyEvent& event);
    virtual void init() = 0;
    std::map<wxString, int64> all_elements_;
private:
    bool is_initialized_;
    wxDECLARE_EVENT_TABLE();
};

class mmComboBoxAccount : public mmComboBox
{
public:
    mmComboBoxAccount(wxWindow* parent
        , wxWindowID id = wxID_ANY
        , wxSize size = wxDefaultSize
        , int64 accountID = -1
        , bool excludeClosed = true
    );
protected:
    void init();
private:
    int64 accountID_ = -1;
    bool excludeClosed_ = true;
};

/* -------------------------------------------- */

class mmComboBoxPayee : public mmComboBox
{
public:
    mmComboBoxPayee(wxWindow* parent
        , wxWindowID id = wxID_ANY
        , wxSize size = wxDefaultSize
        , int64 payeeID = -1
        , bool excludeHidden = false
    );
protected:
    void init();
private:
    int64 payeeID_;
    bool excludeHidden_;
};

class mmComboBoxUsedPayee : public mmComboBox
{
public:
    mmComboBoxUsedPayee(wxWindow* parent
        , wxWindowID id = wxID_ANY
        , wxSize size = wxDefaultSize
    );
protected:
    void init();
};

/* -------------------------------------------- */

class mmComboBoxCurrency : public mmComboBox
{
public:
    mmComboBoxCurrency(wxWindow* parent
        , wxWindowID id = wxID_ANY
        , wxSize size = wxDefaultSize
    );
protected:
    void init();
};

/* -------------------------------------------- */

class mmComboBoxCategory : public mmComboBox
{
public:
    mmComboBoxCategory(wxWindow* parent
        , wxWindowID id = wxID_ANY
        , wxSize size = wxDefaultSize
        , int64 catID = -1
        , bool excludeInactive = false
    );
    int64 mmGetCategoryId() const;
protected:
    void init();
private:
    int64 catID_;
    bool excludeHidden_;
    std::map<wxString, int64> all_categories_;
};

/* -------------------------------------------- */

class mmComboBoxCustom : public mmComboBox
{
public:
    mmComboBoxCustom(wxWindow* parent
        , wxArrayString& a
        , wxWindowID id = wxID_ANY
        , wxSize size = wxDefaultSize
    );
protected:
    void init();
};
inline void mmComboBoxCustom::init() {}

/* -------------------------------------------- */

class mmDatePickerCtrl : public wxPanel
{

public:
    mmDatePickerCtrl(wxWindow* parent, wxWindowID id
        , wxDateTime dt=wxDateTime::Today(), wxPoint pos=wxDefaultPosition, wxSize size=wxDefaultSize
        , long style=wxDP_DROPDOWN | wxDP_SHOWCENTURY);
    ~mmDatePickerCtrl();
    void SetValue(const wxDateTime &dt);    // Override
    bool Enable(bool state=true);           // Override
    wxBoxSizer* mmGetLayout(bool showTimeCtrl = true);
    wxBoxSizer* mmGetLayoutWithTime();
    wxDateTime GetValue();
    bool isItMyDateControl(wxObject* obj);

private:
    wxStaticText* getTextWeek();
    wxSpinButton* getSpinButton();
    wxDateTime dt_;
    wxDatePickerCtrl* datePicker_ = nullptr;
    wxTimePickerCtrl* timePicker_ = nullptr;
    void OnDateChanged(wxDateEvent& event);
    void OnDateSpin(wxSpinEvent&);

    wxWindow* parent_ = nullptr;
    wxStaticText* itemStaticTextWeek_ = nullptr;
    wxSpinButton* spinButton_ = nullptr;
};

inline wxDateTime mmDatePickerCtrl::GetValue() { return dt_; }

/* -------------------------------------------- */

class mmColorButton : public wxButton
{
public:
    mmColorButton(wxWindow* parent
        , wxWindowID id
        , wxSize size = wxDefaultSize
    );
    void SetBackgroundColor(int color_id);
    int GetColorId() const;
private:
    void OnMenuSelected(wxCommandEvent& event);
    void OnColourButton(wxCommandEvent& event);
    int m_color_value;
    wxDECLARE_EVENT_TABLE();
};

class mmChoiceAmountMask : public wxChoice
{
public:
    mmChoiceAmountMask(wxWindow* parent, wxWindowID id);
    virtual void SetDecimalChar(const wxString& str);
};

class mmSingleChoiceDialog : public wxSingleChoiceDialog
{
public:
    using wxSingleChoiceDialog::ShowModal;

    mmSingleChoiceDialog();
    mmSingleChoiceDialog(wxWindow *parent, const wxString& message,
        const wxString& caption, const wxArrayString& choices);
    mmSingleChoiceDialog(wxWindow* parent, const wxString& message,
        const wxString& caption, const Model_Account::Data_Set& accounts);
    int ShowModal()
    {
        return wxSingleChoiceDialog::ShowModal();
    }
};

class mmDialogComboBoxAutocomplete : public wxDialog
{
public:
    mmDialogComboBoxAutocomplete();
    mmDialogComboBoxAutocomplete(wxWindow *parent, const wxString& message, const wxString& caption,
        const wxString& defaultText, const wxArrayString& choices);

    const wxString getText() const;

private:
    bool Create(wxWindow* parent
        , wxWindowID id = wxID_ANY
        , const wxString& caption = ""
        , const wxPoint& pos = wxDefaultPosition
        , const wxSize& size = wxDefaultSize
        , long style = wxCAPTION | wxRESIZE_BORDER | wxCLOSE_BOX);
    wxString m_default_str;
    wxArrayString m_choices;
    wxString m_message;

    mmComboBoxCustom* cbText_ = nullptr;
};

class mmGUIApp;
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

class mmDateYearMonth : public wxPanel
{
    wxDECLARE_EVENT_TABLE();
public:
    mmDateYearMonth();
    mmDateYearMonth(wxWindow *parent);
    void OnButtonPress(wxCommandEvent& event);

private:
    wxWindow* m_parent = nullptr;
    bool Create(wxWindow* parent, wxWindowID id);
    int m_shift = 0;
};

// -------------------------------------------------------------------------- //

class mmMultiChoiceDialog : public wxMultiChoiceDialog
{
public:
    using wxMultiChoiceDialog::ShowModal;

    mmMultiChoiceDialog();
    mmMultiChoiceDialog(wxWindow* parent, const wxString& message,
        const wxString& caption, const wxArrayString& items);
    int ShowModal();
};
inline  int mmMultiChoiceDialog::ShowModal() {   return wxMultiChoiceDialog::ShowModal(); }

/* -------------------------------------------- */
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
    bool initialRefreshDone_ = false;
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
#endif // MM_EX_MMSIMPLEDIALOGS_H_
