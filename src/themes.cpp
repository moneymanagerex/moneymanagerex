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

#include "constants.h"
#include "images_list.h"
#include "option.h"
#include "paths.h"
#include "themes.h"
#include "util.h"
#include "model/Model_Setting.h"
#include "reports/htmlbuilder.h"
#include <memory>
#include <wx/mstream.h>
#include <wx/fs_mem.h>
#include <wx/zipstrm.h>

wxIMPLEMENT_DYNAMIC_CLASS(mmThemesDialog, wxDialog);

wxBEGIN_EVENT_TABLE(mmThemesDialog, wxDialog)
EVT_BUTTON(ID_DIALOG_THEME_IMPORT, mmThemesDialog::OnImport)
EVT_BUTTON(ID_DIALOG_THEME_USE, mmThemesDialog::OnUse)
EVT_BUTTON(ID_DIALOG_THEME_DELETE, mmThemesDialog::OnDelete)
EVT_BUTTON(wxID_OK, mmThemesDialog::OnOk)
EVT_LISTBOX(wxID_ANY, mmThemesDialog::OnThemeView)
EVT_HTML_LINK_CLICKED(wxID_ANY, mmThemesDialog::OnHtmlLink)
wxEND_EVENT_TABLE()

const char HTMLPANEL[] = R"(
<h1>%s <a href="%s"><img src="%s"></a></h1>
<h2>%s</h2>
<p>%s</p>
<p><img src="%s"></p>
)";

bool mmThemesDialog::vfsThemeImageLoaded = false;

mmThemesDialog::ThemeEntry mmThemesDialog::getThemeEntry(const wxString& name)
{
    ThemeEntry thisTheme;
    for (const auto &theme : m_themes)
    {
        if (theme.name == name)
        {
            thisTheme = theme;
            continue;
        }
    }
    return thisTheme;
}

void mmThemesDialog::addThemes(const wxString& themeDir, bool isSystem)
{
    wxString chosenTheme = Model_Setting::instance().Theme();
    wxDir directory(themeDir);
    wxLogDebug("Scanning Theme Dir [%s]", themeDir);
    if (!directory.IsOpened()) return;
    wxString filename;

    bool cont = directory.GetFirst(&filename, "*.mmextheme", wxDIR_FILES);
    while (cont)
    {
        wxFileName themeFile(themeDir, filename);
        wxFileInputStream themeZip(themeFile.GetFullPath());
        if (!themeZip.IsOk())
            continue;

        ThemeEntry thisTheme;
        thisTheme.name = themeFile.GetName();
        thisTheme.fullPath = themeFile.GetFullPath();
        thisTheme.isChosen = !thisTheme.name.Cmp(chosenTheme);
        thisTheme.isSystem = isSystem;
        wxLogDebug(">> Found theme [%s]", thisTheme.name);

        wxZipInputStream themeStream(themeZip);
        std::unique_ptr<wxZipEntry> themeEntry;
        int metaDataFound = 0;
        while (themeEntry.reset(themeStream.GetNextEntry()), themeEntry) // != nullptr
        {
            if (!themeZip.CanRead())
                continue;

            const wxFileName fileEntryName = wxFileName(themeEntry->GetName());
            const wxString fileEntry = fileEntryName.GetFullName();

            if (fileEntry == "_theme.json")
            {
                wxLogDebug(">>> JSON found");
                wxMemoryOutputStream memOut(nullptr);
                themeStream.Read(memOut);
                const wxStreamBuffer* buffer = memOut.GetOutputStreamBuffer();
                std::string metaData(static_cast<char *>(buffer->GetBufferStart()), buffer->GetBufferSize());
                thisTheme.metaData = metaData;
                metaDataFound++;
            }
            else if (fileEntry == "_theme.png")
            {
                wxLogDebug(">>> PNG found");
                wxMemoryOutputStream memOut(nullptr);
                themeStream.Read(memOut);
                const wxStreamBuffer* buffer = memOut.GetOutputStreamBuffer();
                thisTheme.bitMap = wxBitmap::NewFromPNGData(buffer->GetBufferStart(), buffer->GetBufferSize());
                metaDataFound++;
            }
        }
        if (2 == metaDataFound)
        {
            wxLogDebug("Theme Manager: Found theme [%s]", thisTheme.name);
            m_themes.push_back(thisTheme);
        }
        cont = directory.GetNext(&filename);
    }
}

mmThemesDialog::~mmThemesDialog()
{
    Model_Infotable::instance().Set("THEMES_DIALOG_SIZE", GetSize());
}

mmThemesDialog::mmThemesDialog(wxWindow *parent, const wxString &name)
{
    this->SetFont(parent->GetFont());
    Create(parent, name);
}

void mmThemesDialog::Create(wxWindow* parent, const wxString &name)
{
    long style = wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER;
    if (!wxDialog::Create(parent, wxID_ANY, _("Theme Manager")
        , wxDefaultPosition, wxDefaultSize, style, name))
    {
        return;
    }

    CreateControls();

    SetIcon(mmex::getProgramIcon());

    ReadThemes();
    RefreshView();

    mmSetSize(this);
    SetMinSize(wxSize(555, 455));
    Centre();
}

void mmThemesDialog::CreateControls()
{
    wxBoxSizer* bSizer0 = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer *sizermain = new wxBoxSizer(wxHORIZONTAL);
    wxSplitterWindow *splittermain = new wxSplitterWindow(this, wxID_ANY);
    splittermain->SetMinimumPaneSize(50); 
    sizermain->Add(splittermain, 1, wxEXPAND,0 );

    wxPanel *pnl1 = new wxPanel(splittermain, wxID_ANY);
    wxBoxSizer *bSizerp1 = new wxBoxSizer(wxVERTICAL);
    m_themesListBox_ = new wxListBox(pnl1, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxLB_SINGLE | wxLB_NEEDED_SB);
    m_themesListBox_->SetSize(200, wxDefaultCoord, wxDefaultCoord, wxDefaultCoord);
    bSizerp1->Add(m_themesListBox_, 1, wxEXPAND, 0);
    pnl1->SetSizer(bSizerp1);
    pnl1->Fit();

    wxPanel *pnl2 = new wxPanel(splittermain, wxID_ANY);
    wxBoxSizer* bSizerp2 = new wxBoxSizer(wxVERTICAL);
    m_themePanel = new wxHtmlWindow(pnl2, wxID_ANY);
    bSizerp2->Add(m_themePanel, 1, wxEXPAND, 0);
    pnl2->SetSizer(bSizerp2);

    bSizer0->Add(sizermain, g_flagsExpand);

    splittermain->SplitVertically(pnl1, pnl2);

    wxBoxSizer* bSizer02 = new wxBoxSizer(wxHORIZONTAL);
    m_importButton = new wxButton(this, ID_DIALOG_THEME_IMPORT, _("&Import"));
    bSizer02->Add(m_importButton, 0, wxALL, 5);
    m_deleteButton = new wxButton(this, ID_DIALOG_THEME_DELETE, _("&Delete"));
    bSizer02->Add(m_deleteButton, 0, wxALL, 5);
    m_useButton = new wxButton(this, ID_DIALOG_THEME_USE, _("&Use"));
    bSizer02->Add(m_useButton, 0, wxALL, 5);
    m_okButton = new wxButton(this, wxID_OK, _("&Close"));
    bSizer02->Add(m_okButton, 0, wxALL, 5);

    bSizer0->Add(bSizer02, g_flagsCenter);

    this->SetSizer(bSizer0);
    this->Layout();
    bSizer0->Fit(this);
}

void mmThemesDialog::ReadThemes()
{
    m_themes.clear();
    addThemes(mmex::getPathResource(mmex::THEMESDIR), true);
    addThemes(mmex::getPathUser(mmex::USERTHEMEDIR), false);

    m_themesListBox_->Clear();
    for (const auto &theme : m_themes)
        m_themesListBox_->Append(theme.name);
    m_themesListBox_->SetStringSelection(Model_Setting::instance().Theme());
    m_themesListBox_->Refresh();
    m_themesListBox_->Update();
}

void mmThemesDialog::RefreshView()
{
    ThemeEntry thisTheme = getThemeEntry(m_themesListBox_->GetString(m_themesListBox_->GetSelection()));

    Document j_doc;
    if (j_doc.Parse(thisTheme.metaData.utf8_str()).HasParseError())
    {
        j_doc.Parse("{}");
    }

    // name
    Value& j_grab = GetValueByPointerWithDefault(j_doc, "/theme/name", "");
    const wxString& s_name = j_grab.IsString() ? wxString::FromUTF8(j_grab.GetString()) : m_themesListBox_->GetString(m_themesListBox_->GetSelection());

    // author
    j_grab = GetValueByPointerWithDefault(j_doc, "/theme/author", "");
    const wxString& s_author = j_grab.IsString() ? wxString::FromUTF8(j_grab.GetString()) : _("Unknown");

    // description
    j_grab = GetValueByPointerWithDefault(j_doc, "/theme/description", "");
    const wxString& s_description = j_grab.IsString() ? wxString::FromUTF8(j_grab.GetString()) : _("No description available");

    // url
    j_grab = GetValueByPointerWithDefault(j_doc, "/theme/url", "");
    const wxString& s_url = j_grab.IsString() ? wxString::FromUTF8(j_grab.GetString()) : "";

    wxString imgUrl, themeImageUrl;
    const wxString webImageName = "web.png";
    const wxString themeImageName = "themeimage.png";

#if defined(__WXMSW__) || defined(__WXMAC__)
    if (vfsThemeImageLoaded)
    {
        wxMemoryFSHandler::RemoveFile(webImageName);
        wxMemoryFSHandler::RemoveFile(themeImageName);
    }
    wxMemoryFSHandler::AddFile(webImageName, mmBitmapBundle(png::WEB).GetBitmap(wxDefaultSize), wxBITMAP_TYPE_PNG);
    imgUrl = "memory:" + webImageName;
    wxMemoryFSHandler::AddFile(themeImageName, thisTheme.bitMap, wxBITMAP_TYPE_PNG);
    themeImageUrl = "memory:" + themeImageName;
    vfsThemeImageLoaded = true;
#else
    mmBitmapBundle(png::WEB).GetBitmap(wxDefaultSize).SaveFile(mmex::getTempFolder() + webImageName, wxBITMAP_TYPE_PNG);
    imgUrl = "file://" + mmex::getTempFolder() + webImageName;
    thisTheme.bitMap.SaveFile(mmex::getTempFolder() + themeImageName, wxBITMAP_TYPE_PNG);
    themeImageUrl = "file://" + mmex::getTempFolder() + themeImageName;
#endif
    mmHTMLBuilder hb;
    hb.init(true);
    wxString myHtml = wxString::Format(HTMLPANEL, s_name, s_url, imgUrl, s_author
        , s_description, themeImageUrl);
    hb.addText(myHtml);
    hb.end(true);
    m_themePanel->SetPage(hb.getHTMLText());

    m_deleteButton->Enable(!thisTheme.isChosen && !thisTheme.isSystem);
    m_useButton->Enable(!thisTheme.isChosen);
}

void mmThemesDialog::OnThemeView(wxCommandEvent&)
{
    RefreshView();
}

void mmThemesDialog::OnImport(wxCommandEvent&)
{
    wxString fileName = wxFileSelector(_("Choose theme file to import")
        , wxEmptyString, wxEmptyString, wxEmptyString
        , "MMX Theme (*.mmextheme)|*.mmextheme"
        , wxFD_FILE_MUST_EXIST | wxFD_OPEN
        , this
    );

    if (fileName.empty())
        return;

    wxString themesDir = mmex::getPathUser(mmex::USERTHEMEDIR);
    // create the themes sub-directory if it doesn't yet exist
    if (!wxDirExists(themesDir))
        wxMkdir(themesDir);

    wxFileName sourceFile(fileName);
    wxFileName destFile(themesDir, sourceFile.GetFullName());

    if (wxFileExists(destFile.GetFullPath()))
    {
        wxString existingThemeText = _("The theme already exists, do you still want to import and overwrite the existing theme?");
        wxMessageDialog msgDlg(this, existingThemeText, destFile.GetName(),
            wxYES_NO | wxNO_DEFAULT | wxICON_EXCLAMATION);
        if (msgDlg.ShowModal() == wxID_NO)
            return;
    }

    wxLogDebug("Theme import: Copying\n%s\nto\n%s", fileName, destFile.GetFullPath());
    if (!wxCopyFile(fileName, destFile.GetFullPath()))
    {
        wxString copyFailedText = _("Something went wrong importing the theme");
        wxMessageBox(copyFailedText, _("Error"), wxOK | wxICON_ERROR);
    }

    ReadThemes();
    RefreshView();

}

void mmThemesDialog::OnDelete(wxCommandEvent&)
{
    ThemeEntry thisTheme = getThemeEntry(m_themesListBox_->GetString(m_themesListBox_->GetSelection()));
    wxString deletingThemeText = _("Are you sure you want to delete this theme? If you want to use it again you will need to re-import it.");
    wxMessageDialog msgDlg(this, deletingThemeText, thisTheme.name,
        wxYES_NO | wxNO_DEFAULT | wxICON_EXCLAMATION);
    if (msgDlg.ShowModal() == wxID_YES)
    {
        if (!wxRemoveFile(thisTheme.fullPath))
        {
            wxString deleteFailedText = _("Something went wrong when attempting to delete the theme");
            wxMessageBox(deleteFailedText, _("Error"), wxOK | wxICON_ERROR);
        }
    }
    ReadThemes();
    RefreshView();

}

void mmThemesDialog::OnUse(wxCommandEvent&)
{
    ThemeEntry thisTheme = getThemeEntry(m_themesListBox_->GetString(m_themesListBox_->GetSelection()));
    wxString changingThemeText = _("Are you sure you want to use this theme? Please note that this will only take effect when MMEX is re-started.");
    wxMessageDialog msgDlg(this, changingThemeText, thisTheme.name,
        wxYES_NO | wxNO_DEFAULT | wxICON_EXCLAMATION);
    if (msgDlg.ShowModal() == wxID_YES)
    {
        Model_Setting::instance().SetTheme(thisTheme.name);
        for (auto it = begin(m_themes); it != end(m_themes); ++it)
            it->isChosen = (it->name == thisTheme.name);
    }
    RefreshView();
}

void mmThemesDialog::OnOk(wxCommandEvent&)
{
    EndModal(wxID_OK);
}

void mmThemesDialog::OnHtmlLink(wxHtmlLinkEvent& event)
{
    wxLaunchDefaultBrowser(event.GetLinkInfo().GetHref());
}
