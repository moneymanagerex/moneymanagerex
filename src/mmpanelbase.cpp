/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2015 James Higley
 Copyright (C) 2021 Mark Whalley (mark@ipx.co.uk)
 Copyright (C) 2025 George Ef (george.a.ef@gmail.com)

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

#include "images_list.h"
#include "mmpanelbase.h"
#include "model/Model_Setting.h"
#include "mmreportspanel.h"

wxBEGIN_EVENT_TABLE(mmListCtrl, wxListCtrl)
    EVT_LIST_COL_END_DRAG(wxID_ANY,    mmListCtrl::onItemResize)
    EVT_LIST_COL_CLICK(wxID_ANY,       mmListCtrl::OnColClick)
    EVT_LIST_COL_RIGHT_CLICK(wxID_ANY, mmListCtrl::onColRightClick)
    EVT_MENU(wxID_ANY,                 mmListCtrl::onHeaderPopup)
wxEND_EVENT_TABLE()

const std::vector<int> ListColumnInfo::getListId(const std::vector<ListColumnInfo>& a_info)
{
    std::vector<int> a_id;
    for (int i = 0; i < static_cast<int>(a_info.size()); ++i)
        a_id.push_back(a_info[i].id);
    return a_id;
}

//----------------------------------------------------------------------------

mmListCtrl::mmListCtrl(wxWindow* parent, wxWindowID winid) :
    wxListCtrl(
        parent, winid, wxDefaultPosition, wxDefaultSize,
        wxLC_REPORT | wxLC_HRULES | wxLC_VRULES | wxLC_VIRTUAL | wxLC_SINGLE_SEL | wxLC_AUTOARRANGE
    ),
    attr1_(new wxListItemAttr(
        *bestFontColour(mmThemeMetaColour(meta::COLOR_LISTALT0)),
        mmThemeMetaColour(meta::COLOR_LISTALT0),
        wxNullFont
    )),
    attr2_(new wxListItemAttr(
        *bestFontColour(mmThemeMetaColour(meta::COLOR_LIST)),
        mmThemeMetaColour(meta::COLOR_LIST),
        wxNullFont
    ))
{
}

mmListCtrl::~mmListCtrl()
{
    savePreferences();
}

//----------------------------------------------------------------------------

void mmListCtrl::createColumns()
{
    loadPreferences();
    for (int col_nr = 0; col_nr < getColNrSize(); ++col_nr) {
        int col_id = getColId_Nr(col_nr);
        ListColumnInfo col_info = m_col_info_id[col_id];
        int col_width = m_col_width_id[col_id];
        if (isDisabledColId(col_id) || isHiddenColId(col_id)) col_width = 0;
        InsertColumn(col_nr, getColHeader(col_id), col_info.format, col_width);
    }
    updateSortIcon();
}

wxString mmListCtrl::BuildPage(const wxString &title) const
{
    static const wxString eol = wxTextFile::GetEOL();

    wxString text;
    text << "<!DOCTYPE html>" << eol
         << "<html>" << eol
         << "<head>" << eol
         << "<title>" << title << "</title>" << eol
         << "<meta charset = 'utf-8'>" << eol
         << "</head>" + eol
         << "<body>" << eol
         << "<table border="
         << ((GetWindowStyle() & wxLC_HRULES) || (GetWindowStyle() & wxLC_VRULES) ? "1" : "0") << " cellpadding=4 cellspacing=0 >" << eol
         << "<tr>" << eol;

    for (int col_vo = 0; col_vo < GetColumnCount(); ++col_vo) {
        int col_nr = getColNr_Vo(col_vo);
        if (isDisabledColNr(col_nr)) {
            continue;
        }
        wxListItem col_item;
        col_item.SetMask(wxLIST_MASK_TEXT);
        GetColumn(col_nr, col_item);
        text << "<th><i>" << col_item.GetText() << "</i></th>" << eol;
    }
    text << "</tr>" << eol;

    for (int row_nr = 0; row_nr < GetItemCount(); ++row_nr) {
        text << eol << "<tr>" << eol;
        for (int col_vo = 0; col_vo < GetColumnCount(); ++col_vo) {
            int col_nr = getColNr_Vo(col_vo);
            if (isDisabledColNr(col_nr)) {
                continue;
            }
            text << "<td>" << wxListCtrl::GetItemText(row_nr, col_nr) << "</td>" << eol;
        }
        text << "</tr>" << eol;
    }
    text << "</table>" << eol
         << "</body>" << eol
         << "</html>" << eol;

    return text;
}

//----------------------------------------------------------------------------

const wxString mmListCtrl::getColHeader(int col_id, bool show_icon) const
{
    ListColumnInfo col_info = m_col_info_id[col_id];
    if (!show_icon && col_id == 0 && col_info.header == "Icon")
        return " ";
    return col_info.translate ? wxGetTranslation(col_info.header) : col_info.header;
}

//----------------------------------------------------------------------------

void mmListCtrl::savePreferences()
{
    if (m_setting_name.empty())
        return;

    StringBuffer json_buffer;
    Writer<StringBuffer> json_writer(json_buffer);
    json_writer.StartObject();
    json_writer.Key("version");
    json_writer.Int(m_setting_version);

    if (!m_col_id_nr.empty()) {
        json_writer.Key("col_id_vo");
        json_writer.StartArray();
        for (int col_vo = 0; col_vo < getColNrSize(); ++col_vo)
            json_writer.Int(getColId_Vo(col_vo));
        json_writer.EndArray();
    }

    if (static_cast<int>(m_col_width_id.size()) == getColIdSize()) {
        for (int col_nr = 0; col_nr < GetColumnCount(); ++col_nr) {
            int col_id = getColId_Nr(col_nr);
            int new_width = GetColumnWidth(col_nr);
            if (new_width != 0)
                m_col_width_id[col_id] = new_width;
        }
        json_writer.Key("col_width_id");
        json_writer.StartArray();
        for (int col_id = 0; col_id < getColIdSize(); ++col_id)
            json_writer.Int(m_col_width_id[col_id]);
        json_writer.EndArray();
    }

    if (!m_col_hidden_id.empty()) {
        json_writer.Key("col_hidden_id");
        json_writer.StartArray();
        for (int col_id = 0; col_id < getColIdSize(); ++col_id)
            if (isHiddenColId(col_id)) json_writer.Int(col_id);
        json_writer.EndArray();
    }

    if (!m_sort_col_id.empty() && m_sort_col_id.size() == m_sort_asc.size()) {
        json_writer.Key("sort_col_id");
        json_writer.StartArray();
        for (int i = 0; i < static_cast<int>(m_sort_col_id.size()); ++i)
            json_writer.Int(m_sort_col_id[i]);
        json_writer.EndArray();
        json_writer.Key("sort_asc");
        json_writer.StartArray();
        for (int i = 0; i < static_cast<int>(m_sort_asc.size()); ++i)
            json_writer.Bool(m_sort_asc[i]);
        json_writer.EndArray();
    }

    json_writer.EndObject();
    const wxString& key = "LIST_" + m_setting_name;
    const wxString& value = wxString::FromUTF8(json_buffer.GetString());
    Model_Setting::instance().setRaw(key, value);
}

void mmListCtrl::loadPreferences()
{
    const wxString& key = "LIST_" + m_setting_name;
    Document json_doc = Model_Setting::instance().getJdoc(key, "{}");
    int version = 0;
    if (!json_doc.HasParseError()) {
        JSON_GetIntValue(json_doc, "version", version);
    }

    if (version < 1) {
        loadPreferences_v190();
        savePreferences();
        return;
    }
    else if (version != m_setting_version) {
        return;
    }

    // load m_col_id_nr if columns can be ordered
    Value* col_id_a = JSON_GetValue(json_doc, "col_id_vo");
    if (!m_col_id_nr.empty() && col_id_a != nullptr && col_id_a->IsArray()) {
        std::vector<int> col_id_nr;
        for (SizeType i = 0; i < col_id_a->Size(); ++i) {
            int col_id = (*col_id_a)[i].GetInt();
            if (isValidColId(col_id) &&
                std::find(m_col_id_nr.begin(), m_col_id_nr.end(), col_id) != m_col_id_nr.end() &&
                std::find(col_id_nr.begin(), col_id_nr.end(), col_id) == col_id_nr.end()
            )
                col_id_nr.push_back(col_id);
        }
        // assertion: col_id_nr is a subset of m_col_id_nr
        for (int col_id : m_col_id_nr) {
            if (std::find(col_id_nr.begin(), col_id_nr.end(), col_id) == col_id_nr.end())
                col_id_nr.push_back(col_id);
        }
        // assertion: col_id_nr is a permutation of m_col_id_nr
        m_col_id_nr = col_id_nr;
    }

    // load m_col_width_id
    m_col_width_id.clear();
    Value* col_width_a = JSON_GetValue(json_doc, "col_width_id");
    if (col_width_a != nullptr && col_width_a->IsArray()) {
        for (SizeType col_id = 0; col_id < col_width_a->Size(); ++col_id) {
            if (static_cast<int>(col_id) >= getColIdSize())
                break;
            int col_width = (*col_width_a)[col_id].GetInt();
            m_col_width_id.push_back(col_width);
        }
    }
    for (int col_id = static_cast<int>(m_col_width_id.size()); col_id < getColIdSize(); ++col_id) {
        int col_width = m_col_info_id[col_id].default_width;
        m_col_width_id.push_back(col_width);
    }

    // load m_col_hidden_id
    m_col_hidden_id.clear();
    Value* col_hidden_a = JSON_GetValue(json_doc, "col_hidden_id");
    if (col_hidden_a != nullptr && col_hidden_a->IsArray()) {
        for (SizeType i = 0; i < col_hidden_a->Size(); ++i) {
            int col_id = (*col_hidden_a)[i].GetInt();
            if (isValidColId(col_id))
                m_col_hidden_id.insert(col_id);
        }
    }

    // load m_sort_col_id, m_sort_asc
    if (c_sort_col_nr.size() != m_sort_col_id.size())
        c_sort_col_nr = std::vector<int>(m_sort_col_id.size(), -1);
    if (m_sort_asc.size() != m_sort_col_id.size())
        m_sort_asc = std::vector<bool>(m_sort_col_id.size(), true);
    Value* sort_col_a = JSON_GetValue(json_doc, "sort_col_id");
    Value* sort_asc_a = JSON_GetValue(json_doc, "sort_asc");
    if (sort_col_a != nullptr && sort_col_a->IsArray() &&
        sort_asc_a != nullptr && sort_asc_a->IsArray() &&
        sort_col_a->Size() == sort_asc_a->Size()
    ) {
        for (SizeType i = 0; i < sort_col_a->Size(); ++i) {
            if (static_cast<int>(i) >= static_cast<int>(m_sort_col_id.size()))
                break;
            int col_id = (*sort_col_a)[i].GetInt();
            if (isValidColId(col_id)) {
                m_sort_col_id[i] = col_id;
                m_sort_asc[i] = (*sort_asc_a)[i].GetBool();
            }
        }
    }
}

void mmListCtrl::savePreferences_v190()
{
    // save m_col_id_nr
    if (!m_col_id_nr.empty() && !o_col_order_prefix.empty()) {
        wxString order_str;
        for (int col_id : m_col_id_nr)
            order_str.Append((order_str.IsEmpty() ? "" : "|") + wxString::Format("%i", col_id));
        Model_Setting::instance().setString(getColOrderKey_v190(), order_str);
    }

    // save m_col_width_id, m_col_hidden_id
    if (!o_col_width_prefix.IsEmpty()) {
        for (int col_nr = 0; col_nr < GetColumnCount(); ++col_nr) {
            int col_id = getColId_Nr(col_nr);
            Model_Setting::instance().setInt(
                getColWidthKey_v190(col_id), GetColumnWidth(col_nr)
            );
        }
    }

    // save m_sort_col_id, m_sort_asc
    bool asc_isInt = (m_sort_col_id.size() == 2);
    for (int i = 0; i < static_cast<int>(m_sort_col_id.size()); ++i) {
        Model_Setting::instance().setInt(getSortColKey_v190(i), getSortColNr(i));
        saveBoolInt(getSortAscKey_v190(i), getSortAsc(i), asc_isInt);
    }
}

void mmListCtrl::loadPreferences_v190()
{
    // load m_col_id_nr if columns can be ordered
    if (!m_col_id_nr.empty() && !o_col_order_prefix.empty()) {
        wxString order_str = Model_Setting::instance().getString(getColOrderKey_v190(), "");
        wxArrayString col_idstr_nr = wxSplit(order_str, '|');
        std::vector<int> col_id_nr;
        for (const auto& col_idstr : col_idstr_nr) {
            int col_id = wxAtoi(col_idstr);
            if (isValidColId(col_id) &&
                std::find(m_col_id_nr.begin(), m_col_id_nr.end(), col_id) != m_col_id_nr.end() &&
                std::find(col_id_nr.begin(), col_id_nr.end(), col_id) == col_id_nr.end()
            )
                col_id_nr.push_back(col_id);
        }
        // assertion: col_id_nr is a subset of m_col_id_nr
        for (int col_id : m_col_id_nr) {
            if (std::find(col_id_nr.begin(), col_id_nr.end(), col_id) == col_id_nr.end())
                col_id_nr.push_back(col_id);
        }
        // assertion: col_id_nr is a permutation of m_col_id_nr
        m_col_id_nr = col_id_nr;
    }

    // load m_col_width_id, m_col_hidden_id
    m_col_width_id.clear();
    m_col_hidden_id.clear();
    for (int col_id = 0; col_id < getColIdSize(); ++col_id) {
        ListColumnInfo col_info = m_col_info_id[col_id];
        int col_width = col_info.default_width;
        if (!o_col_width_prefix.empty()) {
            col_width = Model_Setting::instance().getInt(
                getColWidthKey_v190(col_id), col_info.default_width
            );
        }
        if (col_width == 0) {
            m_col_hidden_id.insert(col_id);
            col_width = col_info.default_width;
        }
        m_col_width_id.push_back(col_width);
    }

    // load m_sort_col_id, m_sort_asc
    // NOTE:
    //   *_SORT_COL* represents a col_nr in TransactionListCtrl (others have a bug).
    //   *_ASC* is stored as Int in TransactionListCtrl, or as Bool otherwise.
    //   Only TransactionListCtrl has two sorting columns.
    bool asc_isInt = (m_sort_col_id.size() == 2);
    if (c_sort_col_nr.size() != m_sort_col_id.size())
        c_sort_col_nr = std::vector<int>(m_sort_col_id.size(), -1);
    if (m_sort_asc.size() != m_sort_col_id.size())
        m_sort_asc = std::vector<bool>(m_sort_col_id.size(), true);
    for (int i = 0; i < static_cast<int>(m_sort_col_id.size()); ++i) {
        int col_nr = Model_Setting::instance().getInt(getSortColKey_v190(i), -1);
        int col_id = isValidColNr(col_nr) ? getColId_Nr(col_nr) : m_sort_col_id[i];
        bool asc = loadBoolInt(getSortAscKey_v190(i), m_sort_asc[i], asc_isInt);
        if (isValidColId(col_id)) {
            m_sort_col_id[i] = col_id;
            m_sort_asc[i] = asc;
        }
    }
}

void mmListCtrl::saveBoolInt(const wxString& key, bool value, bool isInt)
{
    if (isInt)
        Model_Setting::instance().setInt(key, (value ? 1 : 0));
    else
        Model_Setting::instance().setBool(key, value);
}

bool mmListCtrl::loadBoolInt(const wxString& key, bool default_value, bool isInt) const
{
    return isInt ?
        Model_Setting::instance().getInt(key, (default_value ? 1 : 0)) != 0 :
        Model_Setting::instance().getBool(key, default_value);
}

//----------------------------------------------------------------------------

int mmListCtrl::cacheSortColNr(int i)
{
    int col_id = m_sort_col_id[i];
    int col_nr = isValidColId(col_id) ? getColNr_Id(col_id) : -1;
    c_sort_col_nr[i] = col_nr;
    return col_nr;
}

//----------------------------------------------------------------------------

wxListItemAttr* mmListCtrl::OnGetItemAttr(long row) const
{
    return (row % 2) ? attr2_.get() : attr1_.get();
}

void mmListCtrl::OnColClick(wxListEvent& WXUNUSED(event))
{
    // To be implemented in derived class
}

int mmListCtrl::getSortIcon(bool WXUNUSED(asc)) const
{
    return -1;
}

void mmListCtrl::updateSortIcon()
{
    if (m_sort_col_id.empty())
        return;

    wxListItem item;
    item.SetMask(wxLIST_MASK_IMAGE);

    // clear sort icon in c_icon_col_nr
    if (isValidColNr(c_icon_col_nr)) {
        item.SetImage(-1);
        SetColumn(c_icon_col_nr, item);
    }

    c_icon_col_nr = getSortColNr();

    // show sort icon in c_icon_col_nr
    if (isValidColNr(c_icon_col_nr)) {
        item.SetImage(getSortIcon(getSortAsc()));
        SetColumn(c_icon_col_nr, item);
    }
}

//----------------------------------------------------------------------------

void mmListCtrl::shiftColumn(int col_vo, int offset)
{
    if (m_col_id_nr.empty() || offset == 0)
        return;

    int col_nr = getColNr_Vo(col_vo);
    int col_id = m_col_id_nr[col_nr];
    wxLogDebug("mmListCtrl::shiftColumn(): save column %d:%d (%s)",
        col_vo, col_nr, m_col_info_id[col_id].header
    );
    wxListItem col_item;
    col_item.SetText(getColHeader(col_id));
    col_item.SetAlign(static_cast<wxListColumnFormat>(m_col_info_id[col_id].format));
    int col_width = GetColumnWidth(col_nr);

    int dir = offset > 0 ? 1 : -1;
    int dst_vo = col_vo;
    int dst_nr = getColNr_Vo(dst_vo);
    while (offset != 0) {
        int src_vo = dst_vo + dir;
        int src_nr = getColNr_Vo(src_vo);
        if (!isValidColNr(src_nr))
            break;
        int src_id = m_col_id_nr[src_nr];
        wxLogDebug("mmListCtrl::shiftColumn(): move column %d:%d (%s) -> %d:%d",
            src_vo, src_nr, m_col_info_id[src_id].header, dst_vo, dst_nr
        );
        wxListItem src_item;
        src_item.SetText(getColHeader(src_id));
        src_item.SetAlign(static_cast<wxListColumnFormat>(m_col_info_id[src_id].format));
        int src_width = GetColumnWidth(src_nr);
        SetColumn(dst_nr, src_item);
        SetColumnWidth(dst_nr, src_width);
        m_col_id_nr[dst_nr] = src_id;
        dst_vo = src_vo;
        dst_nr = src_nr;
        offset -= dir;
    }

    wxLogDebug("mmListCtrl::shiftColumn(): restore column (%s) -> %d:%d",
        m_col_info_id[col_id].header, dst_vo, dst_nr
    );
    SetColumn(dst_nr, col_item);
    SetColumnWidth(dst_nr, col_width);
    m_col_id_nr[dst_nr] = col_id;
}

//----------------------------------------------------------------------------

void mmListCtrl::onItemResize(wxListEvent& event)
{
    // update m_col_width_id but do not save in Setting
    int col_nr = event.GetColumn();
    int col_id = getColId_Nr(col_nr);
    if (m_col_width_id.size() > static_cast<long unsigned int>(col_id)) {  // Prevent seg fault!
        m_col_width_id[col_id] = GetColumnWidth(col_nr);
    }
}

void mmListCtrl::onColRightClick(wxListEvent& event)
{
    m_sel_col_nr = event.GetColumn();
    if (!isValidColNr(m_sel_col_nr))
        return;

    wxMenu menu;
    // hide and show columns
    wxMenu* menu_toggle = new wxMenu;
    for (int col_vo = 0; col_vo < getColNrSize(); col_vo++) {
        int col_nr = getColNr_Vo(col_vo);
        int col_id = getColId_Nr(col_nr);
        if (isDisabledColId(col_id))
            continue;
        int event_id = MENU_HEADER_TOGGLE_MIN + col_nr;
        if (event_id > MENU_HEADER_TOGGLE_MAX)
            break;
        menu_toggle->AppendCheckItem(event_id, getColHeader(col_id, true));
        menu_toggle->Check(event_id, !isHiddenColId(col_id));
    }
    menu.AppendSubMenu(menu_toggle, _t("Hide/Show column"));
    menu.Append(MENU_HEADER_HIDE, _t("Hide this column"));

    // move columns
    if (m_col_id_nr.size() > 0) {
        wxMenu* menu_show = new wxMenu;
        bool found = false;
        for (int col_vo = 0; col_vo < getColNrSize(); col_vo++) {
            int col_nr = getColNr_Vo(col_vo);
            int col_id = getColId_Nr(col_nr);
            if (isDisabledColId(col_id))
                continue;
            int event_id = MENU_HEADER_SHOW_MIN + col_nr;
            if (event_id > MENU_HEADER_SHOW_MAX)
                break;
            if (isHiddenColId(col_id)) {
                menu_show->Append(event_id, getColHeader(col_id, true));
                found = true;
            }
        }
        if (found)
            menu.AppendSubMenu(menu_show, _t("Move hidden column"));
        if (m_sel_col_nr > 0)
            menu.Append(MENU_HEADER_MOVE_LEFT, _t("Move column left"));
        if (m_sel_col_nr < getColNrSize() - 1)
            menu.Append(MENU_HEADER_MOVE_RIGHT, _t("Move column right"));
    }

    if (m_col_info_id[getColId_Nr(m_sel_col_nr)].sortable)
        menu.Append(MENU_HEADER_SORT, _t("Sort by this column"));

    menu.Append(MENU_HEADER_RESET, _t("Reset column widths"));

    PopupMenu(&menu);
    SetFocus();
}

void mmListCtrl::onHeaderPopup(wxCommandEvent& event)
{
    int event_id = event.GetId();
    if (event_id == MENU_HEADER_SORT)
        onHeaderSort(event);
    else if (event_id >= MENU_HEADER_TOGGLE_MIN && event_id <= MENU_HEADER_TOGGLE_MAX)
        onHeaderToggle(event);
    else if (event_id == MENU_HEADER_HIDE)
        onHeaderHide(event);
    else if (event_id >= MENU_HEADER_SHOW_MIN && event_id <= MENU_HEADER_SHOW_MAX)
        onHeaderShow(event);
    else if (event_id == MENU_HEADER_MOVE_LEFT)
        onHeaderMove(event, -1);
    else if (event_id == MENU_HEADER_MOVE_RIGHT)
        onHeaderMove(event, 1);
    else if (event_id == MENU_HEADER_RESET)
        onHeaderReset(event);
}

void mmListCtrl::onHeaderSort(wxCommandEvent& WXUNUSED(event))
{
    wxListEvent e;
    e.SetId(MENU_HEADER_SORT);
    OnColClick(e);
}

void mmListCtrl::onHeaderToggle(wxCommandEvent& event)
{
    int col_nr = event.GetId() - MENU_HEADER_TOGGLE_MIN;
    if (!isValidColNr(col_nr))
        return;
    Freeze();
    int col_id = getColId_Nr(col_nr);
    int new_width;
    if (isHiddenColId(col_id)) {
        m_col_hidden_id.erase(col_id);
        new_width = m_col_width_id[col_id];
        if (new_width == 0) new_width = m_col_info_id[col_id].default_width;
    }
    else {
        m_col_width_id[col_id] = GetColumnWidth(col_nr);
        m_col_hidden_id.insert(col_id);
        new_width = 0;
    }
    SetColumnWidth(col_nr, new_width);
    savePreferences();
    Thaw();
}

void mmListCtrl::onHeaderHide(wxCommandEvent& WXUNUSED(event))
{
    if (!isValidColNr(m_sel_col_nr))
        return;
    Freeze();
    int col_id = getColId_Nr(m_sel_col_nr);
    m_col_width_id[col_id] = GetColumnWidth(m_sel_col_nr);
    m_col_hidden_id.insert(col_id);
    SetColumnWidth(m_sel_col_nr, 0);
    savePreferences();
    Thaw();
}

void mmListCtrl::onHeaderShow(wxCommandEvent& event)
{
    int col_nr = event.GetId() - MENU_HEADER_SHOW_MIN;
    if (m_col_id_nr.empty() || !isValidColNr(m_sel_col_nr) || !isValidColNr(col_nr))
        return;
    Freeze();

    // show col_nr
    int col_id = getColId_Nr(col_nr);
    m_col_hidden_id.erase(col_id);
    int col_width = m_col_width_id[col_id];
    if (col_width == 0) col_width = m_col_info_id[col_id].default_width;
    SetColumnWidth(col_nr, col_width);

    // move col_nr to the right of m_sel_col_nr
    int col_vo = getColVo_Nr(col_nr);
    int offset = getColVo_Nr(m_sel_col_nr) - col_vo;
    if (offset < 0) offset++;
    shiftColumn(col_vo, offset);

    updateSortIcon();
    savePreferences();
    Thaw();
}

void mmListCtrl::onHeaderMove(wxCommandEvent& WXUNUSED(event), int dir)
{
    if (m_col_id_nr.empty())
        return;
    Freeze();

    // find the next visible column after m_sel_col_nr in direction dir
    int src_nr = m_sel_col_nr;
    int src_vo = getColVo_Nr(src_nr);
    int dst_vo = src_vo;
    int dst_nr = src_nr;
    do { dst_vo += dir; dst_nr = getColNr_Vo(dst_vo); } while (
        isValidColNr(dst_nr) && (isDisabledColNr(dst_nr) || isHiddenColNr(dst_nr))
    );
    // shift src_vo to dst_vo
    if (isValidColNr(dst_nr)) {
        shiftColumn(src_vo, dst_vo - src_vo);
        updateSortIcon();
        savePreferences();
    }

    Thaw();
}

void mmListCtrl::onHeaderReset(wxCommandEvent& WXUNUSED(event))
{
    Freeze();
    m_col_hidden_id.clear();
    for (int col_nr = 0; col_nr < getColNrSize(); ++col_nr) {
        int col_id = getColId_Nr(col_nr);
        if (isDisabledColId(col_id))
            continue;
        int col_width = m_col_info_id[col_id].default_width;
        m_col_width_id[col_id] = col_width;
        SetColumnWidth(col_nr, col_width);
    }
    savePreferences();
    Thaw();
}

//----------------------------------------------------------------------------

mmPanelBase::mmPanelBase()
{
}

mmPanelBase::~mmPanelBase()
{
}

wxString mmPanelBase::BuildPage() const
{
    mmReportsPanel* rp = wxDynamicCast(this, mmReportsPanel);
    return rp ? rp->getPrintableBase()->getHTMLText() : "TBD";
}

void mmPanelBase::PrintPage()
{
    //this->Freeze();
    wxWebView*  htmlWindow = wxWebView::New(this, wxID_ANY);
    htmlWindow->SetPage(BuildPage(), "");
    htmlWindow->GetPageSource(); // Needed to generate the page - at least on Mac anyway!
    htmlWindow->Print();
    htmlWindow->Destroy();
    //this->Thaw();
}

void mmPanelBase::windowsFreezeThaw()
{
    #ifdef __WXGTK__
    return;
    #endif

    if (this->IsFrozen())
        this->Thaw();
    else
        this->Freeze();
}

