add_subdirectory(matching)
add_subdirectory(pricing)
add_subdirectory(strategy)

add_subdirectory(broadcaster)
add_subdirectory(common)
add_subdirectory(utils)
add_subdirectory(config)
add_subdirectory(enums)

add_library(orchestrator STATIC
    orchestrator.cpp
)

target_include_directories(orchestrator
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/matching
        ${CMAKE_CURRENT_SOURCE_DIR}/pricing
        ${CMAKE_CURRENT_SOURCE_DIR}/broadcaster
        ${CMAKE_CURRENT_SOURCE_DIR}/common
        ${CMAKE_CURRENT_SOURCE_DIR}/utils
        ${CMAKE_CURRENT_SOURCE_DIR}/config
        ${CMAKE_CURRENT_SOURCE_DIR}/enums
)

target_link_libraries(orchestrator PUBLIC matching pricing broadcaster common utils config enums)

add_executable(solstice
    main.cpp
)

target_include_directories(solstice PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/matching
    ${CMAKE_CURRENT_SOURCE_DIR}/pricing
    ${CMAKE_CURRENT_SOURCE_DIR}/broadcaster
    ${CMAKE_CURRENT_SOURCE_DIR}/strategy
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/config
    ${CMAKE_CURRENT_SOURCE_DIR}/enums
)

# comment out to enable/disable logging
target_compile_definitions(solstice PRIVATE ENABLE_LOGGING)

target_link_libraries(solstice PRIVATE orchestrator strategy)

target_link_libraries(solstice PRIVATE ${Boost_LIBRARIES})
