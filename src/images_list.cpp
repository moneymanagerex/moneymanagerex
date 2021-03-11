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
#include "paths.h"
#include <wx/zipstrm.h>
#include <wx/rawbmp.h>
#include <wx/fs_mem.h>
#include "../3rd/lunasvg/include/svgdocument.h"

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

bool buildBitmapsFromSVG(void)
{
    wxString myTheme = Model_Setting::instance().Theme();
    themes->Clear();
    bool myThemeFound = false;
    wxLogDebug ("Loading Theme: %s", myTheme);
    const wxString iconsFile = mmex::getPathResource(mmex::THEMES_ZIP);
    wxFileInputStream iconZip(iconsFile);
    wxASSERT(iconZip.IsOk());   // Make sure we can open find the Zip

    wxZipInputStream iconStream(iconZip);
    std::unique_ptr<wxZipEntry> zipEntry;
    const size_t bufSize = 32768;   // Should really have CSS/SVGs smaller than this.
    unsigned char buffer[bufSize];

    wxString thisTheme;
    while (zipEntry.reset(iconStream.GetNextEntry()), zipEntry) // != nullptr
    {
        wxASSERT(iconZip.CanRead()); // Make sure we can read the Zip Entry

        const wxFileName fileEntryName = wxFileName(zipEntry->GetName());
        const wxString fileEntry = fileEntryName.GetFullName();
        const wxString pathEntry = fileEntryName.GetFullPath();
        const int dirLevel = static_cast<int>(fileEntryName.GetDirCount());

        //wxLogDebug("fileEntry: level=%d, fullpath=%s, filename=%s", dirLevel, pathEntry, fileEntry);
        if (1 == dirLevel && fileEntryName.IsDir())
        {   
            thisTheme = fileEntryName.GetDirs()[0];
            wxLogDebug("Found Theme: %s", thisTheme);
            themes->Add(thisTheme);
            if (!thisTheme.Cmp(myTheme))
                myThemeFound = true;
        }

        if (thisTheme.Cmp(myTheme) || fileEntryName.IsDir())
            continue;   // We can skip if it's not our theme

        std::string fileName = std::string(fileEntry.mb_str());

        bool isMasterCSS = false;
        if (!fileName.compare("master.css"))
            isMasterCSS = true;
        else
            // Skip anything in the ZIP that we don't recognise as a valid SVG
            if (!iconName2enum.count(fileName)) continue;

        wxString fileData;
        while (!iconStream.Eof()) {
            iconStream.Read(buffer, bufSize);
            fileData.Append(buffer);
            if (iconStream.LastRead() > 0) {
                if (isMasterCSS)
                {
                    wxMemoryFSHandler::AddFile("master.css",fileData);
                    continue;
                }
                lunasvg::SVGDocument document;
                document.loadFromData(std::string(fileData.mb_str()));

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
        }
    }
    return (myThemeFound);
}

const wxBitmap mmBitmap(int ref)
{
    // Load icons on first use
    if (!iconsLoaded) 
    { 
        themes = new wxArrayString();
        if (!buildBitmapsFromSVG())     // safety net in case a theme is removed or name changed
        {
            Model_Setting::instance().SetTheme("default");
            buildBitmapsFromSVG();
        }
        iconsLoaded = true;
    }
    int x = Option::instance().getIconSize();
    return x == 16 ? *programIcons16[ref] : x == 24 ? *programIcons24[ref] : x == 32 ? *programIcons32[ref] : *programIcons48[ref];
}

wxArrayString getThemes()
{
    return *themes;
}
