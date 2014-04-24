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

#include "optionsdialog.h"
#include "maincurrencydialog.h"
#include "attachmentdialog.h"
#include "util.h"
#include "mmOption.h"
#include <wx/colordlg.h>
#include "constants.h"
#include "paths.h"
#include "model/Model_Infotable.h"
#include "model/Model_Setting.h"
#include "model/Model_Currency.h"
#include "model/Model_Checking.h"

#include "../resources/main-setup.xpm"
#include "../resources/preferences-attachments.xpm"
#include "../resources/preferences-network.xpm"
#include "../resources/preferences-other.xpm"
#include "../resources/view.xpm"

class mmSettingPanel: public wxPanel
{
    virtual void SaveSetting() = 0;
};

class mmGeneralSettingPanel: public mmSettingPanel
{
    // TODO

    void SaveSetting()
    {
        /*
        Model_Infotable::instance().Set("USERNAME", mmOptions::instance().userNameString_);
        Model_Setting::instance().Set(LANGUAGE_PARAMETER, languageButton->GetLabel().Lower());
        Model_Infotable::instance().SetBaseCurrencyID(currencyId_);
        Model_Infotable::instance().Set("DATEFORMAT", dateFormat_);
        Model_Setting::instance().Set(INIDB_USE_ORG_DATE_COPYPASTE, cbUseOrgDateCopyPaste_->GetValue());
        Model_Setting::instance().Set(INIDB_USE_TRANSACTION_SOUND, cbUseSound_->GetValue());
        */
    }
};

class mmDisplaySettingPanel: public mmSettingPanel
{
    // TODO
};

class mmAttachmentSettingPanel: public mmSettingPanel
{
    // TODO
};

class mmNetworkSettingPanel: public mmSettingPanel
{
    // TODO
};

class mmOtherSettingPanel: public mmSettingPanel
{
    // TODO
};

IMPLEMENT_DYNAMIC_CLASS( mmOptionsDialog, wxDialog )

BEGIN_EVENT_TABLE( mmOptionsDialog, wxDialog )
    EVT_BUTTON(wxID_OK, mmOptionsDialog::OnOk)
    EVT_BUTTON(wxID_APPLY, mmOptionsDialog::OnApply)
    EVT_BUTTON(ID_DIALOG_OPTIONS_BUTTON_CURRENCY, mmOptionsDialog::OnCurrency)
    EVT_BUTTON(wxID_APPLY, mmOptionsDialog::OnDateFormatChanged)
    EVT_BUTTON(ID_DIALOG_OPTIONS_BUTTON_LANGUAGE, mmOptionsDialog::OnLanguageChanged)
	EVT_BUTTON(ID_DIALOG_OPTIONS_BUTTON_ATTACHMENTSFOLDER, mmOptionsDialog::OnAttachmentsFolderChanged)
	EVT_RADIOBUTTON(wxID_ANY, mmOptionsDialog::OnAttachmentSelected)
END_EVENT_TABLE()

mmOptionsDialog::mmOptionsDialog( )
{
}

mmOptionsDialog::~mmOptionsDialog( )
{
    delete m_imageList;
}

mmOptionsDialog::mmOptionsDialog(wxWindow* parent)
{
    long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY, _("New MMEX Options"), wxDefaultPosition, wxSize(500, 400), style);
}

bool mmOptionsDialog::Create(
    wxWindow* parent, wxWindowID id,
    const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    currencyId_ = Model_Infotable::instance().GetBaseCurrencyId();
    dateFormat_ = Model_Infotable::instance().GetStringInfo("DATEFORMAT", mmex::DEFDATEFORMAT);

    CreateControls();
    Centre();
    Fit();

    return TRUE;
}

wxArrayString mmOptionsDialog::viewAccountStrings(bool translated, const wxString& input_string, int& row_id_)
{
    wxArrayString itemChoiceViewAccountStrings;

    if (translated) {
        itemChoiceViewAccountStrings.Add(_("All"));
        itemChoiceViewAccountStrings.Add(_("Open"));
        itemChoiceViewAccountStrings.Add(_("Favorites"));
    } else {
        itemChoiceViewAccountStrings.Add(VIEW_ACCOUNTS_ALL_STR);
        itemChoiceViewAccountStrings.Add(VIEW_ACCOUNTS_OPEN_STR);
        itemChoiceViewAccountStrings.Add(VIEW_ACCOUNTS_FAVORITES_STR);
    }
    if (!input_string.IsEmpty())
    {
        for(size_t i = 0; i < itemChoiceViewAccountStrings.Count(); i++)
        {
            if(input_string == itemChoiceViewAccountStrings[i]) {
                row_id_ = i;
                break;
            }
        }
    }

    return itemChoiceViewAccountStrings;
}

void mmOptionsDialog::CreateControls()
{
    wxSizerFlags flags, flagsExpand;
    flags.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).Border(wxALL, 5);
    flagsExpand.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).Border(wxALL, 5).Expand();

    wxSize imageSize(48, 48);
    m_imageList = new wxImageList(imageSize.GetWidth(), imageSize.GetHeight());
    m_imageList->Add(wxBitmap(view_xpm));
    m_imageList->Add(wxBitmap(preferences_attachments_xpm));
    m_imageList->Add(wxBitmap(main_setup_xpm));
    m_imageList->Add(wxBitmap(preferences_other_xpm));
    m_imageList->Add(wxBitmap(preferences_network_xpm));

    wxBoxSizer* mainDialogSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(mainDialogSizer);

    wxPanel* mainDialogPanel = new wxPanel(this
        , wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    mainDialogSizer->Add(mainDialogPanel, 1, wxGROW|wxALL, 5);

    wxBoxSizer* mainDialogPanelSizer = new wxBoxSizer(wxVERTICAL);
    mainDialogPanel->SetSizer(mainDialogPanelSizer);

    wxListbook* newBook = new wxListbook(mainDialogPanel, ID_DIALOG_OPTIONS_LISTBOOK, wxDefaultPosition, wxDefaultSize, wxLB_LEFT);

    /*********************************************************************************************
     General Panel
    **********************************************************************************************/
    wxPanel* generalPanel = new wxPanel(newBook, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

    SetMinSize(wxSize(500,600));
    wxBoxSizer* generalPanelSizer = new wxBoxSizer(wxVERTICAL);
    generalPanel->SetSizer(generalPanelSizer);

    // Display Header Settings
    wxStaticBox* headerStaticBox = new wxStaticBox(generalPanel, wxID_STATIC, _("Display Heading"));
    // Define the staticBox font and set it as wxFONTWEIGHT_BOLD
    wxFont staticBoxFontSetting = headerStaticBox->GetFont();
    staticBoxFontSetting.SetWeight(wxFONTWEIGHT_BOLD);
    headerStaticBox->SetFont(staticBoxFontSetting);

    wxStaticBoxSizer* headerStaticBoxSizer = new wxStaticBoxSizer(headerStaticBox, wxHORIZONTAL);

    headerStaticBoxSizer->Add(new wxStaticText(generalPanel, wxID_STATIC,
        _("User Name")), flags);

    wxString userName = Model_Infotable::instance().GetStringInfo("USERNAME", "");
    wxTextCtrl* userNameTextCtr = new wxTextCtrl(generalPanel, ID_DIALOG_OPTIONS_TEXTCTRL_USERNAME
        , userName, wxDefaultPosition, wxSize(200, -1));
    userNameTextCtr->SetToolTip(_("The User Name is used as a title for the database."));
    headerStaticBoxSizer->Add(userNameTextCtr, 1, wxEXPAND|wxALL, 5);
    generalPanelSizer->Add(headerStaticBoxSizer, flagsExpand);

    // Language Settings
    wxStaticBox* languageStaticBox = new wxStaticBox(generalPanel, wxID_STATIC, _("Language"));
    languageStaticBox->SetFont(staticBoxFontSetting);
    wxStaticBoxSizer* languageStaticBoxSizer = new wxStaticBoxSizer(languageStaticBox, wxHORIZONTAL);
    generalPanelSizer->Add(languageStaticBoxSizer, flagsExpand);

    currentLanguage_ = Model_Setting::instance().GetStringSetting(LANGUAGE_PARAMETER, "english");
    wxButton* languageButton = new wxButton(generalPanel, ID_DIALOG_OPTIONS_BUTTON_LANGUAGE
        , currentLanguage_.Left(1).Upper() + currentLanguage_.SubString(1,currentLanguage_.Len())
        , wxDefaultPosition, wxSize(150, -1), 0);
    languageButton->SetToolTip(_("Specify the language to use"));
    languageStaticBoxSizer->Add(languageButton, flags);

    // Currency Settings
    wxStaticBox* currencyStaticBox = new wxStaticBox(generalPanel, wxID_STATIC, _("Currency"));
    currencyStaticBox->SetFont(staticBoxFontSetting);
    wxStaticBoxSizer* currencyStaticBoxSizer = new wxStaticBoxSizer(currencyStaticBox, wxHORIZONTAL);
    currencyStaticBox->SetFont(staticBoxFontSetting);
    generalPanelSizer->Add(currencyStaticBoxSizer, flagsExpand);

    currencyStaticBoxSizer->Add(new wxStaticText(generalPanel, wxID_STATIC, _("Base Currency")), flags);

    wxString currName = _("Set Currency");
    Model_Currency::Data* currency = Model_Currency::instance().get(currencyId_);
    if (currency)
        currName = currency->CURRENCYNAME;
    wxButton* baseCurrencyButton = new wxButton(generalPanel, ID_DIALOG_OPTIONS_BUTTON_CURRENCY
        , currName, wxDefaultPosition, wxDefaultSize);
    baseCurrencyButton->SetToolTip(_("Sets the default currency for the database."));
    currencyStaticBoxSizer->Add(baseCurrencyButton, flags);

    // Date Format Settings
    wxStaticBox* dateFormatStaticBox = new wxStaticBox(generalPanel, wxID_STATIC, _("Date Format"));
    dateFormatStaticBox->SetFont(staticBoxFontSetting);
    wxStaticBoxSizer* dateFormatStaticBoxSizer = new wxStaticBoxSizer(dateFormatStaticBox, wxVERTICAL);
    wxFlexGridSizer* flex_sizer = new wxFlexGridSizer(0,2,0,5);
    generalPanelSizer->Add(dateFormatStaticBoxSizer, flagsExpand);
    dateFormatStaticBoxSizer->Add(flex_sizer);

    choiceDateFormat_ = new wxChoice(generalPanel, wxID_STATIC);
    for (const auto& i : date_formats_map())
    {
        choiceDateFormat_->Append(i.second, new wxStringClientData(i.first));
        if (dateFormat_ == i.first) choiceDateFormat_->SetStringSelection(i.second);
    }
    flex_sizer->Add(choiceDateFormat_, flags);
    choiceDateFormat_->SetToolTip(_("Specify the date format for display"));

    wxButton* setFormatButton = new wxButton(generalPanel, wxID_APPLY, _("Set"));
    flex_sizer->Add(setFormatButton, flags);

    sampleDateText_ = new wxStaticText(generalPanel, wxID_STATIC,
        "redefined elsewhere");
    flex_sizer->Add(new wxStaticText(generalPanel, wxID_STATIC,
        _("New date format sample:")), flags);
    flex_sizer->Add(sampleDateText_, flags);
    sampleDateText_->SetLabel(wxDateTime::Now().Format(dateFormat_));

    // Financial Year Settings
    wxStaticBox* financialYearStaticBox = new wxStaticBox(generalPanel, wxID_ANY, _("Financial Year"));
    financialYearStaticBox->SetFont(staticBoxFontSetting);
    wxStaticBoxSizer* financialYearStaticBoxSizer = new wxStaticBoxSizer(financialYearStaticBox, wxVERTICAL);
    wxFlexGridSizer* financialYearStaticBoxSizerGrid = new wxFlexGridSizer(0,2,0,0);
    generalPanelSizer->Add(financialYearStaticBoxSizer, 0, wxGROW|wxALL, 5);
    financialYearStaticBoxSizer->Add(financialYearStaticBoxSizerGrid);

    financialYearStaticBoxSizerGrid->Add(new wxStaticText(generalPanel, wxID_STATIC, _("Start Day")), flags);
    int day = Model_Infotable::instance().GetIntInfo("FINANCIAL_YEAR_START_DAY", 1);

    wxSpinCtrl *textFPSDay = new wxSpinCtrl(generalPanel, ID_DIALOG_OPTIONS_FINANCIAL_YEAR_START_DAY,
        wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 31, day);
    textFPSDay->SetToolTip(_("Specify Day for start of financial year"));

    financialYearStaticBoxSizerGrid->Add(textFPSDay, flags);

    financialYearStaticBoxSizerGrid->Add(new wxStaticText(generalPanel, wxID_STATIC, _("Start Month")), flags);

    wxArrayString financialMonthsSelection;
    for (wxDateTime::Month m = wxDateTime::Jan; m <= wxDateTime::Dec; m = wxDateTime::Month(m + 1))
        financialMonthsSelection.Add(wxGetTranslation(wxDateTime::GetMonthName(m, wxDateTime::Name_Abbr)));

    monthSelection_ = new wxChoice(generalPanel, ID_DIALOG_OPTIONS_FINANCIAL_YEAR_START_MONTH
        , wxDefaultPosition, wxSize(100, -1), financialMonthsSelection);
    financialYearStaticBoxSizerGrid->Add(monthSelection_, flags);

    int monthItem = Model_Infotable::instance().GetIntInfo("FINANCIAL_YEAR_START_MONTH", 7);
    monthSelection_->SetSelection(monthItem - 1);
    monthSelection_->SetToolTip(_("Specify month for start of financial year"));
	
    //a bit more space visual appearance
    generalPanelSizer->AddSpacer(15);

    cbUseOrgDateCopyPaste_ = new wxCheckBox(generalPanel, wxID_STATIC, _("Use Original Date when Pasting Transactions"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    cbUseOrgDateCopyPaste_->SetValue(GetIniDatabaseCheckboxValue(INIDB_USE_ORG_DATE_COPYPASTE, false));
    cbUseOrgDateCopyPaste_->SetToolTip(_("Select whether to use the original transaction date or current date when copying/pasting transactions"));
    generalPanelSizer->Add(cbUseOrgDateCopyPaste_, flags);

    cbUseSound_ = new wxCheckBox(generalPanel, wxID_STATIC, _("Use Transaction Sound"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    cbUseSound_->SetValue(GetIniDatabaseCheckboxValue(INIDB_USE_TRANSACTION_SOUND, true));
    cbUseSound_->SetToolTip(_("Select whether to use sounds when entering transactions"));
    generalPanelSizer->Add(cbUseSound_, flags);

    /*********************************************************************************************
     Views Panel
    **********************************************************************************************/
    wxPanel* viewsPanel = new wxPanel(newBook, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    wxBoxSizer* viewsPanelSizer = new wxBoxSizer(wxVERTICAL);
    viewsPanel->SetSizer(viewsPanelSizer);

    // Account View Options
    wxStaticBox* accountStaticBox = new wxStaticBox(viewsPanel, wxID_STATIC, _("View Options"));
    accountStaticBox->SetFont(staticBoxFontSetting);

    wxStaticBoxSizer* accountStaticBoxSizer = new wxStaticBoxSizer(accountStaticBox, wxVERTICAL);
    viewsPanelSizer->Add(accountStaticBoxSizer, flagsExpand);
    wxFlexGridSizer* view_sizer1 = new wxFlexGridSizer(0,2,0,5);
    accountStaticBoxSizer->Add(view_sizer1);

    view_sizer1->Add(new wxStaticText(viewsPanel, wxID_STATIC, _("Accounts Visible")), flags);

    int row_id_ = 0;
    wxArrayString itemChoiceViewAccountTranslatedStrings = viewAccountStrings(true, wxEmptyString, row_id_);

    choiceVisible_ = new wxChoice(viewsPanel, ID_DIALOG_OPTIONS_VIEW_ACCOUNTS
        , wxDefaultPosition, wxDefaultSize, itemChoiceViewAccountTranslatedStrings);
    view_sizer1->Add(choiceVisible_, flags);

    wxString vAccts = Model_Setting::instance().GetStringSetting("VIEWACCOUNTS", VIEW_ACCOUNTS_ALL_STR);
    row_id_ = 0;
    wxArrayString itemChoiceViewAccountStrings = viewAccountStrings(false, vAccts, row_id_);
    choiceVisible_->SetSelection(row_id_);

    choiceVisible_->SetToolTip(_("Specify which accounts are visible"));

    view_sizer1->Add(new wxStaticText(viewsPanel, wxID_STATIC,
        _("Transactions Visible")), flags);

    wxArrayString view_strings;
    view_strings.Add(VIEW_TRANS_ALL_STR);
    view_strings.Add(VIEW_TRANS_TODAY_STR);
    view_strings.Add(VIEW_TRANS_CURRENT_MONTH_STR);
    view_strings.Add(VIEW_TRANS_LAST_30_DAYS_STR);
    view_strings.Add(VIEW_TRANS_LAST_90_DAYS_STR);
    view_strings.Add(VIEW_TRANS_LAST_MONTH_STR);
    view_strings.Add(VIEW_TRANS_LAST_3MONTHS_STR);
    view_strings.Add(VIEW_TRANS_CURRENT_YEAR_STR);
    view_strings.Add(VIEW_TRANS_LAST_12MONTHS_STR);

    choiceTransVisible_ = new wxChoice(viewsPanel
        , wxID_ANY, wxDefaultPosition, wxDefaultSize);
    for(const auto &entry : view_strings)
        choiceTransVisible_->Append(wxGetTranslation(entry),
        new wxStringClientData(entry));

    view_sizer1->Add(choiceTransVisible_,flags);

    wxString vTrans = Model_Setting::instance().GetStringSetting("VIEWTRANSACTIONS", VIEW_TRANS_ALL_STR);
    choiceTransVisible_->SetStringSelection(wxGetTranslation(vTrans));
    choiceTransVisible_->SetToolTip(_("Specify which transactions are visible by default"));

    view_sizer1->Add(new wxStaticText(viewsPanel, wxID_STATIC, _("Report Font Size")), flags);

    const wxString itemChoiceFontSize[] = {
        wxTRANSLATE("XSmall"),
        wxTRANSLATE("Small"),
        wxTRANSLATE("Normal"),
        wxTRANSLATE("Large"),
        wxTRANSLATE("XLarge"),
        wxTRANSLATE("XXLarge"),
        wxTRANSLATE("Huge")};

    choiceFontSize_ = new wxChoice(viewsPanel, wxID_ANY);

    for(const auto &entry : itemChoiceFontSize)
        choiceFontSize_->Append(wxGetTranslation(entry));

    int vFontSize = -1 + Model_Setting::instance().GetIntSetting("HTMLFONTSIZE", 3);
    choiceFontSize_->SetSelection(vFontSize);

    choiceFontSize_->SetToolTip(_("Specify which font size is used on the report tables"));
    view_sizer1->Add(choiceFontSize_, flags);

    // Navigation Tree Expansion Options
    wxStaticBox* navTreeOptionsStaticBox = new wxStaticBox(viewsPanel
        , wxID_ANY, _("Navigation Tree Expansion Options"));
    navTreeOptionsStaticBox->SetFont(staticBoxFontSetting);
    wxStaticBoxSizer* navTreeOptionsStaticBoxSizer = new wxStaticBoxSizer(navTreeOptionsStaticBox, wxHORIZONTAL);
    viewsPanelSizer->Add(navTreeOptionsStaticBoxSizer, flagsExpand);

    // Expand Bank Tree
    wxCheckBox* expandBankCheckBox = new wxCheckBox(viewsPanel, ID_DIALOG_OPTIONS_EXPAND_BANK_TREE
        , _("Bank Accounts"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    expandBankCheckBox->SetValue(GetIniDatabaseCheckboxValue("EXPAND_BANK_TREE",true));
    expandBankCheckBox->SetToolTip(_("Expand Bank Accounts in Trew View when tree is refreshed"));
    navTreeOptionsStaticBoxSizer->Add(expandBankCheckBox, flags);

    // Expand Term Tree
    wxCheckBox* expandTermCheckBox = new wxCheckBox(viewsPanel, ID_DIALOG_OPTIONS_EXPAND_TERM_TREE
        , _("Term Accounts"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    expandTermCheckBox->SetValue(GetIniDatabaseCheckboxValue("EXPAND_TERM_TREE",false));
    expandTermCheckBox->SetToolTip(_("Expand Term Accounts in Trew View when tree is refreshed"));
    navTreeOptionsStaticBoxSizer->Add(expandTermCheckBox, flags);

    // Home Page Expansion Options
    wxStaticBox* homePageStaticBox = new wxStaticBox(viewsPanel, wxID_STATIC, _("Home Page Expansion Options"));
    homePageStaticBox->SetFont(staticBoxFontSetting);
    wxStaticBoxSizer* homePageStaticBoxSizer = new wxStaticBoxSizer(homePageStaticBox, wxVERTICAL);
    viewsPanelSizer->Add(homePageStaticBoxSizer, flagsExpand);

    // Expand Bank Home
    wxCheckBox* expandBankHomeCheckBox = new wxCheckBox(viewsPanel, ID_DIALOG_OPTIONS_EXPAND_BANK_HOME
        , _("Bank Accounts"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    expandBankHomeCheckBox->SetValue(GetIniDatabaseCheckboxValue("EXPAND_BANK_HOME",true));
    expandBankHomeCheckBox->SetToolTip(_("Expand Bank Accounts on home page when page is refreshed"));
    homePageStaticBoxSizer->Add(expandBankHomeCheckBox, flags);

    // Expand Term Home
    wxCheckBox* itemCheckBoxExpandTermHome = new wxCheckBox(viewsPanel, ID_DIALOG_OPTIONS_EXPAND_TERM_HOME
        , _("Term Accounts"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemCheckBoxExpandTermHome->SetValue(GetIniDatabaseCheckboxValue("EXPAND_TERM_HOME",false));
    itemCheckBoxExpandTermHome->SetToolTip(_("Expand Term Accounts on home page when page is refreshed"));
    homePageStaticBoxSizer->Add(itemCheckBoxExpandTermHome, flags);

    // Expand Stock Home
    wxCheckBox* itemCheckBoxExpandStockHome = new wxCheckBox(viewsPanel, ID_DIALOG_OPTIONS_EXPAND_STOCK_HOME
        , _("Stock Accounts"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemCheckBoxExpandStockHome->SetValue(GetIniDatabaseCheckboxValue("ENABLESTOCKS",true));
    itemCheckBoxExpandStockHome->SetToolTip(_("Expand Stock Accounts on home page when page is refreshed"));
    homePageStaticBoxSizer->Add(itemCheckBoxExpandStockHome, flags);

    cbBudgetFinancialYears_ = new wxCheckBox(viewsPanel, wxID_STATIC, _("View Budgets as Financial Years")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    cbBudgetFinancialYears_->SetValue(GetIniDatabaseCheckboxValue(INIDB_BUDGET_FINANCIAL_YEARS, false));
    viewsPanelSizer->Add(cbBudgetFinancialYears_, flags);

    cbBudgetIncludeTransfers_ = new wxCheckBox(viewsPanel, wxID_STATIC
        , _("View Budgets with 'transfer' transactions")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    cbBudgetIncludeTransfers_->SetValue(GetIniDatabaseCheckboxValue(INIDB_BUDGET_INCLUDE_TRANSFERS, false));
    viewsPanelSizer->Add(cbBudgetIncludeTransfers_, flags);

    cbBudgetSetupWithoutSummary_ = new wxCheckBox(viewsPanel, wxID_STATIC
        , _("View Budgets Setup Without Budget Summaries")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    cbBudgetSetupWithoutSummary_->SetValue(GetIniDatabaseCheckboxValue(INIDB_BUDGET_SETUP_WITHOUT_SUMMARY, false));
    viewsPanelSizer->Add(cbBudgetSetupWithoutSummary_, flags);

    cbBudgetSummaryWithoutCateg_ = new wxCheckBox(viewsPanel, wxID_STATIC
        , _("View Budget Summary Report without Categories")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    cbBudgetSummaryWithoutCateg_->SetValue(GetIniDatabaseCheckboxValue(INIDB_BUDGET_SUMMARY_WITHOUT_CATEG, true));
    viewsPanelSizer->Add(cbBudgetSummaryWithoutCateg_, flags);

    cbIgnoreFutureTransactions_ = new wxCheckBox(viewsPanel, wxID_STATIC
        , _("View Reports without Future Transactions")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    cbIgnoreFutureTransactions_->SetValue(GetIniDatabaseCheckboxValue(INIDB_IGNORE_FUTURE_TRANSACTIONS, false));
    viewsPanelSizer->Add(cbIgnoreFutureTransactions_, flags);

    wxStaticBox* userColourSettingStBox = new wxStaticBox(viewsPanel, wxID_ANY, _("User Colors"));
    userColourSettingStBox->SetFont(staticBoxFontSetting);
    wxStaticBoxSizer* userColourSettingStBoxSizer = new wxStaticBoxSizer(userColourSettingStBox, wxHORIZONTAL);
    viewsPanelSizer->Add(userColourSettingStBoxSizer, 0, wxALL | wxCENTER, 0);

	int size_x = baseCurrencyButton->GetSize().GetY();
    UDFCB1_ = new wxButton(viewsPanel, wxID_HIGHEST + 11
        , _("1"), wxDefaultPosition, wxSize(size_x, -1), 0);
    UDFCB1_->SetBackgroundColour(mmColors::userDefColor1);
    userColourSettingStBoxSizer->Add(UDFCB1_, flags);

    UDFCB2_ = new wxButton(viewsPanel, wxID_HIGHEST + 22
        , _("2"), wxDefaultPosition, wxSize(size_x, -1), 0);
    UDFCB2_->SetBackgroundColour(mmColors::userDefColor2);
    userColourSettingStBoxSizer->Add(UDFCB2_, flags);

    UDFCB3_ = new wxButton(viewsPanel, wxID_HIGHEST + 33
        , _("3"), wxDefaultPosition, wxSize(size_x, -1), 0);
    UDFCB3_->SetBackgroundColour(mmColors::userDefColor3);
    userColourSettingStBoxSizer->Add(UDFCB3_, flags);

    UDFCB4_ = new wxButton(viewsPanel, wxID_HIGHEST + 44
        , _("4"), wxDefaultPosition, wxSize(size_x, -1), 0);
    UDFCB4_->SetBackgroundColour(mmColors::userDefColor4);
    userColourSettingStBoxSizer->Add(UDFCB4_, flags);

    UDFCB5_ = new wxButton(viewsPanel, wxID_HIGHEST + 55
        , _("5"), wxDefaultPosition, wxSize(size_x, -1), 0);
    UDFCB5_->SetBackgroundColour(mmColors::userDefColor5);
    userColourSettingStBoxSizer->Add(UDFCB5_, flags);

    UDFCB6_ = new wxButton(viewsPanel, wxID_HIGHEST + 66
        , _("6"), wxDefaultPosition, wxSize(size_x, -1), 0);
    UDFCB6_->SetBackgroundColour(mmColors::userDefColor6);
    userColourSettingStBoxSizer->Add(UDFCB6_, flags);

    UDFCB7_ = new wxButton(viewsPanel, wxID_HIGHEST + 77
        , _("7"), wxDefaultPosition, wxSize(size_x, -1), 0);
    UDFCB7_->SetBackgroundColour(mmColors::userDefColor7);
    userColourSettingStBoxSizer->Add(UDFCB7_, flags);

    viewsPanel->Connect(wxID_ANY, wxEVT_COMMAND_BUTTON_CLICKED
        , wxCommandEventHandler(mmOptionsDialog::OnNavTreeColorChanged), NULL, this);

    /*********************************************************************************************
     Attachments Panel
    **********************************************************************************************/

    wxPanel* attachmentPanel = new wxPanel(newBook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    wxBoxSizer* attachmentPanelSizer = new wxBoxSizer(wxVERTICAL);
    attachmentPanel->SetSizer(attachmentPanelSizer);

    //Attachments
    wxStaticBox* attachmentStaticBox = new wxStaticBox(attachmentPanel, wxID_ANY, _("Attachments Settings"));
    attachmentStaticBox->SetFont(staticBoxFontSetting);
    wxStaticBoxSizer* attachmentStaticBoxSizer = new wxStaticBoxSizer(attachmentStaticBox, wxVERTICAL);

    attachmentPanelSizer->Add(attachmentStaticBoxSizer, flagsExpand);

    wxStaticText* attachmentStaticText = new wxStaticText(attachmentPanel
        , wxID_STATIC, _("Attachment archive folder"));
    attachmentStaticBoxSizer->Add(attachmentStaticText, flags);

    wxBoxSizer* attachRadioButtonSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* attachDefinedSizer = new wxBoxSizer(wxHORIZONTAL);

    attachmentStaticBoxSizer->Add(attachRadioButtonSizer);
    attachmentStaticBoxSizer->Add(attachDefinedSizer);
    attachmentStaticBoxSizer->AddSpacer(5);

    wxString attachmentFolder = Model_Infotable::instance().GetStringInfo("ATTACHMENTSFOLDER:" + mmPlatformType(), "");
    wxString LastDBPath = Model_Setting::instance().getLastDbPath();
    wxFileName fn(LastDBPath);
    wxString LastDBFileName = fn.FileName(LastDBPath).GetName();
    wxString LastDBFolder = fn.FileName(LastDBPath).GetPath();

    wxRadioButton* attachmentRadioButton1 = new wxRadioButton(attachmentPanel
        , ID_DIALOG_OPTIONS_RADIOBUTTON_ATTACHMENT_1
        , _("System documents directory"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    attachmentRadioButton1->SetToolTip(_("Subfolder") + " 'MMEX_" + LastDBFileName + "_Attachments' " + "\n"
        + _("into ") + wxStandardPaths::Get().GetDocumentsDir() + "\n"
        + "\n"
        + _("MAKE ATTENTION:") + "\n"
        + _("It won't work if you use more than one DB with the same filename!"));
    if (attachmentFolder == INIDB_ATTACHMENTS_FOLDER_DOCUMENTSDIR) attachmentRadioButton1->SetValue(true);

    wxRadioButton* attachmentRadioButton2 = new wxRadioButton(attachmentPanel, ID_DIALOG_OPTIONS_RADIOBUTTON_ATTACHMENT_2, _("Money Manager directory"));
    attachmentRadioButton2->SetToolTip(_("Subfolder") + " 'attachments_" + LastDBFileName + "' " + "\n"
        + _("into ") + mmex::getPathUser(mmex::DIRECTORY) + "\n"
        + "\n"
        + _("MAKE ATTENTION:") + "\n"
        + _("It won't work if you use more than one DB with the same filename!"));
    if (attachmentFolder == INIDB_ATTACHMENTS_FOLDER_MMEXDIR) attachmentRadioButton2->SetValue(true);

    wxRadioButton* attachmentRadioButton3 = new wxRadioButton(attachmentPanel, ID_DIALOG_OPTIONS_RADIOBUTTON_ATTACHMENT_3, _("Database file directory"));
    attachmentRadioButton3->SetToolTip(_("Subfolder") + " 'Attachments_" + LastDBFileName + "' " + "\n"
        + _("into ") + LastDBFolder);
    if (attachmentFolder == INIDB_ATTACHMENTS_FOLDER_DBDIR) attachmentRadioButton3->SetValue(true);

    wxRadioButton* attachmentRadioButtonU = new wxRadioButton(attachmentPanel, ID_DIALOG_OPTIONS_RADIOBUTTON_ATTACHMENT_USER, _("User Defined"));
    attachmentRadioButtonU->SetToolTip(_("Specify the folder to use when archive attachments"));
    wxTextCtrl* textAttachment = new wxTextCtrl(attachmentPanel, ID_DIALOG_OPTIONS_TEXTCTRL_ATTACHMENT, attachmentFolder, wxDefaultPosition, wxSize(225, -1), 0);
    wxButton* AttachmentsFolderButton = new wxButton(attachmentPanel, ID_DIALOG_OPTIONS_BUTTON_ATTACHMENTSFOLDER, "...", wxDefaultPosition, wxSize(25, -1), 0);
    AttachmentsFolderButton->SetToolTip(_("Browse for folder"));

    if (attachmentFolder == INIDB_ATTACHMENTS_FOLDER_DOCUMENTSDIR
        || attachmentFolder == INIDB_ATTACHMENTS_FOLDER_MMEXDIR
        || attachmentFolder == INIDB_ATTACHMENTS_FOLDER_DBDIR)
    {
        textAttachment->Enable(false);
        AttachmentsFolderButton->Enable(false);
    }
    else
    {
        attachmentRadioButtonU->SetValue(true);
        textAttachment->Enable(true);
        textAttachment->SetToolTip(attachmentFolder);
    }

    attachRadioButtonSizer->Add(attachmentRadioButton1, flags);
    attachRadioButtonSizer->Add(attachmentRadioButton2, flags);
    attachRadioButtonSizer->Add(attachmentRadioButton3, flags);

    attachDefinedSizer->Add(attachmentRadioButtonU, flags);
    attachDefinedSizer->Add(textAttachment, flags);
    attachDefinedSizer->Add(AttachmentsFolderButton, flags);

    //TODO: Add a line to separate radio from other settings
    attachmentStaticBoxSizer->AddSpacer(10);

    cbDeleteAttachments_ = new wxCheckBox(attachmentPanel, wxID_STATIC, _("Delete file after import"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    cbDeleteAttachments_->SetValue(Model_Infotable::instance().GetBoolInfo("ATTACHMENTSDELETE", false));
    cbDeleteAttachments_->SetToolTip(_("Select to delete file after import in attachments archive"));
    attachmentStaticBoxSizer->Add(cbDeleteAttachments_, flags);

    cbTrashAttachments_ = new wxCheckBox(attachmentPanel, wxID_STATIC, _("When remove attachment, move file instead of delete"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    cbTrashAttachments_->SetValue(Model_Infotable::instance().GetBoolInfo("ATTACHMENTSTRASH", false));
    cbTrashAttachments_->SetToolTip(_("Select to don't delete file when attachment is removed, but instead move it to 'Deleted' subfolder"));
    attachmentStaticBoxSizer->Add(cbTrashAttachments_, flags);

    /*********************************************************************************************
     Others Panel
    **********************************************************************************************/
    wxPanel* othersPanel = new wxPanel(newBook, ID_BOOK_PANELMISC
        , wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    wxBoxSizer* othersPanelSizer = new wxBoxSizer(wxVERTICAL);
    othersPanel->SetSizer(othersPanelSizer);

    wxStaticText* itemStaticTextURL = new wxStaticText(othersPanel, wxID_STATIC, _("Stock Quote Web Page"));
    itemStaticTextURL->SetFont(staticBoxFontSetting);
    othersPanelSizer->Add(itemStaticTextURL, flags);

    wxString stockURL = Model_Infotable::instance().GetStringInfo("STOCKURL", mmex::DEFSTOCKURL);
    wxTextCtrl* itemTextCtrURL = new wxTextCtrl(othersPanel
        , ID_DIALOG_OPTIONS_TEXTCTRL_STOCKURL, stockURL);
    othersPanelSizer->Add(itemTextCtrURL, flagsExpand);
    itemTextCtrURL->SetToolTip(_("Clear the field to Reset the value to system default."));
    othersPanelSizer->AddSpacer(15);

    // New transaction dialog settings
    wxStaticBox* transSettingsStaticBox = new wxStaticBox(othersPanel, wxID_STATIC,
        _("New Transaction Dialog Settings"));
    transSettingsStaticBox->SetFont(staticBoxFontSetting);

    wxStaticBoxSizer* transSettingsStaticBoxSizer = new wxStaticBoxSizer(transSettingsStaticBox, wxVERTICAL);
    othersPanelSizer->Add(transSettingsStaticBoxSizer, flagsExpand);

    wxArrayString defaultValues_;
    defaultValues_.Add(_("None"));
    defaultValues_.Add(_("Last Used"));

    wxChoice* defaultDateChoice = new wxChoice(othersPanel
        , ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_DATE
        , wxDefaultPosition, wxSize(140, -1), defaultValues_);
    defaultDateChoice->SetSelection(mmIniOptions::instance().transDateDefault_);

    wxChoice* defaultPayeeChoice = new wxChoice(othersPanel
        , ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_PAYEE
        , wxDefaultPosition, wxSize(140, -1), defaultValues_);
    defaultPayeeChoice->SetSelection(mmIniOptions::instance().transPayeeSelectionNone_);

    defaultValues_[1]=(_("Last used for payee"));

    wxChoice* defaultCategoryChoice = new wxChoice(othersPanel
        , ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_CATEGORY
        , wxDefaultPosition, defaultPayeeChoice->GetSize(), defaultValues_);
    defaultCategoryChoice->SetSelection(mmIniOptions::instance().transCategorySelectionNone_);

    wxChoice* default_status = new wxChoice(othersPanel
        , ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_STATUS
        , wxDefaultPosition, defaultDateChoice->GetSize());

    for(const auto& i: Model_Checking::all_status())
        default_status->Append(wxGetTranslation(i), new wxStringClientData(i));

    default_status->SetSelection(mmIniOptions::instance().transStatusReconciled_);

    wxFlexGridSizer* newTransflexGridSizer = new wxFlexGridSizer(0,2,0,0);
    transSettingsStaticBoxSizer->Add(newTransflexGridSizer);
    newTransflexGridSizer->Add(new wxStaticText(othersPanel, wxID_STATIC, _("Default Date:")), flags);
    newTransflexGridSizer->Add(defaultDateChoice,    flags);
    newTransflexGridSizer->Add(new wxStaticText(othersPanel, wxID_STATIC, _("Default Payee:")), flags);
    newTransflexGridSizer->Add(defaultPayeeChoice,   flags);
    newTransflexGridSizer->Add(new wxStaticText(othersPanel, wxID_STATIC, _("Default Category:")), flags);
    newTransflexGridSizer->Add(defaultCategoryChoice,flags);
    newTransflexGridSizer->Add(new wxStaticText(othersPanel, wxID_STATIC, _("Default Status:")), flags);
    newTransflexGridSizer->Add(default_status,  flags);

    //----------------------------------------------
    //a bit more space visual appearance
    othersPanelSizer->AddSpacer(10);

    wxBoxSizer* itemBoxSizerStockURL = new wxBoxSizer(wxVERTICAL);
    othersPanelSizer->Add(itemBoxSizerStockURL);

    // Backup Settings
    wxStaticBox* backupStaticBox = new wxStaticBox(othersPanel, wxID_STATIC, _("Database Backup"));
    backupStaticBox->SetFont(staticBoxFontSetting);
    wxStaticBoxSizer* backupStaticBoxSizer = new wxStaticBoxSizer(backupStaticBox, wxVERTICAL);
    othersPanelSizer->Add(backupStaticBoxSizer, flagsExpand);

    wxCheckBox* backupCheckBox = new wxCheckBox(othersPanel, ID_DIALOG_OPTIONS_CHK_BACKUP
        , _("Create a new backup when MMEX Start"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    backupCheckBox->SetValue(GetIniDatabaseCheckboxValue("BACKUPDB",false));
    backupCheckBox->SetToolTip(_("When MMEX Starts,\ncreates the backup database: dbFile_start_YYYY-MM-DD.ext."));
    backupStaticBoxSizer->Add(backupCheckBox, flags);

    wxCheckBox* backupUpdateCheckBox = new wxCheckBox(othersPanel, ID_DIALOG_OPTIONS_CHK_BACKUP_UPDATE
        , _("Backup database on exit."), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    backupUpdateCheckBox->SetValue(GetIniDatabaseCheckboxValue("BACKUPDB_UPDATE",false));
    backupUpdateCheckBox->SetToolTip(_("When MMEX shuts down and changes made to database,\ncreates or updates the backup database: dbFile_update_YYYY-MM-DD.ext."));
    backupStaticBoxSizer->Add(backupUpdateCheckBox, flags);

    int max = Model_Setting::instance().GetIntSetting("MAX_BACKUP_FILES", 4);
    scMax_files_ = new wxSpinCtrl(othersPanel, wxID_ANY
        , wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 999, max);
    scMax_files_->SetToolTip(_("Specify max number of backup files"));

    wxFlexGridSizer* flex_sizer2 = new wxFlexGridSizer(0,2,0,0);
    flex_sizer2->Add(new wxStaticText(othersPanel, wxID_STATIC, _("Max Files")), flags);
    flex_sizer2->Add(scMax_files_, flags);
    backupStaticBoxSizer->Add(flex_sizer2);

    //CSV Import
    const wxString delimiter = Model_Infotable::instance().GetStringInfo("DELIMITER", mmex::DEFDELIMTER);

    wxStaticBox* csvStaticBox = new wxStaticBox(othersPanel, wxID_ANY, _("CSV Settings"));
    csvStaticBox->SetFont(staticBoxFontSetting);
    wxStaticBoxSizer* csvStaticBoxSizer = new wxStaticBoxSizer(csvStaticBox, wxVERTICAL);

    othersPanelSizer->Add(csvStaticBoxSizer, flagsExpand);
    wxFlexGridSizer* csvStaticBoxSizerGrid = new wxFlexGridSizer(0, 2, 0, 10);
    csvStaticBoxSizer->Add(csvStaticBoxSizerGrid, flags);

    csvStaticBoxSizerGrid->Add(new wxStaticText(othersPanel, wxID_STATIC, _("Delimiter")), flags);
    wxTextCtrl* textDelimiter4 = new wxTextCtrl(othersPanel
        , ID_DIALOG_OPTIONS_TEXTCTRL_DELIMITER4, delimiter);
    textDelimiter4->SetToolTip(_("Specify the delimiter to use when importing/exporting CSV files"));
    textDelimiter4->SetMaxLength(1);
    csvStaticBoxSizerGrid->Add(textDelimiter4, flags);

    /*********************************************************************************************
    Network Panel
    **********************************************************************************************/
    wxPanel* networkPanel = new wxPanel(newBook
        , ID_BOOK_PANEL_EXP_IMP, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

    wxBoxSizer* networkPanelSizer = new wxBoxSizer(wxVERTICAL);
    networkPanel->SetSizer(networkPanelSizer);

    //WebApp settings
    wxStaticBox* WebAppStaticBox = new wxStaticBox(networkPanel, wxID_STATIC, _("WebApp Settings"));
    WebAppStaticBox->SetFont(staticBoxFontSetting);
    wxStaticBoxSizer* WebAppStaticBoxSizer = new wxStaticBoxSizer(WebAppStaticBox, wxVERTICAL);
    wxFlexGridSizer* WebAppStaticBoxSizerGrid = new wxFlexGridSizer(0, 2, 0, 10);
    networkPanelSizer->Add(WebAppStaticBoxSizer, flagsExpand);
    WebAppStaticBoxSizer->Add(WebAppStaticBoxSizerGrid, flagsExpand);

    WebAppStaticBoxSizerGrid->Add(new wxStaticText(networkPanel, wxID_STATIC, _("Url")), flags);
    wxString WebAppURL = Model_Infotable::instance().GetStringInfo("WEBAPPURL", "");
    wxTextCtrl* WebAppURLTextCtr = new wxTextCtrl(networkPanel, ID_DIALOG_OPTIONS_TEXTCTRL_WEBAPPURL,
        WebAppURL, wxDefaultPosition, wxSize(300, -1));
    WebAppURLTextCtr->SetToolTip(_("Specify the Web App URL without final slash"));
    WebAppStaticBoxSizerGrid->Add(WebAppURLTextCtr, 1, wxEXPAND | wxALL, 5);

    WebAppStaticBoxSizerGrid->Add(new wxStaticText(networkPanel, wxID_STATIC, _("GUID")), flags);
    wxString WebAppGUID = Model_Infotable::instance().GetStringInfo("WEBAPPGUID", "");
    wxTextCtrl* WebAppGUIDTextCtr = new wxTextCtrl(networkPanel, ID_DIALOG_OPTIONS_TEXTCTRL_WEBAPPGUID,
        WebAppGUID, wxDefaultPosition, wxSize(300, -1));
    WebAppGUIDTextCtr->SetToolTip(_("Specify the Web App GUID"));
    WebAppStaticBoxSizerGrid->Add(WebAppGUIDTextCtr, 1, wxEXPAND | wxALL, 5);

    // Proxy Settings
    networkPanelSizer->AddSpacer(15);

    wxStaticBox* proxyStaticBox = new wxStaticBox(networkPanel, wxID_STATIC, _("Proxy Settings"));
    proxyStaticBox->SetFont(staticBoxFontSetting);
    wxStaticBoxSizer* proxyStaticBoxSizer = new wxStaticBoxSizer(proxyStaticBox, wxVERTICAL);
    networkPanelSizer->Add(proxyStaticBoxSizer, flagsExpand);

    wxString proxyName = Model_Setting::instance().GetStringSetting("PROXYIP", "");
    wxTextCtrl* proxyNameTextCtr = new wxTextCtrl(networkPanel, ID_DIALOG_OPTIONS_TEXTCTRL_PROXY
        , proxyName, wxDefaultPosition, wxSize(150, -1));
    proxyNameTextCtr->SetToolTip(_("Specify the proxy IP address"));

    int proxyPort = Model_Setting::instance().GetIntSetting("PROXYPORT", 0);
    scProxyPort_ = new wxSpinCtrl(networkPanel, wxID_ANY,
        wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 65535, proxyPort);
    scProxyPort_->SetToolTip(_("Specify proxy port number"));

    wxFlexGridSizer* flex_sizer3 = new wxFlexGridSizer(0, 4, 0, 0);
    flex_sizer3->Add(new wxStaticText(networkPanel, wxID_STATIC, _("Proxy")), flags);
    flex_sizer3->Add(proxyNameTextCtr, flags);
    flex_sizer3->Add(new wxStaticText(networkPanel, wxID_STATIC, _("Port")), flags);
    flex_sizer3->Add(scProxyPort_, flags);

    proxyStaticBoxSizer->Add(flex_sizer3, flags);

    // Web Server Settings
    networkPanelSizer->AddSpacer(15);

    wxStaticBox* webserverStaticBox = new wxStaticBox(networkPanel, wxID_STATIC, _("Web Server"));
    webserverStaticBox->SetFont(staticBoxFontSetting);
    wxStaticBoxSizer* webserverStaticBoxSizer = new wxStaticBoxSizer(webserverStaticBox, wxVERTICAL);
    networkPanelSizer->Add(webserverStaticBoxSizer, flagsExpand);

    cbWebServerCheckBox_ = new wxCheckBox(networkPanel, ID_DIALOG_OPTIONS_ENABLE_WEB_SERVER
        , _("Enable"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    cbWebServerCheckBox_->SetValue(GetIniDatabaseCheckboxValue("ENABLEWEBSERVER", true));
    cbWebServerCheckBox_->SetToolTip(_("Enable internal web server when MMEX Starts."));

    int webserverPort = Model_Setting::instance().GetIntSetting("WEBSERVERPORT", 8080);
    scWebServerPort_ = new wxSpinCtrl(networkPanel, ID_DIALOG_OPTIONS_WEB_SERVER_PORT,
        wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 65535, webserverPort);
    scWebServerPort_->SetToolTip(_("Specify web server port number"));

    wxFlexGridSizer* flex_sizer4 = new wxFlexGridSizer(0, 4, 0, 0);
    flex_sizer4->Add(cbWebServerCheckBox_, flags);
    flex_sizer4->AddSpacer(45);
    flex_sizer4->Add(new wxStaticText(networkPanel, wxID_STATIC, _("Port")), flags);
    flex_sizer4->Add(scWebServerPort_, flags);

    webserverStaticBoxSizer->Add(flex_sizer4, flags);

    networkPanel->SetSizer(networkPanelSizer);

   /**********************************************************************************************
    Setting up the notebook with the 5 pages
    **********************************************************************************************/
    newBook->SetImageList(m_imageList);

    newBook->InsertPage(0, generalPanel, _("General"), true, 2);
    newBook->InsertPage(1, viewsPanel, _("View Options"), false, 0);
    newBook->InsertPage(2, attachmentPanel, _("Attachments"), false, 1);
    newBook->InsertPage(3, networkPanel, _("Network"), false, 4);
    newBook->InsertPage(4, othersPanel, _("Others"), false, 3);

    mainDialogPanelSizer->Add(newBook, 1, wxGROW|wxALL, 5);
    mainDialogPanelSizer->Layout();

   /**********************************************************************************************
    Button Panel with OK and Cancel Buttons
    **********************************************************************************************/
    wxStaticLine* panelSeparatorLine = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    mainDialogSizer->Add(panelSeparatorLine,0,wxGROW|wxLEFT|wxRIGHT, 10);

    wxPanel* buttonPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    wxBoxSizer* buttonPanelSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonPanel->SetSizer(buttonPanelSizer);
    mainDialogSizer->Add(buttonPanel, 0, wxALIGN_RIGHT|wxALL, 5);

    wxButton* itemButtonOK = new wxButton(buttonPanel, wxID_OK, _("&OK "));
    wxButton* itemButtonApply = new wxButton(buttonPanel, wxID_APPLY, _("&Apply"));
    wxButton* itemButtonCancel = new wxButton(buttonPanel, wxID_CANCEL, _("&Cancel "));
    buttonPanelSizer->Add(itemButtonOK, 0, wxALIGN_RIGHT|wxRIGHT, 5);
    buttonPanelSizer->Add(itemButtonApply, wxALIGN_RIGHT|wxRIGHT, 5);
    buttonPanelSizer->Add(itemButtonCancel, 0, wxALIGN_RIGHT|wxRIGHT, 5);
    itemButtonOK->SetFocus();
}

void mmOptionsDialog::OnLanguageChanged(wxCommandEvent& /*event*/)
{
    wxString lang = mmSelectLanguage(this, true, false);
    if (lang.empty()) return;

    wxButton *btn = (wxButton*)FindWindow(ID_DIALOG_OPTIONS_BUTTON_LANGUAGE);
    wxASSERT(btn);
    btn->SetLabel(lang.Left(1).Upper() + lang.SubString(1,lang.Len()));
}

void mmOptionsDialog::OnCurrency(wxCommandEvent& /*event*/)
{
    int currencyID = Model_Infotable::instance().GetBaseCurrencyId();

    if (mmMainCurrencyDialog::Execute(this, currencyID) && currencyID != -1)
    {
        Model_Currency::Data* currency = Model_Currency::instance().get(currencyID);
        wxButton* bn = (wxButton*)FindWindow(ID_DIALOG_OPTIONS_BUTTON_CURRENCY);
        bn->SetLabel(currency->CURRENCYNAME);
        currencyId_ = currencyID;

        wxMessageDialog msgDlg(this, _("Remember to update currency rate"), _("Important note"));
        msgDlg.ShowModal();
    }
}

void mmOptionsDialog::OnDateFormatChanged(wxCommandEvent& /*event*/)
{
    wxStringClientData* data = (wxStringClientData*)(choiceDateFormat_->GetClientObject(choiceDateFormat_->GetSelection()));
    if (data)
    {
        dateFormat_ = data->GetData();
        mmOptions::instance().dateFormat_ = dateFormat_;
        sampleDateText_->SetLabel(wxDateTime::Now().Format(dateFormat_));
    }
    else
        return;
}

void mmOptionsDialog::OnNavTreeColorChanged(wxCommandEvent& event)
{
    int buttonId = event.GetId();
    wxButton* button = (wxButton*)FindWindow(buttonId);
    if (!button) return;

    wxColour colour = button->GetBackgroundColour();
    wxColourData data;
    data.SetChooseFull(true);
    data.SetColour(colour);

    wxColourDialog dialog(this, &data);
    if (dialog.ShowModal() == wxID_OK)
    {
        colour = dialog.GetColourData().GetColour();
        button->SetBackgroundColour(colour);
    }
}

bool mmOptionsDialog::GetIniDatabaseCheckboxValue(wxString dbField, bool defaultState)
{
    bool result = Model_Setting::instance().GetBoolSetting(dbField, defaultState);
    return result;
}

void mmOptionsDialog::OnAttachmentSelected(wxCommandEvent& event)
{
    int id = event.GetId();
    wxTextCtrl* st = (wxTextCtrl*)FindWindow(ID_DIALOG_OPTIONS_TEXTCTRL_ATTACHMENT);
    wxRadioButton* d = (wxRadioButton*) FindWindow(id);
    if (d) {
        if (id == ID_DIALOG_OPTIONS_RADIOBUTTON_ATTACHMENT_1)
        {
            st->SetValue(INIDB_ATTACHMENTS_FOLDER_DOCUMENTSDIR);
            st->Enable(false);
        }
        else if (id == ID_DIALOG_OPTIONS_RADIOBUTTON_ATTACHMENT_2)
        {
            st->SetValue(INIDB_ATTACHMENTS_FOLDER_MMEXDIR);
            st->Enable(false);
        }
        else if (id == ID_DIALOG_OPTIONS_RADIOBUTTON_ATTACHMENT_3)
        {
            st->SetValue(INIDB_ATTACHMENTS_FOLDER_DBDIR);
            st->Enable(false);
        }
        else if (id == ID_DIALOG_OPTIONS_RADIOBUTTON_ATTACHMENT_USER)
        {
            const wxString attachmentFolder = Model_Infotable::instance().GetStringInfo("ATTACHMENTSFOLDER:" + mmPlatformType(), "");
			if (attachmentFolder == INIDB_ATTACHMENTS_FOLDER_DOCUMENTSDIR
				|| attachmentFolder == INIDB_ATTACHMENTS_FOLDER_MMEXDIR
				|| attachmentFolder == INIDB_ATTACHMENTS_FOLDER_DBDIR)
			st->SetValue(wxEmptyString);
			else
            st->SetValue(attachmentFolder);

			st->Enable(true);
        }
    }

	wxButton* button = (wxButton*)FindWindow(ID_DIALOG_OPTIONS_BUTTON_ATTACHMENTSFOLDER);
	button->Enable(true);
}

void mmOptionsDialog::OnAttachmentsFolderChanged(wxCommandEvent& /*event*/)
{
	wxTextCtrl* att = (wxTextCtrl*)FindWindow(ID_DIALOG_OPTIONS_TEXTCTRL_ATTACHMENT);
	wxString attachmentFolder = att->GetValue();

	wxDirDialog dlg(this
		, _("Choose folder to set as attachments archive")
		, attachmentFolder
		, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST
		);

	if (dlg.ShowModal() != wxID_OK)
		return;

	wxString AttachmentsFolder = dlg.GetPath();

	att->SetValue(AttachmentsFolder);
	att->SetToolTip(AttachmentsFolder);
}

void mmOptionsDialog::SaveViewAccountOptions()
{
    int selection = choiceVisible_->GetSelection();
    int row_id_ = 0;
    wxArrayString viewAcct = viewAccountStrings(false, wxEmptyString, row_id_);
    Model_Setting::instance().Set("VIEWACCOUNTS", viewAcct[selection]);
}

void mmOptionsDialog::SaveViewTransactionOptions()
{
    wxString visible = VIEW_TRANS_ALL_STR;
    wxStringClientData* visible_obj = (wxStringClientData *)choiceTransVisible_->GetClientObject(choiceTransVisible_->GetSelection());
    if (visible_obj)
    {
        visible = visible_obj->GetData();
    }
    Model_Setting::instance().Set("VIEWTRANSACTIONS", visible);
}

void mmOptionsDialog::SaveFinancialYearStart()
{
    //Save Financial Year Start Day
    wxSpinCtrl* fysDay = (wxSpinCtrl*)FindWindow(ID_DIALOG_OPTIONS_FINANCIAL_YEAR_START_DAY);
    wxString fysDayVal = wxString::Format("%d",fysDay->GetValue());
    mmOptions::instance().financialYearStartDayString_ = fysDayVal;
    Model_Infotable::instance().Set("FINANCIAL_YEAR_START_DAY", fysDayVal);

    //Save Financial Year Start Month
    wxString fysMonthVal = wxString() << monthSelection_->GetSelection() + 1;
    mmOptions::instance().financialYearStartMonthString_ = fysMonthVal;
    Model_Infotable::instance().Set("FINANCIAL_YEAR_START_MONTH", fysMonthVal);
}

void mmOptionsDialog::SaveStocksUrl()
{
    wxTextCtrl* url = (wxTextCtrl*)FindWindow(ID_DIALOG_OPTIONS_TEXTCTRL_STOCKURL);
    wxString stockURL = url->GetValue();
    if (!stockURL.IsEmpty())
    {
        Model_Infotable::instance().Set("STOCKURL", stockURL);
    }
    else
    {
        Model_Infotable::Data_Set items = Model_Infotable::instance().find(Model_Infotable::INFONAME("STOCKURL"));
        if (!items.empty())
            Model_Infotable::instance().remove(items[0].INFOID);
    }
}

/// Saves the updated System Options to the appropriate databases.
void mmOptionsDialog::SaveNewSystemSettings()
{
    // initialize database saves -------------------------------------------------------------
    
    Model_Setting::instance().Begin();

    // Save all the details for all the panels
    SaveGeneralPanelSettings();
    SaveViewPanelSettings();
    SaveAttachmentPanelSettings();
    SaveOthersPanelSettings();
    SaveNetworkPanelSettings();

    // finalise database saves ---------------------------------------------------------------
    Model_Setting::instance().Commit();
}

void mmOptionsDialog::SaveGeneralPanelSettings()
{
    wxTextCtrl* stun = (wxTextCtrl*)FindWindow(ID_DIALOG_OPTIONS_TEXTCTRL_USERNAME);
    mmOptions::instance().userNameString_ = stun->GetValue();
    Model_Infotable::instance().Set("USERNAME", mmOptions::instance().userNameString_);

    wxButton *languageButton = (wxButton*)FindWindow(ID_DIALOG_OPTIONS_BUTTON_LANGUAGE);
    Model_Setting::instance().Set(LANGUAGE_PARAMETER, languageButton->GetLabel().Lower());
    mmSelectLanguage(this, false);

    Model_Infotable::instance().SetBaseCurrencyID(currencyId_);
    Model_Infotable::instance().Set("DATEFORMAT", dateFormat_);
    SaveFinancialYearStart();

    Model_Setting::instance().Set(INIDB_USE_ORG_DATE_COPYPASTE, cbUseOrgDateCopyPaste_->GetValue());
    Model_Setting::instance().Set(INIDB_USE_TRANSACTION_SOUND, cbUseSound_->GetValue());
}

void mmOptionsDialog::SaveViewPanelSettings()
{
    SaveViewAccountOptions();
    SaveViewTransactionOptions();

    int size = choiceFontSize_->GetCurrentSelection() + 1;
    mmIniOptions::instance().html_font_size_ = size;
    Model_Setting::instance().Set("HTMLFONTSIZE", size);

    wxCheckBox* itemCheckBox = (wxCheckBox*)FindWindow(ID_DIALOG_OPTIONS_EXPAND_BANK_TREE);
    mmIniOptions::instance().expandBankTree_ = itemCheckBox->GetValue();
    Model_Setting::instance().Set("EXPAND_BANK_TREE", itemCheckBox->GetValue() );

    itemCheckBox = (wxCheckBox*)FindWindow(ID_DIALOG_OPTIONS_EXPAND_TERM_TREE);
    mmIniOptions::instance().expandTermTree_ = itemCheckBox->GetValue();
    Model_Setting::instance().Set("EXPAND_TERM_TREE", itemCheckBox->GetValue() );

    itemCheckBox = (wxCheckBox*)FindWindow(ID_DIALOG_OPTIONS_EXPAND_BANK_HOME);
    mmIniOptions::instance().expandBankHome_ = itemCheckBox->GetValue();
    Model_Setting::instance().Set("EXPAND_BANK_HOME", itemCheckBox->GetValue() );

    itemCheckBox = (wxCheckBox*)FindWindow(ID_DIALOG_OPTIONS_EXPAND_TERM_HOME);
    mmIniOptions::instance().expandTermHome_ = itemCheckBox->GetValue();
    Model_Setting::instance().Set("EXPAND_TERM_HOME", itemCheckBox->GetValue() );

    itemCheckBox = (wxCheckBox*)FindWindow(ID_DIALOG_OPTIONS_EXPAND_STOCK_HOME);
    mmIniOptions::instance().expandStocksHome_ = itemCheckBox->GetValue();
    Model_Setting::instance().Set("ENABLESTOCKS", itemCheckBox->GetValue() );

    mmIniOptions::instance().budgetFinancialYears_ = cbBudgetFinancialYears_->GetValue();
    Model_Setting::instance().Set(INIDB_BUDGET_FINANCIAL_YEARS, mmIniOptions::instance().budgetFinancialYears_);

    mmIniOptions::instance().budgetIncludeTransfers_ = cbBudgetIncludeTransfers_->GetValue();
    Model_Setting::instance().Set(INIDB_BUDGET_INCLUDE_TRANSFERS, mmIniOptions::instance().budgetIncludeTransfers_);

    mmIniOptions::instance().budgetSetupWithoutSummaries_ = cbBudgetSetupWithoutSummary_->GetValue();
    Model_Setting::instance().Set(INIDB_BUDGET_SETUP_WITHOUT_SUMMARY, mmIniOptions::instance().budgetSetupWithoutSummaries_);

    mmIniOptions::instance().budgetSummaryWithoutCategories_ = cbBudgetSummaryWithoutCateg_->GetValue();
    Model_Setting::instance().Set(INIDB_BUDGET_SUMMARY_WITHOUT_CATEG, mmIniOptions::instance().budgetSummaryWithoutCategories_);

    mmIniOptions::instance().ignoreFutureTransactions_ = cbIgnoreFutureTransactions_->GetValue();
    Model_Setting::instance().Set(INIDB_IGNORE_FUTURE_TRANSACTIONS, mmIniOptions::instance().ignoreFutureTransactions_);

    mmColors::userDefColor1 = UDFCB1_->GetBackgroundColour();
    mmColors::userDefColor2 = UDFCB2_->GetBackgroundColour();
    mmColors::userDefColor3 = UDFCB3_->GetBackgroundColour();
    mmColors::userDefColor4 = UDFCB4_->GetBackgroundColour();
    mmColors::userDefColor5 = UDFCB5_->GetBackgroundColour();
    mmColors::userDefColor6 = UDFCB6_->GetBackgroundColour();
    mmColors::userDefColor7 = UDFCB7_->GetBackgroundColour();

    Model_Setting::instance().Set("USER_COLOR1", mmColors::userDefColor1);
    Model_Setting::instance().Set("USER_COLOR2", mmColors::userDefColor2);
    Model_Setting::instance().Set("USER_COLOR3", mmColors::userDefColor3);
    Model_Setting::instance().Set("USER_COLOR4", mmColors::userDefColor4);
    Model_Setting::instance().Set("USER_COLOR5", mmColors::userDefColor5);
    Model_Setting::instance().Set("USER_COLOR6", mmColors::userDefColor6);
    Model_Setting::instance().Set("USER_COLOR7", mmColors::userDefColor7);
}

void mmOptionsDialog::SaveAttachmentPanelSettings()
{
    wxTextCtrl* attTextCtrl = (wxTextCtrl*) FindWindow(ID_DIALOG_OPTIONS_TEXTCTRL_ATTACHMENT);
    wxString attachmentFolder = attTextCtrl->GetValue();
    Model_Infotable::instance().Set("ATTACHMENTSFOLDER:" + mmPlatformType(), attachmentFolder);

    //Create attachments folder
    wxString attachmentFolderPath = mmAttachmentManage::GetAttachmentsFolder();
	if (!wxDirExists(attachmentFolderPath) && attachmentFolder != wxEmptyString)
	{
		wxMkdir(attachmentFolderPath);
		mmAttachmentManage::CreateReadmeFile(attachmentFolderPath);
	}

    Model_Infotable::instance().Set("ATTACHMENTSDELETE", cbDeleteAttachments_->GetValue());
    Model_Infotable::instance().Set("ATTACHMENTSTRASH", cbTrashAttachments_->GetValue());
}

void mmOptionsDialog::SaveOthersPanelSettings()
{
    wxChoice* itemChoice = (wxChoice*)FindWindow(ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_PAYEE);
    mmIniOptions::instance().transPayeeSelectionNone_ = itemChoice->GetSelection();
    Model_Setting::instance().Set("TRANSACTION_PAYEE_NONE", itemChoice->GetSelection());

    itemChoice = (wxChoice*)FindWindow(ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_CATEGORY);
    mmIniOptions::instance().transCategorySelectionNone_ = itemChoice->GetSelection();
    Model_Setting::instance().Set("TRANSACTION_CATEGORY_NONE", itemChoice->GetSelection());

    itemChoice = (wxChoice*)FindWindow(ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_STATUS);
    mmIniOptions::instance().transStatusReconciled_ = itemChoice->GetSelection();
    Model_Setting::instance().Set("TRANSACTION_STATUS_RECONCILED", itemChoice->GetSelection());

    itemChoice = (wxChoice*)FindWindow(ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_DATE);
    mmIniOptions::instance().transDateDefault_ = itemChoice->GetSelection();
    Model_Setting::instance().Set("TRANSACTION_DATE_DEFAULT", itemChoice->GetSelection());

    SaveStocksUrl();

    wxCheckBox* itemCheckBox = (wxCheckBox*)FindWindow(ID_DIALOG_OPTIONS_CHK_BACKUP);
    Model_Setting::instance().Set("BACKUPDB", itemCheckBox->GetValue());

    wxCheckBox* itemCheckBoxUpdate = (wxCheckBox*)FindWindow(ID_DIALOG_OPTIONS_CHK_BACKUP_UPDATE);
    Model_Setting::instance().Set("BACKUPDB_UPDATE", itemCheckBoxUpdate->GetValue());

    Model_Setting::instance().Set("MAX_BACKUP_FILES", scMax_files_->GetValue());

    wxTextCtrl* st = (wxTextCtrl*) FindWindow(ID_DIALOG_OPTIONS_TEXTCTRL_DELIMITER4);
    wxString delim = st->GetValue();
    if (!delim.IsEmpty()) Model_Infotable::instance().Set("DELIMITER", delim);
}

void mmOptionsDialog::SaveNetworkPanelSettings()
{
    wxTextCtrl* proxy = (wxTextCtrl*) FindWindow(ID_DIALOG_OPTIONS_TEXTCTRL_PROXY);
    wxString proxyName = proxy->GetValue();
    Model_Setting::instance().Set("PROXYIP", proxyName);
    Model_Setting::instance().Set("PROXYPORT", scProxyPort_->GetValue());

    wxTextCtrl* WebAppURL = (wxTextCtrl*) FindWindow(ID_DIALOG_OPTIONS_TEXTCTRL_WEBAPPURL);
    Model_Infotable::instance().Set("WEBAPPURL", WebAppURL->GetValue());

    wxTextCtrl* WebAppGUID = (wxTextCtrl*) FindWindow(ID_DIALOG_OPTIONS_TEXTCTRL_WEBAPPGUID);
    Model_Infotable::instance().Set("WEBAPPGUID", WebAppGUID->GetValue());

    Model_Setting::instance().Set("ENABLEWEBSERVER", cbWebServerCheckBox_->GetValue());
    Model_Setting::instance().Set("WEBSERVERPORT", scWebServerPort_->GetValue());
}

void mmOptionsDialog::OnOk(wxCommandEvent& /*event*/)
{
    this->SaveNewSystemSettings();
    EndModal(wxID_OK);
}

void mmOptionsDialog::OnApply(wxCommandEvent& /*event*/)
{
    // TODO save according panel setting
}
