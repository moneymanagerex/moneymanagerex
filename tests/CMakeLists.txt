IF(NOT InMMEX)
    MESSAGE(FATAL_ERROR "Use the top-level CMake script!")
ENDIF(NOT InMMEX)

FILE(GLOB MMEXTest_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    )

TARGET_SOURCES(MMEXTest PRIVATE
        ${MMEXTest_SRC}
        "${CMAKE_SOURCE_DIR}/src/mmex.cpp"
        "${CMAKE_SOURCE_DIR}/src/mmex.h"
        "${CMAKE_SOURCE_DIR}/3rd/cgitemplate/html_template.cpp"
        "${CMAKE_SOURCE_DIR}/3rd/mongoose/mongoose.c"
        )

TARGET_COMPILE_DEFINITIONS(MMEXTest PRIVATE wxIMPLEMENT_WXWIN_MAIN=)
# hide warnings from cppunit includes
TARGET_COMPILE_OPTIONS(MMEXTest PRIVATE "-Wno-deprecated-declarations")

IF (WIN32)
    TARGET_SOURCES(MMEXTest PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/mmex_tests.rc")
ENDIF ()