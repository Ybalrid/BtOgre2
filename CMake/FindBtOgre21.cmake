# - Try to find BtOgre21

#set(BtOgre21_ROOT "notSet" CACHE PATH "Where your BtOgre21 instlallation is")

set(BtOgre21_ROOT $ENV{BtOgre21_ROOT})

message("BtOgre21_ROOT :" "${BtOgre21_ROOT}")

find_path(BtOgre21_INCLUDE_DIR BtOgre.hpp HINTS ${BtOgre21_ROOT}/include PATH_SUFFIXES BtOgre21)
find_library(BtOgre21_LIBRARY libBtOgre21.a BtOgre21.lib libBtOgre21 BtOgre21 HINTS ${BtOgre21_ROOT}/lib ${BtOgre21_ROOT}/build/Release/ PATH_SUFFIXES BtOgre21 BtOgre lib)
find_library(BtOgre21_DEBUG_LIBRARY libBtOgre21_d.a BtOgre21_d.lib libBtOgre21_d BtOgre21_d HINTS ${BtOgre21_ROOT}/lib ${BtOgre21_ROOT}/build/Debug PATH_SUFFIXES BtOgre21 BtOgre lib)


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(BtOgre21 DEFAULT_MSG BtOgre21_LIBRARY BtOgre21_INCLUDE_DIR)

mark_as_advanced(BtOgre21_INCLUDE_DIR BtOgre21_LIBRARY BtOgre21_DEBUG_LIBRARY)

set(BtOgre21_INCLUDE_DIRS ${BtOgre21_INCLUDE_DIR})
set(BtOgre21_LIBRARIES optimized ${BtOgre21_LIBRARY} debug ${BtOgre21_DEBUG_LIBRARY})

