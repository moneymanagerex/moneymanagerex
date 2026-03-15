/*******************************************************
 Copyright (C) 2013,2014 Guan Lisheng (guanlisheng@gmail.com)
 Copyright (C) 2026 Klaus Wich

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

#include "base/constants.h"
#include "base/platfdep.h"
#include "base/paths.h"

#include "ReportModel.h"
#include "SettingModel.h"
#include "PrefModel.h"

#include "dialog/AttachmentDialog.h"
#include "panel/ReportPanel.h"
#include "report/htmlbuilder.h"

#include <LuaGlue/LuaGlue.h>
#include <sqlite3mc_amalgamation.h>
#include <wx/fs_mem.h>

#if defined (__WXMSW__)
    #include <wx/msw/registry.h>
#endif

const std::vector<ReportParam> ReportParam::get_param_a()
{
    const wxString def_date = wxDateTime::Today().FormatISODate();
    const wxString def_time = wxDateTime::Now().FormatISOTime();

    const std::vector<ReportParam> param_a = {
        { "&begin_date", "mmDatePickerCtrl", def_date,
            ReportPanel::ID_START_DATE_PICKER, _t("Begin date:") },
        { "&end_date", "mmDatePickerCtrl", def_date,
            ReportPanel::ID_END_DATE_PICKER, _t("End date:") },
        { "&single_date", "mmDatePickerCtrl", def_date,
            ReportPanel::RepPanel::ID_SINGLE_DATE_PICKER, _t("Date:") },
        { "&single_time", "wxTimePickerCtrl", def_time,
            ReportPanel::ID_TIME_PICKER, _t("Time:") },
        { "&only_years", "wxChoice", def_date,
            ReportPanel::ID_YEAR_CHOICE, _t("Year:")},
        { "&account_selection", "wxChoice", def_date,
            ReportPanel::ID_ACCOUNT_CHOICE, _t("Accounts:")},
        { "&stock_selection", "wxChoice", "",
            ReportPanel::ID_STOCK_CHOICE, _t("Stock name:")},
        { "&filter", "wxTextCtrl", "",
            ReportPanel::ID_FILTER_GENERIC_CHOICE, _t("Filter")},
        { "&selection", "wxChoice", "",
            ReportPanel::ID_SELECTION_GENERIC_CHOICE, _t("Selection")}
    };
    return param_a;
}

const std::vector<std::pair<wxString, wxString>> ReportParam::get_label_name_a()
{
    std::vector<std::pair<wxString, wxString>> label_name_a;
    for (const auto& param : get_param_a()) {
        label_name_a.emplace_back(param.label, param.name);
    }
    return label_name_a;
}

// Search and replace in query all parameter placeholders with their values.
// Add the parameter values into label_value_m, indexed by the parameter label.
// The parameter substitutions are specified in get_param_a().
// Return false if query is empty.
bool ReportParam::prepare_sql(wxString& query, std::map<wxString, wxString>& label_value_m)
{
    query.Trim();
    if (query.empty())
        return false;
    if (query.Last() != ';')
        query += ';';

    for (const auto& param : get_param_a()) {
        int pos = query.Lower().Find(param.label);
        if (pos == wxNOT_FOUND)
            continue;

        wxString value = param.def_value;
        const auto w = wxWindow::FindWindowById(param.ID);
        //const auto name = w->GetClassInfo()->GetClassName();
        if (w && param.type == "mmDatePickerCtrl") {
            mmDatePickerCtrl* date = static_cast<mmDatePickerCtrl*>(w);
            value = date->GetValue().FormatISODate();
        }
        else if (w && param.type == "wxTimePickerCtrl") {
            wxTimePickerCtrl* time = static_cast<wxTimePickerCtrl*>(w);
            value = time->GetValue().FormatISOTime();
        }
        else if (w && param.type == "wxChoice") {
            wxChoice* choice = static_cast<wxChoice*>(w);
            value = choice->GetStringSelection();
        }
        else if (w && param.type == "wxTextCtrl") {
            wxTextCtrl* txt_c = static_cast<wxTextCtrl*>(w);
            value = txt_c->GetValue();
        }

        label_value_m[param.label.Mid(1)] = value;

        size_t len = wxString(param.label).size();
        while (pos != wxNOT_FOUND) {
            query.replace(pos, len, value);
            pos = query.Lower().Find(param.label);
        }
    }

    return true;
}

ReportModel::ReportModel() :
    TableFactory<ReportTable, ReportData>()
{
}

ReportModel::~ReportModel()
{
}

// Initialize the single ReportModel table.
// Reset the ReportModel table or create the table if it does not exist.
ReportModel& ReportModel::instance(wxSQLite3Database* db)
{
    ReportModel& ins = Singleton<ReportModel>::instance();
    ins.reset_cache();
    ins.m_db = db;
    ins.ensure_table();

    return ins;
}

// Return the single instance of ReportModel table
ReportModel& ReportModel::instance()
{
    return Singleton<ReportModel>::instance();
}

const ReportData* ReportModel::get_name_data_n(const wxString& name)
{
    const Data* report_n = search_cache_n(ReportCol::REPORTNAME(name));
    if (report_n)
        return report_n;

    DataA report_a = find(ReportCol::REPORTNAME(name));
    if (!report_a.empty())
        report_n = get_id_data_n(report_a[0].m_id);
    return report_n;
}

const wxArrayString ReportModel::find_all_group_name_a()
{
    wxArrayString group_name_a;
    wxString previous_group_name;
    for (const auto& report_d : find_all(Col::COL_ID_GROUPNAME)) {
        if (report_d.m_group_name != previous_group_name) {
            group_name_a.Add(report_d.m_group_name);
            previous_group_name = report_d.m_group_name;
        }
    }
    return group_name_a;
}

// Execute query and write the results into json_writer.
// Return false in case of error.
bool ReportModel::sql_result_as_json(
    const wxString& query,
    PrettyWriter<StringBuffer>& json_writer
) {
    wxSQLite3Statement stmt;
    try {
        stmt = this->m_db->PrepareStatement(query);
        if (!stmt.IsReadOnly()) {
            json_writer.Key("msg");
            json_writer.String("the sql is not readonly");
            return false;
        }
    }
    catch (const wxSQLite3Exception& e) {
        json_writer.Key("msg");
        json_writer.String(e.GetMessage().utf8_str());
        return false;
    }

    json_writer.Key("results");
    json_writer.StartArray();

    try {
        wxSQLite3ResultSet q = stmt.ExecuteQuery();
        int n = q.GetColumnCount();
        while (q.NextRow()) {
            json_writer.StartObject();
            for (int i = 0; i < n; ++i) {
                wxString column_name = q.GetColumnName(i);
                json_writer.Key(column_name.utf8_str());
                switch (q.GetColumnType(i))
                {
                case WXSQLITE_INTEGER:
                    json_writer.Int(q.GetInt(i));
                    break;
                case WXSQLITE_FLOAT:
                    json_writer.Double(q.GetDouble(i));
                    break;
                default:
                    json_writer.String(q.GetString(i).utf8_str());
                    break;
                }
            }
            json_writer.EndObject();
        }
        q.Finalize();

    }
    catch (const wxSQLite3Exception& e) {
        json_writer.Key("msg");
        json_writer.String(e.GetMessage().utf8_str());
        return false;
    }

    json_writer.EndArray();
    return true;
}

// Execute sql query, execute lua content, and create html output into out.
// Return 0 on success, or a non-zero error code.
int ReportModel::generate_html(const Data& report_d, wxString& out)
{
    wxString query = report_d.m_sql_content;
    wxString template_content = report_d.m_template_content;
    if (template_content.empty()) {
        out = _t("Template is empty");
        return 3;
    }

    wxSQLite3ResultSet q;
    int column_c = 0;
    std::map <wxString, wxString> label_value_m;
    try {
        ReportParam::prepare_sql(query, label_value_m);

        wxSQLite3Statement stmt = this->m_db->PrepareStatement(query);
        if (!stmt.IsReadOnly()) {
            out = wxString::Format(_t("The SQL script:\n%s\nwill modify database! Aborted!"),
                report_d.m_sql_content
            );
            return -1;
        }
        else {
            q = stmt.ExecuteQuery();
            column_c = q.GetColumnCount();
        }
    }
    catch (const wxSQLite3Exception& e) {
        out = e.GetMessage();
        return e.GetErrorCode();
    }

    // not used
    std::map<std::wstring, int> column_name_type_m;

    mm_html_template report_template(template_content);
    report_d.to_html_template(report_template);
    loop_t contents;
    loop_t errors;
    row_t error;
    loop_t columns;

    for (int i = 0; i < column_c; ++i) {
        int column_type = q.GetColumnType(i);
        const std::wstring column_name = q.GetColumnName(i).ToStdWstring();
        column_name_type_m[column_name] = column_type;
        row_t row;
        row(L"COLUMN") = column_name;
        columns += row;
    }
    report_template(L"COLUMNS") = columns;

    LuaGlue state;
    state.
        Class<ReportRecord>("ReportRecord").
        ctor("new").
        method("get", &ReportRecord::get).
        method("set", &ReportRecord::set).
        end().open().glue();

    bool skip_lua = report_d.m_lua_content.IsEmpty();

    bool lua_status = state.doString(std::string(report_d.m_lua_content.ToUTF8()));
    if (!skip_lua && !lua_status) {
        error(L"ERROR") = wxString("failed to doString : ") + report_d.m_lua_content +
            wxString(" err: ") + wxString(state.lastError());
        errors += error;
    }
    else {
        //state.doString(R"(sys_locale=os.setlocale("", "numeric"); print(os.setlocale("C", "numeric"));)");
    }

    while (q.NextRow()) {
        ReportRecord rec;
        for (int i = 0; i < column_c; ++i) {
            const wxString column_name = q.GetColumnName(i);
            rec[column_name.ToStdWstring()] = q.GetAsString(i);
        }

        if (lua_status && !skip_lua) {
            try {
                state.invokeVoidFunction("handle_record", &rec);
            }
            catch (const std::runtime_error& e) {
                error(L"ERROR") = wxString("failed to call handle_record : ") + wxString(e.what());
                errors += error;
            }
            catch (const std::exception& e) {
                error(L"ERROR") = wxString("failed to call handle_record : ") + wxString(e.what());
                errors += error;
            }
            catch (...) {
                error(L"ERROR") = L"failed to call handle_record ";
                errors += error;
            }
        }
        row_t row;
        for (const auto& rec_field : rec) {
            row(rec_field.first) = rec_field.second;
        }
        contents += row;
    }
    q.Finalize();

    ReportRecord result;
    if (lua_status && !skip_lua) {
        try {
            state.invokeVoidFunction("complete", &result);
        }
        catch (const std::runtime_error& e) {
            error(L"ERROR") = wxString("failed to call complete: ") + wxString(e.what());
            errors += error;
        }
        catch (const std::exception& e) {
            error(L"ERROR") = wxString("failed to call complete: ") + wxString(e.what());
            errors += error;
        }
        catch (...) {
            error(L"ERROR") = L"failed to call complete";
            errors += error;
        }
    }

    for (const auto& result_item : result)
        report_template(result_item.first) = result_item.second;

    if (!skip_lua || lua_status) {
        //state.doString(R"(print(os.setlocale(sys_locale, "numeric"));)");
    }

    report_template(L"CONTENTS") = contents;
    {
        for (const auto& label_value : label_value_m) {
            report_template(label_value.first.Upper().ToStdWstring()) = label_value.second;
        }
        auto att_path = mmex::getPathAttachment(mmAttachmentManage::InfotablePathSetting());
        //javascript does not handle backslashs
        att_path.Replace("\\", "\\\\");
        report_template(L"ATTACHMENTSFOLDER") = att_path;
        auto path_sep = wxString(wxFileName::GetPathSeparator());
        path_sep.Replace("\\", "\\\\");
        report_template(L"FILESEPARATOR") = path_sep;
        report_template(L"LANGUAGE") = PrefModel::instance().getLanguageCode();
        report_template(L"HTMLSCALE") = wxString::Format("%d",
            PrefModel::instance().getHtmlScale()
        );
    }
    report_template(L"ERRORS") = errors;

    try {
        out = report_template.Process();
        formatHTML(out);
    }
    catch (const syntax_ex& e) {
        out = e.what();
        return 1;
    }
    catch (...) {
        out = _t("Caught exception");
        return 2;
    }

    return 0;
}
