cmake_minimum_required(VERSION 2.8.0)

project(BtOgre21DEMO)

#Use the cmake-extra from the BtOgre directory, and the findBtOgre from the BtOgre directory too 
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../cmake-extras/ ${PROJECT_SOURCE_DIR}/../CMake)

#Need the physics engine
find_package(Bullet REQUIRED)
#Need the graphics engine
find_package(OGRE COMPONENTS HlmsPbs HlmsUnlit REQUIRED)
#Need the BtOgre package we are testing
find_package(BtOgre21 REQUIRED)

#Add the includes directories
include_directories(
        ${PROJECT_SOURCE_DIR}/exampleapp
        ${BULLET_INCLUDE_DIRS}
        ${OGRE_INCLUDE_DIRS}
        ${OGRE_HlmsPbs_INCLUDE_DIRS}
        ${OGRE_HlmsUnlit_INCLUDE_DIRS}
        ${BtOgre21_INCLUDE_DIRS}
)

#Set the executable
add_executable(BtOgre21DEMO main.cpp exampleapp/ExampleApplication.h exampleapp/ExampleFrameListener.h)

#Link the required libraries
target_link_libraries(
        BtOgre21DEMO
        ${BULLET_LIBRARIES}
        ${OGRE_LIBRARIES}
        ${OGRE_HlmsPbs_LIBRARIES}
        ${OGRE_HlmsUnlit_LIBRARIES}
        ${BtOgre21_LIBRARIES}
)

#Force MSVC to use the Windows subsystem. This makes the program GUI only and make the entry point "WinMain"
if (MSVC)
    set_target_properties(BtOgre21DEMO PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
    set_target_properties(BtOgre21DEMO PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS")
    set_target_properties(BtOgre21DEMO PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS")
    set_target_properties(BtOgre21DEMO PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:WINDOWS")
endif ()

#install binary resources
file(COPY data DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${OGRE_MEDIA_DIR}/Hlms DESTINATION ${CMAKE_BINARY_DIR}/HLMS/)
if(UNIX)
    file(COPY ${OGRE_MEDIA_DIR}/../plugins.cfg DESTINATION ${CMAKE_BINARY_DIR})
endif()
