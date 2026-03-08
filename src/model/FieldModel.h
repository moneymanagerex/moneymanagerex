/*******************************************************
 Copyright (C) 2016 Gabriele-V

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
#include "util/mmChoice.h"

#include "table/FieldTable.h"
#include "data/FieldData.h"

#include "_ModelBase.h"

class FieldModel : public TableFactory<FieldTable, FieldData>
{
public:
    FieldModel();
    ~FieldModel();

public:
    static FieldModel& instance(wxSQLite3Database* db);
    static FieldModel& instance();

    static auto UDFC_FIELDS() -> const wxArrayString;

    // TODO: create struct FieldProperties; move to FieldProperties
    static auto getUDFC(const wxString& properties) -> const wxString;
    static auto getRegEx(const wxString& properties) -> const wxString;
    static auto getTooltip(const wxString& properties) -> const wxString;
    static auto getDefault(const wxString& properties) -> const wxString;
    static int  getDigitScale(const wxString& Properties);
    static bool getAutocomplete(const wxString& properties);
    static auto getChoices(const wxString& properties) -> const wxArrayString;
    static auto formatProperties(
        const wxString& Tooltip, const wxString& RegEx,
        bool Autocomplete, const wxString& Default, const wxArrayString& Choices,
        const int DigitScale, const wxString& udfc_str
    ) -> const wxString;

public:
    bool purge_id(int64 field_id) override;
    auto find_id_value_a(const int64 FieldID) -> wxArrayString;
    auto get_udfc_data_n(RefTypeN ref_type, const wxString& udfc) -> const Data*;
    auto get_udfc_id_n(RefTypeN ref_type, const wxString& udfc) -> int64;
    auto get_udfc_name_n(RefTypeN ref_type, const wxString& udfc) -> const wxString;
    auto get_udfc_type_n(RefTypeN ref_type, const wxString& udfc) -> FieldTypeN;
    auto get_udfc_properties_n(RefTypeN ref_type, const wxString& udfc) -> const wxString;
    auto get_all_ucfd_id_m(RefTypeN ref_type) -> const std::map<wxString, int64>;
    auto get_data_udfc_a(const Data* field_n) -> const wxArrayString;
};
