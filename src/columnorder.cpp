#include "columnorder.h"
#include "constants.h"
#include "model/Model_Setting.h"
#include "model/Model_Infotable.h"
#include "paths.h"
#include "option.h"
#include "util.h"

wxIMPLEMENT_DYNAMIC_CLASS(mmColumnsDialog, wxDialog);

wxBEGIN_EVENT_TABLE(mmColumnsDialog, wxDialog)
    EVT_BUTTON(wxID_OK, mmColumnsDialog::OnOk)
    EVT_BUTTON(wxID_CANCEL, mmColumnsDialog::OnCancel)
    EVT_BUTTON(wxID_UP, mmColumnsDialog::OnUp)
    EVT_BUTTON(wxID_DOWN, mmColumnsDialog::OnDown)
wxEND_EVENT_TABLE()

mmColumnsDialog::mmColumnsDialog() {}

mmColumnsDialog::mmColumnsDialog(wxWindow *parent) : wxDialog(parent, wxID_ANY, _("Column Order"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
    Create(parent);
    SetMinSize(wxSize(400, 300));
    SetTitle(_("Column Order"));
}

mmColumnsDialog::~mmColumnsDialog()
{
    Model_Infotable::instance().Set("COLUMNORDER_DIALOG_SIZE", GetSize());
    // TODO: Save the column order
}


bool mmColumnsDialog::Create(wxWindow *parent)
{
    SetEvtHandlerEnabled(false);
    CreateControls();
    SetEvtHandlerEnabled(true);

    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    this->SetInitialSize();
    SetIcon(mmex::getProgramIcon());

    Fit();
    mmSetSize(this);
    Centre();
    return true;
}

/*
  Concept:
  - The column order is stored as a "|" delimited list of header strings in the settings.
  - A dialog allows reordering the columns. This is saved in the settings.
  - Constructing a table (in mmcheckingpanel.cpp) will update this list with columns not yet specified.
    So eventually all existing columns will be in this list (the user needs to have viewed all columns at least once).
    This is important since there may be user defined columns.
  - TODO: Users may remove nonexisting columns from the list in the column order dialog.
*/


// Set new column order. Called when closing the dialog using the "OK" button
void mmColumnsDialog::SetColumnsOrder()
{
    wxLogDebug("SetColumnsOrder: %s", wxJoin(columnList_, ','));
    Model_Setting::instance().Savepoint();
    Model_Setting::instance().Set("COLUMNSORDER", wxJoin(columnList_, '|'));
    Model_Setting::instance().ReleaseSavepoint();
}


void mmColumnsDialog::GetColumnsOrder()
{
    columnList_ = wxSplit(Model_Setting::instance().GetStringSetting("COLUMNSORDER", ""), '|');
    wxLogDebug("GetColumnsOrder: %s", wxJoin(columnList_, ','));
}

// Get the column order from the settings.
// Append new columns from defaultColumns if the list differs. Then save.
wxArrayString mmColumnsDialog::updateColumnsOrder(wxArrayString defaultColumns)
{
    GetColumnsOrder();
    for (const auto& column : defaultColumns) {
        if (columnList_.Index(column) == wxNOT_FOUND) {
            columnList_.Add(column);
        }
    }
    SetColumnsOrder();
    return columnList_;
}

void mmColumnsDialog::OnUp(wxCommandEvent& event) {
    Move(-1);
}

void mmColumnsDialog::OnDown(wxCommandEvent& event) {
    Move(1);
}

void mmColumnsDialog::CreateControls()
{
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    GetColumnsOrder();
    m_listBox = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, columnList_, wxLB_SINGLE | wxLB_NEEDED_SB);

    // Buttons for moving items up and down
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxVERTICAL);
    wxButton* upButton = new wxButton(this, wxID_ANY, "Up");
    wxButton* downButton = new wxButton(this, wxID_ANY, "Down");

    buttonSizer->Add(upButton, 0, wxALL, 5);
    buttonSizer->Add(downButton, 0, wxALL, 5);

    // Horizontal sizer for listbox and up/down buttons
    wxBoxSizer* listSizer = new wxBoxSizer(wxHORIZONTAL);
    listSizer->Add(m_listBox, 1, wxEXPAND | wxALL, 5);
    listSizer->Add(buttonSizer, 0, wxALIGN_CENTER_VERTICAL);

    // OK and Cancel buttons
    wxSizer* okCancelSizer = CreateButtonSizer(wxOK | wxCANCEL);

    // Add components to main sizer
    mainSizer->Add(listSizer, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(okCancelSizer, 0, wxALIGN_CENTER | wxALL, 5);

    SetSizer(mainSizer);

    // Event bindings
    upButton->Bind(wxEVT_BUTTON, &mmColumnsDialog::OnUp, this);
    downButton->Bind(wxEVT_BUTTON, &mmColumnsDialog::OnDown, this);
}

void mmColumnsDialog::OnOk(wxCommandEvent& event)
{
    SetColumnsOrder();
    EndModal(wxID_OK);
}   

void mmColumnsDialog::OnCancel(wxCommandEvent& event)
{
    EndModal(wxID_CANCEL);
}

void mmColumnsDialog::Move(int direction) {
    //print debug message to stderr 
    wxLogDebug("Move %d", direction);

    wxArrayInt selections;
    int count = m_listBox->GetSelections(selections);

    if (count == 0) return;

    if ((direction == -1 && selections[0] > 0) ||
        (direction == 1 && selections[count - 1] < m_listBox->GetCount() - 1)) {
        for (int i = (direction == 1 ? count - 1 : 0);
             (direction == 1 ? i >= 0 : i < count);
             i += (direction == 1 ? -1 : 1)) {

            int pos = selections[i];
            wxString item = m_listBox->GetString(pos);
            m_listBox->Delete(pos);
            m_listBox->Insert(item, pos + direction);
            m_listBox->SetSelection(pos + direction, true);
        }
    }

    // Write new order into columnList_
    columnList_.Clear();
    for (int i = 0; i < m_listBox->GetCount(); i++) {
        columnList_.Add(m_listBox->GetString(i));
    }
}


// Other member function implementations go here...