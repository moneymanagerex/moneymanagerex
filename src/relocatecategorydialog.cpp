/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2011 Stefano Giorgio

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

#include "relocatecategorydialog.h"
#include "paths.h"
#include "categdialog.h"
#include "wx/statline.h"

IMPLEMENT_DYNAMIC_CLASS( relocateCategoryDialog, wxDialog )

BEGIN_EVENT_TABLE( relocateCategoryDialog, wxDialog )
    EVT_BUTTON(wxID_CLEAR, relocateCategoryDialog::OnSelectSource)
    EVT_BUTTON(wxID_NEW, relocateCategoryDialog::OnSelectDest)
    EVT_BUTTON(wxID_OK, relocateCategoryDialog::OnOk)
END_EVENT_TABLE()

relocateCategoryDialog::relocateCategoryDialog( )
{
    core_           =  0;

    destCatID_      = -1;
    destSubCatID_   = -1;
    changedCats_    = 0;
    changedSubCats_ = 0;

}

relocateCategoryDialog::relocateCategoryDialog( mmCoreDB* core,
    wxWindow* parent, int sourceCatID, int sourceSubCatID,
    wxWindowID id, const wxString& caption,
    const wxPoint& pos, const wxSize& size, long style )
{
    core_           = core;

    sourceCatID_    = sourceCatID;
    sourceSubCatID_ = sourceSubCatID;

    destCatID_      = -1;
    destSubCatID_   = -1;

    Create(parent, id, caption, pos, size, style);
}

bool relocateCategoryDialog::Create( wxWindow* parent, wxWindowID id,
                           const wxString& caption, const wxPoint& pos,
                           const wxSize& size, long style )
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    SetIcon(mmex::getProgramIcon());

    Centre();
    return TRUE;
}

void relocateCategoryDialog::CreateControls()
{
    wxSizerFlags flags, flagsExpand;
    flags.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).Border(wxALL, 5).Center();
    flagsExpand.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).Border(wxALL, 5).Expand().Center();
    wxSize btnSize = wxSize(180,-1);

    wxStaticText* headerText = new wxStaticText( this, wxID_STATIC,
        _("Relocate all source categories to the destination category"));
    wxStaticLine* lineTop = new wxStaticLine(this, wxID_STATIC);

    sourceBtn_ = new wxButton( this, wxID_CLEAR,_("Source Category"), wxDefaultPosition, wxSize(200, -1));
    if (sourceCatID_ > -1)
        sourceBtn_->SetLabel(core_->categoryList_.GetFullCategoryString(sourceCatID_, sourceSubCatID_));
    destBtn_ = new wxButton( this, wxID_NEW,_("Destination Category"), wxDefaultPosition, wxSize(200, -1));
    wxStaticLine* lineBottom = new wxStaticLine(this, wxID_STATIC);

    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(topSizer);
    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    wxFlexGridSizer* request_sizer = new wxFlexGridSizer(0, 2, 0, 0);

    topSizer->Add(boxSizer, flags);

    boxSizer->Add(headerText, flags);
    boxSizer->Add(lineTop, flagsExpand);

    request_sizer->Add(new wxStaticText( this, wxID_STATIC,_("Relocate:")), flags);
    request_sizer->Add(new wxStaticText( this, wxID_STATIC,_("to:")), flags);
    request_sizer->Add(sourceBtn_, flags);
    request_sizer->Add(destBtn_, flags);
    boxSizer->Add(request_sizer);

    boxSizer->Add(lineBottom, flagsExpand);

    wxButton* okButton = new wxButton(this, wxID_OK);
    wxButton* cancelButton = new wxButton(this, wxID_CANCEL);
    cancelButton-> SetFocus();
    wxBoxSizer* buttonBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonBoxSizer->Add(okButton, flags);
    buttonBoxSizer->Add(cancelButton, flags);
    boxSizer->Add(buttonBoxSizer, flags);

    this->Fit();
}

void relocateCategoryDialog::OnSelectSource(wxCommandEvent& /*event*/)
{
    mmCategDialog sourceCat(core_ , this, true, false);

    sourceCat.setTreeSelection(core_->categoryList_.GetCategoryName(sourceCatID_)
    , core_->categoryList_.GetSubCategoryName(sourceCatID_, sourceSubCatID_));

    if (sourceCat.ShowModal() == wxID_OK)
    {
        sourceCatID_    = sourceCat.getCategId();
        sourceSubCatID_ = sourceCat.getSubCategId();
        sourceBtn_->SetLabel(core_->categoryList_.GetFullCategoryString(sourceCatID_, sourceSubCatID_));
    }
    sourceCat.Destroy();
}

void relocateCategoryDialog::OnSelectDest(wxCommandEvent& /*event*/)
{
    mmCategDialog destCat(core_ , this, true, false);

    destCat.setTreeSelection(core_->categoryList_.GetCategoryName(destCatID_)
    , core_->categoryList_.GetSubCategoryName(destCatID_, destSubCatID_));

    if (destCat.ShowModal() == wxID_OK)
    {
        destCatID_    = destCat.getCategId();
        destSubCatID_ = destCat.getSubCategId();

        destBtn_->SetLabel(core_->categoryList_.GetFullCategoryString(destCatID_, destSubCatID_));
    }
}

wxString relocateCategoryDialog::updatedCategoriesCount() const
{
    wxString countStr;
    countStr << (changedCats_ + changedSubCats_);
    return countStr;
}

void relocateCategoryDialog::OnOk(wxCommandEvent& /*event*/)
{
    if ((sourceCatID_ > 0) && (destCatID_ > 0) )
    {
        wxString msgStr = _("Please Confirm:");
        msgStr << "\n\n" << _("Changing all categories of: ")
            << sourceBtn_->GetLabelText() << "\n\n" << _("to category: ") << destBtn_->GetLabelText();

        int ans = wxMessageBox(msgStr,_("Category Relocation Confirmation"), wxOK|wxCANCEL|wxICON_QUESTION);
        if (ans == wxOK)
        {
            if (core_->bTransactionList_.RelocateCategory(core_,
                destCatID_, destSubCatID_, sourceCatID_, sourceSubCatID_, changedCats_, changedSubCats_) == 0)
                EndModal(wxID_OK);
        }
    }
}
