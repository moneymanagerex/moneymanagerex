/*******************************************************
 Copyright (C) 2013 Stefano Giorgio

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

#include "payee_table.h"

/************************************************************************************
 TPayeeEntry Methods
 ***********************************************************************************/
/// Constructor used when loading payees from the database
TPayeeEntry::TPayeeEntry(wxSQLite3ResultSet& q1)
: TEntryBase()
{
    id_        = q1.GetInt("PAYEEID");
    name_      = q1.GetString("PAYEENAME");
    cat_id_    = q1.GetInt("CATEGID");
    subcat_id_ = q1.GetInt("SUBCATEGID");
}

/// Constructor for creating a new category entry
TPayeeEntry::TPayeeEntry(const wxString& name)
: TEntryBase()
, name_(name)
, cat_id_(-1)
, subcat_id_(-1)
{}

void TPayeeEntry::SetDatabaseValues(wxSQLite3Statement& st, int& db_index)
{
    st.Bind(++db_index, name_);
    st.Bind(++db_index, cat_id_);
    st.Bind(++db_index, subcat_id_);
}

int TPayeeEntry::Add(wxSQLite3Database* db)
{
    try
    {
        const char SQL_STATEMENT[] =
        "INSERT INTO PAYEE_V1"
        " (PAYEENAME, CATEGID, SUBCATEGID) VALUES(?, ?, ?)";
    
        wxSQLite3Statement st = db->PrepareStatement(SQL_STATEMENT);
        int db_index = 0;
        SetDatabaseValues(st, db_index);
        FinaliseAdd(db, st);
    }
    catch(const wxSQLite3Exception& e)
    {
        wxLogError("TPayeeEntry:Add: %s", e.GetMessage().c_str());
    }

    return id_;
}

void TPayeeEntry::Delete(wxSQLite3Database* db)
{
    DeleteEntry(db, "delete from PAYEE_V1 where PAYEEID = ?");
}

void TPayeeEntry::Update(wxSQLite3Database* db)
{
    try
    {
        const char SQL_STATEMENT[] =
        "UPDATE PAYEE_V1 SET"
        " PAYEENAME = ?, CATEGID = ?, SUBCATEGID = ? "
        "WHERE PAYEEID = ?";
        
        wxSQLite3Statement st = db->PrepareStatement(SQL_STATEMENT);
        int db_index = 0;
        SetDatabaseValues(st, db_index);
        st.Bind(++db_index, id_);

        FinaliseStatement(st);
    }
    catch(const wxSQLite3Exception& e)
    {
        wxLogError("TPayeeEntry:Update: %s", e.GetMessage().c_str());
    }
}

/************************************************************************************
 TPayeeList Methods
 ***********************************************************************************/
/// Constructor
TPayeeList::TPayeeList(wxSQLite3Database* db)
: TListBase(db)
{
    LoadEntries();
}

TPayeeList::~TPayeeList()
{
    DestroyEntryList();
}

void TPayeeList::LoadEntries(bool load_entries)
{
    try
    {
        if (!ListDatabase()->TableExists("PAYEE_V1"))
        {
            const char CREATE_TABLE_PAYEE_V1[]=
            "CREATE TABLE PAYEE_V1(PAYEEID integer primary key, "
            "PAYEENAME TEXT NOT NULL UNIQUE, CATEGID integer, SUBCATEGID integer)";

            ListDatabase()->ExecuteUpdate(CREATE_TABLE_PAYEE_V1);
        }

        if (load_entries)
        {
            LoadEntriesUsing("select * from PAYEE_V1 order by PAYEENAME");
        }
    }
    catch (const wxSQLite3Exception& e)
    {
        wxLogError("TPayeeList:LoadEntries: %s", e.GetMessage().c_str());
    }
}

void TPayeeList::LoadEntriesUsing(const wxString& sql_statement)
{
    DestroyEntryList();
    wxSQLite3ResultSet q1 = ListDatabase()->ExecuteQuery(sql_statement);
    while (q1.NextRow())
    {
        entrylist_.push_back(new TPayeeEntry(q1));
    }
    q1.Finalize();
}

void TPayeeList::DestroyEntryList()
{
    for (size_t i = 0; i < entrylist_.size(); ++i)
    {
        delete entrylist_[i];
    }
    entrylist_.clear();
}

int TPayeeList::AddEntry(const wxString& name, wxString category, wxString subcategory)
{
    int payee_id = -1;
    if (PayeeExists(name))
    {
        payee_id = entrylist_[current_index_]->GetId();
    }
    else
    {
        TPayeeEntry* pEntry = new TPayeeEntry(name);
        entrylist_.push_back(pEntry);
        payee_id = pEntry->Add(ListDatabase());
    }

    return payee_id;
}

void TPayeeList::UpdateEntry(int payee_id, const wxString& new_payee_name, int cat_id, int subcat_id)
{
    //TODO: Associate category_list to get apropriate details.

    TPayeeEntry* pEntry = GetEntryPtr(payee_id);
    pEntry->name_ = new_payee_name;
    if (cat_id > 0)
    {
        pEntry->cat_id_ = cat_id;
    }

    if (subcat_id > 0)
    {
        pEntry->subcat_id_ = subcat_id;
    }
    pEntry->Update(ListDatabase());
}

void TPayeeList::UpdateEntry(const wxString& payee_name, int cat_id, int subcat_id)
{
    TPayeeEntry* pEntry = GetEntryPtr(payee_name);
    pEntry->cat_id_ = cat_id;
    pEntry->subcat_id_ = subcat_id;
    pEntry->Update(ListDatabase());
}

void TPayeeList::DeleteEntry(int payee_id)
{
    TPayeeEntry* pEntry = GetEntryPtr(payee_id);
    if (pEntry)
    {
        pEntry->Delete(ListDatabase());
        entrylist_.erase(entrylist_.begin() + current_index_);
        delete pEntry;
    }
}

void TPayeeList::DeleteEntry(const wxString& payee_name)
{
    DeleteEntry(GetPayeeId(payee_name));
}

//-----------------------------------------------------------------------------

TPayeeEntry* TPayeeList::GetEntryPtr(int payee_id)
{
    TPayeeEntry* pEntry = 0;
    size_t index = 0;

    while (index < entrylist_.size())
    {
        if (entrylist_[index]->GetId() == payee_id)
        {
            pEntry = entrylist_[index];
            current_index_ = index;
            break;
        }
        ++ index;
    }

    return pEntry;
}

TPayeeEntry* TPayeeList::GetEntryPtr(const wxString& name)
{
    TPayeeEntry* pEntry = 0;
    size_t index = 0;

    while (index < entrylist_.size())
    {
        if (entrylist_[index]->name_ == name)
        {
            pEntry = entrylist_[index];
            current_index_ = index;
            break;
        }
        ++ index;
    }

    return pEntry;
}

int TPayeeList::GetPayeeId(const wxString& payee_name)
{
    int payee_id = -1;
    TPayeeEntry* pEntry = GetEntryPtr(payee_name);
    if (pEntry)
    {
        payee_id = pEntry->GetId();
    }

    return payee_id;
}

wxString TPayeeList::GetPayeeName(int payee_id)
{
    wxString payee_name;
    TPayeeEntry* pEntry = GetEntryPtr(payee_id);
    if (pEntry)
    {
        payee_name = pEntry->name_;
    }

    return payee_name;
}

bool TPayeeList::PayeeExists(const wxString& payeeName)
{
    bool payee_result = false;
    if (GetEntryPtr(payeeName))
    {
        payee_result = true;
    }
    return payee_result;
}
