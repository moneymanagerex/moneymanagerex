IF(NOT InMMEX)
    MESSAGE(FATAL_ERROR "Use the top-level CMake script!")
ENDIF(NOT InMMEX)

FIND_PACKAGE(Gettext REQUIRED)

IF(NOT GETTEXT_VERSION_STRING)
    EXECUTE_PROCESS(COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE} --version
        OUTPUT_VARIABLE m_version
        OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    GET_FILENAME_COMPONENT(m_name ${GETTEXT_MSGMERGE_EXECUTABLE} NAME)
    GET_FILENAME_COMPONENT(m_namewe ${GETTEXT_MSGMERGE_EXECUTABLE} NAME_WE)
    IF(m_version MATCHES "^(${m_name}|${m_namewe}) \\([^\\)]*\\) ([0-9\\.]+[^ \n]*)")
        SET(GETTEXT_VERSION_STRING "${CMAKE_MATCH_2}")
    ENDIF()
    UNSET(m_version)
    UNSET(m_name)
    UNSET(m_namewe)
ENDIF()
SET(GETTEXT_VERSION_STRING "${GETTEXT_VERSION_STRING}"
    CACHE INTERNAL "gettext version detected")

FILE(GLOB POFiles RELATIVE "${CMAKE_CURRENT_BINARY_DIR}" *.po)
FOREACH(POFile ${POFiles})
    GET_FILENAME_COMPONENT(CurrentFile "${POFile}" NAME_WE)
    ADD_CUSTOM_COMMAND(OUTPUT ${CurrentFile}.mo
        # COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE} --quiet --update --backup=none -s "${POFile}" ${MMEX_EXE}.pot
        COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} -v --statistics -o ${CurrentFile}.mo "${POFile}"
        COMMENT ""
        MAIN_DEPENDENCY "${POFile}")

    # wxWidgets standard paths
    # https://github.com/wxWidgets/wxWidgets/blob/master/docs/doxygen/overviews/internationalization.h#L86
    IF(APPLE)
        set(MODir ${MMEX_EXE}.app/Contents/Resources/${CurrentFile}.lproj)
    ELSEIF(WIN32)
        set(MODir bin/${CurrentFile})
    ELSE()
        set(MODir share/${MMEX_EXE}/${CurrentFile})
    #    set(MODir share/locale/${CurrentFile}/LC_MESSAGES)
    ENDIF()
    INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/${CurrentFile}.mo"
        DESTINATION ${MODir}
        RENAME ${MMEX_EXE}.mo)

    LIST(APPEND MOFiles "${CMAKE_CURRENT_BINARY_DIR}/${CurrentFile}.mo")
ENDFOREACH()

ADD_CUSTOM_TARGET(Translations ALL DEPENDS ${MOFiles}
    COMMENT "Generated translations")
