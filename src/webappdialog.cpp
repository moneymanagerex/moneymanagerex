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

#include "webappdialog.h"
#include "images_list.h"
#include "constants.h"
#include "paths.h"
#include "transdialog.h"
#include "util.h"
#include "webapp.h"

wxIMPLEMENT_DYNAMIC_CLASS(mmWebAppDialog, wxDialog);

wxBEGIN_EVENT_TABLE(mmWebAppDialog, wxDialog)
    EVT_BUTTON(wxID_CANCEL, mmWebAppDialog::OnCancel)
    EVT_BUTTON(wxID_APPLY, mmWebAppDialog::OnApply)
    EVT_BUTTON(wxID_OK, mmWebAppDialog::OnOk)
    EVT_DATAVIEW_ITEM_ACTIVATED(wxID_ANY, mmWebAppDialog::OnListItemActivated)
    EVT_DATAVIEW_ITEM_CONTEXT_MENU(wxID_ANY, mmWebAppDialog::OnItemRightClick)
    EVT_MENU_RANGE(MENU_OPEN_ATTACHMENT, MENU_DELETE_WEBTRAN, mmWebAppDialog::OnMenuSelected)
wxEND_EVENT_TABLE()


mmWebAppDialog::mmWebAppDialog(wxWindow *parent, const wxString& name) :
    webtranListBox_()
    , refreshRequested_(false)
{
    Create(parent, name);
}

void mmWebAppDialog::Create(wxWindow* parent, const wxString& name)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);

    long style = wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER;
    if (!wxDialog::Create(parent, wxID_ANY, _("Import WebApp transactions")
        , wxDefaultPosition, wxDefaultSize, style, name))
    {
        return;
    }

    CreateControls();

    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    SetIcon(mmex::getProgramIcon());

    fillControls();

    Centre();
}

void mmWebAppDialog::CreateControls()
{
    wxBoxSizer* mainBoxSizer = new wxBoxSizer(wxVERTICAL);

    webtranListBox_ = new wxDataViewListCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(800, 500), wxDV_MULTIPLE | wxDV_ROW_LINES);

    webtranListBox_->AppendTextColumn("#", wxDATAVIEW_CELL_INERT, wxLIST_AUTOSIZE_USEHEADER); //WEBTRAN_ID
    webtranListBox_->AppendTextColumn(_("Date"), wxDATAVIEW_CELL_INERT, wxLIST_AUTOSIZE_USEHEADER); //WEBTRAN_DATE
    webtranListBox_->AppendTextColumn(_("Account"), wxDATAVIEW_CELL_INERT, wxLIST_AUTOSIZE_USEHEADER); //WEBTRAN_ACCOUNT
    webtranListBox_->AppendTextColumn(_("Status"), wxDATAVIEW_CELL_INERT, wxLIST_AUTOSIZE_USEHEADER); //WEBTRAN_STATUS
    webtranListBox_->AppendTextColumn(_("Type"), wxDATAVIEW_CELL_INERT, wxLIST_AUTOSIZE_USEHEADER); //WEBTRAN_TYPE
    webtranListBox_->AppendTextColumn(_("Payee"), wxDATAVIEW_CELL_INERT, wxLIST_AUTOSIZE_USEHEADER); //WEBTRAN_PAYEE, 
    webtranListBox_->AppendTextColumn(_("Category"), wxDATAVIEW_CELL_INERT, wxLIST_AUTOSIZE_USEHEADER); //WEBTRAN_CATEGORY, 
    webtranListBox_->AppendTextColumn(_("Amount"), wxDATAVIEW_CELL_INERT, wxLIST_AUTOSIZE_USEHEADER); //WEBTRAN_AMOUNT, 
    webtranListBox_->AppendTextColumn(_("Notes"), wxDATAVIEW_CELL_INERT, wxLIST_AUTOSIZE_USEHEADER); //WEBTRAN_NOTES, 
    webtranListBox_->AppendTextColumn(_("Attachments"), wxDATAVIEW_CELL_INERT, wxLIST_AUTOSIZE_USEHEADER); //WEBTRAN_ATTACHMENTS, 
    mainBoxSizer->Add(webtranListBox_, wxSizerFlags(g_flagsExpand).Border(wxALL, 10));

    wxPanel* buttons_panel = new wxPanel(this, wxID_ANY);
    mainBoxSizer->Add(buttons_panel, wxSizerFlags(g_flagsExpand).Proportion(0));
    wxBoxSizer*  tools_sizer = new wxBoxSizer(wxVERTICAL);
    buttons_panel->SetSizer(tools_sizer);

    wxStdDialogButtonSizer*  buttons_sizer = new wxStdDialogButtonSizer;
    tools_sizer->Add(buttons_sizer, wxSizerFlags(g_flagsV).Center());
    wxButton* buttonOK = new wxButton(buttons_panel, wxID_OK, _("&Import all "));
    wxButton* buttonApply = new wxButton(buttons_panel, wxID_APPLY, _("Import and open all "));
    wxButton* btnCancel = new wxButton(buttons_panel, wxID_CANCEL, wxGetTranslation(g_CancelLabel));

    buttons_sizer->Add(buttonOK, g_flagsH);
    buttons_sizer->Add(buttonApply, g_flagsH);
    buttons_sizer->Add(btnCancel, g_flagsH);

    this->SetSizer(mainBoxSizer);
}

void mmWebAppDialog::fillControls()
{
    webtranListBox_->DeleteAllItems();
    WebAppTransactions_.clear();

    mmWebApp::WebApp_DownloadNewTransaction(WebAppTransactions_, false);

    for (const auto& WebTran : WebAppTransactions_)
    {
        wxVector<wxVariant> data;
        data.push_back(wxVariant(wxString::Format(wxT("%i"),WebTran.ID))); //WEBTRAN_ID
        data.push_back(wxVariant(mmGetDateForDisplay(WebTran.Date.FormatISODate()))); //WEBTRAN_DATE
        data.push_back(wxVariant(WebTran.Account)); //WEBTRAN_ACCOUNT
        data.push_back(wxVariant(WebTran.Status)); //WEBTRAN_STATUS
        data.push_back(wxVariant(wxGetTranslation(WebTran.Type))); //WEBTRAN_TYPE

        wxString Payee = WebTran.Type != "Transfer" ? WebTran.Payee : "> " + WebTran.ToAccount;
        data.push_back(wxVariant(Payee)); //WEBTRAN_PAYEE

        wxString Category = WebTran.Category;
        if (WebTran.SubCategory != wxEmptyString) Category += ":" + WebTran.SubCategory;
        data.push_back(wxVariant(Category)); //WEBTRAN_CATEGORY

        Model_Currency::Data *currency = Model_Currency::GetBaseCurrency();
        wxString Amount = Model_Currency::toStringNoFormatting(WebTran.Amount, currency, Model_Currency::precision(currency));
        data.push_back(wxVariant(Amount)); //WEBTRAN_AMOUNT

        data.push_back(wxVariant(WebTran.Notes)); //WEBTRAN_NOTES
        data.push_back(wxVariant(WebTran.Attachments)); //WEBTRAN_ATTACHMENTS
        webtranListBox_->AppendItem(data, (wxUIntPtr)WebTran.ID);
    }
}

void mmWebAppDialog::OnListItemActivated(wxDataViewEvent& event)
{
    wxDataViewItem item = event.GetItem();
    int selected_index = webtranListBox_->ItemToRow(item);

    if (selected_index >= 0)
    {
        int WebTrID = (int)webtranListBox_->GetItemData(item);
        mmWebAppDialog::ImportWebTr(WebTrID, true);
        fillControls();
    }
}

bool mmWebAppDialog::ImportWebTr(int WebTrID, bool open)
{
    mmWebApp::webtran_holder WebTrToImport;
    bool bFound = false;

    for (const auto WebTr : WebAppTransactions_)
    {
        if (WebTr.ID == WebTrID)
        {
            bFound = true;
            WebTrToImport = WebTr;
            int InsertedTransactionID = mmWebApp::MMEX_InsertNewTransaction(WebTrToImport);
            if (InsertedTransactionID > 0)
            {
                if (open)
                {
                    //fillControls(); //TODO: Delete transaction from view
                    mmTransDialog EditTransactionDialog(this, 1, InsertedTransactionID, 0);
                    EditTransactionDialog.ShowModal();
                }
                refreshRequested_ = true;
            }
            else
            {
                bFound = false;
            }
            break;
        }
    }
    if (!bFound)
    {
        wxString msgStr = wxString() << _("Unable to insert transaction in MMEX database") << "\n";
        wxMessageBox(msgStr, _("WebApp communication error"), wxICON_ERROR);
    }

    return bFound;
}

void mmWebAppDialog::OpenAttachment()
{
    wxDataViewItemArray Selected;
    webtranListBox_->GetSelections(Selected);

    for (wxDataViewItem Item : Selected)
    {
        int selectedIndex_ = webtranListBox_->ItemToRow(Item);
        if (selectedIndex_ >= 0)
        {
            wxString AttachmentName = webtranListBox_->GetTextValue(selectedIndex_, WEBTRAN_ATTACHMENTS);
            AttachmentName = AttachmentName.BeforeFirst(';');
            wxString Path = mmWebApp::WebApp_GetAttachment(AttachmentName);
            if (Path != wxEmptyString)
                wxLaunchDefaultApplication(Path);
        }
    }
}

void mmWebAppDialog::ImportWebTrSelected()
{
    wxDataViewItemArray Selected;
    webtranListBox_->GetSelections(Selected);

    if (Selected.size() == 0)
        return;

    for (wxDataViewItem Item : Selected)
    {
        int selectedIndex_ = webtranListBox_->ItemToRow(Item);
        if (selectedIndex_ >= 0)
        {
            int WebTrID = (int)webtranListBox_->GetItemData(Item);
            mmWebAppDialog::ImportWebTr(WebTrID, true);
        }
    }
    fillControls();
}

void mmWebAppDialog::DeleteWebTr()
{
    wxDataViewItemArray Selected;
    webtranListBox_->GetSelections(Selected);

    if (Selected.size() == 0)
        return;

    for (wxDataViewItem Item : Selected)
    {
        int selectedIndex_ = webtranListBox_->ItemToRow(Item);
        if (selectedIndex_ >= 0)
        {
            mmWebApp::WebApp_DeleteOneTransaction((int)webtranListBox_->GetItemData(Item));
        }
    }
    fillControls();
}

void mmWebAppDialog::OnMenuSelected(wxCommandEvent& event)
{
    wxCommandEvent evt;

    switch (event.GetId())
    {
    case MENU_OPEN_ATTACHMENT: OpenAttachment(); break;
    case MENU_IMPORT_WEBTRAN: ImportWebTrSelected(); break;
    case MENU_DELETE_WEBTRAN: DeleteWebTr(); break;
    default: break;
    }
}

void mmWebAppDialog::OnItemRightClick(wxDataViewEvent& event)
{
    wxDataViewItemArray Selected;
    webtranListBox_->GetSelections(Selected);

    wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, wxID_ANY);
    evt.SetEventObject(this);

    wxMenu* mainMenu = new wxMenu;
    mainMenu->Append(new wxMenuItem(mainMenu, MENU_OPEN_ATTACHMENT, _("Open Attachment")));
    mainMenu->Append(new wxMenuItem(mainMenu, MENU_IMPORT_WEBTRAN, _("Import")));
    mainMenu->Append(new wxMenuItem(mainMenu, MENU_DELETE_WEBTRAN, _("Delete")));
    if (Selected.size() != 1) mainMenu->Enable(MENU_OPEN_ATTACHMENT, false);

    PopupMenu(mainMenu);
    delete mainMenu;
    event.Skip();
}


void mmWebAppDialog::ImportAllWebTr(const bool open)
{
    for (int i = 0; i < webtranListBox_->GetItemCount(); i++)
    {
        int WebTrID = wxAtoi(webtranListBox_->GetTextValue(i, WEBTRAN_ID));
        mmWebAppDialog::ImportWebTr(WebTrID, open);
    }
}

void mmWebAppDialog::OnCancel(wxCommandEvent& WXUNUSED(event))
{
    EndModal(wxID_CANCEL);
}

void mmWebAppDialog::OnApply(wxCommandEvent& WXUNUSED(event))
{
    mmWebAppDialog::ImportAllWebTr(true);
    EndModal(wxID_APPLY);
}

void mmWebAppDialog::OnOk(wxCommandEvent& WXUNUSED(event))
{
    mmWebAppDialog::ImportAllWebTr(false);
    EndModal(wxID_OK);
}
