include(FindPkgMacros)
include(PreprocessorUtils)


set(BULLET_ROOT_DIR $ENV{BULLET_ROOT_DIR})

message("BULLET_ROOT_DIR :" ${BULLET_ROOT_DIR})

set ( BULLET_FOUND 1 )
set ( BULLET_USE_FILE       "${BULLET_ROOT_DIR}/lib/cmake/bullet/UseBullet.cmake" )
set ( BULLET_DEFINITIONS    "" )
set ( BULLET_INCLUDE_DIR    "${BULLET_ROOT_DIR}/include/bullet" )
set ( BULLET_INCLUDE_DIRS   "${BULLET_ROOT_DIR}/include/bullet" )
set ( BULLET_LIBRARIES      "LinearMath;Bullet3Common;BulletInverseDynamics;BulletCollision;BulletDynamics;BulletSoftBody" )
set ( BULLET_LIBRARY_DIRS   "${BULLET_ROOT_DIR}/lib" )
set ( BULLET_VERSION_STRING "3.05" )
