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
    if (!wxDialog::Create(parent, wxID_ANY, _("Edit Payee")
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
    fgSizer1->Add(new wxStaticText(this, wxID_STATIC, _("Payee")), g_flagsH);
    m_payeeName = new wxTextCtrl(this, wxID_ANY, "");
    mmToolTip(m_payeeName, _("Enter the Name of the Payee. This name can be renamed at any time."));
    fgSizer1->Add(m_payeeName, g_flagsExpand);

    // Hidden Status
    fgSizer1->Add(new wxStaticText(this, wxID_STATIC, _("Hidden")), g_flagsH);
    m_hidden = new wxCheckBox(this, wxID_ANY, "");
    mmToolTip(m_hidden, _("Indicate whether the payee should hidden and not presented in the payee list for new transactions"));
    fgSizer1->Add(m_hidden, g_flagsExpand);

    // Category
    const wxString title = (Option::instance().getTransCategoryNone() == Option::LASTUSED) ?
                                _("Last Used Category") : _("Default Category");
    fgSizer1->Add(new wxStaticText(this, wxID_STATIC, title), g_flagsH);
    m_category = new mmComboBoxCategory(this, mmID_CATEGORY, wxDefaultSize, -1, true);
    mmToolTip(m_category, _("The category used as default for this payee"));
    fgSizer1->Add(m_category, g_flagsExpand);                     

    // Reference
    fgSizer1->Add(new wxStaticText(this, wxID_STATIC, _("Reference")), g_flagsH);
    m_reference = new wxTextCtrl(this, wxID_ANY, "");
    mmToolTip(m_reference, _("A reference to be used for the payee, e.g. bank account number"));
    fgSizer1->Add(m_reference, g_flagsExpand);

    // Website
    fgSizer1->Add(new wxStaticText(this, wxID_STATIC, _("Website")), g_flagsH);
    m_website = new wxTextCtrl(this, wxID_ANY, "");
    mmToolTip(m_website, _("Website URL associated with the payee"));
    fgSizer1->Add(m_website, g_flagsExpand);

    // Notes
    fgSizer1->Add(new wxStaticText(this, wxID_STATIC, _("Notes")), g_flagsH);
    m_Notes = new wxTextCtrl(this, wxID_ANY, ""
        , wxDefaultPosition, wxSize(-1, -1), wxTE_MULTILINE);
    fgSizer1->Add(m_Notes, g_flagsExpand);
    mmToolTip(m_Notes, _("Enter notes to describe this budget entry"));

    // Patterns
    fgSizer1->Add(new wxStaticText(this, wxID_STATIC, _("Match Patterns\non Import")), g_flagsH);
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
        _("Enter any string to match this payee on import.\nPatterns are tested in the order entered here.") + "\n\n" +
        _u("Tips: Wildcard characters—question mark (?), asterisk (*)—can be used in search criteria.") + "\n" +
        _u("Use the question mark (?) to find any single character—for example, “s?t” finds “sat” and “set”.") + "\n" +
        _u("Use the asterisk (*) to find any number of characters—for example, “s*d” finds “sad” and “started”.") + "\n" +
        _u("Use the asterisk (*) at the beginning to find any string in the middle of the sentence.") + "\n" +
        _("Use regex: to match using regular expressions.")
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
    wxButton* itemButtonOK = new wxButton(this, wxID_OK, _("&OK "));
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
        return mmErrorDialogs::ToolTip4Object(m_website, _("Please enter a valid URL"), _("Invalid URL"));

    if (!m_category->GetValue().IsEmpty() && !m_category->mmIsValid())
        return mmErrorDialogs::ToolTip4Object(m_category, _("Invalid value"), _("Category"));

    wxString name = m_payeeName->GetValue();

    if (name.IsEmpty())
        return mmErrorDialogs::ToolTip4Object(m_payeeName, _("Invalid value"), _("Payee"));

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
        return mmErrorDialogs::ToolTip4Object(m_payeeName, _("A payee with this name already exists"), _("Payee"));
    
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
    Model_Infotable::instance().setSize("PAYEES_DIALOG_SIZE", GetSize());
}

mmPayeeDialog::mmPayeeDialog(wxWindow* parent, bool payee_choose, const wxString& name, const wxString& payee_selected) :
    m_payee_choose(payee_choose)
    , m_init_selected_payee(payee_selected)
    , m_sort(PAYEE_NAME)
    , m_lastSort(PAYEE_NAME)    
{
    ColName_[PAYEE_NAME] = _("Name");
    ColName_[PAYEE_HIDDEN] = _("Hidden");
    ColName_[PAYEE_CATEGORY]  = (Option::instance().getTransCategoryNone() == Option::LASTUSED) ?
                                _("Last Used Category") : _("Default Category");
    ColName_[PAYEE_NUMBER] = _("Reference");
    ColName_[PAYEE_WEBSITE] = _("Website");
    ColName_[PAYEE_NOTES] = _("Notes");
    ColName_[PAYEE_PATTERN] = _("Match Pattern");


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

int64 mmPayeeDialog::FindSelectedPayee()
{
    int sel = payeeListBox_->GetFocusedItem();
    if (-1 != sel)
    {
        return (payee_idx_map_[sel]);
    } else
        return -1;
}
void mmPayeeDialog::FindSelectedPayees(std::list<int64> & indexes)
{
    long itemIndex = -1;    

    while ((itemIndex = payeeListBox_->GetNextItem(itemIndex,
          wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED)) != wxNOT_FOUND) {
        // Got the selected item index
        wxLogDebug(payeeListBox_->GetItemText(itemIndex));
        indexes.push_back(payee_idx_map_[itemIndex]);
    }
    if (itemIndex == -1 && indexes.empty()) {
        indexes.push_back(-1);
    }
}


void mmPayeeDialog::Create(wxWindow* parent, const wxString &name)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);

    long style = wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER;
    if (!wxDialog::Create(parent, wxID_ANY, _("Payee Manager")
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

    wxListItem col0, col1, col2, col3, col4, col5, col6;

    col0.SetId(PAYEE_NAME);
    col0.SetText(ColName_[PAYEE_NAME]);
    col0.SetWidth(150);
    payeeListBox_->InsertColumn(0, col0);

    col1.SetId(PAYEE_HIDDEN);
    col1.SetText(ColName_[PAYEE_HIDDEN]);
    col1.SetAlign(wxLIST_FORMAT_CENTER);
    col1.SetWidth(50);
    payeeListBox_->InsertColumn(1, col1);

    col2.SetId(PAYEE_CATEGORY);
    col2.SetText(ColName_[PAYEE_CATEGORY]);
    col2.SetWidth(250);
    payeeListBox_->InsertColumn(2, col2);

    col3.SetId(PAYEE_NUMBER);
    col3.SetText(ColName_[PAYEE_NUMBER]);
    col3.SetWidth(150);
    payeeListBox_->InsertColumn(3, col3);

    col4.SetId(PAYEE_WEBSITE);
    col4.SetText(ColName_[PAYEE_WEBSITE]);
    col4.SetWidth(150);
    payeeListBox_->InsertColumn(4, col4);

    col5.SetId(PAYEE_NOTES);
    col5.SetText(ColName_[PAYEE_NOTES]);
    col5.SetWidth(150);
    payeeListBox_->InsertColumn(5, col5);

    col6.SetId(PAYEE_PATTERN);
    col6.SetText(ColName_[PAYEE_PATTERN]);
    col6.SetWidth(150);
    payeeListBox_->InsertColumn(6, col6);

    mainBoxSizer->Add(payeeListBox_, wxSizerFlags(g_flagsExpand).Border(wxALL, 10));

    wxPanel* buttons_panel = new wxPanel(this, wxID_ANY);
    mainBoxSizer->Add(buttons_panel, wxSizerFlags(g_flagsExpand).Proportion(0));
    wxBoxSizer*  tools_sizer = new wxBoxSizer(wxVERTICAL);
    buttons_panel->SetSizer(tools_sizer);

    wxBoxSizer* tools_sizer2 = new wxBoxSizer(wxHORIZONTAL);
    tools_sizer->Add(tools_sizer2, wxSizerFlags(g_flagsExpand).Border(0));

    m_magicButton = new wxBitmapButton(buttons_panel
        , wxID_APPLY, mmBitmapBundle(png::MORE_OPTIONS, mmBitmapButtonSize));
    mmToolTip(m_magicButton, _("Other tools"));
    tools_sizer2->Add(m_magicButton, g_flagsH);

    m_maskTextCtrl = new wxSearchCtrl(buttons_panel, wxID_FIND);
    m_maskTextCtrl->SetFocus();
    tools_sizer2->Prepend(m_maskTextCtrl, g_flagsExpand);
    tools_sizer2->Prepend(new wxStaticText(buttons_panel, wxID_STATIC, _("Search")), g_flagsH);

    wxStdDialogButtonSizer*  buttons_sizer = new wxStdDialogButtonSizer;
    tools_sizer->Add(buttons_sizer, wxSizerFlags(g_flagsV).Center());
    wxButton* buttonOK = new wxButton(buttons_panel, wxID_OK, _("&OK "));
    wxButton* btnCancel = new wxButton(buttons_panel, wxID_CANCEL, wxGetTranslation(g_CancelLabel));

    buttons_sizer->Add(buttonOK, g_flagsH);
    buttons_sizer->Add(btnCancel, g_flagsH);

    this->SetSizerAndFit(mainBoxSizer);
}

void mmPayeeDialog::fillControls()
{
    this->Freeze();
    payeeListBox_->DeleteAllItems();
    payee_idx_map_.clear();
    m_payee_id = -1;
    
    Model_Payee::Data_Set payees = Model_Payee::instance().FilterPayees(m_maskStr);
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
    case PAYEE_NAME:
    default:
        std::stable_sort(payees.begin(), payees.end(), SorterByPAYEENAME());
        break; 
    }

    if (m_sortReverse)
        std::reverse(payees.begin(), payees.end());

    int idx = 0;
    for (const auto& payee : payees)
    {
        wxListItem item;
        item.SetId(idx);
        payee_idx_map_[idx] = payee.PAYEEID.GetValue();
        payeeListBox_->InsertItem(item);
        const wxString full_category_name = Model_Category::instance().full_name(payee.CATEGID);
        payeeListBox_->SetItem(idx, 0, payee.PAYEENAME);
        if (!m_init_selected_payee.IsEmpty() && payee.PAYEENAME.CmpNoCase(m_init_selected_payee) == 0) {
            payeeListBox_->Select(idx);
        }
        payeeListBox_->SetItem(idx, 1, payee.ACTIVE == 0 ? L"\u2713" : L"");        
        payeeListBox_->SetItem(idx, 2, full_category_name);
        payeeListBox_->SetItem(idx, 3, payee.NUMBER);
        payeeListBox_->SetItem(idx, 4, payee.WEBSITE);
        wxString value = payee.NOTES;
        value.Replace("\n", " ");
        payeeListBox_->SetItem(idx, 5, value);
        value = "";
        if (!payee.PATTERN.IsEmpty())
        {
            Document json_doc;
            if (json_doc.Parse(payee.PATTERN.utf8_str()).HasParseError()) {
                json_doc.Parse("{}");
            }
            for (auto& member : json_doc.GetObject()) {
                if (!value.IsEmpty()) { value.Append(" "); }
                value.Append(wxString::FromUTF8(member.value.GetString()));
            }
        }
        payeeListBox_->SetItem(idx, 6, value);
        
        idx++;
    }
    if (payeeListBox_->GetSelectedItemCount() > 0)
        payeeListBox_->EnsureVisible(payeeListBox_->GetFirstSelected());
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
    mmEditPayeeDialog dlg(this, nullptr);
    dlg.ShowModal();
    refreshRequested_ = true;
    fillControls();
}

void mmPayeeDialog::EditPayee()
{
    Model_Payee::Data *payee = Model_Payee::instance().get(m_payee_id);
    if (payee)
    {
        mmEditPayeeDialog dlg(this, payee);
        dlg.ShowModal();
        refreshRequested_ = true;
        fillControls();
    }
}

void mmPayeeDialog::DeletePayee()
{
    std::list<int64> itemsSelected;
    FindSelectedPayees(itemsSelected);

    for(int64 p : itemsSelected) {
        const auto* payee = Model_Payee::instance().get(p);
        if (payee)
        {
            if (Model_Payee::instance().is_used(p))
            {
                wxString deletePayeeErrMsg = _("Payee in use.");
                deletePayeeErrMsg
                    << "\n"
                    << payee->PAYEENAME
                    << "\n"
                    << _("It will be not removed")
                    << "\n\n"
                    << _("Tip: Change all transactions using this Payee to another Payee"
                        " using the merge command:")
                    << "\n\n" << _u("Tools → Merge → Payees");
                wxMessageBox(deletePayeeErrMsg, _("Payee Manager: Delete Error"), wxOK | wxICON_ERROR);
                continue;
            }
            Model_Checking::Data_Set deletedTrans = Model_Checking::instance().find(Model_Checking::PAYEEID(p));
            wxMessageDialog msgDlg(this
                , _("Deleted transactions exist which use this payee.")
                    + "\n\n" + _("Deleting the payee will also automatically purge the associated deleted transactions.")
                    + "\n\n" + _("Do you want to continue?")
                , _("Confirm Payee Deletion")
                , wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);
            if (deletedTrans.empty() || msgDlg.ShowModal() == wxID_YES)
            {
                if (!deletedTrans.empty()) {
                    Model_Checking::instance().Savepoint();
                    Model_Splittransaction::instance().Savepoint();
                    Model_Attachment::instance().Savepoint();
                    Model_CustomFieldData::instance().Savepoint();
                    const wxString& RefType = Model_Attachment::REFTYPE_STR_TRANSACTION;

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

                Model_Payee::instance().remove(p);
                mmAttachmentManage::DeleteAllAttachments(Model_Attachment::REFTYPE_STR_PAYEE, p);
                m_payee_id = -1;
                refreshRequested_ = true;
                fillControls();
            }    
       }
    }
}

void mmPayeeDialog::DefineDefaultCategory()
{
    Model_Payee::Data *payee = Model_Payee::instance().get(m_payee_id);
    if (payee)
    {
        mmCategDialog dlg(this, true, payee->CATEGID);
        if (dlg.ShowModal() == wxID_OK)
        {
            payee->CATEGID = dlg.getCategId();
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
        refreshRequested_ = true;
        Model_Payee::instance().save(payee);
        mmWebApp::MMEX_WebApp_UpdatePayee();
        fillControls();
    }
}

void mmPayeeDialog::OnOrganizeAttachments()
{
    wxString RefType = Model_Attachment::REFTYPE_STR_PAYEE;

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
        msgStr << _("Merge payees completed") << "\n\n"
            << wxString::Format(_("Records have been updated in the database: %i")
                , dlg.updatedPayeesCount())
            << "\n\n";
        wxMessageBox(msgStr, _("Merge payees result"));
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
    mainMenu.Append(new wxMenuItem(&mainMenu, MENU_DEFINE_CATEGORY, _("Define &Category")));
    if (!payee) mainMenu.Enable(MENU_DEFINE_CATEGORY, false);
    mainMenu.Append(new wxMenuItem(&mainMenu, MENU_REMOVE_CATEGORY, _("Remove Ca&tegory")));
    if (!payee) mainMenu.Enable(MENU_REMOVE_CATEGORY, false);
    mainMenu.AppendSeparator();

    mainMenu.Append(new wxMenuItem(&mainMenu, MENU_NEW_PAYEE, _("&Add ")));
    mainMenu.Append(new wxMenuItem(&mainMenu, MENU_EDIT_PAYEE, _("&Edit ")));
    if (!payee) mainMenu.Enable(MENU_EDIT_PAYEE, false);
    mainMenu.Append(new wxMenuItem(&mainMenu, MENU_DELETE_PAYEE, _("&Remove ")));
    std::list<int64> selected;
    FindSelectedPayees(selected);
    if (!payee || selected.front() == -1) mainMenu.Enable(MENU_DELETE_PAYEE, false);
    mainMenu.AppendSeparator();

    mainMenu.Append(new wxMenuItem(&mainMenu, MENU_ORGANIZE_ATTACHMENTS, _("&Attachment Manager")));
    if (!payee) mainMenu.Enable(MENU_ORGANIZE_ATTACHMENTS, false);
    mainMenu.AppendSeparator();

    mainMenu.Append(new wxMenuItem(&mainMenu, MENU_RELOCATE_PAYEE, _("Merge &Payee")));
    //SetToolTip(_("Change all transactions using one Payee to another Payee"));
    if (!payee) mainMenu.Enable(MENU_RELOCATE_PAYEE, false);

    PopupMenu(&mainMenu);
    event.Skip();
}

void mmPayeeDialog::OnSort(wxListEvent& event)
{
    m_lastSort = m_sort;
    m_sort = event.GetColumn();
    if (m_sort == m_lastSort)
            m_sortReverse = !m_sortReverse;
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
