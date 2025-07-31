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

#include "daterangeeditdialog.h"
#include "constants.h"
#include "images_list.h"


wxIMPLEMENT_DYNAMIC_CLASS(mmDateRangeEditDialog, wxDialog);

wxBEGIN_EVENT_TABLE(mmDateRangeEditDialog, wxDialog)
  EVT_BUTTON(wxID_OK, mmDateRangeEditDialog::OnOk)
  EVT_TEXT(TXT_CTRL_RANGE, mmDateRangeEditDialog::OnRange)
  EVT_CHOICE(CTRL_ANY, mmDateRangeEditDialog::OnUpdateRangeFromControls)
wxEND_EVENT_TABLE()


const wxArrayString sTokenPeriods = { "", "A", "Y", "Q", "M", "W", "T", "S"};
const char *sCountValues[] = {"-12", "-11", "-10", "-9", "-8", "-7", "-6", "-5", "-4", "-3", "-2", "-1",
                              "",
                              "+1", "+2", "+3", "+4", "+5", "+6", "+7", "+8", "9", "+10", "+11", "+12"};
const wxArrayString sCount = {25, sCountValues};


mmDateRangeEditDialog::mmDateRangeEditDialog()
{
}

mmDateRangeEditDialog::mmDateRangeEditDialog(wxWindow* parent, wxString* name_ptr, wxString* range_ptr)
{
    this->SetFont(parent->GetFont());
    Create(parent, -1, name_ptr->IsEmpty() ? _t("New date range") : _t("Edit date range"), wxDefaultPosition, wxSize(550, 250), wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX, "");
    m_name_ptr = name_ptr;
    m_range_ptr = range_ptr;
    CreateControls();
    SetIcon(mmex::getProgramIcon());
    Centre();
}

void mmDateRangeEditDialog::CreateControls()
{
    wxArrayString sPeriods;  // create dynamically to ensure proper translation
    sPeriods.Add("");
    sPeriods.Add(_("All"));
    sPeriods.Add(_("Year"));
    sPeriods.Add(_("Quarter"));
    sPeriods.Add(_("Month"));
    sPeriods.Add(_("Week"));
    sPeriods.Add(_("Today"));
    sPeriods.Add(_("Statement date"));

    wxBoxSizer* mainBoxSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(mainBoxSizer);
    m_defBColor = this->GetBackgroundColour();

    wxBoxSizer* textPanelSizer = new wxBoxSizer(wxHORIZONTAL);
    textPanelSizer->Add(new wxStaticText(this, wxID_STATIC, _("Name:")), g_flagsH);
    m_name_edit = new wxTextCtrl(this, TXT_CTRL_NAME, *m_name_ptr);
    textPanelSizer->AddSpacer(20);
    textPanelSizer->Add(m_name_edit, wxSizerFlags().Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL).Border(wxALL, 1).Proportion(1));
    mainBoxSizer->Add(textPanelSizer, g_flagsExpand);

    wxBoxSizer* ctrlPanelSizer = new wxBoxSizer(wxHORIZONTAL);
    ctrlPanelSizer->Add(new wxStaticText(this, wxID_STATIC, _("Range:")), g_flagsH);
    m_from_st = new wxStaticText(this, wxID_STATIC, _t("From"));
    ctrlPanelSizer->Add(m_from_st, g_flagsH);
    m_count1 = new wxChoice(this, CTRL_ANY, wxDefaultPosition, wxDefaultSize, sCount);
    ctrlPanelSizer->Add(m_count1, g_flagsH);
    m_period1 = new wxChoice(this, CTRL_ANY, wxDefaultPosition, wxDefaultSize, sPeriods);
    ctrlPanelSizer->Add(m_period1, g_flagsH);
    m_to_st = new wxStaticText(this, wxID_STATIC, _t(" to "));
    ctrlPanelSizer->Add(m_to_st, g_flagsH);
    m_count2 = new wxChoice(this, CTRL_ANY, wxDefaultPosition, wxDefaultSize, sCount);
    ctrlPanelSizer->Add(m_count2, g_flagsH);
    m_period2 = new wxChoice(this, CTRL_ANY, wxDefaultPosition, wxDefaultSize, sPeriods);
    ctrlPanelSizer->Add(m_period2, g_flagsH);
    mainBoxSizer->Add(ctrlPanelSizer, g_flagsV);

    wxBoxSizer* rangePanelSizer = new wxBoxSizer(wxHORIZONTAL);
    rangePanelSizer->Add(new wxStaticText(this, wxID_STATIC, _("Range code:")), g_flagsH);
    m_range_edit = new wxTextCtrl(this, TXT_CTRL_RANGE, *m_range_ptr);
    rangePanelSizer->Add(m_range_edit, wxSizerFlags().Align(wxALIGN_LEFT | wxALIGN_CENTRE_VERTICAL).Border(wxALL, 1).Proportion(1));
    mainBoxSizer->Add(rangePanelSizer, g_flagsExpand);

    m_status = new wxStaticText(this, wxID_STATIC, "");
    mainBoxSizer->Add(m_status, wxSizerFlags().Align(wxALIGN_CENTER).Border(wxALL, 5));

    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    btnSizer->Add(new wxButton(this, wxID_OK, _("Save")), 0, wxALL, 5);
    btnSizer->Add(new wxButton(this, wxID_CANCEL, _("Cancel")), 0, wxALL, 5);
    mainBoxSizer->Add(btnSizer, g_flagsCenter);

    updateControlsFromRange(*m_range_ptr);
    if (!m_range_ptr->IsEmpty()) {
        checkRange();
    }
    this->Layout();
}

void mmDateRangeEditDialog::OnRange(wxCommandEvent&)
{
    updateControlsFromRange(m_range_edit->GetValue());
    checkRange();
}

bool mmDateRangeEditDialog::checkRange() {
    bool rangeOk = false;
    DateRange2 rdata = DateRange2();
    if (rdata.parseSpec(m_range_edit->GetValue())) {
        if (!rdata.checking_end().IsValid() || rdata.checking_start() <= rdata.checking_end()) {
          m_status->SetLabelText(_t("Range is ok:") + wxString::Format(" >%s - %s<", rdata.checking_start_str(), rdata.checking_end_str()));
          m_status->SetBackgroundColour(m_defBColor);
          rangeOk = true;
        }
    }

    if (!rangeOk) {
        m_status->SetLabelText(_t("Invalid date range definition"));
        m_status->SetBackgroundColour(*wxRED);
    }
    return rangeOk;
}

void mmDateRangeEditDialog::OnOk(wxCommandEvent&)
{
    wxString msg = "";
    bool saveIt = true;
    if (m_name_edit->GetValue().IsEmpty()) {
      msg << _t("Name must not be empty") << ("\n");
      saveIt = false;
    }
    if (!checkRange()) {
      msg << _t("Invalid date range definition");
      saveIt = false;
    }

    if(saveIt) {
        *m_name_ptr = m_name_edit->GetValue();
        *m_range_ptr = m_range_edit->GetValue();
        EndModal(wxID_OK);
    }
    else {
      wxMessageBox(msg, _t("Definition error"), wxOK | wxICON_ERROR);
    }
}

void mmDateRangeEditDialog::OnUpdateRangeFromControls(wxCommandEvent&)
{
    m_count1->Enable(m_period1->GetSelection() > 1);  // Disable for empty and all values
    m_count2->Enable(m_period2->GetSelection() > 1);

    wxString result = "";
    result << (m_count1->IsEnabled() ? m_count1->GetStringSelection() + " ": "") << sTokenPeriods[m_period1->GetSelection()];
    if (m_period2->GetSelection() > 0) {
        result << " .. " << (m_count2->IsEnabled() ? m_count2->GetStringSelection() + " " : "") << sTokenPeriods[m_period2->GetSelection()];
    }
    m_range_edit->ChangeValue(result);

    checkRange();
}

void mmDateRangeEditDialog::updateControlsFromRange(wxString range)
{
    bool valid = (range.Find(",") == wxNOT_FOUND);
    if (valid) {
        // Reset Ctrls:
        m_count1->SetSelection(12);
        m_period1->SetSelection(0);
        m_count2->SetSelection(12);
        m_period2->SetSelection(0);

        m_to_st->Show(false);
        m_count1->Enable(false);
        m_count2->Enable(false);

        range.Replace("..", " .. ");
        range.Replace("+", "");

        bool firstselection = true;
        wxStringTokenizer tokenizer(range, " ");
        while (tokenizer.HasMoreTokens())
        {
            wxString token = tokenizer.GetNextToken();
            int idx;
            if (token.ToInt(&idx)) {
                if (idx > -13 && idx < 13) {
                    if (firstselection) {
                        m_count1->SetSelection(idx + 12);
                        m_count1->Enable(idx != 0);
                    }
                    else {
                        m_count2->SetSelection(idx + 12);
                    }
                }
                else {
                    valid = false;
                }
            }
            else {
                idx = sTokenPeriods.Index(token);
                if (idx > wxNOT_FOUND) {
                    if (firstselection) {
                        m_period1->SetSelection(idx);
                    }
                    else {
                        m_period2->SetSelection(idx);
                    }
                }
                else {
                    if (token == "..") {
                        m_to_st->Show(true);
                        m_count2->Enable(true);
                        firstselection = false;
                    }
                    else {
                        valid = false;
                    }
                }
            }
        }
    }
    showSelectControls(valid);
}

void mmDateRangeEditDialog::showSelectControls(bool status)
{
    m_from_st->Show(status);
    m_count1->Show(status);
    m_period1->Show(status);
    m_to_st->Show(status);
    m_count2->Show(status);
    m_period2->Show(status);
}
