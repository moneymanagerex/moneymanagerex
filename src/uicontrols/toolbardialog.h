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

#include <wx/wx.h>
#include <wx/treelist.h>
#include <wx/sizer.h>
#include <vector>

#include "daterange2.h"
#include "generic/generictreelistdialog.h"


class mmToolbarDialog: public genericTreeListDialog
{
private:
    wxDECLARE_DYNAMIC_CLASS(mmToolbarDialog);
    //wxDECLARE_EVENT_TABLE();

public:
    mmToolbarDialog();
    ~mmToolbarDialog() override;
    mmToolbarDialog(wxWindow* parent);

private:
    void fillControls(wxTreeListItem root);
    /*
    void createColumns();
    void closeAction();
    void createMiddleElements();
    void createBottomElements();

    void updateControlState(int selIdx, wxClientData* selData);
    void setDefault();
    */
};
