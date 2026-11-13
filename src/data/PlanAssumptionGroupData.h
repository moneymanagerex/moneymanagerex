/*******************************************************
 Copyright (C) 2026

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

// PlanAssumptionGroupData represents a record in table PLANASSUMPTIONGROUP_V1.
//
// A group holds several answers to the same question -- "what is MSFT worth?"
// might carry a conservative, a base and an optimistic figure -- and names
// which of them is currently active. Plan items reference the group rather than
// an individual value, so switching the active member in one place moves every
// dependent figure at once.
//
// The group is typed by m_kind and narrowed by m_scope_key (a ticker, say), so
// only assumptions that genuinely answer the same question can belong to it.
// m_unit says how a member value should be read: a currency symbol, a percent
// sign, or free text such as "shares".

#include "_DataEnum.h"
#include "table/PlanAssumptionGroupTable.h"

struct PlanAssumptionGroupData
{
    int64              m_id;
    wxString           m_name;
    PlanAssumptionKind m_kind;
    wxString           m_scope_key;      // e.g. "MSFT" for a share price
    wxString           m_unit;           // "$", "%", "shares", ...
    int64              m_active_assumption_id;
    wxString           m_notes;
    bool               m_active;

    explicit PlanAssumptionGroupData();
    explicit PlanAssumptionGroupData(wxSQLite3ResultSet& q);
    PlanAssumptionGroupData(const PlanAssumptionGroupData& other) = default;

    int64 id() const { return m_id; }
    void id(const int64 id) { m_id = id; }
    PlanAssumptionGroupRow to_row() const;
    PlanAssumptionGroupData& from_row(const PlanAssumptionGroupRow& row);
    void to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const;
    void to_update_stmt(wxSQLite3Statement& stmt) const;
    PlanAssumptionGroupData& from_select_result(wxSQLite3ResultSet& q);
    wxString to_json() const;
    void as_json(PrettyWriter<StringBuffer>& json_writer) const;
    row_t to_html_row() const;
    void to_html_template(html_template& t) const;
    void destroy() { delete this; }

    PlanAssumptionGroupData& clone_from(const PlanAssumptionGroupData& other);
    bool equals(const PlanAssumptionGroupData* other) const;
    bool operator< (const PlanAssumptionGroupData& other) const { return id() < other.id(); }
    bool operator< (const PlanAssumptionGroupData* other) const { return id() < other->id(); }

    bool is_scoped() const { return !m_scope_key.IsEmpty(); }

    // A group answers one question, so a member must agree on both the kind and
    // the thing being asked about.
    bool accepts(PlanAssumptionKind kind, const wxString& scope_key) const
    {
        if (m_kind.id() != kind.id())
            return false;
        if (m_scope_key.IsEmpty())
            return true;
        return m_scope_key.IsSameAs(scope_key, false);
    }

    // The unit to display when the group does not name one: a rate is a
    // percentage, everything else is left to the caller to decide.
    const wxString unit_or_default() const
    {
        if (!m_unit.IsEmpty())
            return m_unit;
        if (m_kind.id() == PlanAssumptionKind::e_tax_rate ||
            m_kind.id() == PlanAssumptionKind::e_inflation)
            return "%";
        return "";
    }

    struct SorterByGROUPID
    {
        bool operator()(const PlanAssumptionGroupData& x, const PlanAssumptionGroupData& y)
        {
            return x.m_id < y.m_id;
        }
    };

    struct SorterByGROUPNAME
    {
        bool operator()(const PlanAssumptionGroupData& x, const PlanAssumptionGroupData& y)
        {
            return x.m_name < y.m_name;
        }
    };

    struct SorterBySCOPEKEY
    {
        bool operator()(const PlanAssumptionGroupData& x, const PlanAssumptionGroupData& y)
        {
            return x.m_scope_key < y.m_scope_key;
        }
    };
};

inline PlanAssumptionGroupData::PlanAssumptionGroupData(wxSQLite3ResultSet& q) :
    PlanAssumptionGroupData()
{
    from_select_result(q);
}

inline void PlanAssumptionGroupData::to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const
{
    to_row().to_insert_stmt(stmt, id);
}

inline void PlanAssumptionGroupData::to_update_stmt(wxSQLite3Statement& stmt) const
{
    to_row().to_update_stmt(stmt);
}

inline PlanAssumptionGroupData& PlanAssumptionGroupData::from_select_result(wxSQLite3ResultSet& q)
{
    return from_row(PlanAssumptionGroupRow().from_select_result(q));
}

inline wxString PlanAssumptionGroupData::to_json() const
{
    return to_row().to_json();
}

inline void PlanAssumptionGroupData::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    to_row().as_json(json_writer);
}

inline row_t PlanAssumptionGroupData::to_html_row() const
{
    return to_row().to_html_row();
}

inline void PlanAssumptionGroupData::to_html_template(html_template& t) const
{
    to_row().to_html_template(t);
}

inline PlanAssumptionGroupData& PlanAssumptionGroupData::clone_from(const PlanAssumptionGroupData& other)
{
    *this = other;
    id(-1);
    return *this;
}
