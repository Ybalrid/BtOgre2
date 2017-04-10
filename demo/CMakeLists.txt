cmake_minimum_required(VERSION 2.8.0)

project(BtOgre21DEMO)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../cmake-extras/)

find_package(Bullet REQUIRED)
find_package(OGRE COMPONENTS HlmsPbs HlmsUnlit REQUIRED)

set(BTOGRE21_INCLUDE "notSet" CACHE PATH "Directory where BtOgre21.hpp is located")
set(BTOGRE21_LIB "notSet" CACHE PATH "Directory where is BtOgre21.lib is located")

include_directories(
        ${PROJECT_SOURCE_DIR}/exampleapp
        ${BULLET_INCLUDE_DIRS}
        ${OGRE_INCLUDE_DIRS}
        ${OGRE_HlmsPbs_INCLUDE_DIRS}
        ${OGRE_HlmsUnlit_INCLUDE_DIRS}
        ${BTOGRE21_INCLUDE}
)

link_directories(${BTOGRE21_LIB})

if (UNIX)
    set(BtOgre21_LIBRARIES ${BTOGRE21_LIB}/libBtOgre21.a)
elseif (WIN32)
    set(BtOgre21_LIBRARIES ${BTOGRE21_LIB}/BtOgre21.lib)
endif (UNIX)

add_executable(BtOgre21DEMO main.cpp exampleapp/ExampleApplication.h exampleapp/ExampleFrameListener.h)
target_link_libraries(
        BtOgre21DEMO
        ${BULLET_LIBRARIES}
        ${OGRE_LIBRARIES}
        ${OGRE_HlmsPbs_LIBRARIES}
        ${OGRE_HlmsUnlit_LIBRARIES}
        ${BtOgre21_LIBRARIES}
)

if (MSVC)
    set_target_properties(BtOgre21DEMO PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
    set_target_properties(BtOgre21DEMO PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS")
    set_target_properties(BtOgre21DEMO PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS")
    set_target_properties(BtOgre21DEMO PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:WINDOWS")
endif ()

#install dependencies
file(COPY data DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${OGRE_MEDIA_DIR}/Hlms DESTINATION ${CMAKE_BINARY_DIR}/HLMS/)
if(UNIX)
    file(COPY ${OGRE_MEDIA_DIR}/../plugins.cfg DESTINATION ${CMAKE_BINARY_DIR})
endif()
