/*******************************************************
 Copyright (C) 2026

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

#include <set>
#include "base/_defs.h"

#include "PlanGroupModel.h"
#include "PlanItemModel.h"

// Initialize the global PlanGroupModel table.
PlanGroupModel& PlanGroupModel::instance(wxSQLite3Database* db)
{
    PlanGroupModel& ins = Singleton<PlanGroupModel>::instance();
    ins.reset_cache();
    ins.m_db = db;
    ins.ensure_table();

    return ins;
}

// Return the static instance of PlanGroupModel table
PlanGroupModel& PlanGroupModel::instance()
{
    return Singleton<PlanGroupModel>::instance();
}

// -- override

bool PlanGroupModel::purge_id(int64 group_id)
{
    bool ok = true;
    db_savepoint();

    // Remove the whole subtree, deepest first, so no child is left orphaned.
    std::vector<int64> subtree = find_subtree_id_a(group_id);
    for (auto it = subtree.rbegin(); it != subtree.rend(); ++it) {
        for (int64 item_id : PlanItemModel::instance().find_id_a(
            PlanItemCol::WHERE_GROUPID(OP_EQ, *it)
        )) {
            ok = ok && PlanItemModel::instance().purge_id(item_id);
        }
        ok = ok && unsafe_remove_id(*it);
    }

    db_release_savepoint();
    return ok;
}

// -- methods

PlanGroupModel::DataA PlanGroupModel::find_root_a()
{
    DataA out;
    for (const auto& g : find_data_a()) {
        if (g.m_active && g.is_root())
            out.push_back(g);
    }
    std::sort(out.begin(), out.end(), Data::SorterBySORTORDER());

    return out;
}

PlanGroupModel::DataA PlanGroupModel::find_children_a(int64 group_id)
{
    DataA out;
    for (const auto& g : find_data_a(
        PlanGroupCol::WHERE_PARENTID(OP_EQ, group_id)
    )) {
        if (g.m_active)
            out.push_back(g);
    }
    std::sort(out.begin(), out.end(), Data::SorterBySORTORDER());

    return out;
}

std::vector<int64> PlanGroupModel::find_subtree_id_a(int64 group_id)
{
    std::vector<int64> out;
    std::set<int64> seen;

    // Breadth-first, guarding against a corrupt parent chain that loops.
    std::vector<int64> queue { group_id };
    while (!queue.empty()) {
        const int64 id = queue.front();
        queue.erase(queue.begin());

        if (seen.count(id))
            continue;
        seen.insert(id);
        out.push_back(id);

        for (const auto& child : find_children_a(id)) {
            if (!seen.count(child.m_id))
                queue.push_back(child.m_id);
        }
    }

    return out;
}

const wxString PlanGroupModel::get_id_path_name(int64 group_id)
{
    wxString path;
    std::set<int64> seen;

    int64 id = group_id;
    while (id > 0 && !seen.count(id)) {
        seen.insert(id);
        const Data* g_n = get_idN_data_n(id);
        if (!g_n)
            break;
        path = path.IsEmpty() ? g_n->m_name : (g_n->m_name + " | " + path);
        id = g_n->m_parent_id;
    }

    return path;
}
