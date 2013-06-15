/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2011 Stefano Giorgio

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

#include "mmcategory.h"
#include "dbwrapper.h"
#include "util.h"
#include "mmOption.h"
#include "mmcoredb.h"

void mmCategoryList::LoadCategories()
{
    entries_.clear();
    wxSQLite3ResultSet q1 = core_->db_.get()->ExecuteQuery(SELECT_ALL_CATEGORIES);
    wxSQLite3ResultSet q2 = core_->db_.get()->ExecuteQuery(SELECT_ALL_SUBCATEGORIES);

    mmCategory* pCat;
    while (q1.NextRow())
    {
        int catID = q1.GetInt("CATEGID");
        pCat = new mmCategory(catID, q1.GetString("CATEGNAME"));
        entries_.push_back(pCat);

        while (q2.NextRow())
        {
            int scatID = q2.GetInt("CATEGID");
            if (catID == scatID)
            {
                int subCategID = q2.GetInt("SUBCATEGID");
                wxString subCategName = q2.GetString("SUBCATEGNAME");

                mmCategory* pSubCat(new mmCategory(subCategID, subCategName));
                pSubCat->parent_ = pCat;
                pCat->children_.push_back(pSubCat);
            }
        }
    }

    q2.Finalize();
    q1.Finalize();
}

mmCategory* mmCategoryList::GetCategorySharedPtr(int category, int subcategory) const
{
    if (category != -1)
    {
        for (const auto& entry : entries_)
        {
            if (entry->categID_ == category)
            {
                if (subcategory == -1)
                    return entry;

                size_t numSubCategory = entry->children_.size();
                for (size_t idxS = 0; idxS < numSubCategory; ++idxS)
                {
                    if (entry->children_[idxS]->categID_ == subcategory)
                    {
                        return entry->children_[idxS];
                    }
                }
            }
        }
    }
    return NULL;
}

bool mmCategoryList::CategoryExists(const wxString& categoryName) const
{
    for (const auto& it : entries_)
    {
        if (! it->categName_.CmpNoCase(categoryName))
            return true;
    }
    return false;
}

int mmCategoryList::GetCategoryId(const wxString& categoryName) const
{
    for (const auto& it : entries_)
    {
        if (it->categName_ == categoryName)
            return it->categID_;
    }

    return -1;
}

wxString mmCategoryList::GetCategoryName(int categ_id) const
{
    for (const auto& it : entries_)
    {
        if (it->categID_ == categ_id) return it->categName_;
    }

    return "";
}

wxString mmCategoryList::GetSubCategoryName(int categID, int subCategID) const
{
    for (const auto& category : entries_)
    {
        if (category->categID_ == categID)
        {
            for (const auto&  sub_category : category->children_)
            {
                if (subCategID == sub_category->categID_)
                    return sub_category->categName_;
            }
        }
    }
    return "";
}

int mmCategoryList::GetSubCategoryID(int parentID, const wxString& subCategoryName) const
{
    for (const auto& category : entries_)
    {
        if (category->categID_ == parentID)
        {
            for (const auto& sub_category : category->children_)
            {
                if (subCategoryName == sub_category->categName_)
                    return sub_category->categID_;
            }
        }
    }
    return -1;
}

wxString mmCategoryList::GetCategoryString(int categ_id) const
{
    wxString catName = this->GetCategoryName(categ_id);
    catName.Replace ("&", "&&");

    return catName;
}

wxString mmCategoryList::GetSubCategoryString(int categID, int subCategID) const
{
    wxString subcatName = GetSubCategoryName(categID, subCategID);
    subcatName.Replace ("&", "&&");
    return subcatName;
}

wxString mmCategoryList::GetFullCategoryString(int categID, int subCategID) const
{

    if (categID > -1)
    {
        wxString category    = GetCategoryString(categID);
        if (subCategID > -1)
            category << ":" << GetSubCategoryString(categID, subCategID);
        return category;
    }
    else
        return _("Select Category");
}

void mmCategoryList::parseCategoryString(wxString categ, wxString& cat, int& categID, wxString& subcat, int& subCategID)
{
    wxStringTokenizer cattkz(categ, ":");

    cat = cattkz.GetNextToken();
    if (cattkz.HasMoreTokens())
        subcat = cattkz.GetNextToken();
    else
        subcat = "";

    categID = GetCategoryId(cat);

    if (!subcat.IsEmpty() && categID != -1)
        subCategID = GetSubCategoryID(categID, subcat);
    else
        subCategID = -1;
}

bool mmCategoryList::GetCategoryLikeString(wxString& str, int& categID, int& subcategID)
{
    bool found = false;
    for (const auto& category: core_->categoryList_.entries_)
    {
        if (category->categName_.Upper().StartsWith(str))
        {
            categID = category->categID_;
            subcategID = -1;
            found = true;
            break;
        }
        for (const auto& sub_category: category->children_)
        {
            if (sub_category->categName_.Upper().StartsWith(str))
            {
                categID = category->categID_;
                subcategID = sub_category->categID_;
                found = true;
                break;
            }
        }
    }
    return found;
}

int mmCategoryList::AddCategory(const wxString& category)
{
    if (category.IsEmpty()) return -1;
    int cID = -1;

    mmDBWrapper::addCategory(core_->db_.get(), category);
    cID = (core_->db_.get()->GetLastRowId()).ToLong();

    LoadCategories();
    mmOptions::instance().databaseUpdated_ = true;

    return cID;
}

int mmCategoryList::AddSubCategory(int parentID, const wxString& text)
{
    if (text.IsEmpty() || parentID < 0) return -1;

    mmDBWrapper::addSubCategory(core_->db_.get(), parentID, text);
    int cID = (core_->db_.get()->GetLastRowId()).ToLong();

    LoadCategories();
    mmOptions::instance().databaseUpdated_ = true;

    return cID;
}

bool mmCategoryList::DeleteCategory(int categID)
{
    bool bResult = (mmDBWrapper::deleteCategoryWithConstraints(core_->db_.get(), categID));

    LoadCategories();
    mmOptions::instance().databaseUpdated_ = true;

    return bResult;
}

bool mmCategoryList::DeleteSubCategory(int categID, int subCategID)
{
    bool bResult = (mmDBWrapper::deleteSubCategoryWithConstraints(core_->db_.get(), categID, subCategID));

    LoadCategories();
    mmOptions::instance().databaseUpdated_ = true;

    return bResult;
}

bool mmCategoryList::UpdateCategory(int categID, int subCategID, const wxString& text)
{

    bool bResult = mmDBWrapper::updateCategory(core_->db_.get(), categID, subCategID, text);
    LoadCategories();
    mmOptions::instance().databaseUpdated_ = true;

    return bResult;
}
