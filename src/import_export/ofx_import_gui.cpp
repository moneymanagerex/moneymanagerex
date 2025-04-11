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
#include "payeematchandmerge.h"
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
#include <wx/event.h>
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
EVT_CHECKBOX(wxID_ANY, mmOFXImportDialog::OnUseFuzzyMatchingToggled)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(mmOFXImportSummaryDialog, wxDialog)
EVT_BUTTON(wxID_OK, mmOFXImportSummaryDialog::OnOK)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(mmPayeeSelectionDialog, wxDialog)
EVT_RADIOBUTTON(ID_USE_EXISTING, mmPayeeSelectionDialog::OnUseExistingPayee)
EVT_RADIOBUTTON(ID_CREATE_NEW, mmPayeeSelectionDialog::OnCreateNewPayee)
EVT_CHECKBOX(wxID_ANY, mmPayeeSelectionDialog::OnUpdateRegex)
EVT_BUTTON(wxID_OK, mmPayeeSelectionDialog::OnOK)
EVT_BUTTON(wxID_CANCEL, mmPayeeSelectionDialog::OnCancel)
EVT_CHOICE(wxID_ANY, mmPayeeSelectionDialog::OnPayeeChoice)
EVT_BUTTON(ID_TITLE_CASE, mmPayeeSelectionDialog::OnTitleCase)
EVT_GRID_LABEL_LEFT_CLICK(mmPayeeSelectionDialog::OnGridLabelLeftClick)
EVT_BUTTON(ID_UPDATE_CATEGORY, mmPayeeSelectionDialog::OnUpdateCategoryToggle)
EVT_BUTTON(ID_INSERT_ROW, mmPayeeSelectionDialog::OnInsertRow)
EVT_BUTTON(ID_DELETE_ROW, mmPayeeSelectionDialog::OnDeleteRow)
EVT_CHOICE(wxID_ANY, mmPayeeSelectionDialog::OnCategorySelection)
EVT_SET_FOCUS(mmPayeeSelectionDialog::OnCategoryFocus)
EVT_INIT_DIALOG(mmPayeeSelectionDialog::OnInitDialog)
EVT_BUTTON(wxID_CANCEL, mmPayeeSelectionDialog::OnCancel)
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
    categoryManuallyChanged_ = true; // Mark as manually changed
    wxLogDebug("Category manually changed to '%s' (ID=%lld)", categoryChoice_->GetString(sel), selectedCategoryId);
}

void mmPayeeSelectionDialog::OnInitDialog(wxInitDialogEvent& event)
{
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
        {
            int selection = payeeChoice_->GetSelection();
            wxInt64ClientData* clientData = selection != wxNOT_FOUND ? dynamic_cast<wxInt64ClientData*>(payeeChoice_->GetClientObject(selection)) : nullptr;
            bool validSelection = clientData && clientData->GetValue() != -1;
            okButton_->Enable(validSelection && !payeeChoice_->GetStringSelection().IsEmpty());
        }
        else
        {
            okButton_->Enable(!newPayeeTextCtrl_->GetValue().IsEmpty());
        }
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
        int selection = payeeChoice_->GetSelection();
        if (selection == wxNOT_FOUND || selection == 0) // 0 is the blank entry
        {
            wxMessageBox(_("Please select an existing payee."), _("Error"), wxOK | wxICON_ERROR);
            return;
        }
        selectedPayee_ = payeeChoice_->GetString(selection);
    }
    else
    {
        selectedPayee_ = newPayeeTextCtrl_->GetValue();
    }
    wxLogDebug("OnOK: Selected payee='%s'", selectedPayee_);
    if (selectedPayee_.IsEmpty())
    {
        wxMessageBox(_("Please select an existing payee or enter a new payee name."), _("Error"), wxOK | wxICON_ERROR);
        return;
    }

    long long selectedCategoryId = GetSelectedCategoryID();
    bool shouldUpdateCategory = updatePayeeCategory_;
    Model_Payee::Data* payee = Model_Payee::instance().get_one(Model_Payee::PAYEENAME(selectedPayee_));

    // Handle existing payee category update
    if (useExistingRadio_->GetValue() && shouldUpdateCategory && payee && categoryManuallyChanged_)
    {
        if (selectedCategoryId != payee->CATEGID.GetValue())
        {
            payee->CATEGID = selectedCategoryId;
            Model_Payee::instance().save(payee);
            wxLogDebug("Updated payee '%s' category to ID=%lld", selectedPayee_, selectedCategoryId);
        }
    }

    // Handle regex pattern updates
    if (shouldUpdateRegex_)
    {
        std::vector<wxString> patterns;
        for (int i = 0; i < regexGrid_->GetNumberRows(); ++i)
        {
            wxString pattern = regexGrid_->GetCellValue(i, 0).Trim().Trim(false);
            if (!pattern.IsEmpty())
            {
                if (pattern.Contains("*")) // We should check if the apostrophe is mid-string I think. Needs some more refactoring.
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
            wxLogDebug("OnOK: Regex pattern set to '%s'", regexPattern_);

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
                wxLogDebug("Saved regex pattern for payee '%s'", selectedPayee_);
            }
        }
        else
        {
            regexPattern_ = "{}";
            if (payee)
            {
                payee->PATTERN = regexPattern_;
                Model_Payee::instance().save(payee);
            }
        }
    }
    else if (createNewRadio_->GetValue() && !payee)
    {
        payee = Model_Payee::instance().create();
        payee->PAYEENAME = selectedPayee_;
        payee->CATEGID = selectedCategoryId;
        Model_Payee::instance().save(payee);
        wxLogDebug("Created new payee '%s' with category ID=%lld", selectedPayee_, selectedCategoryId);
    }

    // Reset categoryManuallyChanged_ if not persisting the change
    if (!shouldUpdateCategory)
    {
        categoryManuallyChanged_ = false;
        wxLogDebug("Reset categoryManuallyChanged_ to false as category update not persisted");
    }
    else if (shouldUpdateCategory && payee)
    {
        wxLogDebug("Keeping categoryManuallyChanged_ = true as category was updated for payee '%s'", selectedPayee_);
    }

    // Notify parent dialog to reload regex mappings if necessary
    mmOFXImportDialog* parentDialog = dynamic_cast<mmOFXImportDialog*>(GetParent());
    if (parentDialog && (shouldUpdateRegex_ || (payee && shouldUpdateCategory)))
    {
        parentDialog->loadRegexMappings();
        wxLogDebug("Triggered parent dialog to reload regex mappings");
    }

    EndModal(wxID_OK);
}

void mmPayeeSelectionDialog::OnCancel(wxCommandEvent& /*event*/)
{
    shouldCancelImport_ = true;
    EndModal(wxID_CANCEL);
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

mmPayeeSelectionDialog::mmPayeeSelectionDialog(wxWindow* parent, const wxString& memo, const wxString& suggestedPayeeName, const wxString& fitid,
                                               const wxString& date, const wxString& amount, const wxString& transType, int currentTransaction,
                                               int newTransactions, wxLongLong importStartTime, double matchConfidence, const wxString& matchMethod,
                                               int totalTransactions)
    : wxDialog(parent, wxID_ANY, _("Payee Confirmation Required"), wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER),
      suggestedPayeeName_(suggestedPayeeName), matchConfidence_(matchConfidence), matchMethod_(matchMethod), confidenceLabel_(nullptr),
      useExistingRadio_(nullptr), createNewRadio_(nullptr), payeeChoice_(nullptr), newPayeeTextCtrl_(nullptr), titleCaseButton_(nullptr),
      categoryChoice_(nullptr), updateRegexCheckBox_(nullptr), regexGrid_(nullptr), okButton_(nullptr), updateCategoryButton_(nullptr),
      insertRowButton_(nullptr), deleteRowButton_(nullptr), existingPayeeLabel_(nullptr), newPayeeLabel_(nullptr), payeeSizer_(nullptr),
      selectedPayee_(suggestedPayeeName), regexPattern_(memo), shouldUpdateRegex_(false), updatePayeeCategory_(false), memoAdded_(false),
      initialCategoryId_(-1), currentTransaction_(currentTransaction), newTransactions_(newTransactions), importStartTime_(importStartTime),
      categoryManuallyChanged_(false), totalTransactions_(totalTransactions), memo_(memo)
{
    wxLogDebug("PayeeSelectionDialog: Initialized with current=%d, new=%d, total=%d", currentTransaction_, newTransactions_, totalTransactions_);

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Progress and ETA
    wxLongLong currentTime = wxGetUTCTimeMillis();
    double elapsedTimeSec = (currentTime - importStartTime_).ToDouble() / 1000.0;
    double avgTimePerTrans = (currentTransaction_ > 0) ? elapsedTimeSec / currentTransaction_ : 0.0;
    int remainingTrans = totalTransactions_ - currentTransaction_;
    double estimatedTimeSec = avgTimePerTrans * remainingTrans;
    double estimatedTimeMin = estimatedTimeSec / 60.0;

    wxStaticText* progressLabel = new wxStaticText(
        this, wxID_ANY, wxString::Format(_("Transaction %d of %d (%d total in file)"), currentTransaction_ + 1, newTransactions_, totalTransactions_));
    mainSizer->Add(progressLabel, 0, wxALL, 5);
    wxString etaText = (avgTimePerTrans > 0) ? wxString::Format(_("Estimated completion time: %.1f minutes"), estimatedTimeMin) : _("Estimating time...");
    mainSizer->Add(new wxStaticText(this, wxID_ANY, etaText), 0, wxALL, 5);

    // Transaction info
    mainSizer->Add(new wxStaticText(this, wxID_ANY, wxString::Format(_("Transaction Number (FITID): %s"), fitid)), 0, wxLEFT | wxRIGHT | wxTOP, 5);
    mainSizer->Add(new wxStaticText(this, wxID_ANY, wxString::Format(_("Date: %s"), date)), 0, wxALL, 5);
    mainSizer->Add(new wxStaticText(this, wxID_ANY, wxString::Format(_("Amount: %s"), amount)), 0, wxALL, 5);
    mainSizer->Add(new wxStaticText(this, wxID_ANY, wxString::Format(_("Type: %s"), transType)), 0, wxALL, 5);

    // Create regex grid but don't yet add sizer
    regexGrid_ = new wxGrid(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL | wxVSCROLL);
    regexGrid_->CreateGrid(1, 1);
    regexGrid_->SetColLabelValue(0, _("Regex Pattern"));
    regexGrid_->SetDefaultColSize(300);
    regexGrid_->SetColMinimalWidth(0, 50);
    regexGrid_->EnableGridLines(true);
    regexGrid_->EnableEditing(true);
    regexGrid_->SetDefaultCellBackgroundColour(*wxWHITE);
    regexGrid_->SetLabelBackgroundColour(GetBackgroundColour());
    regexGrid_->SetDefaultEditor(new wxGridCellTextEditor());
    regexGrid_->SetScrollRate(10, 10);

    // Show original memo and match result
    mainSizer->Add(new wxStaticText(this, wxID_ANY, wxString::Format(_("Memo: %s"), memo_)), 0, wxALL, 5);
    if (matchMethod_ == "None" || suggestedPayeeName_ == memo_)
    {
        mainSizer->Add(new wxStaticText(this, wxID_ANY, _("No matching payee found.")), 0, wxALL, 5);
        confidenceLabel_ = new wxStaticText(this, wxID_ANY, wxString::Format(_("Match Confidence: %.1f%% (%s)"), matchConfidence_, matchMethod_));
        mainSizer->Add(confidenceLabel_, 0, wxALL, 5);
    }
    else if (matchConfidence_ > 50.0) // Show suggested payee and confidence only if confidence > 50%
    {
        mainSizer->Add(new wxStaticText(this, wxID_ANY, wxString::Format(_("Suggested Payee: '%s'"), suggestedPayeeName_)), 0, wxALL, 5);
        confidenceLabel_ = new wxStaticText(this, wxID_ANY, wxString::Format(_("Match Confidence: %.1f%% (%s)"), matchConfidence_, matchMethod_));
        mainSizer->Add(confidenceLabel_, 0, wxALL, 5);

        // Load regex patterns for the suggested payee
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
    }
    // If confidence <= 50%, neither suggested payee nor confidence is shown

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

    // Add a blank entry as the first item with ID -1
    payeeChoice_->Append("", new wxInt64ClientData(-1));
    int suggestedPayeeIndex = wxNOT_FOUND;

    for (size_t i = 0; i < payees.size(); ++i)
    {
        int index = payeeChoice_->Append(payees[i].PAYEENAME, new wxInt64ClientData(payees[i].PAYEEID.GetValue()));
        // Only pre-select the suggested payee if confidence > 50%
        if (matchConfidence_ > 50.0 && payees[i].PAYEENAME.IsSameAs(suggestedPayeeName_, false))
            suggestedPayeeIndex = index;
    }

    // Set selection only if we have a suggested payee with confidence > 50%
    if (suggestedPayeeIndex != wxNOT_FOUND)
    {
        payeeChoice_->SetSelection(suggestedPayeeIndex);
        wxLogDebug("PayeeSelectionDialog: Pre-selected suggested payee '%s' at index %d (confidence %.1f%%)", suggestedPayeeName_, suggestedPayeeIndex,
                   matchConfidence_);
    }
    else
    {
        payeeChoice_->SetSelection(0); // Select the blank entry by default
        wxLogDebug("PayeeSelectionDialog: No pre-selection (confidence %.1f%% ≤ 50%% or no match), defaulting to blank entry", matchConfidence_);
    }

    payeeSizer_->Add(existingPayeeLabel_, 0, wxLEFT | wxRIGHT | wxTOP, 5);
    payeeSizer_->Add(payeeChoice_, 0, wxALL | wxEXPAND, 2);

    newPayeeLabel_ = new wxStaticText(this, wxID_ANY, _("New Payee Name:"));
    newPayeeTextCtrl_ = new wxTextCtrl(this, wxID_ANY, memo_); // Changed from suggestedPayeeName_ to memo_
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
            AddCategoryToChoice(categoryChoice_, cat.CATEGID.GetValue(), categoryMap, 0);
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
}

void mmPayeeSelectionDialog::OnPayeeChoice(wxCommandEvent& event)
{
    int selection = payeeChoice_->GetSelection();
    if (selection == wxNOT_FOUND || selection == 0) // Blank entry
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
    if (payee)
    {
        // Always update the category to the payee's default, regardless of manual changes
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
                    wxLogDebug("Reset category to '%s' (ID=%lld) for payee '%s'", categoryChoice_->GetString(i), categId, selectedPayeeName);
                    break;
                }
            }
        }
        // If no match found, default to Uncategorized
        if (categoryChoice_->GetSelection() == wxNOT_FOUND)
        {
            categoryChoice_->SetSelection(0); // Assuming 0 is "Uncategorized"
            wxLogDebug("No matching category found for payee '%s', set to Uncategorized", selectedPayeeName);
        }
    }
    else
    {
        // If payee not found, default to Uncategorized
        categoryChoice_->SetSelection(0);
        wxLogDebug("Payee '%s' not found, set category to Uncategorized", selectedPayeeName);
    }

    if (useExistingRadio_->GetValue())
    {
        LoadRegexPatterns(clientData);
    }

    UpdateOKButton(event);
    // Reset categoryManuallyChanged_ to false since we're overriding any manual change
    categoryManuallyChanged_ = false;
    wxLogDebug("Reset categoryManuallyChanged_ to false after payee change to '%s'", selectedPayeeName);
    event.Skip();
}

mmOFXImportDialog::mmOFXImportDialog(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, _("Import OFX File"), wxDefaultPosition, wxSize(500, 360), wxCAPTION | wxCLOSE_BOX | wxMINIMIZE_BOX), fileNameCtrl_(nullptr),
      accountDropDown_(nullptr), account_id_(0), payeeRegExCheckBox_(nullptr), useFuzzyMatchingCheckBox_(nullptr), fuzzyConfidenceChoice_(nullptr),
      markFuzzyFollowUpCheckBox_(nullptr), transferCategId_(-1), importStartTime_(0), promptFuzzyConfirmationCheckBox_(nullptr)
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
    for (const auto& account : Model_Account::instance().all(Model_Account::COL_ACCOUNTNAME))
    {
        wxString accountIdStr = wxString::Format("%lld", account.ACCOUNTID.GetValue());
        int idx = accountDropDown_->Append(account.ACCOUNTNAME, new wxStringClientData(accountIdStr));
        if (account.ACCOUNTID == account_id_)
            accountDropDown_->SetSelection(idx);
    }
    mainSizer->Add(accountDropDown_, 0, wxALL | wxEXPAND, 5);

    payeeRegExCheckBox_ = new wxCheckBox(this, wxID_ANY, _("Use existing regex patterns to automatically match payees"), wxDefaultPosition, wxDefaultSize);
    payeeRegExCheckBox_->SetValue(true);
    mainSizer->Add(payeeRegExCheckBox_, 0, wxALL, 5);

    useFuzzyMatchingCheckBox_ =
        new wxCheckBox(this, wxID_ANY, _("Use fuzzy payee matching if exact match and regex matching not successful"), wxDefaultPosition, wxDefaultSize);
    useFuzzyMatchingCheckBox_->SetValue(true);
    mainSizer->Add(useFuzzyMatchingCheckBox_, 0, wxALL, 5);

    mainSizer->Add(new wxStaticText(this, wxID_ANY, _("Minimum Fuzzy Match Confidence Level:")), 0, wxALL, 5);
    fuzzyConfidenceChoice_ = new wxChoice(this, wxID_ANY);
    for (int i = 100; i >= 40; i -= 1)
    {
        wxString label = wxString::Format("%d%%", i);
        fuzzyConfidenceChoice_->Append(label, new wxStringClientData(wxString::Format("%d", i)));
        if (i == 85)
            fuzzyConfidenceChoice_->SetSelection(fuzzyConfidenceChoice_->GetCount() - 1);
    }
    fuzzyConfidenceChoice_->Enable(useFuzzyMatchingCheckBox_->IsChecked());
    mainSizer->Add(fuzzyConfidenceChoice_, 0, wxALL | wxEXPAND, 5);

    markFuzzyFollowUpCheckBox_ =
        new wxCheckBox(this, wxID_ANY, _("Mark transactions automatically imported with Fuzzy Match as 'Follow Up'"), wxDefaultPosition, wxDefaultSize);
    markFuzzyFollowUpCheckBox_->SetValue(false);
    mainSizer->Add(markFuzzyFollowUpCheckBox_, 0, wxALL, 5);

    promptFuzzyConfirmationCheckBox_ = new wxCheckBox(this, wxID_ANY, _("Prompt to confirm each fuzzy match"), wxDefaultPosition, wxDefaultSize);
    promptFuzzyConfirmationCheckBox_->SetValue(false);
    mainSizer->Add(promptFuzzyConfirmationCheckBox_, 0, wxALL, 5);

    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonSizer->Add(new wxButton(this, wxID_OK, _("Import")), 0, wxALL, 5);
    buttonSizer->Add(new wxButton(this, wxID_CANCEL, _("Cancel")), 0, wxALL, 5);
    mainSizer->Add(buttonSizer, 0, wxALIGN_CENTER | wxALL, 5);

    SetSizer(mainSizer);
    SetMinSize(wxSize(500, 360));
    SetMaxSize(wxSize(-1, 360));
    Layout();

    useFuzzyMatchingCheckBox_->Bind(wxEVT_CHECKBOX, &mmOFXImportDialog::OnUseFuzzyMatchingToggled, this);

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

double mmOFXImportDialog::GetMinFuzzyConfidence() const
{
    wxStringClientData* data = static_cast<wxStringClientData*>(fuzzyConfidenceChoice_->GetClientObject(fuzzyConfidenceChoice_->GetSelection()));
    double confidence = 95.0; // Default if no data passed
    if (data)
    {
        data->GetData().ToDouble(&confidence);
    }
    return confidence;
}

mmOFXImportDialog::~mmOFXImportDialog()
{
    payeeRegexMap_.clear(); // Clear the map to release any held strings
}

void mmOFXImportDialog::OnUseFuzzyMatchingToggled(wxCommandEvent& /*event*/)
{
    fuzzyConfidenceChoice_->Enable(useFuzzyMatchingCheckBox_->IsChecked());
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
        long pos = fileNameCtrl_->XYToPosition(fileNameCtrl_->GetLineLength(0), 0);
        fileNameCtrl_->SetInsertionPoint(pos);
        fileNameCtrl_->ShowPosition(pos); // Ensure the view scrolls to this position

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
        wxString acctId;
        wxStringTokenizer tokenizer(xmlContent, "\n");
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
            accountDropDown_->SetSelection(0); // Default to first account
            return;
        }

        bool accountFound = false;
        for (unsigned int i = 0; i < accountDropDown_->GetCount(); ++i)
        {
            wxStringClientData* data = static_cast<wxStringClientData*>(accountDropDown_->GetClientObject(i));
            if (data)
            {
                long long tempAccountId;
                if (data->GetData().ToLongLong(&tempAccountId))
                {
                    Model_Account::Data* account = Model_Account::instance().get(tempAccountId);
                    if (account)
                    {
                        wxString accountNum = account->ACCOUNTNUM;
                        accountNum.Trim(true).Trim(false); // Normalize
                        acctId.Trim(true).Trim(false);     // Normalize
                        wxLogDebug("Comparing ACCTID '%s' with ACCOUNTNUM '%s' for account '%s' (ID: %lld)", acctId, accountNum, account->ACCOUNTNAME,
                                   tempAccountId);
                        if (accountNum.IsSameAs(acctId, false))
                        { // Case-insensitive
                            accountDropDown_->SetSelection(i);
                            account_id_ = tempAccountId;
                            wxLogDebug("Matched account: '%s' (ID: %lld, ACCOUNTNUM: '%s') at index %d", account->ACCOUNTNAME, account_id_.GetValue(),
                                       accountNum, i);
                            accountFound = true;
                            break;
                        }
                    }
                }
            }
        }

        if (!accountFound)
        {
            wxLogDebug("No account matched ACCTID '%s', defaulting to first account", acctId);
            accountDropDown_->SetSelection(0);
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

    wxLogDebug("Starting OFX import: File='%s', Account='%s' (ID=%lld)", filePath, accountName, account_id_.GetValue());
    importStartTime_ = wxGetUTCTimeMillis();
    std::vector<OFXImportResult> importResults;
    OFXImportStats stats;
    bool importCompleted = ParseOFX(filePath, importResults, stats); // Capture return value but don't use it for popup
    wxLogDebug("Import completed: %s, Results=%zu", importCompleted ? "Yes" : "No", importResults.size());
    if (!importCompleted)
    {
        wxLogDebug("Import was canceled by user; showing partial results.");
    }
    // Always show summary, even if canceled
    wxLogDebug("Showing import summary with %zu transactions processed", importResults.size());
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

wxString mmOFXImportDialog::getPayeeName(const wxString& memo, bool& usedRegex, wxString& regexPattern, double& matchConfidence, wxString& matchMethod, wxString& matchRegexPattern)
{
    wxLogDebug("getPayeeName: Starting with memo='%s'", memo);

    PayeeMatchAndMerge matcher;
    std::vector<PayeeMatchResult> matchResults;
    bool matchFound = matcher.MatchPayee(memo, PayeeMatchMode::BEST_MATCH, matchResults);

    usedRegex = false;
    regexPattern = wxEmptyString;
    matchConfidence = 0.0;
    matchMethod = "None";
    matchRegexPattern = "";

    if (matchFound && !matchResults.empty())
    {
        const PayeeMatchResult& bestMatch = matchResults[0];
        wxString candidatePayeeName = bestMatch.Name;
        long long candidatePayeeID = bestMatch.PayeeID;
        matchConfidence = bestMatch.MatchConfidence;
        matchMethod = bestMatch.matchMethod;
        usedRegex = (matchMethod == "Regex");
        if (usedRegex == true)
        {
            matchRegexPattern = bestMatch.regexPattern;
        }

        wxLogDebug("getPayeeName: Best match from PayeeMatchAndMerge: Name='%s', ID=%lld, Method='%s', Confidence=%.1f%%", candidatePayeeName, candidatePayeeID,
                   matchMethod, matchConfidence);

        // Validate the PayeeID and check if it's a true exact match
        Model_Payee::Data* payee = Model_Payee::instance().get(candidatePayeeID);
        if (payee && payee->PAYEEID.GetValue() == candidatePayeeID)
        {
            wxString dbPayeeName = payee->PAYEENAME;
            if (matchMethod == "Exact" || memo.IsSameAs(dbPayeeName, false))
            {
                matchMethod = "Exact";
                matchConfidence = 100.0;
                usedRegex = false;
                wxLogDebug("getPayeeName: Confirmed exact match by ID=%lld, Name='%s'", candidatePayeeID, dbPayeeName);
                return dbPayeeName;
            }
            else if (!dbPayeeName.IsSameAs(candidatePayeeName, false))
            {
                wxLogWarning("getPayeeName: PayeeID=%lld mismatch: Expected '%s', Found '%s'", candidatePayeeID, candidatePayeeName, dbPayeeName);
                return dbPayeeName; // Use database name but keep original matchMethod
            }
            wxLogDebug("getPayeeName: Returning '%s', Confidence=%.1f%%, Method='%s' (ID=%lld validated)", candidatePayeeName, matchConfidence, matchMethod,
                       candidatePayeeID);
            return candidatePayeeName;
        }
        else
        {
            wxLogWarning("getPayeeName: PayeeID=%lld from match not found in database, using Name='%s', Method='%s'", candidatePayeeID, candidatePayeeName,
                         matchMethod);
            if (matchMethod == "Exact")
            {
                matchConfidence = 100.0; // Trust PayeeMatchAndMerge's exact match
                wxLogDebug("getPayeeName: Exact match assumed despite ID not found: '%s'", candidatePayeeName);
            }
            return candidatePayeeName;
        }

        if (matchMethod == "Fuzzy" && !useFuzzyMatchingCheckBox_->IsChecked())
        {
            wxLogDebug("Fuzzy matching disabled, rejecting fuzzy match: '%s', Confidence=%.1f%%", candidatePayeeName, matchConfidence);
            return memo;
        }

        if (usedRegex && payee && !payee->PATTERN.IsEmpty())
        {
            rapidjson::Document doc;
            doc.Parse(payee->PATTERN.mb_str());
            if (!doc.HasParseError() && doc.IsObject())
            {
                for (rapidjson::Value::ConstMemberIterator itr = doc.MemberBegin(); itr != doc.MemberEnd(); ++itr)
                {
                    if (itr->value.IsString())
                    {
                        wxString pattern = wxString::FromUTF8(itr->value.GetString());
                        wxRegEx re(pattern, wxRE_ADVANCED | wxRE_ICASE);
                        if (re.IsValid() && re.Matches(memo))
                        {
                            regexPattern = pattern;
                            break;
                        }
                    }
                }
            }
            if (regexPattern.IsEmpty())
            {
                wxLogWarning("No matching regex pattern found in '%s' for memo '%s'", payee->PATTERN, memo);
            }
        }

        wxLogDebug("getPayeeName: Returning '%s', Confidence=%.1f%%, Method='%s'", candidatePayeeName, matchConfidence, matchMethod);
        return candidatePayeeName;
    }

    wxLogDebug("getPayeeName: No match found, defaulting to memo: '%s'", memo);
    return memo;
}

bool mmOFXImportDialog::ImportTransactions(wxXmlNode* banktranlist, wxLongLong accountID, std::vector<OFXImportResult>& results, OFXImportStats& stats)
{
    if (!banktranlist)
        return false;

    Model_Account::Data* account = Model_Account::instance().get(accountID);
    if (!account)
        return false;

    double minFuzzyConfidence = GetMinFuzzyConfidence();
    bool markFuzzyFollowUp = markFuzzyFollowUpCheckBox_->IsChecked();

    stats = OFXImportStats();
    int totalTransactions = 0;
    int newTransactions = 0;
    for (wxXmlNode* stmttrn = banktranlist->GetChildren(); stmttrn; stmttrn = stmttrn->GetNext())
    {
        if (stmttrn->GetType() == wxXML_ELEMENT_NODE && stmttrn->GetName().Upper() == "STMTTRN")
        {
            totalTransactions++;
            wxString fitid;
            for (wxXmlNode* trnNode = stmttrn->GetChildren(); trnNode; trnNode = trnNode->GetNext())
            {
                if (trnNode->GetName().Upper() == "FITID")
                {
                    fitid = trnNode->GetNodeContent().Trim(true).Trim(false);
                    break;
                }
            }
            // Check if this FITID exists in the current account first
            if (!Model_Checking::instance().find(Model_Checking::TRANSACTIONNUMBER(fitid), Model_Checking::ACCOUNTID(account->ACCOUNTID.GetValue())).empty())
                continue; // Skip if duplicate in current account
            newTransactions++;
        }
    }
    stats.totalTransactions = totalTransactions;

    int transactionIndex = 0;
    for (wxXmlNode* stmttrn = banktranlist->GetChildren(); stmttrn; stmttrn = stmttrn->GetNext())
    {
        if (stmttrn->GetType() != wxXML_ELEMENT_NODE || stmttrn->GetName().Upper() != "STMTTRN")
            continue;

        wxString fitid, dtposted, trnamt, memo;
        for (wxXmlNode* trnNode = stmttrn->GetChildren(); trnNode; trnNode = trnNode->GetNext())
        {
            wxString nodeName = trnNode->GetName().Upper();
            if (nodeName == "FITID")
                fitid = trnNode->GetNodeContent().Trim(true).Trim(false);
            else if (nodeName == "DTPOSTED")
                dtposted = trnNode->GetNodeContent();
            else if (nodeName == "TRNAMT")
                trnamt = trnNode->GetNodeContent();
            else if (nodeName == "MEMO")
                memo = trnNode->GetNodeContent();
        }

        if (fitid.IsEmpty() || dtposted.IsEmpty() || trnamt.IsEmpty())
        {
            stats.skippedErrors++;
            continue;
        }

        OFXImportResult result;
        result.fitid = fitid;
        result.date = dtposted.Left(8);
        result.amount = trnamt;
        result.ofxPayee = memo.IsEmpty() ? "Unknown Payee" : memo;
        result.matchMode = "None";
        result.matchConfidence = 0.0;

        double amount;
        if (!trnamt.ToDouble(&amount))
        {
            wxLogError("Invalid amount '%s' in transaction FITID: %s", trnamt, fitid);
            stats.skippedErrors++;
            results.push_back(result);
            continue;
        }

        wxDateTime date;
        if (!date.ParseFormat(dtposted.Left(8), "%Y%m%d"))
        {
            wxLogError("Invalid date '%s' in transaction FITID: %s", dtposted, fitid);
            stats.skippedErrors++;
            results.push_back(result);
            continue;
        }

        Model_Checking::Data* transaction = Model_Checking::instance().create();
        transaction->ACCOUNTID = account->ACCOUNTID;
        transaction->TRANSACTIONNUMBER = result.fitid;
        transaction->TRANSDATE = date.FormatISODate();
        transaction->TRANSAMOUNT = fabs(amount);
        transaction->NOTES = memo;

        bool isTransfer = false;
        // Check for existing transaction in the current account first
        Model_Checking::Data_Set sameAccountTrans =
            Model_Checking::instance().find(Model_Checking::TRANSACTIONNUMBER(fitid), Model_Checking::ACCOUNTID(account->ACCOUNTID));
        if (!sameAccountTrans.empty())
        {
            result.imported = false;
            result.importedPayee = "DUPLICATE";
            stats.skippedDuplicates++;
            results.push_back(result);
            //transactionIndex++;
            continue; // Skip if duplicate in current account
        }

        // Only check other accounts if no duplicate in current account
        Model_Checking::Data_Set allExistingTrans = Model_Checking::instance().find(Model_Checking::TRANSACTIONNUMBER(fitid));
        if (!allExistingTrans.empty())
        {
            for (auto& existing : allExistingTrans)
            {
                // Check if this FITID is already a transfer involving the current account
                if (existing.TRANSCODE == "Transfer" &&
                    (existing.ACCOUNTID == account->ACCOUNTID || existing.TOACCOUNTID == account->ACCOUNTID))
                {
                    result.imported = false;
                    result.transType = "";
                    result.importedPayee = "DUPLICATE";
                    result.category = "";
                    result.matchMode = "None";
                    stats.autoImportedCount++;
                    wxLogDebug("FITID='%s' already a transfer involving %lld, skipped", fitid, account->ACCOUNTID);
                    isTransfer = true;
                    results.push_back(result);
                    transactionIndex++;
                    break;
                }
                else if (existing.ACCOUNTID != account->ACCOUNTID)
                {
                    // Potential new transfer
                    double existingAmount = existing.TRANSAMOUNT;
                    wxDateTime existingDate;
                    existingDate.ParseISODate(existing.TRANSDATE);
                    double adjustedExistingAmount = (existing.TRANSCODE == "Withdrawal") ? -existingAmount : existingAmount;

                    double compAmt = fabs(adjustedExistingAmount + amount);
                    int compDate = abs((date - existingDate).GetDays());
                    if (compAmt < 0.01 && compDate <= 7)
                    {
                        if (existing.TRANSCODE == "Transfer")
                        {
                            wxLogWarning("FITID='%s' is a transfer from %lld to %lld, not updating", fitid, existing.ACCOUNTID,
                                         existing.TOACCOUNTID);
                            stats.skippedErrors++;
                            result.imported = false;
                            result.importedPayee = "TRANSFER CONFLICT";
                            isTransfer = true;
                        }
                        else
                        {
                            if (existing.TRANSCODE == "Withdrawal" && amount > 0)
                            {
                                existing.TRANSCODE = "Transfer";
                                existing.TOACCOUNTID = account->ACCOUNTID;
                                existing.TRANSAMOUNT = existingAmount;
                                existing.TOTRANSAMOUNT = amount;
                            }
                            else if (existing.TRANSCODE == "Deposit" && amount < 0)
                            {
                                existing.TRANSCODE = "Transfer";
                                existing.TOACCOUNTID = existing.ACCOUNTID;
                                existing.ACCOUNTID = account->ACCOUNTID;
                                existing.TRANSAMOUNT = fabs(amount);
                                existing.TOTRANSAMOUNT = existingAmount;
                            }
                            else
                            {
                                wxLogWarning("FITID='%s' incompatible (Existing=%s, New=%.2f), skipping", fitid, existing.TRANSCODE, amount);
                                stats.skippedErrors++;
                                result.imported = false;
                                result.importedPayee = "TRANSFER ERROR";
                                isTransfer = true;
                                results.push_back(result);
                                transactionIndex++;
                                break;
                            }

                            existing.PAYEEID = -1;
                            existing.CATEGID = transferCategId_;
                            existing.NOTES = "Existing: " + existing.NOTES + "\nUpdated: " + memo;

                            try
                            {
                                Model_Checking::instance().save(&existing);
                                result.imported = true;
                                result.transType = "Transfer";
                                result.importedPayee = Model_Account::instance().get(existing.ACCOUNTID)->ACCOUNTNAME;
                                result.category = "Transfer";
                                result.matchMode = "Transfer";
                                stats.autoImportedCount++;
                                wxLogDebug("Updated FITID='%s' to Transfer from %lld to %lld", fitid, existing.ACCOUNTID, existing.TOACCOUNTID);
                            }
                            catch (const wxSQLite3Exception& e)
                            {
                                wxLogError("Failed to update FITID='%s': %s", fitid, e.GetMessage());
                                result.imported = false;
                                stats.skippedErrors++;
                            }
                            isTransfer = true;
                        }
                        results.push_back(result);
                        transactionIndex++;
                        break;
                    }
                }
            }
            if (isTransfer)
                continue; // Skip creating a new transaction
        }

        if (!isTransfer)
        {
            transaction->TRANSCODE = (amount >= 0) ? "Deposit" : "Withdrawal";
            result.transType = transaction->TRANSCODE;

            double matchConfidence = 0.0;
            wxString matchMethod;
            bool usedRegex;
            wxString matchRegexPattern;
            wxString payeeName = getPayeeName(memo, usedRegex, result.regexPattern, matchConfidence, matchMethod, matchRegexPattern);
            result.importedPayee = payeeName;
            result.matchMode = matchMethod;
            result.matchConfidence = matchConfidence;

            if (usedRegex && matchRegexPattern)
            {
                result.matchRegexPattern = matchRegexPattern;
            }

            Model_Payee::Data* payee = Model_Payee::instance().get_one(Model_Payee::PAYEENAME(payeeName));
            bool payeeExisted = (payee != nullptr);

            bool promptForConfirmation = false;
            if (!payeeExisted && matchMethod != "Exact")
                promptForConfirmation = true;
            else if (matchMethod == "Fuzzy" && (matchConfidence < minFuzzyConfidence || promptFuzzyConfirmationCheckBox_->IsChecked()))
                promptForConfirmation = true;

            if (promptForConfirmation)
            {
                mmPayeeSelectionDialog payeeDlg(this, memo, payeeName, fitid, date.FormatISODate(), wxString::Format("%.2f", amount), transaction->TRANSCODE,
                                                transactionIndex, newTransactions, importStartTime_, matchConfidence, matchMethod, totalTransactions);
                if (payeeDlg.ShowModal() == wxID_OK)
                {
                    payeeName = payeeDlg.GetSelectedPayee();
                    result.importedPayee = payeeName;
                    payee = Model_Payee::instance().get_one(Model_Payee::PAYEENAME(payeeName));
                    if (payeeDlg.IsCreateNewPayee() && !payee)
                    {
                        Model_Payee::Data* newPayee = Model_Payee::instance().create();
                        newPayee->PAYEENAME = payeeName;
                        newPayee->CATEGID = payeeDlg.GetSelectedCategoryID();
                        Model_Payee::instance().save(newPayee);
                        transaction->PAYEEID = newPayee->PAYEEID;
                        stats.newPayeesCreated++;
                    }
                    else if (payee)
                    {
                        transaction->PAYEEID = payee->PAYEEID;
                        if (payeeDlg.ShouldUpdatePayeeCategory())
                        {
                            payee->CATEGID = payeeDlg.GetSelectedCategoryID();
                            Model_Payee::instance().save(payee);
                        }
                    }
                    transaction->CATEGID = payeeDlg.GetSelectedCategoryID();
                    Model_Category::Data* category = Model_Category::instance().get(transaction->CATEGID);
                    result.category = category ? category->CATEGNAME : "Uncategorized";
                    if (payeeDlg.ShouldUpdateRegex() && payee)
                    {
                        payee->PATTERN = payeeDlg.GetRegexPattern();
                        Model_Payee::instance().save(payee);
                        result.regexPattern = payee->PATTERN;
                    }
                    stats.manuallyAllocated++;
                    result.imported = true;
                    result.matchMode = "Manual";
                }
                else
                {
                    stats.skippedManual++;
                    result.imported = false;
                    result.importedPayee = "CANCELED";
                    results.push_back(result);
                    if (payeeDlg.ShouldCancelImport())
                        return false;
                    transactionIndex++;
                    continue;
                }
            }
            else if (matchMethod == "Regex" || matchMethod == "Exact" || (matchMethod == "Fuzzy" && matchConfidence >= minFuzzyConfidence))
            {
                if (payee)
                {
                    transaction->PAYEEID = payee->PAYEEID;
                    transaction->CATEGID = payee->CATEGID;
                    transaction->STATUS = (matchMethod == "Fuzzy" && markFuzzyFollowUp) ? "F" : "";
                    Model_Category::Data* category = Model_Category::instance().get(transaction->CATEGID);
                    result.category = category ? category->CATEGNAME : "Uncategorized";
                    stats.autoImportedCount++;
                    result.imported = true;
                }
                else if (matchMethod == "Exact")
                {
                    Model_Payee::Data* newPayee = Model_Payee::instance().create();
                    newPayee->PAYEENAME = payeeName;
                    newPayee->CATEGID = -1;
                    Model_Payee::instance().save(newPayee);
                    transaction->PAYEEID = newPayee->PAYEEID;
                    transaction->CATEGID = newPayee->CATEGID;
                    stats.newPayeesCreated++;
                    stats.autoImportedCount++;
                    result.imported = true;
                    Model_Category::Data* category = Model_Category::instance().get(transaction->CATEGID);
                    result.category = category ? category->CATEGNAME : "Uncategorized";
                }
                else
                {
                    stats.skippedErrors++;
                    result.imported = false;
                    result.importedPayee = "PAYEE ERROR";
                }
            }
            else
            {
                stats.skippedErrors++;
                result.imported = false;
                result.importedPayee = "PAYEE ERROR";
            }
        }

        if (result.imported)
        {
            try
            {
                Model_Checking::instance().save(transaction);
                stats.importedTransactions++;
                wxLogDebug("Imported: FITID='%s', Type='%s', Payee='%s', Mode='%s'", fitid, result.transType, result.importedPayee, result.matchMode);
            }
            catch (const wxSQLite3Exception& e)
            {
                wxLogError("Failed to save transaction FITID='%s': %s", fitid, e.GetMessage());
                stats.skippedErrors++;
                result.imported = false;
            }
        }

        results.push_back(result);
        if (result.imported)
          transactionIndex++;
    }

    return stats.totalTransactions > 0;
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

    wxStaticText* statsText =
        new wxStaticText(this, wxID_ANY,
                         wxString::Format(_("Import Statistics:\n"
                                            "- Total Transactions: %d\n"
                                            "- Automatically Imported: %d\n"
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
    grid->SetColLabelValue(5, _("Match Mode"));
    grid->SetColLabelValue(6, _("Match Pattern/Confidence")); // Renamed from "Regex Pattern"
    grid->SetColLabelValue(7, _("Category"));
    grid->SetColLabelValue(8, _("Transaction Type"));

/* We can sort results by uncommenting this block, and changing results in the below for loop to sortedResults

    // Create a copy of results to sort
    std::vector<OFXImportResult> sortedResults = results;

    for (const auto& result : sortedResults)
    // Sort by date in descending order
    std::sort(sortedResults.begin(), sortedResults.end(),
             [](const OFXImportResult& a, const OFXImportResult& b)
             {
                 return a.date > b.date; // Assumes date is YYYYMMDD, which sorts lexicographically
             });
*/
    int row = 0;
    for (const auto& result : results)
    {
        grid->AppendRows(1);

        wxString dateFormatted = result.date;
        if (dateFormatted.Len() == 8) // Ensure it’s in YYYYMMDD format
        {
            dateFormatted = dateFormatted.Mid(0, 4) + "-" + dateFormatted.Mid(4, 2) + "-" + dateFormatted.Mid(6, 2);
        }

        grid->SetCellValue(row, 0, result.fitid);
        grid->SetCellValue(row, 1, dateFormatted);
        grid->SetCellValue(row, 2, result.amount);
        grid->SetCellValue(row, 3, result.ofxPayee);
        grid->SetCellValue(row, 4, result.importedPayee);
        grid->SetCellValue(row, 5, result.matchMode);

        // Set Match Pattern/Confidence based on match mode
        if (result.matchMode == "Regex" && !result.matchRegexPattern.IsEmpty())
        {
            grid->SetCellValue(row, 6, wxString::Format("%s", result.matchRegexPattern));
        }
        else if (result.matchMode == "Fuzzy" && result.matchConfidence > 0.0)
        {
            grid->SetCellValue(row, 6, wxString::Format("Fuzzy: %.0f%%", result.matchConfidence));
        }
        else
        {
        //    grid->SetCellValue(row, 6, result.matchMode); // Exact, Manual, or other
        }

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


