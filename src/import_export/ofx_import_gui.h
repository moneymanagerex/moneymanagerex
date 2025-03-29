#ifndef OFX_IMPORT_GUI_H_
#define OFX_IMPORT_GUI_H_

#include "tinyxml2.h"
#include <map>
#include <wx/grid.h>
#include <wx/wx.h>

struct OFXImportResult
{
    wxString fitid;
    wxString date;
    wxString amount;
    wxString ofxPayee;
    wxString importedPayee;
    bool usedRegex;
    wxString regexPattern;
    wxString category;
    wxString transType;
    bool imported;
};

struct OFXImportStats
{
    int autoImportedCount;
    int newPayeesCreated;
    int manuallyAllocated;
    int totalTransactions;
};

class mmPayeeSelectionDialog : public wxDialog
{
    wxDECLARE_EVENT_TABLE();

public:
    mmPayeeSelectionDialog(wxWindow* parent, const wxString& memo, const wxString& suggestedPayeeName, const wxString& fitid, const wxString& date,
                           const wxString& amount, const wxString& transType, int currentTransaction, int totalTransactions, wxLongLong importStartTime);
    wxString GetSelectedPayee() const
    {
        return selectedPayee_;
    }
    bool ShouldUpdateRegex() const
    {
        return shouldUpdateRegex_;
    }
    wxString GetRegexPattern() const
    {
        return regexPattern_;
    }
    bool IsCreateNewPayee() const
    {
        return createNewRadio_->GetValue();
    }
    wxString GetSelectedCategory() const;
    long long GetSelectedCategoryID() const;
    bool ShouldUpdatePayeeCategory() const
    {
        return updatePayeeCategory_;
    }

private:
    enum
    {
        ID_USE_EXISTING = wxID_HIGHEST + 1,
        ID_CREATE_NEW,
        ID_TITLE_CASE,
        ID_UPDATE_CATEGORY,
        ID_INSERT_ROW,
        ID_DELETE_ROW
    };

    void OnUseExistingPayee(wxCommandEvent& event);
    void OnCreateNewPayee(wxCommandEvent& event);
    void OnUpdateRegex(wxCommandEvent& event);
    void OnPayeeChoice(wxCommandEvent& event);
    void OnOK(wxCommandEvent& event);
    void OnTitleCase(wxCommandEvent& event);
    void OnUpdateCategoryToggle(wxCommandEvent& event);
    void OnGridLabelLeftClick(wxGridEvent& event);
    void OnInsertRow(wxCommandEvent& event);
    void OnDeleteRow(wxCommandEvent& event);
    void OnCategorySelection(wxCommandEvent& event);
    void OnCategoryFocus(wxFocusEvent& event); // Added
    void UpdateOKButton(wxCommandEvent& event);
    void LoadRegexPatterns(const wxString& payeeName);
    void AddCategoryToChoice(wxChoice* choice, long long categId, const std::map<long long, Model_Category::Data>& categoryMap, int level);

    wxRadioButton* useExistingRadio_;
    wxRadioButton* createNewRadio_;
    wxChoice* payeeChoice_;
    wxTextCtrl* newPayeeTextCtrl_;
    wxButton* titleCaseButton_;
    wxChoice* categoryChoice_;
    wxCheckBox* updateRegexCheckBox_;
    wxGrid* regexGrid_;
    wxButton* okButton_;
    wxButton* updateCategoryButton_;
    wxButton* insertRowButton_;
    wxButton* deleteRowButton_;
    wxStaticText* existingPayeeLabel_;
    wxStaticText* newPayeeLabel_;
    wxBoxSizer* payeeSizer_;
    wxString selectedPayee_;
    wxString regexPattern_;
    bool shouldUpdateRegex_;
    bool updatePayeeCategory_;
    bool memoAdded_;
    long long initialCategoryId_;
    std::map<long long, Model_Category::Data> categoryMap;
    int currentTransaction_;
    int totalTransactions_;
    wxLongLong importStartTime_;
    bool categoryManuallyChanged_; // Tracks if the user manually changed the category
};




class mmOFXImportDialog : public wxDialog
{
    wxDECLARE_EVENT_TABLE();

public:
    mmOFXImportDialog(wxWindow* parent);
    virtual ~mmOFXImportDialog();
    void loadRegexMappings();

private:
    void OnBrowse(wxCommandEvent& event);
    void OnImport(wxCommandEvent& event);
    bool ParseOFX(const wxString& filePath, std::vector<OFXImportResult>& importResults, OFXImportStats& stats);
    bool ImportTransactions(tinyxml2::XMLElement* banktranlist, wxLongLong accountID, std::vector<OFXImportResult>& results, OFXImportStats& stats);
    wxString getPayeeName(const wxString& memo, bool& usedRegex, wxString& matchedPattern);

    wxTextCtrl* fileNameCtrl_;
    wxChoice* accountDropDown_;
    wxLongLong account_id_;
    wxCheckBox* payeeRegExCheckBox_;
    long long transferCategId_;
    wxLongLong importStartTime_;
    std::map<wxString, wxString> payeeRegexMap_;
};

class mmOFXImportSummaryDialog : public wxDialog
{
    wxDECLARE_EVENT_TABLE();

public:
    mmOFXImportSummaryDialog(wxWindow* parent, const std::vector<OFXImportResult>& results, const OFXImportStats& stats, wxLongLong importStartTime);

private:
    void OnOK(wxCommandEvent& event);
    void OnGridMouseWheel(wxMouseEvent& event);
    wxString FormatTimeTaken(double seconds) const; // Added helper function

    int autoImportedCount_;
    int newPayeesCreated_;
    int manuallyAllocated_;
    int totalTransactions_;
    wxLongLong importStartTime_;
    wxScrolledWindow* scrolledWindow;
};



#endif // OFX_IMPORT_GUI_H_