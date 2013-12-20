/*******************************************************
 Copyright (C) 2007 Madhan Kanagavel
 Copyright (C) 2011 Stefano Giorgio

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

#ifndef _MM_EX_CUSTOMREPORTDIALOG_H_
#define _MM_EX_CUSTOMREPORTDIALOG_H_

#include "defs.h"

class wxStyledTextCtrl;
class mmGeneralReportManager: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( mmGeneralReportManager )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    mmGeneralReportManager( ) {}
    virtual ~mmGeneralReportManager();

    mmGeneralReportManager(wxWindow* parent);

private:
    enum
    {
        ID_TAB1 = 0,
        ID_TAB2,
        ID_TAB3,
        ID_NEW1 = wxID_HIGHEST + 1,
        ID_NEW2,
        ID_DELETE,
        ID_OUTPUT_WIN,
        ID_NOTEBOOK,
        ID_TEMPLATE,
        ID_TYPELABEL
    };

    bool Create(wxWindow* parent
        , wxWindowID id
        , const wxString& caption
        , const wxPoint& pos
        , const wxSize& size
        , long style);
    /// Creates the controls and sizers
    void CreateControls();
    void fillControls();
    void OnOpenTemplateEvt(wxCommandEvent& event);
    void openTemplate(int id = -1);
    void OnSaveTemplate(wxCommandEvent& event);
    void OnSaveTemplateAs(wxCommandEvent& event);
    void OnUpdateScript(wxCommandEvent& event);
    void OnRun(wxCommandEvent& event);
    void OnClose(wxCommandEvent& event);
    void OnItemRightClick(wxTreeEvent& event);
    void OnSelChanged(wxTreeEvent& event);
    void OnLabelChanged(wxTreeEvent& event);
    bool DeleteReport(int id);
    void OnMenuSelected(wxCommandEvent& event);
    void OnSourceTxtChar(wxKeyEvent& event);
    void newReport();

    //wxStyledTextCtrl* m_scriptText;
    //wxStyledTextCtrl* m_templateText;
    wxTextCtrl* m_fileNameCtrl;
    wxHtmlWindow* m_outputHTML;

    wxButton* button_Open_;
    wxButton* button_Save_;
    wxButton* button_SaveAs_;
    wxButton* button_Run_;
    wxButton* button_Clear_;
    wxButton* button_Update_;
    wxTreeCtrl* treeCtrl_;
    wxTreeItemId root_;
    wxTreeItemId selectedItemId_;
    wxString m_selectedGroup;
};

class MyTreeItemData : public wxTreeItemData
{
public:
    MyTreeItemData(int report_id, wxString selectedGroup) : m_report_id(report_id)
        , m_selectedGroup(selectedGroup) { }
    int get_report_id() { return m_report_id; }
    wxString get_group_name() { return m_selectedGroup; }
private:
    int m_report_id;
    wxString m_selectedGroup;
};

#endif

