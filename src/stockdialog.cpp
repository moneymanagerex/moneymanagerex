/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2016, 2020 Nikolay Akimov

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

#include "stockdialog.h"
#include "images_list.h"
#include "attachmentdialog.h"
#include "constants.h"
#include "mmSimpleDialogs.h"
#include "mmTextCtrl.h"
#include "paths.h"
#include "util.h"
#include "validators.h"

#include "model/allmodel.h"
#include "accountdialog.h"
#include "sharetransactiondialog.h"
#include "mmframe.h"

#include <wx/numdlg.h>
#include <wx/textdlg.h>
#include <wx/valnum.h>

using namespace rapidjson;
    
IMPLEMENT_DYNAMIC_CLASS(mmStockDialog, wxDialog)

wxBEGIN_EVENT_TABLE(mmStockDialog, wxDialog)
    EVT_CLOSE(mmStockDialog::OnQuit)
    EVT_BUTTON(wxID_SAVE, mmStockDialog::OnSave)
    EVT_BUTTON(wxID_CANCEL, mmStockDialog::OnCancel)
    EVT_BUTTON(wxID_INDEX, mmStockDialog::OnStockPriceButton)
    EVT_BUTTON(wxID_FILE, mmStockDialog::OnAttachments)
    EVT_BUTTON(ID_BUTTON_IMPORT, mmStockDialog::OnHistoryImportButton)
    EVT_BUTTON(ID_BUTTON_DOWNLOAD, mmStockDialog::OnHistoryDownloadButton)
    EVT_BUTTON(wxID_ADD, mmStockDialog::OnHistoryAddButton)
    EVT_BUTTON(wxID_DELETE, mmStockDialog::OnHistoryDeleteButton)
    EVT_CHILD_FOCUS(mmStockDialog::OnFocusChange)
    EVT_LIST_ITEM_SELECTED(wxID_ANY, mmStockDialog::OnListItemSelected)
wxEND_EVENT_TABLE()

mmStockDialog::mmStockDialog( )
{
}

mmStockDialog::mmStockDialog(wxWindow* parent
    , mmGUIFrame* gui_frame
    , Model_Stock::Data* stock
    , int accountID
    , const wxString& name
    )
    : m_stock(stock)
    , m_gui_frame(gui_frame)
    , m_edit(stock ? true: false)
    , m_account_id(accountID)
    , m_stock_name_ctrl(nullptr)
    , m_stock_symbol_ctrl(nullptr)
    , m_purchase_date_ctrl(nullptr)
    , m_num_shares_ctrl(nullptr)
    , m_purchase_price_ctrl(nullptr)
    , m_notes_ctrl(nullptr)
    , m_history_price_ctrl(nullptr)
    , m_history_date_ctrl(nullptr)
    , m_value_investment(nullptr)
    , m_commission_ctrl(nullptr)
    , m_current_price_ctrl(nullptr)
    , m_bAttachments(nullptr)
    , m_price_listbox(nullptr)
{
    long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY, "", wxDefaultPosition, wxSize(400, 300), style, name);
}

bool mmStockDialog::Create(wxWindow* parent, wxWindowID id, const wxString& caption
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
    return TRUE;
}

void mmStockDialog::DataToControls()
{
    if (!this->m_stock) return;

    m_stock_id = m_stock->STOCKID;

    m_stock_name_ctrl->SetValue(m_stock->STOCKNAME);
    m_stock_symbol_ctrl->SetValue(m_stock->SYMBOL);
    m_notes_ctrl->SetValue(m_stock->NOTES);
    m_purchase_date_ctrl->SetValue(Model_Stock::PURCHASEDATE(m_stock));

    int precision = m_stock->NUMSHARES == floor(m_stock->NUMSHARES) ? 0 : Option::instance().SharePrecision();
    m_num_shares_ctrl->SetValue(m_stock->NUMSHARES, precision);
    Model_Account::Data* account = Model_Account::instance().get(m_stock->HELDAT);
    Model_Currency::Data *currency = Model_Currency::GetBaseCurrency();
    if (account) currency = Model_Account::currency(account);
    int currency_precision = Model_Currency::precision(currency);
    if (currency_precision < Option::instance().SharePrecision())
        currency_precision = Option::instance().SharePrecision();
    m_purchase_price_ctrl->SetValue(m_stock->PURCHASEPRICE, account, currency_precision);
    m_history_price_ctrl->SetValue(m_stock->CURRENTPRICE, account, currency_precision);
    m_commission_ctrl->SetValue(m_stock->COMMISSION, account, currency_precision);
    m_current_price_ctrl->SetValue(m_stock->CURRENTPRICE, account, currency_precision);
    
    ShowStockHistory();
}

void mmStockDialog::UpdateControls()
{
    this->SetTitle(m_edit ? _("Edit Stock Investment") : _("New Stock Investment"));
    Model_Account::Data* account = Model_Account::instance().get(m_account_id);

    if (m_stock) {
        m_value_investment->SetLabelText(Model_Account::toCurrency(Model_Stock::instance().CurrentValue(m_stock), account));
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

    bool initial_shares = !Model_Translink::HasShares(m_stock_id);
    m_num_shares_ctrl->Enable(!m_edit || initial_shares);
    m_purchase_date_ctrl->Enable(!m_edit || initial_shares);
    m_purchase_price_ctrl->Enable(!m_edit || initial_shares);
    m_commission_ctrl->Enable(!m_edit || initial_shares);

    m_stock_symbol_ctrl->SetValue(m_stock_symbol_ctrl->GetValue().Upper());
}

void mmStockDialog::CreateControls()
{
    bool initial_stock_transaction = true;
    if (m_stock)
    {
        if (!Model_Translink::TranslinkList(Model_Attachment::STOCK, m_stock->STOCKID).empty())
        {
            initial_stock_transaction = false;
        }
    }

    wxBoxSizer* mainBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    this->SetSizer(mainBoxSizer);

    wxBoxSizer* leftBoxSizer = new wxBoxSizer(wxVERTICAL);
    mainBoxSizer->Add(leftBoxSizer, g_flagsH);

    wxStaticBox* itemStaticBoxSizer4Static = new wxStaticBox(this, wxID_ANY, _("Stock Investment Details"));
    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer(itemStaticBoxSizer4Static, wxVERTICAL);
    leftBoxSizer->Add(itemStaticBoxSizer4, g_flagsExpand);

    wxPanel* itemPanel5 = new wxPanel(this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    itemStaticBoxSizer4->Add(itemPanel5, g_flagsV);

    wxFlexGridSizer* itemFlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
    itemPanel5->SetSizer(itemFlexGridSizer6);

    // Name
    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _("Company Name")), g_flagsH);
    m_stock_name_ctrl = new mmTextCtrl(itemPanel5, ID_TEXTCTRL_STOCKNAME, "");
    itemFlexGridSizer6->Add(m_stock_name_ctrl, g_flagsExpand);
    mmToolTip(m_stock_name_ctrl, _("Enter the stock company name"));

    //Symbol
    wxStaticText* symbol = new wxStaticText(itemPanel5, wxID_STATIC, _("Stock Symbol"));
    itemFlexGridSizer6->Add(symbol, g_flagsH);
    symbol->SetFont(this->GetFont().Bold());

    m_stock_symbol_ctrl = new mmTextCtrl(itemPanel5, ID_TEXTCTRL_STOCK_SYMBOL
        , "", wxDefaultPosition, wxSize(150, -1), 0);
    itemFlexGridSizer6->Add(m_stock_symbol_ctrl, g_flagsExpand);
    mmToolTip(m_stock_symbol_ctrl, _("Enter the stock symbol. (Optional) Include exchange. eg: IBM.BE"));

    //Date
    wxStaticText* date_label = new wxStaticText(itemPanel5, wxID_STATIC, _("*Date"));
    itemFlexGridSizer6->Add(date_label, g_flagsH);
    date_label->SetFont(this->GetFont().Bold());
    m_purchase_date_ctrl = new mmDatePickerCtrl(itemPanel5, ID_DPC_STOCK_PDATE);
    itemFlexGridSizer6->Add(m_purchase_date_ctrl->mmGetLayout());
    mmToolTip(m_purchase_date_ctrl, _("Specify the initial date of the stock investment\nUsed when creating the initial Share transaction."));
    m_purchase_date_ctrl->Enable(initial_stock_transaction);

    //Number of Shares
    wxStaticText* number = new wxStaticText(itemPanel5, wxID_STATIC, _("*Share Number"));
    itemFlexGridSizer6->Add(number, g_flagsH);
    number->SetFont(this->GetFont().Bold());
    m_num_shares_ctrl = new mmTextCtrl(itemPanel5, ID_TEXTCTRL_NUMBER_SHARES, ""
        , wxDefaultPosition, wxSize(150, -1), wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    itemFlexGridSizer6->Add(m_num_shares_ctrl, g_flagsExpand);
    mmToolTip(m_num_shares_ctrl, _("Enter number of shares.\nUsed when creating the initial Share transaction."));
    m_num_shares_ctrl->Connect(ID_TEXTCTRL_NUMBER_SHARES, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmStockDialog::OnTextEntered), nullptr, this);
    m_num_shares_ctrl->Enable(initial_stock_transaction);

    //Purchase Price
    wxStaticText* pprice = new wxStaticText(itemPanel5, wxID_STATIC, _("*Share Price"));
    itemFlexGridSizer6->Add(pprice, g_flagsH);
    pprice->SetFont(this->GetFont().Bold());
    m_purchase_price_ctrl = new mmTextCtrl(itemPanel5, ID_TEXTCTRL_STOCK_PP, ""
        , wxDefaultPosition, wxSize(150, -1), wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    itemFlexGridSizer6->Add(m_purchase_price_ctrl, g_flagsExpand);
    mmToolTip(m_purchase_price_ctrl, _("Enter the initial price per share.\nUsed when creating the initial Share transaction."));
    m_purchase_price_ctrl->Connect(ID_TEXTCTRL_STOCK_PP, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmStockDialog::OnTextEntered), nullptr, this);
    m_purchase_price_ctrl->Enable(initial_stock_transaction);

    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _("*Commission")), g_flagsH);
    m_commission_ctrl = new mmTextCtrl(itemPanel5, ID_TEXTCTRL_STOCK_COMMISSION, "0"
        , wxDefaultPosition, wxSize(150, -1), wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    itemFlexGridSizer6->Add(m_commission_ctrl, g_flagsExpand);
    mmToolTip(m_commission_ctrl, _("Enter any commission paid.\nUsed when creating the initial Share transaction."));
    m_commission_ctrl->Connect(ID_TEXTCTRL_STOCK_COMMISSION, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmStockDialog::OnTextEntered), nullptr, this);
    m_commission_ctrl->Enable(initial_stock_transaction);

    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _("Curr. Share Price")), g_flagsH);
    m_current_price_ctrl = new mmTextCtrl(itemPanel5, ID_TEXTCTRL_STOCK_CURR_PRICE, ""
        , wxDefaultPosition, wxSize(150, -1), wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    mmToolTip(m_current_price_ctrl, _("Enter current stock/share price."));
    itemFlexGridSizer6->Add(m_current_price_ctrl, g_flagsExpand);
    m_current_price_ctrl->Connect(ID_TEXTCTRL_STOCK_COMMISSION, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmStockDialog::OnTextEntered), nullptr, this);

    //
    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _("Current Value")), g_flagsH);
    m_value_investment = new wxStaticText(itemPanel5, ID_STATIC_STOCK_VALUE, "--");
    itemFlexGridSizer6->Add(m_value_investment, g_flagsH);

    //
    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _("Notes")), g_flagsH);
    wxBoxSizer* iconsSizer = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer6->Add(iconsSizer, wxSizerFlags(g_flagsH).Align(wxALIGN_RIGHT));
    m_bAttachments = new wxBitmapButton(itemPanel5, wxID_FILE, mmBitmap(png::CLIP, mmBitmapButtonSize));
    mmToolTip(m_bAttachments, _("Organize attachments of this stock"));
    wxBitmapButton* itemButton31 = new wxBitmapButton(itemPanel5, wxID_INDEX, mmBitmap(png::WEB, mmBitmapButtonSize));
    mmToolTip(itemButton31, _("Display the web page for the specified Stock symbol"));
    iconsSizer->Add(m_bAttachments, g_flagsH);
    iconsSizer->Add(itemButton31, g_flagsH);

    m_notes_ctrl = new mmTextCtrl(this, wxID_STATIC, "", wxDefaultPosition, wxSize(200, 90), wxTE_MULTILINE);
    itemStaticBoxSizer4->Add(m_notes_ctrl, g_flagsExpand);
    itemStaticBoxSizer4->AddSpacer(1);
    mmToolTip(m_notes_ctrl, _("Enter notes associated with this investment"));

    leftBoxSizer->AddSpacer(20);

    //History Panel
    wxBoxSizer* rightBoxSizer = new wxBoxSizer(wxVERTICAL);
    mainBoxSizer->Add(rightBoxSizer, g_flagsExpand);

    wxStaticBox* historyStaticBox = new wxStaticBox(this, wxID_ANY, _("Stock History Options"));
    wxStaticBoxSizer* historyStaticBoxSizer = new wxStaticBoxSizer(historyStaticBox, wxVERTICAL);
    rightBoxSizer->Add(historyStaticBoxSizer, g_flagsExpand);

    m_price_listbox = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(250, 150)
        , wxLC_REPORT);
    historyStaticBoxSizer->Add(m_price_listbox, g_flagsExpand);
    mmToolTip(m_price_listbox, _("Stock Price History"));

    // Add first column
    wxListItem col0;
    col0.SetId(0);
    col0.SetText( _("Date") );
    col0.SetWidth(90);
    m_price_listbox->InsertColumn(0, col0);

    // Add second column
    wxListItem col1;
    col1.SetId(1);
    col1.SetText( _("Price") );
    col1.SetWidth(80);
    m_price_listbox->InsertColumn(1, col1);

    //History Buttons
    wxPanel* buttons_panel = new wxPanel(this, wxID_ANY);
    historyStaticBoxSizer->Add(buttons_panel, wxSizerFlags(g_flagsV).Centre());
    wxBoxSizer* buttons_sizer = new wxBoxSizer(wxVERTICAL);
    buttons_panel->SetSizer(buttons_sizer);

    //
    wxFlexGridSizer* date_price = new wxFlexGridSizer(0, 2, 0, 0);
    date_price->Add(new wxStaticText(buttons_panel, wxID_STATIC, _("Price Date")), g_flagsH);
    m_history_date_ctrl = new mmDatePickerCtrl(buttons_panel, ID_DPC_CP_PDATE
        , wxDefaultDateTime, wxDefaultPosition, wxSize(150, -1), wxDP_DROPDOWN | wxDP_SHOWCENTURY);
    date_price->Add(m_history_date_ctrl, g_flagsH);
    mmToolTip(m_history_date_ctrl, _("Specify the stock/share price date."));

    //
    date_price->Add(new wxStaticText(buttons_panel, wxID_STATIC, _("Price")), g_flagsH);
    m_history_price_ctrl = new mmTextCtrl(buttons_panel, ID_TEXTCTRL_STOCK_CP, ""
        , wxDefaultPosition, wxSize(150, -1), wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    date_price->Add(m_history_price_ctrl, g_flagsH);
    m_history_price_ctrl->Connect(ID_TEXTCTRL_STOCK_CP, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmStockDialog::OnTextEntered), nullptr, this);
    buttons_sizer->Add(date_price);

    m_history_date_ctrl->Enable(false);
    m_history_price_ctrl->Enable(false);

    wxStdDialogButtonSizer*  std_buttons_sizer = new wxStdDialogButtonSizer;
    wxBitmapButton* buttonDownload = new wxBitmapButton(buttons_panel, ID_BUTTON_DOWNLOAD, mmBitmap(png::UPDATE, mmBitmapButtonSize));
    mmToolTip(buttonDownload, _("Download Stock Price history"));
    wxBitmapButton* buttonImport = new wxBitmapButton(buttons_panel, ID_BUTTON_IMPORT, mmBitmap(png::IMPORT, mmBitmapButtonSize));
    mmToolTip(buttonImport, _("Import Stock Price history (CSV Format)"));
    wxButton* buttonDel = new wxButton(buttons_panel, wxID_DELETE, _("&Delete "));
    mmToolTip(buttonDel, _("Delete selected Stock Price"));
    wxButton* buttonAdd = new wxButton(buttons_panel, wxID_ADD, _("&Add "));
    mmToolTip(buttonAdd, _("Add Stock Price to history"));

    std_buttons_sizer->Add(buttonDownload, g_flagsH);
    std_buttons_sizer->Add(buttonImport, g_flagsH);
    std_buttons_sizer->Add(buttonDel, g_flagsH);
    std_buttons_sizer->Add(buttonAdd, g_flagsH);
    buttons_sizer->Add(std_buttons_sizer);

    //OK & Cancel buttons
    wxStdDialogButtonSizer*  buttonsOK_CANCEL_sizer = new wxStdDialogButtonSizer;
    leftBoxSizer->Add(buttonsOK_CANCEL_sizer, wxSizerFlags(g_flagsV).Centre());

    wxButton* itemButtonOK = new wxButton(this, wxID_SAVE, _("&Save "));
    wxButton* itemButton30 = new wxButton(this, wxID_CANCEL, wxGetTranslation(g_CloseLabel));

    if (m_edit)
        itemButton30->SetFocus();
    buttonsOK_CANCEL_sizer->Add(itemButtonOK, g_flagsH);
    buttonsOK_CANCEL_sizer->Add(itemButton30, g_flagsH);
}

void mmStockDialog::OnQuit(wxCloseEvent& /*event*/)
{
    const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::STOCK);
    if (!m_edit)
        mmAttachmentManage::DeleteAllAttachments(RefType, 0);
    EndModal(wxID_CANCEL);
}

void mmStockDialog::OnCancel(wxCommandEvent& /*event*/)
{
    const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::STOCK);
    if (m_stock_id <= 0)
        mmAttachmentManage::DeleteAllAttachments(RefType, 0);
    EndModal(wxID_CANCEL);
}

void mmStockDialog::OnAttachments(wxCommandEvent& /*event*/)
{
    const wxString RefType = Model_Attachment::reftype_desc(Model_Attachment::STOCK);
    int RefId = m_stock_id;

    if (RefId < 0)
        RefId = 0;

    mmAttachmentDialog dlg(this, RefType, RefId);
    dlg.ShowModal();
}

void mmStockDialog::OnStockPriceButton(wxCommandEvent& /*event*/)
{
    const wxString stockSymbol = m_stock_symbol_ctrl->GetValue().Trim();

    if (!stockSymbol.IsEmpty())
    {
        const wxString& stockURL = Model_Infotable::instance().GetStringInfo("STOCKURL", mmex::weblink::DefStockUrl);
        const wxString& httpString = wxString::Format(stockURL, stockSymbol);
        wxLaunchDefaultBrowser(httpString);
    }
}

void mmStockDialog::OnSave(wxCommandEvent& /*event*/)
{
    if (m_price_listbox->GetItemCount())
    {
        for (long i = 0; i<m_price_listbox->GetItemCount(); i++)
        {
            if (m_price_listbox->GetItemState(0, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED)
            {
                m_price_listbox->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
                break;
            }
        }
    }

    Model_Account::Data* account = Model_Account::instance().get(m_account_id);
    if (!account)
    {
        mmErrorDialogs::MessageInvalid(this, _("Held At"));
        return;
    }
    
    const wxString& stockSymbol = m_stock_symbol_ctrl->GetValue();
    if (stockSymbol.empty())
    {
        mmErrorDialogs::MessageInvalid(this, _("Symbol"));
        return;
    }
        
    const wxString pdate = m_purchase_date_ctrl->GetValue().FormatISODate();
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

    if (!m_stock) m_stock = Model_Stock::instance().create();

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

    m_stock_id = Model_Stock::instance().save(m_stock);

    if (!m_edit)
    {
        const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::STOCK);
        mmAttachmentManage::RelocateAllAttachments(RefType, 0, m_stock->STOCKID);
    }

    Model_StockHistory::instance().addUpdate(m_stock->SYMBOL, m_history_date_ctrl->GetValue(), m_stock->CURRENTPRICE, Model_StockHistory::MANUAL);
    ShowStockHistory();

    if (!stockName.empty())
    {

        Model_Account::Data* share_account = Model_Account::instance().get(m_stock_name_ctrl->GetValue());
        if (!share_account && !m_edit)
        {
            if (wxMessageBox(_("Share Account not found.\n\n"
                "Do you want to create one?")
                , _("New Stock Investment"), wxOK | wxCANCEL | wxICON_INFORMATION) == wxOK)
            {
                CreateShareAccount(account, stockName);
            }
        }
        else if (!share_account)
        {
            if (wxMessageBox(_(
                "The Company name does not have an associated Share Account.\n\n"
                "You may want to readjust the Company Name to an existing Share Account with the same name. "
                "If this is an existing Stock without a Share Account, it is recommended that a Share Account is created.\n\n"
                "Do you want to create a new Share Acccount?\n")
                , _("Edit Stock Investment"), wxYES_NO | wxICON_WARNING) == wxYES)
            {
                CreateShareAccount(account, stockName);
            }
        }
    }

    m_edit = true;
    UpdateControls();
}

void mmStockDialog::CreateShareAccount(Model_Account::Data* stock_account, const wxString& name)
{
    if (name.empty()) return;
    Model_Account::Data* share_account = Model_Account::instance().create();
    share_account->ACCOUNTNAME = name;
    share_account->ACCOUNTTYPE = Model_Account::all_type()[Model_Account::SHARES];

    share_account->FAVORITEACCT = "TRUE";
    share_account->STATUS = Model_Account::all_status()[Model_Account::OPEN];
    share_account->INITIALBAL = 0;
    share_account->CURRENCYID = stock_account->CURRENCYID;
    Model_Account::instance().save(share_account);

    mmNewAcctDialog account_dialog(share_account, this);
    account_dialog.ShowModal();

    ShareTransactionDialog share_dialog(this, m_stock);
    share_dialog.ShowModal();
    m_gui_frame->RefreshNavigationTree();
}

void mmStockDialog::OnTextEntered(wxCommandEvent& event)
{
    Model_Currency::Data *currency = Model_Currency::GetBaseCurrency();
    Model_Account::Data *account = Model_Account::instance().get(m_account_id);
    if (account) currency = Model_Account::currency(account);
    int currency_precision = Model_Currency::precision(currency);
    if (currency_precision < Option::instance().SharePrecision())
        currency_precision = Option::instance().SharePrecision();

    if (event.GetId() == m_num_shares_ctrl->GetId())
    {
        m_num_shares_ctrl->Calculate(Option::instance().SharePrecision());
    }
    else if (event.GetId() == m_purchase_price_ctrl->GetId())
    {
        m_purchase_price_ctrl->Calculate(currency_precision);
    }
    else if (event.GetId() == m_history_price_ctrl->GetId())
    {
        m_history_price_ctrl->Calculate(currency_precision);
    }
    else if (event.GetId() == m_commission_ctrl->GetId())
    {
        m_commission_ctrl->Calculate(currency_precision);
    }
}

void mmStockDialog::OnListItemSelected(wxListEvent& event)
{
    long selectedIndex = event.GetIndex();
    long histId = m_price_listbox->GetItemData(selectedIndex);
    Model_Account::Data* account = Model_Account::instance().get(m_stock->HELDAT);
    Model_StockHistory::Data *histData = Model_StockHistory::instance().get(histId);

    if (histData->HISTID > 0)
    {
        m_history_date_ctrl->SetValue(Model_StockHistory::DATE(*histData));
        m_history_price_ctrl->SetValue(Model_Account::toString(histData->VALUE, account, Option::instance().SharePrecision()));
    }
}

void mmStockDialog::OnHistoryImportButton(wxCommandEvent& /*event*/)
{
    if (m_stock->SYMBOL.IsEmpty())
        return;

    const wxString fileName = wxFileSelector(_("Choose CSV data file to import")
        , wxEmptyString, wxEmptyString, wxEmptyString, "*.csv", wxFD_FILE_MUST_EXIST);
    Model_Account::Data *account = Model_Account::instance().get(m_stock->HELDAT);
    Model_Currency::Data *currency = Model_Account::currency(account);

    if (!fileName.IsEmpty())
    {
        wxFileName csv_file(fileName);
        if (fileName.IsEmpty() || !csv_file.FileExists())
            return;
        wxTextFile tFile(fileName);
        if (!tFile.Open())
            return;
        wxProgressDialog* progressDlg = new wxProgressDialog(_("Stock History CSV Import")
            , _("Quotes imported from CSV: "), tFile.GetLineCount()
            , NULL, wxPD_AUTO_HIDE | wxPD_APP_MODAL | wxPD_SMOOTH | wxPD_CAN_ABORT);

        bool canceledbyuser = false;
        long countNumTotal = 0;
        long countImported = 0;
        double price;
        wxString dateStr, priceStr;
        Model_StockHistory::Data *data;
        Model_StockHistory::Cache stockData;

        wxString line;
        std::vector<wxString> rows;
        for (line = tFile.GetFirstLine(); !tFile.Eof(); line = tFile.GetNextLine())
        {
            wxString progressMsg;
            progressMsg << _("Quotes imported from CSV: ") << countImported;
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

            const wxString& delimiter = Model_Infotable::instance().GetStringInfo("DELIMITER", mmex::DEFDELIMTER);
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
            mmParseDisplayStringToDate(dt, dateStr, Option::instance().getDateFormat());
            dateStr = dt.FormatISODate();
            // price
            priceStr = tokens[1];
            priceStr.Replace(" ", wxEmptyString);
            if (!Model_Currency::fromString(priceStr, price, currency) || price <= 0.0)
                continue;

            data = Model_StockHistory::instance().create();
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

        wxString msg = wxString::Format(_("Total Lines : %ld"), countNumTotal); 
        msg << "\n";
        msg << wxString::Format(_("Total Imported : %ld"), countImported); 
        msg << "\n";
        msg << _("Date") << "              " << _("Price");
        msg << "\n";
        for (std::vector<wxString>::const_iterator d = rows.begin(); d != rows.end(); ++d)
            msg << *d << "\n";
        wxString confirmMsg = msg + _("Please confirm saving");
        if (!canceledbyuser && wxMessageBox(confirmMsg
            , _("Importing CSV"), wxOK | wxCANCEL | wxICON_INFORMATION) == wxCANCEL)
        {
            canceledbyuser = true;
        }
 
        // Since all database transactions are only in memory,
        if (!canceledbyuser)
        {
            // we need to save them to the database. 
            for (auto &d : stockData)
                Model_StockHistory::instance().save(d);
            // show the data
            ShowStockHistory();
        }
        else 
        {
            //TODO: and discard the database changes.
        }
    }
}

void mmStockDialog::OnHistoryDownloadButton(wxCommandEvent& /*event*/)
{
    if (m_stock->SYMBOL.IsEmpty())
        return;

    /*"ValidRanges":["1d","5d","1mo","3mo","6mo","1y","2y","5y","10y","ytd","max"]*/
    enum { DAY5, MON, MON3, MON6, YEAR, YEAR2, YEAR5, YEAR10, YTD, MAX };
    const wxString ranges[] = { "5d","1mo","3mo","6mo","1y","2y","5y","10y","ytd","max" };
    const std::vector<std::pair<int, wxString> > RANGE_PAIRS =
    {
        { DAY5, _("5 Days") }
        ,{ MON, _("1 Month") }
        ,{ MON3, _("3 Months") }
        ,{ MON6, _("6 Months") }
        ,{ YEAR, _("1 Year") }
        ,{ YEAR2, _("2 Years") }
        ,{ YEAR5, _("5 Years") }
        ,{ YEAR10, _("10 Years") }
        ,{ YTD, _("Current Year to Date") }
        ,{ MAX, _("Max") }
    };

    wxArrayString items;
    for (const auto& entry : RANGE_PAIRS) { items.Add(entry.second); }

    int range_menu_item_no = wxGetSingleChoiceIndex(_("Specify type frequency of stock history")
        , _("Stock History Update"), items);

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
              { static_cast<int>(i::DAY), _("1 Day") }
            , { static_cast<int>(i::DAY5), _("5 Days") }
            , { static_cast<int>(i::WEEK), _("1 Week") }
            , { static_cast<int>(i::MON), _("1 Month") }
            , { static_cast<int>(i::MON3), _("3 Months") }
        };

        items.clear();
        for (const auto& entry : INTERVAL_PAIRS) { items.Add(entry.second); }

        int interval_menu_item_no = wxGetSingleChoiceIndex(_("Specify interval of stock history")
            , _("Stock History Update"), items);

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
        if (sOutput.empty()) sOutput = _("Stock history not found!");
        return mmErrorDialogs::MessageError(this, sOutput, _("Stock History Error"));
    }

    sOutput = _("Stock history not found!");

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

                // {"chart":{"result":null,"error":{"code":"Not Found","description":"No data found, symbol may be delisted"}}}

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
        Model_StockHistory::instance().Savepoint();
        for (const auto& entry : history)
        {
            float dPrice = entry.second;
            const wxString date_str = wxDateTime(static_cast<time_t>(entry.first)).FormatISODate();
            if (date_str == today) {
                continue;
            }

            if (Model_StockHistory::instance()
                .find(
                    Model_StockHistory::SYMBOL(m_stock->SYMBOL)
                    , Model_StockHistory::DB_Table_STOCKHISTORY_V1::DATE(date_str)
                ).empty()
                && dPrice > 0
                )
            {
                Model_StockHistory::Data* ndata = Model_StockHistory::instance().create();
                ndata->SYMBOL = m_stock->SYMBOL;
                ndata->DATE = date_str;
                ndata->VALUE = dPrice;
                ndata->UPDTYPE = Model_StockHistory::ONLINE;
                Model_StockHistory::instance().save(ndata);
            }
        }
        Model_StockHistory::instance().ReleaseSavepoint();
        return ShowStockHistory();
    }
    mmErrorDialogs::MessageError(this, sOutput, _("Stock History Error"));
}

void mmStockDialog::OnHistoryAddButton(wxCommandEvent& /*event*/)
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
    Model_Account::Data* account = Model_Account::instance().get(m_stock->HELDAT);
    Model_Currency::Data* currency = Model_Account::currency(account);
    wxString currentPriceStr = m_history_price_ctrl->GetValue().Trim();
    if (!Model_Currency::fromString(currentPriceStr, dPrice, currency) || (dPrice < 0.0))
        return;

    long histID = Model_StockHistory::instance().addUpdate(m_stock->SYMBOL, m_history_date_ctrl->GetValue(), dPrice, Model_StockHistory::MANUAL);
    long i;
    for (i = 0; i < m_price_listbox->GetItemCount(); i++)
    {
        listStr = m_price_listbox->GetItemText(i, 0);
        mmParseDisplayStringToDate(dt, listStr, Option::instance().getDateFormat());
        if (dt.IsSameDate(m_history_date_ctrl->GetValue()))
            break;
    }
    if (i == m_price_listbox->GetItemCount())
    {
        //add
        for (i = 0; i < m_price_listbox->GetItemCount(); i++)
        {
            listStr = m_price_listbox->GetItemText(i, 0);
            mmParseDisplayStringToDate(dt, listStr, Option::instance().getDateFormat());
            if (dt.GetDateOnly() < m_history_date_ctrl->GetValue().GetDateOnly())
                break;
        }
        wxListItem item;
        item.SetId(i);
        item.SetData(histID);
        m_price_listbox->InsertItem(item);
    }
    if (i != m_price_listbox->GetItemCount())
    {
        listStr = Model_Account::toString(dPrice, account, Option::instance().SharePrecision());
        m_price_listbox->SetItem(i, 0, mmGetDateForDisplay(m_history_date_ctrl->GetValue().FormatISODate()));
        m_price_listbox->SetItem(i, 1, listStr);
    }
}

void mmStockDialog::OnHistoryDeleteButton(wxCommandEvent& /*event*/)
{
    if (m_price_listbox->GetSelectedItemCount() <= 0)
        return;

    long item = -1;
    Model_StockHistory::instance().Savepoint();
    for (;;)
    {
        item = m_price_listbox->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

        if (item == -1)
            break;
        Model_StockHistory::instance().remove(static_cast<int>(m_price_listbox->GetItemData(item)));
    }
    Model_StockHistory::instance().ReleaseSavepoint();
    ShowStockHistory();
}

void mmStockDialog::ShowStockHistory()
{
    m_price_listbox->DeleteAllItems();
    if (m_stock->SYMBOL.IsEmpty())
        return;

    Model_Account::Data* account = Model_Account::instance().get(m_stock->HELDAT);
    Model_StockHistory::Data_Set histData = Model_StockHistory::instance().find(Model_StockHistory::SYMBOL(m_stock->SYMBOL));
    std::stable_sort(histData.begin(), histData.end(), SorterByDATE());
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
            item.SetData(histData.at(idx).HISTID);
            m_price_listbox->InsertItem(item);
            const wxDate dtdt = Model_StockHistory::DATE(histData.at(idx));
            const wxString dispAmount = Model_Account::toString(histData.at(idx).VALUE, account, Option::instance().SharePrecision());
            m_price_listbox->SetItem(static_cast<long>(idx), 0, mmGetDateForDisplay(histData.at(idx).DATE));
            m_price_listbox->SetItem(static_cast<long>(idx), 1, dispAmount);
            if (idx == 0)
            {
                m_history_date_ctrl->SetValue(dtdt);
                m_history_price_ctrl->SetValue(dispAmount);
            }
        }
        m_price_listbox->RefreshItems(0, rows);
    }
}

void mmStockDialog::OnFocusChange(wxChildFocusEvent& event)
{
    UpdateControls();
    event.Skip();
}
