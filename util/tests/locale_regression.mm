// Exercise the production locale workaround and its formatting assumptions.
#import <Cocoa/Cocoa.h>
#include <wx/init.h>
#include <wx/intl.h>
#include <wx/numformatter.h>
#include <wx/platinfo.h>
#include <fmt/format.h>
#include <clocale>
#include <cstdio>
#include <cstring>
#include <locale>
#include <string>
#include <vector>

#include "base/mmPlatform.h"

namespace
{
bool check(bool condition, const char* description)
{
    if (!condition)
        std::fprintf(stderr, "FAIL: %s\n", description);
    return condition;
}

bool checkAlert(size_t button)
{
    NSAlert* alert = [[NSAlert alloc] init];
    alert.alertStyle = NSAlertStyleWarning;
    alert.messageText = @"MMEX locale regression check";
    alert.informativeText = @"Synthetic confirmation; no database is opened.";
    [alert addButtonWithTitle:@"No"];
    [alert addButtonWithTitle:@"Yes"];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_SEC),
        dispatch_get_main_queue(), ^{
            [[alert.buttons objectAtIndex:button] performClick:nil];
        });
    const NSModalResponse result = [alert runModal];
    [alert release];
    return check(result == NSAlertFirstButtonReturn + static_cast<NSModalResponse>(button),
        "native confirmation returns the selected button");
}

bool checkLocale(wxLanguage language, const char* cppLocale, bool alerts)
{
    wxLocale locale(language);
    if (!check(locale.IsOk(), "requested wxLocale is available"))
        return false;

    // Capture the old stock-table representation while LC_NUMERIC still
    // follows the user's locale, then compare it with explicit UI formatting.
    const std::vector<std::pair<double, int>> values = {
        {1234.5, 2}, {0.125, 4}, {-12.34, 2}, {1234.0, 0}
    };
    std::vector<wxString> before;
    for (const auto& value : values)
        before.push_back(wxString::FromDouble(value.first, value.second));

    const std::string numeric = std::setlocale(LC_NUMERIC, nullptr);
    const std::string dates = std::setlocale(LC_TIME, nullptr);
    const std::string characters = std::setlocale(LC_CTYPE, nullptr);
    const std::string money = std::setlocale(LC_MONETARY, nullptr);
    const auto currencyBefore = fmt::format(std::locale(cppLocale), "{:.2Lf}", 1234.56);

    mmPlatform::initNumericLocale();

    const auto& platform = wxPlatformInfo::Get();
    const bool affected = platform.GetOSMajorVersion() == 27 && platform.GetOSMinorVersion() < 2;
    bool ok = check(std::string(std::setlocale(LC_NUMERIC, nullptr)) == (affected ? "C" : numeric),
        "numeric locale is reset only on affected OS versions");
    ok &= check(dates == std::setlocale(LC_TIME, nullptr), "date locale retained");
    ok &= check(characters == std::setlocale(LC_CTYPE, nullptr), "character locale retained");
    ok &= check(money == std::setlocale(LC_MONETARY, nullptr), "monetary locale retained");
    for (size_t i = 0; i < values.size(); ++i) {
        ok &= check(before[i] == wxNumberFormatter::ToString(values[i].first,
            values[i].second, wxNumberFormatter::Style_None), "stock number formatting retained");
    }
    ok &= check(currencyBefore == fmt::format(std::locale(cppLocale), "{:.2Lf}", 1234.56),
        "explicit currency formatting retained");
    double parsed = 0;
    ok &= check(wxString("1234.56").ToCDouble(&parsed) && parsed == 1234.56 &&
        wxString::FromCDouble(parsed, 2) == "1234.56", "storage number round trip");

    std::printf("%s: LC_NUMERIC=%s, stock=%s, currency=%s, %s\n", cppLocale,
        std::setlocale(LC_NUMERIC, nullptr), before[0].utf8_str().data(),
        currencyBefore.c_str(), ok ? "PASS" : "FAIL");
    if (alerts && ok) {
        ok &= checkAlert(0);
        ok &= checkAlert(1);
    }
    return ok;
}
}

int main(int argc, char** argv)
{
    if (argc > 2 || (argc == 2 && std::strcmp(argv[1], "--alerts") != 0)) {
        std::fprintf(stderr, "Usage: %s [--alerts]\n", argv[0]);
        return 2;
    }
    @autoreleasepool {
        wxInitializer init;
        if (!init)
            return 2;
        bool ok = checkLocale(wxLANGUAGE_GERMAN, "de_DE.UTF-8", argc == 2);
        ok &= checkLocale(wxLANGUAGE_FRENCH, "fr_FR.UTF-8", argc == 2);
        ok &= checkLocale(wxLANGUAGE_ENGLISH_US, "en_US.UTF-8", argc == 2);
        return ok ? 0 : 1;
    }
}
