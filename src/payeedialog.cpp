/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2012 - 2016, 2020 - 2022 Nikolay Akimov
 Copyright (C) 2021, 2022 Mark Whalley (mark@ipx.co.uk)
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


// mmEditPayeeDialog
// ------------------------------------------------------------------------------------------

wxIMPLEMENT_DYNAMIC_CLASS(mmEditPayeeDialog, wxDialog);

wxBEGIN_EVENT_TABLE(mmEditPayeeDialog, wxDialog)
    EVT_CHAR_HOOK(mmEditPayeeDialog::OnComboKey)
    EVT_BUTTON(wxID_CANCEL, mmEditPayeeDialog::OnCancel)
    EVT_BUTTON(wxID_OK, mmEditPayeeDialog::OnOk)
    EVT_BUTTON(wxID_UP, mmEditPayeeDialog::OnMoveUp)
    EVT_BUTTON(wxID_DOWN, mmEditPayeeDialog::OnMoveDown)
    EVT_GRID_CELL_CHANGED(mmEditPayeeDialog::OnPatternTableChanged)
wxEND_EVENT_TABLE()

mmEditPayeeDialog::mmEditPayeeDialog()
{
}
mmEditPayeeDialog::mmEditPayeeDialog(wxWindow *parent, Model_Payee::Data* payee, const wxString &name) :
m_payee(payee)
{
    long style = wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER;
    if (!wxDialog::Create(parent, wxID_ANY, _t("Edit Payee")
        , wxDefaultPosition, wxDefaultSize, style, name))
        return;

    CreateControls();
    mmSetSize(this);
    Centre();
    SetIcon(mmex::getProgramIcon());

    fillControls();
}

mmEditPayeeDialog::~mmEditPayeeDialog()
{
    Model_Infotable::instance().setSize("EDITPAYEE_DIALOG_SIZE", GetSize());
    m_patternTable->GetGridWindow()->Unbind(wxEVT_SIZE, &mmEditPayeeDialog::OnPatternTableSize, this);
}

void mmEditPayeeDialog::CreateControls()
{
    wxBoxSizer* bSizer1 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(bSizer1);

    wxFlexGridSizer* fgSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
    fgSizer1->AddGrowableCol(1, 0);
    bSizer1->Add(fgSizer1, g_flagsExpand);

    // Payee Name
    fgSizer1->Add(new wxStaticText(this, wxID_STATIC, _t("Payee")), g_flagsH);
    m_payeeName = new wxTextCtrl(this, wxID_ANY, "");
    mmToolTip(m_payeeName, _t("Enter the Name of the Payee. This name can be renamed at any time."));
    fgSizer1->Add(m_payeeName, g_flagsExpand);

    // Hidden Status
    fgSizer1->Add(new wxStaticText(this, wxID_STATIC, _t("Hidden")), g_flagsH);
    m_hidden = new wxCheckBox(this, wxID_ANY, "");
    mmToolTip(m_hidden, _t("Indicate whether the payee should hidden and not presented in the payee list for new transactions"));
    fgSizer1->Add(m_hidden, g_flagsExpand);

    // Category
    const wxString title = (Option::instance().getTransCategoryNone() == Option::LASTUSED) ?
                                _t("Last Used Category") : _t("Default Category");
    fgSizer1->Add(new wxStaticText(this, wxID_STATIC, title), g_flagsH);
    m_category = new mmComboBoxCategory(this, mmID_CATEGORY, wxDefaultSize, -1, true);
    mmToolTip(m_category, _t("The category used as default for this payee"));
    fgSizer1->Add(m_category, g_flagsExpand);

    // Reference
    fgSizer1->Add(new wxStaticText(this, wxID_STATIC, _t("Reference")), g_flagsH);
    m_reference = new wxTextCtrl(this, wxID_ANY, "");
    mmToolTip(m_reference, _t("A reference to be used for the payee, e.g. bank account number"));
    fgSizer1->Add(m_reference, g_flagsExpand);

    // Website
    fgSizer1->Add(new wxStaticText(this, wxID_STATIC, _t("Website")), g_flagsH);
    m_website = new wxTextCtrl(this, wxID_ANY, "");
    mmToolTip(m_website, _t("Website URL associated with the payee"));
    fgSizer1->Add(m_website, g_flagsExpand);

    // Notes
    fgSizer1->Add(new wxStaticText(this, wxID_STATIC, _t("Notes")), g_flagsH);
    m_Notes = new wxTextCtrl(this, wxID_ANY, ""
        , wxDefaultPosition, wxSize(-1, -1), wxTE_MULTILINE);
    fgSizer1->Add(m_Notes, g_flagsExpand);
    mmToolTip(m_Notes, _t("Enter notes to describe this budget entry"));

    // Patterns
    fgSizer1->Add(new wxStaticText(this, wxID_STATIC, _t("Match Patterns\non Import")), g_flagsH);
    wxBoxSizer* patternTable_Arranger = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* patternTable_Sizer = new wxBoxSizer(wxVERTICAL);
    m_patternTable = new wxGrid(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    m_patternTable->CreateGrid(1, 1);
    m_patternTable->HideColLabels();
    m_patternTable->SetRowLabelSize(wxGRID_AUTOSIZE);
    m_patternTable->DisableDragRowSize();
    m_patternTable->GetGridWindow()->SetSize(m_patternTable->GetSize());
    m_patternTable->SetColSize(0, m_patternTable->GetGridWindow()->GetSize().x);
    m_patternTable->GetGridWindow()->Bind(wxEVT_SIZE, &mmEditPayeeDialog::OnPatternTableSize, this);
    patternTable_Sizer->Add(m_patternTable, wxSizerFlags(g_flagsV).Expand().Proportion(0));
    patternTable_Arranger->Add(patternTable_Sizer, wxSizerFlags(g_flagsExpand).Border(wxALL, 0));
    mmToolTip(m_patternTable->GetGridWindow(),
        _t("Enter any string to match this payee on import.\nPatterns are tested in the order entered here.") + "\n\n" +
        _tu("Tips: Wildcard characters—question mark (?), asterisk (*)—can be used in search criteria.") + "\n" +
        _tu("Use the question mark (?) to find any single character—for example, “s?t” finds “sat” and “set”.") + "\n" +
        _tu("Use the asterisk (*) to find any number of characters—for example, “s*d” finds “sad” and “started”.") + "\n" +
        _tu("Use the asterisk (*) at the beginning to find any string in the middle of the sentence.") + "\n" +
        _t("Use regex: to match using regular expressions.")
    );

    patternButton_Arranger = new wxBoxSizer(wxVERTICAL);
    //Move up button
    wxBitmapButton* itemButton_MoveUp = new wxBitmapButton(this, wxID_UP, mmBitmapBundle(png::UPARROW, mmBitmapButtonSize));
    patternButton_Arranger->Add(itemButton_MoveUp, wxSizerFlags().Align(wxALIGN_LEFT).Border(wxBOTTOM,5));

    //Move down button
    wxBitmapButton* itemButton_MoveDown = new wxBitmapButton(this, wxID_DOWN, mmBitmapBundle(png::DOWNARROW, mmBitmapButtonSize));
    patternButton_Arranger->Add(itemButton_MoveDown, wxSizerFlags().Align(wxALIGN_LEFT).Border(wxTOP, 5));
    patternTable_Arranger->Add(patternButton_Arranger, g_flagsH);
    patternButton_Arranger->Show(false);
    fgSizer1->Add(patternTable_Arranger, wxSizerFlags(g_flagsExpand).Border(wxALL, 0));

    //Buttons
    wxBoxSizer* bSizer3 = new wxBoxSizer(wxHORIZONTAL);
    bSizer1->Add(bSizer3, wxSizerFlags(g_flagsV).Center());
    wxButton* itemButtonOK = new wxButton(this, wxID_OK, _t("&OK "));
    wxButton* itemButtonCancel = new wxButton(this, wxID_CANCEL, wxGetTranslation(g_CancelLabel));
    bSizer3->Add(itemButtonOK, g_flagsH);
    bSizer3->Add(itemButtonCancel, g_flagsH);

    Fit();
    wxSize sz = this->GetSize();
    SetSizeHints(sz.GetWidth(), sz.GetHeight(), -1, sz.GetHeight());
}

void mmEditPayeeDialog::fillControls()
{
    if (!this->m_payee) return;

    m_payeeName->SetValue(m_payee->PAYEENAME);
    m_hidden->SetValue(Model_Payee::is_hidden(m_payee));
    m_reference->SetValue(m_payee->NUMBER);
    m_website->SetValue(m_payee->WEBSITE);
    m_Notes->SetValue(m_payee->NOTES);
    if (!m_payee->PATTERN.IsEmpty())
    {
        Document json_doc;
        if (json_doc.Parse(m_payee->PATTERN.utf8_str()).HasParseError()) {
            json_doc.Parse("{}");
        }
        int row = 0;
        for (const auto& member : json_doc.GetObject()) {
            m_patternTable->AppendRows();
            m_patternTable->SetCellValue(row++, 0, wxString::FromUTF8(member.value.GetString()));
        }

    }
    const wxString category = Model_Category::full_name(m_payee->CATEGID);
    m_category->ChangeValue(category);
    ResizeDialog();
}

void mmEditPayeeDialog::OnOk(wxCommandEvent& /*event*/)
{
    if (!m_website->GetValue().empty() && !isValidURI(m_website->GetValue()))
        return mmErrorDialogs::ToolTip4Object(m_website, _t("Please enter a valid URL"), _t("Invalid URL"));

    if (!m_category->GetValue().IsEmpty() && !m_category->mmIsValid())
        return mmErrorDialogs::ToolTip4Object(m_category, _t("Invalid value"), _t("Category"));

    wxString name = m_payeeName->GetValue();

    if (name.IsEmpty())
        return mmErrorDialogs::ToolTip4Object(m_payeeName, _t("Invalid value"), _t("Payee"));

    Model_Payee::Data_Set payees = Model_Payee::instance().find(Model_Payee::PAYEENAME(name));
    if ((!m_payee && payees.empty()) ||
        (m_payee && (payees.empty() || name.CmpNoCase(m_payee->PAYEENAME) == 0)))
    {
        if (!m_payee)
            m_payee = Model_Payee::instance().create();

        m_payee->PAYEENAME = name;
        m_payee->ACTIVE = m_hidden->GetValue() ? 0 : 1;
        m_payee->NUMBER = m_reference->GetValue();
        m_payee->WEBSITE = m_website->GetValue();
        m_payee->NOTES = m_Notes->GetValue();
        m_payee->CATEGID = m_category->mmGetCategoryId();
        StringBuffer json_buffer;
        PrettyWriter<StringBuffer> json_writer(json_buffer);
        int key = 0;
        json_writer.StartObject();
        for (int row = 0; row < m_patternTable->GetNumberRows(); row++)
        {
            wxString pattern = m_patternTable->GetCellValue(row, 0);
            if (pattern == wxEmptyString) continue;
            if (pattern.StartsWith("regex:")) {
                wxRegEx regex(pattern.Right(pattern.length() - 6), wxRE_ICASE | wxRE_EXTENDED);
                if (!regex.IsValid()) return;
            }
            json_writer.Key(wxString::Format("%i", key++).utf8_str());
            json_writer.String(pattern.utf8_str());
        }
        json_writer.EndObject();
        m_payee->PATTERN = wxString::FromUTF8(json_buffer.GetString());
        Model_Payee::instance().save(m_payee);
        mmWebApp::MMEX_WebApp_UpdatePayee();
    }
    else
        return mmErrorDialogs::ToolTip4Object(m_payeeName, _t("A payee with this name already exists"), _t("Payee"));

    EndModal(wxID_OK);
}

void mmEditPayeeDialog::OnCancel(wxCommandEvent& /*event*/)
{
    EndModal(wxID_OK);
}

void mmEditPayeeDialog::OnMoveUp(wxCommandEvent& /*event*/)
{
    // ignore the last row
    m_patternTable->DeselectRow(m_patternTable->GetNumberRows() - 1);

    //wxGrid row and cell selections do not overlap, so need to get both and combine them
    wxArrayInt selectedRows = m_patternTable->GetSelectedRows();
    wxGridCellCoordsArray selectedCells = m_patternTable->GetSelectedCells();
    for (int i = 0; i < static_cast<int>(selectedCells.GetCount()); i++) {
        int row = selectedCells[i].GetRow();
        if(selectedRows.Index(row) == wxNOT_FOUND)
            selectedRows.Add(row);
    }

    // last movable row
    int maxRow = m_patternTable->GetNumberRows() - 2;

    // add the current working row (not always included in SelectedCells)
    int cursorRow = m_patternTable->GetGridCursorRow();
    if (cursorRow <= maxRow)
    {
        if (selectedRows.Index(cursorRow) == wxNOT_FOUND) selectedRows.Add(cursorRow);
        // the cursor will move up one cell if not already at the top
        if (cursorRow > 0) m_patternTable->MoveCursorUp(false);
    }

    //Loop over all rows
    for (int i = 0; i < static_cast<int>(selectedRows.GetCount()); i++) {
        // reselect the row (cleared by cursor move)
        m_patternTable->SelectRow(selectedRows[i], true);
        // we only want to move the cell up if the row above is not selected (so that selected blocks stay in order)
        // and the cell isn't already in the first row
        if (selectedRows[i] > 0 && selectedRows[i] <= maxRow && selectedRows.Index(selectedRows[i] - 1) == wxNOT_FOUND) {
            // swap the cell contents with the cell above
            wxString swapString = m_patternTable->GetCellValue(selectedRows[i] - 1, 0);
            m_patternTable->SetCellValue(selectedRows[i] - 1, 0, m_patternTable->GetCellValue(selectedRows[i], 0));
            m_patternTable->SetCellValue(selectedRows[i], 0, swapString);
            // deselect old row & select row above
            m_patternTable->DeselectRow(selectedRows[i]);
            m_patternTable->SelectRow(selectedRows[i] - 1, true);
            // remove the row from the array so that it is available for the next row
            selectedRows.Remove(selectedRows[i]);
            i--;
        }
    }

    // If bottom row is now empty, remove it
    if (m_patternTable->GetCellValue(maxRow, 0) == wxEmptyString)
    {
        m_patternTable->DeleteRows(m_patternTable->GetNumberRows() - 1);
        m_patternTable->DeselectRow(m_patternTable->GetNumberRows() - 1);
        ResizeDialog();
    }
}

void mmEditPayeeDialog::OnMoveDown(wxCommandEvent& /*event*/)
{
    // ignore the last row
    m_patternTable->DeselectRow(m_patternTable->GetNumberRows() - 1);

    //wxGrid row and cell selections do not overlap, so need to get both and combine them
    wxArrayInt selectedRows = m_patternTable->GetSelectedRows();
    wxGridCellCoordsArray selectedCells = m_patternTable->GetSelectedCells();
    for (int i = 0; i < static_cast<int>(selectedCells.GetCount()); i++) {
        int row = selectedCells[i].GetRow();
        if (selectedRows.Index(row) == wxNOT_FOUND)
            selectedRows.Add(row);
    }

    // last movable row
    int maxRow = m_patternTable->GetNumberRows() - 2;

    // add the current working row (not always included in SelectedCells)
    int cursorRow = m_patternTable->GetGridCursorRow();
    if (cursorRow < maxRow)
    {
        if (selectedRows.Index(cursorRow) == wxNOT_FOUND) selectedRows.Add(cursorRow);
        // the cursor will move down one cell if not already at the bottom
        m_patternTable->MoveCursorDown(false);
    }
    else if (cursorRow == maxRow) m_patternTable->SelectRow(cursorRow);

    //Loop over all rows (in reverse order)
    for (int i = selectedRows.GetCount() - 1; i >= 0; i--) {
        // reselect the row (cleared by cursor move)
        m_patternTable->SelectRow(selectedRows[i], true);
        // we only want to move the cell up if the row below is not selected (so that selected blocks stay in order)
        // and the cell isn't already in the last row
        if (selectedRows[i] < maxRow && selectedRows.Index(selectedRows[i] + 1) == wxNOT_FOUND) {
            // swap the cell contents with the cell below
            wxString swapString = m_patternTable->GetCellValue(selectedRows[i] + 1, 0);
            m_patternTable->SetCellValue(selectedRows[i] + 1, 0, m_patternTable->GetCellValue(selectedRows[i], 0));
            m_patternTable->SetCellValue(selectedRows[i], 0, swapString);
            // deselect old row & select row below
            m_patternTable->DeselectRow(selectedRows[i]);
            m_patternTable->SelectRow(selectedRows[i] + 1, true);
            // remove the row from the array so that it is available for the next row
            selectedRows.Remove(selectedRows[i]);
        }
    }

    // If bottom row is now empty, remove it
    if (m_patternTable->GetCellValue(maxRow, 0) == wxEmptyString)
    {
        m_patternTable->DeleteRows(m_patternTable->GetNumberRows() - 1);
        m_patternTable->DeselectRow(m_patternTable->GetNumberRows() - 1);
        ResizeDialog();
    }
}

void mmEditPayeeDialog::OnComboKey(wxKeyEvent& event)
{
    if ((event.GetKeyCode() == WXK_RETURN) && (event.GetId() == mmID_CATEGORY))
    {
        auto category = m_category->GetValue();
        if (category.empty())
        {
            mmCategDialog dlg(this, true, -1);
            dlg.ShowModal();
            if (dlg.getRefreshRequested())
                m_category->mmDoReInitialize();
            category = Model_Category::full_name(dlg.getCategId());
            m_category->ChangeValue(category);
            return;
        }
    }
    event.Skip();
}

void mmEditPayeeDialog::OnPatternTableChanged(wxGridEvent& event)
{
    int row = event.GetRow();
    // Reset cell text color
    m_patternTable->SetCellTextColour(row, 0, m_patternTable->GetDefaultCellTextColour());
    // Check the regex and format the text red if it is invalid
    wxString pattern = m_patternTable->GetCellValue(row, 0);
    if (pattern.StartsWith("regex:")) {
        wxRegEx regex(pattern.Right(pattern.length() - 6), wxRE_ICASE | wxRE_EXTENDED);
        if (!regex.IsValid())
            m_patternTable->SetCellTextColour(row, 0, *wxRED);
    }

    // if text was entered in the last row, add a row
    if (row == m_patternTable->GetNumberRows() - 1 && m_patternTable->GetCellValue(row, 0) != wxEmptyString)
        m_patternTable->AppendRows();
    // if text was deleted from the second to last row, delete a row so only one blank row is at the bottom
    else if (row == m_patternTable->GetNumberRows() - 2 && m_patternTable->GetCellValue(row, 0) == wxEmptyString)
        m_patternTable->DeleteRows(m_patternTable->GetNumberRows() - 1);
    ResizeDialog();
}

void mmEditPayeeDialog::OnPatternTableSize(wxSizeEvent&)
{
    m_patternTable->SetColSize(0, m_patternTable->GetGridWindow()->GetSize().x);
}

void mmEditPayeeDialog::ResizeDialog()
{
    // remove current size limits
    SetSizeHints(GetMinWidth(), -1, -1, -1);

    // show/hide the move buttons if there is more than one pattern
    patternButton_Arranger->Show(m_patternTable->GetNumberRows() > 2);
    Layout();
    int width = GetSize().GetWidth();
    wxSize bestSize = GetBestSize();
    SetSizeHints(GetMinWidth(), bestSize.GetHeight(), -1, bestSize.GetHeight());
    SetSize(wxSize((bestSize.GetWidth() > width ? bestSize.GetWidth() : width), bestSize.GetHeight()));
}

// mmPayeeDialog
// ------------------------------------------------------------------------------------------

wxIMPLEMENT_DYNAMIC_CLASS(mmPayeeDialog, wxDialog);

wxBEGIN_EVENT_TABLE(mmPayeeDialog, wxDialog)
EVT_BUTTON(wxID_CANCEL, mmPayeeDialog::OnCancel)
EVT_BUTTON(wxID_OK, mmPayeeDialog::OnOk)
EVT_BUTTON(wxID_APPLY, mmPayeeDialog::OnMagicButton)
EVT_TOGGLEBUTTON(wxID_SELECTALL, mmPayeeDialog::OnShowHiddenToggle)
EVT_TEXT(wxID_FIND, mmPayeeDialog::OnTextChanged)
EVT_LIST_COL_CLICK(wxID_ANY, mmPayeeDialog::OnSort)
EVT_LIST_ITEM_ACTIVATED(wxID_ANY, mmPayeeDialog::OnListItemActivated)
EVT_LIST_ITEM_RIGHT_CLICK(wxID_ANY, mmPayeeDialog::OnItemRightClick)
EVT_MENU_RANGE(MENU_DEFINE_CATEGORY, MENU_RELOCATE_PAYEE, mmPayeeDialog::OnMenuSelected)
wxEND_EVENT_TABLE()


mmPayeeDialog::~mmPayeeDialog()
{
    Model_Infotable::instance().setSize("PAYEES_DIALOG_SIZE", GetSize());
}

mmPayeeDialog::mmPayeeDialog(wxWindow* parent, bool payee_choose, const wxString& name, const wxString& payee_selected) :
    m_payee_choose(payee_choose)
    , m_init_selected_payee(payee_selected)
    , m_sort(PAYEE_NAME)
    , m_lastSort(PAYEE_NAME)
{
    this->SetFont(parent->GetFont());
    m_hiddenColor = mmThemeMetaColour(meta::COLOR_HIDDEN);
    m_normalColor = this->GetForegroundColour();

    m_showHiddenPayees = Model_Setting::instance().getBool("SHOW_HIDDEN_PAYEES", true);
    Create(parent, name);

    const wxAcceleratorEntry entries[] =
    {
        wxAcceleratorEntry(wxACCEL_NORMAL, WXK_F2, MENU_EDIT_PAYEE),
        wxAcceleratorEntry(wxACCEL_NORMAL, WXK_INSERT, MENU_NEW_PAYEE),
        wxAcceleratorEntry(wxACCEL_NORMAL, WXK_DELETE, MENU_DELETE_PAYEE),
        wxAcceleratorEntry(wxACCEL_NORMAL, WXK_CONTROL_H, MENU_SHOW_HIDDEN)
    };

    wxAcceleratorTable tab(sizeof(entries) / sizeof(*entries), entries);
    SetAcceleratorTable(tab);
}

int64 mmPayeeDialog::getPayeeId() const
{
    long sel = payeeListBox_->GetFirstSelected();
    return sel > -1 ? (reinterpret_cast<RowData*>(payeeListBox_->GetItemData(sel)))->payeeId : -1;
}

void mmPayeeDialog::FindSelectedPayees()
{
    m_selectedItems.clear();
    long idx = payeeListBox_->GetFirstSelected();
    while (idx != -1) {
        RowData* entry = reinterpret_cast<RowData*>(payeeListBox_->GetItemData(idx));
        entry->tidx = idx;
        m_selectedItems.push_back(entry);
        idx = payeeListBox_->GetNextSelected(idx);
    }
}

std::list<int64> mmPayeeDialog::getSelectedPayees()
{
    std::list<int64> itemsSelected;
    long idx = payeeListBox_->GetFirstSelected();
    while (idx != -1) {
        RowData* row = reinterpret_cast<RowData*>(payeeListBox_->GetItemData(idx));
        itemsSelected.push_back(row->payeeId);
        idx = payeeListBox_->GetNextSelected(idx);
    }
    return itemsSelected;
};

bool mmPayeeDialog::isPayeeWithStateSelected(bool state)
{
    bool result = false;
    FindSelectedPayees();
    for(RowData* rdata : m_selectedItems) {
        if (rdata->active == state) {
            result = true;
            break;
        }
    }
    return result;
}

void mmPayeeDialog::Create(wxWindow* parent, const wxString &name)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);

    long style = wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER;
    if (!wxDialog::Create(parent, wxID_ANY, _t("Payee Manager")
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
        wxLC_REPORT | wxLC_AUTOARRANGE);
    payeeListBox_->SetMinSize(wxSize(250, 100));

    payeeListBox_->InsertColumn(PAYEE_NAME, _t("Name"), wxLIST_FORMAT_LEFT, 150);
    payeeListBox_->InsertColumn(PAYEE_HIDDEN, _t("Hidden"), wxLIST_FORMAT_CENTER, 50);
    wxString cn = (Option::instance().getTransCategoryNone() == Option::LASTUSED) ? _t("Last Used Category") : _t("Default Category");
    payeeListBox_->InsertColumn(PAYEE_CATEGORY, cn, wxLIST_FORMAT_LEFT, 150);
    payeeListBox_->InsertColumn(PAYEE_NUMBER, _t("Reference"), wxLIST_FORMAT_LEFT, 150);
    payeeListBox_->InsertColumn(PAYEE_WEBSITE, _t("Website"), wxLIST_FORMAT_LEFT, 150);
    payeeListBox_->InsertColumn(PAYEE_NOTES, _t("Notes"), wxLIST_FORMAT_LEFT, 150);
    payeeListBox_->InsertColumn(PAYEE_PATTERN, _t("Match Pattern"), wxLIST_FORMAT_LEFT, 150);
    payeeListBox_->InsertColumn(PAYEE_USED, _t("Used"), wxLIST_FORMAT_RIGHT, 50);

    mainBoxSizer->Add(payeeListBox_, wxSizerFlags(g_flagsExpand).Border(wxALL, 10));

    wxPanel* buttons_panel = new wxPanel(this, wxID_ANY);
    mainBoxSizer->Add(buttons_panel, wxSizerFlags(g_flagsExpand).Proportion(0));
    wxBoxSizer*  tools_sizer = new wxBoxSizer(wxVERTICAL);
    buttons_panel->SetSizer(tools_sizer);

    wxBoxSizer* tools_sizer2 = new wxBoxSizer(wxHORIZONTAL);
    tools_sizer->Add(tools_sizer2, wxSizerFlags(g_flagsExpand).Border(0));

    m_magicButton = new wxBitmapButton(buttons_panel
        , wxID_APPLY, mmBitmapBundle(png::MORE_OPTIONS, mmBitmapButtonSize));
    mmToolTip(m_magicButton, _t("Other tools"));
    tools_sizer2->Add(m_magicButton, g_flagsH);

    m_tbShowAll = new wxToggleButton(buttons_panel, wxID_SELECTALL, _t("Show &hidden"), wxDefaultPosition
        , wxDefaultSize);
    m_tbShowAll->SetValue(m_showHiddenPayees);
    mmToolTip(m_tbShowAll, _t("Show hidden payees") + " (Ctrl-h)");
    tools_sizer2->Add(m_tbShowAll, g_flagsH);

    m_maskTextCtrl = new wxSearchCtrl(buttons_panel, wxID_FIND);
    m_maskTextCtrl->SetFocus();
    tools_sizer2->Prepend(m_maskTextCtrl, g_flagsExpand);
    tools_sizer2->Prepend(new wxStaticText(buttons_panel, wxID_STATIC, _t("&Search")), g_flagsH);

    wxStdDialogButtonSizer*  buttons_sizer = new wxStdDialogButtonSizer;
    tools_sizer->Add(buttons_sizer, wxSizerFlags(g_flagsV).Center());
    wxButton* buttonOK = new wxButton(buttons_panel, wxID_OK, _t("&OK "));
    wxButton* btnCancel = new wxButton(buttons_panel, wxID_CANCEL, wxGetTranslation(g_CancelLabel));

    buttons_sizer->Add(buttonOK, g_flagsH);
    buttons_sizer->Add(btnCancel, g_flagsH);

    this->SetSizerAndFit(mainBoxSizer);
}

void mmPayeeDialog::fillControls()
{
    this->Freeze();
    payeeListBox_->DeleteAllItems();
    m_rowData.clear();

    Model_Payee::Data_Set payees = Model_Payee::instance().FilterPayees(m_maskStr, m_showHiddenPayees);

    if (m_sort != PAYEE_USED) {
        switch (m_sort)
        {
            case PAYEE_HIDDEN:
                std::stable_sort(payees.begin(), payees.end(), SorterByACTIVE());
                break;
            case PAYEE_CATEGORY:
                std::stable_sort(payees.begin(), payees.end(), [] (Model_Payee::Data x, Model_Payee::Data y)
                {
                    return(
                        CaseInsensitiveLocaleCmp(
                            Model_Category::instance().full_name(x.CATEGID)
                            , Model_Category::instance().full_name(y.CATEGID)) < 0
                        );
                });
                break;
            case PAYEE_NUMBER:
                std::stable_sort(payees.begin(), payees.end(), SorterByNUMBER());
                break;
            case PAYEE_WEBSITE:
                std::stable_sort(payees.begin(), payees.end(), SorterByWEBSITE());
                break;
            case PAYEE_NOTES:
                std::stable_sort(payees.begin(), payees.end(), SorterByNOTES());
                break;
            case PAYEE_PATTERN:
                std::stable_sort(payees.begin(), payees.end(), SorterByPATTERN());
                break;
            default:
                std::stable_sort(payees.begin(), payees.end(), SorterByPAYEENAME());
                break;
        }
        if (m_sortReverse)
            std::reverse(payees.begin(), payees.end());
    }

    long idx = 0;
    for (const auto& payee : payees) {
        wxListItem item;
        item.SetId(idx);

        RowData* rdata = new RowData;
        rdata->payeeId = payee.PAYEEID;
        rdata->active = payee.ACTIVE == 1;
        rdata->count = Model_Payee::instance().getUseCount(payee.PAYEEID);
        m_rowData.push_back(rdata);

        payeeListBox_->InsertItem(item);
        addPayeeDataIntoItem(idx, &payee, rdata->count);
        payeeListBox_->SetItemData(idx, reinterpret_cast<wxIntPtr>(rdata));

        idx++;
    }

    if (m_sort == PAYEE_USED) {
        payeeListBox_->SortItems(mmPayeeDialog::SortCallback, reinterpret_cast<wxIntPtr>(this));
    }

    if (payeeListBox_->GetSelectedItemCount() > 0) {
        payeeListBox_->EnsureVisible(payeeListBox_->GetFirstSelected());
    }
    this->Thaw();
}

void mmPayeeDialog::addPayeeDataIntoItem(long idx, const Model_Payee::Data* payee, int count)
{
    payeeListBox_->SetItem(idx, 0, payee->PAYEENAME);
    if (!m_init_selected_payee.IsEmpty() && payee->PAYEENAME.CmpNoCase(m_init_selected_payee) == 0) {
        payeeListBox_->Select(idx);
    }
    payeeListBox_->SetItem(idx, 1, payee->ACTIVE == 0 ? L"\u2713" : L"");
    payeeListBox_->SetItem(idx, 2, Model_Category::instance().full_name(payee->CATEGID));
    payeeListBox_->SetItem(idx, 3, payee->NUMBER);
    payeeListBox_->SetItem(idx, 4, payee->WEBSITE);
    wxString value = payee->NOTES;
    value.Replace("\n", " ");
    payeeListBox_->SetItem(idx, 5, value);
    value = "";
    if (!payee->PATTERN.IsEmpty()) {
        Document json_doc;
        if (json_doc.Parse(payee->PATTERN.utf8_str()).HasParseError()) {
            json_doc.Parse("{}");
        }
        for (auto& member : json_doc.GetObject()) {
            if (!value.IsEmpty()) { value.Append(" "); }
            value.Append(wxString::FromUTF8(member.value.GetString()));
        }
    }
    payeeListBox_->SetItem(idx, 6, value);
    payeeListBox_->SetItemTextColour(idx, payee->ACTIVE == 0 ? m_hiddenColor : m_normalColor);
    payeeListBox_->SetItem(idx, 7, wxString::Format("%d", count));
}

void mmPayeeDialog::OnListItemActivated(wxListEvent& WXUNUSED(event))
{
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
    mmEditPayeeDialog dlg(this, nullptr);
    dlg.ShowModal();
    refreshRequested_ = true;
    fillControls();
}

void mmPayeeDialog::EditPayee()
{
    long sel = payeeListBox_->GetFocusedItem();
    if (sel > -1) {
        RowData* rdata = reinterpret_cast<RowData*>(payeeListBox_->GetItemData(sel));
        Model_Payee::Data *payee = Model_Payee::instance().get(rdata->payeeId);
        mmEditPayeeDialog dlg(this, payee);
        if (dlg.ShowModal() == wxID_OK) {
            rdata->active = payee->ACTIVE == 1;
            if (!m_showHiddenPayees && !rdata->active) {  // is hidden, refresh to remove Item
                fillControls();
            }
            else {
              addPayeeDataIntoItem(payeeListBox_->GetFocusedItem(), payee, rdata->count);
            }
        }
        refreshRequested_ = true;
    }
}

void mmPayeeDialog::DeletePayee()
{
    FindSelectedPayees();
    for(RowData* rdata : m_selectedItems) {
        const Model_Payee::Data *payee = Model_Payee::instance().get(rdata->payeeId);
        if (Model_Payee::instance().is_used(rdata->payeeId)) {
            wxString deletePayeeErrMsg = _t("Payee in use.");
            deletePayeeErrMsg
                << "\n"
                << payee->PAYEENAME
                << "\n"
                << _t("It will be not removed")
                << "\n\n"
                << _t("Tip: Change all transactions using this Payee to another Payee"
                    " using the merge command:")
                << "\n\n" << _tu("Tools → Merge → Payees");
            wxMessageBox(deletePayeeErrMsg, _t("Payee Manager: Delete Error"), wxOK | wxICON_ERROR);
            continue;
        }
        Model_Checking::Data_Set deletedTrans = Model_Checking::instance().find(Model_Checking::PAYEEID(rdata->payeeId));
        wxMessageDialog msgDlg(this
            , _t("Deleted transactions exist which use this payee.")
                + "\n\n" + _t("Deleting the payee will also automatically purge the associated deleted transactions.")
                + "\n\n" + _t("Do you want to continue?")
            , _t("Confirm Payee Deletion")
            , wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);
        if (deletedTrans.empty() || msgDlg.ShowModal() == wxID_YES)
        {
            if (!deletedTrans.empty()) {
                Model_Checking::instance().Savepoint();
                Model_Splittransaction::instance().Savepoint();
                Model_Attachment::instance().Savepoint();
                Model_CustomFieldData::instance().Savepoint();
                const wxString& RefType = Model_Checking::refTypeName;

                for (auto& tran : deletedTrans) {
                    Model_Checking::instance().remove(tran.TRANSID);
                    mmAttachmentManage::DeleteAllAttachments(RefType, tran.TRANSID);
                    Model_CustomFieldData::DeleteAllData(RefType, tran.TRANSID);
                }

                Model_Checking::instance().ReleaseSavepoint();
                Model_Splittransaction::instance().ReleaseSavepoint();
                Model_Attachment::instance().ReleaseSavepoint();
                Model_CustomFieldData::instance().ReleaseSavepoint();
            }

            Model_Payee::instance().remove(payee->PAYEEID);
            mmAttachmentManage::DeleteAllAttachments(Model_Payee::refTypeName, payee->PAYEEID);
            refreshRequested_ = true;
            fillControls();
        }
    }
}

void mmPayeeDialog::DefineDefaultCategory()
{
    FindSelectedPayees();
    int nb = size(m_selectedItems);
    if (nb > 0) {
        Model_Payee::Data *payee = Model_Payee::instance().get(m_selectedItems.front()->payeeId);
        mmCategDialog dlg(this, true, nb == 1 ? payee->CATEGID : -1);
        if (dlg.ShowModal() == wxID_OK) {
            for(RowData* rdata : m_selectedItems) {
                payee = Model_Payee::instance().get(rdata->payeeId);
                payee->CATEGID = dlg.getCategId();
                Model_Payee::instance().save(payee);
                mmWebApp::MMEX_WebApp_UpdatePayee();
                addPayeeDataIntoItem(rdata->tidx, payee, rdata->count);
            }
        }
    }
    else {
        mmCategDialog dlg(this, false, -1);  // show category dialog only
        dlg.ShowModal();
    }
}

void mmPayeeDialog::RemoveDefaultCategory()
{
    Model_Payee::Data *payee;
    FindSelectedPayees();
    for(RowData* rdata : m_selectedItems) {
        payee = Model_Payee::instance().get(rdata->payeeId);
        payee->CATEGID = -1;
        Model_Payee::instance().save(payee);
        mmWebApp::MMEX_WebApp_UpdatePayee();
        addPayeeDataIntoItem(rdata->tidx, payee, rdata->count);
    }
}

void mmPayeeDialog::OnOrganizeAttachments()
{
    wxString RefType = Model_Payee::refTypeName;
    long sel = payeeListBox_->GetFocusedItem();
    if (sel > -1) {
        RowData* rdata = reinterpret_cast<RowData*>(payeeListBox_->GetItemData(sel));
        mmAttachmentDialog dlg(this, RefType, rdata->payeeId);
        dlg.ShowModal();
        refreshRequested_ = true;
    }
}

void mmPayeeDialog::OnPayeeRelocate()
{
    relocatePayeeDialog dlg(this, getPayeeId());
    if (dlg.ShowModal() == wxID_OK) {
        wxString msgStr;
        msgStr << _t("Merge payees completed") << "\n\n"
            << wxString::Format(_t("Records have been updated in the database: %i")
                , dlg.updatedPayeesCount())
            << "\n\n";
        wxMessageBox(msgStr, _t("Merge payees result"));
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
        case MENU_ITEM_HIDE:
        {
            long idx = payeeListBox_->GetFirstSelected();
            while (idx != -1) {
                ToggleHide(idx, false);
                idx = payeeListBox_->GetNextSelected(idx);
            }
            if (!m_showHiddenPayees) {
                fillControls();
            }
            else {
                payeeListBox_->Refresh();
            }
            break;
        }
        case MENU_ITEM_UNHIDE:
        {
            long idx = payeeListBox_->GetFirstSelected();
            while (idx != -1) {
                ToggleHide(idx, true);
                idx = payeeListBox_->GetNextSelected(idx);
            }
            payeeListBox_->Refresh();
            break;
        }
        case MENU_SHOW_TRANSACTIONS:
        {
            FindSelectedPayees();
            m_addActionRequested = true;
            EndModal(wxID_OK);
            break;
        }
        case MENU_SHOW_HIDDEN:
        {
            m_showHiddenPayees = m_tbShowAll->GetValue();
            Model_Setting::instance().setBool("SHOW_HIDDEN_PAYEES", m_showHiddenPayees);
            fillControls();
            break;
        }
        default: break;
        }
}

void mmPayeeDialog::ToggleHide(long idx, bool state) {
    RowData* rdata = reinterpret_cast<RowData*>(payeeListBox_->GetItemData(idx));
    if (rdata->payeeId > -1) {
        rdata->active = state;
        Model_Payee::Data *payee = Model_Payee::instance().get(rdata->payeeId);
        payee->ACTIVE = state ? 1 : 0;
        Model_Payee::instance().save(payee);
        payeeListBox_->SetItemTextColour(idx, state ? m_normalColor : m_hiddenColor);
        payeeListBox_->SetItem(idx, 1, state ? L"" : L"\u2713");
    }
}

void mmPayeeDialog::OnMagicButton(wxCommandEvent& WXUNUSED(event))
{
    wxListEvent evt;
    OnItemRightClick(evt);
}

void mmPayeeDialog::OnItemRightClick(wxListEvent& event)
{
    wxMenu mainMenu;
    mainMenu.Append(new wxMenuItem(&mainMenu, MENU_EDIT_PAYEE, _t("&Edit ")));
    mainMenu.AppendSeparator();
    mainMenu.Append(new wxMenuItem(&mainMenu, MENU_ITEM_HIDE, _t("&Hide Selected")));
    mainMenu.Append(new wxMenuItem(&mainMenu, MENU_ITEM_UNHIDE, _t("&Show Selected")));
    mainMenu.AppendSeparator();
    mainMenu.Append(new wxMenuItem(&mainMenu, MENU_DELETE_PAYEE, _t("&Remove ")));
    mainMenu.AppendSeparator();
    mainMenu.Append(new wxMenuItem(&mainMenu, MENU_NEW_PAYEE, _t("&Add ")));
    mainMenu.AppendSeparator();
    mainMenu.Append(new wxMenuItem(&mainMenu, MENU_DEFINE_CATEGORY, _t("Define &Category")));
    mainMenu.Append(new wxMenuItem(&mainMenu, MENU_REMOVE_CATEGORY, _t("Remove Categor&y")));
    mainMenu.AppendSeparator();
    mainMenu.Append(new wxMenuItem(&mainMenu, MENU_ORGANIZE_ATTACHMENTS, _t("Attachment &Manager")));
    mainMenu.AppendSeparator();
    mainMenu.Append(new wxMenuItem(&mainMenu, MENU_RELOCATE_PAYEE, _t("Merge &Payee")));
    mainMenu.Append(new wxMenuItem(&mainMenu, MENU_SHOW_TRANSACTIONS, _t("&Transaction Report")));

    int nb = payeeListBox_->GetSelectedItemCount();

    mainMenu.Enable(MENU_EDIT_PAYEE, nb == 1);
    mainMenu.Enable(MENU_RELOCATE_PAYEE, nb == 1);
    mainMenu.Enable(MENU_SHOW_TRANSACTIONS, nb == 1);
    mainMenu.Enable(MENU_ORGANIZE_ATTACHMENTS, nb == 1);
    mainMenu.Enable(MENU_REMOVE_CATEGORY, nb > 0);

    mainMenu.Enable(MENU_ITEM_HIDE, nb > 0 && isPayeeWithStateSelected(true));
    mainMenu.Enable(MENU_ITEM_UNHIDE, nb > 0 && m_showHiddenPayees && isPayeeWithStateSelected(false));

    mainMenu.Enable(MENU_DELETE_PAYEE, nb > 0);

    PopupMenu(&mainMenu);
    event.Skip();
}

int mmPayeeDialog::CompareRows(RowData* r1, RowData* r2)
{
    int cmp = (r1->count < r2->count) ? -1 : (r1->count > r2->count ? 1 : 0);
    return m_sortReverse ? cmp : -cmp;
}

int wxCALLBACK mmPayeeDialog::SortCallback(wxIntPtr itemData1, wxIntPtr itemData2, wxIntPtr sortData)
{
    mmPayeeDialog* self = reinterpret_cast<mmPayeeDialog*>(sortData);
    RowData* row1 = reinterpret_cast<RowData*>(itemData1);
    RowData* row2 = reinterpret_cast<RowData*>(itemData2);
    return self->CompareRows(row1, row2);
}

void mmPayeeDialog::OnSort(wxListEvent& event)
{
    m_lastSort = m_sort;
    m_sort = event.GetColumn();
    if (m_sort == m_lastSort)
        m_sortReverse = !m_sortReverse;
    if (m_sort == PAYEE_USED) {
        payeeListBox_->SortItems(mmPayeeDialog::SortCallback, reinterpret_cast<wxIntPtr>(this));
    }
    else {
        fillControls();
    }
}

void mmPayeeDialog::OnShowHiddenToggle(wxCommandEvent& WXUNUSED(event))
{
    m_showHiddenPayees = m_tbShowAll->GetValue();
    Model_Setting::instance().setBool("SHOW_HIDDEN_PAYEES", m_showHiddenPayees);
    fillControls();
}

void mmPayeeDialog::OnCancel(wxCommandEvent& WXUNUSED(event))
{
    EndModal(wxID_CANCEL);
}

void mmPayeeDialog::OnOk(wxCommandEvent& WXUNUSED(event))
{
    if (payeeListBox_->GetItemCount() < 1) {
        AddPayee();
    }
    else {
        EndModal(wxID_OK);
    }
}
