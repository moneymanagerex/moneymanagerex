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

#ifndef _MM_EX_MMCATEGORY_H_
#define _MM_EX_MMCATEGORY_H_
#include <memory>

#include "defs.h"

#include <vector>

class mmCoreDB;

class mmCategory
{
public:
    mmCategory(int id, const wxString& name)
        : categID_(id)
        , categName_(name)
    {}

    /* Public Data */
    int categID_;
    wxString categName_;
    std::vector<mmCategory* > children_;
};

class mmCategoryList
{
public:
    mmCategoryList(mmCoreDB* core)
        : core_(core)
    {}

    /* Category Functions */
    int GetCategoryId(const wxString& categoryName) const;
    wxString GetCategoryName(int categ_id) const;
    wxString GetCategoryString(int categ_id) const;
    wxString GetSubCategoryString(int categID, int subCategID) const;
    wxString GetSubCategoryName(int categID, int subCategID) const;
    wxString GetFullCategoryString(int categID, int subCategID) const;
    void parseCategoryString(wxString categ, wxString& cat, int& categID, wxString& subcat, int& subCategID);

    bool CategoryExists(const wxString& categoryName) const;
    bool GetCategoryLikeString(wxString& str, int& categID, int& subcategID);

    mmCategory* GetCategorySharedPtr(int category, int subcategory) const;
    int GetSubCategoryID(int parentID, const wxString& subCategoryName) const;

    void LoadCategories();
    int AddCategory(const wxString& category);
    int AddSubCategory(int parentID, const wxString& text);

    bool DeleteCategory(int categID);
    bool DeleteSubCategory(int categID, int subCategID);
    bool UpdateCategory(int categID, int subCategID, const wxString& text);

    /* Public Data */
    std::vector< mmCategory* > entries_;
    //typedef std::vector< mmCategory* >::const_iterator const_iterator;

private:
    mmCoreDB* core_;
};
#endif
