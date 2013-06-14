/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel

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

#include "mmpayee.h"
#include "util.h"
#include "mmOption.h"
#include "mmcoredb.h"
#include <algorithm>

// Constructor used when loading a payee from the database
mmPayee::mmPayee(wxSQLite3ResultSet q1)
{
    id_        = q1.GetInt("PAYEEID");
    name_      = q1.GetString("PAYEENAME");
    categoryId_    = q1.GetInt("CATEGID");
    subcategoryId_ = q1.GetInt("SUBCATEGID");
}

// Constructor when adding a new payee to the database
mmPayee::mmPayee(int id, const wxString& name)
: id_(id)
, name_(name)
, categoryId_(-1)
, subcategoryId_(-1)
{}

bool mmPayee::operator < (const mmPayee& payee) const
{
    if (this->name_ < payee.name_) return true; else if (this->name_ > payee.name_) return false;
    return this->id_ < payee.id_;
}

bool mmPayeeList::PayeeExists(const wxString& payeeName) const
{
    for (const auto& payee: entries_)
    {
        if (!payee->name_.CmpNoCase(payeeName)) return true;
    }

    return false;
}

bool mmPayeeList::PayeeExists(int payeeid) const
{
    for (const auto& payee: entries_)
    {
        if (payee->id_ == payeeid) return true;
    }

    return false;
}

int mmPayeeList::GetPayeeId(const wxString& payeeName) const
{
    for (const auto& payee: entries_)
    {
        if (!payee->name_.CmpNoCase(payeeName)) return payee->id_;
    }

    return -1;
}

wxString mmPayeeList::GetPayeeName(int id) const
{
    for (const auto& payee: entries_)
    {
        if (payee->id_ == id) return payee->name_;
    }

    return wxEmptyString;
}

std::shared_ptr<mmPayee> mmPayeeList::GetPayeeSharedPtr(int payeeID)
{
    int numPayees = (int)entries_.size();
    for (int idx = 0; idx < numPayees; idx++)
    {
        if (entries_[idx]->id_ == payeeID)
        {
           return entries_[idx];
        }
    }
    return std::shared_ptr<mmPayee>();
}

wxArrayString mmPayeeList::FilterPayees(const wxString& patt) const
{
    wxArrayString payee_list;

    for (size_t idx = 0; idx < (int)entries_.size(); idx++)
    {
        if (entries_[idx]->name_.Lower().Matches(patt.Lower().Append("*")))
            payee_list.Add(entries_[idx]->name_);
    }
    return payee_list;
}

void mmPayeeList::LoadPayees()
{
    wxSQLite3ResultSet q1 = core_->db_.get()->ExecuteQuery(SELECT_ALL_FROM_PAYEE_V1);

    entries_.clear();
    while (q1.NextRow())
    {
        std::shared_ptr<mmPayee> pPayee(new mmPayee(q1));
        entries_.push_back(pPayee);
    }

    q1.Finalize();
    std::sort(entries_.begin(), entries_.end());
}

bool mmPayeeList::RemovePayee(int payeeID)
{
    if (core_->bTransactionList_.IsPayeeUsed(payeeID)) return false;

    long payeeId = payeeID;
    std::vector<wxString> data;
    data.push_back(wxString()<<payeeID);
    wxString sql = wxString::FromUTF8(DELETE_FROM_PAYEE_V1);
    int iError = mmDBWrapper::mmSQLiteExecuteUpdate(core_->db_.get(), data, sql, payeeId);
    if (iError != 0)
        return false;

    LoadPayees();
    mmOptions::instance().databaseUpdated_ = true;
    return true;
}

int mmPayeeList::AddPayee(const wxString &payeeName)
{
    std::vector<wxString> data;
    data.push_back(payeeName);
    data.push_back("-1");
    data.push_back("-1");
    long payeeID = -1;
    wxString sql = wxString::FromUTF8(INSERT_INTO_PAYEE_V1);
    int iError = mmDBWrapper::mmSQLiteExecuteUpdate(core_->db_.get(), data, sql, payeeID);
    if (iError != 0)
        return -1;

    LoadPayees();
    mmOptions::instance().databaseUpdated_ = true;
    return payeeID;
}

int mmPayeeList::UpdatePayee(int payeeID, const wxString& payeeName)
{
    std::shared_ptr<mmPayee> pPayee = GetPayeeSharedPtr(payeeID);
    if (!payeeName.IsEmpty()) pPayee->name_ = payeeName;
    std::vector<wxString> data;
    data.push_back(pPayee->name_);
    data.push_back(wxString()<<pPayee->categoryId_);
    data.push_back(wxString()<<pPayee->subcategoryId_);
    data.push_back(wxString()<<payeeID);

    long payeeId = -1;
    wxString sql = wxString::FromUTF8(UPDATE_PAYEE_V1);
    int iError = mmDBWrapper::mmSQLiteExecuteUpdate(core_->db_.get(), data, sql, payeeId);
    if (iError != 0)
        return -1;

    LoadPayees();
    mmOptions::instance().databaseUpdated_ = true;
    return iError;
}
