IF(NOT InMMEX)
    MESSAGE(FATAL_ERROR "Use the top-level CMake script!")
ENDIF(NOT InMMEX)

FILE(GLOB_RECURSE MMEX_SRC
    "${CMAKE_SOURCE_DIR}/src/*.h"
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    )
LIST(REMOVE_ITEM MMEX_SRC
    "${CMAKE_SOURCE_DIR}/src/mac/platfdep.cpp"
    "${CMAKE_SOURCE_DIR}/src/unix/platfdep.cpp"
    "${CMAKE_SOURCE_DIR}/src/win/platfdep.cpp"
    )

TARGET_SOURCES(${PROJECT_TARGET_BIN} PRIVATE
        ${MMEX_SRC}
        "${CMAKE_SOURCE_DIR}/3rd/cgitemplate/html_template.cpp"
        "${CMAKE_SOURCE_DIR}/3rd/mongoose/mongoose.c"
        )
#OSX
IF (UNIX AND APPLE)
    TARGET_SOURCES(${PROJECT_TARGET_BIN} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/mac/platfdep.cpp")
ENDIF ()
#*nix
IF (UNIX AND NOT APPLE)
    TARGET_SOURCES(${PROJECT_TARGET_BIN} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/unix/platfdep.cpp")
ENDIF ()
#Windows
IF (WIN32)
    TARGET_SOURCES(${PROJECT_TARGET_BIN} PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/win/platfdep.cpp"
            "resources/mmex.rc")

ENDIF ()