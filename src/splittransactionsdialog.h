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

#ifndef MM_EX_SPLITTRANSACTIONDIALOG_H_
#define MM_EX_SPLITTRANSACTIONDIALOG_H_

#include "defs.h"
#include "model/Model_Checking.h"
#include <wx/dataview.h>
#include <wx/vscroll.h>

class wxListCtrl;
class wxStaticText;

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
#define wxFIXED_MINSIZE 0
#endif

class SplitTransactionDialog: public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(SplitTransactionDialog);
    wxDECLARE_EVENT_TABLE();

public:
    /// Constructors
    SplitTransactionDialog();
    SplitTransactionDialog(wxWindow* parent
        , std::vector<Split>& split
        , int transType
        , int accountID
        , double totalAmount = 0.0
        , const wxString& name = "SplitTransactionDialog"
        );

    /// Creation
    bool Create(
        wxWindow* parent
        , wxWindowID id = wxID_ANY
        , const wxString& caption = _("Split Transaction Dialog")
        , const wxPoint& pos = wxDefaultPosition
        , const wxSize& size = wxDefaultSize
        , long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX
        , const wxString& name = "Split Transaction Dialog"
        );
    std::vector<Split> getResult();
    bool isItemsChanged();

private:
    /// Creates the controls and sizers
    void CreateControls();

    void DataToControls();

    void OnOk( wxCommandEvent& event );
    void OnTextEntered(wxCommandEvent& event);
    void OnCheckBox(wxCommandEvent& event);
    void OnFocusChange(wxChildFocusEvent& event);

    void mmDoEnableLineById(int id, bool value = true);
    void mmDoCreateRow(int i);
    void UpdateSplitTotal();

    std::vector<Split> m_splits;
    std::vector<Split> m_local_splits;
    int transType_;
    int accountID_;
    double totalAmount_;
    bool items_changed_;
    int object_in_focus_;

    wxButton* itemButtonOK_;
    wxBoxSizer* mainSizer;
    wxScrolledWindow* slider;
    wxStaticText* transAmount_;
    wxFlexGridSizer* grid_sizer;

};
inline std::vector<Split> SplitTransactionDialog::getResult() { return m_splits; }
inline bool SplitTransactionDialog::isItemsChanged() { return items_changed_; }

#endif
