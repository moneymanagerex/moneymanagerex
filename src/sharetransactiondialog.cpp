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
#include "mmTextCtrl.h"
#include "paths.h"
#include "util.h"
#include "validators.h"

#include "model/Model_Account.h"
#include "model/Model_Category.h"
#include "model/Model_StockHistory.h"
#include "usertransactionpanel.h"
#include "splittransactionsdialog.h"

#include <wx/numdlg.h>
#include <wx/textdlg.h>
#include <wx/valnum.h>

IMPLEMENT_DYNAMIC_CLASS(ShareTransactionDialog, wxDialog)

wxBEGIN_EVENT_TABLE(ShareTransactionDialog, wxDialog)
    EVT_BUTTON(wxID_OK, ShareTransactionDialog::OnOk)
    EVT_BUTTON(wxID_CANCEL, ShareTransactionDialog::OnCancel)
    EVT_BUTTON(wxID_INDEX, ShareTransactionDialog::OnStockPriceButton)
    EVT_BUTTON(mmID_COMM_SPLIT, ShareTransactionDialog::OnDeductibleSplit)
    EVT_CLOSE(ShareTransactionDialog::OnQuit)
wxEND_EVENT_TABLE()

double ShareTransactionDialog::GetAmount(double shares, double price, double commission)
{
    if (m_transaction_panel->TransactionType() == Model_Checking::TYPE_ID_DEPOSIT)
        return (shares * price - commission);
    else
        return (shares * price + commission);
}

ShareTransactionDialog::ShareTransactionDialog()
{
}

ShareTransactionDialog::ShareTransactionDialog(wxWindow* parent, Model_Stock::Data* stock)
    : m_stock(stock)
    , m_dialog_heading(_t("Add Share Transaction"))
{
    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY, m_dialog_heading, wxDefaultPosition, wxSize(400, 300), style);
}

ShareTransactionDialog::ShareTransactionDialog(wxWindow* parent, Model_Translink::Data* translink_entry, Model_Checking::Data* checking_entry)
    : m_dialog_heading(_t("Edit Share Transaction"))    
    , m_checking_entry(checking_entry)
    , m_translink_entry(translink_entry)
{
    if (m_translink_entry)
    {
        m_stock = Model_Stock::instance().get(m_translink_entry->LINKRECORDID);
        if (m_translink_entry->LINKTYPE == Model_Stock::refTypeName)
        {
            m_share_entry = Model_Shareinfo::ShareEntry(m_translink_entry->CHECKINGACCOUNTID);
            if (m_share_entry->SHARELOT.IsEmpty()) m_share_entry->SHARELOT = m_stock->STOCKID.ToString();

            for (const auto& split: Model_Splittransaction::instance().find(Model_Splittransaction::TRANSID(m_share_entry->SHAREINFOID))) 
            {
                wxArrayInt64 tags;
                for (const auto& tag : Model_Taglink::instance().find(Model_Taglink::REFTYPE(Model_Splittransaction::refTypeName), Model_Taglink::REFID(split.SPLITTRANSID)))
                    tags.push_back(tag.TAGID);
                m_local_deductible_splits.push_back({split.CATEGID, split.SPLITTRANSAMOUNT, tags, split.NOTES});
            }
        }
    }

    if (m_checking_entry) 
    {
        for (const auto& split: Model_Splittransaction::instance().find(Model_Splittransaction::TRANSID(m_checking_entry->TRANSID))) 
        {
            wxArrayInt64 tags;
            for (const auto& tag : Model_Taglink::instance().find(Model_Taglink::REFTYPE(Model_Splittransaction::refTypeName), Model_Taglink::REFID(split.SPLITTRANSID)))
                tags.push_back(tag.TAGID);
            m_local_non_deductible_splits.push_back({split.CATEGID, split.SPLITTRANSAMOUNT, tags, split.NOTES});
        }
    }

    this->SetFont(parent->GetFont());
    Create(parent, wxID_ANY, m_dialog_heading, wxDefaultPosition, wxSize(400, 300));
    this->SetMinSize(wxSize(400, 300));
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
    return true;
}

void ShareTransactionDialog::DataToControls()
{
    if (!m_stock) return;

    m_stock_id = m_stock->STOCKID;
    m_stock_name_ctrl->SetValue(m_stock->STOCKNAME);
    m_transaction_panel->SetTransactionAccount(m_stock->STOCKNAME);
    m_stock_symbol_ctrl->SetValue(m_stock->SYMBOL);
    m_notes_ctrl->SetValue(m_stock->NOTES);

    m_stock_name_ctrl->Enable(false);
    m_stock_symbol_ctrl->Enable(false);
    m_share_lot_ctrl->Enable(false);
    m_notes_ctrl->Enable(false);

    Model_Translink::Data_Set translink_list = Model_Translink::TranslinkList<Model_Stock>(m_stock->STOCKID);

    if (translink_list.empty())
    {   // Set up the transaction as the first entry.
        int precision = m_stock->NUMSHARES == floor(m_stock->NUMSHARES) ? 0 : Option::instance().getSharePrecision();
        m_share_num_ctrl->SetValue(m_stock->NUMSHARES, precision);
        m_share_price_ctrl->SetValue(m_stock->PURCHASEPRICE, Option::instance().getSharePrecision());
        m_share_commission_ctrl->SetValue(m_stock->COMMISSION, Option::instance().getSharePrecision());
        m_share_lot_ctrl->SetValue(m_stock->STOCKID.ToString());
        m_transaction_panel->TransactionDate(Model_Stock::PURCHASEDATE(m_stock));
        m_transaction_panel->SetTransactionValue(GetAmount(m_stock->NUMSHARES, m_stock->PURCHASEPRICE
                , m_stock->COMMISSION), true);
    }
    else
    {
        if (m_share_entry)
        {
            int precision = m_share_entry->SHARENUMBER == floor(m_share_entry->SHARENUMBER) ? 0 : Option::instance().getSharePrecision();
            m_share_num_ctrl->SetValue(std::abs(m_share_entry->SHARENUMBER), precision);
            m_share_price_ctrl->SetValue(m_share_entry->SHAREPRICE, Option::instance().getSharePrecision());
            m_share_commission_ctrl->SetValue(m_share_entry->SHARECOMMISSION, Option::instance().getSharePrecision());
            m_share_lot_ctrl->SetValue(m_share_entry->SHARELOT);

            if (m_translink_entry)
            {
                Model_Checking::Data* checking_entry = Model_Checking::instance().get(m_translink_entry->CHECKINGACCOUNTID);
                if (checking_entry)
                {
                    m_transaction_panel->TransactionDate(Model_Checking::TRANSDATE(checking_entry));
                    m_transaction_panel->SetTransactionValue(GetAmount(std::abs(m_share_entry->SHARENUMBER)
                        , m_share_entry->SHAREPRICE, m_share_entry->SHARECOMMISSION), true);
                    m_transaction_panel->SetTransactionAccount(Model_Account::get_account_name(checking_entry->ACCOUNTID));
                    m_transaction_panel->SetTransactionStatus(Model_Checking::status_id(checking_entry));
                    m_transaction_panel->SetTransactionPayee(checking_entry->PAYEEID);
                    m_transaction_panel->SetTransactionCategory(checking_entry->CATEGID);
                    if (!checking_entry->DELETEDTIME.IsEmpty()) 
                    {
                        m_share_num_ctrl->Enable(false);
                        m_share_price_ctrl->Enable(false);
                        m_share_commission_ctrl->Enable(false);
                        web_button->Enable(false);
                    }
                }
            }
        }
        else
        {
            m_share_num_ctrl->SetValue(0, 0);
            m_share_price_ctrl->SetValue(0, Option::instance().getSharePrecision());
            m_share_lot_ctrl->SetValue(m_stock->STOCKID.ToString());
            m_transaction_panel->SetTransactionValue(0, true);
        }
    }

    bool has_split = !(m_local_deductible_splits.size() <= 1);
    if (has_split)
    {
        m_share_commission_ctrl->Enable(!has_split);
        m_share_commission_ctrl->SetValue(Model_Splittransaction::get_total(m_local_deductible_splits), Option::instance().getSharePrecision());
        mmToolTip(m_deductible_comm_split, Model_Splittransaction::get_tooltip(m_local_deductible_splits, nullptr /* currency */));
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
    wxStaticBox* details_frame = new wxStaticBox(this, wxID_ANY, _t("Stock Investment Details"));
    wxStaticBoxSizer* details_frame_sizer = new wxStaticBoxSizer(details_frame, wxVERTICAL);
    left_sizer->Add(details_frame_sizer, g_flagsExpand);

    wxPanel* stock_details_panel = new wxPanel(this, wxID_STATIC);
    details_frame_sizer->Add(stock_details_panel, g_flagsV);

    wxFlexGridSizer* itemFlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
    stock_details_panel->SetSizer(itemFlexGridSizer6);

    itemFlexGridSizer6->Add(new wxStaticText(stock_details_panel, wxID_STATIC, _t("Company Name")), g_flagsH);

    m_stock_name_ctrl = new wxTextCtrl(stock_details_panel, ID_STOCKTRANS_SHARE_NAME, "");
    itemFlexGridSizer6->Add(m_stock_name_ctrl, g_flagsExpand);
    mmToolTip(m_stock_name_ctrl, _t("Enter the stock company name"));

    //Symbol
    wxStaticText* symbol = new wxStaticText(stock_details_panel, wxID_STATIC, _t("Stock Symbol"));
    itemFlexGridSizer6->Add(symbol, g_flagsH);
    symbol->SetFont(this->GetFont().Bold());

    m_stock_symbol_ctrl = new wxTextCtrl(stock_details_panel, ID_STOCKTRANS_SHARE_SYMBOL
        , "", wxDefaultPosition, wxSize(150, -1), 0);
    itemFlexGridSizer6->Add(m_stock_symbol_ctrl, g_flagsH);
    mmToolTip(m_stock_symbol_ctrl, _t("Enter the stock symbol. (Optional) Include exchange. eg: IBM.BE"));

    //Share Lot
    wxStaticText* lot_text = new wxStaticText(stock_details_panel, wxID_STATIC, _t("Share Lot"));
    itemFlexGridSizer6->Add(lot_text, g_flagsH);
    lot_text->SetFont(this->GetFont().Bold());

    m_share_lot_ctrl = new wxTextCtrl(stock_details_panel, ID_STOCKTRANS_SHARE_LOT
        , "", wxDefaultPosition, wxSize(150, -1), 0);
    itemFlexGridSizer6->Add(m_share_lot_ctrl, g_flagsH);
    mmToolTip(m_share_lot_ctrl, _t("Enter the LOT that this parcel os shares belong to"));

    //Share Unit Number 
    wxStaticText* number = new wxStaticText(stock_details_panel, wxID_STATIC, _t("Share Number"));
    itemFlexGridSizer6->Add(number, g_flagsH);
    number->SetFont(this->GetFont().Bold());
    m_share_num_ctrl = new mmTextCtrl(stock_details_panel, ID_STOCKTRANS_SHARE_NUMBER, ""
        , wxDefaultPosition, wxSize(150, -1), wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    m_share_num_ctrl->SetAltPrecision(Option::instance().getSharePrecision());
    itemFlexGridSizer6->Add(m_share_num_ctrl, g_flagsH);
    mmToolTip(m_share_num_ctrl, _t("Enter number of shares held"));

    m_share_num_ctrl->Connect(ID_STOCKTRANS_SHARE_NUMBER, wxEVT_COMMAND_TEXT_UPDATED
        , wxCommandEventHandler(ShareTransactionDialog::CalculateAmount), nullptr, this);

    //Share Price
    wxStaticText* pprice = new wxStaticText(stock_details_panel, wxID_STATIC, _t("Share Price"));
    pprice->SetFont(this->GetFont().Bold());
    m_share_price_ctrl = new mmTextCtrl(stock_details_panel, ID_STOCKTRANS_SHARE_PRICE, ""
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    m_share_price_ctrl->SetAltPrecision(Option::instance().getSharePrecision());
    m_share_price_ctrl->SetMinSize(wxSize(150, -1));
    itemFlexGridSizer6->Add(pprice, g_flagsH);
    itemFlexGridSizer6->Add(m_share_price_ctrl, g_flagsH);
    mmToolTip(m_share_price_ctrl, _t("Enter the current value for a single share unit"));

    m_share_price_ctrl->Connect(ID_STOCKTRANS_SHARE_PRICE, wxEVT_COMMAND_TEXT_UPDATED
        , wxCommandEventHandler(ShareTransactionDialog::CalculateAmount), nullptr, this);

    // Deductible Commission
    itemFlexGridSizer6->Add(new wxStaticText(stock_details_panel, wxID_STATIC, _t("Deductible Comm.")), g_flagsH);

    wxBoxSizer* commission_sizer = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer6->Add(commission_sizer, wxSizerFlags(g_flagsExpand).Border(0));

    m_share_commission_ctrl = new mmTextCtrl(stock_details_panel, ID_STOCKTRANS_SHARE_COMMISSION, "0"
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    m_share_commission_ctrl->SetAltPrecision(Option::instance().getSharePrecision());
    m_share_commission_ctrl->SetMinSize(wxSize(110, -1));
    commission_sizer->Add(m_share_commission_ctrl, g_flagsH);
    mmToolTip(m_share_commission_ctrl, _t("Enter any commission paid"));

    m_deductible_comm_split = new wxBitmapButton(stock_details_panel, mmID_COMM_SPLIT, mmBitmapBundle(png::NEW_TRX, mmBitmapButtonSize));
    commission_sizer->Add(m_deductible_comm_split, g_flagsH);
    mmToolTip(m_deductible_comm_split, _t("Use Deductible Comm. split Categories"));

    m_share_commission_ctrl->Connect(ID_STOCKTRANS_SHARE_COMMISSION, wxEVT_COMMAND_TEXT_UPDATED
        , wxCommandEventHandler(ShareTransactionDialog::CalculateAmount), nullptr, this);

    //Notes
    itemFlexGridSizer6->Add(new wxStaticText(stock_details_panel, wxID_STATIC, _t("Notes")), g_flagsH);

    //Attachments
    m_attachments_btn = new wxBitmapButton(stock_details_panel, wxID_FILE, mmBitmapBundle(png::CLIP, mmBitmapButtonSize));
    mmToolTip(m_attachments_btn, _t("Manage stock attachments"));
    //TODO m_attachments not used here
    m_attachments_btn->Hide();

    web_button = new wxBitmapButton(stock_details_panel, wxID_INDEX, mmBitmapBundle(png::WEB, mmBitmapButtonSize));
    mmToolTip(web_button, _t("Display the web page for the specified Stock symbol"));

    wxBoxSizer* icon_sizer = new wxBoxSizer(wxHORIZONTAL);
    icon_sizer->Add(m_attachments_btn, g_flagsH);
    icon_sizer->Add(web_button, g_flagsH);
    itemFlexGridSizer6->Add(icon_sizer, wxSizerFlags(g_flagsH).Align(wxALIGN_RIGHT));

    m_notes_ctrl = new wxTextCtrl(this, wxID_STATIC, "", wxDefaultPosition, wxSize(200, 162), wxTE_MULTILINE);
    details_frame_sizer->Add(m_notes_ctrl, g_flagsExpand);
    details_frame_sizer->AddSpacer(1);
    mmToolTip(m_notes_ctrl, _t("Enter notes associated with this investment"));

    /********************************************************************
    Transaction Panel
    *********************************************************************/

    wxStaticBox* transaction_frame = new wxStaticBox(this, wxID_ANY, m_checking_entry ? _t("Edit Transaction Details") : _t("Add Transaction Details"));
    wxStaticBoxSizer* transaction_frame_sizer = new wxStaticBoxSizer(transaction_frame, wxVERTICAL);
    right_sizer->Add(transaction_frame_sizer, g_flagsV);

    m_transaction_panel = new UserTransactionPanel(this, m_checking_entry, false, wxID_STATIC);
    m_transaction_panel->Bind(wxEVT_CHOICE, &ShareTransactionDialog::CalculateAmount, this, wxID_VIEW_DETAILS);
    transaction_frame_sizer->Add(m_transaction_panel, g_flagsV);
    if (m_translink_entry && m_checking_entry)
    {
        m_transaction_panel->CheckingType(Model_Translink::type_checking(m_checking_entry->TOACCOUNTID));
    }
    else
    {
        wxString acc_held = Model_Account::get_account_name(m_stock->HELDAT);
        m_transaction_panel->SetTransactionNumber(m_stock->STOCKNAME + "_" + m_stock->SYMBOL);
        m_transaction_panel->SetTransactionAccount(acc_held);
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
    wxButton* ok_button = new wxButton(button_panel, wxID_OK, _t("&OK "));
    wxButton* close_button = new wxButton(button_panel, wxID_CANCEL, _t("&Cancel "));

    main_sizer->Add(button_panel, wxSizerFlags(g_flagsH).Center());
    button_panel->SetSizer(button_panel_sizer);
    button_panel_sizer->Add(ok_button, g_flagsH);
    button_panel_sizer->Add(close_button, g_flagsH);
    //cancel_button->SetFocus();
}

void ShareTransactionDialog::OnQuit(wxCloseEvent& WXUNUSED(event))
{
    const wxString& RefType = Model_Stock::refTypeName;
    if (!this->m_stock)
        mmAttachmentManage::DeleteAllAttachments(RefType, 0);
    EndModal(wxID_CANCEL);
}


void ShareTransactionDialog::OnCancel(wxCommandEvent& WXUNUSED(event))
{
    const wxString& RefType = Model_Stock::refTypeName;
    if (m_stock_id <= 0)
        mmAttachmentManage::DeleteAllAttachments(RefType, 0);
    EndModal(wxID_CANCEL);
}

void ShareTransactionDialog::OnStockPriceButton(wxCommandEvent& WXUNUSED(event))
{
    const wxString stockSymbol = m_stock_symbol_ctrl->GetValue().Trim();

    if (!stockSymbol.IsEmpty())
    {
        const wxString& stockURL = Model_Infotable::instance().getString("STOCKURL", mmex::weblink::DefStockUrl);
        const wxString& httpString = wxString::Format(stockURL, stockSymbol);
        wxLaunchDefaultBrowser(httpString);
    }
}

void ShareTransactionDialog::OnOk(wxCommandEvent& WXUNUSED(event))
{
    double num_shares = 0;
    if (!m_share_num_ctrl->checkValue(num_shares))
    {
        return;
    }

    double share_price = 0;
    if (!m_share_price_ctrl->checkValue(share_price))
    {
        return;
    }

    double commission = 0;
    m_share_commission_ctrl->GetDouble(commission);

    // allow for loyalty shares. These are "Free"
    bool loyalty_shares = (share_price == 0) && (num_shares > 0);

    if (m_transaction_panel->ValidCheckingAccountEntry())
    {
        // addition or removal shares
        if ((num_shares > 0) && (m_transaction_panel->TransactionType() == Model_Checking::TYPE_ID_DEPOSIT))
        {
            // we need to subtract the number of shares for a sale
            num_shares = num_shares * -1;
        }

        int64 checking_id = m_transaction_panel->SaveChecking();
        if (checking_id < 0)
            return;

        /*
        // The PURCHASEDATE field in STOCK table holds the earliest purchase date of the stock.
        // NUMSHARES, PURCHASEPRICE and COMMISSION fields in the Stocks table are used as
        // a summary and allows Stock history to work in its current form.
        // The Shares table now maintains share_num, share_price, and commission on the
        // date of purchase, together with a record in the checking account table.
        */
        if (!m_translink_entry)
        {
             Model_Translink::SetStockTranslink(m_stock->STOCKID
                , checking_id, m_transaction_panel->CheckingType());
        }
        Model_Shareinfo::ShareEntry(checking_id, num_shares, share_price, commission, m_local_deductible_splits,  m_share_lot_ctrl->GetValue());

        Model_Stock::UpdatePosition(m_stock);
        if (!loyalty_shares)
        {
            Model_StockHistory::instance().addUpdate(m_stock->SYMBOL, m_transaction_panel->TransactionDate(), share_price, Model_StockHistory::MANUAL);
        }         
    }
    else
    {
        mmErrorDialogs::MessageWarning(this, _t("Invalid Transaction"), m_dialog_heading);
        return;
    }

    EndModal(wxID_OK);
}

void ShareTransactionDialog::CalculateAmount(wxCommandEvent& WXUNUSED(event))
{
    double share_num = 0;
    if (!m_share_num_ctrl->GetValue().empty())
    {
        m_share_num_ctrl->GetDouble(share_num);
    }

    double share_price = 0;
    if (!m_share_price_ctrl->GetValue().empty())
    {
        m_share_price_ctrl->GetDouble(share_price);
    }

    double share_commission = 0;
    if (m_share_commission_ctrl && !m_share_commission_ctrl->GetValue().empty())
    {
        m_share_commission_ctrl->GetDouble(share_commission);
    }

    if (share_num > 0)
    {
        m_transaction_panel->SetTransactionValue(GetAmount(share_num, share_price, share_commission));
    }
}
void ShareTransactionDialog::OnDeductibleSplit(wxCommandEvent&)
{
    if (m_local_deductible_splits.empty())
    {
        double commission = 0;
        m_share_commission_ctrl->GetDouble(commission);

        Model_Category::Data* category = Model_Category::instance().get(_("Investment"), int64(-1L));
        if (!category)
        {
            category = Model_Category::instance().create();
            category->CATEGNAME = _("Investment");
            category->ACTIVE = 1;
            category->PARENTID = -1;

            Model_Category::instance().save(category);
        }
        m_local_deductible_splits.push_back({category->CATEGID, commission, wxArrayInt64(), ""});
    }

    mmSplitTransactionDialog dlg(this, m_local_deductible_splits, m_stock->HELDAT);

    if (dlg.ShowModal() == wxID_OK)
    {
        m_local_deductible_splits = dlg.mmGetResult();

        if (m_local_deductible_splits.size() == 1) 
        {
            // TODO other informations
            m_share_commission_ctrl->SetValue(m_local_deductible_splits[0].SPLITTRANSAMOUNT, Option::instance().getSharePrecision());

            m_local_deductible_splits.clear();
        }

        if (m_local_deductible_splits.empty()) 
        {
            m_share_commission_ctrl->Enable(true);
            mmToolTip(m_deductible_comm_split, _t("Use Deductible Comm. split Categories"));
        }
        else
        {
            m_share_commission_ctrl->SetValue(Model_Splittransaction::get_total(m_local_deductible_splits), Option::instance().getSharePrecision());
            m_share_commission_ctrl->Enable(false);
            mmToolTip(m_deductible_comm_split, Model_Splittransaction::get_tooltip(m_local_deductible_splits, nullptr /* currency */));
        }
    }
}
