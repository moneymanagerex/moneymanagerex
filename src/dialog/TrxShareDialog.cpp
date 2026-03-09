/*******************************************************
 Copyright (C) 2016 Stefano Giorgio
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

#include "model/AccountModel.h"
#include "model/CategoryModel.h"
#include "model/StockHistoryModel.h"

#include "AttachmentDialog.h"
#include "SplitDialog.h"
#include "TrxShareDialog.h"
#include "TrxLinkDialog.h"

IMPLEMENT_DYNAMIC_CLASS(TrxShareDialog, wxDialog)

wxBEGIN_EVENT_TABLE(TrxShareDialog, wxDialog)
    EVT_BUTTON(wxID_OK, TrxShareDialog::OnOk)
    EVT_BUTTON(wxID_CANCEL, TrxShareDialog::OnCancel)
    EVT_BUTTON(wxID_INDEX, TrxShareDialog::OnStockPriceButton)
    EVT_BUTTON(mmID_COMM_SPLIT, TrxShareDialog::OnDeductibleSplit)
    EVT_CLOSE(TrxShareDialog::OnQuit)
wxEND_EVENT_TABLE()

double TrxShareDialog::GetAmount(double shares, double price, double commission)
{
    if (m_transaction_panel->TransactionType() == TrxModel::TYPE_ID_DEPOSIT)
        return (shares * price - commission);
    else
        return (shares * price + commission);
}

TrxShareDialog::TrxShareDialog()
{
}

TrxShareDialog::TrxShareDialog(wxWindow* parent, StockData* stock_n) :
    m_stock_n(stock_n),
    m_dialog_heading(_t("Add Share Transaction"))
{
    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY, m_dialog_heading, wxDefaultPosition, wxSize(400, 300), style);
}

TrxShareDialog::TrxShareDialog(
    wxWindow* parent,
    const TrxLinkData* tl_n,
    TrxData* trx_n
) :
    m_dialog_heading(_t("Edit Share Transaction")),
    m_trx_n(trx_n),
    m_tl_n(tl_n)
{
    if (m_tl_n) {
        m_stock_n = StockModel::instance().unsafe_get_id_data_n(
            m_tl_n->m_ref_id
        );
        if (m_tl_n->m_ref_type == StockModel::s_ref_type) {
            m_ts_n = TrxShareModel::instance().unsafe_get_trxId_data_n(
                m_tl_n->m_trx_id
            );
            if (m_ts_n->m_lot.IsEmpty())
                // FIXME: m_ts_n is changed but not saved
                m_ts_n->m_lot = m_stock_n->m_id.ToString();

            for (const auto& tp_d: TrxSplitModel::instance().find(
                TrxSplitCol::TRANSID(m_ts_n->m_id)
            )) {
                wxArrayInt64 tag_id_a;
                for (const auto& gl_d : TagLinkModel::instance().find(
                    TagLinkCol::REFTYPE(TrxSplitModel::s_ref_type.name_n()),
                    TagLinkCol::REFID(tp_d.m_id)
                )) {
                    tag_id_a.push_back(gl_d.m_tag_id);
                }
                m_local_deductible_splits.push_back(
                    {tp_d.m_category_id, tp_d.m_amount, tag_id_a, tp_d.m_notes}
                );
            }
        }
    }

    if (m_trx_n) {
        for (const auto& tp_d: TrxSplitModel::instance().find(
            TrxSplitCol::TRANSID(m_trx_n->m_id)
        )) {
            wxArrayInt64 tag_id_a;
            for (const auto& gl_d : TagLinkModel::instance().find(
                TagLinkCol::REFTYPE(TrxSplitModel::s_ref_type.name_n()),
                TagLinkCol::REFID(tp_d.m_id)
            )) {
                tag_id_a.push_back(gl_d.m_tag_id);
            }
            m_local_non_deductible_splits.push_back(
                {tp_d.m_category_id, tp_d.m_amount, tag_id_a, tp_d.m_notes}
            );
        }
    }

    this->SetFont(parent->GetFont());
    Create(parent, wxID_ANY, m_dialog_heading, wxDefaultPosition, wxSize(400, 300));
    this->SetMinSize(wxSize(400, 300));
}

bool TrxShareDialog::Create(
    wxWindow* parent, wxWindowID id,
    const wxString& caption,
    const wxPoint& pos, const wxSize& size, long style
) {
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style);

    CreateControls();
    mmThemeAutoColour(this);
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    SetIcon(mmex::getProgramIcon());

    DataToControls();

    Centre();
    return true;
}

void TrxShareDialog::DataToControls()
{
    if (!m_stock_n) return;

    m_stock_id = m_stock_n->m_id;
    m_stock_name_ctrl->SetValue(m_stock_n->m_name);
    m_transaction_panel->SetTransactionAccount(m_stock_n->m_name);
    m_stock_symbol_ctrl->SetValue(m_stock_n->m_symbol);
    m_notes_ctrl->SetValue(m_stock_n->m_notes);

    m_stock_name_ctrl->Enable(false);
    m_stock_symbol_ctrl->Enable(false);
    m_share_lot_ctrl->Enable(false);
    m_notes_ctrl->Enable(false);

    TrxLinkModel::DataA tl_a = TrxLinkModel::instance().find_ref_data_a(
        StockModel::s_ref_type, m_stock_n->m_id
    );

    if (tl_a.empty()) {
        // Set up the transaction as the first entry.
        int precision = m_stock_n->m_num_shares == floor(m_stock_n->m_num_shares)
            ? 0
            : PrefModel::instance().getSharePrecision();
        m_share_num_ctrl->SetValue(m_stock_n->m_num_shares, precision);
        m_share_price_ctrl->SetValue(
            m_stock_n->m_purchase_price,
            PrefModel::instance().getSharePrecision()
        );
        m_share_commission_ctrl->SetValue(
            m_stock_n->m_commission,
            PrefModel::instance().getSharePrecision()
        );
        m_share_lot_ctrl->SetValue(m_stock_n->m_id.ToString());
        m_transaction_panel->TransactionDate(m_stock_n->m_purchase_date.getDateTime());
        m_transaction_panel->SetTransactionValue(
            GetAmount(m_stock_n->m_num_shares, m_stock_n->m_purchase_price, m_stock_n->m_commission),
            true
        );
    }
    else {
        if (m_ts_n) {
            int precision = m_ts_n->m_number == floor(m_ts_n->m_number)
                ? 0
                : PrefModel::instance().getSharePrecision();
            m_share_num_ctrl->SetValue(std::abs(m_ts_n->m_number), precision);
            m_share_price_ctrl->SetValue(
                m_ts_n->m_price,
                PrefModel::instance().getSharePrecision()
            );
            m_share_commission_ctrl->SetValue(
                m_ts_n->m_commission,
                PrefModel::instance().getSharePrecision()
            );
            m_share_lot_ctrl->SetValue(m_ts_n->m_lot);

            if (m_tl_n) {
                const TrxData* trx_n = TrxModel::instance().get_id_data_n(m_tl_n->m_trx_id);
                if (trx_n) {
                    m_transaction_panel->TransactionDate(TrxModel::getTransDateTime(*trx_n));
                    m_transaction_panel->SetTransactionValue(
                        GetAmount(std::abs(m_ts_n->m_number), m_ts_n->m_price, m_ts_n->m_commission),
                        true
                    );
                    m_transaction_panel->SetTransactionAccount(AccountModel::instance().get_id_name(trx_n->m_account_id));
                    m_transaction_panel->SetTransactionStatus(TrxModel::status_id(*trx_n));
                    m_transaction_panel->SetTransactionPayee(trx_n->m_payee_id_n);
                    m_transaction_panel->SetTransactionCategory(trx_n->m_category_id_n);
                    if (!trx_n->DELETEDTIME.IsEmpty()) {
                        m_share_num_ctrl->Enable(false);
                        m_share_price_ctrl->Enable(false);
                        m_share_commission_ctrl->Enable(false);
                        web_button->Enable(false);
                    }
                }
            }
        }
        else {
            m_share_num_ctrl->SetValue(0, 0);
            m_share_price_ctrl->SetValue(0, PrefModel::instance().getSharePrecision());
            m_share_lot_ctrl->SetValue(m_stock_n->m_id.ToString());
            m_transaction_panel->SetTransactionValue(0, true);
        }
    }

    bool has_split = !(m_local_deductible_splits.size() <= 1);
    if (has_split) {
        m_share_commission_ctrl->Enable(!has_split);
        m_share_commission_ctrl->SetValue(TrxSplitModel::get_total(m_local_deductible_splits), PrefModel::instance().getSharePrecision());
        mmToolTip(m_deductible_comm_split, TrxSplitModel::get_tooltip(m_local_deductible_splits, nullptr /* currency */));
    }
}

void TrxShareDialog::CreateControls()
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

    wxPanel* stock_details_panel = new wxPanel(details_frame, wxID_STATIC);
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
    m_share_num_ctrl->SetAltPrecision(PrefModel::instance().getSharePrecision());
    itemFlexGridSizer6->Add(m_share_num_ctrl, g_flagsH);
    mmToolTip(m_share_num_ctrl, _t("Enter number of shares held"));

    m_share_num_ctrl->Connect(ID_STOCKTRANS_SHARE_NUMBER, wxEVT_COMMAND_TEXT_UPDATED
        , wxCommandEventHandler(TrxShareDialog::CalculateAmount), nullptr, this);

    //Share Price
    wxStaticText* pprice = new wxStaticText(stock_details_panel, wxID_STATIC, _t("Share Price"));
    pprice->SetFont(this->GetFont().Bold());
    m_share_price_ctrl = new mmTextCtrl(stock_details_panel, ID_STOCKTRANS_SHARE_PRICE, ""
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    m_share_price_ctrl->SetAltPrecision(PrefModel::instance().getSharePrecision());
    m_share_price_ctrl->SetMinSize(wxSize(150, -1));
    itemFlexGridSizer6->Add(pprice, g_flagsH);
    itemFlexGridSizer6->Add(m_share_price_ctrl, g_flagsH);
    mmToolTip(m_share_price_ctrl, _t("Enter the current value for a single share unit"));

    m_share_price_ctrl->Connect(ID_STOCKTRANS_SHARE_PRICE, wxEVT_COMMAND_TEXT_UPDATED
        , wxCommandEventHandler(TrxShareDialog::CalculateAmount), nullptr, this);

    // Deductible Commission
    itemFlexGridSizer6->Add(new wxStaticText(stock_details_panel, wxID_STATIC, _t("Deductible Comm.")), g_flagsH);

    wxBoxSizer* commission_sizer = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer6->Add(commission_sizer, wxSizerFlags(g_flagsExpand).Border(0));

    m_share_commission_ctrl = new mmTextCtrl(stock_details_panel, ID_STOCKTRANS_SHARE_COMMISSION, "0"
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    m_share_commission_ctrl->SetAltPrecision(PrefModel::instance().getSharePrecision());
    m_share_commission_ctrl->SetMinSize(wxSize(110, -1));
    commission_sizer->Add(m_share_commission_ctrl, g_flagsH);
    mmToolTip(m_share_commission_ctrl, _t("Enter any commission paid"));

    m_deductible_comm_split = new wxBitmapButton(stock_details_panel, mmID_COMM_SPLIT, mmBitmapBundle(png::NEW_TRX, mmBitmapButtonSize));
    commission_sizer->Add(m_deductible_comm_split, g_flagsH);
    mmToolTip(m_deductible_comm_split, _t("Use Deductible Comm. split Categories"));

    m_share_commission_ctrl->Connect(ID_STOCKTRANS_SHARE_COMMISSION, wxEVT_COMMAND_TEXT_UPDATED
        , wxCommandEventHandler(TrxShareDialog::CalculateAmount), nullptr, this);

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

    m_notes_ctrl = new wxTextCtrl(details_frame, wxID_STATIC, "", wxDefaultPosition, wxSize(200, 162), wxTE_MULTILINE);
    details_frame_sizer->Add(m_notes_ctrl, g_flagsExpand);
    details_frame_sizer->AddSpacer(1);
    mmToolTip(m_notes_ctrl, _t("Enter notes associated with this investment"));

    /********************************************************************
    Transaction Panel
    *********************************************************************/

    wxStaticBox* transaction_frame = new wxStaticBox(this, wxID_ANY, m_trx_n ? _t("Edit Transaction Details") : _t("Add Transaction Details"));
    wxStaticBoxSizer* transaction_frame_sizer = new wxStaticBoxSizer(transaction_frame, wxVERTICAL);
    right_sizer->Add(transaction_frame_sizer, g_flagsV);

    m_transaction_panel = new TrxLinkDialog(transaction_frame, m_trx_n, false, wxID_STATIC);
    m_transaction_panel->Bind(wxEVT_CHOICE, &TrxShareDialog::CalculateAmount, this, wxID_VIEW_DETAILS);
    transaction_frame_sizer->Add(m_transaction_panel, g_flagsV);
    if (m_tl_n && m_trx_n) {
        m_transaction_panel->CheckingType(
            TrxLinkModel::type_checking(m_trx_n->m_to_account_id_n)
        );
    }
    else {
        wxString acc_held = AccountModel::instance().get_id_name(m_stock_n->m_account_id_n);
        m_transaction_panel->SetTransactionNumber(m_stock_n->m_name + "_" + m_stock_n->m_symbol);
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

void TrxShareDialog::OnQuit(wxCloseEvent& WXUNUSED(event))
{
    // FIXME
    if (!this->m_stock_n)
        mmAttachmentManage::DeleteAllAttachments(StockModel::s_ref_type, 0);
    EndModal(wxID_CANCEL);
}

void TrxShareDialog::OnCancel(wxCommandEvent& WXUNUSED(event))
{
    // FIXME
    if (m_stock_id <= 0)
        mmAttachmentManage::DeleteAllAttachments(StockModel::s_ref_type, 0);
    EndModal(wxID_CANCEL);
}

void TrxShareDialog::OnStockPriceButton(wxCommandEvent& WXUNUSED(event))
{
    const wxString stockSymbol = m_stock_symbol_ctrl->GetValue().Trim();

    if (!stockSymbol.IsEmpty()) {
        const wxString& stockURL = InfoModel::instance().getString("STOCKURL", mmex::weblink::DefStockUrl);
        const wxString& httpString = wxString::Format(stockURL, stockSymbol);
        wxLaunchDefaultBrowser(httpString);
    }
}

void TrxShareDialog::OnOk(wxCommandEvent& WXUNUSED(event))
{
    double num_shares = 0;
    if (!m_share_num_ctrl->checkValue(num_shares))
        return;

    double share_price = 0;
    if (!m_share_price_ctrl->checkValue(share_price))
        return;

    double commission = 0;
    m_share_commission_ctrl->GetDouble(commission);

    // allow for loyalty shares. These are "Free"
    bool loyalty_shares = (share_price == 0) && (num_shares > 0);

    if (m_transaction_panel->ValidCheckingAccountEntry()) {
        // addition or removal shares
        if (num_shares > 0 &&
            m_transaction_panel->TransactionType() == TrxModel::TYPE_ID_DEPOSIT
        ) {
            // we need to subtract the number of shares for a sale
            num_shares = num_shares * -1;
        }

        int64 trx_id = m_transaction_panel->SaveChecking();
        if (trx_id < 0)
            return;

        /*
        // The PURCHASEDATE field in STOCK table holds the earliest purchase date of the stock.
        // NUMSHARES, PURCHASEPRICE and COMMISSION fields in the Stocks table are used as
        // a summary and allows Stock history to work in its current form.
        // The Shares table now maintains share_num, share_price, and commission on the
        // date of purchase, together with a record in the checking account table.
        */
        if (!m_tl_n) {
             TrxLinkModel::instance().SetStockTranslink(
                trx_id, m_stock_n->m_id,
                m_transaction_panel->CheckingType()
            );
        }
        TrxShareModel::instance().update_trxID(trx_id,
            num_shares, share_price, commission, m_share_lot_ctrl->GetValue(),
            m_local_deductible_splits
        );

        StockModel::instance().update_data_position(m_stock_n);
        if (!loyalty_shares) {
            StockHistoryModel::instance().addUpdate(
                m_stock_n->m_symbol,
                m_transaction_panel->TransactionDate(),
                share_price,
                StockHistoryModel::MANUAL
            );
        }
    }
    else {
        mmErrorDialogs::MessageWarning(this, _t("Invalid Transaction"), m_dialog_heading);
        return;
    }

    EndModal(wxID_OK);
}

void TrxShareDialog::CalculateAmount(wxCommandEvent& WXUNUSED(event))
{
    double share_num = 0;
    if (!m_share_num_ctrl->GetValue().empty()) {
        m_share_num_ctrl->GetDouble(share_num);
    }

    double share_price = 0;
    if (!m_share_price_ctrl->GetValue().empty()) {
        m_share_price_ctrl->GetDouble(share_price);
    }

    double share_commission = 0;
    if (m_share_commission_ctrl && !m_share_commission_ctrl->GetValue().empty()) {
        m_share_commission_ctrl->GetDouble(share_commission);
    }

    if (share_num > 0) {
        m_transaction_panel->SetTransactionValue(GetAmount(share_num, share_price, share_commission));
    }
}

void TrxShareDialog::OnDeductibleSplit(wxCommandEvent&)
{
    if (m_local_deductible_splits.empty()) {
        double commission = 0;
        m_share_commission_ctrl->GetDouble(commission);

        const CategoryData* category_n = CategoryModel::instance().get_key_data_n(
            _("Investment"), int64(-1L)
        );
        if (!category_n) {
            CategoryData new_category_d = CategoryData();
            new_category_d.m_name = _("Investment");
            CategoryModel::instance().add_data_n(new_category_d);
            category_n = CategoryModel::instance().get_id_data_n(new_category_d.id());
        }
        m_local_deductible_splits.push_back({category_n->m_id, commission, wxArrayInt64(), ""});
    }

    SplitDialog dlg(this, m_local_deductible_splits, m_stock_n->m_account_id_n);

    if (dlg.ShowModal() == wxID_OK) {
        m_local_deductible_splits = dlg.mmGetResult();

        if (m_local_deductible_splits.size() == 1) {
            // TODO other informations
            m_share_commission_ctrl->SetValue(
                m_local_deductible_splits[0].SPLITTRANSAMOUNT,
                PrefModel::instance().getSharePrecision()
            );

            m_local_deductible_splits.clear();
        }

        if (m_local_deductible_splits.empty()) {
            m_share_commission_ctrl->Enable(true);
            mmToolTip(m_deductible_comm_split, _t("Use Deductible Comm. split Categories"));
        }
        else {
            m_share_commission_ctrl->SetValue(
                TrxSplitModel::get_total(m_local_deductible_splits),
                PrefModel::instance().getSharePrecision()
            );
            m_share_commission_ctrl->Enable(false);
            mmToolTip(m_deductible_comm_split, TrxSplitModel::get_tooltip(m_local_deductible_splits, nullptr /* currency */));
        }
    }
}
