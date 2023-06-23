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

Model_Taglink::Data* Model_Taglink::get(const wxString& refType, int refId, int tagId)
{
    Data* link = this->get_one(REFTYPE(refType), REFID(refId), TAGID(tagId));
    if (link) return link;

    Data_Set items = this->find(REFTYPE(refType), REFID(refId), TAGID(tagId));
    if (!items.empty()) link = this->get(items[0].TAGLINKID, this->db_);
    return link;
}
/* Delete all tags for a REFTYPE + REFID */
void Model_Taglink::DeleteAllTags(const wxString& refType, int refID)
{
    const auto& links = instance().find(REFTYPE(refType), REFID(refID));
    instance().Savepoint();
    for (const auto& link : links)
        instance().remove(link.TAGLINKID);
    instance().ReleaseSavepoint();
}

int Model_Taglink::update(const Data_Set& rows, const wxString& refType, int refId)
{
    bool updateTimestamp = false;
    std::map<int, int> row_id_map;

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
        Data_Set taglinks;
        for (const auto& item : rows)
        {
            Data* taglink = instance().create();
            taglink->REFID = refId;
            taglink->REFTYPE = refType;
            taglink->TAGID = item.TAGID;
            taglinks.push_back(*taglink);
        }
        instance().save(taglinks);
    }

    if (updateTimestamp && refType == Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION))
        Model_Checking::instance().updateTimestamp(refId);

    return rows.size();
}