/*******************************************************
Copyright (C) 2013 - 2018 Guan Lisheng (guanlisheng@gmail.com)
Copyright (C) 2018 Stefano Giorgio (stef145g)

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
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <wx/datetime.h>
#include <wx/log.h>

#include "util/mmSingleton.h"
#include "util/_choices.h"

#include "table/_TableBase.h"

class wxSQLite3Statement;
class wxSQLite3Database;
class wxSQLite3ResultSet;

typedef wxDateTime wxDate;
typedef std::vector<int64> wxArrayInt64;

#if (wxMAJOR_VERSION == 3 && wxMINOR_VERSION >= 1)
// wx 3.1 has implemented such hash
#else
namespace std
{
    template<>
    struct hash<wxString>
    {
        size_t operator()(const wxString& k) const
        {
            return std::hash<std::wstring>()(k.ToStdWstring());
        }
    };
}
#endif

class ModelBase
{
public:
    enum REFTYPE_ID {
        REFTYPE_ID_TRANSACTION = 0,
        REFTYPE_ID_STOCK,
        REFTYPE_ID_ASSET,
        REFTYPE_ID_BANKACCOUNT,
        REFTYPE_ID_BILLSDEPOSIT,
        REFTYPE_ID_PAYEE,
        REFTYPE_ID_TRANSACTIONSPLIT,
        REFTYPE_ID_BILLSDEPOSITSPLIT,
        REFTYPE_ID_size
    };

    static ChoicesName REFTYPE_CHOICES;
    static const wxString REFTYPE_NAME_TRANSACTION;
    static const wxString REFTYPE_NAME_STOCK;
    static const wxString REFTYPE_NAME_ASSET;
    static const wxString REFTYPE_NAME_BANKACCOUNT;
    static const wxString REFTYPE_NAME_BILLSDEPOSIT;
    static const wxString REFTYPE_NAME_PAYEE;
    static const wxString REFTYPE_NAME_TRANSACTIONSPLIT;
    static const wxString REFTYPE_NAME_BILLSDEPOSITSPLIT;
    static const wxString reftype_name(int id);
    static int reftype_id(const wxString& name, int default_id = -1);

public:
    ModelBase() {};
    virtual ~ModelBase() {};

public:
    virtual wxString GetTableStatsAsJson() const = 0;
    virtual void show_statistics() const = 0;
    virtual void destroyCache() = 0;
};

inline const wxString ModelBase::reftype_name(int id)
{
    return REFTYPE_CHOICES.getName(id);
}

inline int ModelBase::reftype_id(const wxString& name, int default_id)
{
    return REFTYPE_CHOICES.findName(name, default_id);
}

template<class DB_TABLE>
class Model : public ModelBase, public DB_TABLE
{
public:
    using DB_TABLE::save;
    using DB_TABLE::remove;
    using DB_TABLE::cache_id;
    using DB_TABLE::get_id;
    using DB_TABLE::get_all;

    template<typename... Args>
    /**
    * Command: find(const Args&... args)
    * Args: One or more Specialised Parameters creating SQL statement conditions used after the WHERE statement.
    * Specialised Parameters: Table_Column_Name(content)[, Table_Column_Name(content)[, ...]]
    * Example:
    *   AssetModel::ASSETID(2), AssetModel::ASSETTYPE(AssetModel::TYPE_ID_JEWELLERY)
    *   produces SQL statement condition: ASSETID = 2 AND ASSETTYPE = "Jewellery"
    * Returns a Data_Set containing the addresses of the items found.
    * The Data_Set is empty when nothing found.
    */
    const typename DB_TABLE::Data_Set find(const Args&... args)
    {
        return this->find_by(this, true, args...);
    }

    template<typename... Args>
    /**
    * Command: find_or(const Args&... args)
    * Args: One or more Specialised Parameters creating SQL statement conditions used after the WHERE statement.
    * Specialised Parameters: Table_Column_Name(content)[, Table_Column_Name(content)[, ...]]
    * Example:
    *   AssetModel::ASSETID(2), AssetModel::ASSETTYPE(AssetModel::TYPE_ID_JEWELLERY)
    *   produces SQL statement condition: ASSETID = 2 OR ASSETTYPE = "Jewellery"
    * Returns a Data_Set containing the addresses of the items found.
    * The Data_Set is empty when nothing found.
    */
    const typename DB_TABLE::Data_Set find_or(const Args&... args)
    {
        return this->find_by(this, false, args...);
    }

    typename DB_TABLE::Data* cache_id(wxLongLong_t id)
    {
        return this->cache_id(int64(id));
    }

    // Save all Data record memory instances contained
    // in the record list (Data_Set) to the database.
    template<class DATA>
    int save(std::vector<DATA>& rows)
    {
        this->Savepoint();
        for (auto& r : rows) {
            if (r.id() < 0)
                wxLogDebug("Incorrect function call to save %s", r.to_json().utf8_str());
            this->save(&r);
        }
        this->ReleaseSavepoint();

        return rows.size();
    }

    template<class DATA>
    int save(std::vector<DATA*>& rows)
    {
        this->Savepoint();
        for (auto& r : rows)
            this->save(r);
        this->ReleaseSavepoint();

        return rows.size();
    }

public:
    void preload(int max_num = 1000)
    {
        int i = 0;
        for (const auto & item : get_all()) {
            cache_id(item.id());
            if (++i >= max_num) break;
        }
    }

    // Return accomulated table stats as a json string
    wxString  GetTableStatsAsJson() const
    {
        StringBuffer json_buffer;
        rapidjson::Writer<StringBuffer> json_writer(json_buffer);
        json_writer.StartObject();
        json_writer.Key("table_name");
        json_writer.String(this->m_table_name.utf8_str());
        json_writer.Key("cache");
        json_writer.Int(this->m_cache.size());
        json_writer.Key("cache_index");
        json_writer.Int(this->m_cache_index.size());
        json_writer.Key("hit");
        json_writer.Int(this->m_hit);
        json_writer.Key("miss");
        json_writer.Int(this->m_miss);
        json_writer.Key("skip");
        json_writer.Int(this->m_skip);
        json_writer.EndObject();

        wxLogDebug("======== model/_ModelBase.h : GetTableStatsAsJson =======");
        wxLogDebug("%s", wxString::FromUTF8(json_buffer.GetString()));

        return wxString::FromUTF8(json_buffer.GetString());
    }

    void destroyCache()
    {
        if (this->m_cache.size() > 0)
            this->destroy_cache();
    }

    // Show table statistics
    void show_statistics() const
    {
        wxLogDebug("%s : (cache %zu, cache_index %zu, hit %zu, miss %zu, skip %zu)",
            this->m_table_name,
            this->m_cache.size(),
            this->m_cache_index.size(),
            this->m_hit, this->m_miss, this->m_skip
        );
    }
};
