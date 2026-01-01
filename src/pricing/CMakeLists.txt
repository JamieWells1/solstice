add_library(pricing STATIC pricer.cpp equity_price_data.cpp future_price_data.cpp option_price_data.cpp pricing_utils.cpp greeks.cpp)

target_include_directories(pricing
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src/common
        ${CMAKE_SOURCE_DIR}/src/utils
        ${CMAKE_SOURCE_DIR}/src/enums
        ${CMAKE_SOURCE_DIR}/src/config
        ${CMAKE_SOURCE_DIR}/src/matching
        ${CMAKE_SOURCE_DIR}/src/broadcaster
)
