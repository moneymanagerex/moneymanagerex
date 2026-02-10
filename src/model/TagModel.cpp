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
#include "TransactionModel.h"

TagModel::TagModel()
: Model<DB_Table_TAG_V1>()
{
}

TagModel::~TagModel()
{
}

/**
* Initialize the global TagModel.
*/
TagModel& TagModel::instance(wxSQLite3Database* db)
{
    TagModel& ins = Singleton<TagModel>::instance();
    ins.db_ = db;
    ins.destroy_cache();
    ins.ensure(db);

    return ins;
}

/** Return the static instance of TagModel */
TagModel& TagModel::instance()
{
    return Singleton<TagModel>::instance();
}

TagModel::Data* TagModel::get(const wxString& name)
{
    Data* tag = this->get_one(TAGNAME(name));
    if (tag) return tag;

    Data_Set items = this->find(TAGNAME(name));
    if (!items.empty()) tag = this->get(items[0].TAGID, this->db_);
    return tag;
}

int TagModel::is_used(int64 id)
{
    TagLinkModel::Data_Set taglink = TagLinkModel::instance().find(TagLinkModel::TAGID(id));

    if (taglink.empty())
        return 0;

    for (const auto& link : taglink)
    {
        if (link.REFTYPE == TransactionModel::refTypeName)
        {
            TransactionModel::Data* t = TransactionModel::instance().get(link.REFID);
            if (t && t->DELETEDTIME.IsEmpty())
                return 1;
        }
        else if (link.REFTYPE == TransactionSplitModel::refTypeName)
        {
            TransactionSplitModel::Data* s = TransactionSplitModel::instance().get(link.REFID);
            if (s)
            {
                TransactionModel::Data* t = TransactionModel::instance().get(s->TRANSID);
                if (t && t->DELETEDTIME.IsEmpty())
                    return 1;
            }
        }
        else
            return 1;
    }

    return -1;
}

