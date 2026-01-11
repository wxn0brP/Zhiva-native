cmake_minimum_required(VERSION 3.16)
project(zhiva LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

include(FetchContent)

FetchContent_Declare(
    webview
    GIT_REPOSITORY https://github.com/webview/webview
    GIT_TAG 0.12.0)
FetchContent_MakeAvailable(webview)

add_executable(zhiva)
target_sources(zhiva PRIVATE main.cpp)

if(WIN32)
    target_sources(zhiva PRIVATE icon.rc)
endif()
target_link_libraries(zhiva PRIVATE webview::core)