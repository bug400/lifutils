noinst_LIBRARIES=liblifutils.a
liblifutils_a_SOURCES=ildisp.c ildrive.c lif_block.c lif_block.h lif_create_entry.c lif_create_entry.h lif_dir_utils.c lif_dir_utils.h print_41_data.c print_41_data.h scramble_41.c scramble_41.h descramble_41.c descramble_41.h xrom.c xrom.h modfile.c modfile.h lif_img.c lif_img.h
if LINUX
liblifutils_a_SOURCES += lif_phy_linux.c lif_phy.h
endif
if WINDOWS
liblifutils_a_SOURCES += getline.c getline.h lif_phy_dummy.c lif_phy.h
endif
if MACOSX
liblifutils_a_SOURCES += lif_phy_dummy.c lif_phy.h
endif
