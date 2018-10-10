/*******************************************************
Copyright (C) 2014 Gabriele-V

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

#ifndef MM_EX_WEBAPPDIALOG_H_
#define MM_EX_WEBAPPDIALOG_H_

#include "defs.h"
#include "webapp.h"
#include <wx/dataview.h>
#include <wx/srchctrl.h>
#include <map>

class mmWebAppDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmWebAppDialog);
    wxDECLARE_EVENT_TABLE();

public:
    mmWebAppDialog(wxWindow* parent, const wxString& name = "mmWebAppDialog");

    bool getRefreshRequested() const { return refreshRequested_; }

private:
    enum cols
    {
        WEBTRAN_ID = 0,
        WEBTRAN_DATE,
        WEBTRAN_ACCOUNT,
        WEBTRAN_STATUS,
        WEBTRAN_TYPE,
        WEBTRAN_PAYEE,
        WEBTRAN_CATEGORY,
        WEBTRAN_AMOUNT,
        WEBTRAN_NOTES,
        WEBTRAN_ATTACHMENTS
    };

    enum menu_items
    {
        MENU_OPEN_ATTACHMENT = 1,
        MENU_IMPORT_WEBTRAN,
        MENU_DELETE_WEBTRAN
    };

    wxDataViewListCtrl* webtranListBox_;

    mmWebApp::WebTranVector WebAppTransactions_;
    bool refreshRequested_;

    mmWebAppDialog() : refreshRequested_(false) {}

    void Create(wxWindow* parent, const wxString& name = "mmWebAppDialog");
    void CreateControls();
    void fillControls();

    void OnCancel(wxCommandEvent& WXUNUSED(event));
    void OnApply(wxCommandEvent& WXUNUSED(event));
    void OnOk(wxCommandEvent& WXUNUSED(event));

    bool ImportWebTr(int WebTrID, bool open);
    void ImportAllWebTr(const bool open);

    void OnListItemActivated(wxDataViewEvent& event);
    void OnMenuSelected(wxCommandEvent& event);
    void OnItemRightClick(wxDataViewEvent& event);

    void ImportWebTrSelected();
    void DeleteWebTr();
    void OpenAttachment();
};

#endif // MM_EX_PAYEEDIALOG_H_
