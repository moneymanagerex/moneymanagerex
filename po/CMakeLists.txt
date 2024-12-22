get_directory_property(m_hasParent PARENT_DIRECTORY)
if(NOT m_hasParent)
    message(FATAL_ERROR "Use the top-level CMake script!")
endif()
unset(m_hasParent)

find_package(Gettext REQUIRED)

if(NOT GETTEXT_VERSION_STRING)
    execute_process(COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE} --version
        OUTPUT_VARIABLE m_version
        OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    get_filename_component(m_name ${GETTEXT_MSGMERGE_EXECUTABLE} NAME)
    get_filename_component(m_namewe ${GETTEXT_MSGMERGE_EXECUTABLE} NAME_WE)
    if(m_version MATCHES "^(${m_name}|${m_namewe}) \\([^\\)]*\\) ([0-9\\.]+[^ \n]*)")
        set(GETTEXT_VERSION_STRING "${CMAKE_MATCH_2}")
    endif()
    unset(m_version)
    unset(m_name)
    unset(m_namewe)
endif()
set(GETTEXT_VERSION_STRING "${GETTEXT_VERSION_STRING}"
    CACHE INTERNAL "gettext version detected")

file(GLOB POFiles RELATIVE "${CMAKE_CURRENT_BINARY_DIR}" *.po)
foreach(POFile ${POFiles})
    get_filename_component(CurrentFile "${POFile}" NAME_WE)
    if(CurrentFile MATCHES "common")
        continue()
    endif()
    # Copy po file and append common translations
    file(COPY ${CurrentFile}.po DESTINATION ${CMAKE_BINARY_DIR}/po)
    add_custom_command(
        OUTPUT ${CurrentFile}.mo
        COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} -v --statistics -o ${CurrentFile}.mo ${CMAKE_BINARY_DIR}/po/${CurrentFile}.po
        COMMENT ""
        MAIN_DEPENDENCY "${POFile}")

    # wxWidgets standard paths
    # https://github.com/wxWidgets/wxWidgets/blob/master/docs/doxygen/overviews/internationalization.h#L86
    if(APPLE)
        set(MODir ${MMEX_EXE}.app/Contents/Resources/${CurrentFile}.lproj)
    elseif(WIN32)
        set(MODir bin/${CurrentFile})
    else()
        set(MODir share/${MMEX_EXE}/${CurrentFile})
    #    set(MODir share/locale/${CurrentFile}/LC_MESSAGES)
    endif()
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${CurrentFile}.mo"
        DESTINATION ${MODir}
        RENAME ${MMEX_EXE}.mo)

    list(APPEND MOFiles "${CMAKE_CURRENT_BINARY_DIR}/${CurrentFile}.mo")
endforeach()

# put the common UI Languages translations in the en_US translations directory
file(COPY "common.po" DESTINATION ${CMAKE_BINARY_DIR}/po)
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/po/common.mo
    COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} -v --statistics -o ${CMAKE_BINARY_DIR}/po/common.mo ${CMAKE_BINARY_DIR}/po/common.po
    COMMENT ""
    MAIN_DEPENDENCY common.po
)

if(APPLE)
    set(MODir ${MMEX_EXE}.app/Contents/Resources/en_US.lproj)
elseif(WIN32)
    set(MODir bin/en_US)
else()
    set(MODir share/${MMEX_EXE}/en_US)
endif()

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/common.mo"
        DESTINATION ${MODir})

list(APPEND MOFiles "${CMAKE_CURRENT_BINARY_DIR}/common.mo")

add_custom_target(Translations ALL DEPENDS ${MOFiles}
    COMMENT "Generated translations")
