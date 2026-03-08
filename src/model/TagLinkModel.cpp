/*******************************************************
 Copyright (C) 2016 Guan Lisheng
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

// Initialize the global TagLinkModel.
TagLinkModel& TagLinkModel::instance(wxSQLite3Database* db)
{
    TagLinkModel& ins = Singleton<TagLinkModel>::instance();
    ins.reset_cache();
    ins.m_db = db;
    ins.ensure_table();
    ins.preload_cache();

    return ins;
}

// Return the static instance of TagLinkModel
TagLinkModel& TagLinkModel::instance()
{
    return Singleton<TagLinkModel>::instance();
}

// Delete all tag links for a (REFTYPE, REFID)
void TagLinkModel::purge_ref(RefTypeN ref_type, int64 ref_id)
{
    const auto& gl_a = instance().find(
        TagLinkCol::REFTYPE(ref_type.name_n()),
        TagLinkCol::REFID(ref_id)
    );
    instance().db_savepoint();
    for (const auto& gl_d : gl_a)
        instance().purge_id(gl_d.m_id);
    instance().db_release_savepoint();
}

const TagLinkData* TagLinkModel::get_key_data_n(int64 tag_id, RefTypeN ref_type, int64 ref_id)
{
    const Data* gl_n = search_cache_n(
        TagLinkCol::TAGID(tag_id),
        TagLinkCol::REFTYPE(ref_type.name_n()),
        TagLinkCol::REFID(ref_id)
    );
    if (gl_n)
        return gl_n;

    DataA gl_a = find(
        TagLinkCol::TAGID(tag_id),
        TagLinkCol::REFTYPE(ref_type.name_n()),
        TagLinkCol::REFID(ref_id)
    );
    if (!gl_a.empty())
        gl_n = get_id_data_n(gl_a[0].m_id);
    return gl_n;
}

std::map<wxString, int64> TagLinkModel::find_ref_tag_m(RefTypeN ref_type, int64 ref_id)
{
    std::map<wxString, int64> tag_name_id_m;
    for (const auto& gl_d : find(
        TagLinkCol::REFTYPE(ref_type.name_n()),
        TagLinkCol::REFID(ref_id)
    )) {
        const TagData* tag_n = TagModel::instance().get_id_data_n(gl_d.m_tag_id);
        tag_name_id_m[tag_n->m_name] = gl_d.m_tag_id;
    }
    return tag_name_id_m;
}

std::map<int64, TagLinkModel::DataA> TagLinkModel::find_refType_mRefId(
    RefTypeN ref_type
) {
    std::map<int64, DataA> refId_dataA_m;
    for (const auto& gl_d : instance().find(
        TagLinkCol::REFTYPE(ref_type.name_n())
    )) {
        refId_dataA_m[gl_d.m_ref_id].push_back(gl_d);
    }
    return refId_dataA_m;
}

int TagLinkModel::update(RefTypeN ref_type, int64 ref_id, const DataA& src_gl_a)
{
    TagLinkModel::instance().db_savepoint();
    bool save_timestamp = false;
    std::map<int, int64> index_id_m;

    DataA old_gl_a = instance().find(
        TagLinkCol::REFTYPE(ref_type.name_n()),
        TagLinkCol::REFID(ref_id)
    );
    if (old_gl_a.size() != src_gl_a.size())
        save_timestamp = true;

    for (const auto& old_gl_d : old_gl_a) {
        if (!save_timestamp) {
            bool match = false;
            for (decltype(src_gl_a.size()) i = 0; i < src_gl_a.size(); i++) {
                match = (src_gl_a[i].m_tag_id == old_gl_d.m_tag_id &&
                    index_id_m.find(i) == index_id_m.end()
                );
                if (match) {
                    index_id_m[i] = old_gl_d.m_id;
                    break;
                }
            }
            save_timestamp = save_timestamp || !match;
        }

        instance().purge_id(old_gl_d.m_id);
    }

    for (const auto& src_gl_d : src_gl_a) {
        Data new_gl_d = Data();
        new_gl_d.m_tag_id   = src_gl_d.m_tag_id;
        new_gl_d.m_ref_type = ref_type;
        new_gl_d.m_ref_id   = ref_id;
        instance().add_data_n(new_gl_d);
    }

    if (save_timestamp) {
        if (ref_type == TrxModel::s_ref_type)
            TrxModel::instance().save_timestamp(ref_id);
        else if (ref_type == TrxSplitModel::s_ref_type)
            TrxModel::instance().save_timestamp(
                TrxSplitModel::instance().get_id_data_n(ref_id)->m_trx_id
            );
    }

    TagLinkModel::instance().db_release_savepoint();

    return src_gl_a.size();
}
