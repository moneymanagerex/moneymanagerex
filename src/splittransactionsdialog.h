/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2013 - 2016, 2020 -2022 Nikolay Akimov
 Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)

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
#include "mmSimpleDialogs.h"
#include "mmTextCtrl.h"
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

class mmEditSplitOther : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmEditSplitOther);
    wxDECLARE_EVENT_TABLE();

public:
    mmEditSplitOther();
    mmEditSplitOther(wxWindow* parent, Model_Currency::Data* currency, Split* split
                        , const wxString &name = "mmEditSplitOther");
    ~mmEditSplitOther();

private:
    Split* m_split = nullptr;
    Model_Currency::Data* m_currency = nullptr;
    wxTextCtrl* m_Notes = nullptr;

    void CreateControls();
    void fillControls();
    void OnCancel(wxCommandEvent& /*event*/);
    void OnOk(wxCommandEvent& /*event*/);

};

class mmSplitTransactionDialog: public wxDialog
{
public:
    ~mmSplitTransactionDialog();
    mmSplitTransactionDialog();
    mmSplitTransactionDialog(wxWindow* parent
        , std::vector<Split>& split
        , int accountID
        , int transType
        , double totalAmount = 0.0
        , bool is_view_only = false
        );
    std::vector<Split> mmGetResult() const;

private:
    bool Create(
        wxWindow* parent
        , wxWindowID id = wxID_ANY
        , const wxString& caption = _("Split Transaction")
        , const wxPoint& pos = wxDefaultPosition
        , const wxSize& size = wxDefaultSize
        , long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX
        , const wxString& name = "Split Transaction Dialog"
        );

    void CreateControls();
    void FillControls(const int focusRow = -1);
    void createNewRow(const bool enabled);
    void activateNewRow();
    void UpdateSplitTotal();
    void UpdateExtraInfo(int row);

    void OnOk(wxCommandEvent& event);
    void OnAddRow(wxCommandEvent& event);
    void OnRemoveRow(wxCommandEvent& event);
    void OnOtherButton(wxCommandEvent& event);
    void OnTextEntered(wxCommandEvent& event);
    void OnFocusChange(wxChildFocusEvent& event);
    void OnComboKey(wxKeyEvent& event);
    bool mmDoCheckRow(int row);

    struct SplitWidget
    {
        mmComboBoxCategory* category;
        mmTextCtrl* amount;
        mmTagTextCtrl* tags;
        wxButton* other;
    };

    std::vector<SplitWidget> m_splits_widgets;
    std::vector<Split> m_orig_splits, m_splits;
    double totalAmount_ =0.0;
    int transType_ = 0;
    int row_num_ = 0;
    Model_Currency::Data* m_currency = nullptr;
    bool is_view_only_;

    wxButton* itemButtonOK_ = nullptr;
    wxScrolledWindow* slider_ = nullptr;
    wxStaticText* transAmount_ = nullptr;
    wxFlexGridSizer* flexGridSizer_ = nullptr;

    wxDECLARE_EVENT_TABLE();
};

inline std::vector<Split> mmSplitTransactionDialog::mmGetResult() const { return m_orig_splits; }

#endif
