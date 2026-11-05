; LibreMines Windows installer

#ifndef VersionFile
  #error VersionFile must point to the generated libremines-version.iss file.
#endif

#include VersionFile

#ifndef QtMajor
  #define QtMajor "6"
#endif

#if QtMajor == "5"
  #define AppId "{30B9A806-3F85-4658-99D1-C77C1B5AFC50}"
#else
  #define AppId "{EA05617B-4653-4CC9-AA4B-77A10FA04904}"
#endif

#ifndef SourceDir
  #error SourceDir must point to the staged Windows application files.
#endif

#ifndef InstallerOutputDir
  #error InstallerOutputDir must point to the installer output directory.
#endif

[Setup]
AppId={{#AppId}
AppName=LibreMines (Qt {#QtMajor})
AppVersion={#AppVersion}
AppPublisher=Bruno Bollos Correa
AppCopyright=Copyright (C) 2020-2026 Bruno Bollos Correa
AppComments=Free/Libre and Open Source Minesweeper game.
AppPublisherURL=https://github.com/Bollos00/LibreMines
AppSupportURL=https://github.com/Bollos00/LibreMines/issues
AppUpdatesURL=https://github.com/Bollos00/LibreMines/releases
UninstallDisplayName=LibreMines (Qt {#QtMajor})
VersionInfoCompany=Bruno Bollos Correa
VersionInfoCopyright=Copyright (C) 2020-2026 Bruno Bollos Correa
VersionInfoDescription=LibreMines (Qt {#QtMajor}) Windows Installer
VersionInfoProductName=LibreMines (Qt {#QtMajor})
VersionInfoProductVersion={#AppVersion}
VersionInfoVersion={#VersionInfoVersion}
DefaultDirName={autopf}\LibreMines Qt {#QtMajor}
DefaultGroupName=LibreMines (Qt {#QtMajor})
DisableProgramGroupPage=yes
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
OutputDir={#InstallerOutputDir}
OutputBaseFilename=libremines-{#ReleaseVersion}-windows-qt{#QtMajor}-setup
SetupIconFile={#SourceDir}/libremines.ico
LicenseFile={#SourceDir}\LICENSE
UninstallDisplayIcon={app}\libremines.exe
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
WizardStyle=modern
CloseApplications=yes
Compression=lzma2
SolidCompression=yes

[Tasks]
Name: "desktopicon"; Description: "Create a &desktop shortcut"; GroupDescription: "Additional shortcuts:"; Flags: unchecked

[Files]
Source: "{#SourceDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{autoprograms}\LibreMines (Qt {#QtMajor})"; Filename: "{app}\libremines.exe"
Name: "{autodesktop}\LibreMines (Qt {#QtMajor})"; Filename: "{app}\libremines.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\libremines.exe"; Description: "Launch LibreMines (Qt {#QtMajor})"; Flags: postinstall nowait skipifsilent
