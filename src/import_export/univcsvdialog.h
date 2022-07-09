/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel

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

#ifndef MM_EX_UNIVCSVDIALOG_H_
#define MM_EX_UNIVCSVDIALOG_H_

#include <vector>
#include <map>
#include <wx/dialog.h>
#include "Model_Checking.h"
#include "mmSimpleDialogs.h"
class wxSpinCtrl;
class wxSpinEvent;
class wxListBox;
class wxListCtrl;
class wxChoice;
class wxTextCtrl;
class wxStaticBox;
class wxCheckBox;

#define ID_MYDIALOG8 10040
#define SYMBOL_UNIVCSVDIALOG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_UNIVCSVDIALOG_IDNAME ID_MYDIALOG8
#define SYMBOL_UNIVCSVDIALOG_SIZE wxSize(400, 300)
#define SYMBOL_UNIVCSVDIALOG_POSITION wxDefaultPosition
#define ID_LISTBOX 10090
#define ID_PANEL10 10091
#define ID_PANEL_LOGS 10092
#define ID_UNIVCSVBUTTON_IMPORT 10094
#define ID_UNIVCSVBUTTON_EXPORT 10095
#define ID_LISTBOX_CANDICATE 10099
#define wxID_BROWSE 10100
#define wxID_RADIO_BOX 10101
#define wxID_STANDARD 10102
#define wxID_ACCOUNT 10103
#define ID_UD_DELIMIT 10104
#define ID_FILE_NAME 10105
#define ID_DATE_FORMAT 10106
#define ID_ENCODING 10107
#define ID_FIRST_ROW 10108
#define ID_LAST_ROW 10109
#define ID_UD_DECIMAL 10110
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
#define wxFIXED_MINSIZE 0
#endif

class ITransactionsFile;

class mmUnivCSVDialog: public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmUnivCSVDialog);
    wxDECLARE_EVENT_TABLE();

public:
    enum EDialogType
    {
        DIALOG_TYPE_IMPORT_CSV,
        DIALOG_TYPE_EXPORT_CSV,
        DIALOG_TYPE_IMPORT_XML,
        DIALOG_TYPE_EXPORT_XML
    };

    /// Constructors
    mmUnivCSVDialog();
    mmUnivCSVDialog(wxWindow* parent, EDialogType dialogType,
                    wxWindowID id = SYMBOL_UNIVCSVDIALOG_IDNAME,
                    const wxPoint& pos = SYMBOL_UNIVCSVDIALOG_POSITION,
                    const wxSize& size = SYMBOL_UNIVCSVDIALOG_SIZE,
                    long style = SYMBOL_UNIVCSVDIALOG_STYLE);

    bool isImportCompletedSuccessfully() const;
    int ImportedAccountID() const;
    bool IsImporter() const;
    bool IsXML() const;
    bool IsCSV() const;
    wxString GetSettingsPrfix() const;

private:
    wxButton* bImport_;

    enum EUnivCvs
    {
        UNIV_CSV_ID = 0,
        UNIV_CSV_DATE,
        UNIV_CSV_STATUS,
        UNIV_CSV_TYPE,
        UNIV_CSV_ACCOUNT,
        UNIV_CSV_PAYEE,
        UNIV_CSV_AMOUNT,
        UNIV_CSV_CURRENCY,
        UNIV_CSV_CATEGORY,
        UNIV_CSV_SUBCATEGORY,
        UNIV_CSV_TRANSNUM,
        UNIV_CSV_NOTES,
        UNIV_CSV_DONTCARE,
        UNIV_CSV_WITHDRAWAL,
        UNIV_CSV_DEPOSIT,
        UNIV_CSV_BALANCE,
        UNIV_CSV_LAST
    };

private:
    struct tran_holder
    {
        wxDateTime Date;
        wxString Type = Model_Checking::all_type()[Model_Checking::WITHDRAWAL];
        wxString Status = "";
        int ToAccountID = -1;
        double ToAmount = 0.0;
        int PayeeID = -1;
        int CategoryID = -1;
        int SubCategoryID = -1;
        double Amount = 0.0;
        wxString Number;
        wxString Notes;
        bool valid = true;
    };
private:
    EDialogType dialogType_;
    wxString delimit_;
    wxString decimal_;

    std::vector<int> csvFieldOrder_;
    wxListBox* csvFieldCandicate_;
    wxListBox* csvListBox_;

    wxButton* m_button_add_;
    wxButton* m_button_remove_;
    wxChoice* m_choice_account_;
    wxListCtrl* m_list_ctrl_; //preview
    wxTextCtrl* m_text_ctrl_;
    wxTextCtrl* m_setting_name_ctrl_;
    wxTextCtrl* log_field_;
    wxTextCtrl* m_textDelimiter;
    wxSpinCtrl* m_spinIgnoreFirstRows_;
    wxSpinCtrl* m_spinIgnoreLastRows_;

    std::map<int, wxString> CSVFieldName_;
private:
    wxChoice* choiceDateFormat_;
    wxChoice* m_choiceEncoding;
    wxString date_format_;
    mmColorButton* colorButton_;
    wxCheckBox* colorCheckBox_;

    wxChoice* m_choiceAmountFieldSign;
    mmChoiceAmountMask* m_choiceDecimalSeparator;
    enum amountFieldSignValues { PositiveIsDeposit, PositiveIsWithdrawal, DefindByType };
    wxCheckBox* m_checkBoxExportTitles;

    int accountID_;
    bool importSuccessful_;
    bool m_userDefinedDateMask;
    int m_object_in_focus;
    bool m_reverce_sign;
    wxString depositType_;

    /// Creation
    bool Create(wxWindow* parent,
                const wxString& caption,
                wxWindowID id = SYMBOL_UNIVCSVDIALOG_IDNAME,
                const wxPoint& pos = SYMBOL_UNIVCSVDIALOG_POSITION,
                const wxSize& size = SYMBOL_UNIVCSVDIALOG_SIZE,
                long style = SYMBOL_UNIVCSVDIALOG_STYLE);
private:
    /// Creates the controls and sizers
    void CreateControls();
    void OnAdd(wxCommandEvent& event);
    bool validateData(tran_holder & holder);
    void OnImport(wxCommandEvent& event);
    void OnExport(wxCommandEvent& event);
    void OnRemove(wxCommandEvent& event);
    bool isIndexPresent(int index) const;
    const wxString getCSVFieldName(int index) const;
    void parseToken(int index, const wxString& token, tran_holder & holder);
    void OnSettingsSave(wxCommandEvent& event);
    void OnMoveUp(wxCommandEvent& event);
    void OnMoveDown(wxCommandEvent& event);
    void OnStandard(wxCommandEvent& event);
    void OnButtonClearClick(wxCommandEvent& event);
    void OnFileBrowse(wxCommandEvent& event);
    void OnListBox(wxCommandEvent& event);
    void OnDelimiterChange(wxCommandEvent& event);
    void OnDecimalChange(wxCommandEvent& event);
    void OnButtonClear(wxCommandEvent& event);
    void OnFileNameEntered(wxCommandEvent& event);
    void OnFileNameChanged(wxCommandEvent& event);
    void OnChoiceChanged(wxCommandEvent& event);
    void changeFocus(wxChildFocusEvent& event);
    void OnSpinCtrlIgnoreRows(wxSpinEvent& event);
    void OnCheckboxClick(wxCommandEvent& event);
    void OnMenuSelected(wxCommandEvent& event);
private:
    void OnLoad();
    void UpdateListItemBackground();
    void update_preview();
    void initDelimiter();
    void initDateMask();

    bool ShowToolTips();
    void OnSettingsSelected(wxCommandEvent& event);
    const wxString GetStoredSettings(int id) const;
    void SetSettings(const wxString &data);
    ITransactionsFile *CreateFileHandler();
};

inline bool mmUnivCSVDialog::isImportCompletedSuccessfully() const
{
    return importSuccessful_;
}
inline int mmUnivCSVDialog::ImportedAccountID() const
{
    return accountID_;
}

inline bool mmUnivCSVDialog::IsImporter() const
{
    return dialogType_ == DIALOG_TYPE_IMPORT_CSV || dialogType_ == DIALOG_TYPE_IMPORT_XML;
}
inline bool mmUnivCSVDialog::IsXML() const
{
    return dialogType_ == DIALOG_TYPE_IMPORT_XML || dialogType_ == DIALOG_TYPE_EXPORT_XML;
}
inline bool mmUnivCSVDialog::IsCSV() const
{
    return !IsXML();
}
inline wxString mmUnivCSVDialog::GetSettingsPrfix() const
{
    return IsXML() ? "XML_SETTINGS_%d" : "CSV_SETTINGS_%d";
}

#endif
