/*******************************************************
 Copyright (C) 2026 George Ef (george.a.ef@gmail.com)

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

#include "base/types.h"
#include "util/mmChoice.h"

struct AccountStatus
{
public:
    enum
    {
        e_open = 0,
        e_closed,
        size
    };
    static mmChoiceNameA s_choice_a;

private:
    mmChoiceId m_id;

public:
    AccountStatus(mmChoiceId id = s_choice_a.default_id_n()) :
        m_id(s_choice_a.valid_id_n(id)) {}
    AccountStatus(const wxString& name) :
        m_id(AccountStatus::s_choice_a.find_name_n(name)) {}

    mmChoiceId id() const { return m_id; }
    const wxString name() const { return AccountStatus::s_choice_a.get_name(m_id); }
};

struct AccountFavorite
{
public:
    enum
    {
        e_false = 0,
        e_true,
        size
    };
    static mmChoiceNameA s_choice_a;

private:
    mmChoiceId m_id;

public:
    AccountFavorite(mmChoiceId id = s_choice_a.default_id_n()) :
        m_id(s_choice_a.valid_id_n(id)) {}
    AccountFavorite(const wxString& name) :
        m_id(AccountFavorite::s_choice_a.find_name_n(name)) {}
    AccountFavorite(bool is_favorite) :
        m_id(is_favorite ? e_true : e_false) {}

    mmChoiceId id() const { return m_id; }
    const wxString name() const { return AccountFavorite::s_choice_a.get_name(m_id); }
};

struct AssetType
{
public:
    enum
    {
        e_property = 0,
        e_automobile,
        e_household,
        e_art,
        e_jewellery,
        e_cash,
        e_other,
        size
    };
    static mmChoiceNameA s_choice_a;

private:
    mmChoiceId m_id;

public:
    AssetType(mmChoiceId id = s_choice_a.default_id_n()) :
        m_id(s_choice_a.valid_id_n(id)) {}
    AssetType(const wxString& name) :
        m_id(AssetType::s_choice_a.find_name_n(name)) {}

    mmChoiceId id() const { return m_id; }
    const wxString name() const { return AssetType::s_choice_a.get_name(m_id); }
};

struct AssetStatus
{
public:
    enum
    {
        e_closed = 0,
        e_open,
        size
    };
    static mmChoiceNameA s_choice_a;

private:
    mmChoiceId m_id;

public:
    AssetStatus(mmChoiceId id = s_choice_a.default_id_n()) :
        m_id(s_choice_a.valid_id_n(id)) {}
    AssetStatus(const wxString& name) :
        m_id(AssetStatus::s_choice_a.find_name_n(name)) {}

    mmChoiceId id() const { return m_id; }
    const wxString name() const { return AssetStatus::s_choice_a.get_name(m_id); }
};

struct AssetChange
{
public:
    enum
    {
        e_none = 0,
        e_appreciates,
        e_depreciates,
        size
    };
    static mmChoiceNameA s_choice_a;

private:
    mmChoiceId m_id;

public:
    AssetChange(mmChoiceId id = s_choice_a.default_id_n()) :
        m_id(s_choice_a.valid_id_n(id)) {}
    AssetChange(const wxString& name) :
        m_id(AssetChange::s_choice_a.find_name_n(name)) {}

    mmChoiceId id() const { return m_id; }
    const wxString name() const { return AssetChange::s_choice_a.get_name(m_id); }
};

struct AssetChangeMode
{
public:
    enum
    {
        e_percentage = 0,
        e_linear,
        size
    };
    static mmChoiceNameA s_choice_a;

private:
    mmChoiceId m_id;

public:
    AssetChangeMode(mmChoiceId id = s_choice_a.default_id_n()) :
        m_id(s_choice_a.valid_id_n(id)) {}
    AssetChangeMode(const wxString& name) :
        m_id(AssetChangeMode::s_choice_a.find_name_n(name)) {}

    mmChoiceId id() const { return m_id; }
    const wxString name() const { return AssetChangeMode::s_choice_a.get_name(m_id); }
};

struct BudgetFrequency
{
public:
    enum
    {
        e_none = 0,
        e_weekly,
        e_biweekly,
        e_monthly,
        e_bimonthly,
        e_quarterly,
        e_halfyearly,
        e_yearly,
        e_daily,
        size
    };
    static mmChoiceNameA s_choice_a;

private:
    mmChoiceId m_id;

public:
    BudgetFrequency(mmChoiceId id = s_choice_a.default_id_n()) :
        m_id(s_choice_a.valid_id_n(id)) {}
    BudgetFrequency(const wxString& name) :
        m_id(BudgetFrequency::s_choice_a.find_name_n(name)) {}

    mmChoiceId id() const { return m_id; }
    const wxString name() const { return BudgetFrequency::s_choice_a.get_name(m_id); }
    int times_per_year() const {
        int a[size] = { 0, 52, 26, 12, 6, 4, 2, 1, 365 };
        return a[m_id];
    }
    double times_per_month() const {
        return double(times_per_year()) / 12.0;
    }
};

struct CurrencyType
{
public:
    enum
    {
        e_fiat = 0,
        e_crypto,
        size
    };
    static mmChoiceNameA s_choice_a;

private:
    mmChoiceId m_id;

public:
    CurrencyType(mmChoiceId id = s_choice_a.default_id_n()) :
        m_id(s_choice_a.valid_id_n(id)) {}
    CurrencyType(const wxString& name) :
        m_id(CurrencyType::s_choice_a.find_name_n(name)) {}

    mmChoiceId id() const { return m_id; }
    const wxString name() const { return CurrencyType::s_choice_a.get_name(m_id); }
};

struct FieldTypeN
{
public:
    enum
    {
        e_string = 0,
        e_integer,
        e_decimal,
        e_boolean,
        e_date,
        e_time,
        e_single_choice,
        e_multi_choice,
        size
    };
    static mmChoiceNameA s_choice_a;

private:
    mmChoiceIdN m_id_n;

public:
    FieldTypeN(mmChoiceIdN id_n = s_choice_a.default_id_n()) :
        m_id_n(s_choice_a.valid_id_n(id_n)) {}
    FieldTypeN(const wxString& name) :
        m_id_n(FieldTypeN::s_choice_a.find_name_n(name)) {}

    bool has_value() const { return m_id_n >= 0; }
    mmChoiceIdN id_n() const { return m_id_n; }
    const wxString name_n() const {
        return has_value() ? FieldTypeN::s_choice_a.get_name(m_id_n) : "";
    }
};

struct RefTypeN
{
public:
    enum
    {
        e_trx = 0,
        e_stock,
        e_asset,
        e_account,
        e_sched,
        e_payee,
        e_trx_split,
        e_sched_split,
        size
    };
    static mmChoiceNameA s_choice_a;

private:
    mmChoiceIdN m_id_n;

public:
    RefTypeN(mmChoiceIdN id_n = s_choice_a.default_id_n()) :
        m_id_n(s_choice_a.valid_id_n(id_n)) {}
    RefTypeN(const wxString& name) :
        m_id_n(RefTypeN::s_choice_a.find_name_n(name)) {}

    static mmChoiceIdN field_id_n(mmChoiceIdN id_n) {
        switch (id_n) {
        case e_sched:
        //case e_trx_split:
        //case e_sched_split:
            return e_trx;
        default:
            return id_n;
        }
    }
    static RefTypeN field_ref_type_n(RefTypeN ref_type_n) {
        return RefTypeN(RefTypeN::field_id_n(ref_type_n.m_id_n));
    }

    bool has_value() const { return m_id_n >= 0; }
    mmChoiceIdN id_n() const { return m_id_n; }
    const wxString name_n() const {
        return has_value() ? RefTypeN::s_choice_a.get_name(m_id_n) : "";
    }
    bool operator== (const RefTypeN& other) const {
        return id_n() == other.id_n();
    }
    bool operator< (const RefTypeN& other) const {
        return id_n() < other.id_n();
    }
};

struct UpdateType
{
public:
    enum
    {
        e_none = 0,
        e_online,
        e_manual,
        size
    };
    static mmChoiceCodeNameA s_choice_a;

private:
    mmChoiceId m_id;

public:
    UpdateType(mmChoiceId id = s_choice_a.default_id_n()) :
        m_id(s_choice_a.valid_id_n(id)) {}
    static UpdateType from_code(int code) {
        return UpdateType(UpdateType::s_choice_a.find_code_n(code));
    }

    mmChoiceId id() const { return m_id; }
    int code() const { return UpdateType::s_choice_a.get_code(m_id); }
    const wxString name() const { return UpdateType::s_choice_a.get_name(m_id); }
};

