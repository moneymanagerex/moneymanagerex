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


#include "Model_Taglink.h"
#include "Model_Attachment.h"
#include "Model_Checking.h"
#include "Model_Tag.h"

Model_Taglink::Model_Taglink()
: Model<DB_Table_TAGLINK_V1>()
{
}

Model_Taglink::~Model_Taglink()
{
}

/**
* Initialize the global Model_Taglink.
*/
Model_Taglink& Model_Taglink::instance(wxSQLite3Database* db)
{
    Model_Taglink& ins = Singleton<Model_Taglink>::instance();
    ins.db_ = db;
    ins.destroy_cache();
    ins.ensure(db);
    ins.preload();

    return ins;
}

/** Return the static instance of Model_Taglink */
Model_Taglink& Model_Taglink::instance()
{
    return Singleton<Model_Taglink>::instance();
}

Model_Taglink::Data* Model_Taglink::get(const wxString& refType, int64 refId, int64 tagId)
{
    Data* link = this->get_one(REFTYPE(refType), REFID(refId), TAGID(tagId));
    if (link) return link;

    Data_Set items = this->find(REFTYPE(refType), REFID(refId), TAGID(tagId));
    if (!items.empty()) link = this->get(items[0].TAGLINKID, this->db_);
    return link;
}

std::map<wxString, int64> Model_Taglink::get(const wxString& refType, int64 refId)
{
    std::map<wxString, int64> tags;
    for (const auto& link : instance().find(REFTYPE(refType), REFID(refId)))
        tags[Model_Tag::instance().get(link.TAGID)->TAGNAME] = link.TAGID;

    return tags;
}

/* Delete all tags for a REFTYPE + REFID */
void Model_Taglink::DeleteAllTags(const wxString& refType, int64 refID)
{
    const auto& links = instance().find(REFTYPE(refType), REFID(refID));
    instance().Savepoint();
    for (const auto& link : links)
        instance().remove(link.TAGLINKID);
    instance().ReleaseSavepoint();
}

int Model_Taglink::update(const Data_Set& rows, const wxString& refType, int64 refId)
{
    Model_Taglink::instance().Savepoint();
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
        if (refType == Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION))
            Model_Checking::instance().updateTimestamp(refId);
        else if (refType == Model_Attachment::reftype_desc(Model_Attachment::TRANSACTIONSPLIT))
            Model_Checking::instance().updateTimestamp(Model_Splittransaction::instance().get(refId)->TRANSID);
    }

    Model_Taglink::instance().ReleaseSavepoint();

    return rows.size();
}

std::map<int64, Model_Taglink::Data_Set> Model_Taglink::get_all(const wxString& refType)
{
    Data_Set taglinks = instance().find(REFTYPE(refType));

    std::map<int64, Data_Set> data;
    for (const auto& taglink : taglinks)
    {
        data[taglink.REFID].push_back(taglink);
    }
    return data;
}
