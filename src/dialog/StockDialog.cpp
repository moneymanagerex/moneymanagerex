/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2016, 2020 Nikolay Akimov
 Copyright (C) 2022  Mark Whalley (mark@ipx.co.uk)
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

#include "base/defs.h"
#include <wx/numdlg.h>
#include <wx/textdlg.h>
#include <wx/valnum.h>

#include "base/constants.h"
#include "base/paths.h"
#include "base/images_list.h"
#include "util/_util.h"
#include "util/_simple.h"
#include "util/mmTextCtrl.h"
#include "util/mmCalcValidator.h"

#include "model/_all.h"

#include "mmframe.h"
#include "AttachmentDialog.h"
#include "AccountDialog.h"
#include "StockDialog.h"
#include "TransactionShareDialog.h"

using namespace rapidjson;

IMPLEMENT_DYNAMIC_CLASS(StockDialog, wxDialog)

wxBEGIN_EVENT_TABLE(StockDialog, wxDialog)
    EVT_CLOSE(StockDialog::OnQuit)
    EVT_BUTTON(wxID_SAVE, StockDialog::OnSave)
    EVT_BUTTON(wxID_CANCEL, StockDialog::OnCancel)
    EVT_BUTTON(wxID_INDEX, StockDialog::OnStockPriceButton)
    EVT_BUTTON(wxID_FILE, StockDialog::OnAttachments)
    EVT_BUTTON(ID_BUTTON_IMPORT, StockDialog::OnHistoryImportButton)
    EVT_BUTTON(ID_BUTTON_DOWNLOAD, StockDialog::OnHistoryDownloadButton)
    EVT_BUTTON(wxID_ADD, StockDialog::OnHistoryAddButton)
    EVT_BUTTON(wxID_DELETE, StockDialog::OnHistoryDeleteButton)
    EVT_CHILD_FOCUS(StockDialog::OnFocusChange)
    EVT_LIST_ITEM_SELECTED(wxID_ANY, StockDialog::OnListItemSelected)
wxEND_EVENT_TABLE()

StockDialog::StockDialog( )
{
}

StockDialog::StockDialog(wxWindow* parent
    , StockModel::Data* stock
    , int64 accountID
    , const wxString& name
    )
    : m_stock(stock)
    , m_edit(stock ? true: false)
    , m_account_id(accountID)
{
    long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY, "", wxDefaultPosition, wxSize(400, 300), style, name);
    mmThemeAutoColour(this);
}

bool StockDialog::Create(wxWindow* parent, wxWindowID id, const wxString& caption
    , const wxPoint& pos, const wxSize& size, long style, const wxString& name)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style, name);

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    SetIcon(mmex::getProgramIcon());

    if (m_edit) DataToControls();
    UpdateControls();

    Centre();
    return true;
}

void StockDialog::DataToControls()
{
    if (!this->m_stock) return;

    m_stock_id = m_stock->STOCKID;

    m_stock_name_ctrl->SetValue(m_stock->STOCKNAME);
    m_stock_symbol_ctrl->SetValue(m_stock->SYMBOL);
    m_notes_ctrl->SetValue(m_stock->NOTES);
    m_purchase_date_ctrl->SetValue(StockModel::PURCHASEDATE(m_stock));

    int precision = m_stock->NUMSHARES == floor(m_stock->NUMSHARES) ? 0 : PreferencesModel::instance().getSharePrecision();
    m_num_shares_ctrl->SetValue(m_stock->NUMSHARES, precision);
    AccountModel::Data* account = AccountModel::instance().cache_id(m_stock->HELDAT);
    CurrencyModel::Data *currency = CurrencyModel::GetBaseCurrency();
    if (account) currency = AccountModel::currency(account);
    int currency_precision = CurrencyModel::precision(currency);
    if (currency_precision < PreferencesModel::instance().getSharePrecision())
        currency_precision = PreferencesModel::instance().getSharePrecision();
    m_purchase_price_ctrl->SetValue(m_stock->PURCHASEPRICE, account, currency_precision);
    m_history_price_ctrl->SetValue(m_stock->CURRENTPRICE, account, currency_precision);
    m_commission_ctrl->SetValue(m_stock->COMMISSION, account, currency_precision);
    m_current_price_ctrl->SetValue(m_stock->CURRENTPRICE, account, currency_precision);

    ShowStockHistory();
}

void StockDialog::UpdateControls()
{
    this->SetTitle(m_edit ? _t("Edit Stock Investment") : _t("New Stock Investment"));
    if (m_account_id > -1) {  // do not use for overview
        AccountModel::Data* account = AccountModel::instance().cache_id(m_account_id);
        if (m_stock) {
            m_value_investment->SetLabelText(AccountModel::toCurrency(StockModel::instance().CurrentValue(m_stock), account));
        }
    }
    else {
        m_value_investment->SetLabelText(wxString::Format(wxT("%.2f"), m_stock->CURRENTPRICE * m_stock->NUMSHARES));
    }

    //Disable history buttons on new stocks

    wxBitmapButton* buttonDownload = static_cast<wxBitmapButton*>(FindWindow(ID_BUTTON_DOWNLOAD));
    buttonDownload->Enable(m_edit);
    wxBitmapButton* buttonImport = static_cast<wxBitmapButton*>(FindWindow(ID_BUTTON_IMPORT));
    buttonImport->Enable(m_edit);
    wxBitmapButton* buttonDel = static_cast<wxBitmapButton*>(FindWindow(wxID_DELETE));
    buttonDel->Enable(m_edit);
    wxBitmapButton* buttonAdd = static_cast<wxBitmapButton*>(FindWindow(wxID_ADD));
    buttonAdd->Enable(m_edit);

    bool initial_shares = !TransactionLinkModel::HasShares(m_stock_id);
    m_num_shares_ctrl->Enable(!m_edit || initial_shares);
    m_purchase_date_ctrl->Enable(!m_edit || initial_shares);
    m_purchase_price_ctrl->Enable(!m_edit || initial_shares);
    m_commission_ctrl->Enable(!m_edit || initial_shares);

    // Disable in stock overview:
    m_stock_name_ctrl->Enable(m_account_id > -1);
    m_stock_symbol_ctrl->Enable(m_account_id > -1);
    m_current_price_ctrl->Enable(m_account_id > -1);
    m_notes_ctrl->Enable(m_account_id > -1);

    // Hide in stock overview:
    m_date_label->Show(m_account_id > -1);
    m_purchase_date_ctrl->Show(m_account_id > -1);
    m_itemButtonOK->Show(m_account_id > -1);


    m_stock_symbol_ctrl->SetValue(m_stock_symbol_ctrl->GetValue().Upper());
}

void StockDialog::CreateControls()
{
    bool initial_stock_transaction = true;
    if (m_stock)
    {
        if (!TransactionLinkModel::TranslinkList<StockModel>(m_stock->STOCKID).empty())
        {
            initial_stock_transaction = false;
        }
    }

    wxBoxSizer* mainBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    this->SetSizer(mainBoxSizer);

    wxBoxSizer* leftBoxSizer = new wxBoxSizer(wxVERTICAL);
    mainBoxSizer->Add(leftBoxSizer, g_flagsH);

    wxStaticBox* itemStaticBoxSizer4Static = new wxStaticBox(this, wxID_ANY, _t("Stock Investment Details"));
    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer(itemStaticBoxSizer4Static, wxVERTICAL);
    leftBoxSizer->Add(itemStaticBoxSizer4, g_flagsExpand);

    wxPanel* itemPanel5 = new wxPanel(itemStaticBoxSizer4Static, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    itemStaticBoxSizer4->Add(itemPanel5, g_flagsV);

    wxFlexGridSizer* itemFlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
    itemPanel5->SetSizer(itemFlexGridSizer6);

    // Name
    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _t("Company Name")), g_flagsH);
    m_stock_name_ctrl = new wxTextCtrl(itemPanel5, ID_TEXTCTRL_STOCKNAME, "");
    itemFlexGridSizer6->Add(m_stock_name_ctrl, g_flagsExpand);
    mmToolTip(m_stock_name_ctrl, _t("Enter the stock company name"));

    //Symbol
    wxStaticText* symbol = new wxStaticText(itemPanel5, wxID_STATIC, _t("Stock Symbol"));
    itemFlexGridSizer6->Add(symbol, g_flagsH);
    symbol->SetFont(this->GetFont().Bold());

    m_stock_symbol_ctrl = new wxTextCtrl(itemPanel5, ID_TEXTCTRL_STOCK_SYMBOL
        , "", wxDefaultPosition, wxSize(150, -1), 0);
    itemFlexGridSizer6->Add(m_stock_symbol_ctrl, g_flagsExpand);
    mmToolTip(m_stock_symbol_ctrl, _t("Enter the stock symbol. (Optional) Include exchange. eg: IBM.BE"));

    //Date
    m_date_label = new wxStaticText(itemPanel5, wxID_STATIC, _t("*Date"));
    itemFlexGridSizer6->Add(m_date_label, g_flagsH);
    m_date_label->SetFont(this->GetFont().Bold());
    m_purchase_date_ctrl = new mmDatePickerCtrl(itemPanel5, ID_DPC_STOCK_PDATE);
    itemFlexGridSizer6->Add(m_purchase_date_ctrl->mmGetLayout(false));
    mmToolTip(m_purchase_date_ctrl, _t("Specify the initial date of the stock investment\nUsed when creating the initial Share transaction."));
    m_purchase_date_ctrl->Enable(initial_stock_transaction);

    //Number of Shares
    wxStaticText* number = new wxStaticText(itemPanel5, wxID_STATIC, _t("*Share Number"));
    itemFlexGridSizer6->Add(number, g_flagsH);
    number->SetFont(this->GetFont().Bold());
    m_num_shares_ctrl = new mmTextCtrl(itemPanel5, ID_TEXTCTRL_NUMBER_SHARES, ""
        , wxDefaultPosition, wxSize(150, -1), wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    itemFlexGridSizer6->Add(m_num_shares_ctrl, g_flagsExpand);
    m_num_shares_ctrl->SetAltPrecision(PreferencesModel::instance().getSharePrecision());
    mmToolTip(m_num_shares_ctrl, _t("Enter number of shares.\nUsed when creating the initial Share transaction."));
    m_num_shares_ctrl->Enable(initial_stock_transaction);

    //Purchase Price
    wxStaticText* pprice = new wxStaticText(itemPanel5, wxID_STATIC, _t("*Share Price"));
    itemFlexGridSizer6->Add(pprice, g_flagsH);
    pprice->SetFont(this->GetFont().Bold());
    m_purchase_price_ctrl = new mmTextCtrl(itemPanel5, ID_TEXTCTRL_STOCK_PP, ""
        , wxDefaultPosition, wxSize(150, -1), wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    itemFlexGridSizer6->Add(m_purchase_price_ctrl, g_flagsExpand);
    m_purchase_price_ctrl->SetAltPrecision(PreferencesModel::instance().getSharePrecision());
    mmToolTip(m_purchase_price_ctrl, _t("Enter the initial price per share.\nUsed when creating the initial Share transaction."));
    m_purchase_price_ctrl->Enable(initial_stock_transaction);

    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _t("Deductible Comm.")), g_flagsH);
    m_commission_ctrl = new mmTextCtrl(itemPanel5, ID_TEXTCTRL_STOCK_COMMISSION, "0"
        , wxDefaultPosition, wxSize(150, -1), wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    itemFlexGridSizer6->Add(m_commission_ctrl, g_flagsExpand);
    m_commission_ctrl->SetAltPrecision(PreferencesModel::instance().getSharePrecision());
    mmToolTip(m_commission_ctrl, _t("Enter any commission paid.\nUsed when creating the initial Share transaction."));
    m_commission_ctrl->Enable(initial_stock_transaction);

    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _t("Curr. Share Price")), g_flagsH);
    m_current_price_ctrl = new mmTextCtrl(itemPanel5, ID_TEXTCTRL_STOCK_CURR_PRICE, ""
        , wxDefaultPosition, wxSize(150, -1), wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    m_current_price_ctrl->SetAltPrecision(PreferencesModel::instance().getSharePrecision());
    mmToolTip(m_current_price_ctrl, _t("Enter current stock/share price."));
    itemFlexGridSizer6->Add(m_current_price_ctrl, g_flagsExpand);

    //
    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _t("Current Value")), g_flagsH);
    m_value_investment = new wxStaticText(itemPanel5, ID_STATIC_STOCK_VALUE, "--");
    m_value_investment->SetFont(this->GetFont().Bold());
    itemFlexGridSizer6->Add(m_value_investment, wxSizerFlags().Align(wxALIGN_RIGHT | wxEXPAND).Border(wxALL, 10).Proportion(1));

    //
    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _t("Notes")), wxSizerFlags().Left().Bottom().Border(wxLEFT, 5).Border(wxBOTTOM,0));
    wxBoxSizer* iconsSizer = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer6->Add(iconsSizer, wxSizerFlags(g_flagsH).Align(wxALIGN_RIGHT));
    m_bAttachments = new wxBitmapButton(itemPanel5, wxID_FILE, mmBitmapBundle(png::CLIP, mmBitmapButtonSize));
    mmToolTip(m_bAttachments, _t("Manage stock attachments"));
    wxBitmapButton* itemButton31 = new wxBitmapButton(itemPanel5, wxID_INDEX, mmBitmapBundle(png::WEB, mmBitmapButtonSize));
    mmToolTip(itemButton31, _t("Display the web page for the specified Stock symbol"));
    iconsSizer->Add(m_bAttachments, g_flagsH);
    iconsSizer->Add(itemButton31, g_flagsH);

    m_notes_ctrl = new wxTextCtrl(itemStaticBoxSizer4Static, wxID_STATIC, "", wxDefaultPosition, wxSize(200, 90), wxTE_MULTILINE);
    itemStaticBoxSizer4->Add(m_notes_ctrl, g_flagsExpand);
    itemStaticBoxSizer4->AddSpacer(1);
    mmToolTip(m_notes_ctrl, _t("Enter notes associated with this investment"));

    leftBoxSizer->AddSpacer(20);

    //History Panel
    wxBoxSizer* rightBoxSizer = new wxBoxSizer(wxVERTICAL);
    mainBoxSizer->Add(rightBoxSizer, g_flagsExpand);

    wxStaticBox* historyStaticBox = new wxStaticBox(this, wxID_ANY, _t("Stock History Options"));
    wxStaticBoxSizer* historyStaticBoxSizer = new wxStaticBoxSizer(historyStaticBox, wxVERTICAL);
    rightBoxSizer->Add(historyStaticBoxSizer, g_flagsExpand);

    m_price_listbox = new wxListCtrl(historyStaticBox, wxID_ANY, wxDefaultPosition, wxSize(250, 150)
        , wxLC_REPORT);
    historyStaticBoxSizer->Add(m_price_listbox, g_flagsExpand);
    mmToolTip(m_price_listbox, _t("Stock Price History"));

    // Add first column
    wxListItem col0;
    col0.SetId(0);
    col0.SetText( _t("Date") );
    col0.SetWidth(90);
    m_price_listbox->InsertColumn(0, col0);

    // Add second column
    wxListItem col1;
    col1.SetId(1);
    col1.SetText( _t("Price") );
    col1.SetWidth(80);
    m_price_listbox->InsertColumn(1, col1);

    //History Buttons
    wxPanel* buttons_panel = new wxPanel(historyStaticBox, wxID_ANY);
    historyStaticBoxSizer->Add(buttons_panel, wxSizerFlags(g_flagsV).Centre());
    wxBoxSizer* buttons_sizer = new wxBoxSizer(wxVERTICAL);
    buttons_panel->SetSizer(buttons_sizer);

    //
    wxFlexGridSizer* date_price = new wxFlexGridSizer(0, 2, 0, 0);
    date_price->Add(new wxStaticText(buttons_panel, wxID_STATIC, _t("Price Date")), g_flagsH);
    m_history_date_ctrl = new mmDatePickerCtrl(buttons_panel, ID_DPC_CP_PDATE
        , wxDefaultDateTime, wxDefaultPosition, wxSize(150, -1), wxDP_DROPDOWN | wxDP_SHOWCENTURY);
    date_price->Add(m_history_date_ctrl, g_flagsH);
    mmToolTip(m_history_date_ctrl, _t("Specify the stock/share price date."));

    //
    date_price->Add(new wxStaticText(buttons_panel, wxID_STATIC, _t("Price")), g_flagsH);
    m_history_price_ctrl = new mmTextCtrl(buttons_panel, ID_TEXTCTRL_STOCK_CP, ""
        , wxDefaultPosition, wxSize(150, -1), wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    m_history_price_ctrl->SetAltPrecision(PreferencesModel::instance().getSharePrecision());
    date_price->Add(m_history_price_ctrl, g_flagsH);
    buttons_sizer->Add(date_price);

    m_history_date_ctrl->Enable(false);
    m_history_price_ctrl->Enable(false);

    wxStdDialogButtonSizer*  std_buttons_sizer = new wxStdDialogButtonSizer;
    wxBitmapButton* buttonDownload = new wxBitmapButton(buttons_panel, ID_BUTTON_DOWNLOAD, mmBitmapBundle(png::UPDATE, mmBitmapButtonSize));
    mmToolTip(buttonDownload, _t("Download Stock Price history"));
    wxBitmapButton* buttonImport = new wxBitmapButton(buttons_panel, ID_BUTTON_IMPORT, mmBitmapBundle(png::IMPORT, mmBitmapButtonSize));
    mmToolTip(buttonImport, _t("Import Stock Price history (CSV Format)"));
    wxButton* buttonDel = new wxButton(buttons_panel, wxID_DELETE, _t("&Delete "));
    mmToolTip(buttonDel, _t("Delete selected Stock Price"));
    wxButton* buttonAdd = new wxButton(buttons_panel, wxID_ADD, _t("&Add "));
    mmToolTip(buttonAdd, _t("Add Stock Price to history"));

    std_buttons_sizer->Add(buttonDownload, g_flagsH);
    std_buttons_sizer->Add(buttonImport, g_flagsH);
    std_buttons_sizer->Add(buttonDel, g_flagsH);
    std_buttons_sizer->Add(buttonAdd, g_flagsH);
    buttons_sizer->Add(std_buttons_sizer);

    //OK & Cancel buttons
    wxStdDialogButtonSizer*  buttonsOK_CANCEL_sizer = new wxStdDialogButtonSizer;
    leftBoxSizer->Add(buttonsOK_CANCEL_sizer, wxSizerFlags(g_flagsV).Centre());

    m_itemButtonOK = new wxButton(this, wxID_SAVE, _t("&Save "));
    wxButton* itemButton30 = new wxButton(this, wxID_CANCEL, wxGetTranslation(g_CloseLabel));

    if (m_edit) {
        itemButton30->SetFocus();
    }
    buttonsOK_CANCEL_sizer->Add(m_itemButtonOK, g_flagsH);
    buttonsOK_CANCEL_sizer->Add(itemButton30, g_flagsH);
}

void StockDialog::OnQuit(wxCloseEvent& /*event*/)
{
    const wxString& RefType = StockModel::refTypeName;
    if (!m_edit)
        mmAttachmentManage::DeleteAllAttachments(RefType, 0);
    EndModal(wxID_CANCEL);
}

void StockDialog::OnCancel(wxCommandEvent& /*event*/)
{
    const wxString& RefType = StockModel::refTypeName;
    if (m_stock_id <= 0)
        mmAttachmentManage::DeleteAllAttachments(RefType, 0);
    EndModal(wxID_CANCEL);
}

void StockDialog::OnAttachments(wxCommandEvent& /*event*/)
{
    const wxString RefType = StockModel::refTypeName;
    int64 RefId = m_stock_id;

    if (RefId < 0)
        RefId = 0;

    AttachmentDialog dlg(this, RefType, RefId);
    dlg.ShowModal();
}

void StockDialog::OnStockPriceButton(wxCommandEvent& /*event*/)
{
    const wxString stockSymbol = m_stock_symbol_ctrl->GetValue().Trim();

    if (!stockSymbol.IsEmpty())
    {
        const wxString& stockURL = InfoModel::instance().getString("STOCKURL", mmex::weblink::DefStockUrl);
        const wxString& httpString = wxString::Format(stockURL, stockSymbol);
        wxLaunchDefaultBrowser(httpString);
    }
}

void StockDialog::OnSave(wxCommandEvent & /*event*/)
{
    const auto itemCount = m_price_listbox->GetItemCount();

    for (auto i = 0; i<itemCount; i++)
    {
        if (m_price_listbox->GetItemState(0, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED)
        {
            m_price_listbox->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
            break;
        }
    }

    AccountModel::Data* account = AccountModel::instance().cache_id(m_account_id);
    if (!account)
    {
        mmErrorDialogs::MessageInvalid(this, _t("Held At"));
        return;
    }

    // TODO unique
    const wxString stockSymbol = m_stock_symbol_ctrl->GetValue();
    if (stockSymbol.empty())
    {
        mmErrorDialogs::MessageInvalid(this, _t("Symbol"));
        return;
    }

    const wxString pdate = m_purchase_date_ctrl->GetValue().FormatISODate();
    if (pdate < account->INITIALDATE)
        return mmErrorDialogs::ToolTip4Object(m_purchase_date_ctrl, _t("The opening date for the account is later than the date of this transaction"), _t("Invalid Date"));

    const wxString stockName = m_stock_name_ctrl->GetValue();
    const wxString notes = m_notes_ctrl->GetValue();

    double numShares = 0;
    if (!m_num_shares_ctrl->checkValue(numShares))
        return;

    double initPrice;
    if (!m_purchase_price_ctrl->checkValue(initPrice))
        return;

    double currentPrice;
    if (!m_current_price_ctrl->GetDouble(currentPrice))
    {
        // assume current price = purchase price on stock creation only
        if (!m_stock)
        {
            currentPrice = initPrice;
            m_history_date_ctrl->SetValue(m_purchase_date_ctrl->GetValue());
            m_current_price_ctrl->SetValue(m_purchase_price_ctrl->GetValue());
        }
    }

    double commission = 0;
    m_commission_ctrl->GetDouble(commission);

    double initValue = 0;
    // maintain initial value for records with share accounts
    m_stock && initPrice == 0
        ? initValue = m_stock->VALUE
        : initValue = initPrice * numShares;

    if (!m_stock) m_stock = StockModel::instance().create();

    m_stock->HELDAT = m_account_id;
    m_stock->PURCHASEDATE = pdate;
    m_stock->STOCKNAME = stockName;
    m_stock->SYMBOL = stockSymbol;
    m_stock->NUMSHARES = numShares;
    m_stock->PURCHASEPRICE = initPrice;
    m_stock->NOTES = notes;
    m_stock->CURRENTPRICE = currentPrice;
    m_stock->VALUE = initValue;
    m_stock->COMMISSION = commission;

    m_stock_id = StockModel::instance().save(m_stock);

    if (!m_edit)
    {
        const wxString RefType = StockModel::refTypeName;
        mmAttachmentManage::RelocateAllAttachments(RefType, 0, RefType, m_stock->STOCKID);
        TransactionShareDialog share_dialog(this, m_stock);
        share_dialog.ShowModal();
    }

    StockHistoryModel::instance().addUpdate(m_stock->SYMBOL, wxDate::Today(), m_stock->CURRENTPRICE, StockHistoryModel::MANUAL);
    ShowStockHistory();

    m_edit = true;
    UpdateControls();
}

void StockDialog::CreateShareAccount(AccountModel::Data* stock_account, const wxString& name, const wxString& openingDate)
{
    if (name.empty()) return;
    AccountModel::Data* share_account = AccountModel::instance().create();
    share_account->ACCOUNTNAME = name;
    share_account->ACCOUNTTYPE = NavigatorTypes::instance().getShareAccountStr();

    share_account->FAVORITEACCT = "FALSE";
    share_account->STATUS = AccountModel::STATUS_NAME_OPEN;
    share_account->INITIALBAL = 0;
    share_account->INITIALDATE = openingDate;
    share_account->CURRENCYID = stock_account->CURRENCYID;
    AccountModel::instance().save(share_account);

    TransactionShareDialog share_dialog(this, m_stock);
    share_dialog.ShowModal();
}

void StockDialog::OnListItemSelected(wxListEvent& event)
{
    long selectedIndex = event.GetIndex();
    int64 histId = m_price_listbox->GetItemData(selectedIndex);
    AccountModel::Data* account = AccountModel::instance().cache_id(m_stock->HELDAT);
    StockHistoryModel::Data *histData = StockHistoryModel::instance().cache_id(histId);

    if (histData->HISTID > 0)
    {
        m_history_date_ctrl->SetValue(StockHistoryModel::DATE(*histData));
        m_history_price_ctrl->SetValue(AccountModel::toString(histData->VALUE, account, PreferencesModel::instance().getSharePrecision()));
    }
}

void StockDialog::OnHistoryImportButton(wxCommandEvent& /*event*/)
{
    if (m_stock->SYMBOL.IsEmpty())
        return;

    const wxString fileName = wxFileSelector(_t("Choose CSV data file to import")
        , wxEmptyString, wxEmptyString, wxEmptyString, "*.csv", wxFD_FILE_MUST_EXIST);
    AccountModel::Data *account = AccountModel::instance().cache_id(m_stock->HELDAT);
    CurrencyModel::Data *currency = AccountModel::currency(account);

    if (!fileName.IsEmpty())
    {
        wxFileName csv_file(fileName);
        if (fileName.IsEmpty() || !csv_file.FileExists())
            return;
        wxTextFile tFile(fileName);
        if (!tFile.Open())
            return;
        wxProgressDialog* progressDlg = new wxProgressDialog(_t("Stock History CSV Import")
            , _t("Quotes imported from CSV: "), tFile.GetLineCount()
            , nullptr, wxPD_AUTO_HIDE | wxPD_APP_MODAL | wxPD_SMOOTH | wxPD_CAN_ABORT);

        bool canceledbyuser = false;
        long countNumTotal = 0;
        long countImported = 0;
        double price;
        wxString dateStr, priceStr;
        StockHistoryModel::Data *data;
        StockHistoryModel::Cache stockData;

        wxString line;
        std::vector<wxString> rows;
        for (line = tFile.GetFirstLine(); !tFile.Eof(); line = tFile.GetNextLine())
        {
            wxString progressMsg;
            progressMsg << _t("Quotes imported from CSV: ") << countImported;
            if (!progressDlg->Update(countImported, progressMsg))
            {
                canceledbyuser = true;
                break; // abort processing
            }

            if (!line.IsEmpty())
                ++countNumTotal;
            else
                continue;

            dateStr.clear();
            priceStr.clear();

            const wxString& delimiter = InfoModel::instance().getString("DELIMITER", mmex::DEFDELIMTER);
            csv2tab_separated_values(line, delimiter);
            wxStringTokenizer tkz(line, "\t", wxTOKEN_RET_EMPTY_ALL);
            if (static_cast<int>(tkz.CountTokens()) < 2)
                continue;

            std::vector<wxString> tokens;
            while (tkz.HasMoreTokens())
            {
                wxString token = tkz.GetNextToken();
                tokens.push_back(token);
            }

            // date
            wxDateTime dt;
            dateStr = tokens[0];
            mmParseDisplayStringToDate(dt, dateStr, PreferencesModel::instance().getDateFormat());
            dateStr = dt.FormatISODate();
            // price
            priceStr = tokens[1];
            priceStr.Replace(" ", wxEmptyString);
            if (!CurrencyModel::fromString(priceStr, price, currency) || price <= 0.0)
                continue;

            data = StockHistoryModel::instance().create();
            data->SYMBOL = m_stock->SYMBOL;
            data->DATE = dateStr;
            data->VALUE = price;
            data->UPDTYPE = 2;
            stockData.push_back(data);

            if (rows.size()<10)
            {
                dateStr <<  wxT ("  ") << priceStr;
                rows.push_back(dateStr);
            }
            countImported++;
        }

        progressDlg->Destroy();

        wxString msg = wxString::Format(_t("Total lines: %ld"), countNumTotal);
        msg << "\n";
        msg << wxString::Format(_t("Total imported: %ld"), countImported);
        msg << "\n";
        msg << _t("Date") << "              " << _t("Price");
        msg << "\n";
        for (std::vector<wxString>::const_iterator d = rows.begin(); d != rows.end(); ++d)
            msg << *d << "\n";
        wxString confirmMsg = msg + _t("Please confirm saving");
        if (!canceledbyuser && wxMessageBox(confirmMsg
            , _t("Importing CSV"), wxOK | wxCANCEL | wxICON_INFORMATION) == wxCANCEL)
        {
            canceledbyuser = true;
        }

        // Since all database transactions are only in memory,
        if (!canceledbyuser)
        {
            // we need to save them to the database.
            for (auto &d : stockData)
                StockHistoryModel::instance().save(d);
            // show the data
            ShowStockHistory();
        }
        else
        {
            //TODO: and discard the database changes.
        }
    }
}

void StockDialog::OnHistoryDownloadButton(wxCommandEvent& /*event*/)
{
    if (m_stock->SYMBOL.IsEmpty())
        return;

    /*"ValidRanges":["1d","5d","1mo","3mo","6mo","1y","2y","5y","10y","ytd","max"]*/
    enum { DAY5, MON, MON3, MON6, YEAR, YEAR2, YEAR5, YEAR10, YTD, MAX };
    const wxString ranges[] = { "5d","1mo","3mo","6mo","1y","2y","5y","10y","ytd","max" };
    const std::vector<std::pair<int, wxString> > RANGE_PAIRS =
    {
        { DAY5, _t("5 Days") }
        ,{ MON, _t("1 Month") }
        ,{ MON3, _t("3 Months") }
        ,{ MON6, _t("6 Months") }
        ,{ YEAR, _t("1 Year") }
        ,{ YEAR2, _t("2 Years") }
        ,{ YEAR5, _t("5 Years") }
        ,{ YEAR10, _t("10 Years") }
        ,{ YTD, _t("Current Year to Date") }
        ,{ MAX, _t("Max") }
    };

    wxArrayString items;
    for (const auto& entry : RANGE_PAIRS) { items.Add(entry.second); }

    int range_menu_item_no = wxGetSingleChoiceIndex(_t("Specify type frequency of stock history")
        , _t("Stock History Update"), items);

    if (range_menu_item_no < 0) return;
    const wxString range = ranges[range_menu_item_no];

    wxString interval = "1d";
    if (range != "5d")
    {
        /* Valid intervals : [1m, 2m, 5m, 15m, 30m, 60m, 90m, 1h, 1d, 5d, 1wk, 1mo, 3mo] */
        enum class i { DAY, DAY5, WEEK, MON, MON3 };
        const wxString intervals[] = { "1d","5d","1wk","1mo","3mo" };
        const std::vector<std::pair<int, wxString> > INTERVAL_PAIRS =
        {
              { static_cast<int>(i::DAY), _t("1 Day") }
            , { static_cast<int>(i::DAY5), _t("5 Days") }
            , { static_cast<int>(i::WEEK), _t("1 Week") }
            , { static_cast<int>(i::MON), _t("1 Month") }
            , { static_cast<int>(i::MON3), _t("3 Months") }
        };

        items.clear();
        for (const auto& entry : INTERVAL_PAIRS) { items.Add(entry.second); }

        int interval_menu_item_no = wxGetSingleChoiceIndex(_t("Specify interval of stock history")
            , _t("Stock History Update"), items);

        if (interval_menu_item_no < 0) return;
        interval = intervals[interval_menu_item_no];
    }
    const wxString URL = wxString::Format("https://query1.finance.yahoo.com/v8/finance/chart/%s?range=%s&interval=%s&fields=currency"
        , m_stock->SYMBOL, range, interval);

    wxString json_data;
    auto err_code = http_get_data(URL, json_data);
    wxString sOutput = json_data;

    if (err_code != CURLE_OK)
    {
        if (sOutput.empty()) sOutput = _t("Stock history not found.");
        return mmErrorDialogs::MessageError(this, sOutput, _t("Stock History Error"));
    }

    sOutput = _t("Stock history not found.");

    while (true)
    {
        Document json_doc;
        if (json_doc.Parse(json_data.utf8_str()).HasParseError()) {
            break;
        }
        else if (!json_doc.HasMember("chart") || !json_doc["chart"].IsObject()) {
            break;
        }

        Value chart = json_doc["chart"].GetObject();
        wxASSERT(chart.HasMember("error"));
        if (chart.HasMember("error"))
        {
            if (chart["error"].IsObject())
            {

                /* {
                    "chart":{
                        "result":null,
                        "error":{
                            "code":"Not Found",
                            "description":"No data found, symbol may be delisted"
                        }
                    }
                } */

                Value e = chart["error"].GetObject();
                if (!e.HasMember("code") || !e.HasMember("description") || !e["code"].IsString() || !e["description"].IsString()) {
                    break;
                }

                const wxString code = wxString::FromUTF8(e["code"].GetString());
                const wxString description = wxString::FromUTF8(e["description"].GetString());
                sOutput = wxString::Format("%s - %s", code, description);
                break;
            }
        }

        if (!chart.HasMember("result") || !chart["result"].IsArray())
            break;
        Value result = chart["result"].GetArray();
        if (!result.IsArray() || !result.Begin()->IsObject())
            break;
        Value data = result.Begin()->GetObject();

        if (!data.HasMember("meta") || !data["meta"].IsObject())
            break;
        Value meta = data["meta"].GetObject();

        float k = 1.0L;
        if (meta.HasMember("currency") && meta["currency"].IsString()) {

            const auto currency = wxString::FromUTF8(meta["currency"].GetString());
            k = (currency == "GBp" ? 100 : 1);
        }

        if (!data.HasMember("timestamp") || !data["timestamp"].IsArray())
            break;
        Value timestamp = data["timestamp"].GetArray();

        if (!data.HasMember("indicators") || !data.IsObject())
            break;
        Value indicators = data["indicators"].GetObject();

        if (!indicators.HasMember("adjclose") || !indicators["adjclose"].IsArray())
            break;
        Value quote_array = indicators["adjclose"].GetArray();
        Value quotes = quote_array.Begin()->GetObject();
        if (!quotes.HasMember("adjclose") || !quotes["adjclose"].IsArray())
            break;
        Value quotes_closed = quotes["adjclose"].GetArray();

        if (timestamp.Size() != quotes_closed.Size())
            break;

        std::map<time_t, float> history;
        for (rapidjson::SizeType i = 0; i < timestamp.Size(); i++)
        {
            if (!timestamp[i].IsInt()) continue;
            time_t time = timestamp[i].GetInt();
            if (!quotes_closed[i].IsFloat()) continue;
            float rate = quotes_closed[i].GetFloat() / k;
            history[time] = rate;
        }

        const wxString today = wxDate::Today().FormatISODate();
        StockHistoryModel::instance().Savepoint();
        for (const auto& entry : history)
        {
            float dPrice = entry.second;
            const wxString date_str = wxDateTime(static_cast<time_t>(entry.first)).FormatISODate();
            if (date_str == today) {
                continue;
            }

            if (StockHistoryModel::instance()
                .find(
                    StockHistoryModel::SYMBOL(m_stock->SYMBOL)
                    , StockHistoryModel::StockHistoryTable::DATE(date_str)
                ).empty()
                && dPrice > 0
                )
            {
                StockHistoryModel::Data* ndata = StockHistoryModel::instance().create();
                ndata->SYMBOL = m_stock->SYMBOL;
                ndata->DATE = date_str;
                ndata->VALUE = dPrice;
                ndata->UPDTYPE = StockHistoryModel::ONLINE;
                StockHistoryModel::instance().save(ndata);
            }
        }
        StockHistoryModel::instance().ReleaseSavepoint();
        return ShowStockHistory();
    }
    mmErrorDialogs::MessageError(this, sOutput, _t("Stock History Error"));
}

void StockDialog::OnHistoryAddButton(wxCommandEvent& /*event*/)
{
    if (m_stock->SYMBOL.IsEmpty())
        return;

    if (!m_history_price_ctrl->IsEnabled())
    {
        m_history_price_ctrl->Enable();
        m_history_date_ctrl->Enable();
        return;
    }

    wxString listStr;
    wxDateTime dt;
    double dPrice = 0.0;
    AccountModel::Data* account = AccountModel::instance().cache_id(m_stock->HELDAT);
    CurrencyModel::Data* currency = AccountModel::currency(account);
    wxString currentPriceStr = m_history_price_ctrl->GetValue().Trim();
    if (!CurrencyModel::fromString(currentPriceStr, dPrice, currency) || (dPrice < 0.0))
        return;

    int64 histID = StockHistoryModel::instance().addUpdate(m_stock->SYMBOL, m_history_date_ctrl->GetValue(), dPrice, StockHistoryModel::MANUAL);
    long i;
    for (i = 0; i < m_price_listbox->GetItemCount(); i++)
    {
        listStr = m_price_listbox->GetItemText(i, 0);
        mmParseDisplayStringToDate(dt, listStr, PreferencesModel::instance().getDateFormat());
        if (dt.IsSameDate(m_history_date_ctrl->GetValue()))
            break;
    }
    if (i == m_price_listbox->GetItemCount())
    {
        //add
        for (i = 0; i < m_price_listbox->GetItemCount(); i++)
        {
            listStr = m_price_listbox->GetItemText(i, 0);
            mmParseDisplayStringToDate(dt, listStr, PreferencesModel::instance().getDateFormat());
            if (dt.GetDateOnly() < m_history_date_ctrl->GetValue().GetDateOnly())
                break;
        }
        wxListItem item;
        item.SetId(i);
        item.SetData(reinterpret_cast<void*>(histID.GetValue()));
        m_price_listbox->InsertItem(item);
    }
    if (i != m_price_listbox->GetItemCount())
    {
        listStr = AccountModel::toString(dPrice, account, PreferencesModel::instance().getSharePrecision());
        m_price_listbox->SetItem(i, 0, mmGetDateTimeForDisplay(m_history_date_ctrl->GetValue().FormatISODate()));
        m_price_listbox->SetItem(i, 1, listStr);
    }
    if (i == m_price_listbox->GetNextItem(-1)) // changed the current price/value
    {
        //refresh m_stock to get updated attributes
        m_stock = StockModel::instance().cache_id(m_stock->STOCKID);
        m_current_price_ctrl->SetValue(AccountModel::toString(m_stock->CURRENTPRICE, account, PreferencesModel::instance().getSharePrecision()));
        m_value_investment->SetLabelText(AccountModel::toCurrency(StockModel::instance().CurrentValue(m_stock), account));
    }
}

void StockDialog::OnHistoryDeleteButton(wxCommandEvent& /*event*/)
{
    if (m_price_listbox->GetSelectedItemCount() <= 0)
        return;

    long item = -1;
    StockHistoryModel::instance().Savepoint();
    for (;;)
    {
        item = m_price_listbox->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

        if (item == -1)
            break;
        StockHistoryModel::instance().remove(static_cast<int64>(m_price_listbox->GetItemData(item)));
    }
    StockHistoryModel::instance().ReleaseSavepoint();
    ShowStockHistory();
}

void StockDialog::ShowStockHistory()
{
    m_price_listbox->DeleteAllItems();
    if (m_stock->SYMBOL.IsEmpty())
        return;

    AccountModel::Data* account = AccountModel::instance().cache_id(m_stock->HELDAT);
    StockHistoryModel::Data_Set histData = StockHistoryModel::instance().find(StockHistoryModel::SYMBOL(m_stock->SYMBOL));
    std::stable_sort(histData.begin(), histData.end(), StockHistoryTable::SorterByDATE());
    std::reverse(histData.begin(), histData.end());
    if (histData.size()>300)
        histData.resize(300);
    size_t rows = histData.size() - 1;
    if (!histData.empty())
    {
        for (size_t idx = 0; idx < histData.size(); idx++ )
        {
            wxListItem item;
            item.SetId(static_cast<long>(idx));
            item.SetData(reinterpret_cast<void*>(histData.at(idx).HISTID.GetValue()));
            m_price_listbox->InsertItem(item);
            const wxDate dtdt = StockHistoryModel::DATE(histData.at(idx));
            const wxString dispAmount = AccountModel::toString(histData.at(idx).VALUE, account, PreferencesModel::instance().getSharePrecision());
            m_price_listbox->SetItem(static_cast<long>(idx), 0, mmGetDateTimeForDisplay(histData.at(idx).DATE));
            m_price_listbox->SetItem(static_cast<long>(idx), 1, dispAmount);
            if (idx == 0)
            {
                m_history_date_ctrl->SetValue(dtdt);
                m_history_price_ctrl->SetValue(dispAmount);
                m_current_price_ctrl->SetValue(dispAmount);
                // if the latest share price is not the current stock price, update it.
                if (m_stock->CURRENTPRICE != histData.at(idx).VALUE)
                {
                    StockModel::UpdateCurrentPrice(m_stock->SYMBOL, histData.at(idx).VALUE);
                    m_stock = StockModel::instance().cache_id(m_stock->STOCKID);
                    m_value_investment->SetLabelText(AccountModel::toCurrency(StockModel::instance().CurrentValue(m_stock), AccountModel::instance().cache_id(m_stock->HELDAT)));
                }
            }
        }
        m_price_listbox->RefreshItems(0, rows);
    }
}

void StockDialog::OnFocusChange(wxChildFocusEvent& event)
{
    UpdateControls();
    event.Skip();
}
