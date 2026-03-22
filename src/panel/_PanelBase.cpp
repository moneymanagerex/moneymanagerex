/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2015 James Higley
 Copyright (C) 2021 Mark Whalley (mark@ipx.co.uk)
 Copyright (C) 2025 George Ef (george.a.ef@gmail.com)
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

#include "model/SettingModel.h"
#include "_ListBase.h"
#include "ReportPanel.h"
#include "base/images_list.h"

PanelBase::PanelBase()
{
}

PanelBase::~PanelBase()
{
}

wxString PanelBase::buildPage() const
{
    ReportPanel* rp = wxDynamicCast(this, ReportPanel);
    return rp ? rp->getReportBase()->getHTMLText() : "TBD";
}

void PanelBase::printPage()
{
    //this->Freeze();
    wxWebView*  htmlWindow = wxWebView::New(this, wxID_ANY);
    htmlWindow->SetPage(buildPage(), "");
    htmlWindow->GetPageSource(); // Needed to generate the page - at least on Mac anyway!
    htmlWindow->Print();
    htmlWindow->Destroy();
    //this->Thaw();
}

void PanelBase::windowsFreezeThaw()
{
    #ifdef __WXGTK__
    return;
    #endif

    if (this->IsFrozen())
        this->Thaw();
    else
        this->Freeze();
}
