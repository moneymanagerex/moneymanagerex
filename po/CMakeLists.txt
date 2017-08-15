IF(NOT InMMEX)
    MESSAGE(FATAL_ERROR "Use the top-level CMake script!")
ENDIF(NOT InMMEX)

FIND_PACKAGE(Gettext REQUIRED)

ADD_CUSTOM_TARGET(Translations ALL)
FILE(GLOB POFiles "${CMAKE_CURRENT_SOURCE_DIR}/*.po")

IF(IS_VS_2017CMAKE EQUAL 1)
    SET(INSTALL_MO_DIR "${CMAKE_SOURCE_DIR}/bin/${TARGET_ARCH}-${CMAKE_BUILD_TYPE}/po/en")
ENDIF()

FOREACH(POFile ${POFiles})
    GET_FILENAME_COMPONENT(CurrentFile ${POFile} NAME_WE)
    SET(MOFile "${CMAKE_CURRENT_BINARY_DIR}/${CurrentFile}.mo")

    ADD_CUSTOM_COMMAND(TARGET Translations
            # Should be COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} -o ${MOFile} ${POFile}
            # but MOFile full path has been removed because VS2017 Ninja appends all commands in a unique command that exceeds max lenght
            # Anyway an error will be triggered if folder is different because install command below is based ${MOFile} and is not optional
            COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} -o "${CurrentFile}.mo" ${POFile}
            DEPENDS ${POFile})
    INSTALL(FILES ${MOFile} DESTINATION ${INSTALL_MO_DIR})
ENDFOREACH()
