cmake_minimum_required(VERSION 2.8.0)

project(BtOgre21)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-extras/)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /fp:fast")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
endif()

find_package(Bullet REQUIRED)
find_package(OGRE REQUIRED)

include_directories(
    ${PROJECT_SOURCE_DIR}/include/
    ${BULLET_INCLUDE_DIRS}
    ${OGRE_INCLUDE_DIRS}
)

if(NOT CMAKE_DEBUG_POSTFIX)
  set(CMAKE_DEBUG_POSTFIX _d)
endif()

add_library(BtOgre21 STATIC sources/BtOgreGP.cpp sources/BtOgrePG.cpp sources/BtOgreExtras.cpp include/BtOgre.hpp include/BtOgreExtras.h include/BtOgreGP.h include/BtOgrePG.h)
target_link_libraries(BtOgre21 ${BULLET_LIBRARIES} ${OGRE_LIBRARIES})

INSTALL(TARGETS BtOgre21 DESTINATION "lib/BtOgre21")
INSTALL(FILES include/BtOgrePG.h include/BtOgreGP.h include/BtOgreExtras.h include/BtOgre.hpp DESTINATION "include/BtOgre21")
INSTALL(FILES CMake DESTINATION "lib/BtOgre21")
