/*************************************************************************
 Copyright (C) 2006 Madhan Kanagavel
 copyright (C) 2011, 2012 Nikolay & Stefano Giorgio.

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

#include "guiid.h"
#include "mmcoredb.h"
#include "mmex_settings.h"
#include <wx/spinctrl.h>

#define SYMBOL_MMOPTIONSDIALOG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_MMOPTIONSDIALOG_TITLE _("New MMEX Options")
#define SYMBOL_MMOPTIONSDIALOG_IDNAME ID_DIALOG_OPTIONS
#define SYMBOL_MMOPTIONSDIALOG_SIZE wxSize(500, 400)
#define SYMBOL_MMOPTIONSDIALOG_POSITION wxDefaultPosition

class mmOptionsDialog: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( mmOptionsDialog )
    DECLARE_EVENT_TABLE()

public:
    mmOptionsDialog( );
    mmOptionsDialog(mmCoreDB* core,
        wxWindow* parent, wxWindowID id = SYMBOL_MMOPTIONSDIALOG_IDNAME,
        const wxString& caption = SYMBOL_MMOPTIONSDIALOG_TITLE,
        const wxPoint& pos = SYMBOL_MMOPTIONSDIALOG_POSITION,
        const wxSize& size = SYMBOL_MMOPTIONSDIALOG_SIZE,
        long style = SYMBOL_MMOPTIONSDIALOG_STYLE );
    ~mmOptionsDialog( );

    /// Saves the updated System Options to the appropriate databases.
    void SaveNewSystemSettings();

    /// Returns the value of the dialog setting;
    bool GetUpdateCurrencyRateSetting();

    bool RequiresRestart()  { return restartRequired_; }
    bool AppliedChanges()   { return changesApplied_;  }

private:
    /// System database access variables
    mmCoreDB* core_;

    /// Dialog specific controls
    wxImageList* m_imageList;
    wxChoice* choiceDateFormat_;
    wxChoice* choiceVisible_;
    wxChoice* choiceTransVisible_;
    wxChoice* choiceFontSize_;
    wxChoice* monthSelection_;

    /// Colour Buttons.
    wxButton* navTreeButton_;
    wxButton* listBackgroundButton_;
    wxButton* listRowZeroButton_;
    wxButton* listRowOneButton_;
    wxButton* listBorderButton_;
    wxButton* listDetailsButton_;
    wxButton* futureTransButton_;
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
    wxCheckBox* cbEnableCurrencyUpd_;

    wxCheckBox* cbBudgetFinancialYears_;
    wxCheckBox* cbBudgetIncludeTransfers_;
    wxCheckBox* cbBudgetSetupWithoutSummary_;
    wxCheckBox* cbBudgetSummaryWithoutCateg_;
    wxCheckBox* cbIgnoreFutureTransactions_;

    wxStaticText* sampleDateText_;
    wxSpinCtrl *scMax_files_;

    bool restartRequired_;
    bool changesApplied_;

    int currencyId_;
    wxString dateFormat_;
    wxString currentLanguage_;

    wxArrayString viewAccountStrings(bool translated, wxString get_string_id);
    wxArrayString viewAccountStrings(bool translated, wxString input_string, int& row_id);

    /// Dialog Creation - Used by constructor
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_MMOPTIONSDIALOG_IDNAME,
        const wxString& caption = SYMBOL_MMOPTIONSDIALOG_TITLE,
        const wxPoint& pos = SYMBOL_MMOPTIONSDIALOG_POSITION,
        const wxSize& size = SYMBOL_MMOPTIONSDIALOG_SIZE,
        long style = SYMBOL_MMOPTIONSDIALOG_STYLE );

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
    void OnRestoreDefaultColors(wxCommandEvent& event);

    bool GetIniDatabaseCheckboxValue(wxString dbField, bool defaultState);

    void OnDelimiterSelectedU(wxCommandEvent& event);
    void OnDelimiterSelectedC(wxCommandEvent& event);
    void OnDelimiterSelectedS(wxCommandEvent& event);
    void OnDelimiterSelectedT(wxCommandEvent& event);

    void SaveViewAccountOptions();
    void SaveViewTransactionOptions();
    void SaveFinancialYearStart();
    void SaveStocksUrl();

    void SaveGeneralPanelSettings();
    void SaveViewPanelSettings();
    void SaveColourPanelSettings();
    void SaveOthersPanelSettings();
    void SaveImportExportPanelSettings();
    void OnOk(wxCommandEvent& /*event*/);
};

#endif
