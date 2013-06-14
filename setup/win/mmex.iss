; Money Manager Ex InnoSetup script

; This file encoded as UTF-8 with BOM.
; UTF-8 enables custom messages in such languages as Chinese, Japanese, etc.

; Copyright (C) 2006 Madhan Kanagavel
; Copyright (C) 2009 VaDiM
; Copyright (C) 2011 Stefano Giorgio

#ifndef AppExePath
        #error Use option -dAppExePath=path_to_your_exe
#endif

#ifndef BuildOpts
        #error Use option -dBuildOpts=value
#endif

#ifndef CpuArch
        #error Use option -dCpuArch=[x86|amd64|ia64]
#endif

#ifndef CrtDlls
        #error Use option -dCrtDlls=value
#endif

#ifndef Format
        #error Use option -dFormat=bakefile_format_var
#endif


#define AppURL "http://www.codelathe.com/mmex"
#define AppExeName ExtractFileName( AppExePath )
#define OutputDir  ExtractFilePath( AppExePath )


; information from .exe GetVersionInfo

#define AppName GetStringFileInfo( AppExePath, PRODUCT_NAME )
#define VersionInfo GetFileVersion( AppExePath )
#define Copyright GetFileCopyright( AppExePath )
#define Company GetFileCompany( AppExePath )

#define OutputBaseFilename GetStringFileInfo( AppExePath, INTERNAL_NAME ) + '-' + VersionInfo + BuildOpts + '-setup'


[Setup]
AppName={#AppName}
AppVerName={#AppName} {#VersionInfo}
AppPublisher={#Company}
AppPublisherURL={#AppURL}
AppSupportURL={#AppURL}
AppUpdatesURL={#AppURL}
DefaultDirName={pf}\{#AppName}
DefaultGroupName={#AppName}
DisableProgramGroupPage=yes
LicenseFile="..\..\doc\license.txt"
InfoBeforeFile="..\..\README.TXT"
InfoAfterFile="..\..\doc\version.txt"

OutputDir={#OutputDir}
OutputBaseFilename={#OutputBaseFilename}
OutputManifestFile="{#OutputBaseFilename}.manifest"
Compression=lzma/ultra
SolidCompression=yes

SetupIconFile="..\..\resources\mmex.ico"
;WizardImageFile="..\..\graphics\new.bmp"
;WizardImageBackColor=clWhite
;WizardImageStretch=no

AppCopyright={#Copyright}
AppId=37153D93-6D91-4763-82BB-0DF646211ED0

VersionInfoVersion={#VersionInfo}
VersionInfoCompany={#Company}
VersionInfoDescription={#GetFileDescription( AppExePath )}
VersionInfoCopyright={#Copyright}

; disable the "Select Setup Language" dialog and have it rely solely on auto-detection
;ShowLanguageDialog=no


; "ArchitecturesAllowed=x64" specifies that Setup cannot run on anything but x64
#if CpuArch == "x86"
  ArchitecturesAllowed=x86 x64
#elif CpuArch == "amd64"
  ArchitecturesAllowed=x64
#else
  ArchitecturesAllowed={#CpuArch}
#endif

; "ArchitecturesInstallIn64BitMode=x64 ia64" requests that the install
; be done in "64-bit mode" on x64 & Itanium, meaning it should use the
; native 64-bit Program Files directory and the 64-bit view of the
; registry. On all other architectures it will install in "32-bit mode".
ArchitecturesInstallIn64BitMode=x64 ia64


[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"
Name: "ru"; MessagesFile: "compiler:Languages\Russian.isl"; InfoBeforeFile: "..\..\README.RU"
Name: "fr"; MessagesFile: "compiler:Languages\French.isl"
Name: "de"; MessagesFile: "compiler:Languages\German.isl"
Name: "es"; MessagesFile: "compiler:Languages\Spanish.isl"
Name: "he"; MessagesFile: "compiler:Languages\Hebrew.isl"
Name: "it"; MessagesFile: "compiler:Languages\Italian.isl"
Name: "pt"; MessagesFile: "compiler:Languages\Portuguese.isl"
Name: "no"; MessagesFile: "compiler:Languages\Norwegian.isl"
Name: "eu"; MessagesFile: "compiler:Languages\Basque.isl"
Name: "pt_br"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"
Name: "ca"; MessagesFile: "compiler:Languages\Catalan.isl"
Name: "cs"; MessagesFile: "compiler:Languages\Czech.isl"
Name: "da"; MessagesFile: "compiler:Languages\Danish.isl"
Name: "nl"; MessagesFile: "compiler:Languages\Dutch.isl"
Name: "fi"; MessagesFile: "compiler:Languages\Finnish.isl"
Name: "hu"; MessagesFile: "compiler:Languages\Hungarian.isl"
Name: "pl"; MessagesFile: "compiler:Languages\Polish.isl"
Name: "sk"; MessagesFile: "compiler:Languages\Slovak.isl"
Name: "sl"; MessagesFile: "compiler:Languages\Slovenian.isl"


[Types]
Name: "full"; Description: "Full installation"
Name: "minimal"; Description: "Minimal installation"
Name: "custom"; Description: "Custom installation"; Flags: iscustom

[Components]
Name: "program"; Description: "Program Files"; Types: full minimal custom; Flags: fixed
Name: "help";    Description: "Help files";    Types: full minimal

; Add language component here then add language file in files section
Name: "lang";                     Description: "Languages"; Types: full
Name: "lang\en";                  Description: "English"; Types: full
Name: "lang\arabic";              Description: "Arabic"; Types: full
Name: "lang\bulgarian";           Description: "Bulgarian"; Types: full
Name: "lang\chinese_chs";         Description: "Chinese Chs"; Types: full
Name: "lang\chinese_zh";          Description: "Chinese Zh"; Types: full
Name: "lang\croatian";            Description: "Croatian"; Types: full
Name: "lang\cs";                  Description: "Czech"; Types: full
Name: "lang\nl";                  Description: "Dutch"; Types: full
Name: "lang\dutch_be";            Description: "Dutch Be"; Types: full
Name: "lang\english_uk";          Description: "English UK"; Types: full
Name: "lang\fr";                  Description: "French"; Types: full
Name: "lang\de";                  Description: "German"; Types: full
Name: "lang\greek";               Description: "Greek"; Types: full
Name: "lang\he";                  Description: "Hebrew"; Types: full
Name: "lang\hu";                  Description: "Hungarian"; Types: full
Name: "lang\indonesian";          Description: "Indonesian"; Types: full
Name: "lang\it";                  Description: "Italian"; Types: full
Name: "lang\latvian";             Description: "Latvian"; Types: full
Name: "lang\no";                  Description: "Norwegian"; Types: full
Name: "lang\pl";                  Description: "Polish"; Types: full
Name: "lang\pt";                  Description: "Portuguese"; Types: full
Name: "lang\portuguese_portugal"; Description: "Portuguese Portugal"; Types: full
Name: "lang\romanian";            Description: "Romanian"; Types: full
Name: "lang\ru";                  Description: "Russian"; Types: full
Name: "lang\serbo_croatian";      Description: "Serbo Croatian"; Types: full
Name: "lang\serbian";             Description: "Serbian"; Types: full
Name: "lang\sk";                  Description: "Slovak"; Types: full
Name: "lang\sl";                  Description: "Slovenian"; Types: full
Name: "lang\es";                  Description: "Spanish"; Types: full
Name: "lang\swedish";             Description: "Swedish"; Types: full
Name: "lang\tamil";               Description: "Tamil"; Types: full
Name: "lang\turkish";             Description: "Turkish"; Types: full
Name: "lang\ukrainian";           Description: "Ukrainian"; Types: full
Name: "lang\vietnamese";          Description: "Vietnamese"; Types: full


[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked


[Files]
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

#define lang_src "..\..\po"
#define lang_dst "{app}\po"
; This is where the language files are added
Source: "{#lang_src}\english.*";             DestDir: {#lang_dst}; Components: "lang\en";                  Flags: ignoreversion
Source: "{#lang_src}\arabic.*";              DestDir: {#lang_dst}; Components: "lang\arabic";              Flags: ignoreversion
Source: "{#lang_src}\bulgarian.*";           DestDir: {#lang_dst}; Components: "lang\bulgarian";           Flags: ignoreversion
Source: "{#lang_src}\chinese_chs.*";         DestDir: {#lang_dst}; Components: "lang\chinese_chs";         Flags: ignoreversion
Source: "{#lang_src}\chinese_zh.*";          DestDir: {#lang_dst}; Components: "lang\chinese_zh";          Flags: ignoreversion
Source: "{#lang_src}\croatian.*";            DestDir: {#lang_dst}; Components: "lang\croatian";            Flags: ignoreversion
Source: "{#lang_src}\czech.*";               DestDir: {#lang_dst}; Components: "lang\cs";                  Flags: ignoreversion
Source: "{#lang_src}\dutch.*";               DestDir: {#lang_dst}; Components: "lang\nl";                  Flags: ignoreversion
Source: "{#lang_src}\dutch_be.*";            DestDir: {#lang_dst}; Components: "lang\dutch_be";            Flags: ignoreversion
Source: "{#lang_src}\english-uk.*";          DestDir: {#lang_dst}; Components: "lang\english_uk";          Flags: ignoreversion
Source: "{#lang_src}\french.*";              DestDir: {#lang_dst}; Components: "lang\fr";                  Flags: ignoreversion
Source: "{#lang_src}\german.*";              DestDir: {#lang_dst}; Components: "lang\de";                  Flags: ignoreversion
Source: "{#lang_src}\greek.*";               DestDir: {#lang_dst}; Components: "lang\greek";               Flags: ignoreversion
Source: "{#lang_src}\hebrew.*";              DestDir: {#lang_dst}; Components: "lang\he";                  Flags: ignoreversion
Source: "{#lang_src}\hungarian.*";           DestDir: {#lang_dst}; Components: "lang\hu";                  Flags: ignoreversion
Source: "{#lang_src}\indonesian.*";          DestDir: {#lang_dst}; Components: "lang\indonesian";          Flags: ignoreversion
Source: "{#lang_src}\italian.*";             DestDir: {#lang_dst}; Components: "lang\it";                  Flags: ignoreversion
Source: "{#lang_src}\latvian.*";             DestDir: {#lang_dst}; Components: "lang\latvian";             Flags: ignoreversion
Source: "{#lang_src}\norwegian.*";           DestDir: {#lang_dst}; Components: "lang\no";                  Flags: ignoreversion
Source: "{#lang_src}\polish.*";              DestDir: {#lang_dst}; Components: "lang\pl";                  Flags: ignoreversion
Source: "{#lang_src}\portuguese.*";          DestDir: {#lang_dst}; Components: "lang\pt";                  Flags: ignoreversion
Source: "{#lang_src}\portuguese_portugal.*"; DestDir: {#lang_dst}; Components: "lang\portuguese_portugal"; Flags: ignoreversion
Source: "{#lang_src}\romanian.*";            DestDir: {#lang_dst}; Components: "lang\romanian";            Flags: ignoreversion
Source: "{#lang_src}\russian.*";             DestDir: {#lang_dst}; Components: "lang\ru";                  Flags: ignoreversion
Source: "{#lang_src}\serbian.*";             DestDir: {#lang_dst}; Components: "lang\serbian";             Flags: ignoreversion
Source: "{#lang_src}\serbo-croatian.*";      DestDir: {#lang_dst}; Components: "lang\serbo_croatian";      Flags: ignoreversion
Source: "{#lang_src}\slovak.*";              DestDir: {#lang_dst}; Components: "lang\sk";                  Flags: ignoreversion
Source: "{#lang_src}\slovenian.*";           DestDir: {#lang_dst}; Components: "lang\sl";                  Flags: ignoreversion
Source: "{#lang_src}\spanish.*";             DestDir: {#lang_dst}; Components: "lang\es";                  Flags: ignoreversion
Source: "{#lang_src}\swedish.*";             DestDir: {#lang_dst}; Components: "lang\swedish";             Flags: ignoreversion
Source: "{#lang_src}\tamil.*";               DestDir: {#lang_dst}; Components: "lang\tamil";               Flags: ignoreversion
Source: "{#lang_src}\turkish.*";             DestDir: {#lang_dst}; Components: "lang\turkish";             Flags: ignoreversion
Source: "{#lang_src}\ukrainian.*";           DestDir: {#lang_dst}; Components: "lang\ukrainian";           Flags: ignoreversion
Source: "{#lang_src}\vietnamese.*";          DestDir: {#lang_dst}; Components: "lang\vietnamese";          Flags: ignoreversion

#define help_src "..\..\doc\help"
#define help_dir "{app}\help"
; Add any common help files to help root directory
;Source: "{#help_src}\*.jpg"; DestDir: {#help_dir}; Components: help; Flags: ignoreversion
Source: "{#help_src}\*.png"; DestDir: {#help_dir}; Components: help; Flags: ignoreversion

; Add the main index file for the appropriate language to help root directory
Source: "{#help_src}\index.html";         Languages: en; DestDir: {#help_dir}; Components: help; Flags: ignoreversion
Source: "{#help_src}\german\index.html";  Languages: de; DestDir: {#help_dir}; Components: help; Flags: ignoreversion
Source: "{#help_src}\french\index.html";  Languages: fr; DestDir: {#help_dir}; Components: help; Flags: ignoreversion
Source: "{#help_src}\italian\index.html"; Languages: it; DestDir: {#help_dir}; Components: help; Flags: ignoreversion
Source: "{#help_src}\polish\index.html";  Languages: pl; DestDir: {#help_dir}; Components: help; Flags: ignoreversion
Source: "{#help_src}\russian\index.html"; Languages: ru; DestDir: {#help_dir}; Components: help; Flags: ignoreversion
Source: "{#help_src}\spanish\index.html"; Languages: es; DestDir: {#help_dir}; Components: help; Flags: ignoreversion

; Add the directory containing the files for the specific language
Source: "{#help_src}\english\*"; Languages: de; DestDir: "{#help_dir}\german"; Components: help; Flags: ignoreversion
Source: "{#help_src}\french\*";  Languages: fr; DestDir: "{#help_dir}\french";  Components: help; Flags: ignoreversion
Source: "{#help_src}\italian\*"; Languages: it; DestDir: "{#help_dir}\italian"; Components: help; Flags: ignoreversion
Source: "{#help_src}\polish\*";  Languages: pl; DestDir: "{#help_dir}\polish";  Components: help; Flags: ignoreversion
Source: "{#help_src}\russian\*"; Languages: ru; DestDir: "{#help_dir}\russian"; Components: help; Flags: ignoreversion
Source: "{#help_src}\spanish\*"; Languages: es; DestDir: "{#help_dir}\spanish"; Components: help; Flags: ignoreversion

; Add files to mmex root directory
Source: "..\..\README.TXT";      Languages: en; DestDir: "{app}"; DestName: "readme.txt"; Components: program; Flags: ignoreversion
Source: "..\..\README.RU";       Languages: ru; DestDir: "{app}"; DestName: "readme.txt"; Components: program; Flags: ignoreversion
Source: "..\..\doc\contrib.txt"; DestDir: "{app}"; Components: program; Flags: ignoreversion
Source: "..\..\doc\license.txt"; DestDir: "{app}"; Components: program; Flags: ignoreversion
Source: "..\..\doc\version.txt"; DestDir: "{app}"; Components: program; Flags: ignoreversion

#define res_dir "{app}\res"
Source: "..\..\resources\kaching.wav";       DestDir: {#res_dir}; Components: program; Flags: ignoreversion
Source: "..\..\resources\mmex.ico";          DestDir: {#res_dir}; Components: program; Flags: ignoreversion
Source: "..\..\resources\splash.png";        DestDir: {#res_dir}; Components: program; Flags: ignoreversion
Source: "..\..\resources\currency_seed.csv"; DestDir: {#res_dir}; Components: program; Flags: ignoreversion

#define bin_dir "{app}\bin"
; .exe already compressed by UPX
Source: {#AppExePath}; DestDir: {#bin_dir}; Components: program; Flags: ignoreversion nocompression
Source: {#CrtDlls};    DestDir: {#bin_dir}; Components: program; Flags: ignoreversion

#if Format == "mingw"
  #define MingwDllPath  GetEnv("MINGWDIR") + "\bin\mingwm10.dll"
  Source: "{#MingwDllPath}"; DestDir: {#bin_dir}; Components: program; Flags: ignoreversion
#endif


[Icons]
Name: "{group}\{#AppName}"; Filename: "{#bin_dir}\{#AppExeName}"; IconFilename: "{#bin_dir}\{#AppExeName}"; WorkingDir: {#bin_dir}; IconIndex: 0
Name: "{group}\{cm:UninstallProgram,{#AppName}}"; Filename: "{uninstallexe}"
; Name: "{commondesktop}\{#AppName}"; Filename: "{app}\{#AppExeName}"; Tasks: desktopicon
Name: "{userdesktop}\{#AppName}"; Filename: "{#bin_dir}\{#AppExeName}"; Tasks: desktopicon; IconFilename: "{#bin_dir}\{#AppExeName}"; WorkingDir: {#bin_dir}; IconIndex: 0
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\{#AppName}"; Filename: "{#bin_dir}\{#AppExeName}"; Tasks: quicklaunchicon; WorkingDir: {#bin_dir}; IconFilename: "{#bin_dir}\{#AppExeName}"; IconIndex: 0

[Run]
Filename: "{#bin_dir}\{#AppExeName}"; Description: "{cm:LaunchProgram,{#AppName}}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: files; Name: "{userappdata}\{#AppName}\mmexini.db3"
Type: dirifempty; Name: "{userappdata}\{#AppName}"

[Messages]
en.WelcomeLabel1=Welcome to [name] Setup
ru.WelcomeLabel1=Вас приветствует инсталлятор [name]
