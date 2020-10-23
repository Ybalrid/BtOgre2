cmake_minimum_required(VERSION 3.17)

project(BtOgre21DEMO)

#Use the cmake-extra from the BtOgre directory, and the findBtOgre from the BtOgre directory too 
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/../cmake-extras/ ${PROJECT_SOURCE_DIR}/../CMake)

set(CMAKE_CXX_STANDARD 11)

#set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../cmake-extras/;
#                      ${PROJECT_SOURCE_DIR}/../CMake/)

message("CMAKE_MODULE_PATH :" ${CMAKE_MODULE_PATH})
include(../cmake-extras/OGRE.cmake)

#Need the physics engine
find_package(Bullet REQUIRED)
#Need the graphics engine
find_package(OGRE COMPONENTS HlmsPbs HlmsUnlit REQUIRED)
#Need the BtOgre package we are testing
find_package(BtOgre21 REQUIRED)
#Need SDL2 for window and input
find_package(SDL2 REQUIRED)

message("BULLET_INCLUDE_DIRS :" ${BULLET_INCLUDE_DIRS})

#Add the includes directories
include_directories(
        ${PROJECT_SOURCE_DIR}/exampleapp
        ${BULLET_INCLUDE_DIRS}
        ${OGRE_INCLUDE_DIRS}
        ${OGRE_HlmsPbs_INCLUDE_DIR}
        ${OGRE_HlmsUnlit_INCLUDE_DIR}
        ${OGRE_HlmsCommon_INCLUDE_DIR}
        ${BtOgre21_INCLUDE_DIRS}
        ${SDL2_INCLUDE_DIR}
)




link_directories(
${BULLET_LIBRARY_DIRS})


#Set the executable
add_executable(BtOgre21DEMO main.cpp)


message("OGRE_HlmsPbs_LIBRARIES :" ${OGRE_HlmsPbs_LIBRARIE})
message("BULLET_LIBRARY_DIRS :" ${BULLET_LIBRARY_DIRS})
message("BULLET_LIBRARIES :" ${BULLET_LIBRARIES})
message("BtOgre21_LIBRARIES :" ${BtOgre21_LIBRARIES})

#Link the required libraries
target_link_libraries(
        BtOgre21DEMO
        ${BtOgre21_LIBRARIES}
        ${BULLET_LIBRARIES}
        ${OGRE_LIBRARIES}
        ${OGRE_HlmsPbs_LIBRARY}
        ${OGRE_HlmsUnlit_LIBRARY}
        ${SDL2_LIBRARY}
)

target_compile_features(BtOgre21DEMO PUBLIC cxx_std_14)

message("OGRE_LIBRARIES :" ${OGRE_LIBRARIES})
if (MSVC)
  set_target_properties(BtOgre21DEMO 
                        PROPERTIES
                        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
else()
  set_target_properties(BtOgre21DEMO 
                        PROPERTIES
                        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}")
endif()

#Force MSVC to use the Windows subsystem. This makes the program GUI only and make the entry point "WinMain"
if (MSVC)
    set_target_properties(BtOgre21DEMO PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
    set_target_properties(BtOgre21DEMO PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS")
    set_target_properties(BtOgre21DEMO PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS")
    set_target_properties(BtOgre21DEMO PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:WINDOWS")
endif ()

#install binary resources
#file(COPY data DESTINATION ${CMAKE_BINARY_DIR})
#file(COPY ${OGRE_MEDIA_DIR}/Hlms DESTINATION ${CMAKE_BINARY_DIR}/data/HLMS/)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG=1 -D_DEBUG=1" )
endif()

setupOgre( OGRE_SOURCE, TOTO, OGRE_LIBRARIES, FALSE )

if(UNIX)
    #file(COPY ${OGRE_MEDIA_DIR}/../plugins.cfg DESTINATION ${CMAKE_BINARY_DIR})
    #file( COPY "${OGRE_SOURCE}/Samples/Media/packs/DebugPack.zip"	DESTINATION "${CMAKE_SOURCE_DIR}/bin/Data" )



elseif(MSVC)
#    file(COPY ${OGRE_MEDIA_DIR}/../bin/release/plugins.cfg DESTINATION ${CMAKE_BINARY_DIR})
#    file(GLOB GLOBBED_DLLS ${OGRE_MEDIA_DIR}/../bin/release/*.dll)
#    file(COPY ${GLOBBED_DLLS} DESTINATION ${CMAKE_BINARY_DIR})
endif()
