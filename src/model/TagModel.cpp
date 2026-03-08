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

#include "TagModel.h"
#include "TagLinkModel.h"
#include "AttachmentModel.h"
#include "TrxModel.h"

TagModel::TagModel() :
    TableFactory<TagTable, TagData>()
{
}

TagModel::~TagModel()
{
}

// Initialize the global TagModel.
TagModel& TagModel::instance(wxSQLite3Database* db)
{
    TagModel& ins = Singleton<TagModel>::instance();
    ins.reset_cache();
    ins.m_db = db;
    ins.ensure_table();

    return ins;
}

// Return the static instance of TagModel
TagModel& TagModel::instance()
{
    return Singleton<TagModel>::instance();
}

int TagModel::is_used(int64 tag_id)
{
    TagLinkModel::DataA gl_a = TagLinkModel::instance().find(
        TagLinkCol::TAGID(tag_id)
    );

    if (gl_a.empty())
        return 0;

    for (const auto& gl_d : gl_a) {
        // FIXME: do not exclude deleted transactions
        if (gl_d.m_ref_type == TrxModel::s_ref_type) {
            const TrxData* trx_n = TrxModel::instance().get_id_data_n(gl_d.m_ref_id);
            if (trx_n && trx_n->DELETEDTIME.IsEmpty())
                return 1;
        }
        else if (gl_d.m_ref_type == TrxSplitModel::s_ref_type) {
            const TrxSplitData* tp_n = TrxSplitModel::instance().get_id_data_n(gl_d.m_ref_id);
            if (tp_n) {
                const TrxData* trx_n = TrxModel::instance().get_id_data_n(tp_n->m_trx_id);
                if (trx_n && trx_n->DELETEDTIME.IsEmpty())
                    return 1;
            }
        }
        else
            return 1;
    }

    return -1;
}

const TagData* TagModel::get_name_data_n(const wxString& name)
{
    const Data* tag_n = search_cache_n(TagCol::TAGNAME(name));
    if (tag_n)
        return tag_n;

    DataA tag_a = this->find(TagCol::TAGNAME(name));
    if (!tag_a.empty())
        tag_n = get_id_data_n(tag_a[0].m_id);
    return tag_n;
}

