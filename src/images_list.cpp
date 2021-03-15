/*******************************************************
Copyright (C) 2014, 2015 Nikolay Akimov
Copyright (C) 2021 Mark Whalley (mark@ipx.co.uk)

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
#include "util.h"
#include <wx/image.h>
#include <wx/bitmap.h>
#include <map>
#include <wx/sharedptr.h>
#include "paths.h"
#include <wx/zipstrm.h>
#include <wx/rawbmp.h>
#include <wx/fs_mem.h>
#include <wx/mstream.h>
#include "../3rd/lunasvg/include/svgdocument.h"

// SVG filename in Zip, the PNG enum to which it relates, the background color for the generated bitmap 0 = transparent
static const std::map<std::string, std::pair<int, std::uint32_t>> iconName2enum = {
    { "NEW_DB.svg", { NEW_DB, 0 } },
    { "OPEN.svg", { OPEN, 0 } },
    { "NEW_ACC.svg", { NEW_ACC, 0 } },
    { "HOME.svg", { HOME, 0 } },
    { "CATEGORY.svg", { CATEGORY, 0 } },
    { "PAYEE.svg", { PAYEE, 0 } },
    { "CURR.svg", { CURR, 0 } },
    { "FILTER.svg", { FILTER, 0 } }, 
    { "GRM.svg", { GRM, 0 } },
    { "OPTIONS.svg", { OPTIONS, 0 } },
    { "NEW_TRX.svg", { NEW_TRX, 0 } },
    { "NEW_NEWS.svg", { NEW_NEWS, 0 } },
    { "NEWS.svg", { NEWS, 0 } },
    { "CURRATES.svg", { CURRATES, 0 } },
    { "FULLSCREEN.svg", { FULLSCREEN, 0 } },
    { "PRINT.svg", { PRINT, 0 } },
    { "ABOUT.svg", { ABOUT, 0 } },
    { "HELP.svg", { HELP, 0 } },

    // Navigation
    { "NAV_HOME.svg", { NAV_HOME, 0xFFFFFFFF } },
    { "ALLTRANSACTIONS.svg", { ALLTRANSACTIONS, 0xFFFFFFFF } },
    { "NAV_FILTER.svg", { NAV_FILTER, 0xFFFFFFFF } },
    { "NAV_GRM.svg", { NAV_GRM, 0xFFFFFFFF } },
    { "NAV_HELP.svg", { NAV_HELP, 0xFFFFFFFF } },
    { "FAVOURITE.svg", { FAVOURITE, 0xFFFFFFFF } },
    { "SAVINGS_NORMAL.svg", { SAVINGS_NORMAL, 0xFFFFFFFF } },
    { "SAVINGS_CLOSED.svg", { SAVINGS_CLOSED, 0xFFFFFFFF } },
    { "CC_NORMAL.svg", { CC_NORMAL, 0xFFFFFFFF } },
    { "CC_CLOSED.svg", { CC_CLOSED, 0xFFFFFFFF } },
    { "CASH_NORMAL.svg", { CASH_NORMAL, 0xFFFFFFFF } },
    { "CASH_CLOSED.svg", { CASH_CLOSED, 0xFFFFFFFF } },
    { "LOAN_ACC_NORMAL.svg", { LOAN_ACC_NORMAL, 0xFFFFFFFF } },
    { "LOAN_ACC_CLOSED.svg", { LOAN_ACC_CLOSED, 0xFFFFFFFF } },
    { "TERM_NORMAL.svg", { TERM_NORMAL, 0xFFFFFFFF } },
    { "TERM_CLOSED.svg", { TERM_CLOSED, 0xFFFFFFFF } },
    { "STOCKS_NORMAL.svg", { STOCKS_NORMAL, 0xFFFFFFFF } },
    { "STOCKS_CLOSED.svg", { STOCKS_CLOSED, 0xFFFFFFFF } },
    { "ASSET_NORMAL.svg", { ASSET_NORMAL, 0xFFFFFFFF } },
    { "ASSET_CLOSED.svg", { ASSET_CLOSED, 0xFFFFFFFF } },
    { "RECURRING.svg", { RECURRING, 0xFFFFFFFF } },
    { "BUDGET.svg", { BUDGET, 0xFFFFFFFF } },
    { "PIE_CHART.svg", { PIE_CHART, 0xFFFFFFFF } },
     
    // Status
    { "UNRECONCILED.svg", { UNRECONCILED, 0 } },
    { "RECONCILED.svg", { RECONCILED, 0 } },
    { "DUPLICATE_STAT.svg", { DUPLICATE_STAT, 0 } },
    { "FOLLOW_UP.svg", { FOLLOW_UP, 0 } },
    { "VOID_STAT.svg", { VOID_STAT, 0 } },
    { "PROFIT.svg", { PROFIT, 0 } },
    { "LOSS.svg", { LOSS, 0 } },
    { "LED_OFF.svg", { LED_OFF, 0 } },
    { "LED_GREEN.svg", { LED_GREEN, 0 } },
    { "LED_YELLOW.svg", { LED_YELLOW, 0 } },
    { "LED_RED.svg", { LED_RED, 0 } },
    { "RUN.svg", { RUN, 0 } },
    { "RUN_AUTO.svg", { RUN_AUTO, 0 } },

    // Assets
    { "PROPERTY.svg", { PROPERTY, 0 } },
    { "CAR.svg", { CAR, 0 } },
    { "HOUSEHOLD_OBJ.svg", { HOUSEHOLD_OBJ, 0 } },
    { "ART.svg", { ART, 0 } },
    { "JEWELLERY.svg", { JEWELLERY, 0 } },
    { "CASH.svg", { CASH, 0 } },
    { "OTHER.svg", { OTHER, 0 } },

    //Controls
    { "CLEAR.svg", { CLEAR, 0 } },
    { "CLIP.svg", { CLIP, 0 } },
    { "DOWNARROW.svg", { DOWNARROW, 0 } },
    { "UPARROW.svg", { UPARROW, 0 } },
    { "LEFTARROW.svg", { LEFTARROW, 0 } },
    { "RIGHTARROW.svg", { RIGHTARROW, 0 } },
    { "IMPORT.svg", { IMPORT, 0 } },
    { "MORE_OPTIONS.svg", { MORE_OPTIONS, 0 } },
    { "RELOCATION.svg", { RELOCATION, 0 } },
    { "SAVE.svg", { SAVE, 0 } },
    { "TRANSFILTER.svg", { TRANSFILTER, 0 } },
    { "TRANSFILTER_ACTIVE.svg", { TRANSFILTER_ACTIVE, 0 } },
    { "TRXNUM.svg", { TRXNUM, 0 } },
    { "UPDATE.svg", { UPDATE, 0 } },
    { "WEB.svg", { WEB, 0 } },

    // Settings
    { "GENERAL.svg", { GENERAL, 0 } },
    { "VIEW.svg", { VIEW, 0 } },
    { "ATTACHMENTS.svg", { ATTACHMENTS, 0 } },
    { "NETWORK.svg", { NETWORK, 0 } },
    { "OTHERS.svg", { OTHERS, 0 } },

    // Extra Account Icons
    { "ACC_MONEY.svg", { ACC_MONEY, 0xFFFFFFFF } },
    { "ACC_EURO.svg", { ACC_EURO, 0xFFFFFFFF } },
    { "ACC_FLAG.svg", { ACC_FLAG, 0xFFFFFFFF } },
    { "ACC_ABOUT.svg", { ACC_ABOUT, 0xFFFFFFFF } },
    { "ACC_COINS.svg", { ACC_COINS, 0xFFFFFFFF } },
    { "ACC_CLOCK.svg", { ACC_CLOCK, 0xFFFFFFFF } },
    { "ACC_CAT.svg", { ACC_CAT, 0xFFFFFFFF } },
    { "ACC_DOG.svg", { ACC_DOG, 0xFFFFFFFF } },
    { "ACC_TREES.svg", { ACC_TREES, 0xFFFFFFFF } },
    { "ACC_HOURGLASS.svg", { ACC_HOURGLASS, 0xFFFFFFFF } },
    { "ACC_WORK.svg", { ACC_WORK, 0xFFFFFFFF } },
    { "ACC_PAYPAL.svg", { ACC_PAYPAL, 0xFFFFFFFF } },
    { "ACC_WALLET.svg", { ACC_WALLET, 0xFFFFFFFF } },
    { "ACC_RUBIK.svg", { ACC_RUBIK, 0xFFFFFFFF } },

    // Other
    { "EMPTY.svg", { EMPTY, 0 } }
};

static bool iconsLoaded = false;

static wxSharedPtr<wxBitmap> programIcons16[MAX_PNG];
static wxSharedPtr<wxBitmap> programIcons24[MAX_PNG];
static wxSharedPtr<wxBitmap> programIcons32[MAX_PNG];
static wxSharedPtr<wxBitmap> programIcons48[MAX_PNG];

static wxSharedPtr<wxArrayString> themes;
static wxArrayString* filesInVFS;

static const std::map<int, wxBitmap> navtree_images()
{
    return{
        { HOUSE_PNG, mmBitmap(png::NAV_HOME) }
        , { ALLTRANSACTIONS_PNG, mmBitmap(png::ALLTRANSACTIONS) }
        , { SCHEDULE_PNG, mmBitmap(png::RECURRING) }
        , { CALENDAR_PNG, mmBitmap(png::BUDGET) }
        , { PIECHART_PNG, mmBitmap(png::PIE_CHART) }
        , { HELP_PNG, mmBitmap(png::NAV_HELP) }
        , { FAVOURITE_PNG, mmBitmap(png::FAVOURITE) }
        , { FILTER_PNG, mmBitmap(png::NAV_FILTER) }
        , { ASSET_NORMAL_PNG, mmBitmap(png::ASSET_NORMAL) }
        , { ASSET_CLOSED_PNG, mmBitmap(png::ASSET_CLOSED) } 
        , { CUSTOMSQL_PNG, mmBitmap(png::NAV_GRM) }
        , { CUSTOMSQL_GRP_PNG, mmBitmap(png::NAV_GRM) } //TODO: GRM rep group ico
        , { SAVINGS_ACC_NORMAL_PNG, mmBitmap(png::SAVINGS_NORMAL) }
        , { SAVINGS_ACC_CLOSED_PNG, mmBitmap(png::SAVINGS_CLOSED) }
        , { CARD_ACC_NORMAL_PNG, mmBitmap(png::CC_NORMAL) }
        , { CARD_ACC_CLOSED_PNG, mmBitmap(png::CC_CLOSED) }
        , { TERMACCOUNT_NORMAL_PNG, mmBitmap(png::TERM_NORMAL) }
        , { TERM_ACC_CLOSED_PNG, mmBitmap(png::TERM_CLOSED) }
        , { STOCK_ACC_NORMAL_PNG, mmBitmap(png::STOCKS_NORMAL) }
        , { STOCK_ACC_CLOSED_PNG, mmBitmap(png::STOCKS_CLOSED) }
        , { CASH_ACC_NORMAL_PNG, mmBitmap(png::CASH_NORMAL) }
        , { CASH_ACC_CLOSED_PNG, mmBitmap(png::CASH_CLOSED) }
        , { LOAN_ACC_NORMAL_PNG, mmBitmap(png::LOAN_ACC_NORMAL) }
        , { LOAN_ACC_CLOSED_PNG, mmBitmap(png::LOAN_ACC_CLOSED) }
    };
};

// Custom icons for accounts
static const std::map<int, wxBitmap> acc_images()
{
    return
    {
        { ACC_ICON_MONEY, mmBitmap(png::ACC_MONEY) }
        , { ACC_ICON_EURO, mmBitmap(png::ACC_EURO) }
        , { ACC_ICON_FLAG, mmBitmap(png::ACC_FLAG) }
        , { ACC_ICON_COINS, mmBitmap(png::ACC_COINS) }
        , { ACC_ICON_ABOUT, mmBitmap(png::ACC_ABOUT) }
        , { ACC_ICON_CLOCK, mmBitmap(png::ACC_CLOCK) }
        , { ACC_ICON_CAT, mmBitmap(png::ACC_CAT) }
        , { ACC_ICON_DOG, mmBitmap(png::ACC_DOG) }
        , { ACC_ICON_TREES, mmBitmap(png::ACC_TREES) }
        , { ACC_ICON_HOURGLASS, mmBitmap(png::ACC_HOURGLASS) }
        , { ACC_ICON_WORK, mmBitmap(png::ACC_WORK) }
        , { ACC_ICON_PAYPAL, mmBitmap(png::ACC_PAYPAL)}
        , { ACC_ICON_WALLET, mmBitmap(png::ACC_WALLET) }
        , { ACC_ICON_RUBIK, mmBitmap(png::ACC_RUBIK) }
    };
}

wxImageList* navtree_images_list()
{
    int x = Option::instance().getIconSize();
    if (x < 16) x = 16;
    if (x > 48) x = 48;
    wxImageList* imageList = createImageList();
    for (const auto& img : navtree_images())
    {
        wxASSERT(img.second.GetHeight() == x && img.second.GetWidth() == x);
        imageList->Add(img.second);
    }
    for (const auto& img : acc_images())
    {
        wxASSERT(img.second.GetHeight() == x && img.second.GetWidth() == x);
        imageList->Add(img.second);
    }

    return imageList;
}

typedef wxAlphaPixelData PixelData;
wxBitmap* CreateBitmapFromRGBA(unsigned char *rgba, int size)
{
   wxBitmap* bitmap = new wxBitmap(size, size, wxBITMAP_SCREEN_DEPTH);
   PixelData bmdata( *bitmap );

   PixelData::Iterator dst(bmdata);

   for( int y = 0; y < size; y++)
   {
      dst.MoveTo(bmdata, 0, y);
      for(int x = 0; x < size; x++)
      {
         // wxBitmap contains rgb values pre-multiplied with alpha
         unsigned char a = rgba[3];
         // you could use "/256" here to speed up the code,
         // but at the price of being not 100% accurate
         dst.Red() = rgba[0] * a / 255;
         dst.Green() = rgba[1] * a / 255;
         dst.Blue() = rgba[2] * a / 255;
         dst.Alpha() = a;
         dst++;
         rgba += 4;
      }
   }
   return bitmap;
} 

bool buildBitmapsFromSVG(wxString iconsFile, wxString myTheme)
{
    wxLogDebug ("Scanning [%s] for Theme [%s]", iconsFile, myTheme);
    wxFileInputStream iconZip(iconsFile);
    wxASSERT(iconZip.IsOk());   // Make sure we can open find the Zip

    wxZipInputStream iconStream(iconZip);
    std::unique_ptr<wxZipEntry> zipEntry;

    wxString thisTheme;
    bool themeMatched = false;
    while (zipEntry.reset(iconStream.GetNextEntry()), zipEntry) // != nullptr
    {
        wxASSERT(iconZip.CanRead()); // Make sure we can read the Zip Entry

        const wxFileName fileEntryName = wxFileName(zipEntry->GetName());
        const wxString fileEntry = fileEntryName.GetFullName();
        const wxString pathEntry = fileEntryName.GetFullPath();
        std::string fileName = std::string(fileEntry.mb_str());

        const int dirLevel = static_cast<int>(fileEntryName.GetDirCount());

        //wxLogDebug("fileEntry: level=%d, fullpath=%s, filename=%s", dirLevel, pathEntry, fileEntry);
        if (1 == dirLevel && fileEntryName.IsDir())
        {   
            thisTheme = fileEntryName.GetDirs()[0];
            wxLogDebug("Found Theme: %s", thisTheme);
            if (wxNOT_FOUND == themes->Index(thisTheme)) // Add user theme if not in the existing list
                themes->Add(thisTheme); 
            if (!thisTheme.Cmp(myTheme))
                themeMatched = true;
        }

        if (thisTheme.Cmp(myTheme) || fileEntryName.IsDir())
            continue;   // We can skip if it's not our theme

        wxMemoryOutputStream memOut(nullptr);
        iconStream.Read(memOut);
        const wxStreamBuffer* buffer = memOut.GetOutputStreamBuffer();
     
        // If the file does not match an icon file then just load it into VFS
        if (!iconName2enum.count(fileName))
        {
            if (wxNOT_FOUND != filesInVFS->Index(fileName)) // If already loaded then remove and replace
                wxMemoryFSHandler::RemoveFile(fileName);
            wxMemoryFSHandler::AddFile(fileName, buffer->GetBufferStart()
                , buffer->GetBufferSize());
            filesInVFS->Add(fileName);
            wxLogDebug("File: %s has been copied to VFS", fileName);
            continue;
        }

        // So we have an icon file now, so need to convert from SVG to PNG at various resolutions and store
        // it away for use
        
        lunasvg::SVGDocument document;
        std::string svgDoc(static_cast<char *>(buffer->GetBufferStart()), buffer->GetBufferSize());
        document.loadFromData(svgDoc);

        int svgEnum = iconName2enum.find(fileName)->second.first;
        std::uint32_t bgColor = iconName2enum.find(fileName)->second.second;
        lunasvg::Bitmap bitmap;

        // Generate bitmaps at the resolutions used by the program - 16, 24, 32, 48
        bitmap = document.renderToBitmap(16, 16, 96.0, bgColor);
        programIcons16[svgEnum] = CreateBitmapFromRGBA(bitmap.data(), 16);
        bitmap = document.renderToBitmap(24, 24, 96.0, bgColor);
        programIcons24[svgEnum] = CreateBitmapFromRGBA(bitmap.data(), 24);
        bitmap = document.renderToBitmap(32, 32, 96.0, bgColor);
        programIcons32[svgEnum] = CreateBitmapFromRGBA(bitmap.data(), 32);
        bitmap = document.renderToBitmap(48, 48, 96.0, bgColor);
        programIcons48[svgEnum] = CreateBitmapFromRGBA(bitmap.data(), 48);
    }
    return (themeMatched);
}

const wxBitmap mmBitmap(int ref)
{
    // Load icons on first use
    if (!iconsLoaded) 
    { 
        themes = new wxArrayString();
        bool myThemeFound;
        filesInVFS = new wxArrayString();
        if (!(myThemeFound = buildBitmapsFromSVG(mmex::getPathResource(mmex::THEMES_ZIP), Model_Setting::instance().Theme())))   
        {   // current theme does not match a system theme so load the default
            buildBitmapsFromSVG(mmex::getPathResource(mmex::THEMES_ZIP), "default");
        }
        // Check if user override exists for the theme, and if does process it
        if (wxFileExists(mmex::getPathUser(mmex::USERTHEME)))
            myThemeFound = buildBitmapsFromSVG(mmex::getPathUser(mmex::USERTHEME), Model_Setting::instance().Theme()) || myThemeFound;  

        if (!myThemeFound)  // if we never found the current theme then alert user and reset to default
        {
            wxMessageBox(wxString::Format(_("Theme %s not found within the application, it may no longer be supported. Reverting to default theme"), Model_Setting::instance().Theme()), _("Warning"), wxOK | wxICON_WARNING);
            Model_Setting::instance().SetTheme("default");
        }
        delete filesInVFS; 
        iconsLoaded = true;
    }
    int x = Option::instance().getIconSize();
    return x == 16 ? *programIcons16[ref] : x == 24 ? *programIcons24[ref] : x == 32 ? *programIcons32[ref] : *programIcons48[ref];
}

wxArrayString getThemes()
{
    return *themes;
}
