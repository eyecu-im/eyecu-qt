cmake_minimum_required(VERSION 2.8)

project(clienticons-plugin)
set(PLUGIN_NAME clienticons)

set(SOURCES clienticons.cpp)
set(HEADERS clienticons.h definitions.h)
set(MOC_HEADERS clienticons.h)

include_directories("${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}")

include("${CMAKE_SOURCE_DIR}/cmake/config.cmake")
include("${CMAKE_SOURCE_DIR}/translations/languages.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/plugin.cmake")

install(DIRECTORY "resources/clienticons" DESTINATION "${INSTALL_RESOURCES}")
