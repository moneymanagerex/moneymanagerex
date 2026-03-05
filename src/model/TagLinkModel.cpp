/*******************************************************
 Copyright (C) 2016 Guan Lisheng

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

#include "TagLinkModel.h"
#include "AttachmentModel.h"
#include "TrxModel.h"
#include "TagModel.h"

TagLinkModel::TagLinkModel() :
    TableFactory<TagLinkTable, TagLinkData>()
{
}

TagLinkModel::~TagLinkModel()
{
}

/**
* Initialize the global TagLinkModel.
*/
TagLinkModel& TagLinkModel::instance(wxSQLite3Database* db)
{
    TagLinkModel& ins = Singleton<TagLinkModel>::instance();
    ins.reset_cache();
    ins.m_db = db;
    ins.ensure_table();
    ins.preload_cache();

    return ins;
}

/** Return the static instance of TagLinkModel */
TagLinkModel& TagLinkModel::instance()
{
    return Singleton<TagLinkModel>::instance();
}

const TagLinkData* TagLinkModel::get_key(const wxString& refType, int64 refId, int64 tagId)
{
    const Data* gl_n = search_cache_n(
        TagLinkCol::REFTYPE(refType), TagLinkCol::REFID(refId), TagLinkCol::TAGID(tagId)
    );
    if (gl_n)
        return gl_n;

    DataA items = this->find(
        TagLinkCol::REFTYPE(refType), TagLinkCol::REFID(refId), TagLinkCol::TAGID(tagId)
    );
    if (!items.empty())
        gl_n = get_id_data_n(items[0].TAGLINKID);
    return gl_n;
}

std::map<wxString, int64> TagLinkModel::get_ref(const wxString& refType, int64 refId)
{
    std::map<wxString, int64> tags;
    for (const auto& gl_d : instance().find(
        TagLinkCol::REFTYPE(refType), TagLinkCol::REFID(refId)
    ))
        tags[TagModel::instance().get_id_data_n(gl_d.TAGID)->m_name] = gl_d.TAGID;

    return tags;
}

/* Delete all tags for a REFTYPE + REFID */
void TagLinkModel::DeleteAllTags(const wxString& refType, int64 refID)
{
    const auto& links = instance().find(
        TagLinkCol::REFTYPE(refType), TagLinkCol::REFID(refID)
    );
    instance().db_savepoint();
    for (const auto& link : links)
        instance().purge_id(link.TAGLINKID);
    instance().db_release_savepoint();
}

int TagLinkModel::update(const DataA& rows, const wxString& refType, int64 ref_id)
{
    TagLinkModel::instance().db_savepoint();
    bool save_timestamp = false;
    std::map<int, int64> row_id_map;

    DataA links = instance().find(
        TagLinkCol::REFTYPE(refType), TagLinkCol::REFID(ref_id)
    );
    if (links.size() != rows.size())
        save_timestamp = true;

    for (const auto& link : links) {
        if (!save_timestamp) {
            bool match = false;
            for (decltype(rows.size()) i = 0; i < rows.size(); i++) {
                match = (rows[i].TAGID == link.TAGID && row_id_map.find(i) == row_id_map.end());
                if (match) {
                    row_id_map[i] = link.TAGLINKID;
                    break;
                }
            }
            save_timestamp = save_timestamp || !match;
        }

        instance().purge_id(link.TAGLINKID);
    }

    for (const auto& item : rows) {
        Data new_gl_d = Data();
        new_gl_d.REFTYPE = refType;
        new_gl_d.REFID   = ref_id;
        new_gl_d.TAGID   = item.TAGID;
        instance().add_data_n(new_gl_d);
    }

    if (save_timestamp) {
        if (refType == TrxModel::refTypeName)
            TrxModel::instance().save_timestamp(ref_id);
        else if (refType == TrxSplitModel::refTypeName)
            TrxModel::instance().save_timestamp(
                TrxSplitModel::instance().get_id_data_n(ref_id)->m_trx_id
            );
    }

    TagLinkModel::instance().db_release_savepoint();

    return rows.size();
}

std::map<int64, TagLinkModel::DataA> TagLinkModel::get_all_id(const wxString& refType)
{
    DataA taglinks = instance().find(TagLinkCol::REFTYPE(refType));

    std::map<int64, DataA> data;
    for (const auto& taglink : taglinks) {
        data[taglink.REFID].push_back(taglink);
    }
    return data;
}
