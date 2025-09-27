/*******************************************************
Copyright (C) 2014, 2015, 2021 Nikolay Akimov
Copyright (C) 2021 Mark Whalley (mark@ipx.co.uk)
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
*******************************************************/

#include "images_list.h"
#include "model/Model_Setting.h"
#include "option.h"
#include "platfdep.h"
#include "util.h"
#include <wx/image.h>
#include <wx/bitmap.h>
#include <map>
#include <wx/sharedptr.h>
#include "paths.h"
#include <wx/dir.h>
#include <wx/zipstrm.h>
#include <wx/rawbmp.h>
#include <wx/fs_mem.h>
#include <wx/mstream.h>
#include <wx/tokenzr.h>
#include <memory>

#include <array>

// SVG filename in Zip, the PNG enum to which it relates, whether to recolor background
static const std::map<std::string, std::pair<int, bool>> iconName2enum = {
    { "NEW_DB.svg", { NEW_DB, false } },
    { "OPEN.svg", { OPEN, false } },
    { "NEW_ACC.svg", { NEW_ACC, false } },
    { "HOME.svg", { HOME, false } },
    { "CATEGORY.svg", { CATEGORY, false } },
    { "PAYEE.svg", { PAYEE, false } },
    { "CURR.svg", { CURR, false } },
    { "TAG.svg", { TAG, false } },
    { "FILTER.svg", { FILTER, false } },
    { "GRM.svg", { GRM, false } },
    { "OPTIONS.svg", { OPTIONS, false } },
    { "NEW_TRX.svg", { NEW_TRX, false } },
    { "NEW_NEWS.svg", { NEW_NEWS, false } },
    { "NEWS.svg", { NEWS, false } },
    { "CURRATES.svg", { CURRATES, false } },
    { "FULLSCREEN.svg", { FULLSCREEN, false } },
    { "PRINT.svg", { PRINT, false } },
    { "ABOUT.svg", { ABOUT, false } },
    { "HELP.svg", { HELP, false } },

    // Navigation
    { "NAV_HOME.svg", { NAV_HOME, true } },
    { "ALLTRANSACTIONS.svg", { ALLTRANSACTIONS, true } },
    { "NAV_FILTER.svg", { NAV_FILTER, true } },
    { "NAV_GRM.svg", { NAV_GRM, true } },
    { "NAV_HELP.svg", { NAV_HELP, true } },
    { "FAVOURITE.svg", { FAVOURITE, true } },
    { "SAVINGS_NORMAL.svg", { SAVINGS_NORMAL, true } },
    { "CC_NORMAL.svg", { CC_NORMAL, true } },
    { "CASH_NORMAL.svg", { CASH_NORMAL, true } },
    { "LOAN_ACC_NORMAL.svg", { LOAN_ACC_NORMAL, true } },
    { "TERM_NORMAL.svg", { TERM_NORMAL, true } },
    { "STOCKS_NORMAL.svg", { STOCKS_NORMAL, true } },
    { "ASSET_NORMAL.svg", { ASSET_NORMAL, true } },
    { "ACCOUNT_CLOSED.svg", { ACCOUNT_CLOSED, true } },
    { "RECURRING.svg", { RECURRING, true } },
    { "TRASH.svg", { TRASH, true } },
    { "BUDGET.svg", { BUDGET, true } },
    { "PIE_CHART.svg", { PIE_CHART, true } },

    // Status
    { "UNRECONCILED.svg", { UNRECONCILED, false } },
    { "RECONCILED.svg", { RECONCILED, false } },
    { "DUPLICATE_STAT.svg", { DUPLICATE_STAT, false } },
    { "FOLLOW_UP.svg", { FOLLOW_UP, false } },
    { "VOID_STAT.svg", { VOID_STAT, false } },
    { "PROFIT.svg", { PROFIT, false } },
    { "LOSS.svg", { LOSS, false } },
    { "LED_OFF.svg", { LED_OFF, false } },
    { "LED_GREEN.svg", { LED_GREEN, false } },
    { "LED_YELLOW.svg", { LED_YELLOW, false } },
    { "LED_RED.svg", { LED_RED, false } },
    { "RUN.svg", { RUN, false } },
    { "RUN_AUTO.svg", { RUN_AUTO, false } },

    // Assets
    { "PROPERTY.svg", { PROPERTY, false } },
    { "CAR.svg", { CAR, false } },
    { "HOUSEHOLD_OBJ.svg", { HOUSEHOLD_OBJ, false } },
    { "ART.svg", { ART, false } },
    { "JEWELLERY.svg", { JEWELLERY, false } },
    { "CASH.svg", { CASH, false } },
    { "OTHER.svg", { OTHER, false } },

    //Controls
    { "CALCULATOR.svg", { CALCULATOR, false } },
    { "CLEAR.svg", { CLEAR, false } },
    { "CLIP.svg", { CLIP, false } },
    { "DOWNARROW.svg", { DOWNARROW, false } },
    { "UPARROW.svg", { UPARROW, false } },
    { "LEFTARROW.svg", { LEFTARROW, false } },
    { "RIGHTARROW.svg", { RIGHTARROW, false } },
    { "IMPORT.svg", { IMPORT, false } },
    { "MORE_OPTIONS.svg", { MORE_OPTIONS, false } },
    { "RELOCATION.svg", { RELOCATION, false } },
    { "SAVE.svg", { SAVE, false } },
    { "TRANSFILTER.svg", { TRANSFILTER, false } },
    { "TRANSFILTER_ACTIVE.svg", { TRANSFILTER_ACTIVE, false } },
    { "TRXNUM.svg", { TRXNUM, false } },
    { "UPDATE.svg", { UPDATE, false } },
    { "WEB.svg", { WEB, false } },

    // Settings
    { "GENERAL.svg", { GENERAL, false } },
    { "VIEW.svg", { VIEW, false } },
    { "ATTACHMENTS.svg", { ATTACHMENTS, false } },
    { "NETWORK.svg", { NETWORK, false } },
    { "OTHERS.svg", { OTHERS, false } },

    // Extra Account Icons
    { "ACC_MONEY.svg", { ACC_MONEY, true } },
    { "ACC_EURO.svg", { ACC_EURO, true } },
    { "ACC_FLAG.svg", { ACC_FLAG, true } },
    { "ACC_ABOUT.svg", { ACC_ABOUT, true } },
    { "ACC_COINS.svg", { ACC_COINS, true } },
    { "ACC_CLOCK.svg", { ACC_CLOCK, true } },
    { "ACC_CAT.svg", { ACC_CAT, true } },
    { "ACC_DOG.svg", { ACC_DOG, true } },
    { "ACC_TREES.svg", { ACC_TREES, true } },
    { "ACC_HOURGLASS.svg", { ACC_HOURGLASS, true } },
    { "ACC_WORK.svg", { ACC_WORK, true } },
    { "ACC_PAYPAL.svg", { ACC_PAYPAL, true } },
    { "ACC_WALLET.svg", { ACC_WALLET, true } },
    { "ACC_RUBIK.svg", { ACC_RUBIK, true } },

    // Other
    { "EMPTY.svg", { EMPTY, false } }
};

// Metadata item id, where it can be found, default and if mandatory
const std::map<int, std::tuple<wxString, wxString, bool> > metaDataTrans()
{
    std::map<int, std::tuple<wxString, wxString, bool> > md;
    md[THEME_NAME]             = std::make_tuple("/theme/name",                 "",        true);
    md[THEME_AUTHOR]           = std::make_tuple("/theme/author",               "",        false);
    md[THEME_DESCRIPTION]      = std::make_tuple("/theme/description",          "",        true);
    md[THEME_URL]              = std::make_tuple("/colors/url",                 "",        false);
    md[COLOR_NAVPANEL_FONT]    = std::make_tuple("/colors/navigationPanelFont", "",        false);
    md[COLOR_NAVPANEL]         = std::make_tuple("/colors/navigationPanel",     "",        false);
    md[COLOR_LISTPANEL]        = std::make_tuple("/colors/listPanel",           "",        false);
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
    md[COLOR_REPORT_DEBIT]     = std::make_tuple("/colors/reports/debit",       "#F75E51", false);
    md[COLOR_REPORT_DELTA]     = std::make_tuple("/colors/reports/delta",       "#008FFB", false);
    md[COLOR_REPORT_PERF]      = std::make_tuple("/colors/reports/perf",       "#FF6307", false);
    md[COLOR_REPORT_FORECOLOR] = std::make_tuple("/colors/reports/foreColor",   "#373D3F", false);
    md[COLOR_REPORT_PALETTE]   = std::make_tuple("/colors/reports/palette",  "#008FFB "
            "#00E396 #FEB019 #FF4560 #775DD0 #3F51B5 #03A9F4 #4cAF50 #F9CE1D #FF9800 "
            "#33B2DF #546E7A #D4526E #13D8AA #A5978B #4ECDC4 #81D4FA #546E7A #FD6A6A "
            "#2B908F #F9A3A4 #90EE7E #FA4443 #69D2E7 #449DD1 #F86624",                     false);
    md[COLOR_HIDDEN]           = std::make_tuple("/colors/other/hidden",    "#81D4FA", false);

    return md;
};

constexpr int numSizes = 4;

const std::vector<std::pair<int, int> > sizes = { {0, 16}, {1, 24}, {2, 32}, {3, 48} };

const int mmBitmapButtonSize = 16;
bool darkFound, darkMode;

Document metaData_doc;

// Using SVG and wxBitmapBundle for better HiDPI support.
static wxSharedPtr<wxBitmapBundle> programIconBundles[numSizes][MAX_PNG];

static wxSharedPtr<wxArrayString> filesInVFS;

static const std::map<int, wxBitmapBundle> navtree_images(const int size)
{
    return{
        { HOUSE_PNG, mmBitmapBundle(png::NAV_HOME, size) }
        , { ALLTRANSACTIONS_PNG, mmBitmapBundle(png::ALLTRANSACTIONS, size) }
        , { SCHEDULE_PNG, mmBitmapBundle(png::RECURRING, size) }
        , { TRASH_PNG, mmBitmapBundle(png::TRASH, size) }
        , { CALENDAR_PNG, mmBitmapBundle(png::BUDGET, size) }
        , { PIECHART_PNG, mmBitmapBundle(png::PIE_CHART, size) }
        , { HELP_PNG, mmBitmapBundle(png::NAV_HELP, size) }
        , { FAVOURITE_PNG, mmBitmapBundle(png::FAVOURITE, size) }
        , { FILTER_PNG, mmBitmapBundle(png::NAV_FILTER, size) }
        , { ASSET_NORMAL_PNG, mmBitmapBundle(png::ASSET_NORMAL, size) }
        , { CUSTOMSQL_PNG, mmBitmapBundle(png::NAV_GRM, size) }
        , { CUSTOMSQL_GRP_PNG, mmBitmapBundle(png::NAV_GRM, size) } //TODO: GRM rep group ico
        , { SAVINGS_ACC_NORMAL_PNG, mmBitmapBundle(png::SAVINGS_NORMAL, size) }
        , { CARD_ACC_NORMAL_PNG, mmBitmapBundle(png::CC_NORMAL, size) }
        , { TERMACCOUNT_NORMAL_PNG, mmBitmapBundle(png::TERM_NORMAL, size) }
        , { STOCK_ACC_NORMAL_PNG, mmBitmapBundle(png::STOCKS_NORMAL, size) }
        , { CASH_ACC_NORMAL_PNG, mmBitmapBundle(png::CASH_NORMAL, size) }
        , { LOAN_ACC_NORMAL_PNG, mmBitmapBundle(png::LOAN_ACC_NORMAL, size) }
        , { ACCOUNT_CLOSED_PNG, mmBitmapBundle(png::ACCOUNT_CLOSED, size) }
    };
};

// Custom icons for accounts
static const std::map<int, wxBitmapBundle> acc_images(int size)
{
    return
    {
        { ACC_ICON_MONEY, mmBitmapBundle(png::ACC_MONEY, size) }
        , { ACC_ICON_EURO, mmBitmapBundle(png::ACC_EURO, size) }
        , { ACC_ICON_FLAG, mmBitmapBundle(png::ACC_FLAG, size) }
        , { ACC_ICON_COINS, mmBitmapBundle(png::ACC_COINS, size) }
        , { ACC_ICON_ABOUT, mmBitmapBundle(png::ACC_ABOUT, size) }
        , { ACC_ICON_CLOCK, mmBitmapBundle(png::ACC_CLOCK, size) }
        , { ACC_ICON_CAT, mmBitmapBundle(png::ACC_CAT, size) }
        , { ACC_ICON_DOG, mmBitmapBundle(png::ACC_DOG, size) }
        , { ACC_ICON_TREES, mmBitmapBundle(png::ACC_TREES, size) }
        , { ACC_ICON_HOURGLASS, mmBitmapBundle(png::ACC_HOURGLASS, size) }
        , { ACC_ICON_WORK, mmBitmapBundle(png::ACC_WORK, size) }
        , { ACC_ICON_PAYPAL, mmBitmapBundle(png::ACC_PAYPAL, size)}
        , { ACC_ICON_WALLET, mmBitmapBundle(png::ACC_WALLET, size) }
        , { ACC_ICON_RUBIK, mmBitmapBundle(png::ACC_RUBIK, size) }
    };
}


wxVector<wxBitmapBundle> navtree_images_list(const int size)
{
    int x = (size > 0) ? size : Option::instance().getIconSize();

    wxVector<wxBitmapBundle> images;
    for (const auto& img : navtree_images(x))
        images.push_back(img.second);
    for (const auto& img : acc_images(x))
         images.push_back(img.second);

    return (images);
}


static int getIconSizeIdx(const int iconSize)
{
    const int x = (iconSize > 0) ? iconSize : Option::instance().getIconSize();
    auto it = find_if(sizes.begin(), sizes.end(), [x](const std::pair<int, int>& p) { return p.second == x; });
    if(it == sizes.end())
        return -1;

    return it->first;
}

bool processThemes(wxString themeDir, wxString myTheme, bool metaPhase)
{
    wxDir directory(themeDir);
    wxLogDebug("{{{ processThemes(metaPhase=%s)", metaPhase ? "YES" : "NO");
    wxLogDebug("Scanning [%s] for Theme [%s]", themeDir, myTheme);
    if (!directory.IsOpened()) {
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

            const wxString bgString = mmThemeMetaString(meta::COLOR_NAVPANEL).AfterFirst('#');
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
                        if (metaData_doc.Parse(metaData.utf8_str()).HasParseError()) {
                            wxMessageBox(
                                wxString::Format(_t("Metadata JSON in Theme '%s' cannot be parsed and looks badly constructed, please correct."), thisTheme),
                                _t("Warning"),
                                wxOK | wxICON_WARNING
                            );
                        }
                    }
                    else {
                        if (!darkFound && fileNameString.StartsWith("dark-"))
                            darkFound = true;
                    }
                    continue;
                }

                // Only process dark mode files when in theme and needed
                if (darkFound) {
                    if (darkMode && !fileNameString.StartsWith("dark-"))
                        continue;
                    else if (!darkMode && fileNameString.StartsWith("dark-"))
                        continue;
                }

                // Remove dark mode prefix
                if (darkFound && darkMode)
                    fileName = fileName.substr(5);

                // If the file does not match an icon file then just load into VFS / tmp
                if (!iconName2enum.count(fileName)) {
#if defined(__WXMSW__) || defined(__WXMAC__)
                    wxMemoryOutputStream memOut(nullptr);
                    themeStream.Read(memOut);
                    const wxStreamBuffer* buffer = memOut.GetOutputStreamBuffer();

                    if (wxNOT_FOUND != filesInVFS->Index(fileName)) // If already loaded then remove and replace
                        wxMemoryFSHandler::RemoveFile(fileName);
                    wxMemoryFSHandler::AddFile(
                        fileName, buffer->GetBufferStart(), buffer->GetBufferSize()
                    );
                    wxLogDebug("Theme: '%s' File: '%s' has been copied to VFS", thisTheme, fileName);
#else
                    const wxString theme_file = mmex::getTempFolder() + fileName;
                    wxFileOutputStream fileOut(theme_file);
                    if (!fileOut.IsOk())
                        wxLogError("Could not copy %s !", fileFullPath);
                    else
                        wxLogDebug("Copying file:\n %s \nto\n %s", fileFullPath, theme_file);
                    themeStream.Read(fileOut);

#endif
                    filesInVFS->Add(fileName);
                    continue;
                }

                // So we have an icon file now, now need to convert from SVG to PNG at various resolutions and store
                // it away for use

                wxMemoryOutputStream memOut(nullptr);
                themeStream.Read(memOut);
                const wxStreamBuffer* buffer = memOut.GetOutputStreamBuffer();

                int svgEnum = iconName2enum.find(fileName)->second.first;
                for(const auto &sizePair : sizes) {
                    const unsigned int icon_size = sizePair.second;
                    programIconBundles[sizePair.first][svgEnum] = new wxBitmapBundle(
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
bool checkThemeContents(wxArrayString *filesinTheme)
{
    bool success = true;

    // Check for required files
    const wxString neededFiles[] = { "master.css", "" };

    for (int i = 0; !neededFiles[i].IsEmpty(); i++)
    {
        const wxString realName = (darkFound && darkMode) ? neededFiles[i].AfterLast('-') : neededFiles[i];
        if (wxNOT_FOUND == filesinTheme->Index(realName)) {
            wxMessageBox(wxString::Format(_t("File '%1$s' missing or invalid in chosen theme '%2$s'")
                , neededFiles[i], Model_Setting::instance().getTheme()), _t("Warning"), wxOK | wxICON_WARNING);
            success = false;
        }
    }

    // Check for required metadata
    for (const auto& it : metaDataTrans())
    {
        if (std::get<2>(it.second) && mmThemeMetaString(it.first).IsEmpty())
        {
            wxMessageBox(wxString::Format(_t("Metadata '%1$s' missing in chosen theme '%2$s'")
                , std::get<0>(it.second), Model_Setting::instance().getTheme()), _t("Warning"), wxOK | wxICON_WARNING);
            success = false;
        }
    }

    // Check for required icons
    wxString missingIcons;
    const int maxCutOff = 10;
    int erroredIcons = 0;
    for (int i = 0; i < MAX_PNG; i++)
    {
        if (!programIconBundles[0][i])
        {
            for (auto it = iconName2enum.begin(); it != iconName2enum.end(); it++)
            {
                if (it->second.first == i)
                {
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

    if (!missingIcons.IsEmpty())
    {
        missingIcons.RemoveLast(2);
        if (erroredIcons > maxCutOff) {
            missingIcons << " " << _tu("and more…");
        }
        wxMessageBox(wxString::Format(_t("There are %1$d missing or invalid icons in chosen theme '%2$s': %3$s")
            , erroredIcons, Model_Setting::instance().getTheme(), missingIcons), _t("Warning"), wxOK | wxICON_WARNING);
    }
    return success;
}

void reverttoDefaultTheme()
{
    Model_Setting::instance().setTheme("default");
    darkFound = false;
    processThemes(mmex::getPathResource(mmex::THEMESDIR), Model_Setting::instance().getTheme(), true);
    processThemes(mmex::getPathResource(mmex::THEMESDIR), Model_Setting::instance().getTheme(), false);
}

void LoadTheme()
{
    darkMode = ( (mmex::isDarkMode() && (Option::THEME_MODE::AUTO == Option::instance().getThemeMode()))
                    || (Option::THEME_MODE::DARK == Option::instance().getThemeMode()));
    filesInVFS = new wxArrayString();

    // Scan first for metadata then for the icons and other files
    darkFound = false;
    if (processThemes(mmex::getPathResource(mmex::THEMESDIR), Model_Setting::instance().getTheme(), true))
        processThemes(mmex::getPathResource(mmex::THEMESDIR), Model_Setting::instance().getTheme(), false);
    else
        if (processThemes(mmex::getPathUser(mmex::USERTHEMEDIR), Model_Setting::instance().getTheme(), true))
            processThemes(mmex::getPathUser(mmex::USERTHEMEDIR), Model_Setting::instance().getTheme(), false);
        else
        {
            wxMessageBox(wxString::Format(_t("Theme %s not found; it may no longer be supported. Reverting to the default theme.")
                , Model_Setting::instance().getTheme()), _t("Warning"), wxOK | wxICON_WARNING);
            reverttoDefaultTheme();
        }

    if (!checkThemeContents(filesInVFS.get()))
    {
        wxMessageBox(wxString::Format(_t("Theme %s has missing items and is incompatible. Reverting to default theme"), Model_Setting::instance().getTheme()), _t("Warning"), wxOK | wxICON_WARNING);
        reverttoDefaultTheme();
        if (!checkThemeContents(filesInVFS.get()))
        {
            // Time to give up as we couldn't find a workable theme
            wxMessageBox(_t("No workable theme found, the installation may be corrupt")
                , _t("Error"), wxOK | wxICON_ERROR);
            exit(EXIT_FAILURE);
        }
    }
}

void CloseTheme()
{
    // Release icons - needed before app closure
    // https://github.com/wxWidgets/wxWidgets/issues/22862
    for (int i = 0; i < numSizes; i++)
        for (int j = 0; j < MAX_PNG; j++)
            programIconBundles[i][j].reset();
}

const wxString mmThemeMetaString(int ref)
{
    auto i = metaDataTrans().find(ref)->second;
    wxString metaLocation = std::get<0>(i);
    if (darkFound && darkMode && !metaLocation.StartsWith("/theme"))
        metaLocation.Prepend("/dark");
    const Pointer ptr(metaLocation.mb_str());
    wxString metaValue = wxString::FromUTF8(GetValueByPointerWithDefault(metaData_doc, ptr, "").GetString());
    if (metaValue.IsEmpty() && !std::get<2>(i))
        metaValue = std::get<1>(i);
    return (metaValue);
}

// helpers
long mmThemeMetaLong(int ref)
{
    long retValue;
    if (!mmThemeMetaString(ref).ToLong(&retValue, 0))
        retValue = 0;
    return retValue;
}

const wxColour mmThemeMetaColour(int ref)
{
    auto c = mmThemeMetaString(ref);
    return wxColour(c);
}

void mmThemeMetaColour(wxWindow *object, int ref, bool foreground)
{
    const wxString c = mmThemeMetaString(ref);
    if (!c.empty())
    {
        if (foreground)
            object->SetForegroundColour(wxColour(c));
        else {
            object->SetBackgroundColour(wxColour(c));
            object->SetForegroundColour(*bestFontColour(c));
        }
    }
}

const std::vector<wxColour> mmThemeMetaColourArray(int ref)
{
    std::vector<wxColour> colours;
    wxStringTokenizer input(mmThemeMetaString(ref));
    while (input.HasMoreTokens())
        colours.push_back(wxColour(input.GetNextToken()));
    return colours;
}

/*const wxBitmap mmBitMap(int ref, int size)
{
    int idx = getIconSizeIdx(size);

    if(idx >= 0)
        return *programIcons[idx][ref].get();

    // Look for a better size match
    int bestAvailSize = size;
    while(idx)
    {
        bestAvailSize /= 2;
        idx = getIconSizeIdx(bestAvailSize);
        if( idx >= 0 )
            break;
    }

    wxSize bmpSize(size, size);
    auto &bundle = programIconBundles[idx][ref];
    return bundle.get()->GetBitmap(bmpSize);
}*/

const wxBitmapBundle mmBitmapBundle(const int ref, const int defSize)
{
    const int idx = getIconSizeIdx(defSize);
    return *programIconBundles[idx][ref];
}
