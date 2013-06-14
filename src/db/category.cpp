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

#include "category.h"

/************************************************************************************
 TCategoryEntry Methods
 ***********************************************************************************/
/// Constructor used when loading categories from the database
TCategoryEntry::TCategoryEntry(wxSQLite3ResultSet& q1)
: TEntryBase()
{
    id_   = q1.GetInt("CATEGID");
    name_ = q1.GetString("CATEGNAME");
}

/// Constructor for creating a new category entry
TCategoryEntry::TCategoryEntry(const wxString& name)
: TEntryBase()
, name_(name)
{}

int TCategoryEntry::Add(wxSQLite3Database* db)
{
    try
    {
        const char SQL_STATEMENT[] =
        "INSERT INTO CATEGORY_V1 (CATEGNAME) VALUES(?)";

        wxSQLite3Statement st = db->PrepareStatement(SQL_STATEMENT);
        st.Bind(1, name_);

        FinaliseAdd(db, st);
    }
    catch(const wxSQLite3Exception& e)
    {
        wxLogError("TCategoryEntry:Add: %s", e.GetMessage().c_str());
    }

    return id_;
}

void TCategoryEntry::Delete(wxSQLite3Database* db)
{
    DeleteEntry(db, "delete from CATEGORY_V1 where CATEGID = ?");
}

void TCategoryEntry::Update(wxSQLite3Database* db)
{
    try
    {
        const char SQL_STATEMENT[] =
        "UPDATE CATEGORY_V1 SET CATEGNAME = ? "
        "WHERE CATEGID = ?";

        wxSQLite3Statement st = db->PrepareStatement(SQL_STATEMENT);
        st.Bind(1, name_);
        st.Bind(2, id_);

        FinaliseStatement(st);
    }
    catch(const wxSQLite3Exception& e)
    {
        wxLogError("TCategoryEntry:Update: %s", e.GetMessage().c_str());
    }
}

/************************************************************************************
 TCategoryList Methods
 ***********************************************************************************/
/// Constructor
TCategoryList::TCategoryList(wxSQLite3Database* db)
: TListBase(db)
{
    LoadEntries();
}

TCategoryList::~TCategoryList()
{
    DestroyEntryList();
}

void TCategoryList::LoadEntries(bool load_entries)
{
    try
    {
        if (!ListDatabase()->TableExists("CATEGORY_V1"))
        {
            const char CREATE_TABLE_CATEGORY_V1[]=
            "CREATE TABLE CATEGORY_V1(CATEGID integer primary key, "
            "CATEGNAME TEXT NOT NULL)";

            ListDatabase()->ExecuteUpdate(CREATE_TABLE_CATEGORY_V1);
        }

        if (load_entries)
        {
            LoadEntriesUsing("select CATEGID, CATEGNAME from CATEGORY_V1 order by CATEGNAME");
        }
    }
    catch (const wxSQLite3Exception& e)
    {
        wxLogError("TCategoryList:LoadEntries: %s", e.GetMessage().c_str());
    }
}

// Allows specialised loads by providing the required SQL statement
void TCategoryList::LoadEntriesUsing(const wxString& sql_statement)
{
    DestroyEntryList();
    wxSQLite3ResultSet q1 = ListDatabase()->ExecuteQuery(sql_statement);
    while (q1.NextRow())
    {
        entrylist_.push_back(new TCategoryEntry(q1));
    }
    q1.Finalize();
}
// delete all the objects in the list and clear the list.
void TCategoryList::DestroyEntryList()
{
    for (size_t i = 0; i < entrylist_.size(); ++i)
    {
        delete entrylist_[i];
    }
    entrylist_.clear();
}

int TCategoryList::AddEntry(const wxString& name)
{
    int cat_id = -1;
    if (CategoryExists(name))
    {
        cat_id = entrylist_[current_index_]->id_;
    }
    else
    {
        TCategoryEntry* pEntry = new TCategoryEntry(name);
        cat_id = pEntry->Add(ListDatabase());
        entrylist_.push_back(pEntry);
    }

    return cat_id;
}

void TCategoryList::UpdateEntry(int cat_id, const wxString& new_category)
{
    TCategoryEntry* pEntry = GetEntryPtr(cat_id);
    pEntry->name_ = new_category;
    pEntry->Update(ListDatabase());
}

/// Note: At this level, no checking is done for usage in other tables.
void TCategoryList::DeleteEntry(int cat_id)
{
    TCategoryEntry* pEntry = GetEntryPtr(cat_id);
    if (pEntry)
    {
        pEntry->Delete(ListDatabase());
        entrylist_.erase(entrylist_.begin() + current_index_);
        delete pEntry;
    }
}

//-----------------------------------------------------------------------------

TCategoryEntry* TCategoryList::GetEntryPtr(int cat_id)
{
    TCategoryEntry* pEntry = 0;
    size_t index = 0;

    while (index < entrylist_.size())
    {
        if (entrylist_[index]->id_ == cat_id)
        {
            pEntry = entrylist_[index];
            current_index_ = index;
            break;
        }
        ++ index;
    }

    return pEntry;
}

TCategoryEntry* TCategoryList::GetEntryPtr(const wxString& name)
{
    TCategoryEntry* pEntry = 0;
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

int TCategoryList::GetCategoryId(const wxString& name)
{
    int cat_id = -1;
    TCategoryEntry* pEntry = GetEntryPtr(name);
    if (pEntry)
    {
        cat_id = pEntry->GetId();
    }

    return cat_id;
}

wxString TCategoryList::GetCategoryName(int cat_id)
{
    wxString cat_name;
    TCategoryEntry* pEntry = GetEntryPtr(cat_id);
    if (pEntry)
    {
        cat_name = pEntry->name_;
    }

    return cat_name;
}

bool TCategoryList::CategoryExists(const wxString& name)
{
    bool cat_exists = false;
    if (GetEntryPtr(name))
    {
        cat_exists = true;
    }

    return cat_exists;
}
