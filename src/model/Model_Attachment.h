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

#include "choices.h"
#include "db/DB_Table_Attachment_V1.h"
#include "Model.h"

class Model_Attachment : public Model<DB_Table_ATTACHMENT_V1>
{
public:
    using Model<DB_Table_ATTACHMENT_V1>::get;
    enum REFTYPE_ID {
        REFTYPE_ID_TRANSACTION = 0,
        REFTYPE_ID_STOCK,
        REFTYPE_ID_ASSET,
        REFTYPE_ID_BANKACCOUNT,
        REFTYPE_ID_BILLSDEPOSIT,
        REFTYPE_ID_PAYEE,
        REFTYPE_ID_TRANSACTIONSPLIT,
        REFTYPE_ID_BILLSDEPOSITSPLIT,
        REFTYPE_ID_size
    };
    static const wxString REFTYPE_NAME_TRANSACTION;
    static const wxString REFTYPE_NAME_STOCK;
    static const wxString REFTYPE_NAME_ASSET;
    static const wxString REFTYPE_NAME_BANKACCOUNT;
    static const wxString REFTYPE_NAME_BILLSDEPOSIT;
    static const wxString REFTYPE_NAME_PAYEE;
    static const wxString REFTYPE_NAME_TRANSACTIONSPLIT;
    static const wxString REFTYPE_NAME_BILLSDEPOSITSPLIT;

    static ChoicesName REFTYPE_CHOICES;
    static const wxString reftype_name(int id);
    static int reftype_id(const wxString& name, int default_id = -1);

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
    const Data_Set FilterAttachments(const wxString& RefType, const int64 RefId);

    /** Return the number of attachments linked to a specific object */
    static int NrAttachments(const wxString& RefType, const int64 RefId);

    /** Return the last attachment number linked to a specific object */
    static int LastAttachmentNumber(const wxString& RefType, const int64 RefId);

    /** Return a dataset with attachments linked to a specific type*/
    std::map<int64, Data_Set> get_all(REFTYPE_ID reftype);

    /** Return all attachments descriptions*/
    wxArrayString allDescriptions();
};

//----------------------------------------------------------------------------

inline const wxString Model_Attachment::reftype_name(int id)
{
    return REFTYPE_CHOICES.getName(id);
}

inline int Model_Attachment::reftype_id(const wxString& name, int default_id)
{
    return REFTYPE_CHOICES.findName(name, default_id);
}

#endif
