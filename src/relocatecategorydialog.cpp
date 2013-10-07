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
#include "model/Model_Category.h"

IMPLEMENT_DYNAMIC_CLASS( relocateCategoryDialog, wxDialog )

BEGIN_EVENT_TABLE( relocateCategoryDialog, wxDialog )
    EVT_BUTTON(wxID_CLEAR, relocateCategoryDialog::OnSelectSource)
    EVT_BUTTON(wxID_NEW, relocateCategoryDialog::OnSelectDest)
    EVT_BUTTON(wxID_OK, relocateCategoryDialog::OnOk)
END_EVENT_TABLE()

relocateCategoryDialog::relocateCategoryDialog( )
{
    destCatID_      = -1;
    destSubCatID_   = -1;
    changedCats_    = 0;
    changedSubCats_ = 0;
}

relocateCategoryDialog::relocateCategoryDialog(
    wxWindow* parent, int sourceCatID, int sourceSubCatID,
    wxWindowID id, const wxString& caption,
    const wxPoint& pos, const wxSize& size, long style )
{

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
    {
        Model_Category::Data* category = Model_Category::instance().get(sourceCatID_);
        Model_Subcategory::Data* sub_category = (sourceSubCatID_ != -1 ? Model_Subcategory::instance().get(sourceSubCatID_) : 0);
        sourceBtn_->SetLabel(Model_Category::full_name(category, sub_category));
    }
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
    mmCategDialog sourceCat(this, true, false);
    Model_Category::Data* category = Model_Category::instance().get(sourceCatID_);
    Model_Subcategory::Data* sub_category = (sourceSubCatID_ != -1 ? Model_Subcategory::instance().get(sourceSubCatID_) : 0);

    sourceCat.setTreeSelection(category->CATEGNAME, Model_Category::full_name(category, sub_category));

    if (sourceCat.ShowModal() == wxID_OK)
    {
        sourceCatID_    = sourceCat.getCategId();
        sourceSubCatID_ = sourceCat.getSubCategId();
        Model_Category::Data* category = Model_Category::instance().get(sourceCatID_);
        Model_Subcategory::Data* sub_category = (sourceSubCatID_ != -1 ? Model_Subcategory::instance().get(sourceSubCatID_) : 0);

        sourceBtn_->SetLabel(Model_Category::full_name(category, sub_category));
    }
    sourceCat.Destroy();
}

void relocateCategoryDialog::OnSelectDest(wxCommandEvent& /*event*/)
{
    mmCategDialog destCat(this, true, false);
    Model_Category::Data* category = Model_Category::instance().get(destCatID_);
    Model_Subcategory::Data* sub_category = (destSubCatID_ != -1 ? Model_Subcategory::instance().get(destSubCatID_) : 0);

    destCat.setTreeSelection(category->CATEGNAME, Model_Category::full_name(category, sub_category));

    if (destCat.ShowModal() == wxID_OK)
    {
        destCatID_    = destCat.getCategId();
        destSubCatID_ = destCat.getSubCategId();
        Model_Category::Data* category = Model_Category::instance().get(destCatID_);
        Model_Subcategory::Data* sub_category = (destSubCatID_ != -1 ? Model_Subcategory::instance().get(destSubCatID_) : 0);

        destBtn_->SetLabel(Model_Category::full_name(category, sub_category));
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
        wxString msgStr = wxString()
            <<_("Please Confirm:")
            << "\n\n" 
            << wxString::Format(_("Changing all categories of: %s\n\nto category: %s")
                , sourceBtn_->GetLabelText(), destBtn_->GetLabelText());

        int ans = wxMessageBox(msgStr,_("Category Relocation Confirmation"), wxOK|wxCANCEL|wxICON_QUESTION);
        if (ans == wxOK)
        {
            Model_Checking::Data_Set transactions = Model_Checking::instance().find(Model_Checking::COL_CATEGID, sourceCatID_
                , Model_Checking::COL_SUBCATEGID, sourceSubCatID_);
            for (auto &trx : transactions)
            {
                trx.CATEGID = destCatID_;
                trx.SUBCATEGID = destSubCatID_;
            }
            Model_Checking::instance().save(transactions);

            Model_Billsdeposits::Data_Set billsdeposits = Model_Billsdeposits::instance().find(Model_Billsdeposits::COL_CATEGID, sourceCatID_);
            for (auto &trx : billsdeposits)
            {
                if (sourceCatID_==trx.CATEGID && sourceSubCatID_==trx.SUBCATEGID)
                {
                    trx.CATEGID = destCatID_;
                    trx.SUBCATEGID = destSubCatID_;
                }
            }
            Model_Billsdeposits::instance().save(billsdeposits);

            Model_Splittransaction::Data_Set checking_split = Model_Splittransaction::instance().find(Model_Splittransaction::COL_CATEGID, sourceCatID_);
            for (auto &trx : checking_split)
            {
                if (sourceCatID_==trx.CATEGID && sourceSubCatID_==trx.SUBCATEGID)
                {
                    trx.CATEGID = destCatID_;
                    trx.SUBCATEGID = destSubCatID_;
                }
            }
            Model_Splittransaction::instance().save(checking_split);
            //TODO: same changes for billsdeposits split transactions

            EndModal(wxID_OK);
        }
    }
}
