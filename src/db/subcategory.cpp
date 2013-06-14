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

#include "subcategory.h"

/************************************************************************************
 TSubCategoryEntry Methods
 ***********************************************************************************/
TSubCategoryEntry::TSubCategoryEntry(wxSQLite3ResultSet& q1)
: TEntryBase()
{
    id_     = q1.GetInt("SUBCATEGID");
    name_   = q1.GetString("SUBCATEGNAME");
    cat_id_ = q1.GetInt("CATEGID");
}

TSubCategoryEntry::TSubCategoryEntry(int cat_id, const wxString& name)
: TEntryBase()
, cat_id_(cat_id)
, name_(name)
{}

int TSubCategoryEntry::Add(wxSQLite3Database* db)
{
    try
    {
        const char INSERT_INTO_SUBCATEGORY_V1[] =
        "insert into SUBCATEGORY_V1 (SUBCATEGNAME, CATEGID) values(?, ?)";

        wxSQLite3Statement st = db->PrepareStatement(INSERT_INTO_SUBCATEGORY_V1);
        int db_index = 0;
        SetDatabaseValues(st, db_index);
        FinaliseAdd(db, st);
    }
    catch(const wxSQLite3Exception& e)
    {
        wxLogError("TSubCategoryEntry:Add: %s", e.GetMessage().c_str());
    }
    
    return id_;
}

void TSubCategoryEntry::SetDatabaseValues(wxSQLite3Statement& st, int& db_index)
{
    st.Bind(++db_index, name_);
    st.Bind(++db_index, cat_id_);
}

void TSubCategoryEntry::Delete(wxSQLite3Database* db)
{
    DeleteEntry(db, "delete from SUBCATEGORY_V1 where SUBCATEGID = ?");
}

void TSubCategoryEntry::Update(wxSQLite3Database* db)
{
    try
    {
        const char SQL_STATEMENT[] =
        "update SUBCATEGORY_V1 set SUBCATEGNAME = ?, CATEGID = ? "
        "where SUBCATEGID = ?";
        
        wxSQLite3Statement st = db->PrepareStatement(SQL_STATEMENT);
        int db_index = 0;
        SetDatabaseValues(st, db_index);
        st.Bind(++db_index, id_);

        FinaliseStatement(st);
    }
    catch(const wxSQLite3Exception& e)
    {
        wxLogError("TSubCategoryEntry:Update: %s ", e.GetMessage().c_str());
    }
}

/************************************************************************************
 TSubCategoryList Methods
 ***********************************************************************************/
/// Constructor
TSubCategoryList::TSubCategoryList(wxSQLite3Database* db, int cat_id)
: TListBase(db)
{
    LoadEntries(cat_id);
}

TSubCategoryList::~TSubCategoryList()
{
    DestroyEntryList();
}

void TSubCategoryList::LoadEntries(int cat_id, bool load_entries)
{
    try
    {
        if (!ListDatabase()->TableExists("SUBCATEGORY_V1"))
        {
            const char CREATE_TABLE_SUBCATEGORY_V1[]=
            "CREATE TABLE SUBCATEGORY_V1(SUBCATEGID integer primary key, "
            "SUBCATEGNAME TEXT NOT NULL, CATEGID integer NOT NULL)";

            ListDatabase()->ExecuteUpdate(CREATE_TABLE_SUBCATEGORY_V1);
        }

        if (load_entries)
        {
            wxString sql_statement = "select SUBCATEGID, SUBCATEGNAME, CATEGID from SUBCATEGORY_V1";
            if (cat_id > 0)
            {
                sql_statement << " where CATEGID = " << cat_id;
            }

            LoadEntriesUsing(sql_statement);
        }
    }
    catch (const wxSQLite3Exception& e)
    {
        wxLogError("TSubCategoryList:LoadEntries: %s", e.GetMessage().c_str());
    }
}

void TSubCategoryList::LoadEntriesUsing(const wxString& sql_statement)
{
    DestroyEntryList();
    wxSQLite3ResultSet q1 = ListDatabase()->ExecuteQuery(sql_statement);
    while (q1.NextRow())
    {
        entrylist_.push_back(new TSubCategoryEntry(q1));
    }
    q1.Finalize();
}

void TSubCategoryList::DestroyEntryList()
{
    for (size_t i = 0; i < entrylist_.size(); ++i)
    {
        delete entrylist_[i];
    }
    entrylist_.clear();
}

int TSubCategoryList::AddEntry(int cat_id, const wxString& name)
{
    int subcat_id;
    if (SubCategoryExists(cat_id, name))
    {
        subcat_id = entrylist_[current_index_]->GetId();
    }
    else
    {
        TSubCategoryEntry* pEntry = new TSubCategoryEntry(cat_id, name);
        subcat_id = pEntry->Add(ListDatabase());
        entrylist_.push_back(pEntry);
    }

    return subcat_id;
}

void TSubCategoryList::UpdateEntry(int cat_id, int subcat_id, const wxString& new_category)
{
    TSubCategoryEntry* pSubCategory = GetEntryPtr(cat_id, subcat_id);
    pSubCategory->name_ = new_category;
    pSubCategory->Update(ListDatabase());
}

/// Note: At this level, no checking is done for usage in other tables.
void TSubCategoryList::DeleteEntry(int cat_id, int subcat_id)
{
    TSubCategoryEntry* pSubCatEntry = GetEntryPtr(cat_id, subcat_id);
    pSubCatEntry->Delete(ListDatabase());
    entrylist_.erase(entrylist_.begin() + current_index_);
    delete pSubCatEntry;
}

TSubCategoryEntry* TSubCategoryList::GetEntryPtr(int cat_id, int subcat_id)
{
    TSubCategoryEntry* pSubCatEntry = 0;
    size_t index = 0;
    while (index < entrylist_.size())
    {
        if ((entrylist_[index]->GetId() == subcat_id) && (entrylist_[index]->cat_id_ == cat_id))
        {
            pSubCatEntry = entrylist_[index];
            current_index_ = index;
            break;
        }
        ++ index;
    }

    return pSubCatEntry;
}

TSubCategoryEntry* TSubCategoryList::GetEntryPtr(int cat_id, const wxString& name)
{
    TSubCategoryEntry* pSubCatEntry = 0;
    size_t index = 0;
    while (index < entrylist_.size())
    {
        if ((entrylist_[index]->cat_id_ == cat_id) && (entrylist_[index]->name_ == name))
        {
            pSubCatEntry = entrylist_[index];
            current_index_ = index;
            break;
        }
        ++ index;
    }

    return pSubCatEntry;
}

int TSubCategoryList::GetSubCategoryId(int cat_id, const wxString& name)
{
    int subcat_id = -1;
    TSubCategoryEntry* pSubCatEntry = GetEntryPtr(cat_id, name);
    if (pSubCatEntry)
    {
        subcat_id = pSubCatEntry->GetId();
    }

    return subcat_id;
}

wxString TSubCategoryList::GetSubCategoryName(int cat_id, int subcat_id)
{
    wxString subcat_name;
    TSubCategoryEntry* pSubCatEntry = GetEntryPtr(cat_id, subcat_id);
    if (pSubCatEntry)
    {
        subcat_name = pSubCatEntry->name_;
    }

    return subcat_name;
}

bool TSubCategoryList::SubCategoryExists(int cat_id, const wxString& name)
{
    bool cat_exists = false;
    if (GetEntryPtr(cat_id, name))
    {
        cat_exists = true;
    }
    return cat_exists;
}
