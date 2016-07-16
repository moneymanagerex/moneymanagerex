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

#include "jsonstore.h"
#include "Model_Infotable.h"

/*************************************************************************
Json base class
*************************************************************************/
JsonStore::JsonStore(const wxString& table_key)
    : m_infotable_key(table_key)
{
    std::wstringstream in_stream;
    wxString jdata = Model_Infotable::instance().GetStringInfo(m_infotable_key, "{}");
    if (!(jdata.StartsWith("{") && jdata.EndsWith("}")))
    {
        jdata = "{}";
    }
    in_stream << jdata.ToStdWstring();
    json::Reader::Read(m_jo, in_stream);
}

JsonStore::~JsonStore()
{
    // save json values to the infotable file
    std::wstringstream out_stream;
    json::Writer::Write(m_jo, out_stream);
    Model_Infotable::instance().Set(m_infotable_key, out_stream.str());
}


/*************************************************************************
Json Account class
*************************************************************************/
JsonAccount::JsonAccount(int account_id)
    :JsonStore(wxString::Format("ACCOUNT_%i_DATA", account_id))
{
    // Set the infotable storage location for the json object string
}

bool JsonAccount::StatementLocked()
{
    return json::Boolean(m_jo[L"StatementLocked"]);
}

void JsonAccount::StatementLocked(bool locked)
{
    m_jo[L"StatementLocked"] = json::Boolean(locked);
}

wxDateTime JsonAccount::StatementDate()
{
    const wxString date_str = json::String(m_jo[L"StatementDate"]);
    wxDateTime date;
    if (!date.ParseISODate(date_str))
    {
        date = wxDateTime::Today();
    }

    return date;
}

void JsonAccount::StatementDate(wxDateTime date)
{
    m_jo[L"StatementDate"] = json::String(date.FormatISODate().ToStdWstring());
}

double JsonAccount::MinimumBalance()
{
    return json::Number(m_jo[L"MinimumBalance"]);
}

void JsonAccount::MinimumBalance(double value)
{
    m_jo[L"MinimumBalance"] = json::Number(value);
}

double JsonAccount::CreditLimit()
{
    return json::Number(m_jo[L"CreditLimit"]);
}

void JsonAccount::CreditLimit(double value)
{
    m_jo[L"CreditLimit"] = json::Number(value);
}

double JsonAccount::InterestRate()
{
    return json::Number(m_jo[L"InterestRate"]);
}

void JsonAccount::InterestRate(double value)
{
    m_jo[L"InterestRate"] = json::Number(value);
}

wxDateTime JsonAccount::PaymentDueDate()
{
    const wxString date_str = json::String(m_jo[L"PaymentDueDate"]);
    wxDateTime date;
    if (!date.ParseISODate(date_str))
    {
        date = wxDateTime::Today();
    }

    return date;
}

void JsonAccount::PaymentDueDate(wxDateTime date)
{
    m_jo[L"PaymentDueDate"] = json::String(date.FormatISODate().ToStdWstring());
}

double JsonAccount::MinimumPayment()
{
    return json::Number(m_jo[L"MinimumPayment"]);
}

void JsonAccount::MinimumPayment(double value)
{
    m_jo[L"MinimumPayment"] = json::Number(value);
}
