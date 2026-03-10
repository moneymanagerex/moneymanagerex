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

# mapping from MMEX basename to wxWidgets basename
set(WXSTD "wxstd")
set(WXSTD_MAP_ar_SA "ar")
set(WXSTD_MAP_ca_ES "ca")
set(WXSTD_MAP_cs_CZ "cs")
set(WXSTD_MAP_da_DK "da")
set(WXSTD_MAP_de_DE "de")
set(WXSTD_MAP_el_GR "el")
set(WXSTD_MAP_es_ES "es")
set(WXSTD_MAP_fa_IR "fa_IR")
set(WXSTD_MAP_fr_FR "fr")
set(WXSTD_MAP_hr_HR "hr")
set(WXSTD_MAP_hu_HU "hu")
set(WXSTD_MAP_id_ID "id")
set(WXSTD_MAP_it_IT "it")
set(WXSTD_MAP_ja_JP "ja")
set(WXSTD_MAP_ko_KR "ko_KR")
set(WXSTD_MAP_lt_LT "lt")
set(WXSTD_MAP_nl_NL "nl")
set(WXSTD_MAP_pl_PL "pl")
set(WXSTD_MAP_pt_BR "pt_BR")
set(WXSTD_MAP_pt_PT "pt")
set(WXSTD_MAP_ro_RO "ro")
set(WXSTD_MAP_ru_RU "ru")
set(WXSTD_MAP_sk_SK "sk")
set(WXSTD_MAP_sl_SI "sl")
set(WXSTD_MAP_sq_AL "sq")
set(WXSTD_MAP_sr    "sr")
set(WXSTD_MAP_sv_SE "sv")
set(WXSTD_MAP_ta_IN "ta")
set(WXSTD_MAP_tr_TR "tr")
set(WXSTD_MAP_uk_UA "uk")
set(WXSTD_MAP_vi_VN "vi")
set(WXSTD_MAP_zh_CN "zh_CN")
set(WXSTD_MAP_zh_TW "zh_TW")

set(MMEX_PO_DIRECTORY "${CMAKE_SOURCE_DIR}/po")
set(WXSTD_PO_DIRECTORY "${CMAKE_SOURCE_DIR}/po_wxstd")
#message(STATUS "MMEX_PO_DIRECTORY: ${MMEX_PO_DIRECTORY}")
#message(STATUS "WXSTD_PO_DIRECTORY: ${WXSTD_PO_DIRECTORY}")

file(GLOB MMEX_PO_FILES RELATIVE "${MMEX_PO_DIRECTORY}" ${MMEX_PO_DIRECTORY}/*.po)
foreach(mmex_po_file ${MMEX_PO_FILES})
    get_filename_component(mmex_basename "${mmex_po_file}" NAME_WE)

    # map mmex basename to wxstd basename
    if(NOT DEFINED WXSTD_MAP_${mmex_basename})
        continue()
    endif()
    set(wxstd_basename "${WXSTD_MAP_${mmex_basename}}")

    # check if wxstd po file exists
    set(wxstd_po_file "${WXSTD_PO_DIRECTORY}/${wxstd_basename}.po")
    if(NOT EXISTS "${wxstd_po_file}")
        continue()
    endif()

    # copy wxstd file to CMAKE_CURRENT_BINARY_DIR
    file(COPY ${wxstd_po_file} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

    # compile wxstd po file
    add_custom_command(
        OUTPUT ${wxstd_basename}.mo
        COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} -v --statistics -o ${wxstd_basename}.mo ${wxstd_basename}.po
        COMMENT ""
        MAIN_DEPENDENCY "${wxstd_po_file}"
    )

    if(APPLE)
        set(WXSTD_MO_DIRECTORY ${MMEX_EXE}.app/Contents/Resources/${mmex_basename}.lproj)
    elseif(WIN32)
        set(WXSTD_MO_DIRECTORY bin/${mmex_basename})
    else()
        set(WXSTD_MO_DIRECTORY share/${MMEX_EXE}/${mmex_basename})
    endif()
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${wxstd_basename}.mo"
        DESTINATION ${WXSTD_MO_DIRECTORY}
        RENAME ${WXSTD}.mo
    )
    #message(STATUS "wxstd map: ${mmex_basename} -> ${wxstd_basename}")

    list(APPEND WXSTD_MO_FILES "${CMAKE_CURRENT_BINARY_DIR}/${wxstd_basename}.mo")
endforeach()

add_custom_target(wxstd_translations ALL DEPENDS ${WXSTD_MO_FILES}
    COMMENT "Generated wxstd translations")
