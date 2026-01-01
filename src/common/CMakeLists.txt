add_library(common STATIC order.cpp transaction.cpp options.cpp)

target_include_directories(common
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src/pricing
        ${CMAKE_SOURCE_DIR}/src/broadcaster
        ${CMAKE_SOURCE_DIR}/src/matching
        ${CMAKE_SOURCE_DIR}/src/config
        ${CMAKE_SOURCE_DIR}/src/utils
        ${CMAKE_SOURCE_DIR}/src/enums
)
