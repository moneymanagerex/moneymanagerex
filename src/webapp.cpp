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


#include "webapp.h"
#include "model/Model_Setting.h"
#include "model/Model_Account.h"
#include "model/Model_Payee.h"
#include "util.h"

//Internal constants
const wxString mmWebApp::geturl()
{
    return Model_Infotable::instance().GetStringInfo("WEBAPPURL", "");
}

const wxString mmWebApp::getguid()
{
    return Model_Infotable::instance().GetStringInfo("WEBAPPGUID", "");
}

const wxString mmWebApp::getdelimiter()
{
    return ";;";
}

const wxString mmWebApp::getservicespage()
{
    return "services.php";
}

const wxString mmWebApp::getimportaccountparameter()
{
    return "import_bankaccount";
}

const wxString mmWebApp::getimportpayeeparameter()
{
    return "import_payee";
}

const wxString mmWebApp::getdeletealltransactionparameter()
{
    return "delete_all";
}

const wxString mmWebApp::getdownloadnewtransactionparameter()
{
    return "download_db";
}

//Public constants
const wxString mmWebApp::getmessagesucceeded()
{
    return "Operation has succeeded";
}

const wxString mmWebApp::getmessagewrongguid()
{
    return "Wrong GUID";
}

// Update Account on WebApp
bool mmWebApp::UpdateAccount()
{
    wxString accounts_list;
    const auto &accounts = Model_Account::instance().all(Model_Account::COL_ACCOUNTNAME);
    for (const auto &account : accounts)
    {
        if (Model_Account::type(account) != Model_Account::INVESTMENT)
        {
            if (accounts_list != "")
                accounts_list += mmWebApp::getdelimiter();
            accounts_list += account.ACCOUNTNAME;
        }
    }
    
    wxString updateaccounturl;
    updateaccounturl = mmWebApp::geturl() + "/" +
        mmWebApp::getservicespage() + "?" +
        mmWebApp::getimportaccountparameter() + "=" +
        accounts_list + "&"
        "guid=" + mmWebApp::getguid()
        ;


    wxString sOutput;
    int err_code = site_content(updateaccounturl, sOutput);

    if (err_code == 0 || sOutput == mmWebApp::getmessagesucceeded())
    {
        return true;
    }
    else
    {
        return false;
    }
}

//Update payee on WebApp
bool mmWebApp::UpdatePayee()
{
    wxString payees_list;
    for (const auto &payee : Model_Payee::instance().all(Model_Payee::COL_PAYEENAME))
    {
        if (payees_list != "")
            payees_list += mmWebApp::getdelimiter();
        payees_list += payee.PAYEENAME;
    }

    wxString updatepayeeurl;
    updatepayeeurl = mmWebApp::geturl() + "/" +
        mmWebApp::getservicespage() + "?" +
        mmWebApp::getimportpayeeparameter() + "=" +
        payees_list + "&"
        "guid=" + mmWebApp::getguid()
        ;

    wxString sOutput;
    int err_code = site_content(updatepayeeurl, sOutput);

    if (err_code == 0 || sOutput == mmWebApp::getmessagesucceeded())
    {
        return true;
    }
    else
    {
        return false;
    }
}

//Delete all transactions on WebApp
bool mmWebApp::DeleteAllTransaction()
{
    wxString deletealltransactionurl;
    deletealltransactionurl = mmWebApp::geturl() + "/" +
        mmWebApp::getservicespage() + "?" +
        mmWebApp::getdeletealltransactionparameter() + "&"
        "guid=" + mmWebApp::getguid()
        ;

    wxString sOutput;
    int err_code = site_content(deletealltransactionurl, sOutput);

    if (err_code == 0 || sOutput == mmWebApp::getmessagesucceeded())
    {
        return true;
    }
    else
    {
        return false;
    }
}

//Download new transaction
bool mmWebApp::DownloadNewTransaction()
{
    //To be done
    return true;
}
