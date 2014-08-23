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

#ifndef MM_EX_STOCKDIALOG_H_
#define MM_EX_STOCKDIALOG_H_

#include "defs.h"
#include "model/Model_Stock.h"

class wxDatePickerCtrl;
class mmTextCtrl;

class mmStockDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmStockDialog);
    wxDECLARE_EVENT_TABLE();

public:
    mmStockDialog();
    mmStockDialog(wxWindow* parent
        , Model_Stock::Data* stock
        , int accountID);

    bool Create(wxWindow* parent, wxWindowID id
        , const wxString& caption
        , const wxPoint& pos
        , const wxSize& size
        , long style);

    void CreateControls();

    void OnOk(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
	void OnAttachments(wxCommandEvent& event);
    void OnStockPriceButton(wxCommandEvent& event);
    void OnHistoryImportButton(wxCommandEvent& event);
    void OnHistoryUpdateButton(wxCommandEvent& event);
    void OnHistoryDeleteButton(wxCommandEvent& event);
    void OnListItemSelected(wxListEvent& event);
    
    void fillControls();
    void dataToControls();
    void showStockHistory();
    int transID_;
    Model_Stock::Data* m_stock;

private:
    void OnTextEntered(wxCommandEvent& event);

    int stockID_;

    mmTextCtrl* stockName_;
    mmTextCtrl* stockSymbol_;
    wxDatePickerCtrl* dpc_;
    mmTextCtrl* numShares_;
    mmTextCtrl* purchasePrice_;
    mmTextCtrl* notes_;
    mmTextCtrl* currentPrice_;
    wxDatePickerCtrl* priceDate_;

    wxStaticText* valueInvestment_;
    mmTextCtrl* commission_;
	wxBitmapButton* bAttachments_;
    wxListCtrl* priceListBox_;

    bool edit_;
    int accountID_;
	bool skip_attachments_init_;

    enum
    {
        ID_DPC_STOCK_PDATE = wxID_HIGHEST + 800,
        ID_TEXTCTRL_STOCKNAME,
        ID_TEXTCTRL_STOCK_SYMBOL,
        ID_TEXTCTRL_NUMBER_SHARES,
        ID_TEXTCTRL_STOCK_PP,
        ID_TEXTCTRL_STOCK_CP,
        ID_STATIC_STOCK_VALUE,
        ID_TEXTCTRL_STOCK_COMMISSION,
        ID_DIALOG_STOCKS,
        ID_DPC_CP_PDATE,
    };
};

#endif
