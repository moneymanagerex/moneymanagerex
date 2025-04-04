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

#include "tinyxml2.cpp"
#include <wx/grid.h>

#include "model/Model_Account.h"
#include "model/Model_Category.h"
#include "model/Model_Checking.h"
#include "model/Model_Payee.h"
#include "ofx_import_gui.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include <climits>
#include <wx/file.h>
#include <wx/filedlg.h>
#include <wx/log.h>
#include <wx/msgdlg.h>
#include <wx/regex.h>
#include <wx/settings.h>
#include <wx/timectrl.h>

BEGIN_EVENT_TABLE(mmOFXImportDialog, wxDialog)
EVT_BUTTON(wxID_OPEN, mmOFXImportDialog::OnBrowse)
EVT_BUTTON(wxID_OK, mmOFXImportDialog::OnImport)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(mmOFXImportSummaryDialog, wxDialog)
EVT_BUTTON(wxID_OK, mmOFXImportSummaryDialog::OnOK)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(mmPayeeSelectionDialog, wxDialog)
EVT_RADIOBUTTON(ID_USE_EXISTING, mmPayeeSelectionDialog::OnUseExistingPayee)
EVT_RADIOBUTTON(ID_CREATE_NEW, mmPayeeSelectionDialog::OnCreateNewPayee)
EVT_CHECKBOX(wxID_ANY, mmPayeeSelectionDialog::OnUpdateRegex)
EVT_BUTTON(wxID_OK, mmPayeeSelectionDialog::OnOK)
EVT_CHOICE(wxID_ANY, mmPayeeSelectionDialog::OnPayeeChoice)
EVT_BUTTON(ID_TITLE_CASE, mmPayeeSelectionDialog::OnTitleCase)
EVT_GRID_LABEL_LEFT_CLICK(mmPayeeSelectionDialog::OnGridLabelLeftClick)
EVT_BUTTON(ID_UPDATE_CATEGORY, mmPayeeSelectionDialog::OnUpdateCategoryToggle)
EVT_BUTTON(ID_INSERT_ROW, mmPayeeSelectionDialog::OnInsertRow) // Added
EVT_BUTTON(ID_DELETE_ROW, mmPayeeSelectionDialog::OnDeleteRow) // Added
END_EVENT_TABLE()



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

// mmPayeeSelectionDialog Implementation
mmPayeeSelectionDialog::mmPayeeSelectionDialog(wxWindow* parent, const wxString& memo, const wxString& suggestedPayeeName, const wxString& fitid,
                                               const wxString& date, const wxString& amount, const wxString& transType, int currentTransaction,
                                               int totalTransactions, wxLongLong importStartTime)
    : wxDialog(parent, wxID_ANY, _("Existing Payee Not Found"), wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER),
      selectedPayee_(suggestedPayeeName), regexPattern_(memo), shouldUpdateRegex_(false), categoryChoice_(nullptr), currentTransaction_(currentTransaction),
      totalTransactions_(totalTransactions), importStartTime_(importStartTime), updatePayeeCategory_(false), okButton_(nullptr), existingPayeeLabel_(nullptr),
      newPayeeLabel_(nullptr), payeeSizer_(nullptr), initialCategoryId_(-1), memoAdded_(false), insertRowButton_(nullptr), deleteRowButton_(nullptr),
      categoryManuallyChanged_(false)
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
    for (const auto& payee : Model_Payee::instance().all(Model_Payee::COL_PAYEENAME))
    {
        payeeChoice_->Append(payee.PAYEENAME);
    }
    payeeSizer_->Add(existingPayeeLabel_, 0, wxLEFT | wxRIGHT | wxTOP, 5);
    payeeSizer_->Add(payeeChoice_, 0, wxALL | wxEXPAND, 2);

    newPayeeLabel_ = new wxStaticText(this, wxID_ANY, _("New Payee Name:"));
    newPayeeTextCtrl_ = new wxTextCtrl(this, wxID_ANY, suggestedPayeeName);
    titleCaseButton_ = new wxButton(this, ID_TITLE_CASE, "T", wxDefaultPosition, wxSize(25, -1));
    titleCaseButton_->SetToolTip(_("Convert to Title Case"));
    wxBoxSizer* newPayeeInnerSizer = new wxBoxSizer(wxHORIZONTAL);
    newPayeeInnerSizer->Add(newPayeeTextCtrl_, 1, wxEXPAND, 0);
    newPayeeInnerSizer->Add(titleCaseButton_, 0, wxLEFT | wxALIGN_CENTER_VERTICAL, 5);

    // Initially add only existing payee controls to payeeSizer_
    mainSizer->Add(payeeSizer_, 0, wxEXPAND);

    // Select Category
    categoryChoice_ = new wxChoice(this, wxID_ANY);
    categoryChoice_->Append(_("Uncategorized"), new wxStringClientData("-1"));
    Model_Category::Data_Set categories = Model_Category::instance().all(Model_Category::COL_CATEGNAME);
    categoryMap.clear();
    for (const auto& cat : categories)
    {
        categoryMap[cat.CATEGID.GetValue()] = cat; // Fixed: Use GetValue()
    }
    for (const auto& cat : categories)
    {
        if (cat.PARENTID.GetValue() == -1) // Fixed: Use GetValue()
        {
            AddCategoryToChoice(categoryChoice_, cat.CATEGID.GetValue(), categoryMap, 0);
        }
    }
    categoryChoice_->SetSelection(0);
    wxLogDebug("Category choice initialized with %u items, selected index: %d", categoryChoice_->GetCount(), categoryChoice_->GetSelection());

    updateCategoryButton_ = new wxButton(this, ID_UPDATE_CATEGORY, "P", wxDefaultPosition, wxSize(25, -1));
    updateCategoryButton_->SetToolTip(_("Toggle to update payee's default category (P) or apply to this transaction only"));
    wxBoxSizer* categoryButtonSizer = new wxBoxSizer(wxHORIZONTAL);
    categoryButtonSizer->Add(categoryChoice_, 1, wxEXPAND, 0);
    categoryButtonSizer->Add(updateCategoryButton_, 0, wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
    mainSizer->Add(new wxStaticText(this, wxID_ANY, _("Select Category for Payee:")), 0, wxLEFT | wxRIGHT | wxTOP, 5);
    mainSizer->Add(categoryButtonSizer, 0, wxALL | wxEXPAND, 2);

    // Regex Patterns
    updateRegexCheckBox_ = new wxCheckBox(this, wxID_ANY, _("Define/Update Regex Patterns"));
    mainSizer->Add(updateRegexCheckBox_, 0, wxALL, 5);

    regexGrid_ = new wxGrid(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL | wxVSCROLL);
    regexGrid_->CreateGrid(2, 1);
    regexGrid_->SetColLabelValue(0, _("Regex Pattern"));
    regexGrid_->SetDefaultColSize(300);
    regexGrid_->SetColMinimalWidth(0, 50);
    regexGrid_->SetRowSize(0, regexGrid_->GetDefaultRowSize());
    regexGrid_->SetRowSize(1, regexGrid_->GetDefaultRowSize());
    regexGrid_->EnableGridLines(true);
    regexGrid_->EnableEditing(true);
    regexGrid_->SetDefaultCellBackgroundColour(*wxWHITE);
    regexGrid_->SetLabelBackgroundColour(GetBackgroundColour());
    regexGrid_->SetDefaultEditor(new wxGridCellTextEditor());
    mainSizer->Add(new wxStaticText(this, wxID_ANY, _("Edit Regex Patterns:")), 0, wxLEFT | wxRIGHT | wxTOP, 5);
    mainSizer->Add(regexGrid_, 1, wxALL | wxEXPAND, 2);

    // Add Insert and Delete buttons for the regex grid
    wxBoxSizer* regexButtonSizer = new wxBoxSizer(wxHORIZONTAL);
    insertRowButton_ = new wxButton(this, ID_INSERT_ROW, _("Insert Row"));
    deleteRowButton_ = new wxButton(this, ID_DELETE_ROW, _("Delete Row"));
    regexButtonSizer->Add(insertRowButton_, 0, wxRIGHT, 5);
    regexButtonSizer->Add(deleteRowButton_, 0, wxRIGHT, 5);
    mainSizer->Add(regexButtonSizer, 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);

    // Buttons
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    okButton_ = new wxButton(this, wxID_OK, _("OK"));
    wxLogDebug("okButton_ created at %p", okButton_);
    buttonSizer->Add(okButton_, 0, wxALL, 5);
    buttonSizer->Add(new wxButton(this, wxID_CANCEL, _("Cancel")), 0, wxALL, 5);
    mainSizer->Add(buttonSizer, 0, wxALIGN_CENTER | wxALL, 5);

    SetSizer(mainSizer);
    SetMinSize(wxSize(450, 680));
    SetSize(wxSize(450, 680));
    Layout();

    // Initial state
    payeeChoice_->Enable(true);
    existingPayeeLabel_->Show(true);
    payeeChoice_->Show(true);
    newPayeeTextCtrl_->Enable(false);
    titleCaseButton_->Enable(false);
    newPayeeLabel_->Show(false);
    newPayeeInnerSizer->Show(false);
    updateCategoryButton_->Enable(true);
    updateCategoryButton_->Show(true);
    updateRegexCheckBox_->Enable(true);
    regexGrid_->EnableEditing(updateRegexCheckBox_->IsChecked());
    regexGrid_->Enable(false);
    insertRowButton_->Enable(false);
    deleteRowButton_->Enable(false);

    // Bind after all controls are initialized
    newPayeeTextCtrl_->Bind(wxEVT_TEXT, &mmPayeeSelectionDialog::UpdateOKButton, this);
    categoryChoice_->Bind(wxEVT_CHOICE, &mmPayeeSelectionDialog::OnCategorySelection, this);
    categoryChoice_->Bind(wxEVT_SET_FOCUS, &mmPayeeSelectionDialog::OnCategoryFocus, this);

    // Bind the size event to dynamically resize the grid column
    regexGrid_->Bind(wxEVT_SIZE,
                     [this](wxSizeEvent& event)
                     {
                         int dialogWidth = GetClientSize().GetWidth();
                         int scrollbarWidth = wxSystemSettings::GetMetric(wxSYS_VSCROLL_X);
                         int padding = (10 * 2 + 2 * 2);
                         int availableWidth = dialogWidth - padding - scrollbarWidth - 50;

                         if (availableWidth > 50)
                         {
                             regexGrid_->SetColSize(0, availableWidth);
                         }
                         else
                         {
                             regexGrid_->SetColSize(0, 50);
                         }
                         regexGrid_->ForceRefresh();
                         event.Skip();
                     });

    // Initial column size adjustment
    int dialogWidth = GetClientSize().GetWidth();
    int scrollbarWidth = wxSystemSettings::GetMetric(wxSYS_VSCROLL_X);
    int padding = (10 * 2 + 2 * 2);
    int initialWidth = dialogWidth - padding - scrollbarWidth - 50;
    if (initialWidth > 50)
    {
        regexGrid_->SetColSize(0, initialWidth);
    }
    else
    {
        regexGrid_->SetColSize(0, 50);
    }
    regexGrid_->ForceRefresh();

    wxLogDebug("Calling UpdateOKButton from constructor");
    UpdateOKButton(wxCommandEvent());
    wxLogDebug("UpdateOKButton completed in constructor");

    LoadRegexPatterns(suggestedPayeeName);
    regexGrid_->SetScrollRate(10, 10);

    // Set the initial category ID
    initialCategoryId_ = GetSelectedCategoryID();
    wxLogDebug("Initial category ID set to: %lld", initialCategoryId_);
}


// New event handler for category selection changes
void mmPayeeSelectionDialog::OnCategorySelection(wxCommandEvent& event)
{
    int sel = categoryChoice_->GetSelection();
    long long selectedCategoryId = GetSelectedCategoryID();
    wxLogDebug("OnCategorySelection: Selected index: %d, Category: %s (ID: %lld)", sel, GetSelectedCategory(), selectedCategoryId);

    if (selectedCategoryId == -1) // "Uncategorized"
    {
        updateCategoryButton_->Enable(false);
        updateCategoryButton_->SetToolTip(_("Cannot set 'Uncategorized' as the default category for the payee"));
    }
    else
    {
        updateCategoryButton_->Enable(true);
        updateCategoryButton_->SetToolTip(_("Toggle to update payee's default category (P) or apply to this transaction only"));
    }

    categoryManuallyChanged_ = true; // Mark as manually changed
    wxLogDebug("Set categoryManuallyChanged_ to true due to category selection");

    event.Skip();
}


void mmPayeeSelectionDialog::OnCategoryFocus(wxFocusEvent& event)
{
    wxLogDebug("categoryChoice_ received focus, current selection: %d (%s)", categoryChoice_->GetSelection(), categoryChoice_->GetStringSelection());
    event.Skip();
}





void mmPayeeSelectionDialog::UpdateOKButton(wxCommandEvent& /*event*/)
{
    wxLogDebug("UpdateOKButton called, okButton_ = %p", okButton_);
    if (okButton_)
    {
        if (useExistingRadio_->GetValue())
        {
            okButton_->Enable(!payeeChoice_->GetStringSelection().IsEmpty());
        }
        else
        {
            okButton_->Enable(!newPayeeTextCtrl_->GetValue().IsEmpty());
        }
    }
    else
    {
        wxLogError("okButton_ is null in UpdateOKButton!");
    }
}

void mmPayeeSelectionDialog::OnUseExistingPayee(wxCommandEvent& /*event*/)
{
    wxLogDebug("Switched to 'Use Existing Payee'");
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
    memoAdded_ = false; // Reset memoAdded_ since we're reloading the grid
    regexGrid_->EnableEditing(updateRegexCheckBox_->IsChecked());
    regexGrid_->Enable(true);
    insertRowButton_->Enable(updateRegexCheckBox_->IsChecked());                                    // Update button state
    deleteRowButton_->Enable(updateRegexCheckBox_->IsChecked() && regexGrid_->GetNumberRows() > 0); // Update button state
    selectedPayee_ = payeeChoice_->GetStringSelection();

    if (regexGrid_->GetNumberRows() > 0)
    {
        regexGrid_->DeleteRows(0, regexGrid_->GetNumberRows());
    }
    regexGrid_->AppendRows(1);
    LoadRegexPatterns(selectedPayee_);
    UpdateOKButton(wxCommandEvent());
    OnPayeeChoice(wxCommandEvent());
    Layout();
}





void mmPayeeSelectionDialog::OnCreateNewPayee(wxCommandEvent& /*event*/)
{
    wxLogDebug("Switched to 'Create New Payee', current category selection: %d", categoryChoice_->GetSelection());
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
    insertRowButton_->Enable(true);                            // Grid is editable
    deleteRowButton_->Enable(regexGrid_->GetNumberRows() > 0); // Depends on row count
    selectedPayee_ = newPayeeTextCtrl_->GetValue();

    if (regexGrid_->GetNumberRows() > 0)
    {
        regexGrid_->DeleteRows(0, regexGrid_->GetNumberRows());
    }
    regexGrid_->AppendRows(1);
    regexGrid_->SetCellValue(0, 0, regexPattern_);
    deleteRowButton_->Enable(regexGrid_->GetNumberRows() > 0); // Update after adding row
    UpdateOKButton(wxCommandEvent());
    Layout();
}




void mmPayeeSelectionDialog::OnUpdateCategoryToggle(wxCommandEvent& /*event*/)
{
    updatePayeeCategory_ = !updatePayeeCategory_;
    updateCategoryButton_->SetLabel(updatePayeeCategory_ ? "P*" : "P");
    wxLogDebug("Category update toggle set to: %s", updatePayeeCategory_ ? "Update Payee" : "This Transaction Only");
}

void mmPayeeSelectionDialog::OnUpdateRegex(wxCommandEvent& /*event*/)
{
    shouldUpdateRegex_ = updateRegexCheckBox_->IsChecked();
    regexGrid_->EnableEditing(shouldUpdateRegex_);
    regexGrid_->Enable(true);
    insertRowButton_->Enable(shouldUpdateRegex_);                                    // Enable insert when grid is editable
    deleteRowButton_->Enable(shouldUpdateRegex_ && regexGrid_->GetNumberRows() > 0); // Enable delete when grid is editable and has rows
    if (shouldUpdateRegex_ && useExistingRadio_->GetValue() && !memoAdded_)
    {
        regexGrid_->AppendRows(1);
        int newRow = regexGrid_->GetNumberRows() - 1;
        regexGrid_->SetCellValue(newRow, 0, regexPattern_); // Add memo
        memoAdded_ = true;                                  // Mark the memo as added
        wxLogDebug("Added memo row for existing payee at row %d: %s", newRow, regexPattern_);
        deleteRowButton_->Enable(shouldUpdateRegex_ && regexGrid_->GetNumberRows() > 0); // Update delete button state
    }
}

void mmPayeeSelectionDialog::OnPayeeChoice(wxCommandEvent& event)
{
    wxString selectedPayee = payeeChoice_->GetStringSelection();
    wxLogDebug("OnPayeeChoice triggered for '%s'", selectedPayee);

    Model_Payee::Data* payee = Model_Payee::instance().get(selectedPayee);
    if (payee && !categoryManuallyChanged_) // Only update if not manually changed
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
                    wxLogDebug("Set categoryChoice_ to index %u (ID: %lld, Name: %s) for payee '%s'", i, categId, categoryChoice_->GetString(i), selectedPayee);
                    categoryChoice_->SetSelection(i);
                    break;
                }
            }
        }
    }
    else if (!payee)
    {
        if (!categoryManuallyChanged_) // Only reset to Uncategorized if not manually changed
        {
            categoryChoice_->SetSelection(0);
            wxLogDebug("No payee data found for '%s', defaulting to Uncategorized", selectedPayee);
        }
    }
    else
    {
        wxLogDebug("Preserving manually changed category for payee '%s'", selectedPayee);
    }

    // Reset the flag only when a new payee is intentionally selected
    if (event.GetEventType() == wxEVT_CHOICE) // Ensure this is a direct user selection
    {
        categoryManuallyChanged_ = false; // Reset when switching to existing payee mode
        wxLogDebug("Reset categoryManuallyChanged_ to false in OnUseExistingPayee");
    }

    UpdateOKButton(event);
    event.Skip();
}





void mmPayeeSelectionDialog::OnInsertRow(wxCommandEvent& /*event*/)
{
    if (!regexGrid_->IsEnabled() || !regexGrid_->IsEditable())
    {
        wxLogDebug("Insert Row button clicked, but grid is not editable");
        return;
    }

    // Insert a new row at the end
    regexGrid_->AppendRows(1);
    wxLogDebug("Inserted new row at index %d", regexGrid_->GetNumberRows() - 1);
    deleteRowButton_->Enable(regexGrid_->GetNumberRows() > 0); // Update delete button state
}

void mmPayeeSelectionDialog::OnDeleteRow(wxCommandEvent& /*event*/)
{
    if (!regexGrid_->IsEnabled() || !regexGrid_->IsEditable())
    {
        wxLogDebug("Delete Row button clicked, but grid is not editable");
        return;
    }

    // Get the selected row (use the first selected row if multiple are selected)
    wxArrayInt selectedRows = regexGrid_->GetSelectedRows();
    if (selectedRows.IsEmpty())
    {
        wxMessageBox(_("Please select a row to delete."), _("Error"), wxOK | wxICON_WARNING);
        return;
    }

    int rowToDelete = selectedRows[0];
    regexGrid_->DeleteRows(rowToDelete, 1);
    wxLogDebug("Deleted row %d", rowToDelete);
    deleteRowButton_->Enable(regexGrid_->GetNumberRows() > 0); // Update delete button state
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
    wxLogDebug("Converted '%s' to Title Case: '%s'", text, result);
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
    wxLogDebug("OnOK: Selected payee '%s', Category ID: %lld", selectedPayee_, selectedCategoryId);

    // For existing payee, check if category has changed when updatePayeeCategory_ is true
    bool shouldUpdateCategory = updatePayeeCategory_;
    Model_Payee::Data* payee = Model_Payee::instance().get_one(Model_Payee::PAYEENAME(selectedPayee_));

    if (useExistingRadio_->GetValue() && shouldUpdateCategory && payee)
    {
        if (selectedCategoryId != payee->CATEGID.GetValue())
        {
            wxLogDebug("Category ID %lld for payee '%s' differs from current (ID: %lld), updating", selectedCategoryId, selectedPayee_,
                       payee->CATEGID.GetValue());
            payee->CATEGID = selectedCategoryId;
            Model_Payee::instance().save(payee);
            wxLogDebug("Updated existing payee '%s' category to ID: %lld in database", payee->PAYEENAME, payee->CATEGID.GetValue());
        }
        else
        {
            wxLogDebug("Category ID %lld for payee '%s' matches existing (ID: %lld), no update needed", selectedCategoryId, selectedPayee_,
                       payee->CATEGID.GetValue());
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
                wxLogDebug("Creating new payee '%s' with Category ID: %lld", selectedPayee_, payee->CATEGID.GetValue());
            }
            if (payee)
            {
                payee->PATTERN = regexPattern_;
                Model_Payee::instance().save(payee);
                wxLogDebug("Payee '%s' saved with updated regex. Verifying: CATEGID = %lld", payee->PAYEENAME, payee->CATEGID.GetValue());
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
        wxLogDebug("Creating new payee '%s' (no regex) with Category ID: %lld", selectedPayee_, payee->CATEGID.GetValue());
        Model_Payee::instance().save(payee);
        wxLogDebug("Payee '%s' saved (no regex). Verifying: CATEGID = %lld", payee->PAYEENAME, payee->CATEGID.GetValue());
    }

    // Refresh regex mappings if needed
    mmOFXImportDialog* parentDialog = dynamic_cast<mmOFXImportDialog*>(GetParent());
    if (parentDialog && (shouldUpdateRegex_ || (payee && shouldUpdateCategory)))
    {
        parentDialog->loadRegexMappings();
        wxLogDebug("Updated regex mappings in mmOFXImportDialog after saving payee '%s'", selectedPayee_);
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
        {
            regexGrid_->SetToolTip(cellValue);
        }
        else
        {
            regexGrid_->UnsetToolTip();
        }
    }
    event.Skip();
}

void mmPayeeSelectionDialog::LoadRegexPatterns(const wxString& payeeName)
{
    // Only delete rows if there are rows to delete
    if (regexGrid_->GetNumberRows() > 0)
    {
        regexGrid_->DeleteRows(0, regexGrid_->GetNumberRows());
    }

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
        }
    }
    // Do not add memo here for existing payees; handled in OnUpdateRegex or OnCreateNewPayee
}


void mmPayeeSelectionDialog::AddCategoryToChoice(wxChoice* choice, long long categId, const std::map<long long, Model_Category::Data>& categoryMap, int level)
{
    auto it = categoryMap.find(categId);
    if (it == categoryMap.end())
    {
        wxLogDebug("Category ID %lld not found in category map", categId);
        return;
    }

    const Model_Category::Data& category = it->second;
    wxString indent;
    for (int i = 0; i < level; ++i)
    {
        indent += "  "; // Add two spaces for each level of indentation
    }
    wxString itemText = indent + category.CATEGNAME;

    wxString clientData = wxString::Format("%lld", categId);
    choice->Append(itemText, new wxStringClientData(clientData));
    wxLogDebug("Added category to choice: %s (ID: %lld, Index: %u)", itemText, categId, choice->GetCount() - 1);

    for (const auto& child : categoryMap)
    {
        if (child.second.PARENTID.GetValue() == categId)
        {
            AddCategoryToChoice(choice, child.first, categoryMap, level + 1);
        }
    }
}


wxString mmPayeeSelectionDialog::GetSelectedCategory() const
{
    int sel = categoryChoice_->GetSelection();
    if (sel == 0)
        return _("Uncategorized");
    return categoryChoice_->GetString(sel).Trim(false);
}

long long mmPayeeSelectionDialog::GetSelectedCategoryID() const
{
    int sel = categoryChoice_->GetSelection();
    if (sel == wxNOT_FOUND)
    {
        wxLogDebug("No category selected in mmPayeeSelectionDialog");
        return -1;
    }
    wxClientData* baseData = categoryChoice_->GetClientObject(sel);
    if (!baseData)
    {
        wxLogDebug("No client data for selected category at index %d in mmPayeeSelectionDialog", sel);
        return -1;
    }
    wxStringClientData* data = dynamic_cast<wxStringClientData*>(baseData);
    if (!data)
    {
        wxLogDebug("Failed to cast client data to wxStringClientData for selection at index %d", sel);
        return -1;
    }
    long long categId;
    if (!data->GetData().ToLongLong(&categId))
    {
        wxLogDebug("Failed to convert category ID string '%s' to long long for selection at index %d", data->GetData(), sel);
        return -1;
    }
    wxLogDebug("Selected category ID: %lld (Name: %s)", categId, categoryChoice_->GetString(sel));
    return categId;
}

// mmOFXImportDialog Implementation
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
    wxLogDebug("Loading regex mappings from PAYEE_V1...");
    Model_Payee::Data_Set payees = Model_Payee::instance().all();
    for (const auto& payee : payees)
    {
        if (!payee.PATTERN.IsEmpty())
        {
            wxLogDebug("Processing payee '%s' with PATTERN: %s", payee.PAYEENAME, payee.PATTERN);
            rapidjson::Document j_doc;
            j_doc.Parse(payee.PATTERN.mb_str());
            if (!j_doc.HasParseError() && j_doc.IsObject())
            {
                for (rapidjson::Value::ConstMemberIterator itr = j_doc.MemberBegin(); itr != j_doc.MemberEnd(); ++itr)
                {
                    if (itr->value.IsString())
                    {
                        wxString pattern = wxString::FromUTF8(itr->value.GetString());
                        wxLogDebug("Extracted pattern from key '%s': '%s'", itr->name.GetString(), pattern);
                        if (!pattern.IsEmpty())
                        {
                            wxString normalizedPattern = pattern;
                            if (normalizedPattern.Contains("*"))
                            {
                                normalizedPattern.Replace("*", ".*", true);
                                normalizedPattern.Replace("..*", ".*", true);
                                wxLogDebug("Normalized '*' to '.*': '%s'", normalizedPattern);
                            }
                            payeeRegexMap_[normalizedPattern] = payee.PAYEENAME;
                            wxLogDebug("Loaded: '%s' (original: '%s') -> '%s'", normalizedPattern, pattern, payee.PAYEENAME);
                        }
                    }
                }
            }
            else
            {
                wxLogDebug("Failed to parse PATTERN JSON for payee '%s': %s (Error at offset %zu)", payee.PAYEENAME, payee.PATTERN, j_doc.GetErrorOffset());
            }
        }
    }

    wxLogDebug("Final payeeRegexMap_ contents:");
    for (const auto& pair : payeeRegexMap_)
    {
        wxLogDebug("Pattern: '%s' -> Payee: '%s'", pair.first, pair.second);
    }
}

void mmOFXImportDialog::OnBrowse(wxCommandEvent& /*event*/)
{
    wxFileDialog dlg(this, _("Choose OFX file"), wxEmptyString, wxEmptyString, "OFX files (*.ofx)|*.ofx", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dlg.ShowModal() == wxID_OK)
    {
        fileNameCtrl_->SetValue(dlg.GetPath());

        wxFile file(dlg.GetPath());
        if (!file.IsOpened())
        {
            wxLogError("Failed to open OFX file: %s", dlg.GetPath());
            return;
        }

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
        return false;
    }

    wxString fileContent;
    wxFileOffset length = file.Length();
    wxCharBuffer buffer(length);
    file.Read(buffer.data(), length);
    fileContent = wxString::FromUTF8(buffer.data(), length);

    wxString xmlContent = fileContent;
    xmlContent.Replace("\r\n", "\n");
    xmlContent.Replace("\r", "\n");
    wxString newXmlContent;
    wxStringTokenizer tokenizer(xmlContent, "\n");
    while (tokenizer.HasMoreTokens())
    {
        wxString currentLine = tokenizer.GetNextToken().Trim().Trim(false);
        if (currentLine.StartsWith("<") && !currentLine.StartsWith("</") && !currentLine.EndsWith(">"))
        {
            wxString tag = currentLine.SubString(1, currentLine.Length() - 1);
            if (tag.Contains(">"))
            {
                tag = tag.BeforeFirst('>');
            }
            newXmlContent << currentLine << "\n";
            if (!tag.IsEmpty() && tag != "OFX" && tag != "SIGNONMSGSRSV1" && tag != "SONRS" && tag != "STATUS" && tag != "BANKMSGSRSV1" && tag != "STMTTRNRS" &&
                tag != "STMTRS" && tag != "BANKACCTFROM" && tag != "BANKTRANLIST" && tag != "STMTTRN" && tag != "LEDGERBAL" && tag != "AVAILBAL")
            {
                newXmlContent << "</" << tag << ">\n";
            }
        }
        else
        {
            newXmlContent << currentLine << "\n";
        }
    }

    tinyxml2::XMLDocument doc;
    if (doc.Parse(newXmlContent.mb_str()) != tinyxml2::XML_SUCCESS)
    {
        return false;
    }

    tinyxml2::XMLElement* root = doc.FirstChildElement("OFX");
    if (!root)
        return false;
    tinyxml2::XMLElement* bankmsgs = root->FirstChildElement("BANKMSGSRSV1");
    if (!bankmsgs)
        return false;
    tinyxml2::XMLElement* stmttrnrs = bankmsgs->FirstChildElement("STMTTRNRS");
    if (!stmttrnrs)
        return false;
    tinyxml2::XMLElement* stmtrs = stmttrnrs->FirstChildElement("STMTRS");
    if (!stmtrs)
        return false;
    tinyxml2::XMLElement* banktranlist = stmtrs->FirstChildElement("BANKTRANLIST");
    if (!banktranlist)
        return false;

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

bool mmOFXImportDialog::ImportTransactions(tinyxml2::XMLElement* banktranlist, wxLongLong accountID, std::vector<OFXImportResult>& results,
                                           OFXImportStats& stats)
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
    for (tinyxml2::XMLElement* stmttrn = banktranlist->FirstChildElement("STMTTRN"); stmttrn; stmttrn = stmttrn->NextSiblingElement("STMTTRN"))
    {
        totalTransactions++;
    }

    int transactionIndex = 0;
    for (tinyxml2::XMLElement* stmttrn = banktranlist->FirstChildElement("STMTTRN"); stmttrn; stmttrn = stmttrn->NextSiblingElement("STMTTRN"))
    {
        const char* fitid = stmttrn->FirstChildElement("FITID") ? stmttrn->FirstChildElement("FITID")->GetText() : "";
        const char* dtposted = stmttrn->FirstChildElement("DTPOSTED") ? stmttrn->FirstChildElement("DTPOSTED")->GetText() : "";
        const char* trnamt = stmttrn->FirstChildElement("TRNAMT") ? stmttrn->FirstChildElement("TRNAMT")->GetText() : "0";
        const char* memo = stmttrn->FirstChildElement("MEMO") ? stmttrn->FirstChildElement("MEMO")->GetText() : "Unknown Payee";

        OFXImportResult result;
        result.fitid = wxString::FromUTF8(fitid);
        result.date = dtposted ? wxString(dtposted).Left(8) : wxEmptyString;
        result.amount = wxString::FromUTF8(trnamt);
        result.ofxPayee = wxString::FromUTF8(memo);

        if (!fitid || !dtposted || !trnamt)
        {
            result.imported = false;
            result.category = "N/A";
            result.transType = "N/A";
            results.push_back(result);
            stats.totalTransactions++;
            transactionIndex++;
            continue;
        }

        wxString trnamtStr(trnamt);
        trnamtStr.Trim(true).Trim(false);
        double amount = 0.0;
        if (!trnamtStr.ToDouble(&amount))
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
        if (!date.ParseFormat(wxString(dtposted).Left(8), "%Y%m%d"))
        {
            result.imported = false;
            result.category = "N/A";
            result.transType = "N/A";
            results.push_back(result);
            stats.totalTransactions++;
            transactionIndex++;
            continue;
        }

        wxString fitidStr = wxString::FromUTF8(fitid);
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
            existingTx->TOTRANSAMOUNT = (existingTx->TRANSAMOUNT < 0) ? -existingTx->TRANSAMOUNT : existingTx->TRANSAMOUNT;
            existingTx->TRANSAMOUNT = existingTx->TOTRANSAMOUNT;
            existingTx->PAYEEID = -1;
            existingTx->CATEGID = static_cast<int>(transferCategId_);
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

        wxString transType = (amount >= 0) ? "Deposit" : "Withdrawal";
        bool usedRegex = false;
        wxString matchedPattern;
        wxString payeeName = getPayeeName(wxString::FromUTF8(memo), usedRegex, matchedPattern);
        result.usedRegex = usedRegex;
        result.importedPayee = payeeName;
        result.regexPattern = matchedPattern;

        wxLongLong payeeId = -1;
        long long categId = -1; // Default to -1 (Uncategorized)
        Model_Payee::Data* payee = Model_Payee::instance().get_one(Model_Payee::PAYEENAME(payeeName));
        bool payeeExisted = (payee != nullptr);

        if (!payee)
        {
            mmPayeeSelectionDialog payeeDlg(this, wxString::FromUTF8(memo), payeeName, fitidStr, date.FormatISODate(), wxString::Format("%.2f", amount),
                                            transType, transactionIndex, totalTransactions, importStartTime_);
            int modalResult = payeeDlg.ShowModal();
            if (modalResult == wxID_OK)
            {
                wxString selectedPayee = payeeDlg.GetSelectedPayee();
                categId = payeeDlg.GetSelectedCategoryID(); // Get user-selected category from dialog
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
                    // If updatePayeeCategory_ is true, the payee’s CATEGID is already updated in the dialog
                    // But we use the dialog’s categId regardless
                }
            }
            else
            {
                break;
            }
        }
        else if (usedRegex)
        {
            payeeId = payee->PAYEEID;
            categId = payee->CATEGID.GetValue(); // Use payee’s default for auto-imported
            result.importedPayee = payee->PAYEENAME;
            stats.autoImportedCount++;
        }
        else
        {
            // For existing payee without regex, prompt user
            mmPayeeSelectionDialog payeeDlg(this, wxString::FromUTF8(memo), payeeName, fitidStr, date.FormatISODate(), wxString::Format("%.2f", amount),
                                            transType, transactionIndex, totalTransactions, importStartTime_);
            int modalResult = payeeDlg.ShowModal();
            if (modalResult == wxID_OK)
            {
                wxString selectedPayee = payeeDlg.GetSelectedPayee();
                categId = payeeDlg.GetSelectedCategoryID(); // Get user-selected category
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
                break;
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
        newTx->CATEGID = categId; // Use the category from the dialog or payee (for auto-import)
        newTx->STATUS = Model_Checking::STATUS_KEY_NONE;
        newTx->NOTES = wxString::FromUTF8(memo);


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
                                                                  "- Time Taken: %.2f seconds"),
                                                                totalTransactions_, autoImportedCount_, newPayeesCreated_, manuallyAllocated_, elapsedTimeSec));
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