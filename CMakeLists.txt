cmake_minimum_required(VERSION 3.25)
project(holonight-thumbnails VERSION 0.1.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
find_package(Qt6 6.11 REQUIRED COMPONENTS Core Gui)

add_library(holonight_thumbnails STATIC src/cache.cpp)
add_library(HolonightThumbnails::Thumbnails ALIAS holonight_thumbnails)
set_target_properties(holonight_thumbnails PROPERTIES EXPORT_NAME Thumbnails POSITION_INDEPENDENT_CODE ON)
target_compile_features(holonight_thumbnails PUBLIC cxx_std_23)
target_compile_options(holonight_thumbnails PRIVATE -Wall -Wextra -Wpedantic)
target_include_directories(holonight_thumbnails PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(holonight_thumbnails PUBLIC Qt6::Core Qt6::Gui)

install(TARGETS holonight_thumbnails EXPORT HolonightThumbnailsTargets ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT HolonightThumbnailsTargets NAMESPACE HolonightThumbnails:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/HolonightThumbnails)
configure_package_config_file(cmake/HolonightThumbnailsConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/HolonightThumbnailsConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/HolonightThumbnails)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/HolonightThumbnailsConfigVersion.cmake
  VERSION ${PROJECT_VERSION} COMPATIBILITY SameMajorVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/HolonightThumbnailsConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/HolonightThumbnailsConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/HolonightThumbnails)
install(FILES LICENSE LICENSES/GPL-3.0-or-later.txt DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/${PROJECT_NAME})

include(CTest)
if(BUILD_TESTING)
  find_package(GTest REQUIRED)
  add_subdirectory(tests)
endif()
