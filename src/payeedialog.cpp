/*******************************************************
Copyright (C) 2006 Madhan Kanagavel
Copyright (C) 2012 Nikolay Akimov

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

#include "payeedialog.h"
#include "attachmentdialog.h"
#include "images_list.h"
#include "categdialog.h"
#include "constants.h"
#include "option.h"
#include "paths.h"
#include "relocatepayeedialog.h"
#include "util.h"
#include "webapp.h"
#include <wx/srchctrl.h>

#include "Model_Payee.h"
#include "Model_Attachment.h"

wxIMPLEMENT_DYNAMIC_CLASS(mmPayeeDialog, wxDialog);

wxBEGIN_EVENT_TABLE(mmPayeeDialog, wxDialog)
    EVT_BUTTON(wxID_CANCEL, mmPayeeDialog::OnCancel)
    EVT_BUTTON(wxID_OK, mmPayeeDialog::OnOk)
    EVT_BUTTON(wxID_APPLY, mmPayeeDialog::OnMagicButton)
    EVT_TEXT(wxID_ANY, mmPayeeDialog::OnTextChanged)
    EVT_DATAVIEW_ITEM_VALUE_CHANGED(wxID_ANY, mmPayeeDialog::OnDataChanged)
    EVT_DATAVIEW_ITEM_EDITING_STARTED(wxID_ANY, mmPayeeDialog::OnDataEditStart)
    EVT_DATAVIEW_SELECTION_CHANGED(wxID_ANY, mmPayeeDialog::OnListItemSelected)
    EVT_DATAVIEW_ITEM_ACTIVATED(wxID_ANY, mmPayeeDialog::OnListItemActivated)
    EVT_DATAVIEW_ITEM_CONTEXT_MENU(wxID_ANY, mmPayeeDialog::OnItemRightClick)
    EVT_MENU_RANGE(MENU_DEFINE_CATEGORY, MENU_RELOCATE_PAYEE, mmPayeeDialog::OnMenuSelected)
wxEND_EVENT_TABLE()


mmPayeeDialog::mmPayeeDialog(wxWindow *parent, bool payee_choose, const wxString &name) :
    payeeListBox_()
    , m_maskTextCtrl()
    , m_payee_id(-1)
    , m_payee_rename(-1)
    , m_payee_choose(payee_choose)
    , refreshRequested_(false)
#ifdef _DEBUG
    , debug_(true)
#else
    , debug_(false)
#endif
{
    if (debug_) ColName_[PAYEE_ID] = _("#");
    ColName_[PAYEE_NAME] = _("Name");
    ColName_[PAYEE_CATEGORY]   = _("Default Category");

    Create(parent, name);
}

void mmPayeeDialog::Create(wxWindow* parent, const wxString &name)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);

    long style = wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER;
    if (!wxDialog::Create(parent, wxID_ANY, _("Organize Payees")
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

void mmPayeeDialog::CreateControls()
{
    wxBoxSizer* mainBoxSizer = new wxBoxSizer(wxVERTICAL);

    //TODO:provide proper style
    payeeListBox_ = new wxDataViewListCtrl( this
        , wxID_ANY, wxDefaultPosition, wxSize(450, 500)/*, wxDV_HORIZ_RULES*/);

    if (debug_) payeeListBox_->AppendTextColumn(ColName_[PAYEE_ID], wxDATAVIEW_CELL_INERT, 30);
    payeeListBox_->AppendTextColumn(ColName_[PAYEE_NAME], wxDATAVIEW_CELL_EDITABLE, 150);
    payeeListBox_->AppendTextColumn(ColName_[PAYEE_CATEGORY], wxDATAVIEW_CELL_INERT, 250);
    mainBoxSizer->Add(payeeListBox_, wxSizerFlags(g_flagsExpand).Border(wxALL, 10));

    wxPanel* buttons_panel = new wxPanel(this, wxID_ANY);
    mainBoxSizer->Add(buttons_panel, wxSizerFlags(g_flagsExpand).Proportion(0));
    wxBoxSizer*  tools_sizer = new wxBoxSizer(wxVERTICAL);
    buttons_panel->SetSizer(tools_sizer);

    wxBoxSizer* tools_sizer2 = new wxBoxSizer(wxHORIZONTAL);
    tools_sizer->Add(tools_sizer2, wxSizerFlags(g_flagsExpand).Border(0));

    wxBitmapButton* magicButton = new wxBitmapButton(buttons_panel
        , wxID_APPLY, mmBitmap(png::RUN));
    magicButton->SetToolTip(_("Other tools"));
    tools_sizer2->Add(magicButton, g_flagsH);

    m_maskTextCtrl = new wxSearchCtrl(buttons_panel, wxID_FIND);
    m_maskTextCtrl->SetFocus();
    tools_sizer2->Prepend(m_maskTextCtrl, g_flagsExpand);
    tools_sizer2->Prepend(new wxStaticText(buttons_panel, wxID_STATIC, _("Search:")), g_flagsH);

    wxStdDialogButtonSizer*  buttons_sizer = new wxStdDialogButtonSizer;
    tools_sizer->Add(buttons_sizer, wxSizerFlags(g_flagsV).Center());
    wxButton* buttonOK = new wxButton(buttons_panel, wxID_OK, _("&OK "));
    wxButton* btnCancel = new wxButton(buttons_panel, wxID_CANCEL, wxGetTranslation(g_CancelLabel));

    buttons_sizer->Add(buttonOK, g_flagsH);
    buttons_sizer->Add(btnCancel, g_flagsH);

    this->SetSizer(mainBoxSizer);
}

void mmPayeeDialog::fillControls()
{
    int firstInTheListPayeeID = -1;
    payeeListBox_->DeleteAllItems();

    for (const auto& payee : Model_Payee::instance().FilterPayees(m_maskStr))
    {
        const wxString full_category_name = Model_Category::instance().full_name(payee.CATEGID, payee.SUBCATEGID);
        if (firstInTheListPayeeID == -1) { firstInTheListPayeeID = payee.PAYEEID; }
        wxVector<wxVariant> data;
        if (debug_) data.push_back(wxVariant(wxString::Format("%i", payee.PAYEEID)));
        data.push_back(wxVariant(payee.PAYEENAME));
        data.push_back(wxVariant(full_category_name));
        payeeListBox_->AppendItem(data, (wxUIntPtr)payee.PAYEEID);
    }
    m_payee_id = firstInTheListPayeeID;
}

void mmPayeeDialog::OnDataEditStart(wxDataViewEvent& WXUNUSED(event))
{
    m_payee_rename = m_payee_id;
}

void mmPayeeDialog::OnDataChanged(wxDataViewEvent& event)
{
    int row = payeeListBox_->ItemToRow(event.GetItem());
    wxVariant var;
    payeeListBox_->GetValue(var, row, event.GetColumn());
    const wxString value = var.GetString();

    auto payee = Model_Payee::instance().get(m_payee_rename);
    if (!payee || value == payee->PAYEENAME) return;

    const auto payees = Model_Payee::instance().find(Model_Payee::PAYEENAME(value));
    if (payees.empty())
    {
        payee->PAYEENAME = value;
        Model_Payee::instance().save(payee);
        mmWebApp::MMEX_WebApp_UpdatePayee();
        refreshRequested_ = true;
    }
    else
    {
        wxMessageBox(_("Payee with same name exists")
            , _("Organize Payees: Add Payee"), wxOK | wxICON_ERROR);
    }
    fillControls();
}

void mmPayeeDialog::OnListItemSelected(wxDataViewEvent& event)
{
    wxDataViewItem item = event.GetItem();
    int selected_index = payeeListBox_->ItemToRow(item);

    if (selected_index >= 0)
        m_payee_id = (int)payeeListBox_->GetItemData(item);
}

void mmPayeeDialog::OnListItemActivated(wxDataViewEvent& WXUNUSED(event))
{
    if (m_payee_id > 0 && m_payee_choose)
        EndModal(wxID_OK);
}

void mmPayeeDialog::AddPayee()
{
    const wxString name = wxGetTextFromUser(_("Enter the name for the new payee:")
        , _("Organize Payees: Add Payee"), m_maskTextCtrl->GetValue());
    if (name.IsEmpty()) return;

    Model_Payee::Data_Set payees = Model_Payee::instance().find(Model_Payee::PAYEENAME(name));
    if (payees.empty())
    {
        Model_Payee::Data *payee = Model_Payee::instance().create();
        payee->PAYEENAME = name;
        m_payee_id = Model_Payee::instance().save(payee);
        mmWebApp::MMEX_WebApp_UpdatePayee();
        m_payee_id = payee->PAYEEID;
        refreshRequested_ = true;
    }
    else
    {
        wxMessageBox(_("Payee with same name exists"), _("Organize Payees: Add Payee"), wxOK | wxICON_ERROR);
    }

    fillControls();
}

void mmPayeeDialog::EditPayee()
{
    Model_Payee::Data *payee = Model_Payee::instance().get(m_payee_id);
    if (payee)
    {
        const wxString name = wxGetTextFromUser(_("Modify the name for the payee:")
            , _("Organize Payees: Edit Payee"), payee->PAYEENAME);
        if (name.IsEmpty()) return;

        if (name == payee->PAYEENAME) return;

        Model_Payee::Data_Set payees = Model_Payee::instance().find(Model_Payee::PAYEENAME(name));
        if (payees.empty())
        {
            payee->PAYEENAME = name;
            m_payee_id = Model_Payee::instance().save(payee);
            mmWebApp::MMEX_WebApp_UpdatePayee();
            m_payee_id = payee->PAYEEID;
            refreshRequested_ = true;
        }
        else
        {
            wxMessageBox(_("Payee with same name exists"), _("Organize Payees: Edit Payee"), wxOK | wxICON_ERROR);
        }

        fillControls();
    }
}

void mmPayeeDialog::DeletePayee()
{
    const auto *payee = Model_Payee::instance().get(m_payee_id);
    if (payee)
    {
        if (!Model_Payee::instance().remove(m_payee_id))
        {
            wxString deletePayeeErrMsg = _("Payee in use.");
            deletePayeeErrMsg
                << "\n\n"
                << _("Tip: Change all transactions using this Payee to another Payee"
                    " using the relocate command:")
                << "\n\n" << _("Tools -> Relocation of -> Payees");
            wxMessageBox(deletePayeeErrMsg, _("Organize Payees: Delete Error"), wxOK | wxICON_ERROR);
            return;
        }
        else
        {
            mmAttachmentManage::DeleteAllAttachments(Model_Attachment::reftype_desc(Model_Attachment::PAYEE), m_payee_id);
            mmWebApp::MMEX_WebApp_UpdatePayee();
        }

        m_payee_id = -1;
        refreshRequested_ = true;
        fillControls();
    }
}

void mmPayeeDialog::DefineDefaultCategory()
{
    Model_Payee::Data *payee = Model_Payee::instance().get(m_payee_id);
    if (payee)
    {
        mmCategDialog dlg(this, payee->CATEGID, payee->SUBCATEGID, false);
        if (dlg.ShowModal() == wxID_OK)
        {
            payee->CATEGID = dlg.getCategId();
            payee->SUBCATEGID = dlg.getSubCategId();
            refreshRequested_ = true;
            Model_Payee::instance().save(payee);
            mmWebApp::MMEX_WebApp_UpdatePayee();
            fillControls();
        }
    }
}

void mmPayeeDialog::OnOrganizeAttachments()
{
    wxString RefType = Model_Attachment::reftype_desc(Model_Attachment::PAYEE);

    mmAttachmentDialog dlg(this, RefType, m_payee_id);
    dlg.ShowModal();
    refreshRequested_ = true;
}

void mmPayeeDialog::OnPayeeRelocate()
{
    relocatePayeeDialog dlg(this, m_payee_id);

    if (dlg.ShowModal() == wxID_OK)
    {
        wxString msgStr;
        msgStr << _("Payee Relocation Completed.") << "\n\n"
            << wxString::Format(_("Records have been updated in the database: %i")
                , dlg.updatedPayeesCount())
            << "\n\n";
        wxMessageBox(msgStr, _("Payee Relocation Result"));
        refreshRequested_ = true;
        fillControls(); // payee can be deleted
    }
}

void mmPayeeDialog::OnTextChanged(wxCommandEvent& event)
{
    m_maskStr = event.GetString();
    fillControls();
}

void mmPayeeDialog::OnMenuSelected(wxCommandEvent& event)
{
    wxCommandEvent evt;

    switch(event.GetId())
    {
        case MENU_DEFINE_CATEGORY: DefineDefaultCategory() ; break;
        case MENU_NEW_PAYEE: AddPayee(); break;
        case MENU_EDIT_PAYEE: EditPayee(); break;
        case MENU_DELETE_PAYEE: DeletePayee(); break;
        case MENU_ORGANIZE_ATTACHMENTS: OnOrganizeAttachments(); break;
        case MENU_RELOCATE_PAYEE: OnPayeeRelocate(); break;
        default: break;
    }
}

void mmPayeeDialog::OnMagicButton(wxCommandEvent& WXUNUSED(event))
{
    wxDataViewEvent evt;
    OnItemRightClick(evt);
}

void mmPayeeDialog::OnItemRightClick(wxDataViewEvent& event)
{
    wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, wxID_ANY) ;
    evt.SetEventObject(this);

    Model_Payee::Data* payee = Model_Payee::instance().get(m_payee_id);

    wxMenu* mainMenu = new wxMenu;
    if (payee) mainMenu->SetTitle(payee->PAYEENAME);
    mainMenu->Append(new wxMenuItem(mainMenu, MENU_DEFINE_CATEGORY, _("Define Default Category")));
    if (!payee) mainMenu->Enable(MENU_DEFINE_CATEGORY, false);
    mainMenu->AppendSeparator();

    mainMenu->Append(new wxMenuItem(mainMenu, MENU_NEW_PAYEE, _("&Add ")));
    mainMenu->Append(new wxMenuItem(mainMenu, MENU_EDIT_PAYEE, _("&Edit ")));
    if (!payee) mainMenu->Enable(MENU_EDIT_PAYEE, false);
    mainMenu->Append(new wxMenuItem(mainMenu, MENU_DELETE_PAYEE, _("&Remove ")));
    if (!payee || Model_Payee::is_used(m_payee_id)) mainMenu->Enable(MENU_DELETE_PAYEE, false);
    mainMenu->AppendSeparator();

    mainMenu->Append(new wxMenuItem(mainMenu, MENU_ORGANIZE_ATTACHMENTS, _("&Organize Attachments")));
    if (!payee) mainMenu->Enable(MENU_ORGANIZE_ATTACHMENTS, false);
    mainMenu->AppendSeparator();

    mainMenu->Append(new wxMenuItem(mainMenu, MENU_RELOCATE_PAYEE, _("Relocate Payee")));
    //SetToolTip(_("Change all transactions using one Payee to another Payee"));
    if (!payee || payeeListBox_->GetItemCount() < 2
        || !Model_Payee::is_used(m_payee_id))
            mainMenu->Enable(MENU_RELOCATE_PAYEE, false);

    PopupMenu(mainMenu);
    delete mainMenu;
    event.Skip();
}

void mmPayeeDialog::OnCancel(wxCommandEvent& WXUNUSED(event))
{
    EndModal(wxID_CANCEL);
}

void mmPayeeDialog::OnOk(wxCommandEvent& WXUNUSED(event))
{
    EndModal(wxID_OK);
}
