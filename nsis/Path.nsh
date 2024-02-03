;--------------------------------------------------------------------
; Path functions
; uses the envar plugin
;
; Usage:
;   Push "dir"
;   Push "AllUsers" or "CurrentUser"
;   Call AddToPath

Function AddToPath
  Exch $0  ; $0 has mode
  Exch
  Exch $1  ; $1 has directory

  Strcmp $0 "AllUsers" 0 +2
     EnVar::SetHKLM
  Strcmp $0 "CurrentUser" 0 +2
     EnVar::SetHKCU
  EnVar::AddValue 'PATH' $1
  Pop $0
  DetailPrint "EnVar:AddValue returned $0"
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

  Strcmp $0 "AllUsers" 0 +2
     EnVar::SetHKLM
  Strcmp $0 "CurrentUser" 0 +2
     EnVar::SetHKCU
  EnVar::DeleteValue 'PATH' $1
  Pop $0
  DetailPrint "EnVar:DeleteValue returned $0"

  Pop $1
  Pop $0
FunctionEnd
