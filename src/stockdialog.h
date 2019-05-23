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

#include <wx/dialog.h>
#include "Model_Stock.h"
#include <wx/dataview.h>

class wxDatePickerCtrl;
class mmTextCtrl;
class mmGUIFrame;
class wxDateEvent;
class wxStaticText;
class wxBitmapButton;

class mmStockDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmStockDialog);
    wxDECLARE_EVENT_TABLE();

public:
    mmStockDialog();
    mmStockDialog(wxWindow* parent
        , mmGUIFrame* gui_frame
        , Model_Stock::Data* stock
        , int accountID
        , const wxString& name = "mmStockDialog"
        );

    bool Create(wxWindow* parent, wxWindowID id
        , const wxString& caption
        , const wxPoint& pos
        , const wxSize& size
        , long style
        , const wxString& name = "mmStockDialog"
        );

    int m_stock_id;

private:
    void OnQuit(wxCloseEvent& event);
    void OnSave(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnAttachments(wxCommandEvent& event);
    void OnStockPriceButton(wxCommandEvent& event);
    void OnHistoryImportButton(wxCommandEvent& event);
    void OnHistoryDownloadButton(wxCommandEvent& event);
    void OnFocusChange(wxChildFocusEvent& event);
    void OnSelectionChanged(wxDataViewEvent& event);
    void OnListValueEditingDone(wxDataViewEvent& event);
    void OnListValueChanged(wxDataViewEvent& event);
    void OnTextEntered(wxCommandEvent& event);
    void OnMagicButton(wxCommandEvent& event);
    void OnItemRightClick(wxDataViewEvent& event);
    void OnMenuSelected(wxCommandEvent& event);
    void OnHistoryAddUpdateEntry(wxCommandEvent& event);

    void OnMenuAddSelected();
    void OnHistoryDelete();

    void CreateControls();
    void UpdateControls();
    void DataToControls();
    void RefreshStockHistory(const wxString& symbol);
    void CreateShareAccount(Model_Account::Data* stock_account, const wxString& name);

    mmTextCtrl* m_stock_symbol_ctrl;
    mmTextCtrl* m_stock_name_ctrl;
    wxDatePickerCtrl* m_purchase_date_ctrl; //*Date
    mmTextCtrl* m_num_shares_ctrl; //*Share Number
    mmTextCtrl* m_share_price_ctrl; //*Share Price
    wxStaticText* m_share_price_txt;
    mmTextCtrl* m_commission_ctrl;
    mmTextCtrl* m_current_price_ctrl;
    mmTextCtrl* m_notes_ctrl;
    wxStaticText* m_value_investment; //Current Value
    wxBitmapButton* m_bAttachments;
    wxDataViewListCtrl* m_price_listbox;
    wxDatePickerCtrl* m_history_date_ctrl;
    mmTextCtrl* m_exchange_text;

    Model_Stock::Data* m_stock;
    bool m_edit;
    bool debug_;
    int m_account_id;
    int m_object_in_focus;
    wxString m_decimal_point;
    int m_precision;
    Model_Account::Data* m_account;
    Model_Currency::Data* m_currency;
    wxString m_current_value;
    mmGUIFrame* m_gui_frame;
    enum
    {
        ID_DPC_STOCK_PDATE = wxID_HIGHEST + 800,
        ID_TEXTCTRL_STOCKNAME,
        ID_TEXTCTRL_STOCK_SYMBOL,
        ID_TEXTCTRL_NUMBER_SHARES,
        ID_TEXTCTRL_HIST_EXCHRATE,
        ID_TEXTCTRL_STOCK_PP,
        ID_TEXTCTRL_STOCK_CP,
        ID_STATIC_STOCK_VALUE,
        ID_TEXTCTRL_STOCK_COMMISSION,
        ID_DIALOG_STOCKS,
        ID_BUTTON_IMPORT,
        ID_BUTTON_DOWNLOAD,
        ID_HIST_PANEL
    };
    enum menu_items
    {
        MENU_NEW_ENTRY = 1,
        MENU_EDIT_ATTACHMENT,
        MENU_DELETE_ENTRY,
    };
};

#endif
