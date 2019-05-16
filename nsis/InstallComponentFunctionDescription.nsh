;
; Install components function description for lifutils
;
        !insertmacro MUI_DESCRIPTION_TEXT ${SectionModifyPath} "Modify PATH variable for ${PRODUCT_NAME}."
        !insertmacro MUI_DESCRIPTION_TEXT ${SectionAddEnvironmentVar} "Add patho to xroms subdirectory to environment variable LIFUTILSXROMDIR."
