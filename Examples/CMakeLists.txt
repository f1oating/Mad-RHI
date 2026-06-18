list(APPEND CMAKE_BUILD_RPATH "${CMAKE_BINARY_DIR}/Debug/lib") # embeds dynamic library path into exe (linux) temproary until I will find something normal

if(WIN32)
    include(FetchContent)
    FetchContent_Declare(
        agility_sdk
        URL "https://www.nuget.org/api/v2/package/Microsoft.Direct3D.D3D12/1.619.1"
    )
    FetchContent_MakeAvailable(agility_sdk)

    set(AGILITY_SDK_BIN_DIR "${agility_sdk_SOURCE_DIR}/build/native/bin/x64" CACHE INTERNAL "")
endif()

if(WIN32)
    function(copy_agility_sdk_dlls target)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${target}>/D3D12"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${AGILITY_SDK_BIN_DIR}/D3D12Core.dll"
                "${AGILITY_SDK_BIN_DIR}/D3D12SDKLayers.dll"
                "$<TARGET_FILE_DIR:${target}>/D3D12"
        )
    endfunction()
endif()

add_subdirectory(Common)

add_subdirectory(Triangle)
add_subdirectory(ExponentialFog)
add_subdirectory(GodRays)
add_subdirectory(Shadows)
add_subdirectory(PBR)

if(WIN32)
    set_target_properties(Mad-RHI PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/res
    )
endif()