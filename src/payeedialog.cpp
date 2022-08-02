/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2012 - 2016, 2020 - 2022 Nikolay Akimov
 Copyright (C) 2021, 2022 Mark Whalley (mark@ipx.co.uk)

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

#include "model/allmodel.h"

wxIMPLEMENT_DYNAMIC_CLASS(mmPayeeDialog, wxDialog);

wxBEGIN_EVENT_TABLE(mmPayeeDialog, wxDialog)
    EVT_BUTTON(wxID_CANCEL, mmPayeeDialog::OnCancel)
    EVT_BUTTON(wxID_OK, mmPayeeDialog::OnOk)
    EVT_BUTTON(wxID_APPLY, mmPayeeDialog::OnMagicButton)
    EVT_TEXT(wxID_FIND, mmPayeeDialog::OnTextChanged)
    EVT_LIST_COL_CLICK(wxID_ANY, mmPayeeDialog::OnSort)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY, mmPayeeDialog::OnListItemActivated)
    EVT_LIST_ITEM_SELECTED(wxID_ANY, mmPayeeDialog::OnListItemSelected)
    EVT_LIST_ITEM_DESELECTED(wxID_ANY, mmPayeeDialog::OnListItemDeselected)
    EVT_LIST_ITEM_RIGHT_CLICK(wxID_ANY, mmPayeeDialog::OnItemRightClick)
    EVT_MENU_RANGE(MENU_DEFINE_CATEGORY, MENU_RELOCATE_PAYEE, mmPayeeDialog::OnMenuSelected)
wxEND_EVENT_TABLE()

mmPayeeDialog::~mmPayeeDialog()
{
    Model_Infotable::instance().Set("PAYEES_DIALOG_SIZE", GetSize());
}

mmPayeeDialog::mmPayeeDialog(wxWindow *parent, bool payee_choose, const wxString &name) :
    m_payee_id(-1)
    , m_maskTextCtrl(nullptr)
    , payeeListBox_(nullptr)
    , m_magicButton(nullptr)
    , m_payee_rename(-1)
    , m_payee_choose(payee_choose)
    , refreshRequested_(false)
    , m_sortByPayee (true)
    , m_sortReverse (false)
{
    ColName_[PAYEE_NAME] = _("Name");
    ColName_[PAYEE_CATEGORY]  = (Option::instance().TransCategorySelectionNonTransfer() == Option::LASTUSED) ?
                                _("Last Used Category") : _("Default Category");

    this->SetFont(parent->GetFont());
    Create(parent, name);

    const wxAcceleratorEntry entries[] =
    {
        wxAcceleratorEntry(wxACCEL_NORMAL, WXK_F2, MENU_EDIT_PAYEE),
        wxAcceleratorEntry(wxACCEL_NORMAL, WXK_INSERT, MENU_NEW_PAYEE),
        wxAcceleratorEntry(wxACCEL_NORMAL, WXK_DELETE, MENU_DELETE_PAYEE)
    };

    wxAcceleratorTable tab(sizeof(entries) / sizeof(*entries), entries);
    SetAcceleratorTable(tab);
}

int mmPayeeDialog::FindSelectedPayee()
{
    int sel = payeeListBox_->GetFocusedItem();
    if (-1 != sel)
    {
        wxListItem item;
        item.SetId(sel);
        payeeListBox_->GetItem(item);
        return (item.GetData());
    } else
        return -1;
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
    mmSetSize(this);
    Centre();
}

void mmPayeeDialog::CreateControls()
{
    wxBoxSizer* mainBoxSizer = new wxBoxSizer(wxVERTICAL);

    payeeListBox_ = new wxListView(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 
        wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_AUTOARRANGE);
    payeeListBox_->SetMinSize(wxSize(250, 100));

    wxListItem col0, col1;
    // Add first column
    col0.SetId(0);
    col0.SetText(ColName_[PAYEE_NAME]);
    col0.SetWidth(150);
    payeeListBox_->InsertColumn(0, col0);

    // Add second column
    col1.SetId(1);
    col1.SetText(ColName_[PAYEE_CATEGORY]);
    col1.SetWidth(250);
    payeeListBox_->InsertColumn(1, col1);

    mainBoxSizer->Add(payeeListBox_, wxSizerFlags(g_flagsExpand).Border(wxALL, 10));

    wxPanel* buttons_panel = new wxPanel(this, wxID_ANY);
    mainBoxSizer->Add(buttons_panel, wxSizerFlags(g_flagsExpand).Proportion(0));
    wxBoxSizer*  tools_sizer = new wxBoxSizer(wxVERTICAL);
    buttons_panel->SetSizer(tools_sizer);

    wxBoxSizer* tools_sizer2 = new wxBoxSizer(wxHORIZONTAL);
    tools_sizer->Add(tools_sizer2, wxSizerFlags(g_flagsExpand).Border(0));

    m_magicButton = new wxBitmapButton(buttons_panel
        , wxID_APPLY, mmBitmap(png::MORE_OPTIONS, mmBitmapButtonSize));
    mmToolTip(m_magicButton, _("Other tools"));
    tools_sizer2->Add(m_magicButton, g_flagsH);

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
    this->Freeze();
    payeeListBox_->DeleteAllItems();
    m_payee_id = -1;
    

    Model_Payee::Data_Set payees = Model_Payee::instance().FilterPayees(m_maskStr);
    if (m_sortByPayee)
        std::stable_sort(payees.begin(), payees.end(), SorterByPAYEENAME());
    else
    {
        // Need to compute here as the payee data set does not have the full name
        std::stable_sort(payees.begin(), payees.end(), [] (Model_Payee::Data x, Model_Payee::Data y)
        {
            return(
                CaseInsensitiveLocaleCmp(
                    Model_Category::instance().full_name(x.CATEGID, x.SUBCATEGID)
                    , Model_Category::instance().full_name(y.CATEGID, y.SUBCATEGID)) < 0
            ); 
        });
    }
    if (m_sortReverse)
        std::reverse(payees.begin(), payees.end());

    int idx = 0;
    for (const auto& payee : payees)
    {
        wxListItem item;
        item.SetId(idx);
        item.SetData(payee.PAYEEID);
        payeeListBox_->InsertItem(item);
        const wxString full_category_name = Model_Category::instance().full_name(payee.CATEGID, payee.SUBCATEGID);
        payeeListBox_->SetItem(idx, 0, payee.PAYEENAME);
        payeeListBox_->SetItem(idx, 1, full_category_name);
        idx++;
    }
    this->Thaw();
}

void mmPayeeDialog::OnListItemSelected(wxListEvent& WXUNUSED(event))
{
    m_payee_id = FindSelectedPayee();
}

void mmPayeeDialog::OnListItemActivated(wxListEvent& WXUNUSED(event))
{
    m_payee_id = FindSelectedPayee();
    if (m_payee_choose)
        EndModal(wxID_OK);
    else
        EditPayee();
}
void mmPayeeDialog::OnTextChanged(wxCommandEvent& event)
{
    m_maskStr = event.GetString().Prepend("*");
    fillControls();
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
        if (payees.empty() || name.CmpNoCase(payee->PAYEENAME) == 0)
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
            mmAttachmentManage::DeleteAllAttachments(Model_Attachment::reftype_desc(Model_Attachment::PAYEE), m_payee_id);

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
        mmCategDialog dlg(this, true, payee->CATEGID, payee->SUBCATEGID);
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

void mmPayeeDialog::RemoveDefaultCategory()
{
    Model_Payee::Data *payee = Model_Payee::instance().get(m_payee_id);
    if (payee)
    {
        payee->CATEGID = -1;
        payee->SUBCATEGID = -1;
        refreshRequested_ = true;
        Model_Payee::instance().save(payee);
        mmWebApp::MMEX_WebApp_UpdatePayee();
        fillControls();
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
    }
}

void mmPayeeDialog::OnMenuSelected(wxCommandEvent& event)
{
    switch(event.GetId())
    {
        case MENU_DEFINE_CATEGORY: DefineDefaultCategory() ; break;
        case MENU_REMOVE_CATEGORY: RemoveDefaultCategory() ; break;
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
    wxListEvent evt;
    OnItemRightClick(evt);
}

void mmPayeeDialog::OnItemRightClick(wxListEvent& event)
{
    if (!m_magicButton->IsEnabled()) return;

    m_payee_id = FindSelectedPayee();

    wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, wxID_ANY) ;
    evt.SetEventObject(this);

    Model_Payee::Data* payee = Model_Payee::instance().get(m_payee_id);

    wxMenu mainMenu;
    if (payee) mainMenu.SetTitle(payee->PAYEENAME);
    mainMenu.Append(new wxMenuItem(&mainMenu, MENU_DEFINE_CATEGORY, _("Define Category")));
    if (!payee) mainMenu.Enable(MENU_DEFINE_CATEGORY, false);
    mainMenu.Append(new wxMenuItem(&mainMenu, MENU_REMOVE_CATEGORY, _("Remove Category")));
    if (!payee) mainMenu.Enable(MENU_REMOVE_CATEGORY, false);
    mainMenu.AppendSeparator();

    mainMenu.Append(new wxMenuItem(&mainMenu, MENU_NEW_PAYEE, _("&Add ")));
    mainMenu.Append(new wxMenuItem(&mainMenu, MENU_EDIT_PAYEE, _("&Edit ")));
    if (!payee) mainMenu.Enable(MENU_EDIT_PAYEE, false);
    mainMenu.Append(new wxMenuItem(&mainMenu, MENU_DELETE_PAYEE, _("&Remove ")));
    if (!payee || Model_Payee::is_used(m_payee_id)) mainMenu.Enable(MENU_DELETE_PAYEE, false);
    mainMenu.AppendSeparator();

    mainMenu.Append(new wxMenuItem(&mainMenu, MENU_ORGANIZE_ATTACHMENTS, _("&Organize Attachments")));
    if (!payee) mainMenu.Enable(MENU_ORGANIZE_ATTACHMENTS, false);
    mainMenu.AppendSeparator();

    mainMenu.Append(new wxMenuItem(&mainMenu, MENU_RELOCATE_PAYEE, _("Relocate Payee")));
    //SetToolTip(_("Change all transactions using one Payee to another Payee"));
    if (!payee) mainMenu.Enable(MENU_RELOCATE_PAYEE, false);

    PopupMenu(&mainMenu);
    event.Skip();
}

void mmPayeeDialog::OnSort(wxListEvent& event)
{
    if (0 == event.GetColumn())
    {
        if (m_sortByPayee)
            m_sortReverse = !m_sortReverse;
        m_sortByPayee = true;
    } else
    {
        if (!m_sortByPayee)
            m_sortReverse = !m_sortReverse;
        m_sortByPayee = false;
    }
    fillControls();
}

void mmPayeeDialog::OnCancel(wxCommandEvent& /*event*/)
{
    m_payee_id = -1;
    EndModal(wxID_CANCEL);
}

void mmPayeeDialog::OnOk(wxCommandEvent& /*event*/)
{
    if (payeeListBox_->GetItemCount() < 1)
        AddPayee();
    else
    {
        m_payee_id = FindSelectedPayee();
        EndModal(wxID_OK);
    }
}
