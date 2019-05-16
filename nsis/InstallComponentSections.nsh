;
; Install components sections for lifutils
;
Section /o "Modify Path variable" SectionModifyPath
        SectionIn 3

        !insertmacro ADD_PATH $INSTDIR $MultiUser.InstallMode
        StrCpy $PathModified "1"
        !insertmacro DeleteRetryAbort "$SMPROGRAMS\$StartMenuFolder\${PRODUCT_NAME} Prompt.lnk"
SectionEnd

Section /o "Add LIFUTILSXROMDIR to environment" SectionAddEnvironmentVar
        SectionIn 3
        !insertmacro SET_ENVIRONMENT "LIFUTILSXROMDIR" $INSTDIR\xroms $MultiUser.InstallMode
SectionEnd

