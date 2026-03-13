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
#include "TrxShareDialog.h"

using namespace rapidjson;

IMPLEMENT_DYNAMIC_CLASS(StockDialog, wxDialog)

wxBEGIN_EVENT_TABLE(StockDialog, wxDialog)
    EVT_CLOSE(                       StockDialog::OnQuit)
    EVT_BUTTON(wxID_SAVE,            StockDialog::OnSave)
    EVT_BUTTON(wxID_CANCEL,          StockDialog::OnCancel)
    EVT_BUTTON(wxID_INDEX,           StockDialog::OnStockPriceButton)
    EVT_BUTTON(wxID_FILE,            StockDialog::OnAttachments)
    EVT_BUTTON(ID_BUTTON_IMPORT,     StockDialog::OnHistoryImportButton)
    EVT_BUTTON(ID_BUTTON_DOWNLOAD,   StockDialog::OnHistoryDownloadButton)
    EVT_BUTTON(wxID_ADD,             StockDialog::OnHistoryAddButton)
    EVT_BUTTON(wxID_DELETE,          StockDialog::OnHistoryDeleteButton)
    EVT_CHILD_FOCUS(                 StockDialog::OnFocusChange)
    EVT_LIST_ITEM_SELECTED(wxID_ANY, StockDialog::OnListItemSelected)
wxEND_EVENT_TABLE()

StockDialog::StockDialog( )
{
}

StockDialog::StockDialog(
    wxWindow* parent,
    StockData* stock,
    int64 accountID,
    const wxString& name
) :
    m_stock_n(stock),
    m_edit(stock ? true: false),
    m_account_id(accountID)
{
    long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY, "", wxDefaultPosition, wxSize(400, 300), style, name);
    mmThemeAutoColour(this);
}

bool StockDialog::Create(
    wxWindow* parent,
    wxWindowID id,
    const wxString& caption,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name
) {
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
    if (!m_stock_n)
        return;

    m_stock_id = m_stock_n->m_id;

    m_stock_name_ctrl->SetValue(m_stock_n->m_name);
    m_stock_symbol_ctrl->SetValue(m_stock_n->m_symbol);
    m_notes_ctrl->SetValue(m_stock_n->m_notes);
    m_purchase_date_ctrl->SetValue(m_stock_n->m_purchase_date.getDateTime());

    int precision = m_stock_n->m_num_shares == floor(m_stock_n->m_num_shares)
        ? 0
        : PrefModel::instance().getSharePrecision();
    m_num_shares_ctrl->SetValue(m_stock_n->m_num_shares, precision);
    const AccountData* account_n = AccountModel::instance().get_id_data_n(m_stock_n->m_account_id_n);
    const CurrencyData* currency_n = CurrencyModel::GetBaseCurrency();
    if (account_n)
        currency_n = AccountModel::instance().get_data_currency_p(*account_n);
    int currency_precision = currency_n->precision();
    if (currency_precision < PrefModel::instance().getSharePrecision())
        currency_precision = PrefModel::instance().getSharePrecision();
    m_purchase_price_ctrl->SetValue(m_stock_n->m_purchase_price, account_n, currency_precision);
    m_history_price_ctrl->SetValue(m_stock_n->m_current_price, account_n, currency_precision);
    m_commission_ctrl->SetValue(m_stock_n->m_commission, account_n, currency_precision);
    m_current_price_ctrl->SetValue(m_stock_n->m_current_price, account_n, currency_precision);

    ShowStockHistory();
}

void StockDialog::UpdateControls()
{
    this->SetTitle(m_edit ? _t("Edit Stock Investment") : _t("New Stock Investment"));
    if (m_account_id > -1) {  // do not use for overview
        const AccountData* account_n = AccountModel::instance().get_id_data_n(m_account_id);
        if (m_stock_n) {
            m_value_investment->SetLabelText(AccountModel::instance().value_number_currency(
                *account_n,
                m_stock_n->current_value()
            ));
        }
    }
    else {
        m_value_investment->SetLabelText(wxString::Format(wxT("%.2f"),
            m_stock_n->m_current_price * m_stock_n->m_num_shares
        ));
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

    bool initial_shares = (TrxLinkModel::instance().find_stock_id_c(m_stock_id) == 0);
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
    if (m_stock_n) {
        if (!TrxLinkModel::instance().find_ref_data_a(
            StockModel::s_ref_type, m_stock_n->m_id
        ).empty()) {
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
    m_num_shares_ctrl->SetAltPrecision(PrefModel::instance().getSharePrecision());
    mmToolTip(m_num_shares_ctrl, _t("Enter number of shares.\nUsed when creating the initial Share transaction."));
    m_num_shares_ctrl->Enable(initial_stock_transaction);

    //Purchase Price
    wxStaticText* pprice = new wxStaticText(itemPanel5, wxID_STATIC, _t("*Share Price"));
    itemFlexGridSizer6->Add(pprice, g_flagsH);
    pprice->SetFont(this->GetFont().Bold());
    m_purchase_price_ctrl = new mmTextCtrl(itemPanel5, ID_TEXTCTRL_STOCK_PP, ""
        , wxDefaultPosition, wxSize(150, -1), wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    itemFlexGridSizer6->Add(m_purchase_price_ctrl, g_flagsExpand);
    m_purchase_price_ctrl->SetAltPrecision(PrefModel::instance().getSharePrecision());
    mmToolTip(m_purchase_price_ctrl, _t("Enter the initial price per share.\nUsed when creating the initial Share transaction."));
    m_purchase_price_ctrl->Enable(initial_stock_transaction);

    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _t("Deductible Comm.")), g_flagsH);
    m_commission_ctrl = new mmTextCtrl(itemPanel5, ID_TEXTCTRL_STOCK_COMMISSION, "0"
        , wxDefaultPosition, wxSize(150, -1), wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    itemFlexGridSizer6->Add(m_commission_ctrl, g_flagsExpand);
    m_commission_ctrl->SetAltPrecision(PrefModel::instance().getSharePrecision());
    mmToolTip(m_commission_ctrl, _t("Enter any commission paid.\nUsed when creating the initial Share transaction."));
    m_commission_ctrl->Enable(initial_stock_transaction);

    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _t("Curr. Share Price")), g_flagsH);
    m_current_price_ctrl = new mmTextCtrl(itemPanel5, ID_TEXTCTRL_STOCK_CURR_PRICE, ""
        , wxDefaultPosition, wxSize(150, -1), wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    m_current_price_ctrl->SetAltPrecision(PrefModel::instance().getSharePrecision());
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
    m_history_price_ctrl->SetAltPrecision(PrefModel::instance().getSharePrecision());
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
    if (!m_edit)
        mmAttachmentManage::DeleteAllAttachments(StockModel::s_ref_type, 0);
    EndModal(wxID_CANCEL);
}

void StockDialog::OnCancel(wxCommandEvent& /*event*/)
{
    if (m_stock_id <= 0)
        mmAttachmentManage::DeleteAllAttachments(StockModel::s_ref_type, 0);
    EndModal(wxID_CANCEL);
}

void StockDialog::OnAttachments(wxCommandEvent& /*event*/)
{
    int64 ref_id = (m_stock_id > 0) ? m_stock_id : 0;
    AttachmentDialog dlg(this, StockModel::s_ref_type, ref_id);
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

    const AccountData* account_n = AccountModel::instance().get_id_data_n(m_account_id);
    if (!account_n)
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

    mmDate pdate = mmDate(m_purchase_date_ctrl->GetValue());
    if (pdate < account_n->m_open_date)
        return mmErrorDialogs::ToolTip4Object(
            m_purchase_date_ctrl,
            _t("The opening date for the account is later than the date of this transaction"),
            _t("Invalid Date")
        );

    const wxString stockName = m_stock_name_ctrl->GetValue();
    const wxString notes = m_notes_ctrl->GetValue();

    double numShares = 0;
    if (!m_num_shares_ctrl->checkValue(numShares))
        return;

    double initPrice;
    if (!m_purchase_price_ctrl->checkValue(initPrice))
        return;

    double currentPrice;
    if (!m_current_price_ctrl->GetDouble(currentPrice)) {
        // assume current price = purchase price on stock creation only
        if (!m_stock_n) {
            currentPrice = initPrice;
            m_history_date_ctrl->SetValue(m_purchase_date_ctrl->GetValue());
            m_current_price_ctrl->SetValue(m_purchase_price_ctrl->GetValue());
        }
    }

    double commission = 0;
    m_commission_ctrl->GetDouble(commission);

    double initValue = 0;
    // maintain initial value for records with share accounts
    initValue = (m_stock_n && initPrice == 0)
        ? m_stock_n->m_purchase_value
        : initPrice * numShares;

    if (!m_stock_n) {
        m_stock_d = StockData();
        m_stock_n = &m_stock_d;
    }

    m_stock_n->m_account_id_n   = m_account_id;
    m_stock_n->m_name           = stockName;
    m_stock_n->m_symbol         = stockSymbol;
    m_stock_n->m_num_shares     = numShares;
    m_stock_n->m_purchase_date  = pdate;
    m_stock_n->m_purchase_price = initPrice;
    m_stock_n->m_current_price  = currentPrice;
    m_stock_n->m_purchase_value = initValue;
    m_stock_n->m_commission     = commission;
    m_stock_n->m_notes          = notes;
    StockModel::instance().unsafe_save_data_n(m_stock_n);
    m_stock_id = m_stock_n->id();

    if (!m_edit) {
        // FIXME
        mmAttachmentManage::RelocateAllAttachments(
            StockModel::s_ref_type, 0,
            StockModel::s_ref_type, m_stock_n->m_id
        );
        TrxShareDialog share_dialog(this, m_stock_n);
        share_dialog.ShowModal();
    }

    StockHistoryModel::instance().addUpdate(
        m_stock_n->m_symbol,
        wxDate::Today(),
        m_stock_n->m_current_price,
        StockHistoryModel::MANUAL
    );
    ShowStockHistory();

    m_edit = true;
    UpdateControls();
}

void StockDialog::CreateShareAccount(
    const AccountData* stock_account,
    const wxString& name,
    const wxString& openingDate
) {
    if (name.empty())
        return;

    AccountData new_account_d = AccountData();
    new_account_d.m_name         = name;
    new_account_d.m_type_        = NavigatorTypes::instance().getShareAccountStr();
    new_account_d.m_currency_id  = stock_account->m_currency_id;
    new_account_d.m_open_date    = mmDate(openingDate);
    new_account_d.m_open_balance = 0;
    AccountModel::instance().add_data_n(new_account_d);

    TrxShareDialog share_dialog(this, m_stock_n);
    share_dialog.ShowModal();
}

void StockDialog::OnListItemSelected(wxListEvent& event)
{
    long selectedIndex = event.GetIndex();
    int64 histId = m_price_listbox->GetItemData(selectedIndex);
    const AccountData* account_n = AccountModel::instance().get_id_data_n(
        m_stock_n->m_account_id_n
    );
    const StockHistoryData* sh_n = StockHistoryModel::instance().get_id_data_n(histId);

    if (sh_n->m_id > 0) {
        m_history_date_ctrl->SetValue(StockHistoryModel::DATE(*sh_n));
        m_history_price_ctrl->SetValue(AccountModel::instance().value_number(
            *account_n, sh_n->m_price, PrefModel::instance().getSharePrecision()
        ));
    }
}

void StockDialog::OnHistoryImportButton(wxCommandEvent& /*event*/)
{
    if (m_stock_n->m_symbol.IsEmpty())
        return;

    const wxString fileName = wxFileSelector(
        _t("Choose CSV data file to import"),
        wxEmptyString, wxEmptyString, wxEmptyString,
        "*.csv", wxFD_FILE_MUST_EXIST
    );
    if (fileName.IsEmpty())
        return;
    wxFileName csv_file(fileName);
    if (fileName.IsEmpty() || !csv_file.FileExists())
        return;
    wxTextFile tFile(fileName);
    if (!tFile.Open())
        return;
    wxProgressDialog* progressDlg = new wxProgressDialog(
        _t("Stock History CSV Import"),
        _t("Quotes imported from CSV: "), tFile.GetLineCount(),
        nullptr, wxPD_AUTO_HIDE | wxPD_APP_MODAL | wxPD_SMOOTH | wxPD_CAN_ABORT
    );

    const AccountData* account_n = AccountModel::instance().get_id_data_n(
        m_stock_n->m_account_id_n
    );
    const CurrencyData* currency_p = AccountModel::instance().get_data_currency_p(*account_n);

    bool canceledbyuser = false;
    long countNumTotal = 0;
    long countImported = 0;
    double price;
    wxString dateStr, priceStr;
    StockHistoryModel::DataA new_sh_a;

    wxString line;
    std::vector<wxString> rows;
    for (line = tFile.GetFirstLine(); !tFile.Eof(); line = tFile.GetNextLine()) {
        wxString progressMsg;
        progressMsg << _t("Quotes imported from CSV: ") << countImported;
        if (!progressDlg->Update(countImported, progressMsg)) {
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
        while (tkz.HasMoreTokens()) {
            wxString token = tkz.GetNextToken();
            tokens.push_back(token);
        }

        // date
        wxDateTime dt;
        dateStr = tokens[0];
        mmParseDisplayStringToDate(dt, dateStr, PrefModel::instance().getDateFormat());
        dateStr = dt.FormatISODate();
        // price
        priceStr = tokens[1];
        priceStr.Replace(" ", wxEmptyString);
        if (!CurrencyModel::fromString(priceStr, price, currency_p) || price <= 0.0)
            continue;

        StockHistoryData new_sh_d = StockHistoryData();
        new_sh_d.m_symbol      = m_stock_n->m_symbol;
        new_sh_d.m_date        = mmDate(dateStr);
        new_sh_d.m_price       = price;
        new_sh_d.m_update_type = UpdateType(UpdateType::e_manual);
        new_sh_a.push_back(new_sh_d);

        if (rows.size()<10) {
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
    if (!canceledbyuser && wxMessageBox(
        confirmMsg,
        _t("Importing CSV"),
        wxOK | wxCANCEL | wxICON_INFORMATION
    ) == wxCANCEL) {
        canceledbyuser = true;
    }

    // Since all database transactions are only in memory,
    if (!canceledbyuser) {
        // we need to save them to the database.
        for (auto& new_sh_d : new_sh_a)
            StockHistoryModel::instance().add_data_n(new_sh_d);
        // show the data
        ShowStockHistory();
    }
    else {
        //TODO: and discard the database changes.
    }
}

void StockDialog::OnHistoryDownloadButton(wxCommandEvent& /*event*/)
{
    if (m_stock_n->m_symbol.IsEmpty())
        return;

    /*"ValidRanges":["1d","5d","1mo","3mo","6mo","1y","2y","5y","10y","ytd","max"]*/
    enum { DAY5, MON, MON3, MON6, YEAR, YEAR2, YEAR5, YEAR10, YTD, MAX };
    const wxString ranges[] = { "5d","1mo","3mo","6mo","1y","2y","5y","10y","ytd","max" };
    const std::vector<std::pair<int, wxString>> RANGE_PAIRS =
    {
        { DAY5,   _t("5 Days") },
        { MON,    _t("1 Month") },
        { MON3,   _t("3 Months") },
        { MON6,   _t("6 Months") },
        { YEAR,   _t("1 Year") },
        { YEAR2,  _t("2 Years") },
        { YEAR5,  _t("5 Years") },
        { YEAR10, _t("10 Years") },
        { YTD,    _t("Current Year to Date") },
        { MAX,    _t("Max") }
    };

    wxArrayString items;
    for (const auto& entry : RANGE_PAIRS) { items.Add(entry.second); }

    int range_menu_item_no = wxGetSingleChoiceIndex(
        _t("Specify type frequency of stock history"),
        _t("Stock History Update"),
        items
    );

    if (range_menu_item_no < 0) return;
    const wxString range = ranges[range_menu_item_no];

    wxString interval = "1d";
    if (range != "5d") {
        /* Valid intervals : [1m, 2m, 5m, 15m, 30m, 60m, 90m, 1h, 1d, 5d, 1wk, 1mo, 3mo] */
        enum class i { DAY, DAY5, WEEK, MON, MON3 };
        const wxString intervals[] = { "1d","5d","1wk","1mo","3mo" };
        const std::vector<std::pair<int, wxString> > INTERVAL_PAIRS =
        {
            { static_cast<int>(i::DAY),  _t("1 Day") },
            { static_cast<int>(i::DAY5), _t("5 Days") },
            { static_cast<int>(i::WEEK), _t("1 Week") },
            { static_cast<int>(i::MON),  _t("1 Month") },
            { static_cast<int>(i::MON3), _t("3 Months") }
        };

        items.clear();
        for (const auto& entry : INTERVAL_PAIRS) { items.Add(entry.second); }

        int interval_menu_item_no = wxGetSingleChoiceIndex(
            _t("Specify interval of stock history"),
            _t("Stock History Update"),
            items
        );

        if (interval_menu_item_no < 0) return;
        interval = intervals[interval_menu_item_no];
    }
    const wxString URL = wxString::Format(
        "https://query1.finance.yahoo.com/v8/finance/chart/%s?range=%s&interval=%s&fields=currency",
        m_stock_n->m_symbol, range, interval
    );

    wxString json_data;
    auto err_code = http_get_data(URL, json_data);
    wxString sOutput = json_data;

    if (err_code != CURLE_OK) {
        if (sOutput.empty()) sOutput = _t("Stock history not found.");
        return mmErrorDialogs::MessageError(this, sOutput, _t("Stock History Error"));
    }

    sOutput = _t("Stock history not found.");

    while (true) {
        Document json_doc;
        if (json_doc.Parse(json_data.utf8_str()).HasParseError()) {
            break;
        }
        else if (!json_doc.HasMember("chart") || !json_doc["chart"].IsObject()) {
            break;
        }

        Value chart = json_doc["chart"].GetObject();
        wxASSERT(chart.HasMember("error"));
        if (chart.HasMember("error") && chart["error"].IsObject()) {

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
            if (!e.HasMember("code") || !e.HasMember("description") ||
                !e["code"].IsString() || !e["description"].IsString()
            ) {
                break;
            }

            const wxString code = wxString::FromUTF8(e["code"].GetString());
            const wxString description = wxString::FromUTF8(e["description"].GetString());
            sOutput = wxString::Format("%s - %s", code, description);
            break;
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

        std::map<mmDate, float> date_price_a;
        for (rapidjson::SizeType i = 0; i < timestamp.Size(); i++) {
            if (!timestamp[i].IsInt() || !quotes_closed[i].IsFloat())
                continue;
            mmDate date = mmDate(wxDateTime(static_cast<time_t>(timestamp[i].GetInt())));
            date_price_a[date] = quotes_closed[i].GetFloat() / k;
        }

        StockHistoryModel::instance().db_savepoint();
        for (const auto& date_price : date_price_a) {
            const mmDate date = date_price.first;
            if (date == mmDate::today())
                continue;
            float price = date_price.second;

            if (StockHistoryModel::instance().find(
                StockHistoryCol::SYMBOL(m_stock_n->m_symbol),
                StockHistoryCol::DATE(date.isoDate())
            ).empty() && price > 0) {
                StockHistoryData new_sh_d = StockHistoryData();
                new_sh_d.m_symbol      = m_stock_n->m_symbol;
                new_sh_d.m_date        = date;
                new_sh_d.m_price       = price;
                new_sh_d.m_update_type = UpdateType(UpdateType::e_online);
                StockHistoryModel::instance().add_data_n(new_sh_d);
            }
        }
        StockHistoryModel::instance().db_release_savepoint();
        return ShowStockHistory();
    }
    mmErrorDialogs::MessageError(this, sOutput, _t("Stock History Error"));
}

void StockDialog::OnHistoryAddButton(wxCommandEvent& /*event*/)
{
    if (m_stock_n->m_symbol.IsEmpty())
        return;

    if (!m_history_price_ctrl->IsEnabled()) {
        m_history_price_ctrl->Enable();
        m_history_date_ctrl->Enable();
        return;
    }

    wxString listStr;
    wxDateTime dt;
    double dPrice = 0.0;
    const AccountData* account = AccountModel::instance().get_id_data_n(
        m_stock_n->m_account_id_n
    );
    const CurrencyData* currency = AccountModel::instance().get_data_currency_p(*account);
    wxString currentPriceStr = m_history_price_ctrl->GetValue().Trim();
    if (!CurrencyModel::fromString(currentPriceStr, dPrice, currency) || (dPrice < 0.0))
        return;

    int64 histID = StockHistoryModel::instance().addUpdate(m_stock_n->m_symbol, m_history_date_ctrl->GetValue(), dPrice, StockHistoryModel::MANUAL);
    long i;
    for (i = 0; i < m_price_listbox->GetItemCount(); i++) {
        listStr = m_price_listbox->GetItemText(i, 0);
        mmParseDisplayStringToDate(dt, listStr, PrefModel::instance().getDateFormat());
        if (dt.IsSameDate(m_history_date_ctrl->GetValue()))
            break;
    }
    if (i == m_price_listbox->GetItemCount()) {
        //add
        for (i = 0; i < m_price_listbox->GetItemCount(); i++)
        {
            listStr = m_price_listbox->GetItemText(i, 0);
            mmParseDisplayStringToDate(dt, listStr, PrefModel::instance().getDateFormat());
            if (dt.GetDateOnly() < m_history_date_ctrl->GetValue().GetDateOnly())
                break;
        }
        wxListItem item;
        item.SetId(i);
        item.SetData(reinterpret_cast<void*>(histID.GetValue()));
        m_price_listbox->InsertItem(item);
    }
    if (i != m_price_listbox->GetItemCount()) {
        listStr = AccountModel::instance().value_number(
            *account, dPrice, PrefModel::instance().getSharePrecision()
        );
        m_price_listbox->SetItem(i, 0, mmGetDateTimeForDisplay(
            m_history_date_ctrl->GetValue().FormatISODate()
        ));
        m_price_listbox->SetItem(i, 1, listStr);
    }
    // changed the current price/value
    if (i == m_price_listbox->GetNextItem(-1)) {
        // refresh m_stock_n to get updated attributes
        m_stock_n = StockModel::instance().unsafe_get_id_data_n(m_stock_n->m_id);
        m_current_price_ctrl->SetValue(AccountModel::instance().value_number(
            *account, m_stock_n->m_current_price, PrefModel::instance().getSharePrecision()
        ));
        m_value_investment->SetLabelText(AccountModel::instance().value_number_currency(
            *account,
            m_stock_n->current_value()
        ));
    }
}

void StockDialog::OnHistoryDeleteButton(wxCommandEvent& /*event*/)
{
    if (m_price_listbox->GetSelectedItemCount() <= 0)
        return;

    long item = -1;
    StockHistoryModel::instance().db_savepoint();
    for (;;) {
        item = m_price_listbox->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

        if (item == -1)
            break;
        StockHistoryModel::instance().purge_id(static_cast<int64>(m_price_listbox->GetItemData(item)));
    }
    StockHistoryModel::instance().db_release_savepoint();
    ShowStockHistory();
}

void StockDialog::ShowStockHistory()
{
    m_price_listbox->DeleteAllItems();
    if (m_stock_n->m_symbol.IsEmpty())
        return;

    const AccountData* account = AccountModel::instance().get_id_data_n(
        m_stock_n->m_account_id_n
    );
    StockHistoryModel::DataA sh_a = StockHistoryModel::instance().find(
        StockHistoryCol::SYMBOL(m_stock_n->m_symbol)
    );
    std::stable_sort(sh_a.begin(), sh_a.end(), StockHistoryData::SorterByDATE());
    std::reverse(sh_a.begin(), sh_a.end());
    if (sh_a.size()>300)
        sh_a.resize(300);
    size_t rows = sh_a.size() - 1;
    if (sh_a.empty())
        return;

    for (size_t sh_i = 0; sh_i < sh_a.size(); ++sh_i ) {
        const auto& sh_d = sh_a.at(sh_i);
        wxListItem item;
        item.SetId(static_cast<long>(sh_i));
        item.SetData(reinterpret_cast<void*>(sh_d.m_id.GetValue()));
        m_price_listbox->InsertItem(item);
        const wxString disp_price = AccountModel::instance().value_number(
            *account, sh_d.m_price, PrefModel::instance().getSharePrecision()
        );
        m_price_listbox->SetItem(static_cast<long>(sh_i), 0,
            mmGetDateTimeForDisplay(sh_d.m_date.isoDate())
        );
        m_price_listbox->SetItem(static_cast<long>(sh_i), 1,
            disp_price
        );
        if (sh_i != 0)
            continue;
        m_history_date_ctrl->SetValue(sh_d.m_date.getDateTime());
        m_history_price_ctrl->SetValue(disp_price);
        m_current_price_ctrl->SetValue(disp_price);
        // if the latest share price is not the current stock price, update it.
        if (m_stock_n->m_current_price != sh_d.m_price) {
            StockModel::instance().update_symbol_current_price(
                m_stock_n->m_symbol, sh_d.m_price
            );
            m_stock_n = StockModel::instance().unsafe_get_id_data_n(m_stock_n->m_id);
            m_value_investment->SetLabelText(AccountModel::instance().value_number_currency(
                *(AccountModel::instance().get_id_data_n(m_stock_n->m_account_id_n)),
                m_stock_n->current_value()
            ));
        }
    }
    m_price_listbox->RefreshItems(0, rows);
}

void StockDialog::OnFocusChange(wxChildFocusEvent& event)
{
    UpdateControls();
    event.Skip();
}
