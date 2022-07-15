/*******************************************************
Copyright (C) 2014 Nikolay Akimov
Copyright (C) 2014 Gabriele-V
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

#ifndef MM_EX_MMSIMPLEDIALOGS_H_
#define MM_EX_MMSIMPLEDIALOGS_H_

#include "mmex.h"

#include "model/Model_Account.h"

#include <wx/choicdlg.h>
#include <wx/spinbutt.h>
#include <wx/dialog.h>
#include <wx/choice.h>

class wxComboBox;
class wxTextCtrl;
class wxChoice;
class wxButton;

class mmComboBox : public wxComboBox
{
public:
    mmComboBox(wxWindow* parent
        , wxWindowID id = wxID_ANY
        , wxSize size = wxDefaultSize
    );
    void mmSetId(int id);
    int mmGetId() const;
    const wxString mmGetPattern() const;
    bool mmIsValid() const;
    void mmDoReInitialize();
protected:
    void OnTextUpdated(wxCommandEvent& event);
    void OnSetFocus(wxFocusEvent& event);
    void OnDropDown(wxCommandEvent& event);
    void OnKeyPressed(wxKeyEvent& event);
    virtual void init() = 0;
    std::map<wxString, int> all_elements_;
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
        , int accountID = -1
        , bool excludeClosed = true
    );
protected:
    void init();
private:
    int accountID_;
    bool excludeClosed_;
};

/* -------------------------------------------- */

class mmComboBoxPayee : public mmComboBox
{
public:
    mmComboBoxPayee(wxWindow* parent
        , wxWindowID id = wxID_ANY
        , wxSize size = wxDefaultSize
    );
protected:
    void init();
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
    );
    int mmGetCategoryId() const;
    int mmGetSubcategoryId() const;
protected:
    void init();
private:
    std::map<wxString, std::pair<int, int> > all_categories_;
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

class mmDatePickerCtrl : public wxDatePickerCtrl
{
    wxDECLARE_EVENT_TABLE();

public: 
    mmDatePickerCtrl(wxWindow* parent, wxWindowID id
    , wxDateTime dt=wxDateTime::Today(), wxPoint pos=wxDefaultPosition, wxSize size=wxDefaultSize
    , long style=wxDP_DROPDOWN | wxDP_SHOWCENTURY);
    ~mmDatePickerCtrl();
    void SetValue(const wxDateTime &dt);    // Override
    bool Enable(bool state=true);           // Override
    wxBoxSizer* mmGetLayout();
private:
    wxStaticText* getTextWeek();
    wxSpinButton* getSpinButton();

    void OnCalendar(wxMouseEvent& event);
    void OnDateChanged(wxDateEvent& event);
    void OnDateSpin(wxSpinEvent& event);

    wxWindow* parent_;
    wxStaticText* itemStaticTextWeek_;
    wxSpinButton* spinButton_;
};

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

    mmComboBoxCustom* cbText_;
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
    static void InvalidCategory(wxWindow *button, bool simple = true);
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
    wxWindow* m_parent;
    bool Create(wxWindow* parent, wxWindowID id);
    int m_shift;
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

#endif // MM_EX_MMSIMPLEDIALOGS_H_
