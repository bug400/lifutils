;
; Installer defines -- product related
!define PRODUCT_NAME "lifutils" ; name of the application as displayed to the user
!define VERSION "${VERSTR}" ; main version of the application (may be 0.1, alpha, beta, etc.)
!define PRODUCT_UUID "{0C786F40-D1C6-4681-9B1D-AFC920428192}" ; do not change this between program versions!
!define PROGEXE "lifversion.exe" ; main application filename
!define PROGHTML "readme.html"; documentation entry point
!define COMMENTS "${PRODUCT_NAME} utilities for LIF images and LIF files" ; stored as comments in the uninstall info of the registry
!define URL_INFO_ABOUT "https://github.com/bug400/${PRODUCT_NAME}" ; stored as the Support Link in the uninstall info of the registry, and when not included, the Help Link as well
!define URL_HELP_LINK "https://github.com/bug400/${PRODUCT_NAME}/blob/master/INSTALL.md" ; stored as the Help Link in the uninstall info of the registry
!define URL_UPDATE_INFO "https://github.com/bug400/${PRODUCT_NAME}/releases" ; stored as the Update Information in the uninstall info of the registry
