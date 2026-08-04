add_library(${CMAKE_PROJECT_NAME} SHARED ""
        DataStructs/Globals.h
        RealisticVehicleCallSystemNative.h
        RealisticVehicleCallSystemNative.cpp
        RedLogger.cpp
        RedLogger.h
        main.cpp
        DataStructs/Addresses.h
        DataStructs/Config.h
        DataStructs/GarageSerialized.h
        DataStructs/SlotSerialized.h
        DataStructs/Garage.h
        DataStructs/Slot.h
        GarageLoader.cpp
        GarageLoader.h
        FastRNG.cpp
        FastRNG.h
        RapidJsonHelper.cpp
        RapidJsonHelper.h
        TweakDBIDHasher.h
        GenericHelpers.h)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${PROJECT_HEADER_FILES} ${PROJECT_SRC_FILES})

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${PROJECT_HEADER_FILES} ${PROJECT_SRC_FILES})

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE RED4ext::SDK MinHook)

set(DEST_DIR "E:/Games/Cyberpunk 2077/red4ext/plugins/RealisticVehicleCallSystem")

add_custom_command(
        TARGET ${CMAKE_PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${DEST_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:${CMAKE_PROJECT_NAME}>"
        "${DEST_DIR}"
)