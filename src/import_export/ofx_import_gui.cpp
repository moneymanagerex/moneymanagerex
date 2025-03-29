/*******************************************************
OFX Import Module for Money Manager Ex
Copyright (C) 2025 joshuammex
xAI's GROK was leveraged in the creation of this module.

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

#include "ofx_import_gui.h"
#include <wx/xml/xml.h>
#include <wx/sstream.h>
#include <wx/grid.h>
#include "model/Model_Account.h"
#include "model/Model_Category.h"
#include "model/Model_Checking.h"
#include "model/Model_Payee.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include <climits>
#include <wx/event.h> // Added for event handling
#include <wx/file.h>
#include <wx/filedlg.h>
#include <wx/log.h>
#include <wx/msgdlg.h>
#include <wx/regex.h>
#include <wx/settings.h>
#include <wx/timectrl.h>

wxBEGIN_EVENT_TABLE(mmOFXImportDialog, wxDialog)
EVT_BUTTON(wxID_OPEN, mmOFXImportDialog::OnBrowse)
EVT_BUTTON(wxID_OK, mmOFXImportDialog::OnImport)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(mmOFXImportSummaryDialog, wxDialog)
EVT_BUTTON(wxID_OK, mmOFXImportSummaryDialog::OnOK)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(mmPayeeSelectionDialog, wxDialog)
EVT_RADIOBUTTON(ID_USE_EXISTING, mmPayeeSelectionDialog::OnUseExistingPayee)
EVT_RADIOBUTTON(ID_CREATE_NEW, mmPayeeSelectionDialog::OnCreateNewPayee)
EVT_CHECKBOX(wxID_ANY, mmPayeeSelectionDialog::OnUpdateRegex)
EVT_BUTTON(wxID_OK, mmPayeeSelectionDialog::OnOK)
EVT_CHOICE(wxID_ANY, mmPayeeSelectionDialog::OnPayeeChoice)
EVT_BUTTON(ID_TITLE_CASE, mmPayeeSelectionDialog::OnTitleCase)
EVT_GRID_LABEL_LEFT_CLICK(mmPayeeSelectionDialog::OnGridLabelLeftClick)
EVT_BUTTON(ID_UPDATE_CATEGORY, mmPayeeSelectionDialog::OnUpdateCategoryToggle)
EVT_BUTTON(ID_INSERT_ROW, mmPayeeSelectionDialog::OnInsertRow)
EVT_BUTTON(ID_DELETE_ROW, mmPayeeSelectionDialog::OnDeleteRow)
EVT_CHOICE(wxID_ANY, mmPayeeSelectionDialog::OnCategorySelection)
EVT_SET_FOCUS(mmPayeeSelectionDialog::OnCategoryFocus)
EVT_INIT_DIALOG(mmPayeeSelectionDialog::OnInitDialog)
wxEND_EVENT_TABLE()

wxString DecodeHTMLEntities(const wxString& input)
{
    wxString result = input;
    result.Replace("&amp;", "&");
    result.Replace("&lt;", "<");
    result.Replace("&gt;", ">");
    result.Replace("&quot;", "\"");
    result.Replace("&apos;", "'");
    return result;
}

mmPayeeSelectionDialog::mmPayeeSelectionDialog(wxWindow* parent, const wxString& memo, const wxString& suggestedPayeeName, const wxString& fitid,
                                               const wxString& date, const wxString& amount, const wxString& transType, int currentTransaction,
                                               int totalTransactions, wxLongLong importStartTime)
    : wxDialog(parent, wxID_ANY, _("Existing Payee Not Found"), wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER),
      selectedPayee_(suggestedPayeeName), regexPattern_(memo), shouldUpdateRegex_(false), categoryChoice_(nullptr), currentTransaction_(currentTransaction),
      totalTransactions_(totalTransactions), importStartTime_(importStartTime), updatePayeeCategory_(false), okButton_(nullptr), existingPayeeLabel_(nullptr),
      newPayeeLabel_(nullptr), payeeSizer_(nullptr), initialCategoryId_(-1), memoAdded_(false), insertRowButton_(nullptr), deleteRowButton_(nullptr),
      categoryManuallyChanged_(false), categoryMap(), suggestedPayeeName_(suggestedPayeeName)
{

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Progress and ETA
    wxLongLong currentTime = wxGetUTCTimeMillis();
    double elapsedTimeSec = (currentTime - importStartTime_).ToDouble() / 1000.0;
    double avgTimePerTrans = (currentTransaction_ > 0) ? elapsedTimeSec / currentTransaction_ : 0.0;
    int remainingTrans = totalTransactions_ - currentTransaction_;
    double estimatedTimeSec = avgTimePerTrans * remainingTrans;
    double estimatedTimeMin = estimatedTimeSec / 60.0;

    wxString progressText = wxString::Format(_("Transaction %d of %d"), currentTransaction_ + 1, totalTransactions_);
    wxString etaText = (avgTimePerTrans > 0) ? wxString::Format(_("Estimated completion time: %.1f minutes"), estimatedTimeMin) : _("Estimating time...");
    mainSizer->Add(new wxStaticText(this, wxID_ANY, progressText), 0, wxALL, 5);
    mainSizer->Add(new wxStaticText(this, wxID_ANY, etaText), 0, wxALL, 5);

    // Transaction info
    mainSizer->Add(new wxStaticText(this, wxID_ANY, wxString::Format(_("Transaction Number (FITID): %s"), fitid)), 0, wxLEFT | wxRIGHT | wxTOP, 5);
    mainSizer->Add(new wxStaticText(this, wxID_ANY, wxString::Format(_("Date: %s"), date)), 0, wxALL, 5);
    mainSizer->Add(new wxStaticText(this, wxID_ANY, wxString::Format(_("Amount: %s"), amount)), 0, wxALL, 5);
    mainSizer->Add(new wxStaticText(this, wxID_ANY, wxString::Format(_("Type: %s"), transType)), 0, wxALL, 5);

    mainSizer->Add(new wxStaticText(this, wxID_ANY, wxString::Format(_("Payee '%s' not found."), suggestedPayeeName)), 0, wxALL, 5);
    mainSizer->Add(new wxStaticText(this, wxID_ANY, _("Do you want to use an existing payee or create a new one?")), 0, wxALL, 5);

    useExistingRadio_ = new wxRadioButton(this, ID_USE_EXISTING, _("Use Existing Payee"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    createNewRadio_ = new wxRadioButton(this, ID_CREATE_NEW, _("Create New Payee"));
    useExistingRadio_->SetValue(true);
    mainSizer->Add(useExistingRadio_, 0, wxALL, 5);
    mainSizer->Add(createNewRadio_, 0, wxALL, 5);

    // Payee Selection Section
    payeeSizer_ = new wxBoxSizer(wxVERTICAL);
    existingPayeeLabel_ = new wxStaticText(this, wxID_ANY, _("Select Existing Payee:"));
    payeeChoice_ = new wxChoice(this, wxID_ANY);
    Model_Payee::Data_Set payees = Model_Payee::instance().all(Model_Payee::COL_PAYEENAME);
    int suggestedIndex = -1;
    for (size_t i = 0; i < payees.size(); ++i)
    {
        const auto& payee = payees[i];
        wxString displayName = payee.PAYEENAME;
        payeeChoice_->Append(displayName, new wxInt64ClientData(payee.PAYEEID.GetValue()));
        if (payee.PAYEENAME == suggestedPayeeName && suggestedIndex == -1)
            suggestedIndex = i;
    }
    if (suggestedIndex != -1)
        payeeChoice_->SetSelection(suggestedIndex);
    else if (!payees.empty())
        payeeChoice_->SetSelection(0);
    payeeSizer_->Add(existingPayeeLabel_, 0, wxLEFT | wxRIGHT | wxTOP, 5);
    payeeSizer_->Add(payeeChoice_, 0, wxALL | wxEXPAND, 2);

    newPayeeLabel_ = new wxStaticText(this, wxID_ANY, _("New Payee Name:"));
    newPayeeTextCtrl_ = new wxTextCtrl(this, wxID_ANY, suggestedPayeeName);
    titleCaseButton_ = new wxButton(this, ID_TITLE_CASE, "T", wxDefaultPosition, wxSize(25, -1));
    titleCaseButton_->SetToolTip(_("Convert to Title Case"));
    wxBoxSizer* newPayeeInnerSizer = new wxBoxSizer(wxHORIZONTAL);
    newPayeeInnerSizer->Add(newPayeeTextCtrl_, 1, wxEXPAND, 0);
    newPayeeInnerSizer->Add(titleCaseButton_, 0, wxLEFT | wxALIGN_CENTER_VERTICAL, 5);

    mainSizer->Add(payeeSizer_, 0, wxEXPAND);

    categoryChoice_ = new wxChoice(this, wxID_ANY);
    categoryChoice_->Append(_("Uncategorized"), new wxStringClientData("-1"));
    Model_Category::Data_Set categories = Model_Category::instance().all(Model_Category::COL_CATEGNAME);
    categoryMap.clear();
    for (const auto& cat : categories)
    {
        categoryMap[cat.CATEGID.GetValue()] = cat;
    }
    for (const auto& cat : categories)
    {
        if (cat.PARENTID.GetValue() == -1)
        {
            AddCategoryToChoice(categoryChoice_, cat.CATEGID.GetValue(), categoryMap, 0);
        }
    }
    categoryChoice_->SetSelection(0);

    updateCategoryButton_ = new wxButton(this, ID_UPDATE_CATEGORY, "P", wxDefaultPosition, wxSize(25, -1));
    updateCategoryButton_->SetToolTip(_("Toggle to update payee's default category (P) or apply to this transaction only"));
    wxBoxSizer* categoryButtonSizer = new wxBoxSizer(wxHORIZONTAL);
    categoryButtonSizer->Add(categoryChoice_, 1, wxEXPAND, 0);
    categoryButtonSizer->Add(updateCategoryButton_, 0, wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
    mainSizer->Add(new wxStaticText(this, wxID_ANY, _("Select Category for Payee:")), 0, wxLEFT | wxRIGHT | wxTOP, 5);
    mainSizer->Add(categoryButtonSizer, 0, wxALL | wxEXPAND, 2);

    updateRegexCheckBox_ = new wxCheckBox(this, wxID_ANY, _("Define/Update Regex Patterns"));
    mainSizer->Add(updateRegexCheckBox_, 0, wxALL, 5);

    regexGrid_ = new wxGrid(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL | wxVSCROLL);
    regexGrid_->CreateGrid(2, 1);
    regexGrid_->SetColLabelValue(0, _("Regex Pattern"));
    regexGrid_->SetDefaultColSize(300);
    regexGrid_->SetColMinimalWidth(0, 50);
    regexGrid_->EnableGridLines(true);
    regexGrid_->EnableEditing(true);
    regexGrid_->SetDefaultCellBackgroundColour(*wxWHITE);
    regexGrid_->SetLabelBackgroundColour(GetBackgroundColour());
    regexGrid_->SetDefaultEditor(new wxGridCellTextEditor());
    mainSizer->Add(new wxStaticText(this, wxID_ANY, _("Edit Regex Patterns:")), 0, wxLEFT | wxRIGHT | wxTOP, 5);
    mainSizer->Add(regexGrid_, 1, wxALL | wxEXPAND, 2);

    wxBoxSizer* regexButtonSizer = new wxBoxSizer(wxHORIZONTAL);
    insertRowButton_ = new wxButton(this, ID_INSERT_ROW, _("Insert Row"));
    deleteRowButton_ = new wxButton(this, ID_DELETE_ROW, _("Delete Row"));
    regexButtonSizer->Add(insertRowButton_, 0, wxRIGHT, 5);
    regexButtonSizer->Add(deleteRowButton_, 0, wxRIGHT, 5);
    mainSizer->Add(regexButtonSizer, 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);

    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    okButton_ = new wxButton(this, wxID_OK, _("OK"));
    buttonSizer->Add(okButton_, 0, wxALL, 5);
    buttonSizer->Add(new wxButton(this, wxID_CANCEL, _("Cancel")), 0, wxALL, 5);
    mainSizer->Add(buttonSizer, 0, wxALIGN_CENTER | wxALL, 5);

    SetSizer(mainSizer);
    SetMinSize(wxSize(450, 680));
    SetSize(wxSize(450, 680));
    Layout();

    payeeChoice_->Enable(true);
    existingPayeeLabel_->Show(true);
    payeeChoice_->Show(true);
    newPayeeTextCtrl_->Enable(false);
    titleCaseButton_->Enable(false);
    newPayeeLabel_->Show(false);
    newPayeeInnerSizer->Show(false);
    updateCategoryButton_->Enable(true);
    updateRegexCheckBox_->Enable(true);
    regexGrid_->EnableEditing(updateRegexCheckBox_->IsChecked());
    regexGrid_->Enable(false);
    insertRowButton_->Enable(false);
    deleteRowButton_->Enable(false);

    newPayeeTextCtrl_->Bind(wxEVT_TEXT, &mmPayeeSelectionDialog::UpdateOKButton, this);
    categoryChoice_->Bind(wxEVT_CHOICE, &mmPayeeSelectionDialog::OnCategorySelection, this);
    categoryChoice_->Bind(wxEVT_SET_FOCUS, &mmPayeeSelectionDialog::OnCategoryFocus, this);

    regexGrid_->Bind(wxEVT_SIZE,
                     [this](wxSizeEvent& event)
                     {
                         int dialogWidth = GetClientSize().GetWidth();
                         int scrollbarWidth = wxSystemSettings::GetMetric(wxSYS_VSCROLL_X);
                         int padding = (10 * 2 + 2 * 2);
                         int availableWidth = dialogWidth - padding - scrollbarWidth - 50;

                         if (availableWidth > 50)
                             regexGrid_->SetColSize(0, availableWidth);
                         else
                             regexGrid_->SetColSize(0, 50);
                         regexGrid_->ForceRefresh();
                         event.Skip();
                     });

    int dialogWidth = GetClientSize().GetWidth();
    int scrollbarWidth = wxSystemSettings::GetMetric(wxSYS_VSCROLL_X);
    int padding = (10 * 2 + 2 * 2);
    int initialWidth = dialogWidth - padding - scrollbarWidth - 50;
    if (initialWidth > 50)
        regexGrid_->SetColSize(0, initialWidth);
    else
        regexGrid_->SetColSize(0, 50);
    regexGrid_->ForceRefresh();

    UpdateOKButton(wxCommandEvent());

    wxInt64ClientData* suggestedPayeeIdData = nullptr;
    if (!suggestedPayeeName.IsEmpty())
    {
        Model_Payee::Data_Set payees = Model_Payee::instance().all(Model_Payee::COL_PAYEENAME);
        for (size_t i = 0; i < payees.size(); ++i)
        {
            if (payees[i].PAYEENAME == suggestedPayeeName)
            {
                suggestedPayeeIdData = new wxInt64ClientData(payees[i].PAYEEID.GetValue());
                break;
            }
        }
    }
    //LoadRegexPatterns(suggestedPayeeIdData);
    //delete suggestedPayeeIdData;
    regexGrid_->SetScrollRate(10, 10);

    initialCategoryId_ = GetSelectedCategoryID();
}

void mmPayeeSelectionDialog::OnCategorySelection(wxCommandEvent& event)
{
    int sel = categoryChoice_->GetSelection();
    long long selectedCategoryId = GetSelectedCategoryID();
    if (selectedCategoryId == -1)
    {
        updateCategoryButton_->Enable(false);
        updateCategoryButton_->SetToolTip(_("Cannot set 'Uncategorized' as the default category for the payee"));
    }
    else
    {
        updateCategoryButton_->Enable(true);
        updateCategoryButton_->SetToolTip(_("Toggle to update payee's default category (P) or apply to this transaction only"));
    }
    categoryManuallyChanged_ = true;
    event.Skip();
}

void mmPayeeSelectionDialog::OnInitDialog(wxInitDialogEvent& event)
{
    wxInt64ClientData* suggestedPayeeIdData = nullptr;
    if (!suggestedPayeeName_.IsEmpty())
    {
        Model_Payee::Data_Set payees = Model_Payee::instance().all(Model_Payee::COL_PAYEENAME);
        for (size_t i = 0; i < payees.size(); ++i)
        {
            if (payees[i].PAYEENAME == suggestedPayeeName_)
            {
                suggestedPayeeIdData = new wxInt64ClientData(payees[i].PAYEEID.GetValue());
                break;
            }
        }
    }
    LoadRegexPatterns(suggestedPayeeIdData);
    delete suggestedPayeeIdData;
    regexGrid_->SetScrollRate(10, 10);
    event.Skip();
}



void mmPayeeSelectionDialog::OnCategoryFocus(wxFocusEvent& event)
{
    event.Skip();
}

void mmPayeeSelectionDialog::UpdateOKButton(wxCommandEvent& /*event*/)
{
    if (okButton_)
    {
        if (useExistingRadio_->GetValue())
            okButton_->Enable(!payeeChoice_->GetStringSelection().IsEmpty());
        else
            okButton_->Enable(!newPayeeTextCtrl_->GetValue().IsEmpty());
    }
}

void mmPayeeSelectionDialog::OnUseExistingPayee(wxCommandEvent& /*event*/)
{
    payeeSizer_->Detach(newPayeeLabel_);
    payeeSizer_->Detach(newPayeeTextCtrl_->GetContainingSizer());
    payeeSizer_->Insert(0, existingPayeeLabel_, 0, wxLEFT | wxRIGHT | wxTOP, 5);
    payeeSizer_->Insert(1, payeeChoice_, 0, wxALL | wxEXPAND, 2);

    payeeChoice_->Enable(true);
    existingPayeeLabel_->Show(true);
    payeeChoice_->Show(true);
    newPayeeTextCtrl_->Enable(false);
    titleCaseButton_->Enable(false);
    newPayeeLabel_->Show(false);
    newPayeeTextCtrl_->GetContainingSizer()->Show(false);

    updateCategoryButton_->Enable(true);
    updateCategoryButton_->Show(true);
    updateRegexCheckBox_->Enable(true);
    updateRegexCheckBox_->SetValue(false);
    shouldUpdateRegex_ = false;
    memoAdded_ = false;
    regexGrid_->EnableEditing(updateRegexCheckBox_->IsChecked());
    regexGrid_->Enable(true);
    insertRowButton_->Enable(updateRegexCheckBox_->IsChecked());
    deleteRowButton_->Enable(updateRegexCheckBox_->IsChecked() && regexGrid_->GetNumberRows() > 0);
    selectedPayee_ = payeeChoice_->GetStringSelection();

    if (regexGrid_->GetNumberRows() > 0)
        regexGrid_->DeleteRows(0, regexGrid_->GetNumberRows());
    regexGrid_->AppendRows(1);
    wxInt64ClientData* clientData = dynamic_cast<wxInt64ClientData*>(payeeChoice_->GetClientObject(payeeChoice_->GetSelection()));
    LoadRegexPatterns(clientData);
    UpdateOKButton(wxCommandEvent());
    OnPayeeChoice(wxCommandEvent());
    Layout();
}

void mmPayeeSelectionDialog::OnCreateNewPayee(wxCommandEvent& /*event*/)
{
    payeeSizer_->Detach(existingPayeeLabel_);
    payeeSizer_->Detach(payeeChoice_);
    payeeSizer_->Insert(0, newPayeeLabel_, 0, wxLEFT | wxRIGHT | wxTOP, 5);
    payeeSizer_->Insert(1, newPayeeTextCtrl_->GetContainingSizer(), 0, wxALL | wxEXPAND, 2);

    payeeChoice_->Enable(false);
    existingPayeeLabel_->Show(false);
    payeeChoice_->Show(false);
    newPayeeTextCtrl_->Enable(true);
    titleCaseButton_->Enable(true);
    newPayeeLabel_->Show(true);
    newPayeeTextCtrl_->GetContainingSizer()->Show(true);

    updateCategoryButton_->Enable(false);
    updateCategoryButton_->Show(false);
    updateRegexCheckBox_->Enable(true);
    updateRegexCheckBox_->SetValue(true);
    shouldUpdateRegex_ = true;
    regexGrid_->EnableEditing(true);
    regexGrid_->Enable(true);
    insertRowButton_->Enable(true);
    deleteRowButton_->Enable(regexGrid_->GetNumberRows() > 0);
    selectedPayee_ = newPayeeTextCtrl_->GetValue();

    if (regexGrid_->GetNumberRows() > 0)
        regexGrid_->DeleteRows(0, regexGrid_->GetNumberRows());
    regexGrid_->AppendRows(1);
    regexGrid_->SetCellValue(0, 0, regexPattern_);
    deleteRowButton_->Enable(regexGrid_->GetNumberRows() > 0);
    UpdateOKButton(wxCommandEvent());
    Layout();
}

void mmPayeeSelectionDialog::OnUpdateCategoryToggle(wxCommandEvent& /*event*/)
{
    updatePayeeCategory_ = !updatePayeeCategory_;
    updateCategoryButton_->SetLabel(updatePayeeCategory_ ? "P*" : "P");
}

void mmPayeeSelectionDialog::OnUpdateRegex(wxCommandEvent& /*event*/)
{
    shouldUpdateRegex_ = updateRegexCheckBox_->IsChecked();
    wxLogDebug("OnUpdateRegex: shouldUpdateRegex_ = %d, rows = %d", shouldUpdateRegex_, regexGrid_->GetNumberRows());
    regexGrid_->EnableEditing(shouldUpdateRegex_);
    regexGrid_->Enable(shouldUpdateRegex_);
    insertRowButton_->Enable(shouldUpdateRegex_);
    deleteRowButton_->Enable(shouldUpdateRegex_ && regexGrid_->GetNumberRows() > 0);

    if (shouldUpdateRegex_)
    {
        if (useExistingRadio_->GetValue() && !memoAdded_)
        {
            wxLogDebug("OnUpdateRegex: Adding memo, current rows = %d", regexGrid_->GetNumberRows());
            if (regexGrid_->GetNumberRows() == 0 || (regexGrid_->GetNumberRows() > 0 && regexGrid_->GetCellValue(0, 0).IsEmpty()))
            {
                if (regexGrid_->GetNumberRows() == 0)
                {
                    regexGrid_->AppendRows(1);
                    regexGrid_->ForceRefresh();
                }
                if (0 < regexGrid_->GetNumberRows() && 0 < regexGrid_->GetNumberCols())
                {
                    regexGrid_->SetCellValue(0, 0, regexPattern_);
                }
            }
            else if (regexGrid_->GetNumberRows() > 0)
            {
                regexGrid_->AppendRows(1);
                int newRow = regexGrid_->GetNumberRows() - 1;
                if (newRow < regexGrid_->GetNumberRows() && 0 < regexGrid_->GetNumberCols())
                {
                    regexGrid_->SetCellValue(newRow, 0, regexPattern_);
                }
            }
            memoAdded_ = true;
            wxLogDebug("OnUpdateRegex: Memo added, now %d rows", regexGrid_->GetNumberRows());
            regexGrid_->ForceRefresh();
        }
    }
    else
    {
        wxLogDebug("OnUpdateRegex: Resetting grid, current rows = %d", regexGrid_->GetNumberRows());
        if (useExistingRadio_->GetValue())
        {
            wxInt64ClientData* clientData = dynamic_cast<wxInt64ClientData*>(payeeChoice_->GetClientObject(payeeChoice_->GetSelection()));
            LoadRegexPatterns(clientData);
        }
        else
        {
            if (regexGrid_->GetNumberRows() > 0)
            {
                regexGrid_->DeleteRows(0, regexGrid_->GetNumberRows());
                regexGrid_->ForceRefresh();
            }
            regexGrid_->AppendRows(1);
            if (0 < regexGrid_->GetNumberRows() && 0 < regexGrid_->GetNumberCols())
            {
                regexGrid_->SetCellValue(0, 0, regexPattern_);
            }
            regexGrid_->ForceRefresh();
        }
        memoAdded_ = false;
        wxLogDebug("OnUpdateRegex: Reset complete, now %d rows", regexGrid_->GetNumberRows());
    }

    deleteRowButton_->Enable(shouldUpdateRegex_ && regexGrid_->GetNumberRows() > 0);
}



void mmPayeeSelectionDialog::OnPayeeChoice(wxCommandEvent& event)
{
    int selection = payeeChoice_->GetSelection();
    if (selection == wxNOT_FOUND)
        return;

    wxString selectedPayeeName = payeeChoice_->GetString(selection);
    wxInt64ClientData* clientData = dynamic_cast<wxInt64ClientData*>(payeeChoice_->GetClientObject(selection));
    if (!clientData)
    {
        wxLogError("No client data for payee '%s'", selectedPayeeName);
        return;
    }

    int64_t payeeId = clientData->GetValue();
    Model_Payee::Data* payee = Model_Payee::instance().get(payeeId);
    if (payee && !categoryManuallyChanged_)
    {
        long long payeeCategoryId = payee->CATEGID.GetValue();
        for (unsigned int i = 0; i < categoryChoice_->GetCount(); ++i)
        {
            wxStringClientData* data = dynamic_cast<wxStringClientData*>(categoryChoice_->GetClientObject(i));
            if (data)
            {
                long long categId;
                if (data->GetData().ToLongLong(&categId) && categId == payeeCategoryId)
                {
                    categoryChoice_->SetSelection(i);
                    break;
                }
            }
        }
    }
    else if (!payee && !categoryManuallyChanged_)
    {
        categoryChoice_->SetSelection(0);
    }

    if (event.GetEventType() == wxEVT_CHOICE)
    {
        categoryManuallyChanged_ = false;
    }

    if (useExistingRadio_->GetValue())
    {
        LoadRegexPatterns(clientData);
    }

    UpdateOKButton(event);
    event.Skip();
}

void mmPayeeSelectionDialog::OnInsertRow(wxCommandEvent& /*event*/)
{
    if (!regexGrid_->IsEnabled() || !regexGrid_->IsEditable())
        return;

    regexGrid_->AppendRows(1);
    deleteRowButton_->Enable(regexGrid_->GetNumberRows() > 0);
}

void mmPayeeSelectionDialog::OnDeleteRow(wxCommandEvent& /*event*/)
{
    if (!regexGrid_->IsEnabled() || !regexGrid_->IsEditable())
        return;

    wxArrayInt selectedRows = regexGrid_->GetSelectedRows();
    if (selectedRows.IsEmpty())
    {
        wxMessageBox(_("Please select a row to delete."), _("Error"), wxOK | wxICON_WARNING);
        return;
    }

    int rowToDelete = selectedRows[0];
    regexGrid_->DeleteRows(rowToDelete, 1);
    deleteRowButton_->Enable(regexGrid_->GetNumberRows() > 0);
}

void mmPayeeSelectionDialog::OnTitleCase(wxCommandEvent& /*event*/)
{
    wxString text = newPayeeTextCtrl_->GetValue();
    if (text.IsEmpty())
        return;

    wxString result;
    bool capitalizeNext = true;
    for (size_t i = 0; i < text.Length(); ++i)
    {
        if (capitalizeNext && wxIsalpha(text[i]))
        {
            result += wxToupper(text[i]);
            capitalizeNext = false;
        }
        else
        {
            result += wxTolower(text[i]);
            capitalizeNext = !wxIsalpha(text[i]);
        }
    }
    newPayeeTextCtrl_->SetValue(result);
}

void mmPayeeSelectionDialog::OnOK(wxCommandEvent& event)
{
    if (useExistingRadio_->GetValue())
    {
        selectedPayee_ = payeeChoice_->GetStringSelection();
    }
    else
    {
        selectedPayee_ = newPayeeTextCtrl_->GetValue();
    }
    if (selectedPayee_.IsEmpty())
    {
        wxMessageBox(_("Please select an existing payee or enter a new payee name."), _("Error"), wxOK | wxICON_ERROR);
        return;
    }

    long long selectedCategoryId = GetSelectedCategoryID();

    bool shouldUpdateCategory = updatePayeeCategory_;
    Model_Payee::Data* payee = Model_Payee::instance().get_one(Model_Payee::PAYEENAME(selectedPayee_));

    if (useExistingRadio_->GetValue() && shouldUpdateCategory && payee)
    {
        if (selectedCategoryId != payee->CATEGID.GetValue())
        {
            payee->CATEGID = selectedCategoryId;
            Model_Payee::instance().save(payee);
        }
    }

    if (shouldUpdateRegex_)
    {
        std::vector<wxString> patterns;
        for (int i = 0; i < regexGrid_->GetNumberRows(); ++i)
        {
            wxString pattern = regexGrid_->GetCellValue(i, 0).Trim().Trim(false);
            if (!pattern.IsEmpty())
            {
                if (pattern.Contains("*"))
                {
                    pattern.Replace("*", ".*", true);
                    pattern.Replace("..*", ".*", true);
                }
                wxRegEx re(pattern, wxRE_ADVANCED);
                if (!re.IsValid())
                {
                    wxMessageBox(wxString::Format(_("Invalid regular expression '%s' in row %d: please correct the pattern"), pattern, i + 1), _("Error"),
                                 wxOK | wxICON_ERROR);
                    return;
                }
                patterns.push_back(pattern);
            }
        }

        if (!patterns.empty())
        {
            rapidjson::Document doc;
            doc.SetObject();
            rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
            for (size_t i = 0; i < patterns.size(); ++i)
            {
                wxString key = wxString::Format("%zu", i);
                rapidjson::Value k(key.mb_str(), allocator);
                rapidjson::Value v(patterns[i].mb_str(), allocator);
                doc.AddMember(k, v, allocator);
            }
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            doc.Accept(writer);
            regexPattern_ = wxString::FromUTF8(buffer.GetString());

            if (!payee && createNewRadio_->GetValue())
            {
                payee = Model_Payee::instance().create();
                payee->PAYEENAME = selectedPayee_;
                payee->CATEGID = selectedCategoryId;
            }
            if (payee)
            {
                payee->PATTERN = regexPattern_;
                Model_Payee::instance().save(payee);
            }
        }
        else
        {
            regexPattern_ = "{}";
        }
    }
    else if (createNewRadio_->GetValue() && !payee)
    {
        payee = Model_Payee::instance().create();
        payee->PAYEENAME = selectedPayee_;
        payee->CATEGID = selectedCategoryId;
        Model_Payee::instance().save(payee);
    }

    mmOFXImportDialog* parentDialog = dynamic_cast<mmOFXImportDialog*>(GetParent());
    if (parentDialog && (shouldUpdateRegex_ || (payee && shouldUpdateCategory)))
    {
        parentDialog->loadRegexMappings();
    }

    EndModal(wxID_OK);
}

void mmPayeeSelectionDialog::OnGridLabelLeftClick(wxGridEvent& event)
{
    int row = event.GetRow();
    int col = event.GetCol();
    if (row >= 0 && col >= 0)
    {
        wxString cellValue = regexGrid_->GetCellValue(row, col);
        if (!cellValue.IsEmpty())
            regexGrid_->SetToolTip(cellValue);
        else
            regexGrid_->UnsetToolTip();
    }
    event.Skip();
}

void mmPayeeSelectionDialog::LoadRegexPatterns(wxInt64ClientData* payeeIdData)
{
    wxLogDebug("LoadRegexPatterns: Starting with %d rows", regexGrid_->GetNumberRows());
    if (regexGrid_->GetNumberRows() > 0)
    {
        regexGrid_->DeleteRows(0, regexGrid_->GetNumberRows());
        regexGrid_->ForceRefresh(); // Ensure grid updates its state
    }
    wxLogDebug("LoadRegexPatterns: Cleared to %d rows", regexGrid_->GetNumberRows());

    int64_t payeeId = payeeIdData ? payeeIdData->GetValue() : -1;
    Model_Payee::Data* payee = (payeeId >= 0) ? Model_Payee::instance().get(payeeId) : nullptr;
    if (payee && !payee->PATTERN.IsEmpty())
    {
        rapidjson::Document j_doc;
        j_doc.Parse(payee->PATTERN.mb_str());
        if (!j_doc.HasParseError() && j_doc.IsObject())
        {
            int row = 0;
            for (rapidjson::Value::ConstMemberIterator itr = j_doc.MemberBegin(); itr != j_doc.MemberEnd(); ++itr)
            {
                if (itr->value.IsString())
                {
                    wxString pattern = wxString::FromUTF8(itr->value.GetString());
                    if (!pattern.IsEmpty())
                    {
                        regexGrid_->AppendRows(1);
                        if (row < regexGrid_->GetNumberRows() && 0 < regexGrid_->GetNumberCols())
                        {
                            regexGrid_->SetCellValue(row, 0, pattern);
                            row++;
                        }
                    }
                }
            }
            if (row > 0)
            {
                wxLogDebug("LoadRegexPatterns: Loaded %d patterns", row);
                regexGrid_->ForceRefresh();
                return;
            }
        }
    }

    regexGrid_->AppendRows(1);
    if (0 < regexGrid_->GetNumberRows() && 0 < regexGrid_->GetNumberCols())
    {
        regexGrid_->SetCellValue(0, 0, "");
    }
    wxLogDebug("LoadRegexPatterns: Set 1 blank row, now %d rows", regexGrid_->GetNumberRows());
    regexGrid_->ForceRefresh();
}



void mmPayeeSelectionDialog::LoadRegexPatterns(const wxString& payeeName)
{
    if (regexGrid_->GetNumberRows() > 0)
        regexGrid_->DeleteRows(0, regexGrid_->GetNumberRows());

    Model_Payee::Data* payee = Model_Payee::instance().get_one(Model_Payee::PAYEENAME(payeeName));
    if (payee && !payee->PATTERN.IsEmpty())
    {
        rapidjson::Document j_doc;
        j_doc.Parse(payee->PATTERN.mb_str());
        if (!j_doc.HasParseError() && j_doc.IsObject())
        {
            int row = 0;
            for (rapidjson::Value::ConstMemberIterator itr = j_doc.MemberBegin(); itr != j_doc.MemberEnd(); ++itr)
            {
                if (itr->value.IsString())
                {
                    wxString pattern = wxString::FromUTF8(itr->value.GetString());
                    if (!pattern.IsEmpty())
                    {
                        regexGrid_->AppendRows(1);
                        regexGrid_->SetCellValue(row, 0, pattern);
                        row++;
                    }
                }
            }
            if (row > 0)
                return;
        }
    }

    regexGrid_->AppendRows(1);
    regexGrid_->SetCellValue(0, 0, "");
    regexGrid_->SetCellValue(1, 0, "");
}

void mmPayeeSelectionDialog::AddCategoryToChoice(wxChoice* choice, long long categId, const std::map<long long, Model_Category::Data>& categoryMap, int level)
{
    auto it = categoryMap.find(categId);
    if (it == categoryMap.end())
        return;

    const Model_Category::Data& category = it->second;
    wxString indent;
    for (int i = 0; i < level; ++i)
        indent += "  ";
    wxString itemText = indent + category.CATEGNAME;

    wxString clientData = wxString::Format("%lld", categId);
    choice->Append(itemText, new wxStringClientData(clientData));

    for (const auto& child : categoryMap)
    {
        if (child.second.PARENTID.GetValue() == categId)
            AddCategoryToChoice(choice, child.first, categoryMap, level + 1);
    }
}

// No duplicate definitions of GetSelectedCategory or GetSelectedCategoryID here

mmOFXImportDialog::mmOFXImportDialog(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, _("Import OFX File"), wxDefaultPosition, wxSize(500, 230), wxCAPTION | wxCLOSE_BOX | wxMINIMIZE_BOX), fileNameCtrl_(nullptr),
      accountDropDown_(nullptr), account_id_(0), payeeRegExCheckBox_(nullptr), transferCategId_(-1), importStartTime_(0)
{
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    mainSizer->Add(new wxStaticText(this, wxID_ANY, _("OFX File:")), 0, wxALL, 5);
    fileNameCtrl_ = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    wxButton* browseButton = new wxButton(this, wxID_OPEN, _("Browse"));
    wxBoxSizer* fileSizer = new wxBoxSizer(wxHORIZONTAL);
    fileSizer->Add(fileNameCtrl_, 1, wxALL | wxEXPAND, 5);
    fileSizer->Add(browseButton, 0, wxALL, 5);
    mainSizer->Add(fileSizer, 0, wxEXPAND);

    mainSizer->Add(new wxStaticText(this, wxID_ANY, _("Account:")), 0, wxALL, 5);
    accountDropDown_ = new wxChoice(this, wxID_ANY);
    wxLogDebug("Populating account dropdown...");
    for (const auto& account : Model_Account::instance().all(Model_Account::COL_ACCOUNTNAME))
    {
        wxString accountIdStr = wxString::Format("%lld", account.ACCOUNTID.GetValue());
        int idx = accountDropDown_->Append(account.ACCOUNTNAME, new wxStringClientData(accountIdStr));
        wxLogDebug("Added account: %s (ID: %lld, ACCOUNTNUM: '%s') at index %d", account.ACCOUNTNAME, account.ACCOUNTID.GetValue(), account.ACCOUNTNUM, idx);
        if (account.ACCOUNTID == account_id_)
            accountDropDown_->SetSelection(idx);
    }
    mainSizer->Add(accountDropDown_, 0, wxALL | wxEXPAND, 5);

    payeeRegExCheckBox_ = new wxCheckBox(this, wxID_ANY, _("Use existing regex patterns to automatically match payees"), wxDefaultPosition, wxDefaultSize);
    payeeRegExCheckBox_->SetValue(true);
    mainSizer->Add(payeeRegExCheckBox_, 0, wxALL, 5);

    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonSizer->Add(new wxButton(this, wxID_OK, _("Import")), 0, wxALL, 5);
    buttonSizer->Add(new wxButton(this, wxID_CANCEL, _("Cancel")), 0, wxALL, 5);
    mainSizer->Add(buttonSizer, 0, wxALIGN_CENTER | wxALL, 5);

    SetSizer(mainSizer);
    SetMinSize(wxSize(500, 230));
    SetMaxSize(wxSize(-1, 230));
    Layout();

    Model_Category::Data_Set transferCats = Model_Category::instance().find(Model_Category::CATEGNAME("Transfer"));
    if (!transferCats.empty())
    {
        transferCategId_ = transferCats[0].CATEGID.GetValue();
    }
    else
    {
        wxLogWarning("Transfer category not found in database. Transfers may not display correctly.");
    }

    loadRegexMappings();
}

mmOFXImportDialog::~mmOFXImportDialog()
{
}

void mmOFXImportDialog::loadRegexMappings()
{
    payeeRegexMap_.clear();
    Model_Payee::Data_Set payees = Model_Payee::instance().all();
    for (const auto& payee : payees)
    {
        if (!payee.PATTERN.IsEmpty())
        {
            rapidjson::Document j_doc;
            j_doc.Parse(payee.PATTERN.mb_str());
            if (!j_doc.HasParseError() && j_doc.IsObject())
            {
                for (rapidjson::Value::ConstMemberIterator itr = j_doc.MemberBegin(); itr != j_doc.MemberEnd(); ++itr)
                {
                    if (itr->value.IsString())
                    {
                        wxString pattern = wxString::FromUTF8(itr->value.GetString());
                        if (!pattern.IsEmpty())
                        {
                            wxString normalizedPattern = pattern;
                            if (normalizedPattern.Contains("*"))
                            {
                                normalizedPattern.Replace("*", ".*", true);
                                normalizedPattern.Replace("..*", ".*", true);
                            }
                            payeeRegexMap_[normalizedPattern] = payee.PAYEENAME;
                        }
                    }
                }
            }
        }
    }
}


void mmOFXImportDialog::OnBrowse(wxCommandEvent& /*event*/)
{
    wxFileDialog dlg(this, _("Choose OFX file"), wxEmptyString, wxEmptyString, "OFX files (*.ofx)|*.ofx", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dlg.ShowModal() == wxID_OK)
    {
        fileNameCtrl_->SetValue(dlg.GetPath());
        //fileNameCtrl_->ShowPosition(fileNameCtrl_->GetLastPosition());
        wxFile file(dlg.GetPath());
        if (!file.IsOpened())
        {
            wxLogError("Failed to open OFX file: %s", dlg.GetPath());
            return;
        }
        long pos = fileNameCtrl_->XYToPosition(fileNameCtrl_->GetLineLength(0), 0);
        fileNameCtrl_->SetInsertionPoint(pos);
        fileNameCtrl_->ShowPosition(pos); // Ensure the view scrolls to this position

        wxString fileContent;
        wxFileOffset length = file.Length();
        wxCharBuffer buffer(length);
        file.Read(buffer.data(), length);
        fileContent = wxString::FromUTF8(buffer.data(), length);
        file.Close();

        wxLogDebug("OFX file content (first 500 chars): %s", fileContent.Left(500));

        wxString xmlContent = fileContent;
        xmlContent.Replace("\r\n", "\n");
        xmlContent.Replace("\r", "\n");
        wxString newXmlContent;
        wxStringTokenizer tokenizer(xmlContent, "\n");
        wxString acctId;
        while (tokenizer.HasMoreTokens())
        {
            wxString currentLine = tokenizer.GetNextToken().Trim().Trim(false);
            wxLogDebug("Processing line: %s", currentLine);
            if (currentLine.StartsWith("<ACCTID>"))
            {
                acctId = currentLine.AfterFirst('>').BeforeFirst('<');
                acctId.Trim(true).Trim(false);
                if (!acctId.IsEmpty())
                {
                    wxLogDebug("Found ACCTID: '%s'", acctId);
                    break;
                }
                else
                {
                    wxLogWarning("Empty ACCTID found in line: %s", currentLine);
                }
            }
        }

        if (acctId.IsEmpty())
        {
            wxLogWarning("No ACCTID found in OFX file: %s", dlg.GetPath());
            return;
        }

        bool accountFound = false;
        for (unsigned int i = 0; i < accountDropDown_->GetCount(); ++i)
        {
            wxStringClientData* data = static_cast<wxStringClientData*>(accountDropDown_->GetClientObject(i));
            if (data)
            {
                long long tempAccountId;
                data->GetData().ToLongLong(&tempAccountId);
                Model_Account::Data* account = Model_Account::instance().get(tempAccountId);
                if (account)
                {
                    wxString accountNum = account->ACCOUNTNUM;
                    accountNum.Trim(true).Trim(false);
                    wxLogDebug("Comparing ACCTID '%s' with ACCOUNTNUM '%s' for account %s (ID: %lld)", acctId, accountNum, account->ACCOUNTNAME, tempAccountId);
                    if (accountNum.IsSameAs(acctId, false))
                    {
                        accountDropDown_->SetSelection(i);
                        account_id_ = tempAccountId;
                        wxLogDebug("Match found! Auto-selected account: %s (ID: %lld, ACCOUNTNUM: '%s') at index %d", account->ACCOUNTNAME,
                                   account_id_.GetValue(), account->ACCOUNTNUM, i);
                        accountFound = true;
                        break;
                    }
                }
            }
        }

        if (!accountFound)
        {
            wxLogWarning("No account found with ACCOUNTNUM matching ACCTID '%s'", acctId);
        }
    }
}

void mmOFXImportDialog::OnImport(wxCommandEvent& /*event*/)
{
    wxString filePath = fileNameCtrl_->GetValue();
    if (filePath.empty())
    {
        wxMessageBox(_("No OFX file selected."), _("Error"), wxOK | wxICON_ERROR);
        return;
    }

    wxString accountName = accountDropDown_->GetStringSelection();
    if (accountName.empty())
    {
        wxMessageBox(_("No account selected."), _("Error"), wxOK | wxICON_ERROR);
        return;
    }

    wxStringClientData* data = static_cast<wxStringClientData*>(accountDropDown_->GetClientObject(accountDropDown_->GetSelection()));
    if (!data)
    {
        wxMessageBox(_("Invalid account selection."), _("Error"), wxOK | wxICON_ERROR);
        return;
    }

    long long tempAccountId;
    if (!data->GetData().ToLongLong(&tempAccountId))
    {
        wxMessageBox(_("Invalid account ID."), _("Error"), wxOK | wxICON_ERROR);
        return;
    }
    account_id_ = tempAccountId;

    Model_Account::Data* account = Model_Account::instance().get(static_cast<int>(account_id_.GetValue()));
    if (!account)
    {
        wxMessageBox(wxString::Format(_("Account ID %s does not exist."), account_id_.ToString()), _("Error"), wxOK | wxICON_ERROR);
        return;
    }

    importStartTime_ = wxGetUTCTimeMillis();
    std::vector<OFXImportResult> importResults;
    OFXImportStats stats;
    ParseOFX(filePath, importResults, stats);
    mmOFXImportSummaryDialog summaryDlg(this, importResults, stats, importStartTime_);
    summaryDlg.ShowModal();
    EndModal(wxID_OK);
}

bool mmOFXImportDialog::ParseOFX(const wxString& filePath, std::vector<OFXImportResult>& importResults, OFXImportStats& stats)
{
    wxFile file(filePath);
    if (!file.IsOpened())
    {
        wxLogError("Failed to open OFX file: %s", filePath);
        return false;
    }

    wxString fileContent;
    wxFileOffset length = file.Length();
    wxCharBuffer buffer(length);
    file.Read(buffer.data(), length);
    fileContent = wxString::FromUTF8(buffer.data(), length);
    file.Close();

    wxLogDebug("Raw OFX content (first 500 chars): %s", fileContent.Left(500));

    // Preprocess OFX content
    wxString xmlContent = fileContent;
    xmlContent.Replace("\r\n", "\n");
    xmlContent.Replace("\r", "\n");

    // Remove SGML header
    int ofxStart = xmlContent.Find("<OFX>");
    if (ofxStart == wxNOT_FOUND)
    {
        wxLogError("No <OFX> tag found in file: %s", filePath);
        return false;
    }
    xmlContent = xmlContent.Mid(ofxStart);

    // Build a well-formed XML string
    wxString newXmlContent = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    wxStringTokenizer tokenizer(xmlContent, "\n");
    wxArrayString openTags; // Stack to track open tags

    while (tokenizer.HasMoreTokens())
    {
        wxString currentLine = tokenizer.GetNextToken().Trim().Trim(false);
        if (currentLine.IsEmpty())
            continue;

        wxLogDebug("Processing line: %s", currentLine);

        if (currentLine.StartsWith("</")) // Closing tag
        {
            wxString tag = currentLine.AfterFirst('/').BeforeFirst('>');
            if (!openTags.IsEmpty() && openTags.Last() == tag)
            {
                openTags.RemoveAt(openTags.GetCount() - 1); // Pop the matching tag
                newXmlContent << currentLine << "\n";
                wxLogDebug("Closed tag: %s", tag);
            }
            else
            {
                wxLogDebug("Unmatched closing tag ignored: %s", tag);
                // Optionally, we could add error handling here
            }
        }
        else if (currentLine.StartsWith("<") && !currentLine.EndsWith("/>")) // Opening tag
        {
            wxString tag = currentLine.AfterFirst('<').BeforeFirst('>');
            if (tag.Contains(" "))
                tag = tag.BeforeFirst(' '); // Strip attributes
            if (!tag.IsEmpty())
            {
                // Check if the line already contains content (e.g., <CODE>0)
                wxString content = currentLine.AfterFirst('>').Trim(false);
                if (!content.IsEmpty() && !content.StartsWith("<"))
                {
                    // This is a tag with immediate content, so close it immediately
                    newXmlContent << currentLine << "</" << tag << ">\n";
                    wxLogDebug("Added tag with content: %s", tag);
                }
                else
                {
                    // Standard opening tag
                    openTags.Add(tag);
                    newXmlContent << currentLine << "\n";
                    wxLogDebug("Added opening tag: %s", tag);
                }
            }
        }
        else
        {
            // Self-closing or already complete tag
            newXmlContent << currentLine << "\n";
            wxLogDebug("Added complete line: %s", currentLine);
        }
    }

    // Close any remaining open tags
    for (int i = openTags.GetCount() - 1; i >= 0; --i)
    {
        wxString tag = openTags[i];
        newXmlContent << "</" << tag << ">\n";
        wxLogDebug("Added missing closing tag: %s", tag);
    }

    wxLogDebug("Preprocessed XML content (first 1000 chars): %s", newXmlContent.Left(1000));

    // Parse with wxXmlDocument
    wxXmlDocument doc;
    wxStringInputStream input(newXmlContent);
    if (!doc.Load(input))
    {
        wxLogError("Failed to parse preprocessed OFX XML content from file: %s", filePath);
        return false;
    }

    wxXmlNode* root = doc.GetRoot();
    if (!root || root->GetName() != "OFX")
    {
        wxLogError("No <OFX> root element found after preprocessing in file: %s", filePath);
        return false;
    }

    // Debug the XML tree
    wxXmlNode* child = root->GetChildren();
    wxString foundTags;
    while (child)
    {
        if (child->GetType() == wxXML_ELEMENT_NODE)
        {
            foundTags << child->GetName() << " ";
            wxLogDebug("Child of <OFX>: %s", child->GetName());
        }
        child = child->GetNext();
    }
    wxLogDebug("All children of <OFX>: %s", foundTags);

    wxXmlNode* bankmsgs = root->GetChildren();
    while (bankmsgs)
    {
        if (bankmsgs->GetType() == wxXML_ELEMENT_NODE && bankmsgs->GetName().Upper() == "BANKMSGSRSV1")
            break;
        bankmsgs = bankmsgs->GetNext();
    }
    if (!bankmsgs)
    {
        wxLogError("No <BANKMSGSRSV1> element found in file: %s", filePath);
        return false;
    }

    wxXmlNode* stmttrnrs = bankmsgs->GetChildren();
    while (stmttrnrs)
    {
        if (stmttrnrs->GetType() == wxXML_ELEMENT_NODE && stmttrnrs->GetName().Upper() == "STMTTRNRS")
            break;
        stmttrnrs = stmttrnrs->GetNext();
    }
    if (!stmttrnrs)
    {
        wxLogError("No <STMTTRNRS> element found in file: %s", filePath);
        return false;
    }

    wxXmlNode* stmtrs = stmttrnrs->GetChildren();
    while (stmtrs)
    {
        if (stmtrs->GetType() == wxXML_ELEMENT_NODE && stmtrs->GetName().Upper() == "STMTRS")
            break;
        stmtrs = stmtrs->GetNext();
    }
    if (!stmtrs)
    {
        wxLogError("No <STMTRS> element found in file: %s", filePath);
        return false;
    }

    wxXmlNode* banktranlist = stmtrs->GetChildren();
    while (banktranlist)
    {
        if (banktranlist->GetType() == wxXML_ELEMENT_NODE && banktranlist->GetName().Upper() == "BANKTRANLIST")
            break;
        banktranlist = banktranlist->GetNext();
    }
    if (!banktranlist)
    {
        wxLogError("No <BANKTRANLIST> element found in file: %s", filePath);
        return false;
    }

    return ImportTransactions(banktranlist, account_id_, importResults, stats);
}




wxString mmOFXImportDialog::getPayeeName(const wxString& memo, bool& usedRegex, wxString& matchedPattern)
{
    wxString decodedMemo = DecodeHTMLEntities(memo);
    wxString payeeName = decodedMemo;
    payeeName.Trim().Trim(false);
    wxLogDebug("getPayeeName called with memo: '%s' (decoded: '%s', trimmed: '%s')", memo, decodedMemo, payeeName);
    if (payeeName.IsEmpty())
    {
        wxLogDebug("Memo is empty after trimming, returning 'Unknown Payee'");
        usedRegex = false;
        matchedPattern = wxEmptyString;
        return "Unknown Payee";
    }

    if (!payeeRegExCheckBox_->IsChecked())
    {
        wxLogDebug("Regex checkbox is unchecked, skipping regex matching");
        usedRegex = false;
        matchedPattern = wxEmptyString;
        return payeeName;
    }

    wxLogDebug("Regex checkbox is checked, attempting to match %zu patterns", payeeRegexMap_.size());
    for (const auto& pair : payeeRegexMap_)
    {
        wxLogDebug("Testing pattern '%s' against memo '%s'", pair.first, payeeName);
        wxRegEx re(pair.first, wxRE_ADVANCED | wxRE_ICASE);
        if (!re.IsValid())
        {
            wxLogDebug("Invalid regex pattern '%s' for payee '%s'", pair.first, pair.second);
            continue;
        }
        if (re.Matches(payeeName))
        {
            wxLogDebug("Matched regex '%s' to payee '%s' for memo '%s'", pair.first, pair.second, payeeName);
            usedRegex = true;
            matchedPattern = pair.first;
            return pair.second;
        }
        else
        {
            wxLogDebug("Pattern '%s' did not match memo '%s'", pair.first, payeeName);
        }
    }

    wxLogDebug("No regex patterns matched, returning original payee name '%s'", payeeName);
    usedRegex = false;
    matchedPattern = wxEmptyString;
    return payeeName;
}

bool mmOFXImportDialog::ImportTransactions(wxXmlNode* banktranlist, wxLongLong accountID, std::vector<OFXImportResult>& results, OFXImportStats& stats)
{
    Model_Account::Data* account = Model_Account::instance().get(accountID.GetValue());
    if (!account)
    {
        wxLogError("Account ID %lld does not exist in the database", accountID.GetValue());
        wxMessageBox(wxString::Format("Account ID %lld does not exist.", accountID.GetValue()), "Error", wxOK | wxICON_ERROR);
        return false;
    }

    stats.autoImportedCount = 0;
    stats.newPayeesCreated = 0;
    stats.manuallyAllocated = 0;
    stats.totalTransactions = 0;

    int totalTransactions = 0;
    for (wxXmlNode* node = banktranlist->GetChildren(); node; node = node->GetNext())
    {
        if (node->GetType() == wxXML_ELEMENT_NODE && node->GetName().Upper() == "STMTTRN")
            totalTransactions++;
    }

    int transactionIndex = 0;
    for (wxXmlNode* stmttrn = banktranlist->GetChildren(); stmttrn; stmttrn = stmttrn->GetNext())
    {
        if (stmttrn->GetType() != wxXML_ELEMENT_NODE || stmttrn->GetName().Upper() != "STMTTRN")
            continue;

        wxString fitid, dtposted, trnamt, memo;
        wxXmlNode* child = stmttrn->GetChildren();
        while (child)
        {
            if (child->GetType() == wxXML_ELEMENT_NODE)
            {
                wxString name = child->GetName().Upper();
                if (name == "FITID")
                    fitid = child->GetNodeContent();
                else if (name == "DTPOSTED")
                    dtposted = child->GetNodeContent();
                else if (name == "TRNAMT")
                    trnamt = child->GetNodeContent();
                else if (name == "MEMO")
                    memo = child->GetNodeContent();
            }
            child = child->GetNext();
        }

        OFXImportResult result;
        result.fitid = fitid;
        result.date = dtposted.IsEmpty() ? wxEmptyString : dtposted.Left(8);
        result.amount = trnamt.IsEmpty() ? "0" : trnamt;
        result.ofxPayee = memo.IsEmpty() ? "Unknown Payee" : memo;

        if (fitid.IsEmpty() || dtposted.IsEmpty() || trnamt.IsEmpty())
        {
            result.imported = false;
            result.category = "N/A";
            result.transType = "N/A";
            results.push_back(result);
            stats.totalTransactions++;
            transactionIndex++;
            continue;
        }

        // Check for duplicates or transfers using FITID
        wxString fitidStr = fitid;
        Model_Checking::Data_Set existingTxs = Model_Checking::instance().find(Model_Checking::TRANSACTIONNUMBER(fitidStr));
        Model_Checking::Data* existingTx = nullptr;
        bool isTransfer = false;
        wxLongLong otherAccountId = -1;
        bool hasDuplicate = false;

        for (auto& tx : existingTxs)
        {
            if (tx.ACCOUNTID == account->ACCOUNTID)
            {
                hasDuplicate = true;
                result.imported = false;
                result.importedPayee = "Duplicate";
                result.category = "N/A";
                result.transType = "N/A";
                break;
            }
            else if (!existingTx)
            {
                existingTx = &tx;
                otherAccountId = tx.ACCOUNTID;
                isTransfer = true;
            }
        }

        if (hasDuplicate)
        {
            results.push_back(result);
            stats.totalTransactions++;
            transactionIndex++;
            continue;
        }

        // Handle transfers
        if (isTransfer && existingTx)
        {
            for (auto& tx : existingTxs)
            {
                if (tx.ACCOUNTID != account->ACCOUNTID && tx.TRANSID != existingTx->TRANSID)
                {
                    wxLogError("Multiple transactions with FITID %s found across accounts. Using first match (TRANSID %lld).", fitidStr,
                               existingTx->TRANSID.GetValue());
                }
            }

            existingTx->TRANSCODE = Model_Checking::TYPE_NAME_TRANSFER;
            existingTx->TOACCOUNTID = account->ACCOUNTID;
            double amount;
            trnamt.ToDouble(&amount);
            existingTx->TOTRANSAMOUNT = (existingTx->TRANSAMOUNT < 0) ? -existingTx->TRANSAMOUNT : existingTx->TRANSAMOUNT;
            existingTx->TRANSAMOUNT = existingTx->TOTRANSAMOUNT;
            existingTx->PAYEEID = -1;
            existingTx->CATEGID = static_cast<int>(transferCategId_);
            wxDateTime date;
            date.ParseFormat(dtposted.Left(8), "%Y%m%d");
            existingTx->TRANSDATE = date.FormatISOCombined();
            wxLongLong txId = Model_Checking::instance().save(existingTx);
            result.imported = (txId.GetValue() > 0);
            result.importedPayee = "Transfer to " + Model_Account::instance().get(otherAccountId.GetValue())->ACCOUNTNAME;
            result.category = "Transfer";
            result.transType = "Transfer";
            if (!result.imported)
            {
                wxLogError("Failed to save transfer transaction with FITID: %s", fitidStr);
            }
            results.push_back(result);
            stats.totalTransactions++;
            transactionIndex++;
            continue;
        }

        // Process new transaction
        double amount;
        if (!trnamt.ToDouble(&amount))
        {
            result.imported = false;
            result.category = "N/A";
            result.transType = "N/A";
            results.push_back(result);
            stats.totalTransactions++;
            transactionIndex++;
            continue;
        }

        wxDateTime date;
        if (!date.ParseFormat(dtposted.Left(8), "%Y%m%d"))
        {
            result.imported = false;
            result.category = "N/A";
            result.transType = "N/A";
            results.push_back(result);
            stats.totalTransactions++;
            transactionIndex++;
            continue;
        }

        wxString transType = (amount >= 0) ? "Deposit" : "Withdrawal";
        bool usedRegex = false;
        wxString matchedPattern;
        wxString payeeName = getPayeeName(memo, usedRegex, matchedPattern);
        result.usedRegex = usedRegex;
        result.importedPayee = payeeName;
        result.regexPattern = matchedPattern;

        wxLongLong payeeId = -1;
        long long categId = -1;
        Model_Payee::Data* payee = Model_Payee::instance().get_one(Model_Payee::PAYEENAME(payeeName));
        bool payeeExisted = (payee != nullptr);

        if (!payee)
        {
            mmPayeeSelectionDialog payeeDlg(this, memo, payeeName, fitidStr, date.FormatISODate(), wxString::Format("%.2f", amount),
                                            transType, transactionIndex, totalTransactions, importStartTime_);
            int modalResult = payeeDlg.ShowModal();
            if (modalResult == wxID_OK)
            {
                wxString selectedPayee = payeeDlg.GetSelectedPayee();
                categId = payeeDlg.GetSelectedCategoryID();
                if (!payeeDlg.IsCreateNewPayee())
                {
                    payee = Model_Payee::instance().get_one(Model_Payee::PAYEENAME(selectedPayee));
                    stats.manuallyAllocated++;
                }
                else
                {
                    stats.newPayeesCreated++;
                    stats.manuallyAllocated++;
                    payee = Model_Payee::instance().get_one(Model_Payee::PAYEENAME(selectedPayee));
                }
                if (payee)
                {
                    payeeId = payee->PAYEEID;
                    result.importedPayee = payee->PAYEENAME;
                }
            }
            else
            {
                break; // User canceled
            }
        }
        else if (usedRegex)
        {
            payeeId = payee->PAYEEID;
            categId = payee->CATEGID.GetValue();
            result.importedPayee = payee->PAYEENAME;
            stats.autoImportedCount++;
        }
        else
        {
            mmPayeeSelectionDialog payeeDlg(this, memo, payeeName, fitidStr, date.FormatISODate(), wxString::Format("%.2f", amount),
                                            transType, transactionIndex, totalTransactions, importStartTime_);
            int modalResult = payeeDlg.ShowModal();
            if (modalResult == wxID_OK)
            {
                wxString selectedPayee = payeeDlg.GetSelectedPayee();
                categId = payeeDlg.GetSelectedCategoryID();
                payee = Model_Payee::instance().get_one(Model_Payee::PAYEENAME(selectedPayee));
                if (payee)
                {
                    payeeId = payee->PAYEEID;
                    result.importedPayee = payee->PAYEENAME;
                }
                stats.manuallyAllocated++;
            }
            else
            {
                break; // User canceled
            }
        }

        if (categId != -1)
        {
            Model_Category::Data* category = Model_Category::instance().get(static_cast<long long>(categId));
            result.category = category ? Model_Category::full_name(categId) : "Unknown";
            wxLogDebug("Transaction category set to: %s (ID: %lld)", result.category, categId);
        }
        else
        {
            result.category = "Uncategorized";
            wxLogDebug("No category assigned, using Uncategorized");
        }

        Model_Checking::Data* newTx = Model_Checking::instance().create();
        newTx->ACCOUNTID = account->ACCOUNTID;
        newTx->TRANSDATE = date.FormatISOCombined();
        newTx->TRANSACTIONNUMBER = fitidStr;
        newTx->PAYEEID = payeeId.GetValue();
        newTx->CATEGID = categId;
        newTx->STATUS = Model_Checking::STATUS_KEY_NONE;
        newTx->NOTES = memo;

        if (amount >= 0)
        {
            newTx->TRANSCODE = Model_Checking::TYPE_NAME_DEPOSIT;
            newTx->TRANSAMOUNT = amount;
            newTx->TOTRANSAMOUNT = amount;
            result.transType = "Deposit";
        }
        else
        {
            newTx->TRANSCODE = Model_Checking::TYPE_NAME_WITHDRAWAL;
            newTx->TRANSAMOUNT = -amount;
            newTx->TOTRANSAMOUNT = -amount;
            result.transType = "Withdrawal";
        }

        wxLongLong txId = Model_Checking::instance().save(newTx);
        result.imported = (txId.GetValue() > 0);
        if (!result.imported)
        {
            wxLogError("Failed to save transaction with FITID: %s", fitidStr);
        }
        results.push_back(result);
        stats.totalTransactions++;
        transactionIndex++;
    }

    return true;
}




mmOFXImportSummaryDialog::mmOFXImportSummaryDialog(wxWindow* parent, const std::vector<OFXImportResult>& results, const OFXImportStats& stats,
                                                   wxLongLong importStartTime)
    : wxDialog(parent, wxID_ANY, _("OFX Import Summary"), wxDefaultPosition,
               wxSize(wxSystemSettings::GetMetric(wxSYS_SCREEN_X) / 1.75, wxSystemSettings::GetMetric(wxSYS_SCREEN_Y) / 2),
               wxCAPTION | wxRESIZE_BORDER | wxCLOSE_BOX),
      autoImportedCount_(stats.autoImportedCount), newPayeesCreated_(stats.newPayeesCreated), manuallyAllocated_(stats.manuallyAllocated),
      totalTransactions_(stats.totalTransactions), importStartTime_(importStartTime), scrolledWindow(nullptr)
{
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    wxLongLong importEndTime = wxGetUTCTimeMillis();
    wxLongLong elapsedTimeMs = importEndTime - importStartTime_;
    double elapsedTimeSec = elapsedTimeMs.ToDouble() / 1000.0;

    wxStaticText* statsText = new wxStaticText(this, wxID_ANY,
                                               wxString::Format(_("Import Statistics:\n"
                                                                  "- Total Transactions: %d\n"
                                                                  "- Automatically Imported (via Regex): %d\n"
                                                                  "- New Payees Created: %d\n"
                                                                  "- Manually Allocated to Payees: %d\n"
                                                                  "- Time Taken: %s"),
                                                                totalTransactions_, autoImportedCount_, newPayeesCreated_, manuallyAllocated_, FormatTimeTaken(elapsedTimeSec)));
    mainSizer->Add(statsText, 0, wxALL | wxEXPAND, 10);

    scrolledWindow = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL);
    scrolledWindow->SetScrollRate(0, 10);

    wxGrid* grid = new wxGrid(scrolledWindow, wxID_ANY);
    grid->CreateGrid(0, 9);
    grid->SetColLabelValue(0, _("FITID"));
    grid->SetColLabelValue(1, _("Date"));
    grid->SetColLabelValue(2, _("Amount"));
    grid->SetColLabelValue(3, _("OFX Payee"));
    grid->SetColLabelValue(4, _("Imported Payee"));
    grid->SetColLabelValue(5, _("Used Regex"));
    grid->SetColLabelValue(6, _("Regex Pattern"));
    grid->SetColLabelValue(7, _("Category"));
    grid->SetColLabelValue(8, _("Transaction Type"));

    int row = 0;
    for (const auto& result : results)
    {
        grid->AppendRows(1);
        grid->SetCellValue(row, 0, result.fitid);
        grid->SetCellValue(row, 1, result.date);
        grid->SetCellValue(row, 2, result.amount);
        grid->SetCellValue(row, 3, result.ofxPayee);
        grid->SetCellValue(row, 4, result.importedPayee);
        grid->SetCellValue(row, 5, result.usedRegex ? "Yes" : "No");
        grid->SetCellValue(row, 6, result.usedRegex ? result.regexPattern : wxEmptyString);
        grid->SetCellValue(row, 7, result.category);
        grid->SetCellValue(row, 8, result.transType);

        if (!result.imported)
        {
            for (int col = 0; col < 9; col++)
            {
                grid->SetCellBackgroundColour(row, col, *wxLIGHT_GREY);
            }
        }
        row++;
    }

    grid->AutoSizeRows(true);
    grid->AutoSizeColumns();
    grid->EnableEditing(false);

    wxBoxSizer* scrolledSizer = new wxBoxSizer(wxVERTICAL);
    scrolledSizer->Add(grid, 1, wxEXPAND);
    scrolledWindow->SetSizer(scrolledSizer);

    mainSizer->Add(scrolledWindow, 1, wxALL | wxEXPAND, 5);

    wxButton* okButton = new wxButton(this, wxID_OK, _("OK"));
    mainSizer->Add(okButton, 0, wxALIGN_CENTER | wxALL, 5);

    SetSizer(mainSizer);
    SetSize(wxSystemSettings::GetMetric(wxSYS_SCREEN_X) / 1.75, wxSystemSettings::GetMetric(wxSYS_SCREEN_Y) / 2);

    grid->Fit();
    int gridHeight = grid->GetColLabelSize();
    for (int i = 0; i < grid->GetNumberRows(); ++i)
    {
        gridHeight += grid->GetRowSize(i);
    }
    wxSize gridSize = grid->GetSize();
    int gridWidth = gridSize.GetWidth();
    scrolledWindow->SetVirtualSize(gridWidth, gridHeight);
    scrolledWindow->FitInside();
    scrolledWindow->ShowScrollbars(wxSHOW_SB_NEVER, wxSHOW_SB_ALWAYS);
    Layout();

    scrolledWindow->Bind(wxEVT_MOUSEWHEEL,
                         [this](wxMouseEvent& event)
                         {
                             int delta = event.GetWheelDelta();
                             int rotation = event.GetWheelRotation();
                             int lines = rotation / delta;

                             int scrollPos = scrolledWindow->GetScrollPos(wxVERTICAL);
                             scrolledWindow->Scroll(-1, scrollPos - lines);

                             event.Skip();
                         });

    grid->Bind(wxEVT_MOUSEWHEEL, &mmOFXImportSummaryDialog::OnGridMouseWheel, this);
}

void mmOFXImportSummaryDialog::OnGridMouseWheel(wxMouseEvent& event)
{
    scrolledWindow->GetEventHandler()->ProcessEvent(event);
}

void mmOFXImportSummaryDialog::OnOK(wxCommandEvent& /*event*/)
{
    EndModal(wxID_OK);
}


wxString mmOFXImportSummaryDialog::FormatTimeTaken(double seconds) const
{
    int hours = static_cast<int>(seconds / 3600);            // Total hours
    int remainingSeconds = static_cast<int>(seconds) % 3600; // Seconds after hours
    int minutes = remainingSeconds / 60;                     // Minutes from remaining seconds
    int secs = remainingSeconds % 60;                        // Seconds after minutes

    wxString result;
    if (hours > 0)
    {
        result += wxString::Format(_("%d hours"), hours);
    }
    if (minutes > 0 || hours > 0) // Show minutes if there are hours, even if minutes is 0
    {
        if (!result.IsEmpty())
            result += " ";
        result += wxString::Format(_("%d minutes"), minutes);
    }
    if (secs > 0 || minutes > 0 || hours > 0) // Show seconds if there are hours or minutes, even if seconds is 0
    {
        if (!result.IsEmpty())
            result += " ";
        result += wxString::Format(_("%d seconds"), secs);
    }
    if (result.IsEmpty()) // If all components are 0, show 0sec
    {
        result = wxString::Format(_("0 sec"));
    }
    return result;
}