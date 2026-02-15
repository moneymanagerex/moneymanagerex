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
#include "TransactionModel.h"
#include "TagModel.h"

TagLinkModel::TagLinkModel()
: Model<TagLinkTable>()
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
    ins.m_db = db;
    ins.destroy_cache();
    ins.ensure_table();
    ins.preload();

    return ins;
}

/** Return the static instance of TagLinkModel */
TagLinkModel& TagLinkModel::instance()
{
    return Singleton<TagLinkModel>::instance();
}

TagLinkModel::Data* TagLinkModel::cache_key(const wxString& refType, int64 refId, int64 tagId)
{
    Data* link = this->search_cache(REFTYPE(refType), REFID(refId), TAGID(tagId));
    if (link)
        return link;

    Data_Set items = this->find(REFTYPE(refType), REFID(refId), TAGID(tagId));
    if (!items.empty())
        link = this->cache_id(items[0].TAGLINKID);
    return link;
}

std::map<wxString, int64> TagLinkModel::cache_ref(const wxString& refType, int64 refId)
{
    std::map<wxString, int64> tags;
    for (const auto& link : instance().find(REFTYPE(refType), REFID(refId)))
        tags[TagModel::instance().cache_id(link.TAGID)->TAGNAME] = link.TAGID;

    return tags;
}

/* Delete all tags for a REFTYPE + REFID */
void TagLinkModel::DeleteAllTags(const wxString& refType, int64 refID)
{
    const auto& links = instance().find(REFTYPE(refType), REFID(refID));
    instance().Savepoint();
    for (const auto& link : links)
        instance().remove(link.TAGLINKID);
    instance().ReleaseSavepoint();
}

int TagLinkModel::update(const Data_Set& rows, const wxString& refType, int64 refId)
{
    TagLinkModel::instance().Savepoint();
    bool updateTimestamp = false;
    std::map<int, int64> row_id_map;

    Data_Set links = instance().find(REFTYPE(refType), REFID(refId));
    if (links.size() != rows.size()) updateTimestamp = true;

    for (const auto& link : links)
    {
        if (!updateTimestamp)
        {
            bool match = false;
            for (decltype(rows.size()) i = 0; i < rows.size(); i++)
            {
                match = (rows[i].TAGID == link.TAGID && row_id_map.find(i) == row_id_map.end());
                if (match)
                {
                    row_id_map[i] = link.TAGLINKID;
                    break;
                }
            }
            updateTimestamp = updateTimestamp || !match;
        }

        instance().remove(link.TAGLINKID);
    }

    if (!rows.empty())
    {
        for (const auto& item : rows)
        {
            Data* taglink = instance().create();
            taglink->REFID = refId;
            taglink->REFTYPE = refType;
            taglink->TAGID = item.TAGID;
            instance().save(taglink);
        }
    }

    if (updateTimestamp)
    {
        if (refType == TransactionModel::refTypeName)
            TransactionModel::instance().updateTimestamp(refId);
        else if (refType == TransactionSplitModel::refTypeName)
            TransactionModel::instance().updateTimestamp(TransactionSplitModel::instance().cache_id(refId)->TRANSID);
    }

    TagLinkModel::instance().ReleaseSavepoint();

    return rows.size();
}

std::map<int64, TagLinkModel::Data_Set> TagLinkModel::get_all_id(const wxString& refType)
{
    Data_Set taglinks = instance().find(REFTYPE(refType));

    std::map<int64, Data_Set> data;
    for (const auto& taglink : taglinks) {
        data[taglink.REFID].push_back(taglink);
    }
    return data;
}
