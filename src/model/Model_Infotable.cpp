/*******************************************************
 Copyright (C) 2013,2014 Guan Lisheng (guanlisheng@gmail.com)
 Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)


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

#include "Model_Infotable.h"
#include "constants.h"
#include "util.h"

Model_Infotable::Model_Infotable()
: Model<DB_Table_INFOTABLE_V1>()
{
}

Model_Infotable::~Model_Infotable()
{
}

// Initialize the global Model_Infotable.
// Reset the Model_Infotable or create the table if it does not exist.
Model_Infotable& Model_Infotable::instance(wxSQLite3Database* db)
{
    Model_Infotable& ins = Singleton<Model_Infotable>::instance();
    ins.db_ = db;
    ins.destroy_cache();
    ins.ensure(db);
    ins.preload();
    if (!ins.contains("MMEXVERSION")) {
        ins.setString("MMEXVERSION", mmex::version::string);
        ins.setString("DATAVERSION", mmex::DATAVERSION);
        ins.setDate("CREATEDATE", wxDateTime::Now());
        ins.setString("DATEFORMAT", mmex::DEFDATEFORMAT);
    }
    return ins;
}

// Return the static instance of Model_Infotable
Model_Infotable& Model_Infotable::instance()
{
    return Singleton<Model_Infotable>::instance();
}

// Returns true if key setting found
bool Model_Infotable::contains(const wxString& key)
{
    return !find(INFONAME(key)).empty();
}

// Raw (the raw value stored in Infotable is always string)
void Model_Infotable::setRaw(const wxString& key, const wxString& newValue)
{
    // search in cache
    Data* info = get_one(INFONAME(key));
    if (!info) {
        // not found in cache; search in db
        Data_Set items = find(INFONAME(key));
        if (!items.empty())
            info = get(items[0].INFOID);
        if (!info) {
            // not found; create
            info = create();
            info->INFONAME = key;
        }
    }
    info->INFOVALUE = newValue;
    info->save(db_);
}
wxString Model_Infotable::getRaw(const wxString& key, const wxString& defaultValue)
{
    // search in cache
    Data* info = get_one(INFONAME(key));
    if (info)
        return info->INFOVALUE;
    // search in db
    Data_Set items = find(INFONAME(key));
    if (!items.empty())
        return items[0].INFOVALUE;
    // not found
    return defaultValue;
}

// String
void Model_Infotable::setString(const wxString& key, const wxString& newValue)
{
    setRaw(key, newValue);
}
wxString Model_Infotable::getString(const wxString& key, const wxString& defaultValue)
{
    return getRaw(key, defaultValue);
}

// Bool
void Model_Infotable::setBool(const wxString& key, bool newValue)
{
    setRaw(key, wxString::Format("%s", newValue ? "TRUE" : "FALSE"));
}
bool Model_Infotable::getBool(const wxString& key, bool defaultValue)
{
    const wxString rawValue = getRaw(key, "");
    if (rawValue == "1" || rawValue.CmpNoCase("TRUE") == 0)
        return true;
    else if (rawValue == "0" || rawValue.CmpNoCase("FALSE") == 0)
        return false;
    else
        return defaultValue;
}

// Int
void Model_Infotable::setInt(const wxString& key, int newValue)
{
    setRaw(key, wxString::Format("%d", newValue));
}
int Model_Infotable::getInt(const wxString& key, int defaultValue)
{
    const wxString rawValue = getRaw(key, "");
    if (!rawValue.IsEmpty() && rawValue.IsNumber())
        return wxAtoi(rawValue);
    return defaultValue;
}

// Int64
void Model_Infotable::setInt64(const wxString& key, int64 newValue)
{
    setRaw(key, wxString::Format("%lld", newValue));
}
int64 Model_Infotable::getInt64(const wxString& key, int64 defaultValue)
{
    const wxString rawValue = getString(key, "");
    if (!rawValue.IsEmpty() && rawValue.IsNumber())
        return int64(wxAtol(rawValue));
    return defaultValue;
}

// Size
void Model_Infotable::setSize(const wxString& key, const wxSize& newValue)
{
    setRaw(key, wxString::Format("%i,%i", newValue.GetWidth(), newValue.GetHeight()));
}
const wxSize Model_Infotable::getSize(const wxString& key)
{
    const wxString rawValue = getRaw(key, "");
    if (!rawValue.IsEmpty()) {
        wxRegEx pattern("^([0-9]+),([0-9]+)$");
        if (pattern.Matches(rawValue)) {
            const auto& x = pattern.GetMatch(rawValue, 1);
            const auto& y = pattern.GetMatch(rawValue, 2);
            return wxSize(wxAtoi(x), wxAtoi(y));
        }
    }
    return wxDefaultSize;
}

// Colour
void Model_Infotable::setColour(const wxString& key, const wxColour& newValue)
{
    setRaw(key, wxString::Format("%d,%d,%d",
        newValue.Red(), newValue.Green(), newValue.Blue()
    ));
}
const wxColour Model_Infotable::getColour(const wxString& key, const wxColour& defaultValue)
{
    const wxString rawValue = getRaw(key, "");
    if (!rawValue.IsEmpty()) {
        wxRegEx pattern("([0-9]{1,3}),([0-9]{1,3}),([0-9]{1,3})");
        if (pattern.Matches(rawValue)) {
            const wxString r = pattern.GetMatch(rawValue, 1);
            const wxString g = pattern.GetMatch(rawValue, 2);
            const wxString b = pattern.GetMatch(rawValue, 3);
            return wxColour(wxAtoi(r), wxAtoi(g), wxAtoi(b));
        }
        else {
            return wxColour(rawValue);
        }
    }
    return defaultValue;
}

// Date
void Model_Infotable::setDate(const wxString& key, const wxDateTime& newValue)
{
    setRaw(key, newValue.FormatISODate());
}

//-------------------------------------------------------------------
// Jdoc
void Model_Infotable::setJdoc(const wxString& key, Document& newValue)
{
    wxString j_str = JSON_PrettyFormated(newValue);
    setRaw(key, j_str);
}
void Model_Infotable::setJdoc(const wxString& key, StringBuffer& newValue)
{
    wxString j_str = wxString::FromUTF8(newValue.GetString());
    setRaw(key, j_str);
}
Document Model_Infotable::getJdoc(const wxString& key, const wxString& defaultValue)
{
    Document j_doc;
    wxString j_str = getRaw(key, defaultValue);
    j_doc.Parse(j_str.utf8_str());
    return j_doc;
}

// ArrayString
void Model_Infotable::setArrayString(const wxString& key, const wxArrayString& a)
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    json_writer.StartArray();
    for (const auto& value : a) {
        json_writer.String(value.utf8_str());
    }
    json_writer.EndArray();
    const wxString& json_string = wxString::FromUTF8(json_buffer.GetString());
    setRaw(key, json_string);
    wxLogDebug("Model_Infotable::setArrayString(%s): %s", key, json_string);
}

const wxArrayString Model_Infotable::getArrayString(const wxString& key, bool sort)
{
    wxString rawValue = getRaw(key, "");
    Document j_doc;
    if (rawValue.IsEmpty() ||
        j_doc.Parse(rawValue.utf8_str()).HasParseError() ||
        !j_doc.IsArray()
    )
        return wxArrayString();

    wxArrayString a;
    wxLogDebug("{{{ Model_Infotable::getArrayString(%s)", key);
    for (rapidjson::SizeType i = 0; i < j_doc.Size(); i++) {
        wxASSERT(j_doc[i].IsString());
        const auto value = wxString::FromUTF8(j_doc[i].GetString());
        wxLogDebug("%s", value);
        a.Add(value);
    }
    wxLogDebug("}}}");

    // Crude sort of JSON (case sensitive), could be improved by actually sorting by a field
    // but this should be sufficient if you want to sort by first element
    if (sort)
        a.Sort();
    return a;
}

// Search through a set of JSON data for a particular label
int Model_Infotable::findArrayItem(const wxString& key, const wxString& label)
{
    // Important: do not sort
    wxArrayString a = Model_Infotable::instance().getArrayString(key);
    int i = 0;
    for (const auto& data : a) {
        Document j_doc;
        if (j_doc.Parse(data.utf8_str()).HasParseError())
            j_doc.Parse("{}");
        Value& j_label = GetValueByPointerWithDefault(j_doc, "/LABEL", "");
        const wxString& s_label = j_label.IsString() ? wxString::FromUTF8(j_label.GetString()) : "";
        if (s_label == label)
            return i;
        ++i;
    }
    return wxNOT_FOUND;
}

void Model_Infotable::updateArrayItem(const wxString& key, int i, const wxString& newValue)
{
    wxString rawValue = getRaw(key, "");
    Document j_doc;
    if (rawValue.IsEmpty() ||
        j_doc.Parse(rawValue.utf8_str()).HasParseError() ||
        !j_doc.IsArray()
    )
        return;

    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    json_writer.StartArray();
    for (SizeType j = 0; j < j_doc.Size(); j++) {
        json_writer.String(
            (i == static_cast<int>(j)) ? newValue.utf8_str() : j_doc[j].GetString()
        );
    }
    json_writer.EndArray();
    const wxString& json_string = wxString::FromUTF8(json_buffer.GetString());

    setRaw(key, json_string);
    wxLogDebug("Model_Infotable::updateArrayItem(%s, %d): %s", key, i, json_string);
}

void Model_Infotable::prependArrayItem(const wxString& key, const wxString& value, int limit)
{
    Data* info = get_one(INFONAME(key));
    if (!info) { // not cached
        Data_Set items = find(INFONAME(key));
        if (!items.empty())
            info = get(items[0].INFOID);
        if (!info) {
            info = create();
            info->INFONAME = key;
        }
    }
    wxArrayString a;
    if (!value.empty() && limit != 0)
        a.Add(value);

    Document j_doc;
    if (!j_doc.Parse(info->INFOVALUE.utf8_str()).HasParseError()
        && j_doc.IsArray()
    ) {
        int i = 1;
        for (auto& v : j_doc.GetArray()) {
            if (i >= limit && limit != -1) break;
            if (v.IsString()) {
                const auto item = wxString::FromUTF8(v.GetString());
                if (a.Index(item) == wxNOT_FOUND) {
                    a.Add(item);
                    i++;
                }
            }
        }
    }

    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    json_writer.StartArray();
    for (const auto& entry : a) {
        json_writer.String(entry.utf8_str());
    }
    json_writer.EndArray();

    info->INFOVALUE = wxString::FromUTF8(json_buffer.GetString());
    info->save(db_);
}

void Model_Infotable::eraseArrayItem(const wxString& key, int i)
{
    wxString rawValue = getRaw(key, "");
    Document j_doc;
    if (rawValue.IsEmpty() ||
        j_doc.Parse(rawValue.utf8_str()).HasParseError() ||
        !j_doc.IsArray()
    )
        return;

    j_doc.Erase(j_doc.Begin() + i);
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    j_doc.Accept(json_writer);

    const wxString json_string = wxString::FromUTF8(json_buffer.GetString());
    setRaw(key, json_string);
    wxLogDebug("Model_Infotable::eraseArrayItem(%s, %d): %s", key, i, json_string);
}

//-------------------------------------------------------------------
// CUSTOMDIALOG_OPEN
void Model_Infotable::setOpenCustomDialog(const wxString& refType, bool newValue)
{
    setBool("CUSTOMDIALOG_OPEN:" + refType, newValue);
}
bool Model_Infotable::getOpenCustomDialog(const wxString& refType)
{
    return getBool("CUSTOMDIALOG_OPEN:" + refType, false);
}

// CUSTOMDIALOG_SIZE
void Model_Infotable::setCustomDialogSize(const wxString& refType, const wxSize& newValue)
{
    wxString rawValue;
    rawValue << newValue.GetWidth() << ";" << newValue.GetHeight();
    setRaw("CUSTOMDIALOG_SIZE:" + refType, rawValue);
}
wxSize Model_Infotable::getCustomDialogSize(const wxString& refType)
{
    wxString rawValue = getRaw("CUSTOMDIALOG_SIZE:" + refType, "0;0");
    return wxSize(wxAtoi(rawValue.BeforeFirst(';')), wxAtoi(rawValue.AfterFirst(';')));
}

//-------------------------------------------------------------------
bool Model_Infotable::checkDBVersion()
{
    if (!contains("DATAVERSION"))
        return false;
    return getInt("DATAVERSION", 0) >= mmex::MIN_DATAVERSION;
}

loop_t Model_Infotable::to_loop_t()
{
    loop_t loop;
    for (const auto &r: instance().all())
        loop += r.to_row_t();
    return loop;
}
