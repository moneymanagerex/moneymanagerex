#ifndef PAYEE_MATCH_AND_MERGE_H_
#define PAYEE_MATCH_AND_MERGE_H_

#include "model/Model_Checking.h"
#include "model/Model_Payee.h"
#include <vector>
#include <wx/string.h>

struct PayeeMatchResult
{
    long long PayeeID;            // Payee ID from Model_Payee
    wxString Name;                // Payee name
    long long LastUsedCategoryID; // Last category ID used with this payee
    double MatchConfidence;       // Confidence score (0.0 to 100.0)
    wxString matchMethod;   // New field: "Exact", "Regex", "Fuzzy"
};

enum class PayeeMatchMode
{
    BEST_MATCH,  // Return only the most likely match
    LIST_MATCHES // Return a list of likely matches
};

class PayeeMatchAndMerge
{
public:
    PayeeMatchAndMerge();
    ~PayeeMatchAndMerge();

    // Main method to match a payee name
    bool MatchPayee(const wxString& payeeName, PayeeMatchMode mode, std::vector<PayeeMatchResult>& results, int maxResults = 5);

private:
    // Matching strategies
    void ExactMatch(const wxString& payeeName, std::vector<PayeeMatchResult>& results);
    void RegexMatch(const wxString& payeeName, std::vector<PayeeMatchResult>& results);
    void FuzzyMatch(const wxString& payeeName, std::vector<PayeeMatchResult>& results);

    // Helper to calculate Levenshtein distance for fuzzy matching
    int CalculateLevenshteinDistance(const wxString& s1, const wxString& s2);

    // Helper to load regex patterns from a payee
    void LoadRegexPatterns(const Model_Payee::Data& payee, std::vector<wxString>& patterns);

    // Helper to sort and trim results
    void SortAndTrimResults(std::vector<PayeeMatchResult>& results, int maxResults);
};

#endif // PAYEE_MATCH_AND_MERGE_H_