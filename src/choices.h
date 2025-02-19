/*******************************************************
 Copyright (C) 2025 George Ef (george.a.ef@gmail.com)

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
#include "defs.h"

class ChoicesName {
public:
    struct Item { int id; wxString name; };

private:
    const std::vector<Item> array;
    const bool nocase;
    std::unordered_map<wxString, int> name_id; // cache for findName()

public:
    ChoicesName(const std::vector<Item>& array_, bool nocase_ = true);
    ~ChoicesName();

    const wxString getName(int id) const;
    int findName(const wxString& name, int default_id);
};

class ChoicesKeyName {
public:
    struct Item { int id; wxString key; wxString name; };

private:
    const std::vector<Item> array;
    const bool nocase;
    std::unordered_map<wxString, int> keyOrName_id;  // cache for findKeyName()

public:
    ChoicesKeyName(const std::vector<Item>& array_, bool nocase_ = true);
    ~ChoicesKeyName();

    const wxString getKey(int id) const;
    const wxString getName(int id) const;
    int findKeyName(const wxString& keyOrName, int default_id);
};

