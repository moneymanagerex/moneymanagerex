/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2013-2022 Nikolay Akimov
 Copyright (C) 2021,2024 Mark Whalley (mark@ipx.co.uk)

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

#include "defs.h"
#include "primitive.h"
#include "option.h"
#include "reports/reportbase.h"

class mmTreeItemData : public wxTreeItemData
{
public:
    enum {
        HOME_PAGE,
        HELP_PAGE_MAIN,
        HELP_PAGE_STOCKS,
        HELP_PAGE_GRM,
        HELP_BUDGET,
        HELP_REPORT,
        CHECKING,
        BUDGET,
        STOCK,
        REPORT,
        GRM,
        ASSETS,
        BILLS,
        FILTER,
        FILTER_REPORT,
        MENU_REPORT,
        DO_NOTHING
    };

private:
    int type_;
    int64 id_ = -1;
    wxString stringData_;
    wxSharedPtr<mmPrintableBase> report_;

public:
    mmTreeItemData(int type, int64 id);
    mmTreeItemData(int type, const wxString& data);
    mmTreeItemData(int type, int64 id, const wxString& data);
    mmTreeItemData(const wxString& data, mmPrintableBase* report);
    mmTreeItemData(mmPrintableBase* report, const wxString& data);
    
    ~mmTreeItemData() {}

    int getType() const;
    int64 getId() const;
    const wxString getString() const;
    mmPrintableBase* getReport() const;
    bool isReadOnly() const;
};

inline int mmTreeItemData::getType() const { return type_; }
inline int64 mmTreeItemData::getId() const { return id_; }
inline const wxString mmTreeItemData::getString() const { return stringData_; }
inline mmPrintableBase* mmTreeItemData::getReport() const { return report_.get(); }

inline bool operator==(const mmTreeItemData& lhs, const mmTreeItemData& rhs)
{
    return (
        lhs.getType()   == rhs.getType() &&
        lhs.getId()     == rhs.getId() &&
        lhs.getString() == rhs.getString()
    );
};

