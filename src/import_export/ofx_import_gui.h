#ifndef OFX_IMPORT_GUI_H_
#define OFX_IMPORT_GUI_H_

#include "model/Model_Category.h"
#include "model/Model_Payee.h"
#include <map>
#include <wx/event.h>
#include <wx/grid.h>
#include <wx/wx.h>
#include <wx/xml/xml.h>

struct OFXImportResult
{
    wxString fitid;
    wxString date;
    wxString amount;
    wxString ofxPayee;
    wxString importedPayee;
    bool usedRegex;
    wxString regexPattern;
    wxString matchRegexPattern;
    wxString category;
    wxString transType;
    bool imported;
    wxString matchMode;
    double matchConfidence;
};

struct OFXImportStats
{
    int totalTransactions;
    int autoImportedCount;
    int newPayeesCreated;
    int manuallyAllocated;
    int skippedDuplicates;    // Added for duplicate transactions
    int skippedErrors;        // Added for parsing or save errors
    int skippedManual;        // Added for manual cancellations
    int importedTransactions; // Added to track successful imports
    OFXImportStats()
        : totalTransactions(0), autoImportedCount(0), newPayeesCreated(0), manuallyAllocated(0), skippedDuplicates(0), skippedErrors(0), skippedManual(0),
          importedTransactions(0)
    {
    }
};

class wxInt64ClientData : public wxClientData
{
public:
    wxInt64ClientData(int64_t value) : value_(value)
    {
    }
    int64_t GetValue() const
    {
        return value_;
    }

private:
    int64_t value_;
};

class mmPayeeSelectionDialog : public wxDialog
{
    wxDECLARE_EVENT_TABLE();

public:
    mmPayeeSelectionDialog(wxWindow* parent, const wxString& memo, const wxString& suggestedPayeeName, const wxString& fitid, const wxString& date,
                           const wxString& amount, const wxString& transType, int currentTransaction, int newTransactions, wxLongLong importStartTime,
                           double matchConfidence, const wxString& matchMethod, int totalTransactions);

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
    bool ShouldCancelImport() const
    {
        return shouldCancelImport_;
    }
    wxString GetSelectedCategory() const
    {
        int sel = categoryChoice_->GetSelection();
        if (sel == 0 || sel == wxNOT_FOUND)
            return _("Uncategorized");
        return categoryChoice_->GetString(sel).Trim(false);
    }
    long long GetSelectedCategoryID() const
    {
        int sel = categoryChoice_->GetSelection();
        if (sel == wxNOT_FOUND)
            return -1;
        wxStringClientData* data = dynamic_cast<wxStringClientData*>(categoryChoice_->GetClientObject(sel));
        if (!data)
            return -1;
        long long categId;
        if (!data->GetData().ToLongLong(&categId))
            return -1;
        return categId;
    }
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

    bool shouldCancelImport_ = false;
    void OnInitDialog(wxInitDialogEvent& event);
    void OnUseExistingPayee(wxCommandEvent& event);
    void OnCreateNewPayee(wxCommandEvent& event);
    void OnUpdateRegex(wxCommandEvent& event);
    void OnPayeeChoice(wxCommandEvent& event);
    void OnOK(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnTitleCase(wxCommandEvent& event);
    void OnUpdateCategoryToggle(wxCommandEvent& event);
    void OnGridLabelLeftClick(wxGridEvent& event);
    void OnInsertRow(wxCommandEvent& event);
    void OnDeleteRow(wxCommandEvent& event);
    void OnCategorySelection(wxCommandEvent& event);
    void OnCategoryFocus(wxFocusEvent& event);
    void UpdateOKButton(wxCommandEvent& event);
    void LoadRegexPatterns(wxInt64ClientData* payeeIdData);
    void LoadRegexPatterns(const wxString& payeeName);
    void AddCategoryToChoice(wxChoice* choice, long long categId, const std::map<long long, Model_Category::Data>& categoryMap, int level);

    double matchConfidence_;
    int totalTransactions_;
    wxString matchMethod_;
    wxStaticText* confidenceLabel_;
    wxString suggestedPayeeName_;
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
    int newTransactions_;
    wxLongLong importStartTime_;
    bool categoryManuallyChanged_;
    wxString memo_;
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
    void OnUseFuzzyMatchingToggled(wxCommandEvent& event);
    bool ParseOFX(const wxString& filePath, std::vector<OFXImportResult>& importResults, OFXImportStats& stats);
    bool ImportTransactions(wxXmlNode* banktranlist, wxLongLong accountID, std::vector<OFXImportResult>& results, OFXImportStats& stats);
    wxString getPayeeName(const wxString& memo, bool& usedRegex, wxString& regexPattern, double& matchConfidence, wxString& matchMethod,
                          wxString& matchRegexPattern);

    wxTextCtrl* fileNameCtrl_;
    wxChoice* accountDropDown_;
    wxLongLong account_id_;
    wxCheckBox* payeeRegExCheckBox_;
    wxCheckBox* useFuzzyMatchingCheckBox_;
    wxChoice* fuzzyConfidenceChoice_;
    wxCheckBox* markFuzzyFollowUpCheckBox_;
    long long transferCategId_;
    wxLongLong importStartTime_;
    std::map<wxString, wxString> payeeRegexMap_;
    wxString selectedPayee_;
    double GetMinFuzzyConfidence() const;
    wxCheckBox* promptFuzzyConfirmationCheckBox_;
};

class mmOFXImportSummaryDialog : public wxDialog
{
    wxDECLARE_EVENT_TABLE();

public:
    mmOFXImportSummaryDialog(wxWindow* parent, const std::vector<OFXImportResult>& results, const OFXImportStats& stats, wxLongLong importStartTime);

private:
    void OnOK(wxCommandEvent& event);
    void OnGridMouseWheel(wxMouseEvent& event);
    wxString FormatTimeTaken(double seconds) const;

    int autoImportedCount_;
    int newPayeesCreated_;
    int manuallyAllocated_;
    int totalTransactions_;
    wxLongLong importStartTime_;
    wxScrolledWindow* scrolledWindow;
};

#endif // OFX_IMPORT_GUI_H_