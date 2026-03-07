/*******************************************************
Copyright (C) 2017 Gabriele-V
Copyright (C) 2018, 2021, 2022 Nikolay Akimov
Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)

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

#pragma once

#include "base/defs.h"
#include "model/FieldModel.h"
#include "model/TrxModel.h"

class mmDatePickerCtrl;
class mmTextCtrl;
class wxDialog;

class FieldValueDialog : public wxDialog
{
private:
    const int FIELDMULTIPLIER = 4;
    const int CONTROLOFFSET = FIELDMULTIPLIER - 1;

    const RefTypeN m_ref_type;
    int64 m_ref_id = -1;
    FieldModel::DataA m_field_a;
    std::map<wxWindowID, wxString> m_data_changed;

    wxDialog* m_dialog = nullptr;
    wxStaticBox* m_static_box = nullptr;
    wxWindowID m_init_control_id = wxID_ANY;

public:
    FieldValueDialog();
    FieldValueDialog(wxDialog* dialog, RefTypeN ref_type, int64 ref_id);
    ~FieldValueDialog();

    bool FillCustomFields(wxBoxSizer* box_sizer);
    bool SaveCustomValues(RefTypeN ref_type, int64 ref_id);
    void UpdateCustomValues(RefTypeN ref_type, int64 ref_id);
    void SetStringValue(int fieldIndex, const wxString& value, bool hasChanged = false);
    bool ValidateCustomValues(int64);
    const wxString GetWidgetData(wxWindowID controlID) const;
    void SetWidgetData(wxWindowID controlID, const wxString& value);
    int GetWidgetType(wxWindowID controlID) const;
    int GetPrecision(wxWindowID controlID) const;
    size_t GetCustomFieldsCount() const;
    size_t GetActiveCustomFieldsCount() const;
    std::map<int64, wxString> GetActiveCustomFields() const;
    void SetBaseID(wxWindowID id);
    wxWindowID GetBaseID() const;
    void ResetRefID();
    bool IsSomeWidgetChanged() const;
    bool IsDataFound(const TrxModel::Full_Data &tran);
    void ResetWidgetsChanged();
    void ClearSettings();
    bool IsCustomPanelShown() const;
    void ShowHideCustomPanel() const;
    void ShowCustomPanel() const;

private:
    void OnStringChanged(wxCommandEvent& event);
    void OnDateChanged(wxDateEvent& event);
    void OnTimeChanged(wxDateEvent& event);
    void OnMultiChoice(wxCommandEvent& event);
    void OnSingleChoice(wxCommandEvent& event);
    void OnRadioButtonChanged(wxCommandEvent& event);
    void OnCheckBoxActivated(wxCommandEvent& event);
    bool IsWidgetChanged(wxWindowID id);
    void SetWidgetChanged(wxWindowID id, const wxString& data);
    void ResetWidgetChanged(wxWindowID id);
};

class mmCustomDataTransaction : public FieldValueDialog
{
public:
    mmCustomDataTransaction(
        wxDialog* dialog, RefTypeN ref_type, int64 ref_id, wxWindowID base_id
    );
};

inline void       FieldValueDialog::ResetRefID() { m_ref_id = -1; }
inline void       FieldValueDialog::SetBaseID(wxWindowID id) { m_init_control_id = id; }
inline size_t     FieldValueDialog::GetCustomFieldsCount() const { return m_field_a.size(); }
inline wxWindowID FieldValueDialog::GetBaseID() const { return m_init_control_id; }

