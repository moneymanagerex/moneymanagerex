// Minimal reproduction of wxWidgets #26977 / MMEX #8536, without wxWidgets.
// See macos-locale.md for the intentionally crashing --reproduce mode.
#import <Cocoa/Cocoa.h>
#include <clocale>
#include <cstdio>
#include <cstring>

int main(int argc, char** argv)
{
    if (argc < 2 || argc > 3 ||
        (std::strcmp(argv[1], "--reproduce") != 0 &&
         std::strcmp(argv[1], "--c-numeric") != 0)) {
        std::fprintf(stderr, "Usage: %s --reproduce|--c-numeric [de_DE.UTF-8]\n", argv[0]);
        return 2;
    }

    @autoreleasepool {
        [NSApplication sharedApplication];
        const char* locale = argc == 3 ? argv[2] : "de_DE.UTF-8";
        if (!std::setlocale(LC_ALL, locale)) {
            std::fprintf(stderr, "Locale unavailable: %s\n", locale);
            return 2;
        }
        if (std::strcmp(argv[1], "--c-numeric") == 0)
            std::setlocale(LC_NUMERIC, "C");

        std::fprintf(stderr, "LC_NUMERIC=%s; decimal=%s\n",
            std::setlocale(LC_NUMERIC, nullptr), std::localeconv()->decimal_point);
        NSAlert* alert = [[NSAlert alloc] init];
        alert.alertStyle = NSAlertStyleWarning;
        alert.messageText = @"Native alert locale probe";
        alert.informativeText = @"Synthetic alert; closes automatically if rendering succeeds.";
        [alert addButtonWithTitle:@"No"];
        [alert addButtonWithTitle:@"Yes"];
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_SEC),
            dispatch_get_main_queue(), ^{
                [[alert.buttons objectAtIndex:0] performClick:nil];
            });
        const NSModalResponse result = [alert runModal];
        [alert release];
        std::fprintf(stderr, "Alert returned %ld\n", static_cast<long>(result));
        return result == NSAlertFirstButtonReturn ? 0 : 1;
    }
}
