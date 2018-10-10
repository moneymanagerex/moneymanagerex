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
#include "Table_Attachment.h"

class Model_Attachment : public Model<DB_Table_ATTACHMENT>
{
public:
    using Model<DB_Table_ATTACHMENT>::get;
    enum REFTYPE { TRANSACTION = 0, STOCK, ASSET, BANKACCOUNT, BILLSDEPOSIT, PAYEE};

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
    /** Return all attachments references */
    wxArrayString all_type();

    /** Return a dataset with attachments linked to a specific object */
    const Data_Set FilterAttachments(const wxString& RefType, const int RefId);

    /** Return the number of attachments linked to a specific object */
    static int NrAttachments(const wxString& RefType, const int RefId);

    /** Return the last attachment number linked to a specific object */
    static int LastAttachmentNumber(const wxString& RefType, const int RefId);

    /** Return the description of the choice reftype */
    static wxString reftype_desc(const int RefTypeEnum);

    /** Return a dataset with attachments linked to a specific type*/
    std::map<int, Data_Set> get_all(REFTYPE reftype);

    /** Return all attachments descriptions*/
    wxArrayString allDescriptions();
};

#endif // 
