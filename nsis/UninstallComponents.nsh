;
; Uninstall components for lifutils
;
        ; Remove path
        ${if} "$PathModified" == "1"
                !insertmacro REMOVE_PATH $INSTDIR $MultiUser.InstallMode
        ${endif}
         ; Remove environment varialbe LIFUTILSXEMOMDIR
        !insertmacro REMOVE_ENVIRONMENT "LIFUTILSXROMDIR" $MultiUser.InstallMode
