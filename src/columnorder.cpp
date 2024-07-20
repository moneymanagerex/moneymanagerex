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

void mmColumnsDialog::CreateControls()
{
    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(boxSizer);

    // Get the Colum names. Use dummy array for now.
    wxArrayString columnList = { "Column 1", "Column 2", "Column 3", "Column 4", "Column 5" };

    // Put colum names into the list box
    m_listBox = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, columnList, wxLB_SINGLE | wxLB_NEEDED_SB);
    boxSizer->Add(m_listBox, 1, wxEXPAND | wxALL, 5);

    wxPanel* buttonsPanel = new wxPanel(this, wxID_ANY);
    boxSizer->Add(buttonsPanel, wxSizerFlags(g_flagsV).Center());
    wxBoxSizer* buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonsPanel->SetSizer(buttonsSizer);

    m_upButton = new wxButton(this, wxID_UP, _("Up"));
    m_downButton = new wxButton(this, wxID_DOWN, _("Down"));
    m_OkButton = new wxButton(this, wxID_OK, _("OK"));
    m_CancelButton = new wxButton(this, wxID_CANCEL, _("Cancel"));

    // arrange the buttons inside the buttonsPanel.
    buttonsSizer->Add(m_upButton, 0, wxALL, 5);
    buttonsSizer->Add(m_downButton, 0, wxALL, 5);
    buttonsSizer->AddStretchSpacer();
    buttonsSizer->Add(m_OkButton, 0, wxALL, 5);
    buttonsSizer->Add(m_CancelButton, 0, wxALL, 5);

}

void mmColumnsDialog::OnOk(wxCommandEvent& event)
{
    // Save the column order
    // Close the dialog
    EndModal(wxID_OK);
}   

void mmColumnsDialog::OnCancel(wxCommandEvent& event)
{
    // Close the dialog
    EndModal(wxID_CANCEL);
}

void mmColumnsDialog::OnUp(wxCommandEvent& event)
{
    // Move the selected item up
}

void mmColumnsDialog::OnDown(wxCommandEvent& event)
{
    // Move the selected item down
}   

// Other member function implementations go here...