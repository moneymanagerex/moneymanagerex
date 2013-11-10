/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel

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

#ifndef _MM_EX_CATEGDIALOG_H_
#define _MM_EX_CATEGDIALOG_H_

#define SYMBOL_CATEGDIALOG_STYLE wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX|wxRESIZE_BORDER
#define SYMBOL_CATEGDIALOG_TITLE _("Organize Categories")
#define SYMBOL_CATEGDIALOG_IDNAME ID_DIALOG_CATEGORY
#define SYMBOL_CATEGDIALOG_SIZE wxSize(500, 300)
#define SYMBOL_CATEGDIALOG_POSITION wxDefaultPosition

#include "guiid.h"
#include "defs.h"
#include "model/Model_Category.h"

class mmTreeItemCateg : public wxTreeItemData
{
public:
    mmTreeItemCateg(const Model_Category::Data& categData, const Model_Subcategory::Data& subcategData)
        : categData_(categData),
          subcategData_(subcategData) { }
    Model_Category::Data* getCategData() { return &categData_; }
    Model_Subcategory::Data* getSubCategData() { return &subcategData_; }

private:
    Model_Category::Data categData_;
    Model_Subcategory::Data subcategData_;
};

class mmCategDialog : public wxDialog
{
    DECLARE_DYNAMIC_CLASS( mmCategDialog )
    DECLARE_EVENT_TABLE()

public:
    mmCategDialog();
    mmCategDialog(wxWindow* parent,
                  bool bEnableSelect = true, bool bEnableRelocate = true,
                  wxWindowID id = SYMBOL_CATEGDIALOG_IDNAME,
                  const wxString& caption = SYMBOL_CATEGDIALOG_TITLE,
                  const wxPoint& pos = SYMBOL_CATEGDIALOG_POSITION,
                  const wxSize& size = SYMBOL_CATEGDIALOG_SIZE,
                  long style = SYMBOL_CATEGDIALOG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CATEGDIALOG_IDNAME,
                 const wxString& caption = SYMBOL_CATEGDIALOG_TITLE,
                 const wxPoint& pos = SYMBOL_CATEGDIALOG_POSITION,
                 const wxSize& size = SYMBOL_CATEGDIALOG_SIZE,
                 long style = SYMBOL_CATEGDIALOG_STYLE );

    void setTreeSelection(const wxString& catName, const wxString& subCatName);
    void setTreeSelection(int &category_id, int &subcategory_id);
    int getCategId()
    {
        return categID_;
    }
    int getSubCategId()
    {
        return subcategID_;
    }
    bool getExpandStatus()
    {
        return cbExpand_->IsChecked();
    }
    bool getRefreshRequested()
    {
        return refreshRequested_;
    }
    wxString getFullCategName();

private:
    void CreateControls();

    // utility functions
    void OnOk(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnAdd(wxCommandEvent& event);
    void OnDelete(wxCommandEvent& event);
    void OnBSelect(wxCommandEvent& event);
    void OnEdit(wxCommandEvent& event);

    void fillControls();
    void OnSelChanged(wxTreeEvent& event);
    void OnDoubleClicked(wxTreeEvent& event);

    wxTreeCtrl* treeCtrl_;
    wxTreeItemId selectedItemId_;
    wxTextCtrl* textCtrl_;
    wxTreeItemId root_;
    wxTreeItemId getTreeItemFor(wxTreeItemId itemID, const wxString& itemText);
    wxButton* addButton_;
    wxButton* editButton_;
    wxButton* selectButton_;
    wxButton* deleteButton_;
    wxBitmapButton* btnCateg_relocate_;
    wxCheckBox* cbExpand_;
    wxCheckBox* cbShowAll_;

    void showCategDialogDeleteError(wxString deleteCategoryErrMsg, bool category = true);
    void OnCategoryRelocation(wxCommandEvent& /*event*/);
    void OnExpandChbClick(wxCommandEvent& /*event*/);
    void OnShowHiddenChbClick (wxCommandEvent& /*event*/);
    void OnMenuSelected(wxCommandEvent& event);
    void OnItemRightClick(wxTreeEvent& event);
    bool categShowStatus(int categId, int subCategId);

    bool bEnableSelect_;
    bool bEnableRelocate_;
    int categID_;
    int subcategID_;
    wxColour NormalColor_;
    wxArrayString hidden_categs_;
    bool refreshRequested_;
};

#endif
