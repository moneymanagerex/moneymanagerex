/*******************************************************
Copyright (C) 2014 Gabriele-V
Copyright (C) 2026 George Ef (george.a.ef@gmail.com)

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

#include "base/defs.h"
#include "base/types.h"
#include <vector>
#include <wx/string.h>
#include <wx/datetime.h>

// Parameters used in services.php
namespace WebAppParam
{
    extern const wxString ApiExpectedVersion;
    extern const wxString CheckApiVersion;
    extern const wxString CheckGuid;
    extern const wxString DeleteAccount;
    extern const wxString DeleteAttachment;
    extern const wxString DeleteCategory;
    extern const wxString DeletePayee;
    extern const wxString DeleteTrx;
    extern const wxString DownloadAttachment;
    extern const wxString DownloadNewTransaction;
    extern const wxString UploadAccount;
    extern const wxString UploadCategory;
    extern const wxString UploadPayee;
    extern const wxString MessageSuccedeed;
    extern const wxString MessageWrongGuid;
    extern const wxString ServicesPage;
}

class mmWebApp
{
public:
    // WebApp transaction Structure
    struct TrxWebData
    {
        int64      ID;
        wxDateTime Date;
        wxString   Account;
        wxString   ToAccount;
        wxString   Status;
        wxString   Type;
        wxString   Payee;
        wxString   Category;
        wxString   SubCategory;
        int64      ParentCategory;
        double     Amount;
        wxString   Notes;
        wxString   Attachments;
    };
    typedef std::vector<TrxWebData> TrxWebDataA;

public:
    static const wxString url();
    static const wxString guid();
    static const wxString services();
    static const wxString apiVersion();

    static bool result(int& error_code, wxString& output_message);
    static bool isEnabled();
    static bool checkGuid();
    static bool checkApiVersion();

    static int postData(
        const wxString& website,
        const wxString& data_json,
        wxString& output
    );

    static bool deleteAccount();
    static bool deletePayee();
    static bool deleteCategory();
    static bool deleteTrxWebId(int64 trx_w_id);

    static bool uploadAccount();
    static bool uploadPayee();
    static bool uploadCategory();

    static bool downloadNewTrx(
        TrxWebDataA& new_trx_wa,
        const bool check_only,
        wxString& error
    );
    static wxString downloadAttachment(
        const wxString& attachment_w_name,
        int64 trx_id,
        int attachment_number,
        wxString& error
    );
    static bool downloadAttachmentFile(
        wxString& file_name,
        wxString& error
    );

    static int64 insertNewTrx(TrxWebData& trx_w);

};

