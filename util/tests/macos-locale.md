# macOS native-alert locale regression

## Problem and scope

MMEX issue [#8536](https://github.com/moneymanagerex/moneymanagerex/issues/8536)
includes startup crashes and a reproducible crash opening a transaction's
deletion confirmation. The stack reaches `-[NSAlert runModal]`, CoreUI symbol
rendering, and `+[NSApplication _crashOnException:]`. The deletion handler has
not yet reached its database mutation when the confirmation crashes.

[wxWidgets #26977](https://github.com/wxWidgets/wxWidgets/issues/26977) reproduces
the same failure in Cocoa without wxWidgets. Setting a comma-decimal C numeric
locale is sufficient. The standalone probe here reproduces the failure on
macOS 27.0.1 (26A434), Apple Silicon, with `de_DE.UTF-8`.

`mmApp` constructs `wxLocale(wxLANGUAGE_DEFAULT)`, which sets `LC_ALL` from the
system language. Its separate `wxTranslations` selection controls the MMEX GUI
language. Changing that translation preference therefore need not change
`LC_NUMERIC`; an environment-only workaround can also be overwritten by the
locale constructor.

`mmPlatform::initNumericLocale()` is called immediately after this constructor,
before wxWidgets initializes Cocoa. It resets only `LC_NUMERIC` on macOS 27.0
and 27.1. The early placement also precedes AppKit's window-restoration alert,
which can appear before `OnInit()`. Resetting the locale only in MMEX's deletion
handler would miss startup and other native alerts.

The upper version boundary follows the upstream
[confirmation of Apple's fix in 27.2 beta 2](https://github.com/wxWidgets/wxWidgets/issues/26977#issuecomment-5776801927).
It is not a claim of local testing on 27.1 or 27.2. Earlier 27.2 betas are outside
this guard. Older macOS, 27.2+, and other platforms retain their previous locale
initialization. Re-evaluate the boundary if Apple changes the fix before release.

## Formatting and limitations

The change preserves `LC_CTYPE`, `LC_TIME`, `LC_MONETARY`, translations and the UI
locale. MMEX currency formatting uses explicit currency separators or an
explicit C++ locale, and numeric storage conversions use `ToCDouble` and
`FromCDouble`.

The stock transaction list previously used `wxString::FromDouble`, which follows
the C numeric locale. It now uses `wxNumberFormatter` with `Style_None` to retain
the UI decimal separator, precision, trailing zeros and lack of digit grouping.
The remaining transaction-number conversion is restricted to digit-only strings
and formats with zero decimal places. OFX import's direct numeric conversions
are also relevant to review; this patch does not refactor the importer.

This is a bounded compatibility workaround, not a general separation of C and
UI locales throughout MMEX. A later `setlocale` call can undo it, including a
custom Lua report using `os.setlocale`. It does not intercept such calls or
suppress Cocoa exceptions. A longer-term migration from `wxLocale` to
`wxUILocale` would require a separate audit of formatting, parsing, supported
wxWidgets versions and locale-dependent report scripts.

## Build the diagnostics

These small executables do not open a database or use MMEX preferences. They
require macOS, CMake 3.16+, the initialized fmt submodule, and wxWidgets 3.1.6+
(validated with the project's wxWidgets 3.3.3). From the repository root:

```sh
cmake -S util/tests -B build/locale-checks \
  -DwxWidgets_CONFIG_EXECUTABLE=/absolute/path/to/wx-config \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build/locale-checks --parallel 4
ctest --test-dir build/locale-checks --output-on-failure
```

The formatting test links the production `mmPlatform.cpp` implementation. For
German, French and US English it checks the OS-dependent numeric locale,
retention of the other locale categories, stock-style formatting before and
after the change, explicit currency formatting and a storage-number round trip.
The checks use return values, so they remain active in Release builds.

In a graphical login session, also run:

```sh
build/locale-checks/locale-regression --alerts
build/locale-checks/native-alert-probe --c-numeric
build/locale-checks/native-alert-probe --reproduce
```

`--alerts` exercises both button results of a native confirmation in all three
languages. Each synthetic alert closes automatically. The standalone probe
links only Cocoa; `--reproduce` deliberately leaves the German numeric locale
active and is expected to terminate with SIGTRAP on affected systems.
`--c-numeric` shows the same alert with the C numeric locale and should return
zero. On a fixed OS, both standalone modes may succeed. An optional final
argument selects another installed C locale, for example `fr_FR.UTF-8`.

To build just the Cocoa reproduction without wxWidgets or CMake:

```sh
clang++ -std=c++17 -framework Cocoa util/tests/native_alert_probe.mm \
  -o /tmp/mmex-native-alert-probe
/tmp/mmex-native-alert-probe --reproduce
/tmp/mmex-native-alert-probe --c-numeric
```

## Application acceptance checks

Use a disposable database and separate settings (`mmex -i /path/to/mmexini.db3
/path/to/test.mmb`; the settings file must already exist). In a comma-decimal
system locale, check startup, canceling and confirming deletion, editing a
decimal amount, stock transaction quantities/prices/commission/total, and a
clean restart. Verify that cancellation leaves the transaction unchanged and
that confirmation performs only the requested deletion. Check the test
database with `PRAGMA integrity_check` afterward.

Use both a database-selected locale and currency-defined separators for amount
entry. A successful startup alone does not exercise the failing native alert.
Manual application checks supplement the isolated diagnostics; the diagnostics
do not establish correctness of every import, report or financial workflow.
