add_library(config STATIC
    config.cpp
    logging.cpp
)

target_include_directories(config PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/enums
    ${CMAKE_SOURCE_DIR}/src/strategy
    ${CMAKE_SOURCE_DIR}/src/utils
)

target_link_libraries(config PRIVATE enums)
