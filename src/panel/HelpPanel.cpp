/*************************************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2011, 2013 Nikolay & Stefano Giorgio
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
 *************************************************************************/

#include "base/constants.h"
#include "base/paths.h"
#include "mmex.h"

#include "model/UsageModel.h"
#include "mmframe.h"
#include "HelpPanel.h"

BEGIN_EVENT_TABLE(HelpPanel, wxPanel)
    EVT_BUTTON(wxID_BACKWARD, HelpPanel::onHelpPageBack)
    EVT_BUTTON(wxID_FORWARD,  HelpPanel::onHelpPageForward)
END_EVENT_TABLE()

HelpPanel::HelpPanel(
    wxWindow* parent_win,
    mmGUIFrame* frame,
    wxWindowID win_id,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name
) :
    w_frame(frame)
{
    create(parent_win, win_id, pos, size, style, name);
}

bool HelpPanel::create(
    wxWindow* parent_win,
    wxWindowID win_id,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name
) {
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create(parent_win, win_id, pos, size, style, name);

    createControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    UsageModel::instance().pageview(this);

    return true;
}

void HelpPanel::createControls()
{
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer2);

    wxPanel* itemPanel3 = new wxPanel(this, wxID_ANY,
        wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL
    );
    itemBoxSizer2->Add(itemPanel3, 0, wxGROW | wxALL, 5);

    wxBoxSizer* itemBoxSizerHeader = new wxBoxSizer(wxHORIZONTAL);
    itemPanel3->SetSizer(itemBoxSizerHeader);

    wxButton* buttonBack = new wxButton(itemPanel3, wxID_BACKWARD, _t("&Back"));
    wxButton* buttonFordward = new wxButton(itemPanel3, wxID_FORWARD, _t("&Forward"));

    wxStaticText* itemStaticText9 = new wxStaticText(itemPanel3, wxID_ANY,
        mmex::getCaption(_t("Help"))
    );
    itemStaticText9->SetFont(this->GetFont().Larger().Bold());

    itemBoxSizerHeader->Add(buttonBack, 0, wxLEFT, 5);
    itemBoxSizerHeader->Add(buttonFordward, 0, wxLEFT | wxRIGHT, 5);
    itemBoxSizerHeader->Add(itemStaticText9, 0, wxLEFT | wxTOP, 5);

    /**************************************************************************
    Allows help files for a specific language.

    Main default help file name: ./help/index.html
    Default filename names can be found in mmex::getPathDoc(fileIndex)

    Default help files will be used when the language help file are not found.
    **************************************************************************/

    int helpFileIndex = w_frame->getHelpFileIndex();
    const wxString help_file = mmex::getPathDoc(static_cast<mmex::EDocFile>(helpFileIndex));
    w_frame->setHelpFileIndex();
    //wxLogDebug("%s", help_file);
    w_browser = wxWebView::New(this, wxID_ANY, help_file);

    Bind(wxEVT_WEBVIEW_NEWWINDOW, &HelpPanel::onNewWindow, this, w_browser->GetId());

    itemBoxSizer2->Add(w_browser, 1, wxGROW | wxALL, 1);
}

void HelpPanel::onHelpPageBack(wxCommandEvent& /*event*/)
{
    if (w_browser->CanGoBack()) {
        w_browser->GoBack();
        w_browser->SetFocus();
    }
}

void HelpPanel::onHelpPageForward(wxCommandEvent& /*event*/)
{
    if (w_browser->CanGoForward()) {
        w_browser->GoForward();
        w_browser->SetFocus();
    }
}

void HelpPanel::onNewWindow(wxWebViewEvent& event)
{
    const wxString uri = event.GetURL();
    wxLaunchDefaultBrowser(uri);
    event.Veto();
    event.Skip();
}
