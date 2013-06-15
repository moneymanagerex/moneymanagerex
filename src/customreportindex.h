/*************************************************************************
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
 *************************************************************************/
#ifndef _MM_EX_CUSTOMREPORTINDEX_H_
#define _MM_EX_CUSTOMREPORTINDEX_H_

#include "mmcoredb.h"

//===============================================================
// Class: CustomReportIndex
//        Handles the text index file for custom reports  
//===============================================================
class CustomReportIndex
{
public:
    CustomReportIndex();
    
    virtual ~CustomReportIndex();
    
    /** Returns true if file is initialised and open */
    bool HasActiveReports();

    /**  Sets the index file pointer at beginning of file */ 
    void ResetReportsIndex();

    /** Sets and Returns the report title. Reads one line of the index file.
        Sets: validTitle_ currentReportTitle_ currentReportFileName_ reportIsSubReport_ */
    wxString NextReportTitle();
    
    /** Sets and Returns the filename for the given index.  */
    wxString ReportFileName(int index);

    /** Valid after calling: NextReportTitle() */
    bool ValidTitle();

    /** Returns the report title.
        Valid after calling: : NextReportTitle() or ReportFileName() */
    wxString CurrentReportTitle();

    /** Returns true if report is a subreport title.
        Valid after calling: : NextReportTitle() or ReportFileName() */
    bool ReportIsSubReport();

    /** Returns the report filename.
        Valid after calling: : NextReportTitle() or ReportFileName() */
    wxString CurrentReportFileName(bool withfilePath = true);
    wxString CurrentReportFileExt();
    wxString CurrentReportFileType();

    /** Return index of user selected report title as ID: Custom_Report_xxx.
        sets: validTitle_ currentReportTitle_ currentReportFileName_ currentReportFileIndex_ */
    wxString GetUserTitleSelection(const wxString& description = ":");

    /** Gets the report title from the given ID: Custom_Report_xxx.
        sets: currentReportTitle_ currentReportFileName_ currentReportFileIndex_  */
    bool GetSelectedTitleSelection(const wxString& titleIndex);

    /** Adds a new listing to the index file at the currentReportFileIndex_
        call GetUserTitleSelection() or GetSelectedTitleSelection() first. */
    void AddReportTitle(const wxString& reportTitle, bool updateIndex, const wxString& ReportFileName = "", bool isSub = false);

    /** Returns true if the report list is not empty */
    bool ReportListHasItems();

    /** Removes the listing from the index file at location: currentReportFileIndex_  */
    void DeleteCurrentReportTitle(bool including_file);

    /** Returns the title header string for User Dialog */
    wxString UserDialogHeading(); 

    /** Returns the file contents of the report file (.sql or .lua) */ 
    bool GetReportFileData(wxString& reportText);
    
    int GetCustomReportId() {return currentReportFileIndex_;}

private:
    bool activeReports_;
    bool validTitle_;
    bool reportIsSubReport_;
    int  currentReportFileIndex_;

    wxTextFile* indexFile_; 
    wxString currentReportTitle_;
    wxString currentReportFileName_;
    wxString currentReportFileExt_;
    wxString currentReportFileType_;

    void SetNewCurrentFileValues();
    void LoadArrays(wxArrayString& titleArray, wxArrayString& fileNameArray, wxArrayString& subArray);
};

#endif

