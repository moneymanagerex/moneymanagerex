/*******************************************************
Copyright (C) 2014 Gabriele-V

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

#ifndef MM_EX_WEBAPP_H_
#define MM_EX_WEBAPP_H_

#include <vector>
#include <wx/string.h>
#include <wx/datetime.h>

//Parameters used in services.php
namespace WebAppParam
{
    extern const wxString ServicesPage;
    extern const wxString ApiExpectedVersion;
    extern const wxString CheckGuid;
    extern const wxString CheckApiVersion;
    extern const wxString DeleteAccount;
    extern const wxString ImportAccount;
    extern const wxString DeletePayee;
    extern const wxString ImportPayee;
    extern const wxString DeleteCategory;
    extern const wxString ImportCategory;
    extern const wxString DeleteOneTransaction;
    extern const wxString DownloadNewTransaction;
    extern const wxString DownloadAttachments;
    extern const wxString DeleteAttachment;
    extern const wxString MessageSuccedeed;
    extern const wxString MessageWrongGuid;
}

class mmWebApp
{
const static wxString getUrl();
const static wxString getGuid();

/** Return services page URL with GUID inserted */
const static wxString getServicesPageURL();

//Internal function
const static wxString WebApp_getApiVersion();
static int WebApp_SendJson(wxString& Website, const wxString& JsonData, wxString& Output);
static bool WebApp_DeleteAllAccount();
static bool WebApp_DeleteAllPayee();
static bool WebApp_DeleteAllCategory();
static wxString WebApp_DownloadOneAttachment(const wxString& AttachmentName, int DesktopTransactionID, int AttachmentNr);

public:
    /*WebApp transaction Structure*/
    struct webtran_holder
    {
        int ID;
        wxDateTime Date;
        wxString Account;
        wxString ToAccount;
        wxString Status;
        wxString Type;
        wxString Payee;
        wxString Category;
        wxString SubCategory;
        double Amount;
        wxString Notes;
        wxString Attachments;
    };
    typedef std::vector<webtran_holder> WebTranVector;

    static bool returnResult(int& ErrorCode, wxString& outputMessage);

    /** Return true if WebApp is enabled */
    static bool WebApp_CheckEnabled();

    /** Return true if WebApp Guid is correct */
    static bool WebApp_CheckGuid();

    /** Return true if WebApp API Version is correct */
    static bool WebApp_CheckApiVersion();

    /** Update all accounts on WebApp */
    static bool WebApp_UpdateAccount();

    /** Update all payees on WebApp */
    static bool WebApp_UpdatePayee();

    /** Update all categories on WebApp */
    static bool WebApp_UpdateCategory();

    /** Download new transaction */
    static bool WebApp_DownloadNewTransaction(WebTranVector& WebAppTransactions_, const bool CheckOnly);

    /** Insert transaction in MMEX desktop, returns transaction ID */
    static int MMEX_InsertNewTransaction(webtran_holder& WebAppTrans);

    /** Delete transaction from WebApp */
    static bool WebApp_DeleteOneTransaction(int WebAppTransactionId);

    /* Return attachment URL */
    static wxString WebApp_GetAttachment(const wxString& AttachmentFileName);

    //FUNCTIONS CALLED IN MMEX TO UPDATE ON CHANGE
    /** Update all payees on WebApp if enabled */
    static bool MMEX_WebApp_UpdatePayee();

    /** Update all accounts on WebApp if enabled */
    static bool MMEX_WebApp_UpdateAccount();

    /** Update all categories on WebApp if enabled */
    static bool MMEX_WebApp_UpdateCategory();
};

#endif // MM_EX_WEBAPP_H_
