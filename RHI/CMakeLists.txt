file(GLOB_RECURSE MAD_RHI_SOURCES
    "src/*.cpp"
    "src/*.c"
)

add_library(Mad-RHI SHARED ${MAD_RHI_SOURCES})

target_include_directories(Mad-RHI
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/thirdparty/VMA/include
)

target_link_libraries(Mad-RHI
    PRIVATE
        volk
        spirv-reflect-static
        Vulkan::Headers
)

target_compile_definitions(Mad-RHI
    PRIVATE
        VK_NO_PROTOTYPES
        MAD_RHI_BUILD_DLL
        $<$<PLATFORM_ID:Windows>:NOMINMAX>
)
