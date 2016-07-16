/*******************************************************
Copyright (C) 2016 Stefano Giorgio

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

/*
   Store extra information about a table in infotable using json objects.
   The class becomes an interface to the storage table.
*/
#include "defs.h"
#include "cajun/json/elements.h"
#include "cajun/json/reader.h"
#include "cajun/json/writer.h"

/*************************************************************************
Json base class
*************************************************************************/
class JsonStore
{
private:
    wxString m_infotable_key;

public:
    json::Object m_jo;
    bool m_save_required;

public:
    JsonStore(const wxString& table_key);
    ~JsonStore();
};

/*************************************************************************
Json Account class
*************************************************************************/
class JsonAccount : public JsonStore
{
public:
    JsonAccount(int account_id);

    bool StatementLocked();
    void StatementLocked(bool locked);

    wxDateTime StatementDate();
    void StatementDate(wxDateTime date);

    double MinimumBalance();
    void MinimumBalance(double value);

    double CreditLimit();
    void CreditLimit(double value);

    double InterestRate();
    void InterestRate(double value);

    wxDateTime PaymentDueDate();
    void PaymentDueDate(wxDateTime date);

    double MinimumPayment();
    void MinimumPayment(double value);
};
