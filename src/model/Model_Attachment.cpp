/*******************************************************
 Copyright (C) 2014 Gabriele-V

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

#include "Model_Attachment.h"

const std::vector<std::pair<Model_Attachment::REFTYPE, wxString> > Model_Attachment::REFTYPE_CHOICES =
{
	std::make_pair(Model_Attachment::TRANSACTION, "Transaction"),
	std::make_pair(Model_Attachment::STOCK, "Stock"),
	std::make_pair(Model_Attachment::ASSET, "Asset"),
	std::make_pair(Model_Attachment::BANKACCOUNT, "BankAccount"),
	std::make_pair(Model_Attachment::BILLSDEPOSIT, "RepeatingTransaction"),
	std::make_pair(Model_Attachment::PAYEE, "Payee")
};

Model_Attachment::Model_Attachment()
: Model<DB_Table_ATTACHMENT_V1>()
{
}

Model_Attachment::~Model_Attachment()
{
}

/**
* Initialize the global Model_Attachment table.
* Reset the Model_Attachment table or create the table if it does not exist.
*/
Model_Attachment& Model_Attachment::instance(wxSQLite3Database* db)
{
    Model_Attachment& ins = Singleton<Model_Attachment>::instance();
    ins.db_ = db;
    ins.destroy_cache();
    ins.ensure(db);

    return ins;
}

/** Return the static instance of Model_Attachment table */
Model_Attachment& Model_Attachment::instance()
{
    return Singleton<Model_Attachment>::instance();
}

/** Return a dataset with attachments linked to a specific object */
const Model_Attachment::Data_Set Model_Attachment::FilterAttachments(const wxString& RefType, const int RefId)
{
	Data_Set attachments;
	for (auto &attachment : this->all(COL_DESCRIPTION))
	{
		if (attachment.REFTYPE.Lower().Matches(RefType.Lower().Append("*")) && attachment.REFID == RefId)
			attachments.push_back(attachment);
	}
	return attachments;
}

/** Return the number of attachments linked to a specific object */
int Model_Attachment::NrAttachments(const wxString& RefType, const int RefId)
{
	return Model_Attachment::instance().find(Model_Attachment::DB_Table_ATTACHMENT_V1::REFTYPE(RefType), Model_Attachment::REFID(RefId)).size();
}

/** Return the last attachment file name linked to a specific object */
wxString Model_Attachment::LastAttachmentFileName(const wxString& RefType, const int RefId)
{
	wxString LastAttachmentFileName = RefType+"_0_Attach0.xxx";
	for (auto &attachment : Model_Attachment::instance().all(COL_FILENAME))
	{
		if (attachment.REFTYPE.Lower().Matches(RefType.Lower().Append("*")) && attachment.REFID == RefId)
			LastAttachmentFileName = attachment.FILENAME;
	}
	return LastAttachmentFileName;
}

/** Return the description of the choice reftype */
wxString Model_Attachment::reftype_desc(const int& RefTypeEnum)
{
	const auto& item = REFTYPE_CHOICES[RefTypeEnum];
	wxString reftype_desc = item.second;
	return reftype_desc;
}

/** Return a dataset with attachments linked to a specific type*/
std::map<int, Model_Attachment::Data_Set> Model_Attachment::get_all(REFTYPE reftype)
{
    std::map<int, Model_Attachment::Data_Set> data;
    wxString reftype_desc = Model_Attachment::reftype_desc(reftype);
    for (const auto & attachment : this->find(Model_Attachment::DB_Table_ATTACHMENT_V1::REFTYPE(reftype_desc)))
    {
        data[attachment.REFID].push_back(attachment);
    }

    return data;
}
