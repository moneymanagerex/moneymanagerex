/*******************************************************
 Copyright (C) 2016 Stefano Giorgio

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

#include "sharetransactiondialog.h"
#include "images_list.h"
#include "attachmentdialog.h"
#include "constants.h"
#include "mmSimpleDialogs.h"
#include "mmtextctrl.h"
#include "paths.h"
#include "util.h"
#include "validators.h"

#include "model/Model_Account.h"
#include "model/Model_Attachment.h"
#include "usertransactionpanel.h"

#include <wx/numdlg.h>
#include <wx/textdlg.h>
#include <wx/valnum.h>

IMPLEMENT_DYNAMIC_CLASS(ShareTransactionDialog, wxDialog)

wxBEGIN_EVENT_TABLE(ShareTransactionDialog, wxDialog)
    EVT_BUTTON(wxID_OK, ShareTransactionDialog::OnOk)
    EVT_BUTTON(wxID_CANCEL, ShareTransactionDialog::OnCancel)
    EVT_BUTTON(wxID_FILE, ShareTransactionDialog::OnAttachments)
    EVT_BUTTON(wxID_INDEX, ShareTransactionDialog::OnStockPriceButton)
    EVT_CLOSE(ShareTransactionDialog::OnQuit)
    EVT_TEXT(ID_STOCKTRANS_SHARE_NUMBER, ShareTransactionDialog::OnTextEntered)
    EVT_TEXT(ID_STOCKTRANS_SHARE_PRICE, ShareTransactionDialog::OnTextEntered)
    EVT_TEXT(ID_STOCKTRANS_SHARE_COMMISSION, ShareTransactionDialog::OnTextEntered)
wxEND_EVENT_TABLE()

ShareTransactionDialog::ShareTransactionDialog()
{
}

ShareTransactionDialog::ShareTransactionDialog(wxWindow* parent, Model_Stock::Data* stock, int account_id)
    : m_stock(stock)
    , m_account_id(account_id)
    , m_stock_name(nullptr)
    , m_stock_symbol(nullptr)
    , m_share_num(nullptr)
    , m_share_price(nullptr)
    , m_share_lot(nullptr)
    , m_notes(nullptr)
    , m_commission(nullptr)
    , m_attachments(nullptr)
    , m_translink_entry(nullptr)
    , m_share_entry(nullptr)
    , m_checking_entry(nullptr)
{
    m_dialog_heading = _("New Stock Investment / Transaction");
    if (m_stock)
    {
        m_dialog_heading = _("Add Share Transactions");
    }

    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY, m_dialog_heading, wxDefaultPosition, wxSize(400, 300), style);
}

ShareTransactionDialog::ShareTransactionDialog(wxWindow* parent, Model_Translink::Data* translink_entry, Model_Checking::Data* checking_entry)
    : m_stock(nullptr)
    , m_account_id(-1)
    , m_stock_name(nullptr)
    , m_stock_symbol(nullptr)
    , m_share_num(nullptr)
    , m_share_price(nullptr)
    , m_share_lot(nullptr)
    , m_notes(nullptr)
    , m_commission(nullptr)
    , m_attachments(nullptr)
    , m_translink_entry(translink_entry)
    , m_checking_entry(checking_entry)
{
    m_dialog_heading = _("Add Share Transactions");
    if (translink_entry)
    {
        m_dialog_heading = _("Edit Share Transaction");
        m_stock = Model_Stock::instance().get(translink_entry->LINKRECORDID);
        m_account_id = m_stock->HELDAT;

        m_share_entry = Model_Shareinfo::ShareEntry(translink_entry->CHECKINGACCOUNTID);
    }
    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY, m_dialog_heading, wxDefaultPosition, wxSize(400, 300), style);
}

bool ShareTransactionDialog::Create(wxWindow* parent, wxWindowID id, const wxString& caption
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

void ShareTransactionDialog::DataToControls()
{
    if (!m_stock) return;

    m_stock_id = m_stock->STOCKID;
    m_stock_name->SetValue(m_stock->STOCKNAME);
    m_transaction_panel->SetTransactionAccount(m_stock->STOCKNAME);
    m_stock_symbol->SetValue(m_stock->SYMBOL);
    m_notes->SetValue(m_stock->NOTES);

    m_stock_name->Enable(false);
    m_stock_symbol->Enable(false);
    m_notes->Enable(false);

    Model_Translink::Data_Set translink_list = Model_Translink::TranslinkList(Model_Attachment::STOCK, m_stock->STOCKID);

    if (translink_list.empty())
    {   // Set up the transaction as the first entry.
        m_share_num->SetValue(m_stock->NUMSHARES, 0);
        m_share_price->SetValue(m_stock->PURCHASEPRICE, 4);
        m_commission->SetValue(m_stock->COMMISSION, 4);
        m_transaction_panel->TransactionDate(Model_Stock::PURCHASEDATE(m_stock));
        m_transaction_panel->SetTransactionValue(
            ( m_stock->NUMSHARES * m_stock->PURCHASEPRICE) + m_stock->COMMISSION, true);
    }
    else
    {
        if (m_translink_entry && &m_share_entry)
        {
            m_share_num->SetValue(m_share_entry.SHARENUMBER, 0);
            m_share_price->SetValue(m_share_entry.SHAREPRICE, 4);
            m_commission->SetValue(m_share_entry.SHARECOMMISSION);
            m_share_lot->SetValue(m_share_entry.SHARELOT);

            Model_Checking::Data* checking_entry = Model_Checking::instance().get(m_translink_entry->CHECKINGACCOUNTID);
            m_transaction_panel->TransactionDate(Model_Checking::TRANSDATE(checking_entry));
            m_transaction_panel->SetTransactionValue(
                (m_share_entry.SHARENUMBER * m_share_entry.SHAREPRICE) + m_share_entry.SHARECOMMISSION, true);
        }
        else
        {
            m_share_num->SetValue(0, 0);
            m_share_price->SetValue(0, 4);
        }
    }
}

void ShareTransactionDialog::CreateControls()
{
    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(main_sizer);

    wxBoxSizer* panel_sizer = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* left_sizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* right_sizer = new wxBoxSizer(wxVERTICAL);

    main_sizer->Add(panel_sizer, wxSizerFlags(g_flagsV));
    panel_sizer->Add(left_sizer, 0);
    panel_sizer->Add(right_sizer, 0);

    /********************************************************************
    Stock Details Panel
    *********************************************************************/
    wxStaticBox* details_frame = new wxStaticBox(this, wxID_ANY, _("Stock Investment Details"));
    wxStaticBoxSizer* details_frame_sizer = new wxStaticBoxSizer(details_frame, wxVERTICAL);
    left_sizer->Add(details_frame_sizer, g_flagsExpand);

    wxPanel* stock_details_panel = new wxPanel(this, wxID_STATIC);
    details_frame_sizer->Add(stock_details_panel, g_flagsV);

    wxFlexGridSizer* itemFlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
    stock_details_panel->SetSizer(itemFlexGridSizer6);

    itemFlexGridSizer6->Add(new wxStaticText(stock_details_panel, wxID_STATIC, _("Company Name")), g_flagsH);

    m_stock_name = new mmTextCtrl(stock_details_panel, ID_STOCKTRANS_SHARE_NAME, "");
    itemFlexGridSizer6->Add(m_stock_name, g_flagsExpand);
    m_stock_name->SetToolTip(_("Enter the stock company name"));

    //Symbol
    wxStaticText* symbol = new wxStaticText(stock_details_panel, wxID_STATIC, _("Stock Symbol"));
    itemFlexGridSizer6->Add(symbol, g_flagsH);
    symbol->SetFont(this->GetFont().Bold());

    m_stock_symbol = new mmTextCtrl(stock_details_panel, ID_STOCKTRANS_SHARE_SYMBOL
        , "", wxDefaultPosition, wxSize(150, -1), 0);
    itemFlexGridSizer6->Add(m_stock_symbol, g_flagsH);
    m_stock_symbol->SetToolTip(_("Enter the stock symbol. (Optional) Include exchange. eg: IBM.BE"));

    //Share Number
    wxStaticText* number = new wxStaticText(stock_details_panel, wxID_STATIC, _("Unit Number"));
    itemFlexGridSizer6->Add(number, g_flagsH);
    number->SetFont(this->GetFont().Bold());
    m_share_num = new mmTextCtrl(stock_details_panel, ID_STOCKTRANS_SHARE_NUMBER, ""
        , wxDefaultPosition, wxSize(150, -1), wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    itemFlexGridSizer6->Add(m_share_num, g_flagsH);
    m_share_num->SetToolTip(_("Enter number of shares held"));
    m_share_num->Connect(ID_STOCKTRANS_SHARE_NUMBER, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(ShareTransactionDialog::OnTextEntered), nullptr, this);

    //Share Price
    wxStaticText* pprice = new wxStaticText(stock_details_panel, wxID_STATIC, _("Share Price"));
    pprice->SetFont(this->GetFont().Bold());
    m_share_price = new mmTextCtrl(stock_details_panel, ID_STOCKTRANS_SHARE_PRICE, ""
        , wxDefaultPosition, wxSize(150, -1), wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    itemFlexGridSizer6->Add(pprice, g_flagsH);
    itemFlexGridSizer6->Add(m_share_price, g_flagsH);
    m_share_price->SetToolTip(_("Enter the current value for a single share unit"));
    m_share_price->Connect(ID_STOCKTRANS_SHARE_PRICE, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(ShareTransactionDialog::OnTextEntered), nullptr, this);

    //Share Lot
    wxStaticText* lot_text = new wxStaticText(stock_details_panel, wxID_STATIC, _("Share Lot"));
    itemFlexGridSizer6->Add(lot_text, g_flagsH);
    lot_text->SetFont(this->GetFont().Bold());

    m_share_lot = new mmTextCtrl(stock_details_panel, ID_STOCKTRANS_SHARE_LOT
        , "", wxDefaultPosition, wxSize(150, -1), 0);
    itemFlexGridSizer6->Add(m_share_lot, g_flagsH);
    m_share_lot->SetToolTip(_("Enter the LOT that this parcel os shares belong to"));

    // Commission
    m_commission = new mmTextCtrl(stock_details_panel, ID_STOCKTRANS_SHARE_COMMISSION, "0"
        , wxDefaultPosition, wxSize(150, -1), wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    itemFlexGridSizer6->Add(new wxStaticText(stock_details_panel, wxID_STATIC, _("Commission")), g_flagsH);
    itemFlexGridSizer6->Add(m_commission, g_flagsH);
    m_commission->SetToolTip(_("Enter any commission paid"));
    m_commission->Connect(ID_STOCKTRANS_SHARE_COMMISSION, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(ShareTransactionDialog::OnTextEntered), nullptr, this);

    //Notes
    itemFlexGridSizer6->Add(new wxStaticText(stock_details_panel, wxID_STATIC, _("Notes")), g_flagsH);

    //Attachments
    m_attachments = new wxBitmapButton(stock_details_panel, wxID_FILE, mmBitmap(png::CLIP));
    m_attachments->SetToolTip(_("Organize attachments of this stock"));
    //TODO m_attachments not used here
    m_attachments->Hide();

    wxBitmapButton* web_button = new wxBitmapButton(stock_details_panel, wxID_INDEX, mmBitmap(png::WEB));
    web_button->SetToolTip(_("Display the web page for the specified Stock symbol"));

    wxBoxSizer* icon_sizer = new wxBoxSizer(wxHORIZONTAL);
    icon_sizer->Add(m_attachments, g_flagsH);
    icon_sizer->Add(web_button, g_flagsH);
    itemFlexGridSizer6->Add(icon_sizer, wxSizerFlags(g_flagsH).Align(wxALIGN_RIGHT));

    m_notes = new mmTextCtrl(this, wxID_STATIC, "", wxDefaultPosition, wxSize(200, 162), wxTE_MULTILINE);
    details_frame_sizer->Add(m_notes, g_flagsExpand);
    details_frame_sizer->AddSpacer(1);
    m_notes->SetToolTip(_("Enter notes associated with this investment"));

    /********************************************************************
    Transaction Panel
    *********************************************************************/
    wxStaticBox* transaction_frame = new wxStaticBox(this, wxID_ANY, _("Edit Transaction Details"));
    wxStaticBoxSizer* transaction_frame_sizer = new wxStaticBoxSizer(transaction_frame, wxVERTICAL);
    right_sizer->Add(transaction_frame_sizer, g_flagsV);

    m_transaction_panel = new UserTransactionPanel(this, m_checking_entry, wxID_STATIC);
    transaction_frame_sizer->Add(m_transaction_panel, g_flagsV);
    if (m_translink_entry && m_checking_entry)
    {
        m_transaction_panel->SetCheckingType(Model_Translink::type_checking(m_checking_entry->TOACCOUNTID));
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

    main_sizer->Add(button_panel, wxSizerFlags(g_flagsH).Center());
    button_panel->SetSizer(button_panel_sizer);
    button_panel_sizer->Add(ok_button, g_flagsH);
    button_panel_sizer->Add(close_button, g_flagsH);
    //cancel_button->SetFocus();
}

void ShareTransactionDialog::OnQuit(wxCloseEvent& WXUNUSED(event))
{
    const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::STOCK);
    if (!this->m_stock)
        mmAttachmentManage::DeleteAllAttachments(RefType, 0);
    EndModal(wxID_CANCEL);
}


void ShareTransactionDialog::OnCancel(wxCommandEvent& WXUNUSED(event))
{
    const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::STOCK);
    if (m_stock_id <= 0)
        mmAttachmentManage::DeleteAllAttachments(RefType, 0);
    EndModal(wxID_CANCEL);
}

void ShareTransactionDialog::OnAttachments(wxCommandEvent& WXUNUSED(event))
{
    const wxString RefType = Model_Attachment::reftype_desc(Model_Attachment::STOCK);
    int RefId = m_stock_id;

    if (RefId < 0)
        RefId = 0;

    mmAttachmentDialog dlg(this, RefType, RefId);
    dlg.ShowModal();
}

void ShareTransactionDialog::OnStockPriceButton(wxCommandEvent& WXUNUSED(event))
{
    const wxString stockSymbol = m_stock_symbol->GetValue().Trim();

    if (!stockSymbol.IsEmpty())
    {
        const wxString& stockURL = Model_Infotable::instance().GetStringInfo("STOCKURL", mmex::weblink::DefStockUrl);
        const wxString& httpString = wxString::Format(stockURL, stockSymbol);
        wxLaunchDefaultBrowser(httpString);
    }
}

void ShareTransactionDialog::OnOk(wxCommandEvent& WXUNUSED(event))
{
    Model_Account::Data* account = Model_Account::instance().get(m_account_id);
    if (!account)
    {
        mmErrorDialogs::MessageInvalid(this, _("Held At"));
        return;
    }
    
    const wxString& stock_symbol = m_stock_symbol->GetValue();
    if (stock_symbol.empty())
    {
        mmErrorDialogs::MessageInvalid(this, _("Symbol"));
        return;
    }

    //Model_Currency::Data* currency = m_transaction_panel->GetCurrencyData();

    double num_shares = 0;
    if (!m_share_num->checkValue(num_shares))
    {
        return;
    }

    double share_price = 0;
    if (!m_share_price->checkValue(share_price))
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
        m_stock->PURCHASEDATE = m_transaction_panel->TransactionDate().FormatISODate();
        m_stock->STOCKNAME = m_stock_name->GetValue();
        m_stock->SYMBOL = stock_symbol;
        m_stock->NUMSHARES = num_shares;
        m_stock->PURCHASEPRICE = share_price;
        m_stock->NOTES = m_notes->GetValue();
        m_stock->CURRENTPRICE = share_price;
        m_stock->VALUE = (num_shares * share_price) + commission;
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
        // addition or removal shares
        if ((num_shares > 0) && (m_transaction_panel->TransactionType() == Model_Checking::DEPOSIT))
        {
            // we need to subtract the number of shares for a sale
            num_shares = num_shares * -1;
        }

        int checking_id = m_transaction_panel->SaveChecking();

        /*
        // The PURCHASEDATE, field in STOCK table becomes obsolete.
        // NUMSHARES, PURCHASEPRICE and COMMISSION fields in the Stocks table are used as
        // a summary and allows Stock history to work in its current form.
        // The Shares table now maintains share_num, share_price, and commission on the
        // date of purchase, together with a record in the chechingaccount table.
        */
        if (!m_translink_entry)
        {
             Model_Translink::SetStockTranslink(m_stock->STOCKID
                , checking_id, m_transaction_panel->CheckingType());
        }
        Model_Shareinfo::ShareEntry(checking_id, num_shares, share_price, commission, m_share_lot->GetValue());

        Model_Translink::UpdateStockValue(m_stock);
    }
    else
    {
        mmErrorDialogs::MessageWarning(this, _("Invalid Transaction"), m_dialog_heading);
        return;
    }

    EndModal(wxID_OK);
}

void ShareTransactionDialog::OnTextEntered(wxCommandEvent& event)
{
    //Model_Currency::Data* currency = m_transaction_panel->GetCurrencyData();

    double share_num = 0;
    if (!m_share_num->GetValue().empty())
    {
        m_share_num->GetDouble(share_num);
    }

    double share_price = 0;
    if (!m_share_price->GetValue().empty())
    {
        m_share_price->GetDouble(share_price);
    }

    double share_commission = 0;
    if (!m_commission->GetValue().empty())
    {
        m_commission->GetDouble(share_commission);
    }

    if (share_num > 0)
    {
        m_transaction_panel->SetTransactionValue((share_num * share_price) + share_commission);
    }
}
