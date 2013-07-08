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
#include "constants.h"
#include "paths.h"
#include "mmCurrencyFormatter.h"
#include "util.h"
#include "validators.h"
#include <wx/valnum.h>
#include "model/Model_Infotable.h"

IMPLEMENT_DYNAMIC_CLASS( mmStockDialog, wxDialog )

BEGIN_EVENT_TABLE( mmStockDialog, wxDialog )
    EVT_BUTTON(wxID_OK, mmStockDialog::OnOk)
    EVT_BUTTON(wxID_CANCEL, mmStockDialog::OnCancel)
    EVT_BUTTON(wxID_INDEX, mmStockDialog::OnStockPriceButton)
END_EVENT_TABLE()

mmStockDialog::mmStockDialog( )
{
}

mmStockDialog::mmStockDialog(mmCoreDB* core, mmStockTransactionHolder* stock_holder, bool edit,
                             int accountID, wxWindow* parent, wxWindowID id,
                             const wxString& caption, const wxPoint& pos,
                             const wxSize& size, long style )
{
    core_ = core;
    stock_holder_ = stock_holder;
    edit_ = edit;
    accountID_ = accountID;
    Create(parent, id, caption, pos, size, style);
    //core_->currencyList_.LoadBaseCurrencySettings();
    mmDBWrapper::loadCurrencySettings(core_->db_.get(), accountID_);
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
    stockID_ = stock_holder_->id_;

    stockName_->SetValue(stock_holder_->shareName_);
    stockSymbol_->SetValue(stock_holder_->stockSymbol_);
    notes_->SetValue(stock_holder_->shareNotes_);

    wxString dateString = stock_holder_-> stockPDate_;
    wxDateTime dtdt = mmGetStorageStringAsDate(dateString);
    dpc_->SetValue(dtdt);

    double numShares = stock_holder_->numShares_;
    wxString numSharesString;
    //I wish see integer if it integer else double
    if ((numShares - static_cast<long>(numShares)) != 0.0 )
    {
        numSharesString = CurrencyFormatter::float2String(numShares);
        //numSharesString=wxString::Format("%0.4f",numShares);
    }
    else
        numSharesString <<  static_cast<long>(numShares);

    numShares_->SetValue(numSharesString);
    valueInvestment_->SetLabel(CurrencyFormatter::float2String(stock_holder_->value_));
    purchasePrice_->SetValue(CurrencyFormatter::float2String(stock_holder_->purchasePrice_));
    currentPrice_->SetValue(CurrencyFormatter::float2String(stock_holder_->currentPrice_));
    commission_->SetValue(CurrencyFormatter::float2String(stock_holder_->commission_));
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

    stockName_ = new wxTextCtrl( itemPanel5, ID_TEXTCTRL_STOCKNAME, "");
    itemFlexGridSizer6->Add(stockName_, flagsExpand);
    stockName_->SetToolTip(_("Enter the stock company name"));
    if (!edit_)
        stockName_->SetFocus();

    //Date
    itemFlexGridSizer6->Add(new wxStaticText( itemPanel5, wxID_STATIC, _("Date")), flags);

    dpc_ = new wxDatePickerCtrl( itemPanel5, ID_DPC_STOCK_PDATE,
        wxDefaultDateTime, wxDefaultPosition, wxSize(120, -1), wxDP_DROPDOWN | wxDP_SHOWCENTURY);
    itemFlexGridSizer6->Add(dpc_, flags);
    dpc_->SetToolTip(_("Specify the purchase date of the stock investment"));
    //

    //Symbol
    itemFlexGridSizer6->Add(new wxStaticText( itemPanel5, wxID_STATIC, _("Symbol")), flags);

    stockSymbol_ = new wxTextCtrl( itemPanel5, ID_TEXTCTRL_STOCK_SYMBOL,
        "", wxDefaultPosition, wxSize(150, -1), 0 );
    itemFlexGridSizer6->Add(stockSymbol_, flags);
    stockSymbol_->SetToolTip(_("Enter the stock symbol. (Optional) Include exchange. eg: IBM.BE"));
    //

    itemFlexGridSizer6->Add(new wxStaticText( itemPanel5, wxID_STATIC, _("Number of Shares")), flags);

    numShares_ = new wxTextCtrl( itemPanel5, ID_TEXTCTRL_NUMBER_SHARES, "",
        wxDefaultPosition, wxSize(150, -1), wxALIGN_RIGHT|wxTE_PROCESS_ENTER , mmDoubleValidator4() );
    itemFlexGridSizer6->Add(numShares_, flags);
    numShares_->SetToolTip(_("Enter number of shares held"));

    itemFlexGridSizer6->Add(new wxStaticText( itemPanel5, wxID_STATIC, _("Purchase Price")), flags);

    purchasePrice_ = new wxTextCtrl( itemPanel5, ID_TEXTCTRL_STOCK_PP, "",
        wxDefaultPosition, wxSize(150, -1), wxALIGN_RIGHT|wxTE_PROCESS_ENTER , mmDoubleValidator4() );
    itemFlexGridSizer6->Add(purchasePrice_, flags);
    purchasePrice_->SetToolTip(_("Enter purchase price for each stock"));

    itemFlexGridSizer6->Add(new wxStaticText( itemPanel5, wxID_STATIC, _("Current Price")), flags);

    currentPrice_ = new wxTextCtrl( itemPanel5, ID_TEXTCTRL_STOCK_CP, "",
        wxDefaultPosition, wxSize(150, -1), wxALIGN_RIGHT|wxTE_PROCESS_ENTER , mmDoubleValidator4() );
    itemFlexGridSizer6->Add(currentPrice_, flags);
    currentPrice_->SetToolTip(_("Enter current stock price"));

    itemFlexGridSizer6->Add(new wxStaticText( itemPanel5, wxID_STATIC, _("Commission")), flags);

    commission_ = new wxTextCtrl( itemPanel5, ID_TEXTCTRL_STOCK_COMMISSION, "0",
        wxDefaultPosition, wxSize(150, -1), wxALIGN_RIGHT|wxTE_PROCESS_ENTER , mmDoubleValidator4() );
    itemFlexGridSizer6->Add(commission_, flags);
    commission_->SetToolTip(_("Enter any commission paid"));

    itemFlexGridSizer6->Add(new wxStaticText( itemPanel5, wxID_STATIC, _("Value")), flags);

    valueInvestment_ = new wxStaticText( itemPanel5, ID_STATIC_STOCK_VALUE, "--");
    itemFlexGridSizer6->Add(valueInvestment_, flags);

    itemFlexGridSizer6->Add(new wxStaticText( itemPanel5, wxID_STATIC, _("Notes")), flags);

    notes_ = new wxTextCtrl( this, wxID_STATIC, "", wxDefaultPosition, wxSize(200, 90), wxTE_MULTILINE );
    itemStaticBoxSizer4->Add(notes_, flagsExpand);
    itemStaticBoxSizer4->AddSpacer(1);
    notes_->SetToolTip(_("Enter notes associated with this investment"));

    wxPanel* itemPanel27 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    itemBoxSizer3->Add(itemPanel27, flags.Right());

    wxBoxSizer* itemBoxSizer28 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel27->SetSizer(itemBoxSizer28);

    wxButton* itemButtonOK = new wxButton( itemPanel27, wxID_OK);
    wxButton* itemButton30 = new wxButton( itemPanel27, wxID_CANCEL);
    wxBitmapButton* itemButton31 = new wxBitmapButton(itemPanel27, wxID_INDEX, wxNullBitmap,
        wxDefaultPosition, wxSize(itemButtonOK->GetSize().GetHeight(), itemButtonOK->GetSize().GetHeight()));
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

void mmStockDialog::OnStockPriceButton(wxCommandEvent& /*event*/)
{
    wxString stockSymbol = stockSymbol_->GetValue().Trim();

    if (!stockSymbol.IsEmpty())
    {
        // Use Google for stock quotes
        wxString stockURL = Model_Infotable::instance().GetStringInfo("STOCKURL", mmex::DEFSTOCKURL);
        //wxString paddedURL = "\"" + stockURL + "\"";
        //wxString httpString = wxString::Format(paddedURL, stockSymbol);
        //wxExecute(_T("explorer ") + httpString, wxEXEC_ASYNC, NULL );

        int yahooSite = stockURL.Find("yahoo");
        if ( yahooSite != wxNOT_FOUND )
        {
            int hasSuffix = stockSymbol.Find(".");
            if ( hasSuffix == wxNOT_FOUND)
            {
                wxString stockSuffix = Model_Infotable::instance().GetStringInfo("HTTP_YAHOO_SUFFIX", "");
                if (! stockSuffix.IsEmpty() )
                    stockSymbol << stockSuffix;
            }
        }
        wxString httpString = wxString::Format(stockURL, stockSymbol);
        wxLaunchDefaultBrowser(httpString);
    }
}

void mmStockDialog::OnOk(wxCommandEvent& /*event*/)
{
    wxString pdate = dpc_->GetValue().FormatISODate();

    if (accountID_ == -1)
    {
        mmShowErrorMessageInvalid(this, _("Held At"));
        return;
    }
    wxString heldAt =  core_->accountList_.GetAccountName(accountID_);

    wxString stockName = stockName_->GetValue();
    wxString stockSymbol = stockSymbol_->GetValue();
    wxString notes       = notes_->GetValue();

    wxString numSharesStr = numShares_->GetValue().Trim();
    double numShares = 0;
    if (!numSharesStr.ToDouble(&numShares) /*|| (numShares = 0.0)*/)
    {
        mmShowErrorMessage(this, _("Invalid number of shares entered "), _("Error"));
        return;
    }

    /*if (numShares <= 0)
    {
        mmShowErrorMessageInvalid(this, _("Num Shares"));
        return;
    }*/

    wxString pPriceStr    = purchasePrice_->GetValue().Trim();
    double pPrice;
    if (! CurrencyFormatter::formatCurrencyToDouble(pPriceStr, pPrice) || (pPrice < 0.0))
    {
        mmShowErrorMessage(this, _("Invalid purchase price entered "), _("Error"));
        return;
    }

    wxString currentPriceStr = currentPrice_->GetValue().Trim();
    double cPrice;
    if (! CurrencyFormatter::formatCurrencyToDouble(currentPriceStr, cPrice) || (cPrice < 0.0))
    {
        //mmShowErrorMessage(this, _("Invalid current price entered "), _("Error"));
        //return;
        // we assume current price = purchase price
        cPrice = pPrice;
    }

    wxString commissionStr = commission_->GetValue().Trim();
    double commission;
    if (! CurrencyFormatter::formatCurrencyToDouble(commissionStr, commission) || (commission < 0.0))
    {
        mmShowErrorMessage(this, _("Invalid commission entered "), _("Error"));
        return;
    }

    double cValue = cPrice * numShares;

    wxSQLite3Statement st;
    if (edit_)
        st = core_->db_.get()->PrepareStatement(UPDATE_ROW_STOCK_V1);
    else
        st = core_->db_.get()->PrepareStatement(INSERT_ROW_INTO_STOCK_V1);

    int i = 0;
    st.Bind(++i, accountID_);
    st.Bind(++i, pdate);
    st.Bind(++i, stockName);
    st.Bind(++i, stockSymbol);
    st.Bind(++i, numShares);
    st.Bind(++i, pPrice);
    st.Bind(++i, notes);
    st.Bind(++i, cPrice);
    st.Bind(++i, cValue);
    st.Bind(++i, commission);
    if (edit_) st.Bind(++i, stockID_);

    wxASSERT(st.GetParamCount() == i);

    st.ExecuteUpdate();
    if (!edit_)
        transID_ = core_->db_.get()->GetLastRowId().ToLong();
    else
        transID_ = stockID_;
    st.Finalize();

    EndModal(wxID_OK);
}
