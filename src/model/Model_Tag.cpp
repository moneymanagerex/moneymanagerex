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


#include "Model_Tag.h"
#include "Model_Taglink.h"
#include "Model_Attachment.h"
#include "Model_Checking.h"

Model_Tag::Model_Tag()
: Model<DB_Table_TAG_V1>()
{
}

Model_Tag::~Model_Tag()
{
}

/**
* Initialize the global Model_Tag.
*/
Model_Tag& Model_Tag::instance(wxSQLite3Database* db)
{
    Model_Tag& ins = Singleton<Model_Tag>::instance();
    ins.db_ = db;
    ins.destroy_cache();
    ins.ensure(db);

    return ins;
}

/** Return the static instance of Model_Tag */
Model_Tag& Model_Tag::instance()
{
    return Singleton<Model_Tag>::instance();
}

Model_Tag::Data* Model_Tag::get(const wxString& name)
{
    Data* tag = this->get_one(TAGNAME(name));
    if (tag) return tag;

    Data_Set items = this->find(TAGNAME(name));
    if (!items.empty()) tag = this->get(items[0].TAGID, this->db_);
    return tag;
}

int Model_Tag::is_used(int64 id)
{
    Model_Taglink::Data_Set taglink = Model_Taglink::instance().find(Model_Taglink::TAGID(id));

    if (taglink.empty())
        return 0;

    for (const auto& link : taglink)
    {
        if (link.REFTYPE == Model_Attachment::REFTYPE_STR_TRANSACTION)
        {
            Model_Checking::Data* t = Model_Checking::instance().get(link.REFID);
            if (t && t->DELETEDTIME.IsEmpty())
                return 1;
        }
        else if (link.REFTYPE == Model_Attachment::REFTYPE_STR_TRANSACTIONSPLIT)
        {
            Model_Splittransaction::Data* s = Model_Splittransaction::instance().get(link.REFID);
            if (s)
            {
                Model_Checking::Data* t = Model_Checking::instance().get(s->TRANSID);
                if (t && t->DELETEDTIME.IsEmpty())
                    return 1;
            }
        }
        else
            return 1;
    }

    return -1;
}

