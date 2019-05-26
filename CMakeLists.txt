#
# build file for lifutils
#
project(lifutils)
cmake_minimum_required (VERSION 2.8.4)
enable_testing()
#
# tweaks for Microsoft Visual C: 
# - link VC runtime statically
# - get rid of unsafe or deprecated warnings
# - this must be after the project statement and before the build type 
#   definition
# - increase stack size to 2 MB else lifpack will not run
#
if(MSVC)
  set(CMAKE_C_FLAGS_DEBUG "/D_DEBUG /MTd /Zi /Ob0 /Od /RTC1")
  set(CMAKE_C_FLAGS_MINSIZEREL     "/MT /O1 /Ob1 /D NDEBUG")
  set(CMAKE_C_FLAGS_RELEASE        "/MT /O2 /Ob2 /D NDEBUG")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO "/MT /Zi /O2 /Ob1 /D NDEBUG")
  add_definitions("-D_CRT_SECURE_NO_WARNINGS")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -STACK:2097152")
endif(MSVC)
#
# set build type = Release
#
set(CMAKE_BUILD_TYPE "Release")
#
# Install Prefix
#
if(UNIX)
  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CMAKE_INSTALL_PREFIX "/usr")
  else()
    set(CMAKE_INSTALL_PREFIX "/usr/local")
  endif()
endif(UNIX)
#
# do portability checks and generate config.h
#
include(CheckIncludeFile)
include(CheckSymbolExists)
check_include_file("unistd.h" HAVE_UNISTD_H)
if(HAVE_UNISTD_H)
check_symbol_exists("getopt" "unistd.h" HAVE_GETOPT_F)
endif(HAVE_UNISTD_H)
check_symbol_exists("getline" "stdio.h" HAVE_GETLINE_F)
if(WIN32)
  check_include_file("io.h" HAVE_IO_H)
  check_include_file("BaseTsd.h" HAVE_BASETSD_H)
  check_symbol_exists("_setmode" "io.h;fcntl.h" HAVE__SETMODE)
  if(NOT HAVE__SETMODE)
    check_symbol_exists("setmode" "io.h;fcntl.h" HAVE_SETMODE)
  endif(NOT HAVE__SETMODE)
  check_symbol_exists("_fileno" "stdio.h" HAVE__FILENO)
  if(NOT HAVE__FILENO)
    check_symbol_exists("fileno" "stdio.h" HAVE_FILENO)
  endif(NOT HAVE__FILENO)
  check_symbol_exists("_stricmp" "string.h" HAVE__STRICMP_F)
  check_symbol_exists("_strnicmp" "string.h" HAVE__STRNICMP_F)
  check_symbol_exists("_MAX_PATH" "stdlib.h" HAVE__MAX_PATH)
endif(WIN32)
configure_file("config.h.cmake" "${CMAKE_CURRENT_BINARY_DIR}/config.h")

set( LIB_SOURCES "" )
set( LIB_HEADERS "" )
#
# build library
#
set(srclist lif_create_entry.c lif_dir_utils.c print_41_data.c scramble_41.c descramble_41.c xrom.c modfile.c lif_block.c)
set(inclist lif_create_entry.h lif_dir_utils.h print_41_data.h scramble_41.h descramble_41.h xrom.h modfile.h lif_img.h lif_block.h lif_phy.h)
if(UNIX)
   if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
      message("Including physical floppy support on ${CMAKE_SYSTEM_NAME}")
      list(APPEND srclist lif_img.c lif_phy_linux.c)
   else()
      message("No physical floppy support on ${CMAKE_SYSTEM_NAME} system")
      list(APPEND srclist lif_img.c lif_phy_dummy.c)
   endif()
endif(UNIX)
if(WIN32)
   list(APPEND srclist lif_img_win.c getline.c getopt.c lif_phy_dummy.c)
   list(APPEND inclist getline.h getopt.h)
endif(WIN32)
foreach (sourcefile ${srclist} )
   list(APPEND LIB_SOURCES src/lib/${sourcefile} )
endforeach (sourcefile ${srclist} )
foreach (incfile ${inclist} )
   list(APPEND LIB_HEADERS src/lib/${incfile} )
endforeach (incfile ${inclist} )
list(APPEND LIB_HEADERS "${CMAKE_CURRENT_BINARY_DIR}/config.h")

add_library (lifutils ${LIB_SOURCES} ${LIB_HEADERS} )
#
# buld emu7470
#
set( LIB_SOURCES "" )
list(APPEND LIB_SOURCES src/emu7470/chardraw.c src/emu7470/lindef.c)
add_library ("emu7470support" ${LIB_SOURCES} ${LIB_HEADERS} )
include_directories("src/emu7470")
add_executable( "emu7470" "src/emu7470/emu7470.c")
target_link_libraries ( "emu7470" "emu7470support")
IF(UNIX)
   target_link_libraries("emu7470" "m")
ENDIF(UNIX)
#
# build all other executables
#
include_directories ("src/lib" "${CMAKE_CURRENT_BINARY_DIR}")
set(srclist lifdir.c lifget.c lifpurge.c liflabel.c lifrename.c liftext.c sdata.c decomp41.c liftext75.c textlif75.c regs41.c stat41.c key41.c wall41.c wcat41.c lifstat.c sdatabar.c comp41.c barprt.c barps.c rom41er.c er41rom.c prog41bar.c lifput.c textlif.c raw41lif.c lifraw.c rom41hx.c lifinit.c lifpack.c liffix.c lifmod.c lexcat71.c hx41rom.c lifheader.c lifversion.c rom41cat.c rom41lif.c in71.c out71.c inp41.c outp41.c)
if(UNIX)
   if(NOT APPLE)
      list(APPEND srclist lifimage.c lifdump.c)
   endif(NOT APPLE)
endif(UNIX)
if(WIN32)
endif(WIN32)

foreach (sourcefile ${srclist} )
   string( REPLACE ".c" "" progname ${sourcefile} )
   add_executable( ${progname} src/progs/${sourcefile} res/lifutils.rc)
   target_link_libraries( ${progname} lifutils )
endforeach (sourcefile ${srclist} )
#
# install executables
#
IF(UNIX)
   set(BINPATH "bin")
   set(DOCPATH "share/doc/lifutils")
   set(DATAPATH "share/lifutils")
   set(MANPATH "share/man")
ENDIF(UNIX)
IF(WIN32)
   set(BINPATH ".")
   set(DOCPATH "doc")
   set(DATAPATH ".")
ENDIF(WIN32)
install(TARGETS "emu7470" DESTINATION ${BINPATH})
foreach (sourcefile ${srclist} )
   string( REPLACE ".c" "" progname ${sourcefile} )
   install(TARGETS ${progname} DESTINATION ${BINPATH})
endforeach (sourcefile ${srclist} )
#
# install documentation
#
install(FILES doc/copying.html doc/readme.html doc/tutorial.html doc/GPL-2 DESTINATION ${DOCPATH} )
foreach (sourcefile ${srclist} )
   string( REPLACE ".c" "" progname ${sourcefile} )
   install(FILES doc/html/${progname}.html DESTINATION ${DOCPATH}/html)
endforeach (sourcefile ${srclist} )
install(DIRECTORY doc/hardware DESTINATION ${DOCPATH})
#
# install program data
#
install(DIRECTORY xroms DESTINATION ${DATAPATH})
#
# install man pages
#
if(UNIX)
foreach (sourcefile ${srclist} )
   string( REPLACE ".c" "" progname ${sourcefile} )
   install(FILES man/${progname}.1 DESTINATION ${MANPATH}/man1)
endforeach (sourcefile ${srclist} )
install(FILES man/barcode.5 DESTINATION ${MANPATH}/man5)
endif(UNIX)
#
# tests
#
add_test(NAME basic WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/test COMMAND  "${CMAKE_SOURCE_DIR}/test/runtest" "basic" "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_SOURCE_DIR}/xroms")
