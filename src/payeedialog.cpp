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

#include "payeedialog.h"
#include "relocatepayeedialog.h"
#include "util.h"
#include "mmOption.h"
#include "paths.h"

IMPLEMENT_DYNAMIC_CLASS( mmPayeeDialog, wxDialog )

BEGIN_EVENT_TABLE( mmPayeeDialog, wxDialog )
    EVT_BUTTON(wxID_ADD, mmPayeeDialog::OnAdd)
    EVT_BUTTON(wxID_REMOVE, mmPayeeDialog::OnDelete)
    EVT_BUTTON(wxID_OK, mmPayeeDialog::OnBSelect)
    EVT_BUTTON(wxID_CANCEL, mmPayeeDialog::OnCancel)
    EVT_BUTTON(wxID_EDIT, mmPayeeDialog::OnEdit)
    EVT_BUTTON(wxID_SAVE, mmPayeeDialog::saveFilterSettings)
    EVT_LISTBOX(wxID_ANY, mmPayeeDialog::OnSelChanged)
    EVT_LISTBOX_DCLICK(wxID_ANY, mmPayeeDialog::OnDoubleClicked)
    EVT_TEXT(wxID_ANY, mmPayeeDialog::OnTextCtrlChanged)
    EVT_TEXT_ENTER(wxID_ANY, mmPayeeDialog::OnBSelect)
    EVT_CHAR_HOOK(mmPayeeDialog::OnListKeyDown)
END_EVENT_TABLE()


mmPayeeDialog::mmPayeeDialog(wxWindow *parent, mmCoreDB *core, bool showSelectButton) :
    m_payee_id_(-1),
    core_(core),
    showSelectButton_(showSelectButton)
{
    do_create(parent);
}

void mmPayeeDialog::do_create(wxWindow* parent)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);

    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX | wxRESIZE_BORDER | wxWANTS_CHARS;
    if (!wxDialog::Create(parent, wxID_ANY, _("Organize Payees"), wxDefaultPosition, wxDefaultSize, style)) {
        return;
    }

    CreateControls();

    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    if (!showSelectButton_)
    {
        selectButton_->Disable();
    }

    SetIcon(mmex::getProgramIcon());

    wxString sResult = core_->dbInfoSettings_->GetStringSetting("HIDDEN_PAYEES_STRING", "");
    hideTextCtrl_->ChangeValue(sResult);

    fillControls();

    Centre();
}

void mmPayeeDialog::CreateControls()
{
    int border = 5;
    wxSizerFlags flags, flagsExpand;
    flags.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).Border(wxALL, border);
    flagsExpand.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).Border(wxALL, border).Expand();

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer22 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer22);
    wxBitmapButton* payee_relocate = new wxBitmapButton(this,
        wxID_STATIC, wxBitmap(relocate_payees_xpm));
    itemBoxSizer22->Add(payee_relocate, flags);
    payee_relocate->Connect(wxID_STATIC, wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(mmPayeeDialog::OnPayeeRelocate), NULL, this);
    payee_relocate->SetToolTip(_("Change all transactions using one Payee to another Payee"));
    itemBoxSizer22->AddSpacer(10);

    cbShowAll_ = new wxCheckBox(this, wxID_SELECTALL, _("Show All"), wxDefaultPosition,
        wxDefaultSize, wxCHK_2STATE);
    cbShowAll_->SetToolTip(_("Show all hidden payees"));
    itemBoxSizer22->Add(cbShowAll_, flags);
    cbShowAll_->Connect(wxID_SELECTALL, wxEVT_COMMAND_CHECKBOX_CLICKED,
        wxCommandEventHandler(mmPayeeDialog::OnShowHiddenChbClick), NULL, this);

    wxArrayString filtd = core_->payeeList_.FilterPayees("");
    int vertical_size_ = (filtd.GetCount()>10 ? 320 : 240);
    listBox_ = new wxListBox( this, wxID_ANY,
       wxDefaultPosition, wxSize(100, vertical_size_), wxArrayString(), wxLB_SINGLE);
    itemBoxSizer2->Add(listBox_, 1, wxEXPAND|wxALL, 1);

    wxNotebook* payee_notebook = new wxNotebook(this,
        wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_MULTILINE );
    wxPanel* filter_tab = new wxPanel(payee_notebook, wxID_ANY);
    payee_notebook->AddPage(filter_tab, _("Filter"));
    wxBoxSizer *notes_sizer = new wxBoxSizer(wxVERTICAL);
    filter_tab->SetSizer(notes_sizer);
    itemBoxSizer2->Add(payee_notebook, flagsExpand);

    textCtrl_ = new wxTextCtrl( filter_tab, wxID_FIND, "",
        wxDefaultPosition, wxSize(240,-1), wxTE_PROCESS_ENTER);
    notes_sizer->Add(textCtrl_, flagsExpand);
    textCtrl_->SetFocus();

    wxPanel* hide_tab = new wxPanel(payee_notebook, wxID_ANY);
    payee_notebook->AddPage(hide_tab, _("Hide"));
    wxBoxSizer *others_sizer = new wxBoxSizer(wxHORIZONTAL);
    hide_tab->SetSizer(others_sizer);

    hideTextCtrl_ = new wxTextCtrl( hide_tab, wxID_ANY, "",
        wxDefaultPosition, wxDefaultSize);
    others_sizer->Add(hideTextCtrl_, 1, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxBitmapButton* save_button = new wxBitmapButton( hide_tab,
        wxID_SAVE, wxNullBitmap, wxDefaultPosition,
        wxSize(hideTextCtrl_->GetSize().GetHeight(), hideTextCtrl_->GetSize().GetHeight()));
    save_button->Show(true);
    save_button->SetBitmapLabel(save_xpm);
    others_sizer->Add(save_button, flags);

    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer9, flagsExpand);

    wxPanel* maintenance_tab = new wxPanel(payee_notebook, wxID_ANY);
    payee_notebook->AddPage(maintenance_tab, _("Tools"));
    wxBoxSizer *maintenance_sizer = new wxBoxSizer(wxHORIZONTAL);
    maintenance_tab->SetSizer(maintenance_sizer);

    addButton_ = new wxButton( maintenance_tab, wxID_ADD);
    editButton_ = new wxButton( maintenance_tab, wxID_EDIT);
    editButton_->Disable();
    deleteButton_ = new wxButton( maintenance_tab, wxID_REMOVE);
    deleteButton_->Disable();

    maintenance_sizer->Add(addButton_, flags.Border(wxALL, 1));
    maintenance_sizer->Add(editButton_, flags);
    maintenance_sizer->Add(deleteButton_, flags);

    selectButton_ = new wxButton( this, wxID_OK, _("&Select"));
    itemBoxSizer9->Add(selectButton_, 1, wxALIGN_CENTER_VERTICAL|wxALL, 1);
    selectButton_->Disable();

    //Some interfaces has no any close buttons, it may confuse user. Cancel button added
    btnCancel_ = new wxButton( this, wxID_CANCEL);
    itemBoxSizer9->Add(btnCancel_,  flags);

    textCtrl_->SetToolTip(_("Enter a search string.  You can use * as a wildcard to match zero or more characters or ? to match a single character."));
    addButton_->SetToolTip(_("Add a new payee name"));
    editButton_->SetToolTip(_("Change the name of an existing payee"));
    deleteButton_->SetToolTip(_("Delete the selected payee. The payee cannot be used by an existing transaction."));
    selectButton_->SetToolTip(_("Select the currently selected payee as the selected payee for the transaction"));
}

void mmPayeeDialog::OnListKeyDown(wxKeyEvent &event)
{
    long keycode = event.GetKeyCode();

    // Check to see if the up or down arrow is pressed while text control has focus
    wxWindow *win = wxWindow::FindFocus();
    wxTextCtrl *text = wxDynamicCast(win, wxTextCtrl);
    // if text, then text control has focus
    if ( text && (keycode == WXK_DOWN) && ( listBox_->GetSelection() < (int)listBox_->GetCount()-1) )
    {
        listBox_->Select(listBox_->GetSelection()+1);
        wxCommandEvent event;
        OnSelChanged(event);
    }
    else if ( text && (keycode == WXK_UP) && ( listBox_->GetSelection() > 0) )
    {
        listBox_->Select(listBox_->GetSelection()-1);
        wxCommandEvent event;
        OnSelChanged(event);
    }
    else
    {
        // text control doesn't have focus OR up/down not pressed OR up/down not possible
    }

    event.Skip();
}

void mmPayeeDialog::fillControls()
{
    bool bResult = core_->dbInfoSettings_->GetBoolSetting("SHOW_HIDDEN_PAYEES", true);
    cbShowAll_->SetValue(bResult);

    wxArrayString filtd = core_->payeeList_.FilterPayees(textCtrl_->GetValue());

    listBox_->Clear();

    for (size_t i = 0; i < filtd.GetCount(); ++i) {
        bool bHideItem = filtd.Item(i).Matches(hideTextCtrl_->GetValue().Append("*"))
            && !hideTextCtrl_->GetValue().IsEmpty();
        if (cbShowAll_->IsChecked() || !bHideItem)
        {
            listBox_->Append(filtd.Item(i));
        }
    }
}

void mmPayeeDialog::OnTextCtrlChanged(wxCommandEvent& event)
{
    wxString payee = listBox_->GetStringSelection();
    fillControls();

    if (!listBox_->IsEmpty() && !listBox_->SetStringSelection(payee)) {
        listBox_->Select(0);
    }

    OnSelChanged(event);
}

void mmPayeeDialog::OnSelChanged(wxCommandEvent& /*event*/)
{
    wxString payee = listBox_->GetStringSelection();
    m_payee_id_ = payee.IsEmpty() ? -1 : core_->payeeList_.GetPayeeId(payee);
    bool ok = m_payee_id_ > -1;

    editButton_->Enable(ok);
    deleteButton_->Enable(ok && !core_->bTransactionList_.IsPayeeUsed(m_payee_id_));
    selectButton_->Enable(ok && showSelectButton_);
}

void mmPayeeDialog::OnAdd(wxCommandEvent& event)
{
    wxString text = wxGetTextFromUser(_("Enter the name for the new payee:")
        , _("Organize Payees: Add Payee"), textCtrl_->GetValue());
    if (text.IsEmpty()) {
        return;
    }

    if (core_->payeeList_.PayeeExists(text))
    {
        wxMessageBox(_("Payee with same name exists"), _("Organize Payees: Add Payee"), wxOK|wxICON_ERROR);
    }
    else
    {
        int payeeID = core_->payeeList_.AddPayee(text);
		if (payeeID < 0) return;
        wxASSERT(payeeID > 0);

        textCtrl_->Clear();
        fillControls();

        listBox_->SetStringSelection(text);

        // SetStringSelection does not emit event, so we need to do it manually.
        // This is important because it is where m_payee_id_ gets set
        OnSelChanged(event);
    }
}

void mmPayeeDialog::OnDelete(wxCommandEvent& event)
{
    if (!core_->payeeList_.RemovePayee(m_payee_id_))
    {
        wxString deletePayeeErrMsg = _("Payee in use.");
        deletePayeeErrMsg
            << "\n\n"
            << _("Tip: Change all transactions using this Payee to another Payee\nusing the relocate command:")
            << "\n\n" << _("Tools -> Relocation of -> Payees");
        wxMessageBox(deletePayeeErrMsg,_("Organize Payees: Delete Error"),wxOK|wxICON_ERROR);
        return;
    }
    textCtrl_->Clear();
    fillControls();
    if (!listBox_->IsEmpty()) {
        listBox_->Select(0);
    }
    OnSelChanged(event);
}

void mmPayeeDialog::OnBSelect(wxCommandEvent& /*event*/)
{
    if (m_payee_id_ != -1) {
        EndModal(wxID_OK);
    }
}

void mmPayeeDialog::OnDoubleClicked(wxCommandEvent& event)
{
    if (showSelectButton_)
        OnBSelect(event);
    else
        OnEdit(event);

}

void mmPayeeDialog::OnEdit(wxCommandEvent& event)
{
    wxString old_name = listBox_->GetStringSelection();

    wxString mesg;
    mesg.Printf(_("Enter a new name for %s"), old_name);
    wxString new_name = wxGetTextFromUser(mesg, _("Edit Payee Name"), old_name);
    if (new_name != wxGetEmptyString())
    {
        core_->payeeList_.UpdatePayee(m_payee_id_, new_name);
        editButton_->Disable();
        fillControls();

        // Now we need to make sure that the edited name is selected after the dialog is closed
        textCtrl_->Clear();
        listBox_->SetStringSelection(new_name);
        OnSelChanged(event);
    }
}

void mmPayeeDialog::OnPayeeRelocate(wxCommandEvent& /*event*/)
{
    relocatePayeeDialog* dlg = new relocatePayeeDialog(core_, this);
    if (dlg->ShowModal() == wxID_OK)
    {
        wxString msgStr;
        msgStr << _("Payee Relocation Completed.") << "\n\n"
            << wxString::Format(_("Records have been updated in the database: %s"),
                dlg->updatedPayeesCount())
            << "\n\n";
        wxMessageBox(msgStr, _("Payee Relocation Result"));
        mmOptions::instance().databaseUpdated_ = true;
    }
}

void mmPayeeDialog::OnShowHiddenChbClick(wxCommandEvent& /*event*/)
{
    core_->dbInfoSettings_->SetSetting("SHOW_HIDDEN_PAYEES", cbShowAll_->IsChecked());
    fillControls();
}

void mmPayeeDialog::OnCancel(wxCommandEvent& /*event*/)
{
    wxWindow *w = FindFocus();

    if (w && !textCtrl_->GetValue().IsEmpty() && w->GetId() == wxID_FIND)
    {
        textCtrl_->Clear();
        return;
    }
    else
        EndModal(wxID_CANCEL);
}

void mmPayeeDialog::saveFilterSettings(wxCommandEvent& event)
{
    core_->dbInfoSettings_->SetSetting("HIDDEN_PAYEES_STRING", hideTextCtrl_->GetValue());
    event.Skip();
}
