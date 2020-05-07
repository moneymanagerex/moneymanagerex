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
    mmWebAppDialog(wxWindow* parent, const bool startup, const wxString& name = "mmWebAppDialog");
    ~mmWebAppDialog();
    bool getRefreshRequested() const { return refreshRequested_; }
    void fillControls();

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
        MENU_IMPORTOPEN_WEBTRAN,
        MENU_DELETE_WEBTRAN
    };

    wxBoxSizer* mainBoxSizer_;
    wxFlexGridSizer* loadingSizer_;
    wxDataViewListCtrl* webtranListBox_;
    wxSearchCtrl* m_maskTextCtrl;
    wxTextCtrl* url_text_;
    wxTextCtrl* guid_text_;
    wxBitmapButton* net_button_;
    wxGauge* gauge_;

    wxTimer autoWebAppDialogTimer_;

    wxArrayString tempFiles_;
    bool refreshRequested_;
    bool isStartup_;
    bool isFilledOnce_;
    int m_webtran_id;
    mmWebApp::WebTranVector WebAppTransactions_;

    mmWebAppDialog() : m_webtran_id(-1), refreshRequested_(false) {}

    void Create(wxWindow* parent, const wxString& name = "mmWebAppDialog");
    void CreateControls();
    void OnTimer(wxTimerEvent& /*event*/);

    void OnCancel(wxCommandEvent& /*event*/);
    void OnApply(wxCommandEvent& /*event*/);
    void OnOk(wxCommandEvent& /*event*/);
    void OnCheckNetwork(wxCommandEvent& /*event*/);

    bool ImportWebTr(int WebTrID, bool open);
    void ImportAllWebTr(const bool open);

    void OnListItemActivated(wxDataViewEvent& event);
    void OnMenuSelected(wxCommandEvent& event);
    void OnItemRightClick(wxDataViewEvent& event);

    void ImportWebTrSelected(const bool open);
    void DeleteWebTr();
    void OpenAttachment();
    void OnButtonHelpClick(wxCommandEvent& WXUNUSED(event));
};

#endif // MM_EX_PAYEEDIALOG_H_
