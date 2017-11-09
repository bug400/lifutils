;--------------------------------------------------------------------
; Path functions
;
; Based on example from:
; http://nsis.sourceforge.net/Path_Manipulation
;
; AddToPath - Appends dir to PATH
;   (does not work on Win9x/ME)
;
; Usage:
;   Push "dir"
;   Push "AllUsers" or "CurrentUser"
;   Call AddToPath

Function AddToPath
  Exch $0  ; $0 has mode
  Exch
  Exch $1  ; $1 has directory
  Push $2
  Push $3
  Push $4
  Push $5

  ; NSIS ReadRegStr returns empty string on string overflow
  ; Native calls are used here to check actual length of PATH

  ; $5 = RegOpenKey(HKEY_CURRENT_USER, "Environment", &$4)
  Strcmp $0 "AllUsers" 0 +2
     System::Call "advapi32::RegOpenKey(i 0x80000002, t'SYSTEM\CurrentControlSet\Control\Session Manager\Environment', *i.r4) i.r5"
  Strcmp $0 "CurrentUser" 0 +2
     System::Call "advapi32::RegOpenKey(i 0x80000001, t'Environment', *i.r4) i.r5"
  IntCmp $5 0 0 done done
  ; $5 = RegQueryValueEx($4, "PATH", (DWORD*)0, (DWORD*)0, &$2, ($3=NSIS_MAX_STRLEN, &$3))
  ; RegCloseKey($4)
  System::Call "advapi32::RegQueryValueEx(i $4, t'PATH', i 0, i 0, t.r2, *i ${NSIS_MAX_STRLEN} r3) i.r5"
  System::Call "advapi32::RegCloseKey(i $4)"

  IntCmp $5 234 0 +4 +4 ; $5 == ERROR_MORE_DATA
    DetailPrint "AddToPath: original length $3 > ${NSIS_MAX_STRLEN}"
    MessageBox MB_OK "PATH not updated, original length $3 > ${NSIS_MAX_STRLEN}"
    Goto done

  IntCmp $5 0 +5 ; $5 != NO_ERROR
    IntCmp $5 2 +3 ; $5 != ERROR_FILE_NOT_FOUND
      DetailPrint "AddToPath: unexpected error code $5"
      Goto done
    StrCpy $2 ""

  ; Check if already in PATH
  Push "$2;"
  Push "$1;"
  Call StrStr
  Pop $3
  StrCmp $3 "" 0 done
  Push "$2;"
  Push "$1\;"
  Call StrStr
  Pop $3
  StrCmp $3 "" 0 done

  ; Prevent NSIS string overflow
  StrLen $3 $1
  StrLen $4 $2
  IntOp $3 $3 + $4
  IntOp $3 $3 + 2 ; $3 = strlen(dir) + strlen(PATH) + sizeof(";")
  IntCmp $3 ${NSIS_MAX_STRLEN} +4 +4 0
    DetailPrint "AddToPath: new length $3 > ${NSIS_MAX_STRLEN}"
    MessageBox MB_OK "PATH not updated, new length $3 > ${NSIS_MAX_STRLEN}."
    Goto done

  ; Append dir to PATH
  DetailPrint "Add to PATH: $1"
  StrCpy $3 $2 1 -1
  StrCmp $3 ";" 0 +2
    StrCpy $2 $2 -1 ; remove trailing ';'
  StrCmp $2 "" +2   ; no leading ';'
    StrCpy $1 "$2;$1"
  Strcmp $0 "AllUsers" 0 +2
    WriteRegExpandStr HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "PATH" $1
  Strcmp $0 "CurrentUser" 0 +2
    WriteRegExpandStr HKCU "Environment" "PATH" $1
  SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000

done:
  Pop $5
  Pop $4
  Pop $3
  Pop $2
  Pop $1
  Pop $0
FunctionEnd


; RemoveFromPath - Removes dir from PATH
;
; Usage:
;   Push "dir"
;   Push "AllUsers" or "CurrentUser"
;   Call RemoveFromPath

Function un.RemoveFromPath
  Exch $0  ; $1 has mode
  Exch
  Exch $1  ; $0 has directory
  Push $2
  Push $3
  Push $4
  Push $5
  Push $6
  Push $7


  Strcmp $0 "AllUsers" 0 +2
    ReadRegStr $2 HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "PATH" 
  Strcmp $0 "CurrentUser" 0 +2
    ReadRegStr $2 HKCU "Environment" "PATH"
  StrCpy $6 $2 1 -1
  StrCmp $6 ";" +2
    StrCpy $2 "$2;" ; ensure trailing ';'
  Push $2
  Push "$1;"
  Call un.StrStr
  Pop $3 ; pos of our dir
  StrCmp $3 "" done

  DetailPrint "Remove from PATH: $1"
  StrLen $4 "$1;"
  StrLen $5 $3
  StrCpy $6 $2 -$5 ; $6 is now the part before the path to remove
  StrCpy $7 $3 "" $4 ; $7 is now the part after the path to remove
  StrCpy $4 "$6$7"
  StrCpy $6 $4 1 -1
  StrCmp $6 ";" 0 +2
    StrCpy $4 $4 -1 ; remove trailing ';'
  Strcmp $0 "AllUsers" 0 +2
    WriteRegExpandStr HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "PATH" $4
  Strcmp $0 "CurrentUser" 0 +2
    WriteRegExpandStr HKCU "Environment" "PATH" $4
  SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000

done:
  Pop $7
  Pop $6
  Pop $5
  Pop $4
  Pop $3
  Pop $2
  Pop $1
  Pop $0
FunctionEnd
 

; StrStr - find substring in a string
;
; Usage:
;   Push "this is some string"
;   Push "some"
;   Call StrStr
;   Pop $0 ; "some string"

!macro StrStr un
Function ${un}StrStr
  Exch $R1 ; $R1=substring, stack=[old$R1,string,...]
  Exch     ;                stack=[string,old$R1,...]
  Exch $R2 ; $R2=string,    stack=[old$R2,old$R1,...]
  Push $R3
  Push $R4
  Push $R5
  StrLen $R3 $R1
  StrCpy $R4 0
  ; $R1=substring, $R2=string, $R3=strlen(substring)
  ; $R4=count, $R5=tmp
  loop:
    StrCpy $R5 $R2 $R3 $R4
    StrCmp $R5 $R1 done
    StrCmp $R5 "" done
    IntOp $R4 $R4 + 1
    Goto loop
done:
  StrCpy $R1 $R2 "" $R4
  Pop $R5
  Pop $R4
  Pop $R3
  Pop $R2
  Exch $R1 ; $R1=old$R1, stack=[result,...]
FunctionEnd
!macroend
!insertmacro StrStr ""
!insertmacro StrStr "un."
