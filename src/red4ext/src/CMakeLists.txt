add_library(${CMAKE_PROJECT_NAME} SHARED ""
        DataStructs/Globals.h
        RealisticVehicleCallSystem.h
        RealisticVehicleCallSystem.cpp
        RedLogger.cpp
        RedLogger.h
        main.cpp)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${PROJECT_HEADER_FILES} ${PROJECT_SRC_FILES})

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${PROJECT_HEADER_FILES} ${PROJECT_SRC_FILES})

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE RED4ext::SDK)

set(DEST_DIR "E:/Games/Cyberpunk 2077/red4ext/plugins/RealisticVehicleCallSystem")

add_custom_command(
        TARGET ${CMAKE_PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${DEST_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:${CMAKE_PROJECT_NAME}>"
        "${DEST_DIR}"
)