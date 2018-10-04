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

#ifndef MM_EX_CATEGDIALOG_H_
#define MM_EX_CATEGDIALOG_H_


#include "defs.h"
#include "Model_Category.h"

class mmTreeItemCateg : public wxTreeItemData
{
public:
    mmTreeItemCateg(const Model_Category::Data& categData, const Model_Subcategory::Data& subcategData)
        : categData_(categData)
        , subcategData_(subcategData)
    {}
    Model_Category::Data* getCategData() { return &categData_; }
    Model_Subcategory::Data* getSubCategData() { return &subcategData_; }

private:
    Model_Category::Data categData_;
    Model_Subcategory::Data subcategData_;
};

class mmCategDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmCategDialog);
    wxDECLARE_EVENT_TABLE();

public:
    mmCategDialog();
    mmCategDialog(wxWindow* parent
        , int category_id = -1, int subcategory_id = -1
        , bool bEnableRelocate = true
        , bool bEnableSelect = true);

    bool Create(wxWindow* parent
        , wxWindowID id
        , const wxString& caption
        , const wxPoint& pos
        , const wxSize& size
        , long style);

    int getCategId()
    {
        return m_categ_id;
    }
    int getSubCategId()
    {
        return m_subcateg_id;
    }
    bool getRefreshRequested()
    {
        return m_refresh_requested;
    }
    wxString getFullCategName();

private:
    void CreateControls();
    void fillControls();
    void setTreeSelection(int category_id, int subcategory_id);

    void OnOk(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnAdd(wxCommandEvent& event);
    void OnDelete(wxCommandEvent& event);
    void OnBSelect(wxCommandEvent& event);
    void OnEdit(wxCommandEvent& event);
    void OnSelChanged(wxTreeEvent& event);
    void OnDoubleClicked(wxTreeEvent& event);
    void showCategDialogDeleteError(bool category = true);
    void OnCategoryRelocation(wxCommandEvent& WXUNUSED(event));
    void OnExpandChbClick(wxCommandEvent& WXUNUSED(event));
    void OnShowHiddenChbClick(wxCommandEvent& WXUNUSED(event));
    void OnMenuSelected(wxCommandEvent& event);
    void OnItemRightClick(wxTreeEvent& event);
    bool categShowStatus(int categId, int subCategId);
    void setTreeSelection(const wxString& catName, const wxString& subCatName);

    wxTreeCtrl* m_treeCtrl;
    wxButton* m_buttonAdd;
    wxButton* m_buttonEdit;
    wxButton* m_buttonSelect;
    wxButton* m_buttonDelete;
    wxBitmapButton* m_buttonRelocate;
    wxCheckBox* m_cbExpand;
    wxCheckBox* m_cbShowAll;
    wxTreeItemId selectedItemId_;
    wxTreeItemId root_;
    wxTreeItemId getTreeItemFor(const wxTreeItemId& itemID, const wxString& itemText);
    bool m_enable_select;
    bool m_enable_relocate;
    int m_categ_id;
    int m_subcateg_id;
    int m_init_selected_categ_id;
    int m_init_selected_subcateg_id;
    wxColour NormalColor_;
    wxArrayString m_hidden_categs;
    bool m_refresh_requested;

    enum
    {
        MENU_ITEM_HIDE = wxID_HIGHEST + 1500,
        MENU_ITEM_UNHIDE,
        MENU_ITEM_CLEAR,
        ID_DIALOG_CATEGORY
    };
};

#endif
