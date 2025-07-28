/*******************************************************
 Copyright (C) 2025 Klaus Wich

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

#include "daterangedialog.h"
#include "daterangeeditdialog.h"
#include "constants.h"
#include "images_list.h"
#include "option.h"


wxIMPLEMENT_DYNAMIC_CLASS(mmDateRangeDialog, wxDialog);

wxBEGIN_EVENT_TABLE(mmDateRangeDialog, wxDialog)
  EVT_BUTTON(wxID_OK, mmDateRangeDialog::OnOk)
  EVT_BUTTON(BTN_UP_TOP, mmDateRangeDialog::OnTop)
  EVT_BUTTON(BTN_UP, mmDateRangeDialog::OnUp)
  EVT_BUTTON(BTN_EDIT, mmDateRangeDialog::OnEdit)
  EVT_BUTTON(BTN_DOWN, mmDateRangeDialog::OnDown)
  EVT_BUTTON(BTN_DOWN_BOTTOM, mmDateRangeDialog::OnBottom)
  EVT_BUTTON(BTN_NEW, mmDateRangeDialog::OnNew)
  EVT_BUTTON(BTN_DELETE, mmDateRangeDialog::OnDelete)
  EVT_BUTTON(BTN_DEFAULT, mmDateRangeDialog::OnDefault)
  EVT_DATAVIEW_SELECTION_CHANGED(wxID_ANY, mmDateRangeDialog::OnDateRangeSelected)
wxEND_EVENT_TABLE()


mmDateRangeDialog::mmDateRangeDialog()
{
}

mmDateRangeDialog::mmDateRangeDialog(wxWindow* parent, std::vector<DateRange2::Spec>* dateRangesPtr, int* subMenuBeginPtr)
{
    m_dateRangesPtr = dateRangesPtr;
    m_subMenuBeginPtr = subMenuBeginPtr;
    m_subMenuBegin = *subMenuBeginPtr;
    this->SetFont(parent->GetFont());
    Create(parent, -1, _t("Manage date ranges"), wxDefaultPosition, wxSize(500, 600), wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX, "");
    CreateControls();
    SetIcon(mmex::getProgramIcon());
    fillControls();
    Centre();
}

void mmDateRangeDialog::fillControls()
{
    m_dateRangesLb->DeleteAllItems();
    bool submenumissing = true;
   for (size_t k = 1; k < m_dateRangesPtr->size(); k++) {
        if (k == static_cast<size_t>(m_subMenuBegin)) {
            insertItemToLb(-1, m_subMenuHeader, "========");
            submenumissing = false;
        }
        insertItemToLb(-1, (*m_dateRangesPtr)[k].getName(), (*m_dateRangesPtr)[k].getLabel());
    }
    if (submenumissing) {
        insertItemToLb(-1, m_subMenuHeader, "========");
        m_subMenuBegin = m_dateRangesLb->GetItemCount() -1;
    }
    else {
        m_subMenuBegin--;
    }
}

void mmDateRangeDialog::CreateControls()
{
    wxBoxSizer* mainBoxSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(mainBoxSizer);

    wxBoxSizer* mainPanelSizer = new wxBoxSizer(wxHORIZONTAL);
    mainBoxSizer->Add(mainPanelSizer, g_flagsExpand);

    wxBoxSizer* itemBoxVleft = new wxBoxSizer(wxVERTICAL);
    mainPanelSizer->Add(itemBoxVleft, g_flagsExpand);

    m_dateRangesLb = new wxDataViewListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_ROW_LINES | wxDV_NO_HEADER);
    m_dateRangesLb->AppendTextColumn("");
    m_dateRangesLb->AppendTextColumn("");
    itemBoxVleft->Add(m_dateRangesLb, g_flagsExpand);

    wxBoxSizer* itemBoxVright = new wxBoxSizer(wxVERTICAL);
    mainPanelSizer->Add(itemBoxVright, wxSizerFlags().Left().Border(wxALL, 5));
    itemBoxVright->AddSpacer(20);

    m_up_top = new wxButton(this, BTN_UP_TOP, _t("Top"));
    m_up_top->SetBitmap(mmBitmapBundle(png::UPARROW, mmBitmapButtonSize));
    m_up_top->Enable(false);
    itemBoxVright->Add(m_up_top, g_flagsV);

    m_up = new wxBitmapButton(this, BTN_UP, mmBitmapBundle(png::UPARROW, mmBitmapButtonSize));
    m_up->Enable(false);
    itemBoxVright->Add(m_up, g_flagsV);

    m_edit = new wxButton(this, BTN_EDIT, _t("Edit"));
    m_edit->Enable(false);
    itemBoxVright->Add(m_edit, g_flagsV);

    m_down = new wxBitmapButton(this, BTN_DOWN, mmBitmapBundle(png::DOWNARROW, mmBitmapButtonSize));
    m_down->Enable(false);
    itemBoxVright->Add(m_down, g_flagsV);

    m_down_bottom = new wxButton(this, BTN_DOWN_BOTTOM, _t("Bottom"));
    m_down_bottom->SetBitmap(mmBitmapBundle(png::DOWNARROW, mmBitmapButtonSize));
    m_down_bottom->Enable(false);
    itemBoxVright->Add(m_down_bottom, g_flagsV);

    itemBoxVright->AddSpacer(15);
    itemBoxVright->Add(new wxButton(this, BTN_NEW, _t("New")), g_flagsV);

    itemBoxVright->AddSpacer(15);
    m_delete = new wxButton(this, BTN_DELETE, _t("Delete"));
    itemBoxVright->Add(m_delete, g_flagsV);

    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    mainBoxSizer->Add(btnSizer, g_flagsCenter);

    btnSizer->Add(new wxButton(this, wxID_OK, _t("Save")), 0, wxALL, 5);
    btnSizer->Add(new wxButton(this, BTN_DEFAULT, _t("Restore default ranges")), 0, wxALL, 5);
    btnSizer->Add(new wxButton(this, wxID_CANCEL, _t("Cancel")), 0, wxALL, 5);

    this->Layout();
}

void mmDateRangeDialog::OnTop(wxCommandEvent&)
{
    wxVector<wxVariant> data = getItemData(m_selected_row);
    m_dateRangesLb->DeleteItem(m_selected_row);
    m_dateRangesLb->UnselectRow(m_dateRangesLb->GetSelectedRow());
    m_dateRangesLb->PrependItem(data);
    if (m_selected_row == m_subMenuBegin) {
        m_subMenuBegin = 0;
    }
    else if (m_selected_row > m_subMenuBegin) {
        m_subMenuBegin++;
    }
    m_selected_row = 0;
    updateButtonState();
    m_hasChanged = true;
}

void mmDateRangeDialog::OnUp(wxCommandEvent&)
{
    exchangeItemData(m_selected_row, m_selected_row - 1);
    if (m_selected_row == m_subMenuBegin) {
        m_subMenuBegin--;
    }
    else if (m_selected_row == m_subMenuBegin + 1) {
        m_subMenuBegin++;
    }
    m_selected_row--;
    updateButtonState();
}

void mmDateRangeDialog::OnEdit(wxCommandEvent&)
{
    wxString name = m_dateRangesLb->GetTextValue(m_selected_row, 0);
    wxString range = m_dateRangesLb->GetTextValue(m_selected_row, 1);
    mmDateRangeEditDialog dlg(this, &name, &range);
    if (dlg.ShowModal() == wxID_OK) {
        m_dateRangesLb->SetTextValue(name, m_selected_row, 0);
        m_dateRangesLb->SetTextValue(range, m_selected_row, 1);
        m_hasChanged = true;
    }
}

void mmDateRangeDialog::OnDown(wxCommandEvent&)
{
    exchangeItemData(m_selected_row, m_selected_row + 1);
    if (m_selected_row == m_subMenuBegin) {
        m_subMenuBegin++;
    }
    else if (m_selected_row == m_subMenuBegin - 1) {
        m_subMenuBegin--;
    }
    m_selected_row++;
    updateButtonState();
}

void mmDateRangeDialog::OnBottom(wxCommandEvent&)
{
    wxVector<wxVariant> data = getItemData(m_selected_row);
    m_dateRangesLb->DeleteItem(m_selected_row);
    m_dateRangesLb->UnselectRow(m_dateRangesLb->GetSelectedRow());
    m_dateRangesLb->AppendItem(data);
    if (m_selected_row == m_subMenuBegin) {
        m_subMenuBegin = m_dateRangesLb->GetItemCount() - 1;
    }
    else if (m_selected_row < m_subMenuBegin) {
        m_subMenuBegin--;
    }
    m_selected_row = m_dateRangesLb->GetItemCount() - 1;
    updateButtonState();
    m_hasChanged = true;
}

void mmDateRangeDialog::OnNew(wxCommandEvent&)
{
    wxString name, range;
    mmDateRangeEditDialog dlg(this, &name, &range);
    if (dlg.ShowModal() == wxID_OK) {
        insertItemToLb(m_selected_row > -1 ? m_selected_row : 0, name, range);
        m_selected_row = m_dateRangesLb->GetItemCount() - 1;
        updateButtonState();
        m_hasChanged = true;
    }
}

void mmDateRangeDialog::OnDelete(wxCommandEvent&)
{
    if (m_selected_row > -1 && m_selected_row < m_dateRangesLb->GetItemCount()) {
       if (wxMessageBox(_t("Do you want to delete the selected date range?")
            , _t("Delete date range")
            , wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION) == wxYES)
        {
            m_dateRangesLb->DeleteItem(m_selected_row);
            m_dateRangesLb->UnselectRow(m_dateRangesLb->GetSelectedRow());
            if (m_selected_row < m_subMenuBegin) {
                m_subMenuBegin--;
            }
            m_selected_row = -1;
            updateButtonState(false);
            m_hasChanged = true;
        }
    }
}

void mmDateRangeDialog::OnDateRangeSelected(wxDataViewEvent& event)
{
    wxDataViewItem item = event.GetItem();
    m_selected_row = m_dateRangesLb->ItemToRow(item);
    updateButtonState(false);
}

void mmDateRangeDialog::updateButtonState(bool setselected) {
    bool isvalid = m_selected_row > -1;
    m_up_top->Enable(m_selected_row > 0);
    m_up->Enable(m_selected_row > 0);
    m_edit->Enable(isvalid && m_selected_row != m_subMenuBegin);
    m_down->Enable(isvalid && m_selected_row < m_dateRangesLb->GetItemCount() - 1);
    m_down_bottom->Enable(isvalid && m_selected_row < m_dateRangesLb->GetItemCount() - 1);
    m_delete->Enable(isvalid && m_selected_row != m_subMenuBegin);
    if (setselected) {
        m_dateRangesLb->SelectRow(m_selected_row);
    }
}

void mmDateRangeDialog::OnOk(wxCommandEvent&)
{
    if (m_hasChanged) {
        m_dateRangesPtr->clear();
        DateRange2::Spec sdata = DateRange2::Spec();
        sdata.parseSpec("A", _t("All"));
        m_dateRangesPtr->push_back(sdata);
        for (int k = 0; k < m_dateRangesLb->GetItemCount(); k++) {
            sdata = DateRange2::Spec();
            if (k != m_subMenuBegin) {
                if (sdata.parseSpec(m_dateRangesLb->GetTextValue(k, 1), m_dateRangesLb->GetTextValue(k, 0))) {
                    m_dateRangesPtr->push_back(sdata);
                }
            }
        }
        // Store new settings
        wxArrayString arr;
        for (size_t k = 0; k < m_dateRangesPtr->size(); k++) {
            if (k == static_cast<size_t>(m_subMenuBegin + 1)) {
                arr.Add("");
            }
            arr.Add((*m_dateRangesPtr)[k].getLabelName());
        }
        Option::instance().setCheckingRange(arr);
        *m_subMenuBeginPtr = m_subMenuBegin + 1;
    }
    EndModal(m_hasChanged ? wxID_OK : wxID_CANCEL);
}

void mmDateRangeDialog::OnDefault(wxCommandEvent&)
{
    if (wxMessageBox(_t("Do you really want to restore the default ranges?\n\nAll customization will be lost!")
        , _t("Default date ranges")
        , wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION) == wxYES)
    {
        m_dateRangesPtr->clear();
        wxArrayString arr;
        Option::instance().setCheckingRange(arr);  // delete stored settings
        EndModal(wxID_OK);
    }
}

// --- Support functions ---
wxVector<wxVariant> mmDateRangeDialog::getItemData(int row) {
    wxVector<wxVariant> data;
    data.push_back(m_dateRangesLb->GetTextValue(row, 0));
    data.push_back(m_dateRangesLb->GetTextValue(row, 1));
    return data;
}

void mmDateRangeDialog::exchangeItemData(int row1, int row2) {
    wxString name = m_dateRangesLb->GetTextValue(row1, 0);
    wxString range = m_dateRangesLb->GetTextValue(row1, 1);
    m_dateRangesLb->SetTextValue(m_dateRangesLb->GetTextValue(row2, 0), row1, 0);
    m_dateRangesLb->SetTextValue(m_dateRangesLb->GetTextValue(row2, 1), row1, 1);
    m_dateRangesLb->SetTextValue(name, row2, 0);
    m_dateRangesLb->SetTextValue(range, row2, 1);
    m_hasChanged = true;
}

void mmDateRangeDialog::insertItemToLb(int pos, wxString name, wxString range)
{
    wxVector<wxVariant> hdata;
    hdata.push_back(name);
    hdata.push_back(range);
    if (pos == -1 || pos >= m_dateRangesLb->GetItemCount()) {
        m_dateRangesLb->AppendItem(hdata);
    }
    else if (pos == 0) {
        m_dateRangesLb->PrependItem(hdata);
    }
    else {
        m_dateRangesLb->InsertItem(pos, hdata);
    }
}
