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

#pragma once

#include "model/Model_Stock.h"

class wxDatePickerCtrl;
class mmTextCtrl;
class mmUserPanelTrans;

class mmStockTransDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmStockTransDialog);
    wxDECLARE_EVENT_TABLE();

public:
    mmStockTransDialog();
    mmStockTransDialog(wxWindow* parent, Model_Stock::Data* stock, int accountID);

    Model_Stock::Data* m_stock;
    int m_stock_id;

private:
    bool Create(wxWindow* parent, wxWindowID id
        , const wxString& caption
        , const wxPoint& pos
        , const wxSize& size
        , long style);

    void CreateControls();
    void DataToControls();

    void OnOk(wxCommandEvent& WXUNUSED(event));
    void OnCancel(wxCommandEvent& WXUNUSED(event));
	void OnAttachments(wxCommandEvent& event);
    void OnStockPriceButton(wxCommandEvent& event);
    void OnQuit(wxCloseEvent& WXUNUSED(event));
    void OnDateChange(wxDateEvent& WXUNUSED(event));
    void OnTextEntered(wxCommandEvent& event);

 

    mmTextCtrl* m_stock_name;
    mmTextCtrl* m_stock_symbol;
    wxDatePickerCtrl* m_dpc;
    mmTextCtrl* m_num_shares;
    mmTextCtrl* m_purchase_price;
    mmTextCtrl* m_notes;

    mmTextCtrl* m_commission;
	wxBitmapButton* m_attachments;
    mmUserPanelTrans* m_checking_entry_panel;

    bool m_edit;
    int m_accountID;

    enum
    {
        ID_STOCKTRANS_DATEPICKER_CHANGE = wxID_HIGHEST + 800,
        ID_STOCKTRANS_NAME,
        ID_STOCKTRANS_SYMBOL,
        ID_STOCKTRANS_NUMBER_SHARES,
        ID_STOCKTRANS_PURCHASEPRICE,
        ID_STOCKTRANS_COMMISSION,
    };
};
