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

#include "mmCache.h"

// If key is in cache, return a pointer to the value in cache.
// If key is not in cache, return nullptr.
// The returned pointer can modify the value in cache.
// The returned pointer is invalidated at the next clear() or reset();
// if capacity is set (> 0) and the cache is not locked (lock_c == 0),
// the returned may be invalidated at the next add() or set().
template<typename K, typename V>
auto mmCache<K, V>::unsafe_get(const Key& key) -> Value*
{
    auto it = m_key_value.find(key);
    if (it != m_key_value.end()) {
        ++m_stat.hit_c;
        return it->second;
    }
    else {
        ++m_stat.miss_c;
        return nullptr;
    }
}

// Same as unsafe_get(const Key&), except that
// the returned pointer cannot modify the value.
template<typename K, typename V>
auto mmCache<K, V>::get(const Key& key) -> const Value*
{
    return unsafe_get(key);
}

// If key is not in cache, copy value into cache and return a pointer
// to the copy owned by cache. If key is alredy in cache, return nullptr.
// value shall not be owned by cache before the call; it is not embraced
// by cache after the call.
// If capacity is set (> 0) and the cache is not locked (lock_c == 0),
// this call may invalidate pointers into cache.
template<typename K, typename V>
auto mmCache<K, V>::add(const Key& key, const Value& value) -> const Value*
{
    if (m_key_value.find(key) != m_key_value.end())
        return nullptr;

    if (m_stat.lock_c == 0 && m_stat.capacity > 0 && m_key_value.size() >= m_stat.capacity)
        clear();

    m_key_value[key] = new Value(value);

    size_t size = m_key_value.size();
    if (m_stat.max_size < size)
        m_stat.max_size = size;

    return m_key_value[key];
}

// If key is in cache, update value in cache and return a pointer
// to the copy owned by cache. If key is not in cache, return nullptr.
template<typename K, typename V>
auto mmCache<K, V>::update(const Key& key, const Value& value) -> const Value*
{
    if (m_key_value.find(key) == m_key_value.end())
        return nullptr;

    *(m_key_value[key]) = value;

    return m_key_value[key];
}

// Copy or update value into cache and return a pointer to the copy owned by cache.
// If capacity is set (> 0) and the cache is not locked (lock_c == 0),
// this call may invalidate pointers into cache.
template<typename K, typename V>
auto mmCache<K, V>::set(const Key& key, const Value& value) -> const Value*
{
    if (m_key_value.find(key) == m_key_value.end()) {
        return add(key, value);
    }
    else {
        return update(key, value);
    }
}

// If key is in cache, remove it and return true.
// If key is not in cache, return false.
template<typename K, typename V>
bool mmCache<K, V>::remove(const Key& key)
{
    if (m_key_value.find(key) == m_key_value.end())
        return false;

    delete m_key_value[key];
    m_key_value.erase(key);

    return true;
}

// Increase the lock counter, in order to prevent cleanup when capacity is reached.
template<typename K, typename V>
void mmCache<K, V>::lock()
{
    ++m_stat.lock_c;
}

// Decrease the lock counter.
template<typename K, typename V>
void mmCache<K, V>::unlock()
{
    if (m_stat.lock_c > 0)
        --m_stat.lock_c;
}

// If the cache is not locked, remove all keys and delete all values;
// this invalidates all pointers into cache.
// Do not reset statistics.
template<typename K, typename V>
void mmCache<K, V>::clear()
{
    if (m_stat.lock_c > 0)
        return;

    for (auto& [_, v] : m_key_value)
        delete v;

    m_key_value.clear();
}

// Remove all keys and delete all values, even if the cache is locked;
// this invalidates all pointers into cache.
// Reset statistics.
template<typename K, typename V>
void mmCache<K, V>::reset()
{
    m_stat.reset();
    clear();
}
