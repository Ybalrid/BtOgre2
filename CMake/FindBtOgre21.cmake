# - Try to find BtOgre21

find_path(BtOgre21_INCLUDE_DIR BtOgre21/BtOgre.hpp)
find_library(BtOgre21_LIBRARY BtOgre21 BtOgre21/BtOgre21)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(BtOgre21 DEFAULT_MSG BtOgre21_LIBRARY BtOgre21_INCLUDE_DIR)

mark_as_advanced(BtOgre21_INCLUDE_DIR BtOgre21_LIBRARY)

set(BtOgre21_LIBRARIES ${BtOgre21_LIBRARY})
set(BtOgre21_INCLUDE_DIRS ${BtOgre21_INCLUDE_DIRS})

