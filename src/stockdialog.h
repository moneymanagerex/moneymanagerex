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

#ifndef _MM_EX_STOCKDIALOG_H_
#define _MM_EX_STOCKDIALOG_H_

#include "defs.h"
#include "guiid.h"
#include "model/Model_Stock.h"

#define SYMBOL_STOCKDIALOG_STYLE wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_STOCKDIALOG_TITLE _("New/Edit Stock Investment")
#define SYMBOL_STOCKDIALOG_IDNAME ID_DIALOG_STOCKS
#define SYMBOL_STOCKDIALOG_SIZE wxSize(400, 300)
#define SYMBOL_STOCKDIALOG_POSITION wxDefaultPosition

class wxDatePickerCtrl;
class mmCoreDB;
class mmTextCtrl;

class mmStockDialog : public wxDialog
{
    DECLARE_DYNAMIC_CLASS( mmStockDialog )
    DECLARE_EVENT_TABLE()

public:
    mmStockDialog();
    mmStockDialog(mmCoreDB* core
                  , Model_Stock::Data* stock
                  , bool edit, int accountID, wxWindow* parent
                  , wxWindowID id = SYMBOL_STOCKDIALOG_IDNAME
                  , const wxString& caption = SYMBOL_STOCKDIALOG_TITLE
                  , const wxPoint& pos = SYMBOL_STOCKDIALOG_POSITION
                  , const wxSize& size = SYMBOL_STOCKDIALOG_SIZE
                  , long style = SYMBOL_STOCKDIALOG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_STOCKDIALOG_IDNAME
                 , const wxString& caption = SYMBOL_STOCKDIALOG_TITLE
                 , const wxPoint& pos = SYMBOL_STOCKDIALOG_POSITION
                 , const wxSize& size = SYMBOL_STOCKDIALOG_SIZE
                 , long style = SYMBOL_STOCKDIALOG_STYLE );

    void CreateControls();

    void OnOk(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnStockPriceButton(wxCommandEvent& event);

    void fillControls();
    void dataToControls();
    int transID_;
    Model_Stock::Data* m_stock;

private:
    mmCoreDB* core_;
    int stockID_;

    mmTextCtrl* stockName_;
    mmTextCtrl* stockSymbol_;
    wxDatePickerCtrl* dpc_;
    mmTextCtrl* numShares_;
    mmTextCtrl* purchasePrice_;
    mmTextCtrl* notes_;
    mmTextCtrl* currentPrice_;
    wxStaticText* valueInvestment_;
    mmTextCtrl* commission_;

    bool edit_;
    int accountID_;
};

#endif
