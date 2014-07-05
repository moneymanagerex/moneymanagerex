/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel

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
#include "mmtextctrl.h"
#include "attachmentdialog.h"
#include "constants.h"
#include "paths.h"
#include "util.h"
#include "validators.h"
#include <wx/valnum.h>
#include "model/Model_Infotable.h"
#include "model/Model_Account.h"
#include "model/Model_Attachment.h"
#include "../resources/update_currency.xpm"
#include "../resources/attachment.xpm"

IMPLEMENT_DYNAMIC_CLASS( mmStockDialog, wxDialog )

BEGIN_EVENT_TABLE( mmStockDialog, wxDialog )
    EVT_BUTTON(wxID_OK, mmStockDialog::OnOk)
    EVT_BUTTON(wxID_CANCEL, mmStockDialog::OnCancel)
    EVT_BUTTON(wxID_INDEX, mmStockDialog::OnStockPriceButton)
    EVT_BUTTON(wxID_FILE, mmStockDialog::OnAttachments)
END_EVENT_TABLE()

mmStockDialog::mmStockDialog( )
{
}

mmStockDialog::mmStockDialog(wxWindow* parent
    , Model_Stock::Data* stock
    , int accountID)
    : m_stock(stock)
    , edit_(stock ? true: false)
    , accountID_(accountID)
    , skip_attachments_init_(false)
{
    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX;
    wxString heading = _("New Stock Investment");
    if (edit_) heading = _("Edit Stock Investment");
    Create(parent, wxID_ANY, heading, wxDefaultPosition, wxSize(400, 300), style);
}

bool mmStockDialog::Create( wxWindow* parent, wxWindowID id, const wxString& caption,
                           const wxPoint& pos, const wxSize& size, long style )
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    SetIcon(mmex::getProgramIcon());

    fillControls();
    if (edit_) dataToControls();

    Centre();
    return TRUE;
}

void mmStockDialog::dataToControls()
{
    if (!this->m_stock) return;

    stockID_ = m_stock->STOCKID;

    stockName_->SetValue(m_stock->STOCKNAME);
    stockSymbol_->SetValue(m_stock->SYMBOL);
    notes_->SetValue(m_stock->NOTES);
    dpc_->SetValue(Model_Stock::PURCHASEDATE(m_stock));

    int precision = m_stock->NUMSHARES == floor(m_stock->NUMSHARES) ? 0 : 4;
    numShares_->SetValue(m_stock->NUMSHARES, precision);
    Model_Account::Data* account = Model_Account::instance().get(m_stock->HELDAT);
    valueInvestment_->SetLabel(Model_Account::toCurrency(m_stock->VALUE, account));
    purchasePrice_->SetValue(m_stock->PURCHASEPRICE, account, 4);
    currentPrice_->SetValue(m_stock->CURRENTPRICE, account, 4);
    commission_->SetValue(m_stock->COMMISSION, account, 4);
}

void mmStockDialog::fillControls()
{

}

void mmStockDialog::CreateControls()
{
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    this->SetSizer(itemBoxSizer2);

    wxSizerFlags flags, flagsExpand;
    flags.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).Border(wxALL, 5);
    flagsExpand.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).Border(wxALL, 5).Expand();

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, flags);

    wxStaticBox* itemStaticBoxSizer4Static = new wxStaticBox(this, wxID_ANY, _("Stock Investment Details"));
    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer(itemStaticBoxSizer4Static, wxVERTICAL);
    itemBoxSizer3->Add(itemStaticBoxSizer4, flags);

    wxPanel* itemPanel5 = new wxPanel( this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    itemStaticBoxSizer4->Add(itemPanel5, flags);

    wxFlexGridSizer* itemFlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
    itemPanel5->SetSizer(itemFlexGridSizer6);

    itemFlexGridSizer6->Add(new wxStaticText( itemPanel5, wxID_STATIC, _("Stock Name")), flags);

    stockName_ = new mmTextCtrl( itemPanel5, ID_TEXTCTRL_STOCKNAME, "");
    itemFlexGridSizer6->Add(stockName_, flagsExpand);
    stockName_->SetToolTip(_("Enter the stock company name"));
    if (!edit_)
        stockName_->SetFocus();

    //Date
    itemFlexGridSizer6->Add(new wxStaticText( itemPanel5, wxID_STATIC, _("Date")), flags);

    dpc_ = new wxDatePickerCtrl( itemPanel5, ID_DPC_STOCK_PDATE,
        wxDefaultDateTime, wxDefaultPosition, wxSize(150, -1), wxDP_DROPDOWN | wxDP_SHOWCENTURY);
    itemFlexGridSizer6->Add(dpc_, flags);
    dpc_->SetToolTip(_("Specify the purchase date of the stock investment"));
    //

    //Symbol
    wxStaticText* symbol = new wxStaticText(itemPanel5, wxID_STATIC, _("Symbol"));
    itemFlexGridSizer6->Add(symbol, flags);
    symbol->SetFont(this->GetFont().Bold());

    stockSymbol_ = new mmTextCtrl( itemPanel5, ID_TEXTCTRL_STOCK_SYMBOL,
        "", wxDefaultPosition, wxSize(150, -1), 0 );
    itemFlexGridSizer6->Add(stockSymbol_, flags);
    stockSymbol_->SetToolTip(_("Enter the stock symbol. (Optional) Include exchange. eg: IBM.BE"));

    //Number of Shares
    wxStaticText* number = new wxStaticText(itemPanel5, wxID_STATIC, _("Number of Shares"));
    itemFlexGridSizer6->Add(number, flags);
    number->SetFont(this->GetFont().Bold());
    numShares_ = new mmTextCtrl(itemPanel5, ID_TEXTCTRL_NUMBER_SHARES, ""
        , wxDefaultPosition, wxSize(150, -1), wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    itemFlexGridSizer6->Add(numShares_, flags);
    numShares_->SetToolTip(_("Enter number of shares held"));
    numShares_->Connect(ID_TEXTCTRL_NUMBER_SHARES, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmStockDialog::OnTextEntered), nullptr, this);

    //Purchase Price
    wxStaticText* pprice = new wxStaticText(itemPanel5, wxID_STATIC, _("Purchase Price"));
    itemFlexGridSizer6->Add(pprice, flags);
    pprice->SetFont(this->GetFont().Bold());
    purchasePrice_ = new mmTextCtrl( itemPanel5, ID_TEXTCTRL_STOCK_PP, ""
        , wxDefaultPosition, wxSize(150, -1), wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    itemFlexGridSizer6->Add(purchasePrice_, flags);
    purchasePrice_->SetToolTip(_("Enter purchase price for each stock"));
    purchasePrice_->Connect(ID_TEXTCTRL_STOCK_PP, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmStockDialog::OnTextEntered), nullptr, this);

    //
    itemFlexGridSizer6->Add(new wxStaticText( itemPanel5, wxID_STATIC, _("Current Price")), flags);
    currentPrice_ = new mmTextCtrl( itemPanel5, ID_TEXTCTRL_STOCK_CP, ""
        , wxDefaultPosition, wxSize(150, -1), wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    itemFlexGridSizer6->Add(currentPrice_, flags);
    currentPrice_->SetToolTip(_("Enter current stock price"));
    currentPrice_->Connect(ID_TEXTCTRL_STOCK_CP, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmStockDialog::OnTextEntered), nullptr, this);

    //
    itemFlexGridSizer6->Add(new wxStaticText( itemPanel5, wxID_STATIC, _("Commission")), flags);
    commission_ = new mmTextCtrl( itemPanel5, ID_TEXTCTRL_STOCK_COMMISSION, "0"
        , wxDefaultPosition, wxSize(150, -1), wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    itemFlexGridSizer6->Add(commission_, flags);
    commission_->SetToolTip(_("Enter any commission paid"));
    commission_->Connect(ID_TEXTCTRL_STOCK_COMMISSION, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmStockDialog::OnTextEntered), nullptr, this);

    //
    itemFlexGridSizer6->Add(new wxStaticText( itemPanel5, wxID_STATIC, _("Value")), flags);
    valueInvestment_ = new wxStaticText( itemPanel5, ID_STATIC_STOCK_VALUE, "--");
    itemFlexGridSizer6->Add(valueInvestment_, flags);

    //
    itemFlexGridSizer6->Add(new wxStaticText( itemPanel5, wxID_STATIC, _("Notes")), flags);
    bAttachments_ = new wxBitmapButton( itemPanel5, wxID_FILE
        , wxBitmap(attachment_xpm), wxDefaultPosition
        , wxSize(commission_->GetSize().GetY(), commission_->GetSize().GetY()));
    itemFlexGridSizer6->Add(bAttachments_, wxSizerFlags(flags).Align(wxALIGN_RIGHT));
    bAttachments_->SetToolTip(_("Organize attachments of this stock"));

    notes_ = new mmTextCtrl( this, wxID_STATIC, "", wxDefaultPosition, wxSize(200, 90), wxTE_MULTILINE );
    itemStaticBoxSizer4->Add(notes_, flagsExpand);
    itemStaticBoxSizer4->AddSpacer(1);
    notes_->SetToolTip(_("Enter notes associated with this investment"));

    wxPanel* itemPanel27 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    itemBoxSizer3->Add(itemPanel27, flags.Right());

    wxBoxSizer* itemBoxSizer28 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel27->SetSizer(itemBoxSizer28);

    wxButton* itemButtonOK = new wxButton( itemPanel27, wxID_OK);
    wxButton* itemButton30 = new wxButton( itemPanel27, wxID_CANCEL);
    wxBitmapButton* itemButton31 = new wxBitmapButton(itemPanel27, wxID_INDEX, wxNullBitmap
        , wxDefaultPosition, wxSize(itemButtonOK->GetSize().GetHeight(), itemButtonOK->GetSize().GetHeight()));
    itemButton31->SetToolTip(_("Will display the web page for the specified Stock symbol"));
    //TODO: Provide other ico
    itemButton31->SetBitmapLabel(update_currency_xpm);

    if (edit_)
        itemButton30->SetFocus();
    itemBoxSizer28->Add(itemButton31, flags);
    itemBoxSizer28->Add(itemButtonOK, flags);
    itemBoxSizer28->Add(itemButton30, flags);

}


void mmStockDialog::OnCancel(wxCommandEvent& /*event*/)
{
    EndModal(wxID_CANCEL);
}

void mmStockDialog::OnAttachments(wxCommandEvent& /*event*/)
{
    wxString RefType = Model_Attachment::reftype_desc(Model_Attachment::STOCK);
    int RefId = stockID_;

    if (RefId < 0)
        RefId = 0;

    if (RefId == 0 && !skip_attachments_init_)
    {
        mmAttachmentManage::DeleteAllAttachments(RefType, 0);
        skip_attachments_init_ = true;
    }

    mmAttachmentDialog dlg(this, RefType, RefId);
    dlg.ShowModal();
}

void mmStockDialog::OnStockPriceButton(wxCommandEvent& /*event*/)
{
    wxString stockSymbol = stockSymbol_->GetValue().Trim();

    if (!stockSymbol.IsEmpty())
    {
        wxString stockURL = Model_Infotable::instance().GetStringInfo("STOCKURL", mmex::weblink::DefStockUrl);
        wxString httpString = wxString::Format(stockURL, stockSymbol);
        wxLaunchDefaultBrowser(httpString);
    }
}

void mmStockDialog::OnOk(wxCommandEvent& /*event*/)
{
    Model_Account::Data* account = Model_Account::instance().get(accountID_);
    if (!account)
    {
        mmShowErrorMessageInvalid(this, _("Held At"));
        return;
    }
    wxString pdate = dpc_->GetValue().FormatISODate();
    wxString heldAt = account->ACCOUNTNAME; 

    wxString stockName = stockName_->GetValue();
    wxString stockSymbol = stockSymbol_->GetValue();
    wxString notes       = notes_->GetValue();

    wxString numSharesStr = numShares_->GetValue().Trim();
    double numShares = 0;
    if (!wxNumberFormatter::FromString(numSharesStr, &numShares) || numShares < 0)
    {
        mmShowErrorMessage(this, _("Invalid number of shares entered "), _("Error"));
        return;
    }

    Model_Currency::Data *currency = Model_Account::currency(account);

    wxString pPriceStr = purchasePrice_->GetValue().Trim();
    double pPrice;
    if (!Model_Currency::fromString(pPriceStr, pPrice, currency) || pPrice < 0)
    {
        mmShowErrorMessage(this, _("Invalid purchase price entered "), _("Error"));
        return;
    }

    wxString currentPriceStr = currentPrice_->GetValue().Trim();
    double cPrice;
    if (!Model_Currency::fromString(currentPriceStr, cPrice, currency) || cPrice < 0)
    {
        // we assume current price = purchase price
        cPrice = pPrice;
    }

    wxString commissionStr = commission_->GetValue().Trim();
    double commission;
    if (!Model_Currency::fromString(commissionStr, commission, currency) || commission < 0)
    {
        mmShowErrorMessage(this, _("Invalid commission entered "), _("Error"));
        return;
    }

    double cValue = cPrice * numShares;

    if (!m_stock) m_stock = Model_Stock::instance().create();

    m_stock->HELDAT = accountID_;
    m_stock->PURCHASEDATE = pdate;
    m_stock->STOCKNAME = stockName;
    m_stock->SYMBOL = stockSymbol;
    m_stock->NUMSHARES = numShares;
    m_stock->PURCHASEPRICE = pPrice;
    m_stock->NOTES = notes;
    m_stock->CURRENTPRICE = cPrice;
    m_stock->VALUE = cValue;
    m_stock->COMMISSION = commission;
    if (edit_) m_stock->STOCKID = stockID_;

    Model_Stock::instance().save(m_stock);

    if (!edit_)
    {
        transID_ = m_stock->STOCKID;
        wxString RefType = Model_Attachment::reftype_desc(Model_Attachment::STOCK);
        mmAttachmentManage::RelocateAllAttachments(RefType, 0, transID_);
    }
    else
        transID_ = stockID_;

    EndModal(wxID_OK);
}

void mmStockDialog::OnTextEntered(wxCommandEvent& event)
{
    Model_Currency::Data *currency = Model_Currency::GetBaseCurrency();
    Model_Account::Data *account = Model_Account::instance().get(accountID_);
    if (account) currency = Model_Account::currency(account);

    if (event.GetId() == numShares_->GetId())
    {
        numShares_->Calculate(currency, 4);
    }
    else if (event.GetId() == purchasePrice_->GetId())
    {
        purchasePrice_->Calculate(currency, 4);
    }
    else if (event.GetId() == currentPrice_->GetId())
    {
        currentPrice_->Calculate(currency, 4);
    }
    else if (event.GetId() == commission_->GetId())
    {
        commission_->Calculate(currency, 4);
    }
}
