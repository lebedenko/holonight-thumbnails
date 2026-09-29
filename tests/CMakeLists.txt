add_executable(thumbnails-test cache_test.cpp)
target_link_libraries(thumbnails-test PRIVATE HolonightThumbnails::Thumbnails GTest::gtest GTest::gtest_main)
add_test(NAME thumbnails COMMAND thumbnails-test)
add_test(NAME installed-package COMMAND ${CMAKE_COMMAND}
  -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/package-consumer
  -DPROVIDER_BUILD=${PROJECT_BINARY_DIR}
  -DTEST_ROOT=${CMAKE_CURRENT_BINARY_DIR}/package-test
  -P ${CMAKE_CURRENT_SOURCE_DIR}/package-test.cmake)
