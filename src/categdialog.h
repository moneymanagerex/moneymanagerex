/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2015, 2016, 2020, 2022 Nikolay Akimov
 Copyright (C) 2022 Mark Whalleuy (mark@ipx.co.uk)

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
#include <wx/srchctrl.h>
#include "model/Model_Category.h"

class mmCategDialogTreeCtrl : public wxTreeCtrl
{
    wxDECLARE_DYNAMIC_CLASS(mmCategDialogTreeCtrl);
public:
    mmCategDialogTreeCtrl() { };
    mmCategDialogTreeCtrl(wxWindow *parent, const wxWindowID id,
                       const wxPoint& pos, const wxSize& size,
                       long style=wxTR_DEFAULT_STYLE);
protected:
    int OnCompareItems(const wxTreeItemId& item1, const wxTreeItemId& item2);	
};

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
    ~mmCategDialog();
    mmCategDialog();
    mmCategDialog(wxWindow* parent
        , bool bIsSelection
        , int category_id, int subcategory_id);

    bool Create(wxWindow* parent
        , wxWindowID id = wxID_ANY
        , const wxString& caption = _("Organize Categories")
        , const wxString& name = "Organize Categories"
        , const wxPoint& pos = wxDefaultPosition
        , const wxSize& size = wxDefaultSize
        , long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX | wxRESIZE_BORDER);

    int getCategId() const;
    int getSubCategId() const;
    bool getRefreshRequested() const;
    bool mmIsUsed() const;
    wxString getFullCategName();

private:
    void CreateControls();
    void fillControls();
    void setTreeSelection(int category_id, int subcategory_id);

    void OnOk(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnAdd(wxCommandEvent& event);
    void OnDelete(wxCommandEvent& event);
    void mmDoDeleteSelectedCategory();
    void OnBSelect(wxCommandEvent& event);
    void OnEdit(wxCommandEvent& event);
    void OnSelChanged(wxTreeEvent& event);
    void OnDoubleClicked(wxTreeEvent& event);
    void showCategDialogDeleteError(bool category = true);
    void OnCategoryRelocation(wxCommandEvent& /*event*/);
    void OnExpandChbClick(wxCommandEvent& /*event*/);
    void OnShowHiddenChbClick(wxCommandEvent& /*event*/);
    void OnTextChanged(wxCommandEvent& event);
    void OnMenuSelected(wxCommandEvent& event);
    void OnItemRightClick(wxTreeEvent& event);
    bool categShowStatus(int categId, int subCategId);
    void setTreeSelection(const wxString& catName, const wxString& subCatName);
    bool validateName(wxString name);

    mmCategDialogTreeCtrl* m_treeCtrl;
    wxSearchCtrl* m_maskTextCtrl;
    wxButton* m_buttonAdd;
    wxButton* m_buttonEdit;
    wxButton* m_buttonSelect;
    wxButton* m_buttonDelete;
    wxBitmapButton* m_buttonRelocate;
    wxCheckBox* m_cbExpand;
    wxCheckBox* m_cbShowAll;
    wxTreeItemId m_selectedItemId;
    wxTreeItemId root_;
    wxTreeItemId getTreeItemFor(const wxTreeItemId& itemID, const wxString& itemText);
    bool m_IsSelection;
    int m_categ_id;
    int m_subcateg_id;
    int m_init_selected_categ_id;
    int m_init_selected_subcateg_id;
    wxColour NormalColor_;
    wxArrayString m_hidden_categs;
    bool m_refresh_requested;
    wxString m_maskStr;

    enum
    {
        MENU_ITEM_HIDE = wxID_HIGHEST + 1500,
        MENU_ITEM_UNHIDE,
        MENU_ITEM_CLEAR,
        MENU_ITEM_DELETE,
        ID_DIALOG_CATEGORY
    };
};

inline int mmCategDialog::getCategId() const { return m_categ_id; }
inline int mmCategDialog::getSubCategId() const { return m_subcateg_id; }
inline bool mmCategDialog::getRefreshRequested() const { return m_refresh_requested; }

#endif
