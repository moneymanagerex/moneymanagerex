/*******************************************************
Copyright (C) 2009 VaDiM

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
#ifndef _MM_EX_CONSTANTS_H_
#define _MM_EX_CONSTANTS_H_

#include "defs.h"
//----------------------------------------------------------------------------

class wxString;

extern const wxSizerFlags g_flags;
extern const wxSizerFlags g_flagsExpand;

namespace mmex
{
extern const int MIN_DATAVERSION;
extern const wxString DATAVERSION;

extern const wxString DEFDATEFORMAT;
extern const wxString DEFDELIMTER;
extern const wxString DEFSTOCKURL;

/*
        Uses for titles, reports, etc.
        Do not use mmex::GetAppName() for such things.
*/
const wxString getProgramName();
const wxString getTitleProgramVersion();
const wxString getProgramVersion();
const wxString getProgramCopyright();
const wxString getProgramWebSite();
const wxString getProgramForum();
const wxString getProgramDanateSite();
const wxString getProgramTwitterSite();
const wxString getProgramFacebookSite();
const wxString getProgramDescription();
} // namespace mmex

extern const wxString VIEW_TRANS_ALL_STR;            
extern const wxString VIEW_TRANS_TODAY_STR;          
extern const wxString VIEW_TRANS_CURRENT_MONTH_STR;  
extern const wxString VIEW_TRANS_LAST_30_DAYS_STR;   
extern const wxString VIEW_TRANS_LAST_90_DAYS_STR;   
extern const wxString VIEW_TRANS_LAST_MONTH_STR;     
extern const wxString VIEW_TRANS_LAST_3MONTHS_STR;   
extern const wxString VIEW_TRANS_LAST_12MONTHS_STR;  
extern const wxString VIEW_TRANS_CURRENT_YEAR_STR;   

extern const wxString  VIEW_ACCOUNTS_ALL_STR;       
extern const wxString  VIEW_ACCOUNTS_OPEN_STR;      
extern const wxString  VIEW_ACCOUNTS_FAVORITES_STR; 

extern const wxString LANGUAGE_PARAMETER;
extern const wxString INIDB_USE_TRANSACTION_SOUND;
extern const wxString INIDB_USE_ORG_DATE_COPYPASTE;

extern const wxString INIDB_BUDGET_FINANCIAL_YEARS;
extern const wxString INIDB_BUDGET_INCLUDE_TRANSFERS;
extern const wxString INIDB_BUDGET_SETUP_WITHOUT_SUMMARY;
extern const wxString INIDB_BUDGET_SUMMARY_WITHOUT_CATEG;
extern const wxString INIDB_IGNORE_FUTURE_TRANSACTIONS;

extern const wxString INIDB_ATTACHMENTS_FOLDER_DOCUMENTSDIR;
extern const wxString INIDB_ATTACHMENTS_FOLDER_MMEXDIR;
extern const wxString INIDB_ATTACHMENTS_FOLDER_DBDIR;

#endif // _MM_EX_CONSTANTS_H_
//----------------------------------------------------------------------------
