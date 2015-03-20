/*******************************************************
 Copyright (C) 2015 Stefano Giorgio

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

#include "stock_transdialog.h"
#include "attachmentdialog.h"
#include "constants.h"
#include "mmSimpleDialogs.h"
#include "mmtextctrl.h"
#include "paths.h"
#include "util.h"
#include "validators.h"

#include "model/Model_Account.h"
#include "model/Model_Attachment.h"
#include "mmUserPanelTrans.h"

#include "../resources/attachment.xpm"
#include "../resources/checkupdate.xpm"
#include"../resources/uparrow.xpm"
#include "../resources/web.xpm"

#include <wx/numdlg.h>
#include <wx/textdlg.h>
#include <wx/valnum.h>

IMPLEMENT_DYNAMIC_CLASS(mmStockTransDialog, wxDialog)

wxBEGIN_EVENT_TABLE(mmStockTransDialog, wxDialog)
    EVT_BUTTON(wxID_OK, mmStockTransDialog::OnOk)
    EVT_BUTTON(wxID_CANCEL, mmStockTransDialog::OnCancel)
    EVT_BUTTON(wxID_FILE, mmStockTransDialog::OnAttachments)
    EVT_BUTTON(wxID_INDEX, mmStockTransDialog::OnStockPriceButton)
    EVT_CLOSE(mmStockTransDialog::OnQuit)
    EVT_DATE_CHANGED(ID_STOCKTRANS_DATEPICKER_CHANGE, mmStockTransDialog::OnDateChange)
    EVT_TEXT(ID_STOCKTRANS_NUMBER_SHARES, mmStockTransDialog::OnTextEntered)
    EVT_TEXT(ID_STOCKTRANS_PURCHASEPRICE, mmStockTransDialog::OnTextEntered)
    EVT_TEXT(ID_STOCKTRANS_COMMISSION, mmStockTransDialog::OnTextEntered)
wxEND_EVENT_TABLE()

mmStockTransDialog::mmStockTransDialog()
{
}

mmStockTransDialog::mmStockTransDialog(wxWindow* parent, Model_Stock::Data* stock, int account_id)
    : m_stock(stock)
    , m_account_id(account_id)
    , m_stock_name(nullptr)
    , m_stock_symbol(nullptr)
    , m_dpc(nullptr)
    , m_num_shares(nullptr)
    , m_purchase_price(nullptr)
    , m_notes(nullptr)
    , m_commission(nullptr)
    , m_attachments(nullptr)
    , m_transfer_entry(nullptr)
    , m_checking_entry(nullptr)
{
    m_dialog_heading = _("New Stock Investment / Transaction");
    if (m_stock)
        m_dialog_heading = _("Add Stock Transactions");

    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY, m_dialog_heading, wxDefaultPosition, wxSize(400, 300), style);
}

mmStockTransDialog::mmStockTransDialog(wxWindow* parent, Model_TransferTrans::Data* transfer_entry, Model_Checking::Data* checking_entry)
    : m_stock(nullptr)
    , m_account_id(-1)
    , m_stock_name(nullptr)
    , m_stock_symbol(nullptr)
    , m_dpc(nullptr)
    , m_num_shares(nullptr)
    , m_purchase_price(nullptr)
    , m_notes(nullptr)
    , m_commission(nullptr)
    , m_attachments(nullptr)
    , m_transfer_entry(transfer_entry)
    , m_checking_entry(checking_entry)
{
    m_dialog_heading = _("Add Stock Transactions");
    if (transfer_entry)
    {
        m_dialog_heading = _("Edit Stock Transaction");
        m_stock = Model_Stock::instance().get(transfer_entry->ID_TABLERECORD);
        m_account_id = m_stock->HELDAT;
    }
    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY, m_dialog_heading, wxDefaultPosition, wxSize(400, 300), style);
}

bool mmStockTransDialog::Create(wxWindow* parent, wxWindowID id, const wxString& caption
    , const wxPoint& pos, const wxSize& size, long style)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style);

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    SetIcon(mmex::getProgramIcon());

    DataToControls();

    Centre();
    return TRUE;
}

void mmStockTransDialog::DataToControls()
{
    if (!m_stock) return;

    m_stock_id = m_stock->STOCKID;

    m_stock_name->SetValue(m_stock->STOCKNAME);
    m_dpc->SetValue(Model_Stock::PURCHASEDATE(m_stock));
    m_stock_symbol->SetValue(m_stock->SYMBOL);
    m_notes->SetValue(m_stock->NOTES);

    m_stock_name->Enable(false);
    m_dpc->Enable(false);
    m_stock_symbol->Enable(false);
    m_notes->Enable(false);
    
    // Set up the transaction if this is the first entry.
    if (Model_TransferTrans::instance().TransferList(Model_TransferTrans::TABLE_TYPE::STOCKS, m_stock->STOCKID).empty())
    {
        m_num_shares->SetValue(m_stock->NUMSHARES);
        m_purchase_price->SetValue(m_stock->PURCHASEPRICE);
        m_commission->SetValue(m_stock->COMMISSION);
        m_transaction_panel->SetTransactionDate(Model_Stock::PURCHASEDATE(m_stock));
        m_transaction_panel->SetTransactionValue((m_stock->NUMSHARES * m_stock->PURCHASEPRICE) + m_stock->COMMISSION);
    }
    else
    {
        if (m_transfer_entry)
        {
            m_num_shares->SetValue(m_transfer_entry->SHARE_NUMBER);
            m_purchase_price->SetValue(m_transfer_entry->SHARE_UNITPRICE);
            m_commission->SetValue(m_transfer_entry->SHARE_COMMISSION);
            m_transaction_panel->SetTransactionDate(Model_Stock::PURCHASEDATE(m_stock));
            m_transaction_panel->SetTransactionValue((m_transfer_entry->SHARE_NUMBER * m_transfer_entry->SHARE_UNITPRICE) + m_transfer_entry->SHARE_COMMISSION);
        }
        else
        {
            m_num_shares->SetValue(0);
            m_purchase_price->SetValue(0);
        }
    }
}

void mmStockTransDialog::CreateControls()
{
    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(main_sizer);

    wxBoxSizer* panel_sizer = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* left_sizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* right_sizer = new wxBoxSizer(wxVERTICAL);

    main_sizer->Add(panel_sizer, wxSizerFlags(g_flags));
    panel_sizer->Add(left_sizer, 0);
    panel_sizer->Add(right_sizer, 0);

    /********************************************************************
    Stock Details Panel
    *********************************************************************/
    wxStaticBox* details_frame = new wxStaticBox(this, wxID_ANY, _("Stock Investment Details"));
    wxStaticBoxSizer* details_frame_sizer = new wxStaticBoxSizer(details_frame, wxVERTICAL);
    left_sizer->Add(details_frame_sizer, g_flags);

    wxPanel* stock_details_panel = new wxPanel(this, wxID_STATIC);
    details_frame_sizer->Add(stock_details_panel, g_flags);

    wxFlexGridSizer* itemFlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
    stock_details_panel->SetSizer(itemFlexGridSizer6);

    itemFlexGridSizer6->Add(new wxStaticText(stock_details_panel, wxID_STATIC, _("Company Name")), g_flags);

    m_stock_name = new mmTextCtrl(stock_details_panel, ID_STOCKTRANS_NAME, "");
    m_stock_name->SetToolTip(_("Enter the stock company name"));
    itemFlexGridSizer6->Add(m_stock_name, g_flagsExpand);

    //Purchase Date
    itemFlexGridSizer6->Add(new wxStaticText(stock_details_panel, wxID_STATIC, _("Date")), g_flags);

    m_dpc = new wxDatePickerCtrl(stock_details_panel, ID_STOCKTRANS_DATEPICKER_CHANGE
        , wxDefaultDateTime, wxDefaultPosition, wxSize(150, -1), wxDP_DROPDOWN | wxDP_SHOWCENTURY);
    itemFlexGridSizer6->Add(m_dpc, g_flags);
    m_dpc->SetToolTip(_("Specify the purchase date of the stock investment"));

    //Symbol
    wxStaticText* symbol = new wxStaticText(stock_details_panel, wxID_STATIC, _("Stock Symbol"));
    itemFlexGridSizer6->Add(symbol, g_flags);
    symbol->SetFont(this->GetFont().Bold());

    m_stock_symbol = new mmTextCtrl(stock_details_panel, ID_STOCKTRANS_SYMBOL
        , "", wxDefaultPosition, wxSize(150, -1), 0);
    itemFlexGridSizer6->Add(m_stock_symbol, g_flags);
    m_stock_symbol->SetToolTip(_("Enter the stock symbol. (Optional) Include exchange. eg: IBM.BE"));

    //Number of Shares
    wxStaticText* number = new wxStaticText(stock_details_panel, wxID_STATIC, _("Number of Shares"));
    itemFlexGridSizer6->Add(number, g_flags);
    number->SetFont(this->GetFont().Bold());
    m_num_shares = new mmTextCtrl(stock_details_panel, ID_STOCKTRANS_NUMBER_SHARES, ""
        , wxDefaultPosition, wxSize(150, -1), wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    itemFlexGridSizer6->Add(m_num_shares, g_flags);
    m_num_shares->SetToolTip(_("Enter number of shares held"));
    m_num_shares->Connect(ID_STOCKTRANS_NUMBER_SHARES, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmStockTransDialog::OnTextEntered), nullptr, this);

    //Purchase Price
    wxStaticText* pprice = new wxStaticText(stock_details_panel, wxID_STATIC, _("Purchase Price"));
    pprice->SetFont(this->GetFont().Bold());
    m_purchase_price = new mmTextCtrl(stock_details_panel, ID_STOCKTRANS_PURCHASEPRICE, ""
        , wxDefaultPosition, wxSize(150, -1), wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    itemFlexGridSizer6->Add(pprice, g_flags);
    itemFlexGridSizer6->Add(m_purchase_price, g_flags);
    m_purchase_price->SetToolTip(_("Enter purchase price for each stock"));
    m_purchase_price->Connect(ID_STOCKTRANS_PURCHASEPRICE, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmStockTransDialog::OnTextEntered), nullptr, this);

    m_commission = new mmTextCtrl(stock_details_panel, ID_STOCKTRANS_COMMISSION, "0"
        , wxDefaultPosition, wxSize(150, -1), wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    itemFlexGridSizer6->Add(new wxStaticText(stock_details_panel, wxID_STATIC, _("Commission")), g_flags);
    itemFlexGridSizer6->Add(m_commission, g_flags);
    m_commission->SetToolTip(_("Enter any commission paid"));
    m_commission->Connect(ID_STOCKTRANS_COMMISSION, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmStockTransDialog::OnTextEntered), nullptr, this);

    itemFlexGridSizer6->AddSpacer(8);
    itemFlexGridSizer6->AddSpacer(8);
    //Notes
    itemFlexGridSizer6->Add(new wxStaticText(stock_details_panel, wxID_STATIC, _("Notes")), g_flags);

    //Attachments
    m_attachments = new wxBitmapButton(stock_details_panel, wxID_FILE
        , wxBitmap(attachment_xpm), wxDefaultPosition
        , wxSize(m_commission->GetSize().GetY(), m_commission->GetSize().GetY()));
    m_attachments->SetToolTip(_("Organize attachments of this stock"));
    //TODO m_attachments not used here
    m_attachments->Hide();

    wxBitmapButton* web_button = new wxBitmapButton(stock_details_panel, wxID_INDEX, wxBitmap(web_xpm)
        , wxDefaultPosition, wxSize(m_commission->GetSize().GetY(), m_commission->GetSize().GetY()));
    web_button->SetToolTip(_("Display the web page for the specified Stock symbol"));

    wxBoxSizer* icon_sizer = new wxBoxSizer(wxHORIZONTAL);
    icon_sizer->Add(m_attachments, g_flags);
    icon_sizer->Add(web_button, g_flags);

    m_notes = new mmTextCtrl(this, wxID_STATIC, "", wxDefaultPosition, wxSize(200, 162), wxTE_MULTILINE);
    m_notes->SetToolTip(_("Enter notes associated with this investment"));

    itemFlexGridSizer6->Add(icon_sizer, wxSizerFlags(g_flags).Align(wxALIGN_RIGHT).Border(wxALL, 0));
    details_frame_sizer->Add(m_notes, 0, wxGROW | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    /********************************************************************
    Transaction Panel
    *********************************************************************/
    wxStaticBox* transaction_frame = new wxStaticBox(this, wxID_ANY, _("New Transaction Details"));
    wxStaticBoxSizer* transaction_frame_sizer = new wxStaticBoxSizer(transaction_frame, wxVERTICAL);
    right_sizer->Add(transaction_frame_sizer, g_flags);

    m_transaction_panel = new mmUserPanelTrans(this, m_checking_entry, wxID_STATIC);
    transaction_frame_sizer->Add(m_transaction_panel, g_flags);
    if (m_transfer_entry && m_checking_entry)
    {
        m_transaction_panel->SetCheckingType(Model_TransferTrans::type_checking(m_checking_entry->TOACCOUNTID));
    }
    else
    {
        wxString acc_held = Model_Account::get_account_name(m_account_id);
        m_transaction_panel->SetTransactionNumber(acc_held);
    }

    /********************************************************************
    Separation Line
    *********************************************************************/
    wxStaticLine* separation_line = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
    main_sizer->Add(separation_line, 0, wxEXPAND | wxALL, 1);

    /********************************************************************
    Button Panel
    *********************************************************************/
    wxPanel* button_panel = new wxPanel(this, wxID_STATIC);
    wxBoxSizer* button_panel_sizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* ok_button = new wxButton(button_panel, wxID_OK, _("&OK "));
    wxButton* close_button = new wxButton(button_panel, wxID_CANCEL, _("&Cancel "));

    main_sizer->Add(button_panel, wxSizerFlags(g_flags).Center());
    button_panel->SetSizer(button_panel_sizer);
    button_panel_sizer->Add(ok_button, g_flags);
    button_panel_sizer->Add(close_button, g_flags);
    //cancel_button->SetFocus();
}

void mmStockTransDialog::OnQuit(wxCloseEvent& WXUNUSED(event))
{
    const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::STOCK);
    if (!this->m_stock)
        mmAttachmentManage::DeleteAllAttachments(RefType, 0);
    EndModal(wxID_CANCEL);
}

void mmStockTransDialog::OnDateChange(wxDateEvent& WXUNUSED(event))
{
    m_transaction_panel->SetTransactionDate(m_dpc->GetValue());
}

void mmStockTransDialog::OnCancel(wxCommandEvent& WXUNUSED(event))
{
    const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::STOCK);
    if (m_stock_id <= 0)
        mmAttachmentManage::DeleteAllAttachments(RefType, 0);
    EndModal(wxID_CANCEL);
}

void mmStockTransDialog::OnAttachments(wxCommandEvent& WXUNUSED(event))
{
    const wxString RefType = Model_Attachment::reftype_desc(Model_Attachment::STOCK);
    int RefId = m_stock_id;

    if (RefId < 0)
        RefId = 0;

    mmAttachmentDialog dlg(this, RefType, RefId);
    dlg.ShowModal();
}

void mmStockTransDialog::OnStockPriceButton(wxCommandEvent& WXUNUSED(event))
{
    const wxString stockSymbol = m_stock_symbol->GetValue().Trim();

    if (!stockSymbol.IsEmpty())
    {
        const wxString& stockURL = Model_Infotable::instance().GetStringInfo("STOCKURL", mmex::weblink::DefStockUrl);
        const wxString& httpString = wxString::Format(stockURL, stockSymbol);
        wxLaunchDefaultBrowser(httpString);
    }
}

void mmStockTransDialog::OnOk(wxCommandEvent& WXUNUSED(event))
{
    Model_Account::Data* account = Model_Account::instance().get(m_account_id);
    if (!account)
    {
        mmErrorDialogs::MessageInvalid(this, _("Held At"));
        return;
    }
    
    const wxString& stock_symbol = m_stock_symbol->GetValue();
    if (stock_symbol.empty()) {
        mmErrorDialogs::MessageInvalid(this, _("Symbol"));
        return;
    }

    Model_Currency::Data* currency = m_transaction_panel->GetCurrencyData();

    double num_shares = 0;
    if (!m_num_shares->checkValue(num_shares, currency))
    {
        return;
    }

    double purchase_price;
    if (!m_purchase_price->checkValue(purchase_price, currency))
    {
        return;
    }

    double commission = 0;
    m_commission->GetDouble(commission);

    int old_stock_id = -1;

    if (m_stock)
    {
        old_stock_id = m_stock->STOCKID;
    }
    else // create a new stock record if the record does not exist
    {
        m_stock = Model_Stock::instance().create();

        m_stock->HELDAT = m_account_id;
        m_stock->PURCHASEDATE = m_dpc->GetValue().FormatISODate();
        m_stock->STOCKNAME = m_stock_name->GetValue();
        m_stock->SYMBOL = stock_symbol;
        m_stock->NUMSHARES = num_shares;
        m_stock->PURCHASEPRICE = purchase_price;
        m_stock->NOTES = m_notes->GetValue();
        m_stock->CURRENTPRICE = purchase_price;
        m_stock->VALUE = (num_shares * purchase_price) + commission;
        m_stock->COMMISSION = commission;
        
        Model_Stock::instance().save(m_stock);
    }

    if (old_stock_id < 0)
    {
        const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::STOCK);
        mmAttachmentManage::RelocateAllAttachments(RefType, 0, m_stock->STOCKID);
    }

    if (m_transaction_panel->ValidCheckingAccountEntry())
    {
        int checking_id = m_transaction_panel->SaveChecking();
        // Check if this is the first entry

        if (m_transfer_entry)
        {
            m_transfer_entry->SHARE_UNITPRICE = purchase_price;
            m_transfer_entry->SHARE_NUMBER = num_shares;
            m_transfer_entry->SHARE_COMMISSION = commission;
            
            Model_TransferTrans::instance().save(m_transfer_entry);
            EndModal(wxID_SAVE);
        }

        // The Stocks table maintains the total number of shares.
        // The Unit price field in the Stocks table becomes obsolete.

        // if we get to this point, setup the transfer table
        Model_TransferTrans::Data_Set trans_list = Model_TransferTrans::TransferList(Model_TransferTrans::TABLE_TYPE::STOCKS, m_stock->STOCKID);
        if (!trans_list.empty())
        {
            if (num_shares > 0) // addition or removal shares
            {
                m_stock->CURRENTPRICE = purchase_price;
                m_stock->COMMISSION += commission;
                if (m_transaction_panel->TransactionType() == Model_Checking::WITHDRAWAL)
                {
                    m_stock->NUMSHARES += num_shares;
                }
                else
                {
                    m_stock->NUMSHARES -= num_shares;
                    // we need to subtract the number of shares for a sale
                    num_shares = num_shares * -1;
                }
            }
        }

        Model_TransferTrans::SetShareTransferTransaction(m_stock->STOCKID, checking_id
            , purchase_price, num_shares, commission
            , m_transaction_panel->CheckingType()
            , m_transaction_panel->CurrencySymbol());
        Model_TransferTrans::UpdateStockValue(m_stock);
    }
    else
    {
        mmErrorDialogs::MessageWarning(this, _("Invalid Transaction"), m_dialog_heading);
        return;
    }

    EndModal(wxID_OK);
}

void mmStockTransDialog::OnTextEntered(wxCommandEvent& event)
{
    Model_Currency::Data* currency = m_transaction_panel->GetCurrencyData();

    double num_shares = 0;
    if (!m_num_shares->GetValue().empty())
        m_num_shares->GetDouble(num_shares, currency);

    double purchase_price = 0;
    if (!m_purchase_price->GetValue().empty())
        m_purchase_price->GetDouble(purchase_price, currency);

    double commission = 0;
    if (!m_commission->GetValue().empty())
        m_commission->GetDouble(commission);
    if (num_shares > 0)
    {
        m_transaction_panel->SetTransactionValue((num_shares * purchase_price) + commission);
    }
}

