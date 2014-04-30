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
#include <vector>

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
    typedef std::pair<qifLineType, wxString> Line_Value;
    typedef std::vector<Line_Value> Record;
public:
    mmQIFImport() {}

    static bool isLineOK(const wxString& line);
    static wxString getLineData(const wxString& line);
    static wxString getFileLine(wxTextInputStream& textFile, int& lineNumber);
    static wxString getFinancistoProject(wxString& sSubCateg);

    static qifAccountInfoType accountInfoType(const wxString& line);
    static qifLineType lineType(const wxString& line); 

public:
    bool handle_file(wxFileInputStream& input);
    bool handle_file(const wxString& input_file);
    bool handle_line(const wxString& line, Line_Value& lv);
    bool handle_record(const Record & record);
};
#endif // 
