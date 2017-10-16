SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_PROCESSOR x86)

SET(CMAKE_C_COMPILER gcc -m32)
SET(CMAKE_CXX_COMPILER g++ -m32)

SET(ENV{PKG_CONFIG_PATH} /usr/lib/i386-linux-gnu/pkgconfig)
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "i386")
SET(CPACK_RPM_PACKAGE_ARCHITECTURE "i686")
