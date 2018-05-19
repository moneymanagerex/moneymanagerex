/*******************************************************
Copyright (C) 2014 Nikolay Akimov

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

// http://en.wikipedia.org/wiki/QIF
// http://linuxfinances.info/info/financeformats.html
// https://metacpan.org/pod/Finance::QIF
// https://github.com/Gnucash/gnucash/blob/master/src/import-export/qif-imp/file-format.txt

struct QIF_Account
{
    wxString N; // Account name
    wxString T; // Account type
};

struct QIF_Transaction
{
    wxString D; // Date
    wxString T; // Amount
    wxString C; // Cleared status
    wxString N; // Number (check or reference)
    wxString P; // Payee/description
    wxString M; // Memo
    wxString A; // Address (up to 5 lines; 6th line is an optional message)
    wxString L; // Category/class or transfer/class
    wxString S; // Category in split (category/class or transfer/class)
    wxString E; // Memo in split
    wxString DOLLOR; // Dollar amount of split
    wxString CARET; // End of entry
};

struct QIF_Invst_Transaction
{
    wxString D; // Date (optional)
    wxString N; // Action
    wxString Y; // Security
    wxString I; // Price
    wxString Q; // Quantity ( of shares or split ratio)
    wxString C; // Cleared status
    wxString P; // 1st line text for transfers/reminders
    wxString M; // Memo
    wxString O; // Commission
    wxString L; /* For MiscIncX or MiscExpX actions, Category/classtransfer/class or transfer/class
                   For all other actions, Category/class or transfer/class */
    wxString DOLLOR; // Amount transferred
    wxString CARET; // End of entry (required)
};

struct QIF_Category
{
    wxString N; // Category name
    wxString D; // Verbose Category name
    wxString T; // Is this category relevant to taxes?
    wxString E; // Expense account?
    wxString R; // Appears to be an indicator of a "Tax Account," or something of the sort.
};

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
    typedef std::pair<qifLineType, wxString> QIF_Line;
    typedef std::vector<QIF_Line> QIF_Record;
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
    bool handle_line(const wxString& line, QIF_Line& qif_line);
    bool handle_qif_record(const QIF_Record & qif_record, QIF_Transaction& tran);
    bool handle_qif_line(const QIF_Line& qif_line, QIF_Transaction& tran);
};
#endif // 
