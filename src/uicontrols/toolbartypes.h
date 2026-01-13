/*******************************************************
 Copyright (C) 2025 Klaus Wich

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

#include "choices.h"
#include "defs.h"
#include "images_list.h"
#include "mmframe.h"


class ToolBarEntries
{
    public:
        enum TYPE_ID {
            TOOLBAR_BTN,
            TOOLBAR_SEPARATOR,
            TOOLBAR_SPACER,
            TOOLBAR_STRETCH
        };

        struct ToolBarEntry {
            int toolId;
            wxString label;
            wxString helpstring;
            int seq_no;
            int imageListID;
            int imageId;
            bool active;
            TYPE_ID type;
        };

        ToolBarEntries();
        ~ToolBarEntries();
        static ToolBarEntries& instance();

        void SetToDefault();
        wxImageList* getImageList();

        ToolBarEntry* getFirstEntry();
        ToolBarEntry* getNextEntry(ToolBarEntry* previous);
        ToolBarEntry* newEntry(TYPE_ID type, ToolBarEntry* previous);
        bool DeleteEntry(ToolBarEntry* info);
        void Save();
        void Load();
        void SetToolbarParent(mmGUIFrame* parent);
        void SortEntriesBySeq();

    private:
        std::vector<ToolBarEntry*> m_toolbar_entries;
        long unsigned int m_lastIdx;
        ToolBarEntry* m_previous;
        mmGUIFrame* m_toolbarParent;
};
