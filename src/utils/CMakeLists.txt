add_library(utils STATIC
    get_random.cpp
    time_point.cpp
    truncate.cpp
    types.cpp
)

target_include_directories(utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
