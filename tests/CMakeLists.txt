add_executable(thumbnails-test cache_test.cpp encoding_test.cpp)
target_link_libraries(thumbnails-test PRIVATE HolonightThumbnails::Thumbnails GTest::gtest GTest::gtest_main)
add_test(NAME thumbnails COMMAND thumbnails-test)
add_test(NAME installed-package COMMAND ${CMAKE_COMMAND}
  -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/package-consumer
  -DPROVIDER_BUILD=${PROJECT_BINARY_DIR}
  -DTEST_ROOT=${CMAKE_CURRENT_BINARY_DIR}/package-test
  -P ${CMAKE_CURRENT_SOURCE_DIR}/package-test.cmake)

if(HOLONIGHT_THUMBNAILS_ISOLATED)
  set(HOLONIGHT_THUMBNAILS_TEST_RUNTIME "" CACHE PATH "Explicit native runtime for cache acceptance")
  add_executable(thumbnails-isolated-test cache_test.cpp isolated_test.cpp)
  target_link_libraries(thumbnails-isolated-test PRIVATE HolonightThumbnails::Isolated GTest::gtest GTest::gtest_main)
  target_compile_definitions(thumbnails-isolated-test PRIVATE THUMBNAILS_ISOLATED_TEST THUMBNAILS_TEST_RUNTIME="${HOLONIGHT_THUMBNAILS_TEST_RUNTIME}")
  add_test(NAME thumbnails-isolated COMMAND thumbnails-isolated-test)
  add_executable(thumbnails-transaction-test cache_test.cpp)
  target_link_libraries(thumbnails-transaction-test PRIVATE HolonightThumbnails::Isolated GTest::gtest_main)
  target_compile_definitions(thumbnails-transaction-test PRIVATE THUMBNAILS_ISOLATED_TEST THUMBNAILS_TRANSACTION_TEST THUMBNAILS_TEST_RUNTIME="${HOLONIGHT_THUMBNAILS_TEST_RUNTIME}")
  add_test(NAME thumbnails-transaction COMMAND thumbnails-transaction-test)
  add_test(NAME installed-isolated-package COMMAND ${CMAKE_COMMAND}
    -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/package-consumer
    -DPROVIDER_BUILD=${PROJECT_BINARY_DIR}
    -DTEST_ROOT=${CMAKE_CURRENT_BINARY_DIR}/isolated-package-test
    -DISOLATED=ON "-DPROVIDER_PREFIX_PATH=${CMAKE_PREFIX_PATH}"
    -P ${CMAKE_CURRENT_SOURCE_DIR}/package-test.cmake)
endif()
