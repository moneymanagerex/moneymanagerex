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
#include "mmSimpleDialogs.h"
#include <wx/timer.h>

wxIMPLEMENT_DYNAMIC_CLASS(mmWebAppDialog, wxDialog);

wxBEGIN_EVENT_TABLE(mmWebAppDialog, wxDialog)
    EVT_BUTTON(wxID_CANCEL, mmWebAppDialog::OnCancel)
    EVT_BUTTON(wxID_APPLY, mmWebAppDialog::OnApply)
    EVT_BUTTON(wxID_OK, mmWebAppDialog::OnOk)
    EVT_BUTTON(wxID_EXECUTE, mmWebAppDialog::OnCheckNetwork)
    EVT_DATAVIEW_ITEM_ACTIVATED(wxID_ANY, mmWebAppDialog::OnListItemActivated)
    EVT_DATAVIEW_ITEM_CONTEXT_MENU(wxID_ANY, mmWebAppDialog::OnItemRightClick)
    EVT_MENU_RANGE(MENU_OPEN_ATTACHMENT, MENU_DELETE_WEBTRAN, mmWebAppDialog::OnMenuSelected)
    EVT_BUTTON(wxID_HELP, mmWebAppDialog::OnButtonHelpClick)
    EVT_TIMER(wxID_REFRESH, mmWebAppDialog::OnTimer)
wxEND_EVENT_TABLE()

mmWebAppDialog::~mmWebAppDialog()
{
    autoWebAppDialogTimer_.Stop();
    for (const auto& entry : tempFiles_)
    {
        wxRemoveFile(entry);
    }
}

void mmWebAppDialog::OnButtonHelpClick(wxCommandEvent& WXUNUSED(event))
{
    EndModal(wxID_HELP);
}

mmWebAppDialog::mmWebAppDialog(wxWindow *parent, const bool startup, const wxString& name) :
    autoWebAppDialogTimer_(this, wxID_REFRESH)
{
    isStartup_ = startup;
    Create(parent, name);
}

void mmWebAppDialog::Create(wxWindow* parent, const wxString& name)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    long style = wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER;
    if (!wxDialog::Create(parent, wxID_ANY, _t("Import WebApp transactions")
        , wxDefaultPosition, wxDefaultSize, style, name))
    {
        return;
    }

    CreateControls();

    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    SetIcon(mmex::getProgramIcon());
    Centre();
}

void mmWebAppDialog::CreateControls()
{
    mainBoxSizer_ = new wxBoxSizer(wxVERTICAL);

    // Header --------------------------------------------
    wxFlexGridSizer* flex_sizer = new wxFlexGridSizer(0, 6, 0, 0);

    net_button_ = new wxBitmapButton(this, wxID_EXECUTE, mmBitmapBundle(png::LED_OFF, mmBitmapButtonSize));
    mmToolTip(net_button_, _t("Network status (click to refresh)"));
    flex_sizer->Add(net_button_, g_flagsCenter);

    wxStaticText* url_label = new wxStaticText(this, wxID_STATIC, _t("URL"));
    url_text_ = new wxTextCtrl(this, wxID_FILE, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    url_text_->SetMinSize(wxSize(300, -1));
    url_text_->Enable(false);
    url_text_->SetValue(mmWebApp::getUrl());
    mmToolTip(url_text_, mmWebApp::getUrl());

    wxStaticText* guid_label = new wxStaticText(this, wxID_STATIC, _t("GUID"));
    guid_text_ = new wxTextCtrl(this, wxID_FILE, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    guid_text_->SetMinSize(wxSize(300, -1));
    guid_text_->Enable(false);
    guid_text_->SetValue(mmWebApp::getGuid());

    wxBitmapButton* help_button = new wxBitmapButton(this, wxID_HELP, mmBitmapBundle(png::HELP, mmBitmapButtonSize));
    if (isStartup_)
    {
        help_button->Disable();
    }

    flex_sizer->Add(url_label, g_flagsH);
    flex_sizer->Add(url_text_, g_flagsH);
    flex_sizer->Add(guid_label, g_flagsH);
    flex_sizer->Add(guid_text_, g_flagsH);
    flex_sizer->Add(help_button, g_flagsH);
    mainBoxSizer_->Add(flex_sizer, wxSizerFlags(g_flagsV).Center());

    //Loading --------------------------------------------
    loadingSizer_ = new wxFlexGridSizer(0, 2, 0, 0);

    wxStaticText* gauge_label = new wxStaticText(this, wxID_STATIC, _t("Checking for new transactions"));
    gauge_ = new wxGauge(this, wxID_STATIC, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL, wxDefaultValidator);
    gauge_->SetMinSize(wxSize(300, -1));

    loadingSizer_->Add(gauge_label, g_flagsH);
    loadingSizer_->Add(gauge_, g_flagsH);
    mainBoxSizer_->Add(loadingSizer_, wxSizerFlags(g_flagsV).Center());

    gauge_->Pulse();
    autoWebAppDialogTimer_.Start(500, false);

    // ListBox --------------------------------------------
    webtranListBox_ = new wxDataViewListCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(800, 500), wxDV_MULTIPLE | wxDV_ROW_LINES);

    webtranListBox_->AppendTextColumn("#", wxDATAVIEW_CELL_INERT, wxLIST_AUTOSIZE_USEHEADER); //WEBTRAN_ID
    webtranListBox_->AppendTextColumn(_t("Date"), wxDATAVIEW_CELL_INERT, wxLIST_AUTOSIZE_USEHEADER); //WEBTRAN_DATE
    webtranListBox_->AppendTextColumn(_t("Account"), wxDATAVIEW_CELL_INERT, wxLIST_AUTOSIZE_USEHEADER); //WEBTRAN_ACCOUNT
    webtranListBox_->AppendTextColumn(_t("Status"), wxDATAVIEW_CELL_INERT, wxLIST_AUTOSIZE_USEHEADER); //WEBTRAN_STATUS
    webtranListBox_->AppendTextColumn(_t("Type"), wxDATAVIEW_CELL_INERT, wxLIST_AUTOSIZE_USEHEADER); //WEBTRAN_TYPE
    webtranListBox_->AppendTextColumn(_t("Payee"), wxDATAVIEW_CELL_INERT, wxLIST_AUTOSIZE_USEHEADER); //WEBTRAN_PAYEE, 
    webtranListBox_->AppendTextColumn(_t("Category"), wxDATAVIEW_CELL_INERT, wxLIST_AUTOSIZE_USEHEADER); //WEBTRAN_CATEGORY, 
    webtranListBox_->AppendTextColumn(_t("Amount"), wxDATAVIEW_CELL_INERT, wxLIST_AUTOSIZE_USEHEADER); //WEBTRAN_AMOUNT, 
    webtranListBox_->AppendTextColumn(_t("Notes"), wxDATAVIEW_CELL_INERT, wxLIST_AUTOSIZE_USEHEADER); //WEBTRAN_NOTES, 
    webtranListBox_->AppendTextColumn(_t("Attachments"), wxDATAVIEW_CELL_INERT, wxLIST_AUTOSIZE_USEHEADER); //WEBTRAN_ATTACHMENTS, 
    mainBoxSizer_->Add(webtranListBox_, g_flagsExpand);

    wxPanel* buttons_panel = new wxPanel(this, wxID_ANY);
    mainBoxSizer_->Add(buttons_panel, g_flagsCenter);
    wxBoxSizer*  tools_sizer = new wxBoxSizer(wxVERTICAL);
    buttons_panel->SetSizer(tools_sizer);

    wxStdDialogButtonSizer*  buttons_sizer = new wxStdDialogButtonSizer;
    tools_sizer->Add(buttons_sizer, wxSizerFlags(g_flagsV).Center());
    wxButton* buttonOK = new wxButton(buttons_panel, wxID_OK, _t("&Import all "));
    buttonOK->Enable(false);
    wxButton* buttonApply = new wxButton(buttons_panel, wxID_APPLY, _t("Import and open all "));
    buttonApply->Enable(false);
    wxButton* btnCancel = new wxButton(buttons_panel, wxID_CANCEL, wxGetTranslation(g_CancelLabel));

    buttons_sizer->Add(buttonOK, g_flagsH);
    buttons_sizer->Add(buttonApply, g_flagsH);
    buttons_sizer->Add(btnCancel, g_flagsH);

    this->SetSizer(mainBoxSizer_);
}

void mmWebAppDialog::OnCheckNetwork(wxCommandEvent& /*event*/)
{
    fillControls();
}

void mmWebAppDialog::fillControls()
{
    isFilledOnce_ = true;
    net_button_->SetBitmap(mmBitmapBundle(png::LED_OFF, mmBitmapButtonSize));
    webtranListBox_->DeleteAllItems();
    WebAppTransactions_.clear();
    mainBoxSizer_->Show(loadingSizer_, true);

    if (mmWebApp::getUrl().empty())
    {
        mainBoxSizer_->Hide(loadingSizer_, true);
        return mmErrorDialogs::ToolTip4Object(url_text_, _t("Empty value"), _t("Error"));
    }
    if (mmWebApp::getGuid().empty())
    {
        mainBoxSizer_->Hide(loadingSizer_, true);
        return mmErrorDialogs::ToolTip4Object(guid_text_, _t("Empty value"), _t("Error"));
    }

    if (!mmWebApp::WebApp_CheckGuid() || !mmWebApp::WebApp_CheckApiVersion())
    {
        mainBoxSizer_->Hide(loadingSizer_, true);
        return;
    }

    wxString CurlError = "";
    if (!mmWebApp::WebApp_DownloadNewTransaction(WebAppTransactions_, false, CurlError))
    {
        mainBoxSizer_->Hide(loadingSizer_, true);
        if (!isStartup_)
        {
            wxString msgStr = wxString() << _t("Unable to download transactions from the WebApp.") << "\n" << CurlError;
            wxMessageBox(msgStr, _t("Transactions download error"), wxICON_ERROR);
        }

        return net_button_->SetBitmap(mmBitmapBundle(png::LED_RED, mmBitmapButtonSize));
    }

    net_button_->SetBitmap(mmBitmapBundle(png::LED_GREEN));

    for (const auto& WebTran : WebAppTransactions_)
    {
        //wxVector<wxVariant> data;
        std::vector<wxVariant> data;
        data.emplace_back(wxString::Format(wxT("%lld"), WebTran.ID)); //WEBTRAN_ID
        data.emplace_back(mmGetDateTimeForDisplay(WebTran.Date.FormatISODate())); //WEBTRAN_DATE
        data.emplace_back(WebTran.Account); //WEBTRAN_ACCOUNT
        data.emplace_back(WebTran.Status); //WEBTRAN_STATUS
        data.emplace_back(wxGetTranslation(WebTran.Type)); //WEBTRAN_TYPE

        wxString Payee = WebTran.Type != "Transfer" ? WebTran.Payee : "> " + WebTran.ToAccount;
        data.emplace_back(Payee); //WEBTRAN_PAYEE

        wxString Category = WebTran.Category;
        if (WebTran.SubCategory != wxEmptyString) Category += ":" + WebTran.SubCategory;
        data.emplace_back(Category); //WEBTRAN_CATEGORY

        Model_Currency::Data *currency = Model_Currency::GetBaseCurrency();
        wxString Amount = Model_Currency::toStringNoFormatting(WebTran.Amount, currency, Model_Currency::precision(currency));
        data.emplace_back(Amount); //WEBTRAN_AMOUNT

        data.emplace_back(WebTran.Notes); //WEBTRAN_NOTES
        data.emplace_back(WebTran.Attachments); //WEBTRAN_ATTACHMENTS

        wxVector<wxVariant> wxData;
        std::copy(data.begin(), data.end(), wxData.begin());
        webtranListBox_->AppendItem(wxData, static_cast<wxUIntPtr>(WebTran.ID.GetValue()));
    }

    if (!WebAppTransactions_.empty())
    {
        wxButton* buttonOK = static_cast<wxButton*>(FindWindow(wxID_OK));
        if (buttonOK) buttonOK->Enable(true);
        wxButton* buttonApply = static_cast<wxButton*>(FindWindow(wxID_APPLY));
        if (buttonApply) buttonApply->Enable(true);
    }
    else if (isStartup_)
    {
        this->Close();
    }

    mainBoxSizer_->Hide(loadingSizer_, true);
}

void mmWebAppDialog::OnTimer(wxTimerEvent& /*event*/)
{
    if (gauge_)
    {
        gauge_->Pulse();
    }

    if (!isFilledOnce_)
    {
        fillControls();
    }
}

void mmWebAppDialog::OnListItemActivated(wxDataViewEvent& event)
{
    wxDataViewItem item = event.GetItem();
    int selected_index = webtranListBox_->ItemToRow(item);

    if (selected_index >= 0)
    {
        int64 WebTrID = static_cast<int64>(webtranListBox_->GetItemData(item));
        mmWebAppDialog::ImportWebTr(WebTrID, true);
        fillControls();
    }
}

bool mmWebAppDialog::ImportWebTr(int64 WebTrID, bool open)
{
    mmWebApp::webtran_holder WebTrToImport;
    bool bFound = false;

    for (const auto &webTr : WebAppTransactions_)
    {
        if (webTr.ID == WebTrID)
        {
            bFound = true;
            WebTrToImport = webTr;
            int64 InsertedTransactionID = mmWebApp::MMEX_InsertNewTransaction(WebTrToImport);
            if (InsertedTransactionID > 0)
            {
                if (open)
                {
                    //fillControls(); //TODO: Delete transaction from view
                    mmTransDialog EditTransactionDialog(this, 1, {InsertedTransactionID, false});
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
        wxString msgStr = wxString() << _t("Unable to insert transaction in MMEX database") << "\n";
        wxMessageBox(msgStr, _t("WebApp communication error"), wxICON_ERROR);
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
            wxString CurlError = "";
            if (mmWebApp::WebApp_DownloadAttachment(AttachmentName, CurlError))
            {
                wxLaunchDefaultApplication(AttachmentName);
                tempFiles_.Add(AttachmentName);
            }
            else
            {
                wxString msgStr = wxString() << _t("Unable to download attachments from the WebApp.") << "\n" << CurlError;
                wxMessageBox(msgStr, _t("Attachment download error"), wxICON_ERROR);
            }
        }
    }
}

void mmWebAppDialog::ImportWebTrSelected(const bool open)
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
            int64 WebTrID = static_cast<int64>(webtranListBox_->GetItemData(Item));
            mmWebAppDialog::ImportWebTr(WebTrID, open);
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
            mmWebApp::WebApp_DeleteOneTransaction(static_cast<int64>(webtranListBox_->GetItemData(Item)));
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
    case MENU_IMPORT_WEBTRAN: ImportWebTrSelected(false); break;
    case MENU_IMPORTOPEN_WEBTRAN: ImportWebTrSelected(true); break;
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

    wxSharedPtr<wxMenu> mainMenu(new wxMenu);
    mainMenu->Append(new wxMenuItem(mainMenu.get(), MENU_OPEN_ATTACHMENT, _t("Open Attachment")));
    mainMenu->Append(new wxMenuItem(mainMenu.get(), MENU_IMPORT_WEBTRAN, _t("Import")));
    mainMenu->Append(new wxMenuItem(mainMenu.get(), MENU_IMPORTOPEN_WEBTRAN, _t("Import and open")));
    mainMenu->Append(new wxMenuItem(mainMenu.get(), MENU_DELETE_WEBTRAN, _t("Delete")));
    if (Selected.size() != 1) mainMenu->Enable(MENU_OPEN_ATTACHMENT, false);

    PopupMenu(mainMenu.get());
    event.Skip();
}


void mmWebAppDialog::ImportAllWebTr(const bool open)
{
    for (int i = 0; i < webtranListBox_->GetItemCount(); i++)
    {
        long long WebTrID;
        webtranListBox_->GetTextValue(i, WEBTRAN_ID).ToLongLong(&WebTrID);
        mmWebAppDialog::ImportWebTr(WebTrID, open);
    }
}

void mmWebAppDialog::OnCancel(wxCommandEvent& /*event*/)
{
    EndModal(wxID_CANCEL);
}

void mmWebAppDialog::OnApply(wxCommandEvent& /*event*/)
{
    mmWebAppDialog::ImportAllWebTr(true);
    EndModal(wxID_APPLY);
}

void mmWebAppDialog::OnOk(wxCommandEvent& /*event*/)
{
    mmWebAppDialog::ImportAllWebTr(false);
    EndModal(wxID_OK);
}
