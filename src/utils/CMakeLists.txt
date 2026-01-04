add_library(utils STATIC
    get_random.cpp
    time_point.cpp
    truncate.cpp
    types.cpp
)

target_include_directories(utils
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src/enums
        ${CMAKE_SOURCE_DIR}/src/pricing
        ${CMAKE_SOURCE_DIR}/src/config)

target_link_libraries(utils PUBLIC pricing config enums)
