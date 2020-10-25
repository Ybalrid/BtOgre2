include(FindPkgMacros)
include(PreprocessorUtils)


set(BULLET_ROOT_DIR $ENV{BULLET_ROOT_DIR})

message("BULLET_ROOT_DIR :" ${BULLET_ROOT_DIR})

if(NOT BULLET_ROOT_DIR)
    message(FATAL_ERROR "The environement variable: BULLET_ROOT_DIR is not set")
endif()

set ( BULLET_FOUND 1 )
set ( BULLET_USE_FILE       "${BULLET_ROOT_DIR}/lib/cmake/bullet/UseBullet.cmake" )
set ( BULLET_DEFINITIONS    "" )
set ( BULLET_INCLUDE_DIR    "${BULLET_ROOT_DIR}/include/bullet" )
set ( BULLET_INCLUDE_DIRS   "${BULLET_ROOT_DIR}/include/bullet" )
set ( BULLET_LIBRARIES_REL      "LinearMath;Bullet3Common;BulletInverseDynamics;BulletCollision;BulletDynamics;BulletSoftBody" )

foreach(name ${BULLET_LIBRARIES_REL})
  if(WIN32)
      set(BULLET_LIBRARIES_DBG ${BULLET_LIBRARIES_DBG} ${name}_Debug)
  else()
      set(BULLET_LIBRARIES_DBG ${BULLET_LIBRARIES_DBG} ${name})
  endif()
endforeach(name)

set(BULLET_LIBRARIES ${BULLET_LIBRARIES_DBG})

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(BULLET_LIBRARIES ${BULLET_LIBRARIES_REL})
endif()

message("BULLET_LIBRARIES :" ${BULLET_LIBRARIES})
message("BULLET_LIBRARIES_REL :" ${BULLET_LIBRARIES_REL})
message("BULLET_LIBRARIES_DBG :" ${BULLET_LIBRARIES_DBG})


set ( BULLET_LIBRARY_DIRS   "${BULLET_ROOT_DIR}/lib" )
set ( BULLET_VERSION_STRING "3.05" )

