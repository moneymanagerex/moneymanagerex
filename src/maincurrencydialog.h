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

#ifndef _MM_EX_MAINCURRENCY_DIALOG_H_
#define _MM_EX_MAINCURRENCY_DIALOG_H_

#include "guiid.h"
#include "defs.h"
#include <wx/dataview.h>
#include <map>

#define ID_MYDIALOG8 10040
#define SYMBOL_MAINCURRENCYDIALOG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_MAINCURRENCYDIALOG_TITLE _("Currency Dialog")
#define SYMBOL_MAINCURRENCYDIALOG_IDNAME ID_MYDIALOG8
#define SYMBOL_MAINCURRENCYDIALOG_SIZE wxSize(320, 350)
#define SYMBOL_MAINCURRENCYDIALOG_POSITION wxDefaultPosition
#define ID_LISTBOX 10090
#define ID_PANEL10 10091

////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
#define wxFIXED_MINSIZE 0
#endif

class mmMainCurrencyDialog: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( mmMainCurrencyDialog )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    mmMainCurrencyDialog( ) {}

    mmMainCurrencyDialog(wxWindow* parent,
                          bool bEnableSelect = true,
                          wxWindowID id = SYMBOL_MAINCURRENCYDIALOG_IDNAME,
                          const wxString& caption = SYMBOL_MAINCURRENCYDIALOG_TITLE,
                          const wxPoint& pos = SYMBOL_MAINCURRENCYDIALOG_POSITION,
                          const wxSize& size = SYMBOL_MAINCURRENCYDIALOG_SIZE,
                          long style = SYMBOL_MAINCURRENCYDIALOG_STYLE );

    static bool Execute(wxWindow* parent, int& currencyID);

private:
    enum cols
    {
        CURR_BASE = 0,
        CURR_SYMBOL,
        CURR_NAME,
        BASE_RATE
    };

    /// Creation
    bool Create( wxWindow* parent,
                 wxWindowID id = SYMBOL_MAINCURRENCYDIALOG_IDNAME,
                 const wxString& caption = SYMBOL_MAINCURRENCYDIALOG_TITLE,
                 const wxPoint& pos = SYMBOL_MAINCURRENCYDIALOG_POSITION,
                 const wxSize& size = SYMBOL_MAINCURRENCYDIALOG_SIZE,
                 long style = SYMBOL_MAINCURRENCYDIALOG_STYLE );
    /// Creates the controls and sizers
    void CreateControls();
    void OnBtnAdd(wxCommandEvent& event);
    void OnBtnEdit(wxCommandEvent& event);
    void OnBtnSelect(wxCommandEvent& event);
    void OnBtnDelete(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnListItemActivated(wxDataViewEvent& event);
    void OnListItemSelected(wxDataViewEvent& event);
    void OnValueChanged(wxDataViewEvent& event);
    void fillControls();
    void OnShowHiddenChbClick(wxCommandEvent& event);
    virtual bool ShowToolTips() { return TRUE;}

    void OnOnlineUpdateCurRate(wxCommandEvent& event);
    bool onlineUpdateCurRate(int curr_id = -1);
    void OnItemRightClick(wxDataViewEvent& event);
    void OnMenuSelected(wxCommandEvent& event);

    wxDataViewListCtrl* currencyListBox_;
    std::map<int, wxString> ColName_;
    bool bEnableSelect_;
    double curr_rate_;
    wxButton* itemButtonEdit_;
    wxButton* itemButtonDelete_;
    wxCheckBox* cbShowAll_;

    int currencyID_;
    int selectedIndex_;
};

#endif // _MM_EX_MAINCURRENCY_DIALOG_H_

