/*******************************************************
 Copyright (C) 2025 Klaus Wich

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

#pragma once

#include "mmSimpleDialogs.h"
#include "mmcheckingpanel.h"
#include "model/Model_Account.h"
#include "model/Model_Checking.h"
#include "generic/genericFocusButton.h"
#include "generic/genericFocusBitmapButton.h"


class mmReconcileDialog: public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmReconcileDialog);

public:
    mmReconcileDialog();
    ~mmReconcileDialog();
    mmReconcileDialog(wxWindow* parent, Model_Account::Data* account, mmCheckingPanel* cp);

private:
    mmTextCtrl*           m_amountCtrl;
    wxStaticText*         m_previousCtrl;
    wxStaticText*         m_clearedBalanceCtrl;
    wxStaticText*         m_endingCtrl;
    wxStaticText*         m_differenceLabel;
    wxStaticText*         m_differenceCtrl;
    wxListCtrl*           m_listLeft;
    wxListCtrl*           m_listRight;
    genFocusButton*       m_btnCancel;
    genFocusButton*       m_btnReconcile;
    genFocusButton*       m_btnReconcileLater;
    genFocusBitmapButton* m_btnCalc;
    genFocusButton*       m_btnEdit;
    std::vector<int64>    m_itemDataMap;
    wxVector<wxBitmapBundle> m_images;

    mmCalculatorPopup*    m_calculaterPopup;
    Model_Account::Data*  m_account;
    mmCheckingPanel*      m_checkingPanel;
    double                m_reconciledBalance;
    bool                  m_ignore;

    void CreateControls();
    void UpdateAll();
    void FillControls();

    void OnCalculator(wxCommandEvent& event);
    void OnAmountChanged(wxCommandEvent& event);

    void OnToggle(wxCommandEvent& event);
    void OnClose(wxCommandEvent& event);
    void OnEdit(wxCommandEvent& event);
    void OnNew(wxCommandEvent& event);

    void OnLeftItemLeftClick(wxMouseEvent& event);
    void OnLeftItemRightClick(wxMouseEvent& event);
    void OnRightItemLeftClick(wxMouseEvent& event);
    void OnRightItemRightClick(wxMouseEvent& event);

    void OnListKeyDown(wxKeyEvent& event);

    void OnLeftFocus(wxFocusEvent& event);
    void OnRightFocus(wxFocusEvent& event);
    void handleListFocus(wxListCtrl* list);

    void OnLeftFocusKill(wxFocusEvent& event);
    void OnRightFocusKill(wxFocusEvent& event);
    void handleListFocusKill(wxListCtrl* list);

    void setListItemData(const Model_Checking::Data* trx, wxListCtrl* list, long item);
    void processRightClick(wxListCtrl* list, long item);
    void processLeftClick(wxListCtrl* list, wxPoint pt);
    void addTransaction2List(const Model_Checking::Data* trx);
    long getListIndexByDate(const Model_Checking::Data* trx, wxListCtrl* list);
    void moveItemData(wxListCtrl* list, int row1, int row2);
    void resetListSelections(wxListCtrl* list);
    void newTransaction();
    void editTransaction(wxListCtrl* list, long item);

    void OnSize(wxSizeEvent& event);
    bool isListItemChecked(wxListCtrl* list, long item);
};
