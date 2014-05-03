/*************************************************************************
 Copyright (C) 2006 Madhan Kanagavel
 copyright (C) 2011, 2012 Nikolay & Stefano Giorgio.
 copyright (C) 2014 Guan Lisheng (guanlisheng@gmail.com)

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
 *************************************************************************/

#ifndef _MM_EX_OPTIONSDIALOG_H_
#define _MM_EX_OPTIONSDIALOG_H_

#include "defs.h"
#include <wx/spinctrl.h>

class mmOptionsDialog: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( mmOptionsDialog )
    DECLARE_EVENT_TABLE()

public:
    mmOptionsDialog( );
    mmOptionsDialog(wxWindow* parent);
    ~mmOptionsDialog( );

    /// Saves the updated System Options to the appropriate databases.
    void SaveNewSystemSettings();
private:
    /// Dialog Creation - Used by constructor
    bool Create(wxWindow* parent, wxWindowID id,
        const wxString& caption,
        const wxPoint& pos,
        const wxSize& size,
        long style);

    void CreateControls();

    void OnCurrency(wxCommandEvent& event);
    void OnDateFormatChanged(wxCommandEvent& event);
    void OnLanguageChanged(wxCommandEvent& event);

    /// Colour Changing events
    void OnNavTreeColorChanged(wxCommandEvent& event);
    void OnAlt0Changed(wxCommandEvent& event);
    void OnAlt1Changed(wxCommandEvent& event);
    void OnListBackgroundChanged(wxCommandEvent& event);
    void OnListBorderChanged(wxCommandEvent& event);
    void OnListDetailsColors(wxCommandEvent& event);
    void OnListFutureDates(wxCommandEvent& event);

    bool GetIniDatabaseCheckboxValue(wxString dbField, bool defaultState);

	void OnAttachmentsButton(wxCommandEvent& event);

    void SaveViewAccountOptions();
    void SaveViewTransactionOptions();
    void SaveFinancialYearStart();
    void SaveStocksUrl();
    void OnAttachmentsMenu(wxCommandEvent& event);

    void SaveGeneralPanelSettings();
    void SaveViewPanelSettings();
    void SaveAttachmentPanelSettings();
    void SaveOthersPanelSettings();
    void SaveNetworkPanelSettings();
    void OnOk(wxCommandEvent& /*event*/);
    void OnApply(wxCommandEvent& /*event*/);

    /// Dialog specific controls
    wxImageList* m_imageList;
    wxChoice* choiceDateFormat_;
    wxChoice* choiceVisible_;
    wxChoice* choiceTransVisible_;
    wxChoice* choiceFontSize_;
    wxChoice* monthSelection_;

    /// Colour Buttons.
    wxButton* restoreDefaultButton_;
    wxButton* UDFCB1_;
    wxButton* UDFCB2_;
    wxButton* UDFCB3_;
    wxButton* UDFCB4_;
    wxButton* UDFCB5_;
    wxButton* UDFCB6_;
    wxButton* UDFCB7_;

    wxCheckBox* cbUseOrgDateCopyPaste_;
    wxCheckBox* cbUseSound_;

    wxCheckBox* cbBudgetFinancialYears_;
    wxCheckBox* cbBudgetIncludeTransfers_;
    wxCheckBox* cbBudgetSetupWithoutSummary_;
    wxCheckBox* cbBudgetSummaryWithoutCateg_;
    wxCheckBox* cbIgnoreFutureTransactions_;

	wxCheckBox* cbDeleteAttachments_;
	wxCheckBox* cbTrashAttachments_;

    wxStaticText* sampleDateText_;
    wxSpinCtrl *scMax_files_;

    wxSpinCtrl *scProxyPort_;

    wxCheckBox* cbWebServerCheckBox_;
    wxSpinCtrl *scWebServerPort_;

	wxCheckBox* cbSendData_;

    int currencyId_;
    wxString dateFormat_;
    wxString currentLanguage_;

    wxArrayString viewAccountStrings(bool translated, const wxString& get_string_id);
    wxArrayString viewAccountStrings(bool translated, const wxString& input_string, int& row_id);

    enum
    {
        ID_BOOK_PANEL_EXP_IMP = wxID_HIGHEST + 100,
        ID_DIALOG_OPTIONS_TEXTCTRL_DELIMITER4,
		ID_DIALOG_OPTIONS_TEXTCTRL_ATTACHMENT,
        ID_DIALOG_OPTIONS_BUTTON_CURRENCY,
        ID_DIALOG_OPTIONS_BUTTON_LANGUAGE,
        ID_DIALOG_OPTIONS_LISTBOOK,
        ID_BOOK_PANELGENERAL,
        ID_BOOK_PANELVIEWS,
        ID_BOOK_PANELCOLORS,
        ID_DIALOG_OPTIONS,
        ID_DIALOG_OPTIONS_PANEL1,
        ID_DIALOG_OPTIONS_PANEL2,
        ID_DIALOG_OPTIONS_DATE_FORMAT,
        ID_DIALOG_OPTIONS_VIEW_ACCOUNTS,
        ID_DIALOG_OPTIONS_STATIC_SAMPLE_DATE,
        ID_DIALOG_OPTIONS_BUTTON_COLOR_NAVTREE,
        ID_DIALOG_OPTIONS_BUTTON_COLOR_ALT0,
        ID_DIALOG_OPTIONS_BUTTON_COLOR_ALT1,
        ID_DIALOG_OPTIONS_BUTTON_COLOR_LISTBACK,
        ID_DIALOG_OPTIONS_BUTTON_COLOR_LISTBORDER,
        ID_DIALOG_OPTIONS_BUTTON_COLOR_RESTOREDEFAULT,
        ID_DIALOG_OPTIONS_BUTTON_COLOR_LISTDETAILS,
        ID_BOOK_PANELMISC,
        ID_DIALOG_OPTIONS_CHK_BACKUP,
        ID_DIALOG_OPTIONS_CHK_BACKUP_UPDATE,
        ID_DIALOG_OPTIONS_TEXTCTRL_STOCKURL,
        ID_DIALOG_OPTIONS_VIEW_TRANS,
        ID_DIALOG_OPTIONS_FONT_SIZE,
        ID_DIALOG_OPTIONS_TEXTCTRL_USERNAME,
        ID_DIALOG_OPTIONS_BUTTON_COLOR_FUTUREDATES,
        ID_DIALOG_OPTIONS_BUTTON_DATEFORMAT,
        ID_DIALOG_OPTIONS_EXPAND_BANK_HOME,
        ID_DIALOG_OPTIONS_EXPAND_TERM_HOME,
        ID_DIALOG_OPTIONS_EXPAND_STOCK_HOME,
        ID_DIALOG_OPTIONS_EXPAND_BANK_TREE,
        ID_DIALOG_OPTIONS_EXPAND_TERM_TREE,
        ID_DIALOG_OPTIONS_FINANCIAL_YEAR_START_DAY,
        ID_DIALOG_OPTIONS_FINANCIAL_YEAR_START_MONTH,
        ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_PAYEE,
        ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_CATEGORY,
        ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_STATUS,
        ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_DATE,
        ID_DIALOG_OPTIONS_TEXTCTRL_PROXY,
        ID_DIALOG_OPTIONS_TEXTCTRL_WEBAPPURL,
        ID_DIALOG_OPTIONS_TEXTCTRL_WEBAPPGUID,
		ID_DIALOG_OPTIONS_BUTTON_ATTACHMENTSFOLDER,
        ID_DIALOG_OPTIONS_ENABLE_WEB_SERVER,
        ID_DIALOG_OPTIONS_WEB_SERVER_PORT,
		ID_DIALOG_OPTIONS_ALLOW_SEND_USAGE
    };
};

#endif
