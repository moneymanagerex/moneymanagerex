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

#pragma once

#include "base/defs.h"
#include "util/_choices.h"
#include "table/AttachmentTable.h"
#include "_ModelBase.h"

class AttachmentModel : public Model<AttachmentTable>
{
public:
    using Model<AttachmentTable>::cache_id;

public:
    AttachmentModel();
    ~AttachmentModel();

public:
    /**
    Initialize the global AttachmentModel table on initial call.
    Resets the global table on subsequent calls.
    * Return the static instance address for AttachmentModel table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static AttachmentModel& instance(wxSQLite3Database* db);

    /**
    * Return the static instance address for AttachmentModel table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static AttachmentModel& instance();

public:
    /** Return a dataset with attachments linked to a specific object */
    const Data_Set FilterAttachments(const wxString& RefType, const int64 RefId);

    /** Return the number of attachments linked to a specific object */
    static int NrAttachments(const wxString& RefType, const int64 RefId);

    /** Return the last attachment number linked to a specific object */
    static int LastAttachmentNumber(const wxString& RefType, const int64 RefId);

    /** Return a dataset with attachments linked to a specific type*/
    std::map<int64, Data_Set> get_reftype(const wxString& reftype);

    /** Return all attachments descriptions*/
    wxArrayString allDescriptions();
};

