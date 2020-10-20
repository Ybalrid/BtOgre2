cmake_minimum_required(VERSION 3.1)

project(BtOgre21)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-extras/)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG=1 -D_DEBUG=1")
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /fp:fast")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
endif()

message("CMAKE_MODULE_PATH :" ${CMAKE_MODULE_PATH})

find_package(Bullet REQUIRED)
find_package(OGRE REQUIRED)

message("BULLET_INCLUDE_DIR: " ${BULLET_INCLUDE_DIR})

include_directories(
    ${PROJECT_SOURCE_DIR}/include/
    ${BULLET_INCLUDE_DIRS}
    ${OGRE_INCLUDE_DIRS}
    ${OGRE_HlmsUnlit_INCLUDE_DIR}
    ${OGRE_HlmsCommon_INCLUDE_DIR}
)

if(NOT CMAKE_DEBUG_POSTFIX)
  set(CMAKE_DEBUG_POSTFIX _d)
endif()

add_library(BtOgre21 STATIC sources/BtOgreGP.cpp sources/BtOgrePG.cpp sources/BtOgreExtras.cpp include/BtOgre.hpp include/BtOgreExtras.h include/BtOgreGP.h include/BtOgrePG.h)
target_link_libraries(BtOgre21 ${BULLET_LIBRARIES} ${OGRE_LIBRARIES})

file(GLOB PDB_Files Debug/*.pdb RelWithDebInfo/*.pdb)

if(NOT PDB_Files STREQUAL "")
    INSTALL(FILES PDB_Files DESTINATION "lib/BtOgre21")
endif()

INSTALL(TARGETS BtOgre21 DESTINATION "lib/BtOgre21")
INSTALL(FILES include/BtOgrePG.h include/BtOgreGP.h include/BtOgreExtras.h include/BtOgre.hpp DESTINATION "include/BtOgre21")
file (COPY CMake DESTINATION ${CMAKE_BINARY_DIR})
INSTALL(DIRECTORY CMake DESTINATION "lib/BtOgre21")
