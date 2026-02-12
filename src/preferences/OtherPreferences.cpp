/*******************************************************
Copyright (C) 2014 Stefano Giorgio
Copyright (C) 2020 - 2022 Nikolay Akimov
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

#include "defs.h"
#include <wx/spinctrl.h>

#include "util/util.h"

#include "model/PreferencesModel.h"
#include "model/TransactionModel.h"

#include "OtherPreferences.h"

/*******************************************************/
wxBEGIN_EVENT_TABLE(OtherPreferences, wxPanel)
    EVT_CHECKBOX(ID_DIALOG_OPTIONS_CHK_BACKUP, OtherPreferences::OnBackupChanged)
    EVT_CHECKBOX(ID_DIALOG_OPTIONS_CHK_BACKUP_UPDATE, OtherPreferences::OnBackupChanged)
wxEND_EVENT_TABLE()
/*******************************************************/

OtherPreferences::OtherPreferences()
{
}

OtherPreferences::OtherPreferences(wxWindow *parent
    , wxWindowID id
    , const wxPoint &pos
    , const wxSize &size
    , long style, const wxString &name)
{
    wxPanel::Create(parent, id, pos, size, style, name);
    Create();
}

OtherPreferences::~OtherPreferences()
{
}

void OtherPreferences::Create()
{
    wxBoxSizer* othersPanelSizer0 = new wxBoxSizer(wxVERTICAL);
    SetSizer(othersPanelSizer0);

    wxScrolledWindow* misc_panel = new wxScrolledWindow(this, wxID_ANY);

    wxBoxSizer* othersPanelSizer = new wxBoxSizer(wxVERTICAL);
    misc_panel->SetSizer(othersPanelSizer);
    othersPanelSizer0->Add(misc_panel, wxSizerFlags(g_flagsExpand).Proportion(0));

    wxStaticBox* stockStaticBox = new wxStaticBox(misc_panel, wxID_STATIC, _t("Stocks"));

    wxStaticBoxSizer* stockStaticBoxSizer = new wxStaticBoxSizer(stockStaticBox, wxVERTICAL);
    othersPanelSizer->Add(stockStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    wxStaticText* itemStaticTextURL = new wxStaticText(stockStaticBox, wxID_STATIC, _t("Stock Quote Web Page"));

    stockStaticBoxSizer->Add(itemStaticTextURL, g_flagsV);

    wxArrayString list;
    list.Add(mmex::weblink::DefStockUrl);
    //list.Add("https://www.morningstar.com/stocks/xnas/%s/quote");
    //list.Add("https://www.marketwatch.com/investing/stock/%s");
    //list.Add("https://www.ifcmarkets.co.in/en/market-data/stocks-prices/%s");

    wxString stockURL = InfotableModel::instance().getString("STOCKURL", mmex::weblink::DefStockUrl);
    wxComboBox* itemListOfURL = new wxComboBox(stockStaticBox, ID_DIALOG_OPTIONS_TEXTCTRL_STOCKURL, ""
        , wxDefaultPosition, wxDefaultSize, list);
    itemListOfURL->SetValue(stockURL);

    stockStaticBoxSizer->Add(itemListOfURL, wxSizerFlags(g_flagsExpand).Proportion(0));
    mmToolTip(itemListOfURL, _t("Clear the field to Reset the value to system default."));

    // Share Precision
    wxFlexGridSizer* share_precision_sizer = new wxFlexGridSizer(0, 3, 0, 0);
    share_precision_sizer->Add(new wxStaticText(stockStaticBox, wxID_STATIC, _t("Share Precision")), g_flagsH);

    m_share_precision = new wxSpinCtrl(misc_panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize
        , wxSP_ARROW_KEYS, 2, 10, PreferencesModel::instance().getSharePrecision());
    m_share_precision->SetValue(PreferencesModel::instance().getSharePrecision());
    mmToolTip(m_share_precision, _t("Set the precision for Share prices"));
    share_precision_sizer->Add(m_share_precision, wxSizerFlags(g_flagsExpand).Proportion(0));

    m_refresh_quotes_on_open = new wxCheckBox(stockStaticBox, wxID_REFRESH, _t("Refresh at Startup"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_refresh_quotes_on_open->SetValue(SettingModel::instance().getBool("REFRESH_STOCK_QUOTES_ON_OPEN", false));
    share_precision_sizer->Add(m_refresh_quotes_on_open, wxSizerFlags(g_flagsH).Border(wxLEFT, 20));
    stockStaticBoxSizer->Add(share_precision_sizer, g_flagsBorder1V);

    // Asset Compounding
    wxStaticBox* assetStaticBox = new wxStaticBox(misc_panel, wxID_STATIC, _t("Assets"));
    wxStaticBoxSizer* assetStaticBoxSizer = new wxStaticBoxSizer(assetStaticBox, wxVERTICAL);
    othersPanelSizer->Add(assetStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    wxFlexGridSizer* asset_compounding_sizer = new wxFlexGridSizer(0, 3, 0, 0);
    asset_compounding_sizer->Add(new wxStaticText(assetStaticBox, wxID_STATIC, _t("Asset Compounding Period")), g_flagsH);
    m_asset_compounding = new wxChoice(assetStaticBox, ID_DIALOG_OPTIONS_ASSET_COMPOUNDING);
    for (const auto& a : PreferencesModel::COMPOUNDING_NAME)
        m_asset_compounding->Append(wxGetTranslation(a.second));
    m_asset_compounding->SetSelection(PreferencesModel::instance().getAssetCompounding());
    mmToolTip(m_asset_compounding,
        _t("Select the compounding period for the appreciation/depreciation rate of assets")
    );
    asset_compounding_sizer->Add(m_asset_compounding, wxSizerFlags(g_flagsExpand).Proportion(0));
    //othersPanelSizer->Add(asset_compounding_sizer, g_flagsBorder1V);
    assetStaticBoxSizer->Add(asset_compounding_sizer, g_flagsV);

    //----------------------------------------------
    //a bit more space visual appearance
    //othersPanelSizer->AddSpacer(10);

    wxBoxSizer* itemBoxSizerStockURL = new wxBoxSizer(wxVERTICAL);
    othersPanelSizer->Add(itemBoxSizerStockURL);

    // Database Settings
    wxStaticBox* databaseStaticBox = new wxStaticBox(misc_panel, wxID_STATIC, _t("Database"));
    wxStaticBoxSizer* databaseStaticBoxSizer = new wxStaticBoxSizer(databaseStaticBox, wxVERTICAL);
    othersPanelSizer->Add(databaseStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    wxCheckBox* databaseCheckBox = new wxCheckBox(databaseStaticBox, ID_DIALOG_OPTIONS_CHK_BACKUP
        , _t("Backup database on startup"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    databaseCheckBox->SetValue(GetIniDatabaseCheckboxValue("BACKUPDB", false));
    databaseCheckBox->SetToolTip(_t("When MMEX starts,\n"
        "create the backup database: dbFile_start_YYYY-MM-DD.bak"));
    databaseStaticBoxSizer->Add(databaseCheckBox, g_flagsV);

    wxCheckBox* databaseUpdateCheckBox = new wxCheckBox(databaseStaticBox, ID_DIALOG_OPTIONS_CHK_BACKUP_UPDATE
        , _t("Backup database on exit"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    databaseUpdateCheckBox->SetValue(GetIniDatabaseCheckboxValue("BACKUPDB_UPDATE", true));
    databaseUpdateCheckBox->SetToolTip(_t("When MMEX shuts down and changes were made to the database,\n"
        "create or update the backup database: dbFile_update_YYYY-MM-DD.bak"));
    databaseStaticBoxSizer->Add(databaseUpdateCheckBox, g_flagsV);

    int max = SettingModel::instance().getInt("MAX_BACKUP_FILES", 4);
    m_max_files = new wxSpinCtrl(databaseStaticBox, wxID_ANY
        , wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 999, max);
    m_max_files->SetValue(max);
    mmToolTip(m_max_files, _t("Specify max number of backup files"));

    wxFlexGridSizer* flex_sizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    flex_sizer2->Add(new wxStaticText(databaseStaticBox, wxID_STATIC, _t("Max Files")), g_flagsH);
    flex_sizer2->Add(m_max_files, g_flagsH);
    databaseStaticBoxSizer->Add(flex_sizer2);

    int days = SettingModel::instance().getInt("DELETED_TRANS_RETAIN_DAYS", 30);
    m_deleted_trans_retain_days = new wxSpinCtrl(databaseStaticBox, wxID_ANY
        , wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 999, days);
    m_deleted_trans_retain_days->SetValue(days);
    mmToolTip(m_deleted_trans_retain_days, _t("Specify number of days to retain deleted transactions. Transactions older than this will be automatically purged upon database open."));
    wxFlexGridSizer* flex_sizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    flex_sizer3->Add(new wxStaticText(databaseStaticBox, wxID_STATIC, _t("Days to retain deleted transactions")), g_flagsH);
    flex_sizer3->Add(m_deleted_trans_retain_days, g_flagsBorder1H);
    databaseStaticBoxSizer->Add(flex_sizer3);

    //CSV Import
    const wxString delimiter = InfotableModel::instance().getString("DELIMITER", mmex::DEFDELIMTER);

    wxStaticBox* csvStaticBox = new wxStaticBox(misc_panel, wxID_ANY, _t("CSV"));
    wxStaticBoxSizer* csvStaticBoxSizer = new wxStaticBoxSizer(csvStaticBox, wxVERTICAL);

    othersPanelSizer->Add(csvStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));
    wxFlexGridSizer* csvStaticBoxSizerGrid = new wxFlexGridSizer(0, 2, 0, 10);
    csvStaticBoxSizer->Add(csvStaticBoxSizerGrid, g_flagsV);

    csvStaticBoxSizerGrid->Add(new wxStaticText(csvStaticBox, wxID_STATIC, _t("Delimiter")), g_flagsH);
    wxTextCtrl* textDelimiter4 = new wxTextCtrl(csvStaticBox
        , ID_DIALOG_OPTIONS_TEXTCTRL_DELIMITER4, delimiter);
    mmToolTip(textDelimiter4, _t("Specify the delimiter to use when importing/exporting CSV files"));
    textDelimiter4->SetMaxLength(1);
    csvStaticBoxSizerGrid->Add(textDelimiter4, g_flagsH);

    // Filter Settings
    wxStaticBox* filterStaticBox = new wxStaticBox(misc_panel, wxID_STATIC, _t("Filter"));
    wxStaticBoxSizer* filterStaticBoxSizer = new wxStaticBoxSizer(filterStaticBox, wxVERTICAL);

    othersPanelSizer->Add(filterStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    m_store_account_specific_filter = new wxCheckBox(filterStaticBox, ID_DIALOG_OPTIONS_CHK_FILTER
        , _t("Enable date range filter per account or report"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_store_account_specific_filter->SetValue(PreferencesModel::instance().getUsePerAccountFilter());
    m_store_account_specific_filter->SetToolTip(_t("Store filter values per account or report and not globally"));
    filterStaticBoxSizer->Add(m_store_account_specific_filter, g_flagsV);

    wxCommandEvent evt;
    OtherPreferences::OnBackupChanged(evt);

    SetBoldFontToStaticBoxHeader(stockStaticBox);
    SetBoldFontToStaticBoxHeader(assetStaticBox);
    SetBoldFontToStaticBoxHeader(databaseStaticBox);
    SetBoldFontToStaticBoxHeader(csvStaticBox);
    SetBoldFontToStaticBoxHeader(filterStaticBox);

    Fit();
    misc_panel->SetMinSize(misc_panel->GetBestVirtualSize());
    misc_panel->SetScrollRate(6, 6);
}

void OtherPreferences::OnBackupChanged(wxCommandEvent& WXUNUSED(event))
{
    wxCheckBox* ChkBackup = static_cast<wxCheckBox*>(FindWindow(ID_DIALOG_OPTIONS_CHK_BACKUP));
    wxCheckBox* ChkBackupUpdate = static_cast<wxCheckBox*>(FindWindow(ID_DIALOG_OPTIONS_CHK_BACKUP_UPDATE));
    m_max_files->Enable(ChkBackup->GetValue() || ChkBackupUpdate->GetValue());
}

void OtherPreferences::SaveStocksUrl()
{
    wxComboBox* url = static_cast<wxComboBox*>(FindWindow(ID_DIALOG_OPTIONS_TEXTCTRL_STOCKURL));
    wxString stockURL = url->GetValue().Trim(false).Trim();
    if (!stockURL.IsEmpty())
    {
        InfotableModel::instance().setString("STOCKURL", stockURL);
    }
    else
    {
        InfotableModel::Data_Set items = InfotableModel::instance().find(InfotableModel::INFONAME("STOCKURL"));
        if (!items.empty())
            InfotableModel::instance().remove(items[0].INFOID);
    }
}

bool OtherPreferences::SaveSettings()
{
    SaveStocksUrl();
    PreferencesModel::instance().setSharePrecision(m_share_precision->GetValue());
    PreferencesModel::instance().setAssetCompounding(m_asset_compounding->GetSelection());

    wxCheckBox* itemCheckBox = static_cast<wxCheckBox*>(FindWindow(ID_DIALOG_OPTIONS_CHK_BACKUP));
    SettingModel::instance().setBool("BACKUPDB", itemCheckBox->GetValue());

    wxCheckBox* itemCheckBoxUpdate = static_cast<wxCheckBox*>(FindWindow(ID_DIALOG_OPTIONS_CHK_BACKUP_UPDATE));
    SettingModel::instance().setBool("BACKUPDB_UPDATE", itemCheckBoxUpdate->GetValue());

    SettingModel::instance().setInt("MAX_BACKUP_FILES", m_max_files->GetValue());
    SettingModel::instance().setInt("DELETED_TRANS_RETAIN_DAYS", m_deleted_trans_retain_days->GetValue());
    SettingModel::instance().setBool("REFRESH_STOCK_QUOTES_ON_OPEN", m_refresh_quotes_on_open->IsChecked());

    PreferencesModel::instance().setUsePerAccountFilter(m_store_account_specific_filter->IsChecked());

    wxTextCtrl* st = static_cast<wxTextCtrl*>(FindWindow(ID_DIALOG_OPTIONS_TEXTCTRL_DELIMITER4));
    const wxString& delim = st->GetValue();
    if (!delim.IsEmpty()) InfotableModel::instance().setString("DELIMITER", delim);

    return true;
}
