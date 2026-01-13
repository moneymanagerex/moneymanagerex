/*******************************************************
 Copyright (C) 2025 Klaus Wich

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 ********************************************************/

#include "toolbardialog.h"


wxIMPLEMENT_DYNAMIC_CLASS(mmToolbarDialog, wxDialog);

mmToolbarDialog::mmToolbarDialog()
{
}

mmToolbarDialog::~mmToolbarDialog()
{
}

mmToolbarDialog::mmToolbarDialog(wxWindow* parent):genericTreeListDialog(parent, _t("Toolbar configuration"))
{
}

/*void mmToolbarDialog::createColumns() {
    genericTreeListDialog::createColumns();  // Aufruf der Basisklassen-Methode
}

void mmToolbarDialog::closeAction() {
    genericTreeListDialog::closeAction();
};

void mmToolbarDialog::createMiddleElements() {
    genericTreeListDialog::createMiddleElements();  // Aufruf der Basisklassen-Methode
}

void mmToolbarDialog::createBottomElements() {
    genericTreeListDialog::createBottomElements();  // Aufruf der Basisklassen-Methode
}

void mmToolbarDialog::updateControlState(int selIdx, wxClientData* selData) {
    genericTreeListDialog::updateControlState(selIdx, selData);
};

void mmToolbarDialog::setDefault() {
    genericTreeListDialog::setDefault();
}*/

void mmToolbarDialog::fillControls(wxTreeListItem root)
{
    wxTreeListItem item;
    for (int i = 0; i < 10; i++) {
        item = m_treeList->AppendItem(root, wxString::Format("%d",i));
    }
}