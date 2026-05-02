/*******************************************************
Copyright (C) 2014, 2015, 2021 Nikolay Akimov
Copyright (C) 2021 Mark Whalley (mark@ipx.co.uk)
Copyright (C) 2025, 2026 Klaus Wich

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
*******************************************************/

#include "mmImage.h"

#include <memory>
#include <map>
#include <array>

#include <wx/image.h>
#include <wx/bitmap.h>
#include <wx/sharedptr.h>
#include <wx/zipstrm.h>
#include <wx/rawbmp.h>
#include <wx/fs_mem.h>
#include <wx/mstream.h>

#include "base/mmPlatform.h"
#include "base/mmUserColor.h"
#include "mmPath.h"
#include "_util.h"

#include "model/SettingModel.h"
#include "model/PrefModel.h"

#include "dialog/IconManagerDialog.h"

const int mmImage::bitmapButtonSize = 16;
const std::vector<std::pair<int, int>> mmImage::sizes = {
    { 0, 16 },
    { 1, 24 },
    { 2, 32 },
    { 3, 48 },
};

// SVG filename in Zip, the PNG enum to which it relates, whether to recolor background
const std::map<std::string, std::pair<int, bool>> mmImage::iconName2enum = {
    // Toolbar
    { "NEW_DB.svg",             { png::NEW_DB,             false } },
    { "OPEN.svg",               { png::OPEN,               false } },
    { "NEW_ACC.svg",            { png::NEW_ACC,            false } },
    { "HOME.svg",               { png::HOME,               false } },
    { "CATEGORY.svg",           { png::CATEGORY,           false } },
    { "PAYEE.svg",              { png::PAYEE,              false } },
    { "CURR.svg",               { png::CURR,               false } },
    { "TAG.svg",                { png::TAG,                false } },
    { "FILTER.svg",             { png::FILTER,             false } },
    { "GRM.svg",                { png::GRM,                false } },
    { "OPTIONS.svg",            { png::OPTIONS,            false } },
    { "NEW_TRX.svg",            { png::NEW_TRX,            false } },
    { "NEW_NEWS.svg",           { png::NEW_NEWS,           false } },
    { "NEWS.svg",               { png::NEWS,               false } },
    { "CURRATES.svg",           { png::CURRATES,           false } },
    { "FULLSCREEN.svg",         { png::FULLSCREEN,         false } },
    { "PRINT.svg",              { png::PRINT,              false } },
    { "ABOUT.svg",              { png::ABOUT,              false } },
    { "HELP.svg",               { png::HELP,               false } },

    // Navigation
    { "NAV_HOME.svg",           { png::NAV_HOME,           true } },
    { "ALLTRANSACTIONS.svg",    { png::ALLTRANSACTIONS,    true } },
    { "NAV_FILTER.svg",         { png::NAV_FILTER,         true } },
    { "NAV_GRM.svg",            { png::NAV_GRM,            true } },
    { "NAV_HELP.svg",           { png::NAV_HELP,           true } },
    { "FAVOURITE.svg",          { png::FAVOURITE,          true } },
    { "SAVINGS_NORMAL.svg",     { png::SAVINGS_NORMAL,     true } },
    { "CC_NORMAL.svg",          { png::CC_NORMAL,          true } },
    { "CASH_NORMAL.svg",        { png::CASH_NORMAL,        true } },
    { "LOAN_ACC_NORMAL.svg",    { png::LOAN_ACC_NORMAL,    true } },
    { "TERM_NORMAL.svg",        { png::TERM_NORMAL,        true } },
    { "STOCKS_NORMAL.svg",      { png::STOCKS_NORMAL,      true } },
    { "ASSET_NORMAL.svg",       { png::ASSET_NORMAL,       true } },
    { "ACCOUNT_CLOSED.svg",     { png::ACCOUNT_CLOSED,     true } },
    { "RECURRING.svg",          { png::RECURRING,          true } },
    { "TRASH.svg",              { png::TRASH,              true } },
    { "BUDGET.svg",             { png::BUDGET,             true } },
    { "PIE_CHART.svg",          { png::PIE_CHART,          true } },

    // Status
    { "UNRECONCILED.svg",       { png::UNRECONCILED,       false } },
    { "RECONCILED.svg",         { png::RECONCILED,         false } },
    { "DUPLICATE_STAT.svg",     { png::DUPLICATE_STAT,     false } },
    { "FOLLOW_UP.svg",          { png::FOLLOW_UP,          false } },
    { "VOID_STAT.svg",          { png::VOID_STAT,          false } },
    { "PROFIT.svg",             { png::PROFIT,             false } },
    { "LOSS.svg",               { png::LOSS,               false } },
    { "LED_OFF.svg",            { png::LED_OFF,            false } },
    { "LED_GREEN.svg",          { png::LED_GREEN,          false } },
    { "LED_YELLOW.svg",         { png::LED_YELLOW,         false } },
    { "LED_RED.svg",            { png::LED_RED,            false } },
    { "RUN.svg",                { png::RUN,                false } },
    { "RUN_AUTO.svg",           { png::RUN_AUTO,           false } },

    // Assets
    { "PROPERTY.svg",           { png::PROPERTY,           false } },
    { "CAR.svg",                { png::CAR,                false } },
    { "HOUSEHOLD_OBJ.svg",      { png::HOUSEHOLD_OBJ,      false } },
    { "ART.svg",                { png::ART,                false } },
    { "JEWELLERY.svg",          { png::JEWELLERY,          false } },
    { "CASH.svg",               { png::CASH,               false } },
    { "OTHER.svg",              { png::OTHER,              false } },

    // Controls
    { "CALCULATOR.svg",         { png::CALCULATOR,         false } },
    { "CLEAR.svg",              { png::CLEAR,              false } },
    { "CLIP.svg",               { png::CLIP,               false } },
    { "DOWNARROW.svg",          { png::DOWNARROW,          false } },
    { "UPARROW.svg",            { png::UPARROW,            false } },
    { "LEFTARROW.svg",          { png::LEFTARROW,          false } },
    { "RIGHTARROW.svg",         { png::RIGHTARROW,         false } },
    { "IMPORT.svg",             { png::IMPORT,             false } },
    { "MORE_OPTIONS.svg",       { png::MORE_OPTIONS,       false } },
    { "RELOCATION.svg",         { png::RELOCATION,         false } },
    { "SAVE.svg",               { png::SAVE,               false } },
    { "TRANSFILTER.svg",        { png::TRANSFILTER,        false } },
    { "TRANSFILTER_ACTIVE.svg", { png::TRANSFILTER_ACTIVE, false } },
    { "TRXNUM.svg",             { png::TRXNUM,             false } },
    { "UPDATE.svg",             { png::UPDATE,             false } },
    { "WEB.svg",                { png::WEB,                false } },

    // Settings
    { "GENERAL.svg",            { png::GENERAL,            false } },
    { "VIEW.svg",               { png::VIEW,               false } },
    { "ATTACHMENTS.svg",        { png::ATTACHMENTS,        false } },
    { "NETWORK.svg",            { png::NETWORK,            false } },
    { "OTHERS.svg",             { png::OTHERS,             false } },

    // Extra Account Icons
    { "ACC_MONEY.svg",          { png::ACC_MONEY,          true } },
    { "ACC_EURO.svg",           { png::ACC_EURO,           true } },
    { "ACC_FLAG.svg",           { png::ACC_FLAG,           true } },
    { "ACC_ABOUT.svg",          { png::ACC_ABOUT,          true } },
    { "ACC_COINS.svg",          { png::ACC_COINS,          true } },
    { "ACC_CLOCK.svg",          { png::ACC_CLOCK,          true } },
    { "ACC_CAT.svg",            { png::ACC_CAT,            true } },
    { "ACC_DOG.svg",            { png::ACC_DOG,            true } },
    { "ACC_TREES.svg",          { png::ACC_TREES,          true } },
    { "ACC_HOURGLASS.svg",      { png::ACC_HOURGLASS,      true } },
    { "ACC_WORK.svg",           { png::ACC_WORK,           true } },
    { "ACC_PAYPAL.svg",         { png::ACC_PAYPAL,         true } },
    { "ACC_WALLET.svg",         { png::ACC_WALLET,         true } },
    { "ACC_RUBIK.svg",          { png::ACC_RUBIK,          true } },

    // Other
    { "EMPTY.svg",              { png::EMPTY,              false } },
};

// -- static state

bool mmImage::darkFound;
bool mmImage::darkMode;
rapidjson::Document mmImage::metaData_doc;

// Using SVG and wxBitmapBundle for better HiDPI support.
wxSharedPtr<wxBitmapBundle> mmImage::programIconBundles[mmImage::numSizes][mmImage::MAX_PNG];
wxSharedPtr<wxArrayString> mmImage::filesInVFS;

// -- static methods

// Metadata item id, where it can be found, default and if mandatory
const std::map<int, std::tuple<wxString, wxString, bool>> mmImage::metaDataTrans()
{
    std::map<int, std::tuple<wxString, wxString, bool>> md;
    md[THEME_NAME]             = std::make_tuple("/theme/name",                 "",        true);
    md[THEME_AUTHOR]           = std::make_tuple("/theme/author",               "",        false);
    md[THEME_DESCRIPTION]      = std::make_tuple("/theme/description",          "",        true);
    md[THEME_URL]              = std::make_tuple("/colors/url",                 "",        false);
    md[COLOR_NAVPANEL_FONT]    = std::make_tuple("/colors/navigationPanelFont", "",        false);
    md[COLOR_NAVPANEL]         = std::make_tuple("/colors/navigationPanel",     "",        false);
    md[COLOR_TOOLBAR]          = std::make_tuple("/colors/toolbar",             wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE).GetAsString(wxC2S_HTML_SYNTAX), false);
    md[COLOR_BUTTON]           = std::make_tuple("/colors/button",              "",        false);
    md[COLOR_LISTPANEL]        = std::make_tuple("/colors/listPanel",           wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE).GetAsString(wxC2S_HTML_SYNTAX), false);
    md[COLOR_LIST]             = std::make_tuple("/colors/list",                "#FFFFFF", false);
    md[COLOR_LISTALT0]         = std::make_tuple("/colors/listAlternative1",    "#F0F5EB", false);
    md[COLOR_LISTALT0A]        = std::make_tuple("/colors/listAlternative2",    "#E0E7F0", false);
    md[COLOR_LISTTOTAL]        = std::make_tuple("/colors/listTotal",           "#7486A8", false);
    md[COLOR_LISTBORDER]       = std::make_tuple("/colors/listBorder",          "#000000", false);
    md[COLOR_LISTFUTURE]       = std::make_tuple("/colors/listFutureDate",      "#7486A8", false);
    md[COLOR_HTMLPANEL_BACK]   = std::make_tuple("/colors/htmlPanel/background", "",       false);
    md[COLOR_HTMLPANEL_FORE]   = std::make_tuple("/colors/htmlPanel/foreColor",  "",       false);
    md[COLOR_REPORT_ALTROW]    = std::make_tuple("/colors/reports/altRow",      "#F5F5F5", false);
    md[COLOR_REPORT_CREDIT]    = std::make_tuple("/colors/reports/credit",      "#50B381", false);
    md[COLOR_REPORT_DEBIT]     = std::make_tuple("/colors/reports/debit",       "#f12121", false);
    md[COLOR_REPORT_DELTA]     = std::make_tuple("/colors/reports/delta",       "#008FFB", false);
    md[COLOR_REPORT_PERF]      = std::make_tuple("/colors/reports/perf",        "#FF6307", false);
    md[COLOR_REPORT_FORECOLOR] = std::make_tuple("/colors/reports/foreColor",   "#373D3F", false);
    md[COLOR_REPORT_PALETTE]   = std::make_tuple("/colors/reports/palette",  "#008FFB "
            "#00E396 #FEB019 #FF4560 #775DD0 #3F51B5 #03A9F4 #4cAF50 #F9CE1D #FF9800 "
            "#33B2DF #546E7A #D4526E #13D8AA #A5978B #4ECDC4 #81D4FA #546E7A #FD6A6A "
            "#2B908F #F9A3A4 #90EE7E #FA4443 #69D2E7 #449DD1 #F86624",                     false);
    md[COLOR_HIDDEN]           = std::make_tuple("/colors/other/hidden",         "#81D4FA", false);
    md[COLOR_GRM_LINENUMBER]   = std::make_tuple("/colors/grm/linenumber",       "#6b6c6dff", false);
    md[COLOR_GRM_LINENUMBER_BG] = std::make_tuple("/colors/grm/linenumberbackground", "#ddddddff", false);
    md[COLOR_GRM_MARKER]       = std::make_tuple("/colors/grm/marker",           "#d5dceb96", false);
    md[COLOR_GRM_KEYWORD]      = std::make_tuple("/colors/grm/keyword",          "#2532e9ff", false);
    md[COLOR_GRM_KEYATTRIBUTE] = std::make_tuple("/colors/grm/keyattribute",     "#18da59ff", false);
    md[COLOR_GRM_STRING]       = std::make_tuple("/colors/grm/string",           "#9c41f1ff", false);
    md[COLOR_GRM_COMMENT]      = std::make_tuple("/colors/grm/comment",          "#909a9eff", false);
    md[COLOR_GRM_IDENTIFIER]   = std::make_tuple("/colors/grm/identifier",       "#d38e40d0", false);
    md[COLOR_GRM_UNKNOWN]      = std::make_tuple("/colors/grm/unknown",          "#f6fa1ed8", false);
    md[COLOR_GRM_OPERATOR]     = std::make_tuple("/colors/grm/operator",         "#3ba5d6ff", false);
    md[COLOR_GRM_NUMBER]       = std::make_tuple("/colors/grm/number",           "#ff0d0dff", false);
    md[COLOR_GRM_SPECIAL]      = std::make_tuple("/colors/grm/special",          "#e70870ff", false);
    md[COLOR_TEXTCONTROL]      = std::make_tuple("/colors/textControl",         wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX).GetAsString(wxC2S_HTML_SYNTAX), false);
    md[COLOR_TEXTCONTROL_FONT] = std::make_tuple("/colors/textControlFont",     wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXTEXT).GetAsString(wxC2S_HTML_SYNTAX), false);

    return md;
};

const std::map<int, wxBitmapBundle> mmImage::navtree_images(const int size)
{
    // TODO: GRM rep group ico,
    return{
        { img::HOUSE_PNG,              bitmapBundle(png::NAV_HOME,        size) },
        { img::ALLTRANSACTIONS_PNG,    bitmapBundle(png::ALLTRANSACTIONS, size) },
        { img::SCHEDULE_PNG,           bitmapBundle(png::RECURRING,       size) },
        { img::TRASH_PNG,              bitmapBundle(png::TRASH,           size) },
        { img::CALENDAR_PNG,           bitmapBundle(png::BUDGET,          size) },
        { img::PIECHART_PNG,           bitmapBundle(png::PIE_CHART,       size) },
        { img::HELP_PNG,               bitmapBundle(png::NAV_HELP,        size) },
        { img::FAVOURITE_PNG,          bitmapBundle(png::FAVOURITE,       size) },
        { img::FILTER_PNG,             bitmapBundle(png::NAV_FILTER,      size) },
        { img::ASSET_NORMAL_PNG,       bitmapBundle(png::ASSET_NORMAL,    size) },
        { img::CUSTOMSQL_PNG,          bitmapBundle(png::NAV_GRM,         size) },
        { img::CUSTOMSQL_GRP_PNG,      bitmapBundle(png::NAV_GRM,         size) },
        { img::SAVINGS_ACC_NORMAL_PNG, bitmapBundle(png::SAVINGS_NORMAL,  size) },
        { img::CARD_ACC_NORMAL_PNG,    bitmapBundle(png::CC_NORMAL,       size) },
        { img::TERMACCOUNT_NORMAL_PNG, bitmapBundle(png::TERM_NORMAL,     size) },
        { img::STOCK_ACC_NORMAL_PNG,   bitmapBundle(png::STOCKS_NORMAL,   size) },
        { img::CASH_ACC_NORMAL_PNG,    bitmapBundle(png::CASH_NORMAL,     size) },
        { img::LOAN_ACC_NORMAL_PNG,    bitmapBundle(png::LOAN_ACC_NORMAL, size) },
        { img::ACCOUNT_CLOSED_PNG,     bitmapBundle(png::ACCOUNT_CLOSED,  size) },
    };
};

// Custom icons for accounts
const std::map<int, wxBitmapBundle> mmImage::acc_images(int size)
{
    return {
        { acc_img::ACC_ICON_MONEY,     bitmapBundle(png::ACC_MONEY,     size) },
        { acc_img::ACC_ICON_EURO,      bitmapBundle(png::ACC_EURO,      size) },
        { acc_img::ACC_ICON_FLAG,      bitmapBundle(png::ACC_FLAG,      size) },
        { acc_img::ACC_ICON_COINS,     bitmapBundle(png::ACC_COINS,     size) },
        { acc_img::ACC_ICON_ABOUT,     bitmapBundle(png::ACC_ABOUT,     size) },
        { acc_img::ACC_ICON_CLOCK,     bitmapBundle(png::ACC_CLOCK,     size) },
        { acc_img::ACC_ICON_CAT,       bitmapBundle(png::ACC_CAT,       size) },
        { acc_img::ACC_ICON_DOG,       bitmapBundle(png::ACC_DOG,       size) },
        { acc_img::ACC_ICON_TREES,     bitmapBundle(png::ACC_TREES,     size) },
        { acc_img::ACC_ICON_HOURGLASS, bitmapBundle(png::ACC_HOURGLASS, size) },
        { acc_img::ACC_ICON_WORK,      bitmapBundle(png::ACC_WORK,      size) },
        { acc_img::ACC_ICON_PAYPAL,    bitmapBundle(png::ACC_PAYPAL,    size) },
        { acc_img::ACC_ICON_WALLET,    bitmapBundle(png::ACC_WALLET,    size) },
        { acc_img::ACC_ICON_RUBIK,     bitmapBundle(png::ACC_RUBIK,     size) },
    };
}

int mmImage::getIconSizeIdx(const int iconSize)
{
    const int x = (iconSize > 0)
        ? iconSize
        : PrefModel::instance().getIconSize();
    auto it = find_if(mmImage::sizes.begin(), mmImage::sizes.end(),
        [x](const std::pair<int, int>& p) {
            return p.second == x;
        }
    );
    if (it == mmImage::sizes.end())
        return -1;

    return it->first;
}

bool mmImage::processThemes(wxString themeDir, wxString myTheme, bool metaPhase)
{
    wxDir directory(themeDir);
    wxLogDebug("{{{ mmImage::processThemes(metaPhase=%s)", metaPhase ? "YES" : "NO");
    wxLogDebug("Scanning [%s] for Theme [%s]", themeDir, myTheme);
    if (!directory.IsOpened()) {
        // matching {{{ for next line
        wxLogDebug("}}}");
        return false;
    }

    bool themeMatched = false;
    wxString filename;

    bool cont = directory.GetFirst(&filename, "*.mmextheme", wxDIR_FILES);
    while (cont) {
        wxFileName themeFile(themeDir, filename);
        const wxString thisTheme = themeFile.GetName();
        wxLogDebug("Found theme [%s]", thisTheme);

        wxFileInputStream themeZip(themeFile.GetFullPath());
        wxASSERT(themeZip.IsOk());   // Make sure we can open find the Zip

        if (!thisTheme.Cmp(myTheme)) {
            themeMatched = true;
            wxZipInputStream themeStream(themeZip);
            std::unique_ptr<wxZipEntry> themeEntry;

            const wxString bgString = mmImage::themeMetaString(mmImage::COLOR_NAVPANEL).AfterFirst('#');
            long bgStringConv;
            if (!bgString.ToLong(&bgStringConv, 16))
                bgStringConv = -1;
            else
                bgStringConv = bgStringConv * 256 + 255;  // Need to add Alpha

            while (themeEntry.reset(themeStream.GetNextEntry()), themeEntry) { // != nullptr
                wxASSERT(themeZip.CanRead()); // Make sure we can read the Zip Entry

                const wxFileName fileEntryName = wxFileName(themeEntry->GetName());
                const wxString fileFullPath = fileEntryName.GetFullPath();
                const wxString fileEntry = fileEntryName.GetFullName();
                std::string fileName = std::string(fileEntry.mb_str());
                const wxString fileNameString(fileName);

                if (fileEntryName.IsDir())
                    continue;   // We can skip directories

                if (metaPhase) {
                    // For this phase we are only interested in the metadata and checking
                    // if theme has dark-mode components
                    if (fileName == "_theme.json") {
                        wxMemoryOutputStream memOut(nullptr);
                        themeStream.Read(memOut);
                        const wxStreamBuffer* buffer = memOut.GetOutputStreamBuffer();
                        wxString metaData(static_cast<char *>(buffer->GetBufferStart()), buffer->GetBufferSize());
                        if (mmImage::metaData_doc.Parse(metaData.utf8_str()).HasParseError()) {
                            wxMessageBox(
                                wxString::Format(_tu("Metadata JSON in Theme “%s” is unable to be parsed and looks badly constructed, please correct."), thisTheme),
                                _t("Warning"),
                                wxOK | wxICON_WARNING
                            );
                        }
                    }
                    else {
                        if (!mmImage::darkFound && fileNameString.StartsWith("dark-"))
                            mmImage::darkFound = true;
                    }
                    continue;
                }

                // Only process dark mode files when in theme and needed
                if (mmImage::darkFound) {
                    if (mmImage::darkMode && !fileNameString.StartsWith("dark-"))
                        continue;
                    else if (!mmImage::darkMode && fileNameString.StartsWith("dark-"))
                        continue;
                }

                // Remove dark mode prefix
                if (mmImage::darkFound && mmImage::darkMode)
                    fileName = fileName.substr(5);

                // If the file does not match an icon file then just load into VFS / tmp
                if (!mmImage::iconName2enum.count(fileName)) {
#if defined(__WXMSW__) || defined(__WXMAC__)
                    wxMemoryOutputStream memOut(nullptr);
                    themeStream.Read(memOut);
                    const wxStreamBuffer* buffer = memOut.GetOutputStreamBuffer();

                    if (wxNOT_FOUND != mmImage::filesInVFS->Index(fileName)) // If already loaded then remove and replace
                        wxMemoryFSHandler::RemoveFile(fileName);
                    wxMemoryFSHandler::AddFile(
                        fileName, buffer->GetBufferStart(), buffer->GetBufferSize()
                    );
                    wxLogDebug("Theme: '%s' File: '%s' has been copied to VFS", thisTheme, fileName);
#else
                    const wxString theme_file = mmPath::getTempFolder() + fileName;
                    wxFileOutputStream fileOut(theme_file);
                    if (!fileOut.IsOk())
                        wxLogError("Could not copy %s !", fileFullPath);
                    else
                        wxLogDebug("Copying file:\n %s \nto\n %s", fileFullPath, theme_file);
                    themeStream.Read(fileOut);

#endif
                    mmImage::filesInVFS->Add(fileName);
                    continue;
                }

                // So we have an icon file now, now need to convert from SVG to PNG at various resolutions and store
                // it away for use

                wxMemoryOutputStream memOut(nullptr);
                themeStream.Read(memOut);
                const wxStreamBuffer* buffer = memOut.GetOutputStreamBuffer();

                int svgEnum = mmImage::iconName2enum.find(fileName)->second.first;
                for(const auto& sizePair : mmImage::sizes) {
                    const unsigned int icon_size = sizePair.second;
                    mmImage::programIconBundles[sizePair.first][svgEnum] = new wxBitmapBundle(
                        wxBitmapBundle::FromSVG(
                            static_cast<wxByte*>(buffer->GetBufferStart()),
                            buffer->GetBufferSize(),
                            wxSize(icon_size, icon_size)
                        )
                    );
                }
            }
        }
        cont = directory.GetNext(&filename);
    }
    wxLogDebug("}}}");
    return themeMatched;
}

// Check that the loaded theme contains all the minimal files needed
bool mmImage::checkThemeContents(wxArrayString* filesinTheme)
{
    bool success = true;

    // Check for required files
    const wxString neededFiles[] = { "master.css", "" };

    for (int i = 0; !neededFiles[i].IsEmpty(); i++) {
        const wxString realName = (mmImage::darkFound && mmImage::darkMode)
            ? neededFiles[i].AfterLast('-')
            : neededFiles[i];
        if (wxNOT_FOUND == filesinTheme->Index(realName)) {
            wxMessageBox(
                wxString::Format(_t("File '%1$s' missing or invalid in chosen theme '%2$s'"),
                    neededFiles[i],
                    SettingModel::instance().getTheme()
                ),
                _t("Warning"),
                wxOK | wxICON_WARNING
            );
            success = false;
        }
    }

    // Check for required metadata
    for (const auto& it : mmImage::metaDataTrans()) {
        if (std::get<2>(it.second) && mmImage::themeMetaString(it.first).IsEmpty()) {
            wxMessageBox(
                wxString::Format(_t("Metadata '%1$s' missing in chosen theme '%2$s'"),
                    std::get<0>(it.second),
                    SettingModel::instance().getTheme()
                ),
                _t("Warning"),
                wxOK | wxICON_WARNING
            );
            success = false;
        }
    }

    // Check for required icons
    wxString missingIcons;
    const int maxCutOff = 10;
    int erroredIcons = 0;
    for (int i = 0; i < MAX_PNG; i++) {
        if (!mmImage::programIconBundles[0][i]) {
            for (auto it = mmImage::iconName2enum.begin(); it != mmImage::iconName2enum.end(); it++) {
                if (it->second.first == i) {
                    if (erroredIcons <= maxCutOff) {
                        missingIcons << it->first << ", ";
                    }
                    erroredIcons++;
                    success = false;
                    continue;
                }
            }
        }
    }

    if (!missingIcons.IsEmpty()) {
        missingIcons.RemoveLast(2);
        if (erroredIcons > maxCutOff) {
            missingIcons << " " << _tu("and more…");
        }
        wxMessageBox(
            wxString::Format(_t("There are %1$d missing or invalid icons in chosen theme '%2$s': %3$s"),
                erroredIcons,
                SettingModel::instance().getTheme(),
                missingIcons
            ),
            _t("Warning"),
            wxOK | wxICON_WARNING
        );
    }
    return success;
}

void mmImage::reverttoDefaultTheme()
{
    SettingModel::instance().saveTheme("default");
    mmImage::darkFound = false;
    mmImage::processThemes(
        mmPath::getPathResource(mmPath::THEMESDIR),
        SettingModel::instance().getTheme(),
        true
    );
    mmImage::processThemes(
        mmPath::getPathResource(mmPath::THEMESDIR),
        SettingModel::instance().getTheme(),
        false
    );
}

void mmImage::loadTheme()
{
    mmImage::darkMode = (
        (mmPlatform::isDarkMode() &&
            PrefModel::THEME_MODE::AUTO == PrefModel::instance().getThemeMode()
        ) ||
        PrefModel::THEME_MODE::DARK == PrefModel::instance().getThemeMode()
    );
    mmImage::filesInVFS = new wxArrayString();

    // Scan first for metadata then for the icons and other files
    mmImage::darkFound = false;
    if (mmImage::processThemes(
        mmPath::getPathResource(mmPath::THEMESDIR),
        SettingModel::instance().getTheme(),
        true
    ))
        mmImage::processThemes(
            mmPath::getPathResource(mmPath::THEMESDIR),
            SettingModel::instance().getTheme(),
            false
        );
    else
        if (
            mmImage::processThemes(
                mmPath::getPathUser(mmPath::USERTHEMEDIR),
                SettingModel::instance().getTheme(),
                true
            ))
            mmImage::processThemes(
                mmPath::getPathUser(mmPath::USERTHEMEDIR),
                SettingModel::instance().getTheme(),
                false
            );
        else {
            wxMessageBox(
                wxString::Format(_t("Theme %s not found; it may no longer be supported. Reverting to the default theme."),
                    SettingModel::instance().getTheme()
                ),
                _t("Warning"),
                wxOK | wxICON_WARNING
            );
            mmImage::reverttoDefaultTheme();
        }

    if (!mmImage::checkThemeContents(mmImage::filesInVFS.get())) {
        wxMessageBox(
            wxString::Format(_t("Theme %s has missing items and is incompatible. Reverting to default theme"),
                SettingModel::instance().getTheme()
            ),
            _t("Warning"),
            wxOK | wxICON_WARNING
        );
        mmImage::reverttoDefaultTheme();
        if (!mmImage::checkThemeContents(mmImage::filesInVFS.get())) {
            // Time to give up as we couldn't find a workable theme
            wxMessageBox(
                _t("No workable theme found, the installation may be corrupt"),
                _t("Error"),
                wxOK | wxICON_ERROR
            );
            exit(EXIT_FAILURE);
        }
    }
}

void mmImage::closeTheme()
{
    // Release icons - needed before app closure
    // https://github.com/wxWidgets/wxWidgets/issues/22862
    for (int i = 0; i < mmImage::numSizes; i++)
        for (int j = 0; j < MAX_PNG; j++)
            mmImage::programIconBundles[i][j].reset();
}

const wxString mmImage::themeMetaString(int ref)
{
    auto i = mmImage::metaDataTrans().find(ref)->second;
    wxString metaLocation = std::get<0>(i);
    if (mmImage::darkFound && mmImage::darkMode && !metaLocation.StartsWith("/theme"))
        metaLocation.Prepend("/dark");
    const Pointer ptr(metaLocation.mb_str());
    wxString metaValue = wxString::FromUTF8(
        GetValueByPointerWithDefault(mmImage::metaData_doc, ptr, "").GetString()
    );
    if (metaValue.IsEmpty() && !std::get<2>(i))
        metaValue = std::get<1>(i);
    return (metaValue);
}

// helpers
long mmImage::themeMetaLong(int ref)
{
    long retValue;
    if (!mmImage::themeMetaString(ref).ToLong(&retValue, 0))
        retValue = 0;
    return retValue;
}

const wxColour mmImage::themeMetaColour(int ref)
{
    auto c = mmImage::themeMetaString(ref);
    return wxColour(c);
}

void mmImage::themeMetaColour(wxWindow* object, wxColour c, bool foreground)
{
    if (foreground)
        object->SetForegroundColour(c);
    else {
        object->SetBackgroundColour(c);
        object->SetForegroundColour(*mmUserColor::bestFontColor(c));
    }

    enableMSWDarkMode(object, mmImage::darkMode);
}

void mmImage::themeMetaColour(wxWindow* object, int ref, bool foreground)
{
    const wxString c = mmImage::themeMetaString(ref);

    if (!c.empty())
        mmImage::themeMetaColour(object, wxColour(c), foreground);
}

const std::vector<wxColour> mmImage::themeMetaColour_a(int ref)
{
    std::vector<wxColour> colours;
    wxStringTokenizer input(mmImage::themeMetaString(ref));
    while (input.HasMoreTokens())
        colours.push_back(wxColour(input.GetNextToken()));
    return colours;
}

const wxBitmapBundle mmImage::bitmapBundle(const int ref, const int defSize)
{
    const int idx = mmImage::getIconSizeIdx(defSize);
    return *mmImage::programIconBundles[idx][ref];
}

//----------------------------------------------------------------------------
// (Nav Tree) Icon  handling
NavTreeIconImages::NavTreeIconImages()
{
    m_changed = false;
    m_size = 0;
}

NavTreeIconImages& NavTreeIconImages::instance()
{
    return Singleton<NavTreeIconImages>::instance();
}

wxVector<wxBitmapBundle> NavTreeIconImages::getList(const int size)
{
    int x = (size > 0) ? size : PrefModel::instance().getIconSize();
    wxVector<wxBitmapBundle> bitmaps;
    for (const auto& img : mmImage::navtree_images(x))
        bitmaps.push_back(img.second);
    for (const auto& img : mmImage::acc_images(x))
         bitmaps.push_back(img.second);

    // Reset maps:
    m_indexMap = {};
    m_indexReverseMap = {};
    wxSize bSize = wxSize(x,x);
    // Get downloaded icon images:
    wxFileName resPath = mmPath::getPathUserRaw(mmPath::USERICONS);
    if (resPath.IsOk()) {
        wxDir dir(resPath.GetPath());
        if (dir.IsOpened()) {
            wxString filename;
            int bidx = mmImage::acc_img::MAX_ACC_ICON;
            bool cont = dir.GetFirst(&filename);
            while (cont) {
                wxFileName fullPath(resPath.GetPath(), filename);
                if (fullPath.FileExists()) {
                    if (fullPath.GetExt().Lower() == "svg") {
                        wxBitmapBundle bundle = wxBitmapBundle::FromSVGFile(fullPath.GetFullPath(), bSize);

                        #if wxUSE_STD_CONTAINERS
                        bitmaps.emplace_back(bundle);
                        #else
                        bitmaps.push_back(bundle);
                        #endif
                        m_indexMap[bidx] = filename;
                        m_indexReverseMap[filename] = bidx++;
                    }
                    else {
                        wxImage image;
                        if (image.LoadFile(fullPath.GetFullPath())) {
                            wxImage img = image.Scale(x, x, wxIMAGE_QUALITY_HIGH);
                            #if wxUSE_STD_CONTAINERS
                            bitmaps.emplace_back(wxBitmap(img));
                            #else
                            auto bmp = wxBitmap(img);
                            bitmaps.push_back(bmp);
                            #endif
                            m_indexMap[bidx] = filename;
                            m_indexReverseMap[filename] = bidx++;
                        }
                    }
                }
                cont = dir.GetNext(&filename);
            }
        }
        else {
            wxLogDebug("Icon directory could not be opened: %s", resPath.GetPath());
        }
    }

    m_size = static_cast<int>(bitmaps.size());
    return (bitmaps);
}

// Use with care!!, only creates the maps for init
void NavTreeIconImages::initIndexMap()
{
    m_indexMap = {};
    m_indexReverseMap = {};
    // Map downloaded icon images:
    wxFileName resPath = mmPath::getPathUserRaw(mmPath::USERICONS);
    int bidx = mmImage::acc_img::MAX_ACC_ICON;
    if (resPath.IsOk()) {
        wxDir dir(resPath.GetPath());
        if (dir.IsOpened()) {
            wxString filename;
            bool cont = dir.GetFirst(&filename);
            while (cont) {
                wxFileName fullPath(resPath.GetPath(), filename);
                if (fullPath.FileExists()) {
                    if (fullPath.GetExt().Lower() == "svg") {
                        m_indexMap[bidx] = filename;
                        m_indexReverseMap[filename] = bidx++;
                    }
                    else {
                        wxImage image;
                        if (image.LoadFile(fullPath.GetFullPath())) {
                            m_indexMap[bidx] = filename;
                            m_indexReverseMap[filename] = bidx++;
                        }
                    }
                }
                cont = dir.GetNext(&filename);
            }
        }
        else {
            wxLogDebug("Icon directory could not be opened: %s", resPath.GetPath());
        }
    }
    m_size = bidx;
}

wxImageList* NavTreeIconImages::getImageList(const int rsize)
{
    int x = (rsize > 0) ? rsize : PrefModel::instance().getNavigationIconSize();
    wxImageList* imageList = new wxImageList(x, x);
    wxSize size = (wxSize(x, x));

    for (const auto& img : mmImage::navtree_images(x)) {
        imageList->Add(img.second.GetBitmap(size));
    }
    for (const auto& img : mmImage::acc_images(x)) {
        imageList->Add(img.second.GetBitmap(size));
    }

    // Get downloaded icon images:
    wxFileName resPath = mmPath::getPathUserRaw(mmPath::USERICONS);
    if (resPath.IsOk()) {
        wxDir dir(resPath.GetPath());
        if (dir.IsOpened()) {
            wxString filename;
            bool cont = dir.GetFirst(&filename);
            while (cont) {
                wxFileName fullPath(resPath.GetPath(), filename);
                if (fullPath.FileExists()) {
                    if (fullPath.GetExt().Lower() == "svg") {
                        wxBitmapBundle bundle = wxBitmapBundle::FromSVGFile(fullPath.GetFullPath(), size);
                        wxBitmap bmp = bundle.GetBitmap(size);
                        imageList->Add(bmp.ConvertToImage());
                    }
                    else {
                        wxImage image;
                        if (image.LoadFile(fullPath.GetFullPath())) {
                            wxImage img = image.Scale(x, x, wxIMAGE_QUALITY_HIGH);
                            imageList->Add(img);
                        }
                    }
                }
                cont = dir.GetNext(&filename);
            }
        }
        else {
            wxLogDebug("Icon directory could not be opened: %s", resPath.GetPath());
        }
    }
    return imageList;
}

int NavTreeIconImages::getImgIndex(wxString imgName)
{
    return m_indexReverseMap.count(imgName) > 0 ? m_indexReverseMap[imgName] : -1;
}

int NavTreeIconImages::getImgIndexFromStorageString(wxString strString)
{
    int imageIndex;
    wxString fileid;
    if (strString.StartsWith("CI:", &fileid)) {
        imageIndex = NavTreeIconImages::instance().getImgIndex(fileid);
    }
    else {
        imageIndex = wxAtoi(strString);
    }
    return imageIndex;
}

void NavTreeIconImages::setChanged()
{
    m_changed = true;
}

bool NavTreeIconImages::isListChanged()
{
    bool result = m_changed;
    m_changed = false;
    return result;
}

//----------------------------------------------------------------------------
