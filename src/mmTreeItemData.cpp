/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2013-2022 Nikolay Akimov
 Copyright (C) 2021-2024 Mark Whalley (mark@ipx.co.uk)

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

#ifdef _MSC_VER
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"crypt32.lib")
#pragma comment(lib,"wldap32.lib")
#endif

#include "mmTreeItemData.h"

mmTreeItemData::mmTreeItemData(int type, int64 id)
    : type_(type)
    , id_(id)
    , report_(nullptr)
{
    stringData_ = wxString::Format("%lld", id);
}

mmTreeItemData::mmTreeItemData(int type, const wxString& data)
    : type_(type)
    , stringData_(data)
    , report_(nullptr)
{}

mmTreeItemData::mmTreeItemData(int type, int64 id, const wxString& data)
    : type_(type)
    , id_(id)
    , stringData_(data)
    , report_(nullptr)
{}

mmTreeItemData::mmTreeItemData(const wxString& data, mmPrintableBase* report)
    : type_(mmTreeItemData::REPORT)
    , stringData_(data)
    , report_(report)
{
    const wxString& n = wxString::Format("REPORT_%d", report_->getReportId());
    const wxString& settings = Model_Infotable::instance().getString(n, "");
    report_->initReportSettings(settings);
}

mmTreeItemData::mmTreeItemData(mmPrintableBase* report, const wxString& data)
    : type_(mmTreeItemData::GRM)
    , stringData_(data)
    , report_(report)
{}

