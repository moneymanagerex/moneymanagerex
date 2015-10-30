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

#include "defs.h"
#include <vector>
#include <map>

#define ID_MYDIALOG8 10040
#define SYMBOL_UNIVCSVDIALOG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_UNIVCSVDIALOG_TITLE _("Universal CSV Dialog")
#define SYMBOL_UNIVCSVDIALOG_IDNAME ID_MYDIALOG8
#define SYMBOL_UNIVCSVDIALOG_SIZE wxSize(400, 300)
#define SYMBOL_UNIVCSVDIALOG_POSITION wxDefaultPosition
#define ID_LISTBOX 10090
#define ID_PANEL10 10091
#define ID_PANEL_LOGS 10092
#define ID_UNIVCSVBUTTON_IMPORT 10094
#define ID_UNIVCSVBUTTON_EXPORT 10095
#define ID_LISTBOX_CANDICATE 10099
#define wxID_SEARCH 10100
#define wxID_RADIO_BOX 10101
#define wxID_STANDARD 10102
#define wxID_ACCOUNT 10103
#define ID_UD_DELIMIT 10104
#define ID_FILE_NAME 10105
#define ID_DATE_FORMAT 10106
#define ID_ENCODING 10107
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

class mmUnivCSVDialog: public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmUnivCSVDialog);
    wxDECLARE_EVENT_TABLE();

public:
    /// Constructors
    mmUnivCSVDialog();
    mmUnivCSVDialog(wxWindow* parent, bool is_importer = true,
                    wxWindowID id = SYMBOL_UNIVCSVDIALOG_IDNAME,
                    const wxString& caption = SYMBOL_UNIVCSVDIALOG_TITLE,
                    const wxPoint& pos = SYMBOL_UNIVCSVDIALOG_POSITION,
                    const wxSize& size = SYMBOL_UNIVCSVDIALOG_SIZE,
                    long style = SYMBOL_UNIVCSVDIALOG_STYLE);

    bool InportCompletedSuccessfully()
    {
        return importSuccessful_;
    }
    int ImportedAccountID()
    {
        return fromAccountID_;
    }

private:
    enum EUnivCvs
    {
        UNIV_CSV_DATE = 0,
        UNIV_CSV_PAYEE,
        UNIV_CSV_AMOUNT,
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

    struct tran_holder
    {
        wxDateTime Date;
        wxString Type;
        wxString Status = "";
        int ToAccountID = -1;
        double ToAmount = 0.0;
        int PayeeID = -1;
        int CategoryID = -1;
        int SubCategoryID = -1;
        double Amount = 0.0;
        wxString Number;
        wxString Notes;
    };
    bool is_importer_;
    wxString delimit_;

    std::vector<int> csvFieldOrder_;
    wxListBox* csvFieldCandicate_;
    wxListBox* csvListBox_;

    wxButton* m_button_add_;
    wxButton* m_button_remove_;
    wxChoice* m_choice_account_;
    wxListCtrl* m_list_ctrl_; //preview
    wxTextCtrl* m_text_ctrl_;
    wxTextCtrl* log_field_;
    wxTextCtrl* m_textDelimiter;

    std::map<int, wxString> CSVFieldName_;

    wxChoice* choiceDateFormat_;
    wxChoice* m_choiceEncoding;
    wxString date_format_;

    int fromAccountID_;
    bool importSuccessful_;
    int m_oject_in_focus;

    /// Creation
    bool Create(wxWindow* parent,
                wxWindowID id = SYMBOL_UNIVCSVDIALOG_IDNAME,
                const wxString& caption = SYMBOL_UNIVCSVDIALOG_TITLE,
                const wxPoint& pos = SYMBOL_UNIVCSVDIALOG_POSITION,
                const wxSize& size = SYMBOL_UNIVCSVDIALOG_SIZE,
                long style = SYMBOL_UNIVCSVDIALOG_STYLE);

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
    void OnSave(wxCommandEvent& event);
    void OnMoveUp(wxCommandEvent& event);
    void OnMoveDown(wxCommandEvent& event);
    void OnStandard(wxCommandEvent& event);
    void OnSearch(wxCommandEvent& event);
    void OnListBox(wxCommandEvent& event);
    void OnDelimiterChange(wxCommandEvent& event);
    void OnButtonClear(wxCommandEvent& event);
    void OnFileNameEntered(wxCommandEvent& event);
    void OnFileNameChanged(wxCommandEvent& event);
    void OnDateFormatChanged(wxCommandEvent& event);
    void changeFocus(wxChildFocusEvent& event);

    void OnLoad();
    void update_preview();
    void initDelimiter();
    void initDateMask();

    wxBitmap GetBitmapResource(const wxString& name);
    wxIcon GetIconResource(const wxString& name);
    const bool ShowToolTips();
    void OnSettingsSelected(wxCommandEvent& event);
    wxString GetStoredSettings(int id);
    void SetSettings(const wxString &data);
};
#endif
