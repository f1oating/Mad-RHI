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