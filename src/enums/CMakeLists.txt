add_library(enums STATIC market_side.cpp log_level.cpp strategy.cpp position_type.cpp order_type.cpp option_type.cpp)

target_include_directories(enums PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
