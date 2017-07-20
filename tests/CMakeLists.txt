IF(NOT InMMEX)
    MESSAGE(FATAL_ERROR "Use the top-level CMake script!")
ENDIF(NOT InMMEX)

FILE(GLOB CPPUnit_SRC
    "${CMAKE_SOURCE_DIR}/lib/cppunit/src/cppunit/*.h"
    "${CMAKE_SOURCE_DIR}/lib/cppunit/src/cppunit/*.cpp"
    )

FILE(GLOB MMEXTest_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    )

FILE(GLOB_RECURSE MMEX_SRC
    "${CMAKE_SOURCE_DIR}/src/*.h"
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    )
LIST(REMOVE_ITEM MMEX_SRC
    "${CMAKE_SOURCE_DIR}/src/mac/platfdep.cpp"
    "${CMAKE_SOURCE_DIR}/src/unix/platfdep.cpp"
    "${CMAKE_SOURCE_DIR}/src/win/platfdep.cpp"
    )

TARGET_SOURCES(CPPUnit PRIVATE
    ${CPPUnit_SRC}
    )

TARGET_SOURCES(MMEXTest PRIVATE
    ${MMEXTest_SRC}
    ${MMEX_SRC}
    "${CMAKE_SOURCE_DIR}/3rd/cgitemplate/html_template.cpp"
    "${CMAKE_SOURCE_DIR}/3rd/mongoose/mongoose.c"
    )

TARGET_COMPILE_DEFINITIONS(MMEXTest PRIVATE wxIMPLEMENT_WXWIN_MAIN=)

#OSX
IF (UNIX AND APPLE)
    TARGET_SOURCES(MMEXTest PRIVATE "${CMAKE_SOURCE_DIR}/src/mac/platfdep.cpp")
ENDIF ()
#*nix
IF (UNIX AND NOT APPLE)
    TARGET_SOURCES(MMEXTest PRIVATE "${CMAKE_SOURCE_DIR}/src/unix/platfdep.cpp")
ENDIF ()
#Windows
IF (WIN32)
    TARGET_SOURCES(MMEXTest PRIVATE
        "${CMAKE_SOURCE_DIR}/src/win/platfdep.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/mmex_tests.rc"
        )
ENDIF ()