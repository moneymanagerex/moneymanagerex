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

#include <unordered_map>

struct mmCacheStat
{
    size_t capacity, max_size, lock_c, hit_c, miss_c;

    mmCacheStat(size_t capacity_ = 0) :
        capacity(capacity_), max_size(0), lock_c(0), hit_c(0), miss_c(0)
    {
    }

    void reset() { max_size = 0; lock_c = 0; hit_c = 0; miss_c = 0; }
};

template<typename KeyType, typename ValueType>
struct mmCache
{
    using Key   = KeyType;
    using Value = ValueType;
    using Map   = std::unordered_map<Key, Value*>;

public:
    Map m_key_value;
    mmCacheStat m_stat;

public:
    mmCache(size_t capacity_ = 0) : m_stat(mmCacheStat(capacity_)) {}

    auto unsafe_get(const Key& key) -> Value*;
    auto get(const Key& key) -> const Value*;
    auto add(const Key& key, const Value& value) -> const Value*;
    auto update(const Key& key, const Value& value) -> const Value*;
    auto set(const Key& key, const Value& value) -> const Value*;
    bool remove(const Key& key);
    void lock();
    void unlock();
    void clear();
    void reset();

    auto get_map() const -> const Map& { return m_key_value; }
    auto get_stat() const -> const mmCacheStat& { return m_stat; }
};
