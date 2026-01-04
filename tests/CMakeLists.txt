file(GLOB_RECURSE TEST_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/test_*.cpp
)

add_executable(tests ${TEST_SOURCES})

target_link_libraries(tests
    PRIVATE
    gtest_main
    ${Boost_LIBRARIES}
    orchestrator
    strategy
)

include(GoogleTest)
gtest_discover_tests(tests)

target_include_directories(tests PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/private
    ${PROJECT_SOURCE_DIR}/src/matching
    ${PROJECT_SOURCE_DIR}/src/pricing
    ${PROJECT_SOURCE_DIR}/src/strategy
    ${PROJECT_SOURCE_DIR}/src/common
    ${PROJECT_SOURCE_DIR}/src/enums
    ${PROJECT_SOURCE_DIR}/src/utils
    ${PROJECT_SOURCE_DIR}/src/config
)
