/*******************************************************
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
 ********************************************************/

#ifndef MM_EX_THEMES_H_
#define MM_EX_THEMES_H_

#include "defs.h"
#include <wx/bitmap.h>
#include <vector>

class mmThemesDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmThemesDialog);
    wxDECLARE_EVENT_TABLE();

public:
    ~mmThemesDialog();
    mmThemesDialog(wxWindow* parent, const wxString& name = "Themes Dialog");

private:

    static bool vfsThemeImageLoaded;
    struct ThemeEntry
    {
        wxString name;
        wxString fullPath;
        bool isSystem;
        bool isChosen;
        wxString metaData;
        wxBitmap bitMap;
    };

    std::vector<ThemeEntry> m_themes;

    wxListBox* m_themesListBox_ = nullptr;
    wxHtmlWindow* m_themePanel = nullptr;
    wxButton* m_importButton = nullptr;
    wxButton* m_deleteButton = nullptr;
    wxButton* m_useButton = nullptr;
    wxButton* m_okButton = nullptr;

private:
    mmThemesDialog() {}

    void Create(wxWindow* parent, const wxString &name);
    void CreateControls();
    void ReadThemes();
    void RefreshView();
    void addThemes(const wxString& themeDir, bool isSystem);
    ThemeEntry getThemeEntry(const wxString& name);
 
    void OnOk(wxCommandEvent&);
    void OnImport(wxCommandEvent&);
    void OnDelete(wxCommandEvent& event);
    void OnUse(wxCommandEvent&);
    void OnHtmlLink(wxHtmlLinkEvent& event);
    void OnThemeView(wxCommandEvent&);

    enum
    {
        /* Themes Dialog */
        ID_DIALOG_TRANS_TYPE = wxID_HIGHEST + 555,
        ID_DIALOG_THEME_IMPORT,
        ID_DIALOG_THEME_DELETE,
        ID_DIALOG_THEME_USE
    };
};

#endif // MM_EX_THEMES_H_
