/*******************************************************
 Copyright (C) 2025-2026 George Ef (george.a.ef@gmail.com)
 Copyright (C) 2025      Klaus Wich

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

#include <unordered_map>
#include "base/defs.h"

typedef int mmChoiceId;   // choice id (enum case), always >= 0
typedef int mmChoiceIdN;  // nullable choice id, either -1 or >= 0

class mmChoiceNameA {
public:
    struct Choice { mmChoiceId id; wxString name; };

private:
    const std::vector<Choice> m_choice_a;
    const mmChoiceIdN m_default_id_n;
    const bool m_nocase;
    std::unordered_map<wxString, mmChoiceIdN> m_name_id_m; // name -> id_n

public:
    mmChoiceNameA(
        const std::vector<Choice>& choice_a,
        mmChoiceIdN default_id_n = -1,
        bool nocase = true
    ) :
        m_choice_a(choice_a),
        m_default_id_n(default_id_n),
        m_nocase(nocase)
    {}
    ~mmChoiceNameA() {}

    mmChoiceIdN default_id_n() const { return m_default_id_n; }
    mmChoiceIdN valid_id_n(mmChoiceIdN id_n) const;
    const wxString get_name(mmChoiceId id) const;
    mmChoiceIdN find_name_n(const wxString& name);
};

class mmChoiceCodeNameA {
public:
    struct Choice { mmChoiceId id; int code; wxString name; };

private:
    const std::vector<Choice> m_choice_a;
    const mmChoiceIdN m_default_id_n;
    const bool m_nocase;
    std::unordered_map<int,      mmChoiceIdN> m_code_id_m; // code -> id_n
    std::unordered_map<wxString, mmChoiceIdN> m_name_id_m; // name -> id_n

public:
    mmChoiceCodeNameA(
        const std::vector<Choice>& choice_a,
        mmChoiceIdN default_id_n = -1,
        bool nocase = true
    ) :
        m_choice_a(choice_a),
        m_default_id_n(default_id_n),
        m_nocase(nocase)
    {}
    ~mmChoiceCodeNameA() {}

    mmChoiceIdN default_id_n() const { return m_default_id_n; }
    mmChoiceIdN valid_id_n(mmChoiceIdN id_n) const;
    int get_code(mmChoiceId id) const;
    const wxString get_name(mmChoiceId id) const;
    mmChoiceIdN find_code_n(int code);
    mmChoiceIdN find_name_n(const wxString& name);
};

class mmChoiceKeyNameA {
public:
    struct Choice { mmChoiceId id; wxString key; wxString name; };

private:
    const std::vector<Choice> m_choice_a;
    const mmChoiceIdN m_default_id_n;
    const bool m_nocase;
    std::unordered_map<wxString, mmChoiceIdN> m_keyname_id_m; // key or name -> id_n

public:
    mmChoiceKeyNameA(
        const std::vector<Choice>& choice_a,
        mmChoiceIdN default_id_n = -1,
        bool nocase = true
    ) :
        m_choice_a(choice_a),
        m_default_id_n(default_id_n),
        m_nocase(nocase)
    {}
    ~mmChoiceKeyNameA() {}

    mmChoiceIdN default_id_n() const { return m_default_id_n; }
    mmChoiceIdN valid_id_n(mmChoiceIdN id_n) const;
    const wxString get_key(mmChoiceId id) const;
    const wxString get_name(mmChoiceId id) const;
    mmChoiceIdN find_keyname_n(const wxString& keyname);
};
