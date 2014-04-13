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

#ifndef MODEL_ATTACHMENT_H
#define MODEL_ATTACHMENT_H

#include "Model.h"
#include "db/DB_Table_Attachment_V1.h"

class Model_Attachment : public Model<DB_Table_ATTACHMENT_V1>
{
public:
    using Model<DB_Table_ATTACHMENT_V1>::get;
	enum REFTYPE { TRANSACTION = 0, PAYEE};

	static const std::vector<std::pair<REFTYPE, wxString> > REFTYPE_CHOICES;

public:
    Model_Attachment();
    ~Model_Attachment();

public:
    /**
    Initialize the global Model_Attachment table on initial call.
    Resets the global table on subsequent calls.
    * Return the static instance address for Model_Attachment table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_Attachment& instance(wxSQLite3Database* db);

    /**
    * Return the static instance address for Model_Attachment table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_Attachment& instance();

public:
	/** Return a dataset with attachments linked to a specific object */
	Data_Set FilterAttachments(const wxString& RefType, const int RefId);

	/** Return the number of attachments linked to a specific object */
	static int NrAttachments(const wxString& RefType, const int RefId);

	/** Return the last attachment file name linked to a specific object */
	static wxString LastAttachmentFileName(const wxString& RefType, const int RefId);

	static wxArrayString all_reftype();
	static wxString reftype_desc(const int& RefTypeEnum);
	static REFTYPE reftype(const Data* attachment);
	static REFTYPE reftype(const Data& attachment);
};

#endif // 
