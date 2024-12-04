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
#include <wx/tglbtn.h>
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
    mmTreeItemCateg(const Model_Category::Data& categData)
        : categData_(categData)
    {}
    Model_Category::Data* getCategData() { return &categData_; }

private:
    Model_Category::Data categData_;
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
        , int64 category_id);

    bool Create(wxWindow* parent
        , wxWindowID id = wxID_ANY
        , const wxString& caption = _("Category Manager")
        , const wxString& name = "Organize Categories"
        , const wxPoint& pos = wxDefaultPosition
        , const wxSize& size = wxDefaultSize
        , long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX | wxRESIZE_BORDER);
    
    int64 getCategId() const;
    bool getRefreshRequested() const;
    bool mmIsUsed() const;
    wxString getFullCategName();

private:
    void CreateControls();
    void fillControls();
    void setTreeSelection(int64 category_id);
    void saveCurrentCollapseState();
    bool AppendSubcategoryItems(wxTreeItemId parentid, const Model_Category::Data* child);
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
    void OnExpandOrCollapseToggle(wxCommandEvent& event);
    void OnShowHiddenToggle(wxCommandEvent& /*event*/);
    void OnTextChanged(wxCommandEvent& event);
    void OnMenuSelected(wxCommandEvent& event);
    void OnItemRightClick(wxTreeEvent& event);
    void OnItemCollapseOrExpand(wxTreeEvent& event);
    void OnBeginDrag(wxTreeEvent& event);
    void OnEndDrag(wxTreeEvent& event);
    bool categShowStatus(int64 categId);
    void setTreeSelection(const wxString& catName, const int64 parentid);
    bool validateName(wxString name);

    mmCategDialogTreeCtrl* m_treeCtrl = nullptr;
    wxSearchCtrl* m_maskTextCtrl = nullptr;
    wxButton* m_buttonAdd = nullptr;
    wxButton* m_buttonEdit = nullptr;
    wxButton* m_buttonSelect = nullptr;
    wxButton* m_buttonDelete = nullptr;
    wxBitmapButton* m_buttonRelocate = nullptr;
    wxToggleButton* m_tbCollapse = nullptr;
    wxToggleButton* m_tbExpand = nullptr;
    wxToggleButton* m_tbShowAll = nullptr;
    wxTreeItemId m_selectedItemId;
    wxTreeItemId root_;
    wxTreeItemId getTreeItemFor(const wxTreeItemId& itemID, const wxString& itemText, const int64 parentid);
    bool m_IsSelection = false;
    int64 m_categ_id = -1;
    int64 m_init_selected_categ_id = -1;
    int64 m_dragSourceCATEGID = 0;
    std::map<int64, bool> m_categoryVisible;
    std::map<int64, std::vector<Model_Category::Data>> m_categ_children;
    bool m_processExpandCollapse = true;
    wxColour NormalColor_;
    bool m_refresh_requested = false;
    wxString m_maskStr;

    enum
    {
        MENU_ITEM_HIDE = wxID_HIGHEST + 1500,
        MENU_ITEM_UNHIDE,
        MENU_ITEM_CLEAR,
        MENU_ITEM_DELETE,
        ID_DIALOG_CATEGORY,
        ID_EXPAND,
        ID_COLLAPSE
    };
};

inline int64 mmCategDialog::getCategId() const { return m_categ_id; }
inline bool mmCategDialog::getRefreshRequested() const { return m_refresh_requested; }

#endif
