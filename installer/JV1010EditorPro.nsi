; ============================================================================
; Roland JV-1010 Editor Pro — NSIS Windows Installer (64-bit)
; Installs Standalone App to Program Files and VST3 to Common Files\VST3
; ============================================================================

!ifndef APP_VERSION
  !define APP_VERSION "1.0.0"
!endif

!ifndef BUILD_DIR
  !define BUILD_DIR "..\build\JV1010EditorPro_artefacts\Release"
!endif

!define APP_NAME "JV-1010 Editor Pro"
!define PUBLISHER "JV-1010 Editor Pro"
!define UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}"

!include "MUI2.nsh"
!include "x64.nsh"

Name "${APP_NAME} ${APP_VERSION}"
OutFile "..\build\JV-1010-Editor-Pro-${APP_VERSION}-Windows-x64-Setup.exe"
InstallDir "$PROGRAMFILES64\${APP_NAME}"
InstallDirRegKey HKLM "${UNINST_KEY}" "InstallLocation"
RequestExecutionLevel admin
SetCompressor /SOLID lzma
Unicode True

VIProductVersion "${APP_VERSION}.0"
VIAddVersionKey "ProductName" "${APP_NAME}"
VIAddVersionKey "CompanyName" "${PUBLISHER}"
VIAddVersionKey "FileDescription" "Roland JV-1010 Visual MIDI/SysEx Editor & Librarian"
VIAddVersionKey "FileVersion" "${APP_VERSION}"
VIAddVersionKey "ProductVersion" "${APP_VERSION}"
VIAddVersionKey "LegalCopyright" "Open Source"

; Modern UI Pages
!define MUI_ABORTWARNING
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Spanish"

Function .onInit
  ${IfNot} ${RunningX64}
    MessageBox MB_OK|MB_ICONSTOP "This installer requires 64-bit Windows."
    Abort
  ${EndIf}
  SetRegView 64
FunctionEnd

Function un.onInit
  SetRegView 64
FunctionEnd

Section "Standalone Application" SecStandalone
  SectionIn RO
  SetOutPath "$INSTDIR"
  File "${BUILD_DIR}\Standalone\JV-1010 Editor Pro.exe"

  ; Start Menu Shortcuts
  CreateDirectory "$SMPROGRAMS\${APP_NAME}"
  CreateShortcut "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk" "$INSTDIR\JV-1010 Editor Pro.exe"
  CreateShortcut "$SMPROGRAMS\${APP_NAME}\Uninstall ${APP_NAME}.lnk" "$INSTDIR\Uninstall.exe"

  ; Uninstaller & Add/Remove Programs Registration
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  WriteRegStr HKLM "${UNINST_KEY}" "DisplayName" "${APP_NAME}"
  WriteRegStr HKLM "${UNINST_KEY}" "DisplayVersion" "${APP_VERSION}"
  WriteRegStr HKLM "${UNINST_KEY}" "Publisher" "${PUBLISHER}"
  WriteRegStr HKLM "${UNINST_KEY}" "InstallLocation" "$INSTDIR"
  WriteRegStr HKLM "${UNINST_KEY}" "UninstallString" "$\"$INSTDIR\Uninstall.exe$\""
  WriteRegDWORD HKLM "${UNINST_KEY}" "NoModify" 1
  WriteRegDWORD HKLM "${UNINST_KEY}" "NoRepair" 1
SectionEnd

Section "VST3 Plugin (64-bit)" SecVST3
  SetOutPath "$COMMONFILES64\VST3\JV-1010 Editor Pro.vst3"
  File /r "${BUILD_DIR}\VST3\JV-1010 Editor Pro.vst3\*"
SectionEnd

Section "Desktop Shortcut" SecDesktop
  CreateShortcut "$DESKTOP\${APP_NAME}.lnk" "$INSTDIR\JV-1010 Editor Pro.exe"
SectionEnd

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecStandalone} "Installs the standalone ${APP_NAME} application into Program Files."
  !insertmacro MUI_DESCRIPTION_TEXT ${SecVST3} "Installs the 64-bit VST3 plugin into the canonical Windows VST3 folder (C:\Program Files\Common Files\VST3)."
  !insertmacro MUI_DESCRIPTION_TEXT ${SecDesktop} "Creates a shortcut on your Desktop."
!insertmacro MUI_FUNCTION_DESCRIPTION_END

Section "Uninstall"
  ; Remove Standalone & Shortcuts
  Delete "$INSTDIR\JV-1010 Editor Pro.exe"
  Delete "$INSTDIR\Uninstall.exe"
  RMDir "$INSTDIR"

  Delete "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk"
  Delete "$SMPROGRAMS\${APP_NAME}\Uninstall ${APP_NAME}.lnk"
  RMDir "$SMPROGRAMS\${APP_NAME}"
  Delete "$DESKTOP\${APP_NAME}.lnk"

  ; Remove VST3 Bundle
  RMDir /r "$COMMONFILES64\VST3\JV-1010 Editor Pro.vst3"

  ; Remove Registry Key
  DeleteRegKey HKLM "${UNINST_KEY}"
SectionEnd
