cmake_minimum_required(VERSION 3.16)
project(MMEXMacOSLocaleChecks LANGUAGES CXX OBJCXX)

if(NOT APPLE)
    message(FATAL_ERROR "These checks exercise macOS native alerts.")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OBJCXX_STANDARD 17)
set(CMAKE_OBJCXX_STANDARD_REQUIRED ON)

find_package(wxWidgets 3.1.6 REQUIRED COMPONENTS core base)
include(${wxWidgets_USE_FILE})

# Standalone Cocoa reproducer: no wxWidgets or MMEX initialization.
add_executable(native-alert-probe native_alert_probe.mm)
target_link_libraries(native-alert-probe PRIVATE "-framework Cocoa")

# Link the production workaround rather than copying it into the test.
add_executable(locale-regression
    locale_regression.mm
    ../../src/base/mmPlatform.cpp)
target_include_directories(locale-regression PRIVATE
    ../../src ../../3rd/fmt/include)
target_compile_definitions(locale-regression PRIVATE FMT_HEADER_ONLY)
target_link_libraries(locale-regression PRIVATE ${wxWidgets_LIBRARIES}
    "-framework Cocoa")

enable_testing()
# Rendering tests are opt-in because they require a graphical login session.
add_test(NAME locale-formatting COMMAND locale-regression)
