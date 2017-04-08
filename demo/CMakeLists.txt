cmake_minimum_required(VERSION 2.8.0)

project(BtOgre21DEMO)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../cmake-extras/)

find_package(Bullet REQUIRED)
find_package(OGRE REQUIRED)

set (BTOGRE21_INCLUDE "notSet" CACHE PATH "Where BtOgre21.hpp is located")
set (BTOGRE21_LIB "notSet" CACHE PATH "Wher is BtOgre21.lib is located")

include_directories(
	${PROJECT_SOURCE_DIR}/exampleapp
	${BULLET_INCLUDE_DIRS}
	${OGRE_INCLUDE_DIRS}
	${BTOGRE21_INCLUDE}
)

link_directories(${BTOGRE21_LIB})


add_executable(BtOgre21DEMO main.cpp exampleapp/ExampleApplication.h exampleapp/ExampleFrameListener.h)
target_link_libraries(
	BtOgre21DEMO 
	${BULLET_LIBRARIES} 
	${OGRE_LIBRARIES}
	${OGRE_LIBRARY_DIR_REL}/OgreHlmsPbs.lib 
	${OGRE_LIBRARY_DIR_REL}/OgreHlmsUnlit.lib 
	${BTOGRE21_LIB}/BtOgre21.lib)

if(MSVC)
   set_target_properties(BtOgre21DEMO PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
   set_target_properties(BtOgre21DEMO PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS")
   set_target_properties(BtOgre21DEMO PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS")
   set_target_properties(BtOgre21DEMO PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:WINDOWS")
endif()
