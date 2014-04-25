/*******************************************************
Copyright (C) 2014 Nikolay

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

#ifndef QIF_IMPORT_H
#define QIF_IMPORT_H

#include "defs.h"

enum qifLineType
{
    AcctType = 1, // !
    Date = 2, // D
    Amount = 3, // T
    Address = 4, // A
    Payee = 5, // P
    EOTLT = 6, // ^
    TransNumber = 7, // N
    Status = 8, // C
    UnknownType = 9,
    Memo = 10, // M
    Category = 11,  // L
    CategorySplit = 12,  // S
    MemoSplit = 13,  // E
    AmountSplit = 14,   // '$'
    //mmex expra
    AccountName = 64,
    ToAccountName,
    TrxType,
    isValid
};

//QIF specific data
enum qifAccountInfoType
{
    Name = 1, // N
    AccountType = 2, // T
    Description = 3, // D
    CreditLimit = 4, // L
    BalanceDate = 5, // /
    Balance = 6, // $
    EOT = 7, // ^
    UnknownInfo = 8
};

class mmQIFImport
{

public:
    mmQIFImport() {}

    bool isLineOK(const wxString& line);
    wxString getLineData(const wxString& line) const;
    wxString getFileLine(wxTextInputStream& textFile, int& lineNumber) const;
    wxString getFinancistoProject(wxString& sSubCateg) const;

    qifAccountInfoType accountInfoType(const wxString& line)
    {
        if (line.IsEmpty())
            return UnknownInfo;

        wxChar fChar = line.GetChar(0);
        switch (fChar)
        {
        case 'N':
            return Name;
        case 'T':
            return AccountType;
        case 'D':
            return Description;
        case 'L':
            return CreditLimit;
        case '/':
            return BalanceDate;
        case '$':
            return Balance;
        case '^':
            return EOT;
        default:
            return UnknownInfo;
        }
    }

    qifLineType lineType(const wxString& line)
    {
        if (line.IsEmpty())
            return UnknownType;

        wxChar fChar = line.GetChar(0);
        switch (fChar)
        {
        case '!':
            return AcctType;
        case 'D':
            return Date;
        case 'N':
            return TransNumber;
        case 'P':
            return Payee;
        case 'A':
            return Address;
        case 'T':
            return Amount;
        case '^':
            return EOTLT;
        case 'M':
            return Memo;
        case 'L':
            return Category;
        case 'S':
            return CategorySplit;
        case 'E':
            return MemoSplit;
        case '$':
            return AmountSplit;
        case 'C':
            return Status;
        default:
            return UnknownType;
        }
    }

};
#endif // 
