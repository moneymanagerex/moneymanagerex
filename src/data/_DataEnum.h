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

#include "base/_types.h"
#include "base/mmChoice.h"

struct AccountStatus
{
public:
    enum
    {
        e_open = 0,
        e_closed,
        size
    };
    static mmChoiceKeyNameA s_choice_a;

private:
    mmChoiceId m_id;

public:
    AccountStatus(mmChoiceId id = s_choice_a.default_id_n()) :
        m_id(s_choice_a.valid_id_n(id)) {}
    AccountStatus(const wxString& key) :
        m_id(AccountStatus::s_choice_a.find_key_n(key)) {}

    mmChoiceId id() const { return m_id; }
    const wxString key() const { return AccountStatus::s_choice_a.get_key(m_id); }
    const wxString name() const { return AccountStatus::s_choice_a.get_name(m_id); }

    bool operator== (const AccountStatus& other) const {
        return id() == other.id();
    }
    bool operator!= (const AccountStatus& other) const {
        return id() != other.id();
    }
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
    static mmChoiceKeyA s_choice_a;

private:
    mmChoiceId m_id;

public:
    AccountFavorite(mmChoiceId id = s_choice_a.default_id_n()) :
        m_id(s_choice_a.valid_id_n(id)) {}
    AccountFavorite(const wxString& key) :
        m_id(AccountFavorite::s_choice_a.find_key_n(key)) {}
    AccountFavorite(bool is_favorite) :
        m_id(is_favorite ? e_true : e_false) {}

    mmChoiceId id() const { return m_id; }
    const wxString key() const { return AccountFavorite::s_choice_a.get_key(m_id); }

    bool operator== (const AccountFavorite& other) const {
        return id() == other.id();
    }
    bool operator!= (const AccountFavorite& other) const {
        return id() != other.id();
    }
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
    static mmChoiceKeyNameA s_choice_a;

private:
    mmChoiceId m_id;

public:
    AssetType(mmChoiceId id = s_choice_a.default_id_n()) :
        m_id(s_choice_a.valid_id_n(id)) {}
    AssetType(const wxString& key) :
        m_id(AssetType::s_choice_a.find_key_n(key)) {}

    mmChoiceId id() const { return m_id; }
    const wxString key() const { return AssetType::s_choice_a.get_key(m_id); }
    const wxString name() const { return AssetType::s_choice_a.get_name(m_id); }

    bool operator== (const AssetType& other) const {
        return id() == other.id();
    }
    bool operator!= (const AssetType& other) const {
        return id() != other.id();
    }
};

struct AssetStatus
{
public:
    enum
    {
        e_open = 0,
        e_closed,
        size
    };
    static mmChoiceKeyNameA s_choice_a;

private:
    mmChoiceId m_id;

public:
    AssetStatus(mmChoiceId id = s_choice_a.default_id_n()) :
        m_id(s_choice_a.valid_id_n(id)) {}
    AssetStatus(const wxString& key) :
        m_id(AssetStatus::s_choice_a.find_key_n(key)) {}

    mmChoiceId id() const { return m_id; }
    const wxString key() const { return AssetStatus::s_choice_a.get_key(m_id); }
    const wxString name() const { return AssetStatus::s_choice_a.get_name(m_id); }

    bool operator== (const AssetStatus& other) const {
        return id() == other.id();
    }
    bool operator!= (const AssetStatus& other) const {
        return id() != other.id();
    }
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
    static mmChoiceKeyNameA s_choice_a;

private:
    mmChoiceId m_id;

public:
    AssetChange(mmChoiceId id = s_choice_a.default_id_n()) :
        m_id(s_choice_a.valid_id_n(id)) {}
    AssetChange(const wxString& key) :
        m_id(AssetChange::s_choice_a.find_key_n(key)) {}

    mmChoiceId id() const { return m_id; }
    const wxString key() const { return AssetChange::s_choice_a.get_key(m_id); }
    const wxString name() const { return AssetChange::s_choice_a.get_name(m_id); }

    bool operator== (const AssetChange& other) const {
        return id() == other.id();
    }
    bool operator!= (const AssetChange& other) const {
        return id() != other.id();
    }
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
    static mmChoiceKeyNameA s_choice_a;

private:
    mmChoiceId m_id;

public:
    AssetChangeMode(mmChoiceId id = s_choice_a.default_id_n()) :
        m_id(s_choice_a.valid_id_n(id)) {}
    AssetChangeMode(const wxString& key) :
        m_id(AssetChangeMode::s_choice_a.find_key_n(key)) {}

    mmChoiceId id() const { return m_id; }
    const wxString key() const { return AssetChangeMode::s_choice_a.get_key(m_id); }
    const wxString name() const { return AssetChangeMode::s_choice_a.get_name(m_id); }

    bool operator== (const AssetChangeMode& other) const {
        return id() == other.id();
    }
    bool operator!= (const AssetChangeMode& other) const {
        return id() != other.id();
    }
};

struct BudgetFreq
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
    static mmChoiceKeyNameA s_choice_a;

private:
    mmChoiceId m_id;

public:
    BudgetFreq(mmChoiceId id = s_choice_a.default_id_n()) :
        m_id(s_choice_a.valid_id_n(id)) {}
    BudgetFreq(const wxString& key) :
        m_id(BudgetFreq::s_choice_a.find_key_n(key)) {}

    mmChoiceId id() const { return m_id; }
    const wxString key() const { return BudgetFreq::s_choice_a.get_key(m_id); }
    const wxString name() const { return BudgetFreq::s_choice_a.get_name(m_id); }
    int times_per_year() const {
        int a[size] = { 0, 52, 26, 12, 6, 4, 2, 1, 365 };
        return a[m_id];
    }
    double times_per_month() const {
        return double(times_per_year()) / 12.0;
    }

    bool operator== (const BudgetFreq& other) const {
        return id() == other.id();
    }
    bool operator!= (const BudgetFreq& other) const {
        return id() != other.id();
    }
};

struct BudgetAmountType
{
public:
    enum
    {
        e_fixed = 0,
        e_estimated,
        e_auto,
        size
    };
    static mmChoiceKeyNameA s_choice_a;

private:
    mmChoiceId m_id;

public:
    BudgetAmountType(mmChoiceId id = s_choice_a.default_id_n()) :
        m_id(s_choice_a.valid_id_n(id)) {}
    BudgetAmountType(const wxString& key) :
        m_id(BudgetAmountType::s_choice_a.find_key_n(key)) {}

    mmChoiceId id() const { return m_id; }
    const wxString key() const { return BudgetAmountType::s_choice_a.get_key(m_id); }
    const wxString name() const { return BudgetAmountType::s_choice_a.get_name(m_id); }

    // an amount that the planner computes instead of the user typing it
    bool is_derived() const { return m_id == e_auto; }
    // an amount that is known to be a guess (rendered with a '~' marker)
    bool is_uncertain() const { return m_id == e_estimated || m_id == e_auto; }

    bool operator== (const BudgetAmountType& other) const {
        return id() == other.id();
    }
    bool operator!= (const BudgetAmountType& other) const {
        return id() != other.id();
    }
};

// Whether a budget entry starts each period afresh, or carries what was left
// over. A month can be net positive while its first half is deeply negative
// because bills cluster around due dates, so an envelope that accumulates is
// often closer to how the money actually behaves than one that resets.
struct BudgetRollover
{
public:
    enum
    {
        e_none = 0,
        e_surplus,
        e_deficit,
        e_both,
        size
    };
    static mmChoiceKeyNameA s_choice_a;

private:
    mmChoiceId m_id;

public:
    BudgetRollover(mmChoiceId id = s_choice_a.default_id_n()) :
        m_id(s_choice_a.valid_id_n(id)) {}
    BudgetRollover(const wxString& key) :
        m_id(BudgetRollover::s_choice_a.find_key_n(key)) {}

    mmChoiceId id() const { return m_id; }
    const wxString key() const { return BudgetRollover::s_choice_a.get_key(m_id); }
    const wxString name() const { return BudgetRollover::s_choice_a.get_name(m_id); }

    bool carries() const { return m_id != e_none; }
    // An unspent amount is kept for later.
    bool carries_surplus() const { return m_id == e_surplus || m_id == e_both; }
    // An overspend is owed out of a later period rather than forgiven.
    bool carries_deficit() const { return m_id == e_deficit || m_id == e_both; }

    bool operator== (const BudgetRollover& other) const {
        return id() == other.id();
    }
    bool operator!= (const BudgetRollover& other) const {
        return id() != other.id();
    }
};

struct PlanAssumptionKind
{
public:
    enum
    {
        e_share_price = 0,
        e_tax_rate,
        e_inflation,
        e_exchange_rate,
        e_generic,
        size
    };
    static mmChoiceKeyNameA s_choice_a;

private:
    mmChoiceId m_id;

public:
    PlanAssumptionKind(mmChoiceId id = s_choice_a.default_id_n()) :
        m_id(s_choice_a.valid_id_n(id)) {}
    PlanAssumptionKind(const wxString& key) :
        m_id(PlanAssumptionKind::s_choice_a.find_key_n(key)) {}

    mmChoiceId id() const { return m_id; }
    const wxString key() const { return PlanAssumptionKind::s_choice_a.get_key(m_id); }
    const wxString name() const { return PlanAssumptionKind::s_choice_a.get_name(m_id); }

    // Rates are entered either as 0..1 or as a percentage; both are accepted.
    bool is_rate() const {
        return m_id == e_tax_rate || m_id == e_inflation;
    }

    bool operator== (const PlanAssumptionKind& other) const {
        return id() == other.id();
    }
    bool operator!= (const PlanAssumptionKind& other) const {
        return id() != other.id();
    }
};

struct PlanItemKind
{
public:
    enum
    {
        e_expense = 0,
        e_income,
        size
    };
    static mmChoiceKeyNameA s_choice_a;

private:
    mmChoiceId m_id;

public:
    PlanItemKind(mmChoiceId id = s_choice_a.default_id_n()) :
        m_id(s_choice_a.valid_id_n(id)) {}
    PlanItemKind(const wxString& key) :
        m_id(PlanItemKind::s_choice_a.find_key_n(key)) {}

    mmChoiceId id() const { return m_id; }
    const wxString key() const { return PlanItemKind::s_choice_a.get_key(m_id); }
    const wxString name() const { return PlanItemKind::s_choice_a.get_name(m_id); }

    bool is_income() const { return m_id == e_income; }
    // sign applied to a plan amount when accumulating a net total
    double sign() const { return m_id == e_income ? 1.0 : -1.0; }

    bool operator== (const PlanItemKind& other) const {
        return id() == other.id();
    }
    bool operator!= (const PlanItemKind& other) const {
        return id() != other.id();
    }
};

struct PlanStatus
{
public:
    enum
    {
        e_planned = 0,
        e_committed,
        e_wishlist,
        e_done,
        e_cancelled,
        size
    };
    static mmChoiceKeyNameA s_choice_a;

private:
    mmChoiceId m_id;

public:
    PlanStatus(mmChoiceId id = s_choice_a.default_id_n()) :
        m_id(s_choice_a.valid_id_n(id)) {}
    PlanStatus(const wxString& key) :
        m_id(PlanStatus::s_choice_a.find_key_n(key)) {}

    mmChoiceId id() const { return m_id; }
    const wxString key() const { return PlanStatus::s_choice_a.get_key(m_id); }
    const wxString name() const { return PlanStatus::s_choice_a.get_name(m_id); }

    // Cancelled and Done items no longer consume future funds.
    bool is_active_plan() const {
        return m_id == e_planned || m_id == e_committed || m_id == e_wishlist;
    }
    // Only these reduce "free assets": a wishlist is explicitly NOT an obligation.
    bool is_obligation() const {
        return m_id == e_planned || m_id == e_committed;
    }
    bool is_wishlist() const { return m_id == e_wishlist; }

    bool operator== (const PlanStatus& other) const {
        return id() == other.id();
    }
    bool operator!= (const PlanStatus& other) const {
        return id() != other.id();
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
    static mmChoiceKeyNameA s_choice_a;

private:
    mmChoiceId m_id;

public:
    CurrencyType(mmChoiceId id = s_choice_a.default_id_n()) :
        m_id(s_choice_a.valid_id_n(id)) {}
    CurrencyType(const wxString& key) :
        m_id(CurrencyType::s_choice_a.find_key_n(key)) {}

    mmChoiceId id() const { return m_id; }
    const wxString key() const { return CurrencyType::s_choice_a.get_key(m_id); }
    const wxString name() const { return CurrencyType::s_choice_a.get_name(m_id); }

    bool operator== (const CurrencyType& other) const {
        return id() == other.id();
    }
    bool operator!= (const CurrencyType& other) const {
        return id() != other.id();
    }
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
    static mmChoiceKeyNameA s_choice_a;

private:
    mmChoiceIdN m_id_n;

public:
    FieldTypeN(mmChoiceIdN id_n = s_choice_a.default_id_n()) :
        m_id_n(s_choice_a.valid_id_n(id_n)) {}
    FieldTypeN(const wxString& key) :
        m_id_n(FieldTypeN::s_choice_a.find_key_n(key)) {}

    bool has_value() const { return m_id_n >= 0; }
    mmChoiceIdN id_n() const { return m_id_n; }
    const wxString key_n() const {
        return has_value() ? FieldTypeN::s_choice_a.get_key(m_id_n) : "";
    }
    const wxString name_n() const {
        return has_value() ? FieldTypeN::s_choice_a.get_name(m_id_n) : "";
    }

    bool operator== (const FieldTypeN& other) const {
        return id_n() == other.id_n();
    }
    bool operator!= (const FieldTypeN& other) const {
        return id_n() != other.id_n();
    }
};

struct TrxType
{
public:
    enum
    {
        e_withdrawal = 0,
        e_deposit,
        e_transfer,
        size
    };
    static mmChoiceKeyNameA s_choice_a;

private:
    mmChoiceId m_id;

public:
    TrxType(mmChoiceId id = s_choice_a.default_id_n()) :
        m_id(s_choice_a.valid_id_n(id)) {}
    TrxType(const wxString& key) :
        m_id(TrxType::s_choice_a.find_key_n(key)) {}

    mmChoiceId id() const { return m_id; }
    const wxString key() const { return TrxType::s_choice_a.get_key(m_id); }
    const wxString name() const { return TrxType::s_choice_a.get_name(m_id); }
    const wxString trade_name() const {
        static const wxArrayString trade_name_a = {
            _n("Buy"), _n("Sell"), _n("Revalue")
        };
        return trade_name_a[m_id];
    }

    bool operator== (const TrxType& other) const {
        return id() == other.id();
    }
    bool operator!= (const TrxType& other) const {
        return id() != other.id();
    }
};

struct TrxStatus
{
public:
    enum
    {
        e_unreconciled = 0,
        e_reconciled,
        e_void,
        e_followup,
        e_duplicate,
        size
    };
    static mmChoiceKeyKeyNameA s_choice_a;

private:
    mmChoiceId m_id;

public:
    TrxStatus(mmChoiceId id = s_choice_a.default_id_n()) :
        m_id(s_choice_a.valid_id_n(id)) {}
    TrxStatus(const wxString& key) :
        m_id(TrxStatus::s_choice_a.find_key_n(key)) {}

    mmChoiceId id() const { return m_id; }
    const wxString key() const { return TrxStatus::s_choice_a.get_key(m_id); }
    const wxString name() const { return TrxStatus::s_choice_a.get_name(m_id); }

    bool operator== (const TrxStatus& other) const {
        return id() == other.id();
    }
    bool operator!= (const TrxStatus& other) const {
        return id() != other.id();
    }
};

struct RepeatMode
{
public:
    enum
    {
        e_none = 0,
        e_suggested,
        e_automated,
        size
    };
    static mmChoiceCodeNameA s_choice_a;

private:
    mmChoiceId m_id;

public:
    RepeatMode(mmChoiceId id = s_choice_a.default_id_n()) :
        m_id(s_choice_a.valid_id_n(id)) {}
    static RepeatMode from_code(int code) {
        return RepeatMode(RepeatMode::s_choice_a.find_code_n(code));
    }

    mmChoiceId id() const { return m_id; }
    int code() const { return RepeatMode::s_choice_a.get_code(m_id); }
    const wxString name() const { return RepeatMode::s_choice_a.get_name(m_id); }

    bool operator== (const RepeatMode& other) const {
        return id() == other.id();
    }
    bool operator!= (const RepeatMode& other) const {
        return id() != other.id();
    }
};

struct RepeatFreq
{
public:
    enum
    {
        e_once = 0,
        e_1_day,
        e_1_week,
        e_2_weeks,
        e_4_weeks,
        e_1_month,
        e_month_last_day,
        e_month_last_business_day,
        e_2_months,
        e_3_months,
        e_4_months,
        e_6_months,
        e_1_year,
        e_in_x_days,
        e_in_x_months,
        e_every_x_days,
        e_every_x_months,
        size
    };
    static mmChoiceCodeNameA s_choice_a;

private:
    mmChoiceId m_id;

public:
    RepeatFreq(mmChoiceId id = s_choice_a.default_id_n()) :
        m_id(s_choice_a.valid_id_n(id)) {}
    static RepeatFreq from_code(int code) {
        return RepeatFreq(RepeatFreq::s_choice_a.find_code_n(code));
    }

    mmChoiceId id() const { return m_id; }
    int code() const { return RepeatFreq::s_choice_a.get_code(m_id); }
    const wxString name() const { return RepeatFreq::s_choice_a.get_name(m_id); }
    bool is_once() const { return m_id == e_once; }
    bool is_in_x() const { return m_id >= e_in_x_days && m_id <= e_in_x_months; }
    bool is_every_x() const { return m_id >= e_every_x_days && m_id <= e_every_x_months; }
    bool has_x() const { return m_id >= e_in_x_days && m_id <= e_every_x_months; }
    bool has_num() const { return !is_once() && !has_x(); }

    bool operator== (const RepeatFreq& other) const {
        return id() == other.id();
    }
    bool operator!= (const RepeatFreq& other) const {
        return id() != other.id();
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
    static mmChoiceKeyNameA s_choice_a;

private:
    mmChoiceIdN m_id_n;

public:
    RefTypeN(mmChoiceIdN id_n = s_choice_a.default_id_n()) :
        m_id_n(s_choice_a.valid_id_n(id_n)) {}
    RefTypeN(const wxString& key) :
        m_id_n(RefTypeN::s_choice_a.find_key_n(key)) {}

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
    const wxString key_n() const {
        return has_value() ? RefTypeN::s_choice_a.get_key(m_id_n) : "";
    }
    const wxString name_n() const {
        return has_value() ? RefTypeN::s_choice_a.get_name(m_id_n) : "";
    }
    bool operator== (const RefTypeN& other) const {
        return id_n() == other.id_n();
    }
    bool operator!= (const RefTypeN& other) const {
        return id_n() != other.id_n();
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

    bool operator== (const UpdateType& other) const {
        return id() == other.id();
    }
    bool operator!= (const UpdateType& other) const {
        return id() != other.id();
    }
};
